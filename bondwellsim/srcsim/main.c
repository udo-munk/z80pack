/* SPDX-License-Identifier: BSD-3-Clause */
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "bondwell.h"
#include "simglb.h"
#include "simcore.h"
#include "simz80.h"
#include "simport.h"
static volatile sig_atomic_t stopping;
static void stop(int signal_number) { (void)signal_number; stopping = 1; }
static void usage(void)
{
    puts("Bondwell 12/14 emulator using z80pack\n"
         "Usage: bondwellsim [options]\n"
         "  --model 12|14       Hardware model (default 14)\n"
         "  --rom PATH          4096-byte boot ROM (default roms/BOOTROM.BIN)\n"
         "  --chargen PATH      4096-byte font ROM (default roms/CHAROM.BIN)\n"
         "  --disk-a PATH       Raw 180/360 KiB disk, hardware unit 1\n"
         "  --disk-b PATH       Second disk, hardware unit 2\n"
         "  --writable          Allow writes; save to PATH.updated on exit\n"
         "  --headless          Run without a display window\n"
         "  --cycles N          Stop after N T-states (default unlimited)\n"
         "  --turbo             Disable 4 MHz pacing\n"
         "  --trace PATH        Log port traffic and FDC commands\n"
         "  --cpu-trace PATH    Log instruction address and registers\n"
         "  --screenshot PATH   Save character-ROM rendering as PPM on exit\n"
         "  --text              Print ASCII view of video RAM on exit\n"
         "  --type TEXT         Type text through the keyboard after 15 seconds\n"
         "  --type-after N      Seconds before typing (default 15)\n"
         "  --type-delay N      Milliseconds between keys (default 100)\n"
         "  --serial-a-in/out PATH  Binary serial input/output file\n"
         "  --serial-b-in/out PATH  Binary serial input/output file\n"
         "  --printer PATH      Printer output file\n"
         "  --audio PATH        Timestamped DAC sample CSV\n"
         "Close the window or press Ctrl+C in the console to exit.");
}
int main(int argc, char **argv)
{
    const char *rom = "roms/BOOTROM.BIN", *font = "roms/CHAROM.BIN";
    const char *disks[2] = {NULL,NULL}, *trace = NULL, *screenshot = NULL;
    const char *serial_in[2] = {NULL,NULL}, *serial_out[2] = {NULL,NULL};
    const char *printer = NULL, *audio = NULL, *typing = NULL, *cpu_trace = NULL;
    uint64_t limit = 0, type_after = 60000000, type_delay = 400000;
    int model = 14;
    bool headless = false, writable = false, turbo = false, text = false;
    FILE *instructions = NULL;
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!strcmp(arg, "--help") || !strcmp(arg, "-h")) { usage(); return 0; }
        if (!strcmp(arg, "--headless")) { headless = true; continue; }
        if (!strcmp(arg, "--writable")) { writable = true; continue; }
        if (!strcmp(arg, "--turbo")) { turbo = true; continue; }
        if (!strcmp(arg, "--text")) { text = true; continue; }
        if (++i == argc) { fprintf(stderr, "Missing value for %s\n", arg); return 2; }
        const char *v = argv[i];
        if (!strcmp(arg, "--rom")) rom = v;
        else if (!strcmp(arg, "--chargen")) font = v;
        else if (!strcmp(arg, "--disk-a")) disks[0] = v;
        else if (!strcmp(arg, "--disk-b")) disks[1] = v;
        else if (!strcmp(arg, "--trace")) trace = v;
        else if (!strcmp(arg, "--cpu-trace")) cpu_trace = v;
        else if (!strcmp(arg, "--screenshot")) screenshot = v;
        else if (!strcmp(arg, "--type")) typing = v;
        else if (!strcmp(arg, "--printer")) printer = v;
        else if (!strcmp(arg, "--audio")) audio = v;
        else if (!strcmp(arg, "--serial-a-in")) serial_in[0] = v;
        else if (!strcmp(arg, "--serial-b-in")) serial_in[1] = v;
        else if (!strcmp(arg, "--serial-a-out")) serial_out[0] = v;
        else if (!strcmp(arg, "--serial-b-out")) serial_out[1] = v;
        else if (!strcmp(arg, "--model")) {
            if (!strcmp(v, "12")) model = 12;
            else if (!strcmp(v, "14")) model = 14;
            else { fprintf(stderr, "Model must be 12 or 14\n"); return 2; }
        } else if (!strcmp(arg, "--type-after") || !strcmp(arg, "--type-delay")) {
            char *end; errno = 0;
            uint64_t number = strtoull(v, &end, 0);
            if (errno || !*v || *end || *v == '-' || number > UINT64_MAX / 4000000) return 2;
            if (!strcmp(arg, "--type-after")) type_after = number * 4000000;
            else type_delay = number * 4000;
        } else if (!strcmp(arg, "--cycles")) {
            char *end; errno = 0;
            limit = strtoull(v, &end, 0);
            if (errno || !*v || *end || *v == '-') { fprintf(stderr, "Invalid cycle limit\n"); return 2; }
        } else { fprintf(stderr, "Unknown option: %s\n", arg); return 2; }
    }
    bw_reset(model);
    int status = 1;
    if (bw_load_rom(rom, bw.rom) || bw_load_rom(font, bw.chargen)) goto done;
    for (unsigned i = 0; i < 2; ++i) {
        if (disks[i] && bw_mount(i, disks[i], writable)) goto done;
        if (serial_in[i] && !(bw.sio[i].input = fopen(serial_in[i], "rb"))) { perror(serial_in[i]); goto done; }
        if (serial_out[i] && !(bw.sio[i].output = fopen(serial_out[i], "wb"))) { perror(serial_out[i]); goto done; }
    }
    if (trace && !(bw.trace = fopen(trace, "w"))) { perror(trace); goto done; }
    if (cpu_trace && !(instructions = fopen(cpu_trace, "w"))) { perror(cpu_trace); goto done; }
    if (printer && !(bw.printer = fopen(printer, "wb"))) { perror(printer); goto done; }
    if (audio && !(bw.audio = fopen(audio, "w"))) { perror(audio); goto done; }
    if (!headless && !bw_ui_open()) {
        fprintf(stderr, "Display unavailable. Use --headless, or build with SDL2 on Unix.\n"); goto done;
    }
    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    init_cpu(); reset_cpu();
    tmax = 40000; f_value = 0;
    uint64_t start = get_clock_us(), next_frame = 0, next_key = type_after;
    bool halted = false;
    fprintf(stderr, "Bondwell %d, 4 MHz Z80; ROM loaded, disks %s / %s\n", model,
            disks[0] ? disks[0] : "empty", disks[1] ? disks[1] : "empty");
    while (!stopping && (!limit || T < limit)) {
        bw_tick();
        /* Keep HALT in the device scheduler so the UI and timers still run. */
        if (halted && !int_nmi && !(int_int && IFF == 3)) T += 4;
        else if (!int_nmi && !(int_int && IFF == 3) && bw_mem_read(PC) == 0x76) {
            ++PC; ++R; T += 4; halted = true;
        } else {
            halted = false;
            if (instructions) fprintf(instructions,
                "%012" PRIu64 " PC=%04x OP=%02x AF=%02x%02x BC=%02x%02x DE=%02x%02x HL=%02x%02x SP=%04x BANK=%u\n",
                T, PC, bw_mem_read(PC), A, F & 255, B, C, D, E, H, L, SP, bw.latch & 3);
            cpu_state = ST_SINGLE_STEP; cpu_error = NONE; cpu_z80();
            if (cpu_error != NONE) { fprintf(stderr, "CPU error %d at %04x\n", cpu_error, PC); goto save; }
        }
        if (typing && *typing && T >= next_key) {
            bw_key_ascii((unsigned char)*typing++); next_key = T + type_delay;
        }
        if (T >= next_frame) {
            next_frame = T + 60000;
            if (!headless) {
                if (!bw_ui_poll()) break;
                bw_ui_draw();
            }
            if (!turbo) {
                uint64_t target = start + T / 4, now = get_clock_us();
                if (target > now) sleep_for_us((unsigned long)(target - now));
            }
        }
    }
    status = 0;
save:
    if (bw_save_disks()) status = 1;
    if (screenshot && bw_screenshot(screenshot)) status = 1;
    if (text) bw_text(stdout);
    fprintf(stderr, "Stopped PC=%04x bank=%u T=%" PRIu64 " FDC commands=%" PRIu64
            " sectors read=%" PRIu64 " written=%" PRIu64 "\n", PC, bw.latch & 3, T,
            bw.fdc.commands, bw.fdc.sectors_read, bw.fdc.sectors_written);
done:
    if (instructions) fclose(instructions);
    bw_ui_close(); bw_close();
    return status;
}

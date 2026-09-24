/* SPDX-License-Identifier: BSD-3-Clause */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bondwell.h"
#include "simglb.h"
#include "simcore.h"
#include "simz80.h"
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); } } while (0)
static void command(const BYTE *c, unsigned n)
{
    for (unsigned i = 0; i < n; ++i) bw_out(0x21, c[i]);
}
static void read_result(BYTE r[7])
{
    CHECK(bw_in(0x20) == 0xd0);
    for (unsigned i = 0; i < 7; ++i) r[i] = bw_in(0x21);
    CHECK(bw_in(0x20) == 0x80);
}
static void memory_test(void)
{
    bw_reset(14);
    bw.rom[0] = 0x42;
    CHECK(bw_mem_read(0) == 0x42 && bw_mem_read(0x7000) == 0x42);
    bw_mem_write(0, 9); CHECK(bw_mem_read(0) == 0x42);
    bw_out(1, 0); bw_mem_write(0, 1); bw_mem_write(0xf800, 0x55);
    bw_out(3, 0); bw_mem_write(0, 3);
    CHECK(bw_mem_read(0xf800) == 0x55);
    bw_out(0, 255); bw_mem_write(0, 2);
    bw_out(1, 255); CHECK(bw_mem_read(0) == 3);
    bw_in(2); CHECK(bw_mem_read(0) == 1);
    bw_in(0); CHECK(bw_mem_read(0) == 0x42);
    bw_reset(12); bw_in(3); bw_mem_write(0, 7); CHECK(bw_mem_read(0) == 0xff);
}
static void cpu_test(void)
{
    bw_reset(14);
    /* Real z80pack CPU copies ROM code to common RAM, selects RAM, stores A. */
    const BYTE program[] = {0x21,0x10,0,0x11,0,0x80,0x01,8,0,0xed,0xb0,0xc3,0,0x80,0,0,
                           0x3e,0x5a,0xd3,1,0x32,0,0,0};
    memcpy(bw.rom, program, sizeof program);
    init_cpu(); reset_cpu(); tmax = 40000;
    for (unsigned i = 0; i < 25; ++i) {
        cpu_state = ST_SINGLE_STEP; cpu_z80(); CHECK(cpu_error == NONE);
    }
    CHECK((bw.latch & 3) == 1 && bw_mem_read(0) == 0x5a);
}
static void fdc_test(void)
{
    bw_reset(14);
    bw.drive[0].bytes = malloc(368640); CHECK(bw.drive[0].bytes);
    bw.drive[0].size = 368640; bw.drive[0].heads = 2;
    for (unsigned i = 0; i < 368640; ++i) bw.drive[0].bytes[i] = i & 255;
    bw_out(0x0b, 0); CHECK(bw.motors);
    BYTE sense[] = {7,1}; command(sense, 2);
    CHECK(bw.fdc.irq);
    bw_out(0x21, 8); CHECK(bw_in(0x21) == 0x21); CHECK(bw_in(0x21) == 0);
    BYTE read[] = {0x46,1,0,0,0,1,17,12,255}; command(read, 9);
    CHECK(bw_in(0x20) == 0xf0);
    for (unsigned i = 0; i < 256; ++i) CHECK(bw_in(0x21) == (i & 255));
    bw_out(0x0f, 0); BYTE r[7]; read_result(r);
    CHECK(r[0] == 1 && r[1] == 0 && r[5] == 0);
    bw_out(0x0e, 0);
    BYTE write[] = {0x45,1,0,0,0,1,17,12,255}; command(write, 9);
    read_result(r); CHECK(r[1] == 2); /* write protection */
    bw.drive[0].writable = true; command(write, 9);
    for (unsigned i = 0; i < 256; ++i) bw_out(0x21, 0xa5);
    bw_out(0x0f, 0); read_result(r);
    CHECK(!r[1] && bw.drive[0].bytes[255] == 0xa5 && bw.drive[0].bytes[256] == 0);
    bw_out(0x0e, 0);
    read[4] = 18; command(read, 9); read_result(r); CHECK(r[1] == 4);
    read[1] = 0; read[4] = 0; command(read, 9); read_result(r); CHECK((r[0] & 0x48) == 0x48);
    bw_close();
}
static void peripheral_test(void)
{
    bw_reset(14); T = 0;
    /* SIO A registers use odd control port, B is the second pair. */
    bw_out(0x41, 4); bw_out(0x41, 0x44);
    bw_out(0x41, 5); bw_out(0x41, 8);
    CHECK(bw_in(0x41) & 4);
    bw.sio[0].output = tmpfile(); CHECK(bw.sio[0].output);
    bw_out(0x40, 0xa5); CHECK(!(bw_in(0x41) & 4));
    T = bw.sio[0].tx_until; CHECK(bw_in(0x41) & 4);
    rewind(bw.sio[0].output); CHECK(fgetc(bw.sio[0].output) == 0xa5);
    bw.sio[1].input = tmpfile(); CHECK(bw.sio[1].input);
    fputc(0x7e, bw.sio[1].input); rewind(bw.sio[1].input);
    bw_out(0x43, 3); bw_out(0x43, 1);
    CHECK(bw_in(0x43) & 1); CHECK(bw_in(0x42) == 0x7e);
    CHECK(!(bw_in(0x43) & 1));
    /* Counter 2, low/high byte, mode 3: count and latched read. */
    T = 0; bw_out(0x63, 0xb6); bw_out(0x62, 0x00); bw_out(0x62, 0x01);
    bw_out(0x31, 4); CHECK(bw_in(0x30) & 16);
    T = 300; CHECK(!(bw_in(0x30) & 16));
    bw_out(0x63, 0x80);
    unsigned low = bw_in(0x62); T += 1000;
    unsigned count = low | ((unsigned)bw_in(0x62) << 8);
    CHECK(count == 118);
    /* Queue preserves consecutive keys rather than replacing the first. */
    bw_queue_key(74, false, false); bw_queue_key(76, false, false);
    bw_tick(); CHECK(bw.key_ready && bw.key_head == 1);
    T += 200000; bw_tick(); CHECK(!bw.key_ready);
    T += 200000; bw_tick(); CHECK(bw.key_ready && bw.key_head == 2);
    bw_close();
}
static void keyboard_video_test(void)
{
    bw_reset(14);
    bw_out(0x31, 4); /* PA reads inputs */
    bw_key(74, false, false); /* A = matrix 74 -> encoded 0x10a */
    CHECK(bw_in(0x30) & 32);
    unsigned value = 0;
    static const unsigned order[9] = {6,3,1,0,2,4,5,7,8};
    for (unsigned i = 0; i < 9; ++i) {
        value |= ((bw_in(0x30) >> 6) & 1) << order[i];
        bw_out(0x33, 0x30); bw_out(0x33, 0x38);
    }
    CHECK(value == 0x10a);
    bw_key_up(); CHECK(bw_key_ascii('A')); bw_keyboard_tick(); CHECK(bw.key_bits[0] == 1);
    bw_key_up(); CHECK(!(bw_in(0x30) & 32));
    bw.crtc[1] = 80; bw.crtc[6] = 25; bw.crtc[9] = 8; bw.crtc[10] = 0x20;
    bw.common[0x7800] = 65; bw.chargen[65 * 16] = 0x81;
    BYTE pixels[640*256]; unsigned height;
    bw_render(pixels, &height);
    CHECK(height == 225 && pixels[0] && pixels[7] && !pixels[1] && !pixels[8]);
}
int main(void)
{
    memory_test(); cpu_test(); fdc_test(); peripheral_test(); keyboard_video_test();
    puts("Bondwell memory, real Z80 integration, FDC, keyboard and video tests passed");
    return 0;
}

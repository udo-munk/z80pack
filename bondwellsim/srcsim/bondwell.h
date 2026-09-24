/* SPDX-License-Identifier: BSD-3-Clause */
#ifndef BONDWELL_H
#define BONDWELL_H
#include <stdio.h>
#include "sim.h"
#include "simdefs.h"

typedef struct {
    BYTE *bytes;
    size_t size;
    unsigned heads;
    bool writable, dirty;
    const char *path;
} BWDrive;
typedef struct {
    BYTE command[9], result[7], sector[256];
    unsigned have, need, result_pos, result_len, pos;
    unsigned unit, head, cylinder, record, size_code, eot;
    BYTE pcn[4], pending_st[4];
    bool pending[4], irq, non_dma, writing, formatting, mt;
    unsigned phase, format_count, format_pos;
    BYTE format_id[4];
    uint64_t commands, sectors_read, sectors_written;
} BWFdc;
typedef struct {
    BYTE control, access, mode, write_phase, read_phase;
    unsigned reload, holding, latched;
    bool latch_valid, latch_high;
    uint64_t start;
} BWPit;
typedef struct {
    BYTE wr[8], pointer;
    FILE *input, *output;
    int received;
    uint64_t tx_until;
} BWSio;
typedef struct {
    int model;
    BYTE rom[4096], chargen[4096], ram[3][32768], common[32768];
    BYTE latch, crtc[32], crtc_index, pia[4], ddr[2], output[2], dac;
    BYTE key_bits[9];
    unsigned key_shift;
    unsigned key_queue[64], key_head, key_tail;
    uint64_t key_next;
    bool key_ready, cb2;
    uint64_t key_release, motor_deadline;
    bool motors;
    BWPit pit[3];
    BWSio sio[2];
    BWDrive drive[2];
    BWFdc fdc;
    FILE *trace, *printer, *audio;
} Bondwell;
extern Bondwell bw;
void bw_reset(int model);
int bw_load_rom(const char *path, BYTE *dest);
BYTE bw_mem_read(WORD address);
void bw_mem_write(WORD address, BYTE value);
BYTE bw_in(BYTE port);
void bw_out(BYTE port, BYTE value);
void bw_tick(void);
void bw_key(unsigned matrix, bool shift, bool control);
void bw_key_up(void);
void bw_queue_key(unsigned matrix, bool shift, bool control);
void bw_keyboard_tick(void);
bool bw_key_ascii(unsigned ch);
void bw_render(BYTE pixels[640 * 256], unsigned *height);
int bw_screenshot(const char *path);
void bw_text(FILE *out);
int bw_mount(unsigned drive, const char *path, bool writable);
int bw_save_disks(void);
void bw_fdc_write(BYTE value);
BYTE bw_fdc_read(void);
BYTE bw_fdc_status(void);
void bw_fdc_tc(void);
void bw_close(void);
bool bw_ui_open(void);
bool bw_ui_poll(void);
void bw_ui_draw(void);
void bw_ui_close(void);
#endif

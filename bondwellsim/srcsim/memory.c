/* SPDX-License-Identifier: BSD-3-Clause */
#include <string.h>
#include "bondwell.h"
#include "simglb.h"
Bondwell bw;

void bw_reset(int model)
{
    memset(&bw, 0, sizeof bw);
    bw.model = model;
    memset(bw.rom, 0xff, sizeof bw.rom);
    bw.sio[0].received = bw.sio[1].received = -1;
    bw.fdc.non_dma = true;
}

int bw_load_rom(const char *path, BYTE *dest)
{
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return -1; }
    size_t n = fread(dest, 1, 4096, f);
    int extra = fgetc(f), bad = ferror(f);
    fclose(f);
    if (n != 4096 || extra != EOF || bad) {
        fprintf(stderr, "%s: ROM must contain exactly 4096 bytes\n", path);
        return -1;
    }
    return 0;
}

BYTE bw_mem_read(WORD a)
{
    unsigned bank = bw.latch & 3;
    if (a >= 0x8000) return bw.common[a - 0x8000];
    if (!bank) return bw.rom[a & 0xfff];
    if (bw.model == 12 && bank > 1) return 0xff;
    return bw.ram[bank - 1][a];
}

void bw_mem_write(WORD a, BYTE value)
{
    unsigned bank = bw.latch & 3;
    if (a >= 0x8000) bw.common[a - 0x8000] = value;
    else if (bank && (bw.model == 14 || bank == 1))
        bw.ram[bank - 1][a] = value;
}

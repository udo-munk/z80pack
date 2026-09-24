/* SPDX-License-Identifier: BSD-3-Clause
 * uPD765 programmed-I/O subset for normal Bondwell sector disks.
 * Units 1 and 2 are A: and B: (the hardware leaves unit 0 unconnected).
 */
#include <stdlib.h>
#include <string.h>
#include "bondwell.h"
enum { IDLE, COMMAND, READ_DATA, WRITE_DATA, RESULT };

int bw_mount(unsigned index, const char *path, bool writable)
{
    if (index > 1) return -1;
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return -1; }
    if (fseek(f, 0, SEEK_END)) { fclose(f); return -1; }
    long size = ftell(f);
    if (size != 184320 && size != 368640) {
        fprintf(stderr, "%s: expected raw 180/360 KiB Bondwell disk; convert CopyQM first\n", path);
        fclose(f); return -1;
    }
    if (bw.model == 12 && size != 184320) {
        fprintf(stderr, "%s: Model 12 requires a single-sided disk\n", path);
        fclose(f); return -1;
    }
    BYTE *p = malloc((size_t)size);
    if (!p) { fclose(f); return -1; }
    rewind(f);
    bool good = fread(p, 1, size, f) == (size_t)size && !ferror(f);
    fclose(f);
    if (!good) { free(p); return -1; }
    free(bw.drive[index].bytes);
    bw.drive[index] = (BWDrive){p, (size_t)size, (unsigned)size / 184320,
                              writable, false, path};
    return 0;
}

int bw_save_disks(void)
{
    int status = 0;
    for (unsigned i = 0; i < 2; ++i) {
        BWDrive *d = &bw.drive[i];
        if (!d->dirty || !d->writable) continue;
        /* Save to a new sidecar, never truncate the original mounted disk. */
        size_t len = strlen(d->path) + 9;
        char *path = malloc(len);
        if (!path) { status = -1; continue; }
        snprintf(path, len, "%s.updated", d->path);
        FILE *f = fopen(path, "wb");
        bool ok = false;
        if (f) {
            ok = fwrite(d->bytes, 1, d->size, f) == d->size;
            if (fclose(f)) ok = false;
        }
        if (!ok) { fprintf(stderr, "Cannot save %s\n", path); status = -1; }
        else { fprintf(stderr, "Saved modified disk to %s\n", path); d->dirty = false; }
        free(path);
    }
    return status;
}

static BWDrive *drive(void)
{
    unsigned u = bw.fdc.unit;
    return (u == 1 || u == 2) ? &bw.drive[u - 1] : NULL;
}
static bool ready(void)
{
    BWDrive *d = drive();
    return d && d->bytes && bw.motors;
}
static BYTE st0(void) { return (BYTE)(bw.fdc.unit | (bw.fdc.head << 2)); }
static void result(unsigned count)
{
    bw.fdc.phase = RESULT;
    bw.fdc.result_len = count;
    bw.fdc.result_pos = 0;
}
static void finish(BYTE status1, BYTE status2)
{
    BWFdc *f = &bw.fdc;
    f->result[0] = st0() | ((status1 || status2) ? 0x40 : 0);
    if (!ready()) f->result[0] |= 0x48;
    f->result[1] = status1;
    f->result[2] = status2;
    f->result[3] = (BYTE)f->cylinder;
    f->result[4] = (BYTE)f->head;
    f->result[5] = (BYTE)f->record;
    f->result[6] = (BYTE)f->size_code;
    result(7);
    f->irq = true;
}
static size_t offset(void)
{
    BWFdc *f = &bw.fdc;
    return ((f->cylinder * drive()->heads + f->head) * 18 + f->record) * 256;
}
static bool sector_valid(void)
{
    BWFdc *f = &bw.fdc;
    if (!ready()) { finish(4, 0); return false; }
    if (f->cylinder >= 40 || f->head >= drive()->heads ||
        f->record >= 18 || f->size_code != 1) {
        finish(4, 0); return false;
    }
    if (f->cylinder != f->pcn[f->unit]) { finish(4, 0x10); return false; }
    if (f->writing && !drive()->writable) { finish(2, 0); return false; }
    return true;
}
static bool next_sector(void)
{
    BWFdc *f = &bw.fdc;
    if (f->record == f->eot) {
        if (f->mt && !f->head && drive()->heads == 2) {
            f->head = 1;
            f->record = 0;
        } else { finish(0x80, 0); return false; }
    } else ++f->record;
    f->pos = 0;
    return sector_valid();
}
static void execute(void)
{
    BWFdc *f = &bw.fdc;
    BYTE *c = f->command;
    unsigned op = c[0] & 31;
    f->commands++;
    if (bw.trace) {
        fprintf(bw.trace, "FDC command");
        for (unsigned i = 0; i < f->need; ++i) fprintf(bw.trace, " %02x", c[i]);
        fputc('\n', bw.trace);
    }
    f->unit = c[1] & 3;
    f->head = (c[1] >> 2) & 1;
    f->phase = IDLE;
    switch (op) {
    case 3: f->non_dma = (c[2] & 1) != 0; break; /* SPECIFY */
    case 4: /* SENSE DRIVE STATUS */
        f->result[0] = st0() | (f->pcn[f->unit] == 0 ? 0x10 : 0);
        /* Controller READY is tied active on the Bondwell. */
        if (drive()) f->result[0] |= 0x20;
        if (drive() && bw.model == 14) f->result[0] |= 8;
        if (drive() && !drive()->writable) f->result[0] |= 0x40;
        result(1); break;
    case 7: case 15: /* RECALIBRATE, SEEK */
        f->pcn[f->unit] = op == 7 ? 0 : c[2];
        f->pending[f->unit] = true;
        f->pending_st[f->unit] = st0() | (drive() ? 0x20 : 0x68);
        f->irq = true;
        break;
    case 8: /* SENSE INTERRUPT STATUS */
        f->result[0] = 0x80;
        result(1);
        for (unsigned u = 0; u < 4; ++u) {
            if (f->pending[u]) {
                f->result[0] = f->pending_st[u];
                f->result[1] = f->pcn[u];
                f->pending[u] = false;
                result(2); break;
            }
        }
        f->irq = false;
        for (unsigned u = 0; u < 4; ++u) f->irq |= f->pending[u];
        break;
    case 10: /* READ ID */
        f->cylinder = f->pcn[f->unit];
        f->record = 0; f->size_code = 1;
        finish(ready() && f->head < drive()->heads ? 0 : 4, 0);
        break;
    case 5: case 6: /* WRITE DATA, READ DATA */
        f->cylinder = c[2]; f->head = c[3]; f->record = c[4];
        f->size_code = c[5]; f->eot = c[6];
        f->writing = op == 5; f->formatting = false;
        f->mt = (c[0] & 0x80) != 0;
        f->pos = 0;
        if (sector_valid()) {
            f->phase = f->writing ? WRITE_DATA : READ_DATA;
            f->irq = f->non_dma;
        }
        break;
    case 13: /* FORMAT TRACK: receive SC four-byte CHRN tuples. */
        f->cylinder = f->pcn[f->unit]; f->record = 0;
        f->size_code = c[2]; f->eot = 17;
        f->writing = f->formatting = true;
        f->format_count = c[3]; f->format_pos = 0;
        if (sector_valid()) {
            if (!f->format_count || f->format_count > 18) finish(4, 0);
            else { f->phase = WRITE_DATA; f->irq = f->non_dma; }
        }
        break;
    default: f->result[0] = 0x80; result(1); break;
    }
}

BYTE bw_fdc_status(void)
{
    BWFdc *f = &bw.fdc;
    if ((f->phase == READ_DATA || f->phase == WRITE_DATA) && !f->formatting &&
        f->pos == 256 && f->record == f->eot &&
        !(f->mt && !f->head && drive()->heads == 2)) finish(0x80, 0);
    BYTE status = 0x80; /* RQM, no host-time-dependent busy spinning */
    if (f->phase != IDLE) status |= 0x10;
    if (f->phase == READ_DATA || f->phase == RESULT) status |= 0x40;
    if ((f->phase == READ_DATA || f->phase == WRITE_DATA) && f->non_dma)
        status |= 0x20;
    return status;
}

BYTE bw_fdc_read(void)
{
    BWFdc *f = &bw.fdc;
    if (f->phase == RESULT) {
        BYTE v = f->result[f->result_pos++];
        if (f->result_pos == f->result_len) {
            f->phase = IDLE;
            f->irq = false;
            for (unsigned u = 0; u < 4; ++u) f->irq |= f->pending[u];
        }
        return v;
    }
    if (f->phase != READ_DATA) return 0xff;
    if (f->pos == 256 && !next_sector()) return 0xff;
    BYTE value = drive()->bytes[offset() + f->pos++];
    if (f->pos == 256) f->sectors_read++;
    return value;
}

void bw_fdc_write(BYTE v)
{
    BWFdc *f = &bw.fdc;
    if (f->phase == WRITE_DATA) {
        if (f->formatting) {
            f->format_id[f->format_pos++ % 4] = v;
            if (f->format_pos % 4 == 0) {
                f->cylinder = f->format_id[0]; f->head = f->format_id[1];
                f->record = f->format_id[2]; f->size_code = f->format_id[3];
                if (!sector_valid()) return;
                memset(drive()->bytes + offset(), f->command[5], 256);
                drive()->dirty = true; ++f->sectors_written;
                if (f->format_pos == f->format_count * 4) finish(0, 0);
            }
        } else {
            if (f->pos == 256 && !next_sector()) return;
            f->sector[f->pos++] = v;
            if (f->pos == 256) {
                memcpy(drive()->bytes + offset(), f->sector, 256);
                drive()->dirty = true; ++f->sectors_written;
            }
        }
        return;
    }
    if (f->phase != IDLE && f->phase != COMMAND) return;
    if (f->phase == IDLE) {
        static const BYTE lengths[32] = {
            1,1,9,3,2,9,9,2,1,9,2,1,9,6,1,3,
            1,9,1,1,1,1,1,1,1,9,1,1,1,9,1,1
        };
        memset(f->command, 0, sizeof f->command);
        f->need = lengths[v & 31]; f->have = 0;
        f->phase = COMMAND;
    }
    f->command[f->have++] = v;
    if (f->have == f->need) execute();
}

void bw_fdc_tc(void)
{
    if (bw.fdc.phase == READ_DATA || bw.fdc.phase == WRITE_DATA)
        finish(0, 0);
}

void bw_close(void)
{
    for (unsigned i = 0; i < 2; ++i) {
        free(bw.drive[i].bytes); bw.drive[i].bytes = NULL;
        if (bw.sio[i].input) fclose(bw.sio[i].input);
        if (bw.sio[i].output) fclose(bw.sio[i].output);
    }
    if (bw.printer) fclose(bw.printer);
    if (bw.audio) fclose(bw.audio);
    if (bw.trace) fclose(bw.trace);
}

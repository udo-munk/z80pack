/* SPDX-License-Identifier: BSD-3-Clause */
#include "bondwell.h"
#include "simglb.h"
#include "simio.h"

static unsigned pit_count(BWPit *p)
{
    if (!p->reload) return 0;
    uint64_t ticks = (T - p->start) * 18432 / 40000;
    if (p->mode == 2 || p->mode == 3) return p->reload - ticks % p->reload;
    return ticks >= p->reload ? 0 : p->reload - (unsigned)ticks;
}
static bool pit_out(BWPit *p)
{
    if (!p->reload) return false;
    uint64_t ticks = (T - p->start) * 18432 / 40000;
    if (p->mode == 3) return ticks % p->reload < (p->reload + 1) / 2;
    if (p->mode == 2) return ticks % p->reload != p->reload - 1;
    return ticks >= p->reload;
}
static BYTE pit_read(unsigned channel)
{
    BWPit *p = &bw.pit[channel];
    unsigned count = p->latch_valid ? p->latched : pit_count(p);
    BYTE v;
    if (p->access == 2) v = count >> 8;
    else if (p->access == 3) {
        v = p->read_phase ? count >> 8 : count;
        p->read_phase ^= 1;
        if (p->read_phase) return v;
    } else v = count;
    p->latch_valid = false;
    return v;
}
static void pit_write(unsigned channel, BYTE v)
{
    if (channel == 3) {
        unsigned ch = v >> 6;
        if (ch == 3) return; /* 8253, not 8254 read-back */
        BWPit *p = &bw.pit[ch];
        if (!(v & 0x30)) {
            if (!p->latch_valid) { p->latched = pit_count(p); p->latch_valid = true; }
        } else {
            p->control = v; p->access = (v >> 4) & 3;
            p->mode = (v >> 1) & 7;
            if (p->mode > 5) p->mode -= 4;
            p->write_phase = p->read_phase = 0; p->latch_valid = false;
        }
        return;
    }
    BWPit *p = &bw.pit[channel];
    if (p->access == 3) {
        if (!p->write_phase) { p->holding = v; p->write_phase = 1; return; }
        p->holding |= (unsigned)v << 8; p->write_phase = 0;
    } else p->holding = p->access == 2 ? (unsigned)v << 8 : v;
    p->reload = p->holding ? p->holding : 65536;
    p->start = T;
}

static BYTE pia_inputs(void)
{
    /* Printer online, not busy; PA3 motors, PA4 PIT, PA5/6 keyboard,
       PA7 FDC interrupt. FDC IRQ does NOT directly interrupt the CPU. */
    return 2 | (bw.motors ? 8 : 0) | (pit_out(&bw.pit[2]) ? 16 : 0) |
           (bw.key_ready ? 32 : 0) |
           (bw.key_shift < 9 && bw.key_bits[bw.key_shift] ? 64 : 0) |
           (bw.fdc.irq ? 128 : 0);
}
static void cb2(bool state)
{
    if (state && !bw.cb2 && bw.key_shift < 9) ++bw.key_shift;
    bw.cb2 = state;
}
static BYTE pia_read(unsigned r)
{
    unsigned side = r >> 1;
    if (r & 1) return bw.pia[r];
    if (!(bw.pia[r + 1] & 4)) return bw.ddr[side];
    BYTE v = (bw.output[side] & bw.ddr[side]) |
             ((side ? 0xff : pia_inputs()) & (BYTE)~bw.ddr[side]);
    bw.pia[r + 1] &= 0x3f;
    return v;
}
static void pia_write(unsigned r, BYTE v)
{
    unsigned side = r >> 1;
    if (r & 1) {
        BYTE old = bw.pia[r];
        bw.pia[r] = (old & 0xc0) | (v & 0x3f);
        if (side && (v & 0x30) == 0x30) cb2((v & 8) != 0);
        /* CA2 manual falling edge is the printer strobe. */
        if (!side && (old & 0x38) == 0x38 && (v & 0x38) == 0x30 && bw.printer) {
            fputc(bw.output[1], bw.printer); fflush(bw.printer);
        }
    } else if (!(bw.pia[r + 1] & 4)) bw.ddr[side] = v;
    else {
        bw.output[side] = v;
        if (side && (bw.pia[3] & 0x30) == 0x20) { cb2(false); cb2(true); }
    }
}

static BYTE sio_read(unsigned r)
{
    unsigned channel = (r >> 1) & 1;
    BWSio *s = &bw.sio[channel];
    if (s->received < 0 && s->input && (s->wr[3] & 1)) s->received = fgetc(s->input);
    if (!(r & 1)) {
        BYTE v = s->received < 0 ? 0 : (BYTE)s->received;
        s->received = -1; return v;
    }
    unsigned reg = s->pointer; s->pointer = 0;
    if (reg == 0) return 0x28 | (T >= s->tx_until ? 4 : 0) | (s->received >= 0 ? 1 : 0); /* CTS,DCD,TX empty */
    if (reg == 1) return T >= s->tx_until ? 1 : 0; /* all sent, no parity/overrun errors */
    if (reg == 2) return bw.sio[1].wr[2];
    return 0;
}
static void sio_write(unsigned r, BYTE v)
{
    BWSio *s = &bw.sio[(r >> 1) & 1];
    if (!(r & 1)) {
        unsigned divisors[4] = {1,16,32,64};
        unsigned reload = bw.pit[(r >> 1) & 1].reload;
        if (!reload) reload = 12;
        s->tx_until = T + (uint64_t)reload * divisors[s->wr[4] >> 6] * 10 * 40000 / 18432;
        if (s->output && (s->wr[5] & 8)) { fputc(v, s->output); fflush(s->output); }
    } else if (s->pointer) {
        s->wr[s->pointer] = v; s->pointer = 0;
    } else {
        if ((v & 0x38) == 0x18) { /* channel reset */
            for (unsigned i = 0; i < 8; ++i) s->wr[i] = 0;
            s->received = -1;
        }
        s->pointer = v & 7;
    }
}

void bw_tick(void)
{
    if (!(bw.latch & 0x60) && bw.motors && T >= bw.motor_deadline) bw.motors = false;
    bw_keyboard_tick();
    int_int = ((bw.pia[1] & 0x81) == 0x81) || ((bw.pia[3] & 0x81) == 0x81);
    int_data = 0xff;
}
static void latch(BYTE port)
{
    BYTE mask = 1u << (port >> 1), old = bw.latch;
    if (port & 1) bw.latch |= mask; else bw.latch &= (BYTE)~mask;
    if (bw.latch & 0x60) bw.motors = true;
    else if (old & 0x60) bw.motor_deadline = T + 680000; /* 170 ms at 4 MHz */
    if ((bw.latch & 0x80) && !(old & 0x80)) bw_fdc_tc();
}
BYTE bw_in(BYTE port)
{
    BYTE v = 0xff;
    bw_tick();
    switch (port >> 4) {
    case 0: latch(port); v = 0; break;
    case 1: if (port & 1) v = bw.crtc[bw.crtc_index]; break;
    case 2: v = port & 1 ? bw_fdc_read() : bw_fdc_status(); break;
    case 3: v = pia_read(port & 3); break;
    case 4: v = sio_read(port & 3); break;
    case 6: if ((port & 3) != 3) v = pit_read(port & 3); break;
    }
    if (bw.trace) fprintf(bw.trace, "%012" PRIu64 " PC=%04x IN  %02x=%02x\n", T, PC, port, v);
    return v;
}
void bw_out(BYTE port, BYTE v)
{
    if (bw.trace) fprintf(bw.trace, "%012" PRIu64 " PC=%04x OUT %02x=%02x\n", T, PC, port, v);
    switch (port >> 4) {
    case 0: latch(port); break;
    case 1: if (port & 1) bw.crtc[bw.crtc_index] = v; else bw.crtc_index = v & 31; break;
    case 2: if (port & 1) bw_fdc_write(v); break;
    case 3: pia_write(port & 3, v); break;
    case 4: sio_write(port & 3, v); break;
    case 5:
        bw.dac = v;
        if (bw.audio) fprintf(bw.audio, "%" PRIu64 ",%u\n", T, v);
        break;
    case 6: pit_write(port & 3, v); break;
    }
    bw_tick();
}
static BYTE input(void) { return bw_in(io_port); }
static void output(BYTE v) { bw_out(io_port, v); }
#define EIGHT(x) x,x,x,x,x,x,x,x
#define SIXTEEN(x) EIGHT(x),EIGHT(x)
#define PORTS(x) SIXTEEN(x),SIXTEEN(x),SIXTEEN(x),SIXTEEN(x),SIXTEEN(x),SIXTEEN(x),SIXTEEN(x)
in_func_t *const port_in[256] = { PORTS(input) };
out_func_t *const port_out[256] = { PORTS(output) };

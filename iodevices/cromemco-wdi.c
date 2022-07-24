/*
 * cromemco-wdi.c
 *
 * Copyright (C) 2022 by David McNaughton
 *
 * Cromemco WDI-II Hard disk controller interface
 *
 * History:
 * 23-JUL-2022    1.0     Initial Release 
 *
 */

// #include "cromemco-wdi.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "sim.h"
#include "simglb.h"
#include "memory.h"

#define LOG_LOCAL_LEVEL LOG_ERROR
#include "log.h"

static const char *TAG = "wdi";

#define WMI_UNITS       4
#define WMI_HEADS       3
#define WMI_CYLINDERS   0x162
#define WMI_SECTORS     0x14
#define WMI_BLOCK_SIZE  512

#define WMI_MAX_BUFFER WMI_BLOCK_SIZE + 8
static BYTE buffer[WMI_MAX_BUFFER];

#ifdef HDD_IN_MEMORY
static BYTE hdd[WMI_HEADS][WMI_CYLINDERS][WMI_SECTORS][WMI_BLOCK_SIZE];
#endif

static int fd;
static int unit;

static char *images[] = { "disks/hd0.hdd", "disks/hd1.hdd", "disks/hd2.hdd", "disks/hd3.hdd"};

#define RPM 3600
#define INDEX_INT (1000000*60*4/RPM) /* Index interval in T ticks @ 4MHz */

enum wreg { WR_BASE, WR0, WR1, WR2, WR3, WR4, WR5, WR6 };
typedef enum wreg wreg_t;

enum rreg { RR_BASE, RR0, RR1, RR2, RR3, RR4, RR5, RR6 };
typedef enum rreg rreg_t;

enum dma_dev { MREQ, IORQ };
typedef enum dma_dev dma_dev_t;

static struct {
    struct {
        BYTE cmd_A;
        BYTE cmd_B;
        BYTE data_A;
        BYTE data_B;
    } pio0;
    struct {
        BYTE cmd_A;
        BYTE cmd_A_state;
        BYTE cmd_B;
        BYTE cmd_B_state;
        BYTE data_A;
        BYTE data_B;
        BYTE dir_A;
        BYTE dir_B;
        BYTE mode_A;
        BYTE mode_B;
    } pio1;
    struct {
        wreg_t wr_state;
        rreg_t rr_state;
        struct {
            BYTE base;
            BYTE dest;
            BYTE mode;
            WORD a_start;
            WORD len;
            WORD a_addr_counter;
            WORD byte_counter;
        } wr0;
        struct {
            BYTE base;
            BYTE a_timing;

            dma_dev_t a_device;
            BYTE a_fixed;
        } wr1;
        struct {
            BYTE base;
            BYTE b_timing;

            dma_dev_t b_device;
            BYTE b_fixed;
        } wr2;
        struct {
            BYTE base;
            BYTE mask;
            BYTE match;
        } wr3;
        struct {
            BYTE base;
            WORD b_start;
            WORD b_addr_counter;
            BYTE intr_control;
            BYTE pulse_control;
            BYTE intr_vector;
        } wr4;
        struct {
            BYTE base;
        } wr5;
        struct {
            BYTE base;
            BYTE mask;
        } wr6;
        struct {
            BYTE status;
        } rr0;
        BYTE force_ready;
        BYTE enable_dma;

        // BYTE __eob;
        // BYTE __memory;
    } dma;
    struct {
        BYTE mode0;
        BYTE mode1;
        BYTE mode2;
        BYTE time0;
        BYTE time1;
        BYTE time2;
        BYTE now0;
        BYTE now1;
        BYTE now2;
        BYTE state0;
        BYTE state1;
        BYTE state2;
        unsigned long long T0;
        unsigned long long T1;
        unsigned long long T2;
    } ctc;
    struct {
        BYTE _cmd_ak;
        BYTE brd;
        BYTE _seek_cmp;
        BYTE dsk_id;
        BYTE _cmd_stb;
        BYTE _cmd_sel1;
        BYTE _cmd_sel0;
        BYTE cmd__r_w;
        BYTE _crc_error;
        BYTE _sel_un_ad3;
        BYTE _sel_un_ad2;
        BYTE _sel_un_ad1;
        BYTE _sel_un_ad0;
        BYTE dma_rdy;
        BYTE _fault;
        BYTE _disk_op;

        BYTE bus_addr;
        BYTE sector;

        char *fn;

        struct {
            BYTE uas;
            BYTE has;
            WORD cas;
            // BYTE rezero;
            BYTE write_gate;
            BYTE read_gate;
        } command;
        struct {
            BYTE uav;
            BYTE hav;
            WORD cav;
            BYTE unit_rdy;
            BYTE on_cyl;
            BYTE seeking;
            BYTE rezeroing;
            BYTE write_prot;
        } status;
    } hd[WMI_UNITS];
} wdi;

void wdi_init(void)
{
    wdi.pio1.cmd_A = 0xff;
    wdi.pio1.cmd_B = 0xff;
    wdi.pio1.cmd_A_state = 0;
    wdi.pio1.cmd_B_state = 0;
    wdi.pio1.data_A = 0;
    wdi.pio1.data_B = 0;
    wdi.pio1.dir_A = 0xFF;
    wdi.pio1.dir_B = 0xFF;
    wdi.pio1.mode_A = 1;
    wdi.pio1.mode_B = 1;

    wdi.dma.wr_state = WR_BASE;
    wdi.dma.rr_state = RR_BASE;

    wdi.ctc.now0 = 0;
    wdi.ctc.now1 = 0;
    wdi.ctc.now2 = 0;

    wdi.ctc.state0 = 0;
    wdi.ctc.state1 = 0;
    wdi.ctc.state2 = 0;

    for (unit = 0; unit < WMI_UNITS; unit++) {

        wdi.hd[unit]._fault = 1;
        wdi.hd[unit]._crc_error = 0;

        // wdi.hd[unit].command.rezero = 0;
        wdi.hd[unit].command.write_gate = 0;
        wdi.hd[unit].command.read_gate = 0;

        wdi.hd[unit].status.unit_rdy = 0;
        wdi.hd[unit].status.on_cyl = 0;
        wdi.hd[unit].status.seeking = 0;
        wdi.hd[unit].status.rezeroing = 0;
        wdi.hd[unit].status.write_prot = 0;

        wdi.hd[unit].fn = images[unit];
    }

    unit = 0;
}
long wdi_pos(BYTE *buf)
{
    long p;
    const int c = WMI_CYLINDERS;
    const int s = WMI_SECTORS;
    const int b = WMI_BLOCK_SIZE;

    int cyl = buf[1] + (buf[2] << 8);

    p = buf[0] * c * s * b;
    p += cyl * s * b;
    p += buf[3] * b;

    // LOG(TAG, "POS h:%1d c:%3d s:%2d p:%8d\r", buf[0], cyl, buf[3], p);

    return p;
}
void wdi_dma_write(void)
{
    LOGI(TAG, "DISK WRITE: head: %d, cyl: %x", wdi.hd[unit].status.hav, wdi.hd[unit].status.cav);
    LOGI(TAG, "            start: %04x, addr: %04x, len: %d", wdi.dma.wr4.b_start, wdi.dma.wr4.b_addr_counter, wdi.dma.wr0.len);

    if (wdi.dma.wr0.len >= WMI_MAX_BUFFER) {
        wdi.hd[unit]._fault = 0; // SET FAULT
        LOGE(TAG, "DMA length: %d > buffer: %d", wdi.dma.wr0.len, WMI_MAX_BUFFER);
        return;
    }

    for (int i = 0; i <= wdi.dma.wr0.len; i++) {
        buffer[i] = dma_read(wdi.dma.wr4.b_addr_counter++);
    }

    LOGI(TAG, "            SYNC: %02x, HEAD: %02x, CYL: %02x%02x, SEC: %02x", buffer[0], buffer[1], buffer[3], buffer[2], buffer[4]);

    int cyl = (buffer[3] << 8 ) | buffer[2];

    if (wdi.hd[unit].status.hav != buffer[1]) {
        LOGE(TAG, "DISK WRITE ERROR - BAD HEAD");
        return;
    }
    if (wdi.hd[unit].status.cav != cyl) {
        LOGE(TAG, "DISK WRITE ERROR - BAD CYLINDER");
        return;
    }

#ifdef DEBUG
    register int sum = 0;
#endif

    for (int i = 0; i < WMI_BLOCK_SIZE; i++) {
#ifdef DEBUG
        sum += buffer[i+5];
#endif
#ifdef HDD_IN_MEMORY
        hdd[wdi.hd[unit].status.hav][wdi.hd[unit].status.cav][buffer[4]][i] = buffer[i+5];
    }
#else
    }
    struct stat s;

    if ((fd = open(wdi.hd[unit].fn, O_RDWR|O_CREAT, 0644)) == -1) {
        if ((fd = open(wdi.hd[unit].fn, O_RDONLY)) != -1) {
            // close(fd);
            wdi.hd[unit].status.write_prot = 1;
        } else {
            LOGE(TAG, "HD0 FILE DOES NOT EXIST - %s", wdi.hd[unit].fn);
            wdi.hd[unit]._fault = 0; // SET FAULT
        }
        return;
    }

    fstat(fd, &s);
    if (s.st_mode & S_IWUSR)
        wdi.hd[unit].status.write_prot = 0;
    else
        wdi.hd[unit].status.write_prot = 1;

    long pos = wdi_pos(&buffer[1]);

    if (lseek(fd, pos, SEEK_SET) == -1L) {
        wdi.hd[unit]._fault = 0; /* write fault */
        close(fd);
        return;
    }

    /* write the sector */
    if (write(fd, &buffer[5], WMI_BLOCK_SIZE) == WMI_BLOCK_SIZE)
        wdi.hd[unit]._fault = 1;
    else
        wdi.hd[unit]._fault = 0; /* write fault */

    close(fd);
#endif

#ifdef DEBUG
    if ( sum != (WMI_BLOCK_SIZE * 0xe5)) {
        LOGW(TAG, "SECTOR SUM %d IS DIFFERENT", sum);
        char txt[] = "................";
        char *t = txt;
        for (int i = 0; i < WMI_BLOCK_SIZE; i++) {
            if (!(i % 16)) {
                if (i) LOG(TAG, "  %s\n\r", txt);
                t = txt;
                LOG(TAG, "%04x:", i);
            }
            if (!(i % 4)) LOG(TAG, " ");
            register c = buffer[i+5];
            LOG(TAG, "%02x ", c);
            *t++ = (c>31 && c<127)?c:'.';
        }
        LOG(TAG, "\n\r");
    }
#endif
}
void wdi_dma_read(void)
{
    register int v;
    buffer[0] = wdi.hd[unit].status.hav;
    buffer[1] = wdi.hd[unit].status.cav & 0xff;
    buffer[2] = wdi.hd[unit].status.cav >> 8;
    buffer[3] = wdi.hd[unit].sector;

    LOGI(TAG, "DISK READ: head: %d, cyl: %d, sec: %d", wdi.hd[unit].status.hav, wdi.hd[unit].status.cav, wdi.hd[unit].sector);
    LOGI(TAG, "           start: %04x, addr: %04x, len: %d", wdi.dma.wr4.b_start, wdi.dma.wr4.b_addr_counter, wdi.dma.wr0.len);

    wdi.hd[unit].sector++;
    wdi.hd[unit].sector %= WMI_SECTORS;

#ifndef HDD_IN_MEMORY
    struct stat s;

    if ((fd = open(wdi.hd[unit].fn, O_RDONLY)) == -1) {
        LOGE(TAG, "HD0 FILE DOES NOT EXIST - %s", wdi.hd[unit].fn);
        wdi.hd[unit]._fault = 0; // SET FAULT
        return;
    }

    fstat(fd, &s);
    if (s.st_mode & S_IWUSR)
        wdi.hd[unit].status.write_prot = 0;
    else
        wdi.hd[unit].status.write_prot = 1;

    long pos = wdi_pos(buffer);

    if (lseek(fd, pos, SEEK_SET) == -1L) {
        wdi.hd[unit]._fault = 0; /* read fault */
        close(fd);
        return;
    }

    /* read the sector */
    if (read(fd, &buffer[4], WMI_BLOCK_SIZE) == WMI_BLOCK_SIZE) {
        wdi.hd[unit]._fault = 1; 
    } else {
        wdi.hd[unit]._fault = 0; /* read fault */
        close(fd);
        return;
    }

    close(fd);
#endif

    for (int i = 0; i < wdi.dma.wr0.len; i++) {
#ifdef HDD_IN_MEMORY
        if (i < 4) v = buffer[i];
        else v = hdd[wdi.hd[unit].status.hav][wdi.hd[unit].status.cav][buffer[3]][i-4];
#else
        v = buffer[i];
#endif

        dma_write(wdi.dma.wr4.b_addr_counter, v);
        // dma_write(wdi.dma.wr0.a_addr_counter++, v);
        // LOG(TAG, "%04x: %02x\n\r", wdi.dma.wr4.b_addr_counter, dma_read(wdi.dma.wr4.b_addr_counter));
        wdi.dma.wr4.b_addr_counter++;
    }
}
BYTE cromemco_wdi_pio0a_data_in(void)
{
	LOGE(TAG, "E0 IN:");
	return((BYTE) 0xFF);
}
BYTE cromemco_wdi_pio0b_data_in(void)
{
    BYTE val = 0;
    int bus = wdi.hd[unit].bus_addr;

	LOGD(TAG, "E1 IN: - bus %d",bus);

    switch (bus)
    {
        case 0:
        case 1:
        case 2:
        case 3:
	        val = wdi.pio0.data_A;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
	        val = wdi.pio0.data_B;
            break;
        default:
            break;
    }
    LOGD(TAG, "STATUS %d = %02x", bus, val);
	return(val);
}
BYTE cromemco_wdi_pio0a_cmd_in(void)
{
	LOGW(TAG, "E2 IN:");
	return((BYTE) 0xff);
}
BYTE cromemco_wdi_pio0b_cmd_in(void)
{
	LOGW(TAG, "E3 IN:");
	return((BYTE) 0xff);
}
BYTE cromemco_wdi_pio1a_data_in(void)
{
	BYTE val = 0;

    val |= wdi.hd[unit].brd;
    val |= wdi.hd[unit].status.seeking << 5;
    if (!wdi.hd[unit]._cmd_stb) {
        val |= 0x80; /* CMD_AK */
    }
    val = (wdi.pio1.data_A & ~wdi.pio1.dir_A) | val;

    LOGD(TAG, "E4 IN: = %02x", val);
	return(val);
}
BYTE cromemco_wdi_pio1b_data_in(void)
{
	BYTE val = wdi.hd[unit]._fault << 1;
    val |= wdi.hd[unit]._crc_error << 7;
    val |= wdi.hd[unit].status.uav << 3;

    val = (wdi.pio1.data_B & ~wdi.pio1.dir_B) | val;

	LOGD(TAG, "E5 IN: = %02x", val);
	return(val);
}
BYTE cromemco_wdi_pio1a_cmd_in(void)
{
	LOGW(TAG, "E6 IN:");
	return((BYTE) 0xff);
}
BYTE cromemco_wdi_pio1b_cmd_in(void)
{
	LOGW(TAG, "E7 IN:");
	return((BYTE) 0xff);
}
BYTE cromemco_wdi_dma0_in(void)
{
    /* DMA READ STATUS NOT YET COMPLETE */
	LOGE(TAG, "E8 IN: [%d]", wdi.dma.rr_state);
    if (wdi.dma.rr_state == RR_BASE) return wdi.dma.rr0.status;
    else return((BYTE) 0xff);
}
BYTE cromemco_wdi_dma1_in(void)
{
	LOGW(TAG, "E9 IN:");
	return((BYTE) 0xff);
}
BYTE cromemco_wdi_dma2_in(void)
{
	LOGW(TAG, "EA IN:");
	return((BYTE) 0xff);
}
BYTE cromemco_wdi_dma3_in(void)
{
	LOGW(TAG, "EB IN:");
	return((BYTE) 0xff);
}
BYTE cromemco_wdi_ctc0_in(void)
{
    BYTE val = wdi.ctc.now0;
	LOGD(TAG, "IN: CTC #0 = %02x - Tdiff=%lld", val, T - wdi.ctc.T0);

    if (T < wdi.ctc.T0) wdi.ctc.T0 = T; /* clock rollover has occured in theta */
    else if ((T - wdi.ctc.T0) > INDEX_INT) {
        if (val > 0) wdi.ctc.now0--; // DOESN'T HANDLE MULTIPLES YET
        wdi.hd[unit].sector = 0;
        wdi.ctc.T0 += INDEX_INT;
    }
	return(val);
}
BYTE cromemco_wdi_ctc1_in(void)
{
    unsigned long long Tdiff = T - wdi.ctc.T1;
    unsigned int sectors = (Tdiff * WMI_SECTORS + INDEX_INT / 10) / INDEX_INT; /* +10% on sector time */

	LOGD(TAG, "IN: CTC #1 = %02x - Tdiff=%lld = %d sectors", wdi.ctc.now1, T - wdi.ctc.T1, sectors);

    if (sectors) {
        if (wdi.ctc.now1 > 0) { 
            int r = (int)wdi.ctc.now1 - sectors;
            if (r < 0) r = 0;
            wdi.ctc.now1 = r;
        }
        wdi.hd[unit].sector += sectors;
        wdi.hd[unit].sector %= WMI_SECTORS;
        // LOGI(TAG, "IN: CTC #1 = %02x, sect: %02x", wdi.ctc.now1, wdi.hd[unit].sector);

        wdi.ctc.T1 = T;
    }

	return(wdi.ctc.now1);
}
BYTE cromemco_wdi_ctc2_in(void)
{
    BYTE val = wdi.ctc.now2;
	LOGD(TAG, "IN: CTC #2 = %d", val);

/**
 * COULD SIMULATE HEAD SEEK TIME HERE 
 * IMI-7710 spec gives:
 *   single track access time   10 ms
 *   average access time        50 ms
 *   maximum access time        100 ms
 * 
 * For now, seek is complete the first time this counter is checked
 */

    if (val > 0) wdi.ctc.now2--;

	return(val);
}
BYTE cromemco_wdi_ctc3_in(void)
{
	LOGW(TAG, "EF IN:");
	return((BYTE) 0xff);
}

void command_bus_strobe(void)
{

    int bus = wdi.hd[unit].bus_addr;
    BYTE data = wdi.pio0.data_A;

    switch (bus)
    {
        case 0:
            unit = data >> 4;
            wdi.hd[unit].command.uas = unit;
            wdi.hd[unit].command.has = (data & 0x0c) >> 2;
            wdi.hd[unit].command.cas = (wdi.hd[unit].command.cas & 0xff) | ((data & 0x03) << 8);
            wdi.hd[unit].status.seeking = 1;
            wdi.hd[unit].status.on_cyl = 0;
            wdi.hd[unit].status.unit_rdy = 0;

            LOGI(TAG, "DISK COMMAND 0 - UNIT: %02x, HEAD: %02x, CYL: %03x", wdi.hd[unit].command.uas, wdi.hd[unit].command.has, wdi.hd[unit].command.cas);
            break;
        case 1:
            wdi.hd[unit].command.cas = (wdi.hd[unit].command.cas & 0xf00) | data;
            wdi.hd[unit].status.cav = wdi.hd[unit].command.cas;
            wdi.hd[unit].status.seeking = 1;
            wdi.hd[unit].status.on_cyl = 0;
            wdi.hd[unit].status.unit_rdy = 0;

            LOGI(TAG, "DISK COMMAND 1 - CYL: %03x", wdi.hd[unit].status.cav);
            break;
        case 2:           
            if (data & 0xfc) {
                LOGE(TAG, "DISK COMMAND 2 - %d NOT IMPLEMENTED - %02x", bus, data);
            }
            break;
        case 3:
            if (data & 1) {
                LOGI(TAG, "DISK COMMAND 3 - FAULT CLEAR ");
                wdi.hd[unit]._fault = 1; /* CLEAR FAULT */
            }
            if (data & 2) {
                LOGI(TAG, "DISK COMMAND 3 - REZERO");
                
                wdi.hd[unit].status.rezeroing = 1;
                wdi.hd[unit].status.on_cyl = 0;
                wdi.hd[unit].status.unit_rdy = 0;
                wdi.hd[unit]._fault = 1; /* CLEAR FAULT */
            }
            break;
        case 4:
            wdi.pio0.data_B = wdi.hd[unit].status.unit_rdy;
            wdi.pio0.data_B |= wdi.hd[unit].status.on_cyl << 1;
            wdi.pio0.data_B |= wdi.hd[unit].status.seeking << 2;
            wdi.pio0.data_B |= wdi.hd[unit].status.rezeroing << 3;
            break;
        case 5:
	        wdi.pio0.data_B = 0x00;
            break;
        case 6:
            wdi.pio0.data_B = wdi.hd[unit].status.cav & 0xff;
            break;
        case 7:
            wdi.pio0.data_B = wdi.hd[unit].status.cav >> 8;
            wdi.pio0.data_B |= wdi.hd[unit].status.uav << 4;
            wdi.pio0.data_B |= wdi.hd[unit].status.hav << 2;
            break;
        default:
            break;
    }
}
void cromemco_wdi_pio0a_data_out(BYTE data)
{
    int bus = wdi.hd[unit].bus_addr;

	LOGD(TAG, "E0 OUT: %02x - bus:%d", data, bus);
    wdi.pio0.data_A = data;

    // if (bus == 2) {
        wdi.hd[unit].command.write_gate = data & 1;
        wdi.hd[unit].command.read_gate = data & 2;
    // }
}
void cromemco_wdi_pio0b_data_out(BYTE data)
{
	LOGD(TAG, "E1 OUT: %02x", data);
    wdi.pio0.data_B = data;

    if (data) LOGE(TAG, "Extended Address non-zero [%02x]", data);
}
void cromemco_wdi_pio0a_cmd_out(BYTE data)
{
	LOGW(TAG, "E2 OUT: %02x", data);
}
void cromemco_wdi_pio0b_cmd_out(BYTE data)
{
	LOGW(TAG, "E3 OUT: %02x", data);
}
void cromemco_wdi_pio1a_data_out(BYTE data)
{
	LOGD(TAG, "E4 OUT: %02x", data);
    wdi.pio1.data_A = data;
    wdi.hd[unit].dsk_id = data & 0x10;
    wdi.hd[unit].brd = wdi.hd[unit].dsk_id << 2;
    wdi.hd[unit]._cmd_stb = data & 0x08;
    wdi.hd[unit]._cmd_sel1 = data & 0x04;
    wdi.hd[unit]._cmd_sel0 = data & 0x02;
    wdi.hd[unit].cmd__r_w = data & 0x01;

    wdi.hd[unit].bus_addr = (wdi.hd[unit].cmd__r_w << 2) | (wdi.hd[unit]._cmd_sel1 >> 1) | (wdi.hd[unit]._cmd_sel0 >> 1);
	LOGD(TAG, "OUT - PIO A : %02x - bus: %d", data, wdi.hd[unit].bus_addr);

    if (wdi.hd[unit]._cmd_stb) {
	    LOGD(TAG, "PIO A - STROBE: %02x - bus: %d", data, wdi.hd[unit].bus_addr);

        command_bus_strobe();
    }
}
void cromemco_wdi_pio1b_data_out(BYTE data)
{
	LOGD(TAG, "E5 OUT: %02x", data);
    wdi.pio1.data_B = data;
    wdi.hd[unit].dma_rdy = data & 0x04;
    wdi.hd[unit]._disk_op = data & 0x01;

	LOGD(TAG, "E5 OUT: %02x, DMA RDY: %d, DISK_OP: %d", data, wdi.hd[unit].dma_rdy, wdi.hd[unit]._disk_op);

    if (wdi.hd[unit]._disk_op) {
        if (wdi.hd[unit].status.rezeroing) {
            LOGI(TAG, "REZEROING");
            wdi.hd[unit].status.hav = 0;
            wdi.hd[unit].status.cav = 0;
            wdi.hd[unit].status.rezeroing = 0;
            wdi.hd[unit].status.on_cyl = 1;
            wdi.hd[unit].status.unit_rdy = 1;
        }
        if (wdi.hd[unit].status.seeking) {
            LOGI(TAG, "SEEKING: UNIT: %02x, HEAD: %02x, CYL: %03x", wdi.hd[unit].command.uas, wdi.hd[unit].command.has, wdi.hd[unit].command.cas);
            wdi.hd[unit].status.uav = wdi.hd[unit].command.uas;
            wdi.hd[unit].status.hav = wdi.hd[unit].command.has;
            wdi.hd[unit].status.cav = wdi.hd[unit].command.cas;
            wdi.hd[unit].status.seeking = 0;
            wdi.hd[unit].status.on_cyl = 1;
            wdi.hd[unit].status.unit_rdy = 1;
        }
    }

    if (wdi.hd[unit].dma_rdy && wdi.hd[unit].command.write_gate) {
        wdi_dma_write();
    };
    if (wdi.hd[unit].dma_rdy && wdi.hd[unit].command.read_gate) {
        wdi_dma_read();
    }
}
void cromemco_wdi_pio1a_cmd_out(BYTE data)
{
	LOGD(TAG, "E6 OUT: %02x", data);
    wdi.pio1.cmd_A = data;

    switch (wdi.pio1.cmd_A_state)
    {
        case 0:
            if (data & 0x0f) {
                if ((wdi.pio1.mode_A = data >> 6) == 3) {
                    wdi.pio1.cmd_A_state = 1;
                } else {
                    LOGW(TAG, "Unexpected PIO A mode [%02x]", wdi.pio1.mode_A);
                }
            } else {
                LOGW(TAG, "Unexpected PIO A command [%02x]", data);
            }
            break;
        case 1:
            wdi.pio1.cmd_A_state = 0;
            wdi.pio1.dir_A = data;
            LOGD(TAG, "PIO1_A DIR=[%02x]", data);

            break;
        default:
            LOGE(TAG, "Unknown PIO A cmd state [%d]", wdi.pio1.cmd_A_state);
            break;
    }
}
void cromemco_wdi_pio1b_cmd_out(BYTE data)
{
	LOGD(TAG, "E7 OUT: %02x", data);
    wdi.pio1.cmd_B = data;

    switch (wdi.pio1.cmd_B_state)
    {
        case 0:
            if (data & 0x0f) {
                if ((wdi.pio1.mode_B = data >> 6) == 3) {
                    wdi.pio1.cmd_B_state = 1;
                } else {
                    LOGW(TAG, "Unexpected PIO B mode [%02x]", wdi.pio1.mode_B);
                }
            } else {
                LOGW(TAG, "Unexpected PIO B command [%02x]", data);
            }
            break;
        case 1:
            wdi.pio1.cmd_B_state = 0;
            wdi.pio1.dir_B = data;
            LOGD(TAG, "PIO1_B DIR=[%02x]", data);
            break;
        default:
            LOGE(TAG, "Unknown PIO B cmd state [%d]", wdi.pio1.cmd_B_state);
            break;
    }
}
void cromemco_wdi_dma0_out(BYTE data)
{
    char *cmd;

    cmd = "";

	LOGD(TAG, "E8 OUT: %02x", data);

    switch (wdi.dma.wr_state) 
    {
        case WR_BASE:

            if (data & 0x80) { /* WR4 - WR6 */

                if ((data & 0x03) == 1) { /* WR4 */
                    wdi.dma.wr4.base = data;
                    if (wdi.dma.wr4.base & 0x1C) {
                        wdi.dma.wr_state = WR4;
                    }
                } else if ((data & 0x03) == 2) { /* WR5 */
                    wdi.dma.wr5.base = data;
                    LOGD(TAG, "WR5 = %02x", data);
                } else if ((data & 0x03) == 3) { /* WR6 */
                    wdi.dma.wr6.base = data;

                    switch (data) 
                    {
                        case 0x83:
                            cmd = "DISABLE DMA";
                            wdi.dma.enable_dma = 0;
                            break;
                        case 0x87:
                            cmd = "ENABLE DMA";
                            wdi.dma.enable_dma = 1;

                            if (wdi.dma.force_ready 
                                    && wdi.dma.wr0.dest
                                    && (wdi.dma.wr0.mode == 3) /* TRANSFER/SEARCH */
                                    && (wdi.dma.wr1.a_device == MREQ)) {
                                
                                LOGD(TAG, "DMA: MEMORY-TO-MEMORY from: %04x to: %04x length: %04x", 
                                    wdi.dma.wr0.a_addr_counter,
                                    wdi.dma.wr4.b_addr_counter,
                                    wdi.dma.wr0.len
                                );

                                register int v;
                                for (int i = 0; i <= wdi.dma.wr0.len; i++) {
                                    v = dma_read(wdi.dma.wr0.a_addr_counter++);
                                    dma_write(wdi.dma.wr4.b_addr_counter++, v);
                                }
                            } else if (!wdi.dma.force_ready
                                    && !wdi.dma.wr0.dest
                                    && (wdi.dma.wr0.mode == 2) /* SEARCH */
                                    && (wdi.dma.wr1.a_device == MREQ)) {

                                LOGD(TAG, "DMA: R/W-TO/FROM-DISK addr: %04x length: %04x", 
                                    wdi.dma.wr4.b_addr_counter,
                                    wdi.dma.wr0.len
                                );
                                LOGD(TAG, "UNIT: %d, HEAD: %d, CYL: %d", wdi.hd[unit].status.uav, wdi.hd[unit].status.hav, wdi.hd[unit].status.cav);

                            } else {
                                LOGE(TAG, "DMA FUNCTION NOT IMPLEMENTED");
                                LOGE(TAG, "FORCE_READY = %02x", wdi.dma.force_ready);
                                LOGE(TAG, "DEVICE = %02x", wdi.dma.wr1.a_device);
                                LOGE(TAG, "MODE = %02x", wdi.dma.wr0.mode);
                                LOGE(TAG, "DEST = %02x", wdi.dma.wr0.dest);
                            }
                            break;
                        case 0xB3:
                            cmd = "FORCE READY";
                            wdi.dma.force_ready = 1;
                            break;
                        case 0xC3:
                            cmd = "RESET";
                            wdi.dma.enable_dma = 0;
                            wdi.dma.force_ready = 0;
                            wdi.dma.wr1.a_timing = 0;
                            wdi.dma.wr2.b_timing = 0;
                            break;
                        case 0xcf:
                            cmd = "LOAD";
                            wdi.dma.wr0.a_addr_counter = wdi.dma.wr0.a_start;
                            wdi.dma.wr4.b_addr_counter = wdi.dma.wr4.b_start;
                            wdi.dma.wr0.byte_counter = 0;
                            wdi.dma.force_ready = 0;
                            break;
                        default:
                            cmd = "NOT IMPLEMENTED";
                    }
                    LOGD(TAG, "DMA WR6 = %02x - %s", data, cmd);
                } else {
                    LOGW(TAG, "DMA WR [%02x] NOT IMPLEMENTED", data);
                }
            } else { /* WR0 - WR3 */

                if (data & 0x03) { /* WR0 */
                    wdi.dma.wr0.base = data;
                    wdi.dma.wr0.dest = data & 0x04;
                    wdi.dma.wr0.mode = data & 0x03;

                    LOGD(TAG, "DMA WR0: source %c, mode %d", wdi.dma.wr0.dest?'A':'B', wdi.dma.wr0.mode);

                    if (wdi.dma.wr0.base & 0x78) {
                        wdi.dma.wr_state = WR0;
                    }
                } else { /* WR1 - WR3 */

                    if ((data & 0x07) == 4) { /* WR1 */
                        wdi.dma.wr1.base = data;
                        wdi.dma.wr1.a_device = (data & 0x08) >> 3;
                        wdi.dma.wr1.a_fixed = (data & 0x30) >> 4;
                        if (wdi.dma.wr1.base & 0x40) {
                            wdi.dma.wr_state = WR1;
                        }
                        LOGD(TAG, "DMA WR1: Port A: inc. mode %d, dev %c", wdi.dma.wr1.a_device, wdi.dma.wr1.a_device?'I':'M');
                    } else if ((data & 0x07) == 0) { /* WR2 */
                        wdi.dma.wr2.base = data;
                        wdi.dma.wr2.b_device = (data & 0x08) >> 3;
                        wdi.dma.wr2.b_fixed = (data & 0x30) >> 4;
                        if (wdi.dma.wr2.base & 0x40) {
                            wdi.dma.wr_state = WR2;
                        }
                        LOGD(TAG, "DMA WR2: Port B: inc. mode %d, dev %c", wdi.dma.wr1.a_device, wdi.dma.wr1.a_device?'I':'M');
                    } else {

                        LOGW(TAG, "WR [%02x] NOT IMPLEMENTED", data);
                    }
                }
            }
            break;
        case WR0:
            if (wdi.dma.wr0.base & 0x08) {
                wdi.dma.wr0.a_start = (wdi.dma.wr0.a_start & 0xff00) | data;
                wdi.dma.wr0.base &= ~0x08;
            } else if (wdi.dma.wr0.base & 0x10) {
                wdi.dma.wr0.a_start = (wdi.dma.wr0.a_start & 0xff) | (data << 8);
                wdi.dma.wr0.base &= ~0x10;
            } else if (wdi.dma.wr0.base & 0x20) {
                wdi.dma.wr0.len = (wdi.dma.wr0.len & 0xff00) | data;
                wdi.dma.wr0.base &= ~0x20;
            } else if (wdi.dma.wr0.base & 0x40) {
                wdi.dma.wr0.len = (wdi.dma.wr0.len & 0xff) | (data << 8);
                wdi.dma.wr0.base &= ~0x40;
            }  

            if (!(wdi.dma.wr0.base & 0x78)) {
                wdi.dma.wr_state = WR_BASE;
                LOGD(TAG, "DMA WR0 [%02x] A start: %04x len: %02x", wdi.dma.wr0.base, wdi.dma.wr0.a_start, wdi.dma.wr0.len);
            }
            break;
        case WR1:
            wdi.dma.wr1.a_timing = data;
            wdi.dma.wr_state = WR_BASE;
            LOGD(TAG, "DMA WR1 [%02x] A timing: %02x", wdi.dma.wr1.base, wdi.dma.wr1.a_timing);
            break;
        case WR2:
            wdi.dma.wr2.b_timing = data;
            wdi.dma.wr_state = WR_BASE;
            LOGD(TAG, "DMA WR2 [%02x] B timing: %02x", wdi.dma.wr2.base, wdi.dma.wr2.b_timing);
            break;
        case WR4:
            if (wdi.dma.wr4.base & 0x04) {
                wdi.dma.wr4.b_start = (wdi.dma.wr4.b_start & 0xff00) | data;
                wdi.dma.wr4.base &= ~0x04;
            } else if (wdi.dma.wr4.base & 0x08) {
                wdi.dma.wr4.b_start = (wdi.dma.wr4.b_start & 0xff) | (data << 8);
                wdi.dma.wr4.base &= ~0x08;
            } else if (wdi.dma.wr4.base & 0x10) {
                wdi.dma.wr4.intr_control = data;
                wdi.dma.wr4.base &= ~0x10;
            }

            if (!(wdi.dma.wr4.base & 0x1C)) {
                wdi.dma.wr_state = WR_BASE;
                LOGD(TAG, "DMA WR4 [%02x] B start: %04x int: %02x", wdi.dma.wr4.base, wdi.dma.wr4.b_start, wdi.dma.wr4.intr_control);
            }
            break;
        default:
            LOGE(TAG, "DMA WR STATE [%d] NOT IMPLEMENTED", wdi.dma.wr_state);
            break;
    }
}

void cromemco_wdi_dma1_out(BYTE data)
{
	LOGW(TAG, "E9 OUT: %02x", data);
}
void cromemco_wdi_dma2_out(BYTE data)
{
	LOGW(TAG, "EA OUT: %02x", data);
}
void cromemco_wdi_dma3_out(BYTE data)
{
	LOGW(TAG, "EB OUT: %02x", data);
}
void cromemco_wdi_ctc0_out(BYTE data)
{
	LOGD(TAG, "OUT: CTC #0 - %02x", data);

    switch (wdi.ctc.state0) 
    {
        case 0:
            wdi.ctc.mode0 = data;

            if (data & 0x80 ) LOGE(TAG, "CTC #0 - INT set");
            if (!(data & 0x40)) LOGE(TAG, "CTC #0 - TIMER set");
            if (data & 0x04 ) wdi.ctc.state0 = 1;
            if (data & 0x02 ) wdi.ctc.now0 = 0;
            if (!(data & 0x01)) LOGE(TAG, "CTC #0 - not CONTROL");
            break;
        case 1:
            if (wdi.ctc.mode0 & 0x40) LOGD(TAG, "CTC #0 - COUNTER set to %02x", data);
            wdi.ctc.state0 = 0;
            wdi.ctc.time0 = data;
            wdi.ctc.now0 = data;
            wdi.ctc.T0 = T;
            break;
        default:
        LOGE(TAG, "CTC #0 - unknown state %d", wdi.ctc.state0);
    }
}
void cromemco_wdi_ctc1_out(BYTE data)
{
    LOGD(TAG, "OUT: CTC #1 - %02x", data);

    switch (wdi.ctc.state1) 
    {
        case 0:
            wdi.ctc.mode1 = data;

            if (data & 0x80 ) LOGE(TAG, "CTC #1 - INT set");

            if (!(data & 0x40)) {
                LOGD(TAG, "CTC #1 - TIMER set");
            } else {
                LOGD(TAG, "CTC #1 - COUNTER set");
            }

            if (data & 0x04 ) wdi.ctc.state1 = 1;
            if (data & 0x02 ) wdi.ctc.now1 = 0;
            if (!(data & 0x01)) LOGE(TAG, "CTC #1 - not CONTROL");
            break;
        case 1:
            wdi.ctc.state1 = 0;
            wdi.ctc.time1 = data;
            wdi.ctc.now1 = data;
            wdi.ctc.T1 = T;

            if (wdi.ctc.mode1 & 0x40) {
                LOGI(TAG, "CTC #1 - COUNTER set to %02x", data);
            } else {
                LOGI(TAG, "CTC #1 - TIMER set to %02x", data);
            }

            break;
        default:
        LOGE(TAG, "CTC #1 - unknown state %d", wdi.ctc.state0);
    }
}
void cromemco_wdi_ctc2_out(BYTE data)
{
	LOGD(TAG, "OUT: CTC #2 - %02x", data);

    switch (wdi.ctc.state2) 
    {
        case 0:
            wdi.ctc.mode2 = data;

            if (data & 0x80 ) LOGE(TAG, "CTC #2 - INT set");
            if (!(data & 0x40)) LOGE(TAG, "CTC #2 - TIMER set");
            if (data & 0x04 ) wdi.ctc.state2 = 1;
            if (data & 0x02 ) wdi.ctc.now2 = 0;
            if (!(data & 0x01)) LOGE(TAG, "CTC #2 - not CONTROL");
            break;
        case 1:
            wdi.ctc.state2 = 0;
            wdi.ctc.time2 = data;
            wdi.ctc.now2 = data;
            wdi.ctc.T2 = T;
            break;
        default:
        LOGE(TAG, "CTC #2 - unknown state %d", wdi.ctc.state2);
    }
}
void cromemco_wdi_ctc3_out(BYTE data)
{
	LOGW(TAG, "EF OUT: %02x", data);
}

// #include "cromemco-wdi.h"
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"

#define LOG_LOCAL_LEVEL LOG_INFO
#include "log.h"

static const char *TAG = "wdi";

BYTE wdi_index = 0;

BYTE buffer[520];

BYTE hdd[3][0x162][20][512];

struct timeval t1, t2;
int tdiff;
extern int time_diff(struct timeval *, struct timeval *);

#define RPM 5000
#define INDEX_INT (1000000*60/RPM)

enum wreg { WR_BASE, WR0, WR1, WR2, WR3, WR4, WR5, WR6 };
typedef enum wreg wreg_t;

enum rreg { RR_BASE, RR0, RR1, RR2, RR3, RR4, RR5, RR6 };
typedef enum rreg rreg_t;

enum dma_dev { MREQ, IORQ };
typedef enum dma_dev dev_t;

struct {
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

            dev_t a_device;
            BYTE a_fixed;
        } wr1;
        struct {
            BYTE base;
            BYTE b_timing;

            dev_t b_device;
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

        BYTE __eob;
        BYTE __memory;
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

        struct {
            BYTE uas;
            BYTE has;
            int cas;
            BYTE rezero;
            BYTE write_gate;
            BYTE read_gate;
        } command;
        struct {
            BYTE uav;
            BYTE hav;
            int cav;
            BYTE unit_rdy;
            BYTE on_cyl;
            BYTE seeking;
            BYTE rezeroing;
        } status;
    } hd0;
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

    wdi.ctc.state0 = 0;
    wdi.ctc.state1 = 0;
    wdi.ctc.state2 = 0;

    wdi.hd0._fault = 1;
    wdi.hd0._crc_error = 1;

    wdi.hd0.command.rezero = 0;
    wdi.hd0.command.write_gate = 0;
    wdi.hd0.command.read_gate = 0;

    wdi.hd0.status.unit_rdy = 0;
    wdi.hd0.status.on_cyl = 0;
    wdi.hd0.status.seeking = 0;
    wdi.hd0.status.rezeroing = 0;

    wdi_index = 0;
}
BYTE e0_in(void)
{
	LOGE(TAG, "E0 IN:");
	return((BYTE) 0xFF);
}
BYTE e1_in(void)
{
    BYTE val;
    int bus = wdi.hd0.bus_addr;

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
        // case 4:
        //     val = wdi.hd0.status.unit_rdy;
        //     val |= wdi.hd0.status.on_cyl << 1;
        //     val |= wdi.hd0.status.seeking << 2;
        //     val |= wdi.hd0.status.rezeroing << 3;
        //     break;
        // case 5:
        //     // LOGE(TAG, "STATUS %d NOT IMPLEMENTED", bus);
	    //     val = 0x00;
        //     break;
        // case 6:
        //     val = wdi.hd0.status.cav & 0xff;
        //     break;
        // case 7:
        //     val = wdi.hd0.status.cav >> 8;
        //     val |= wdi.hd0.status.uav << 4;
        //     val |= wdi.hd0.status.hav << 2;
        //     break;
        default:
            break;
    }
    LOGD(TAG, "STATUS %d = %02x", bus, val);
	return(val);
}
BYTE e2_in(void)
{
	LOGW(TAG, "E2 IN:");
	return((BYTE) 0xff);
}
BYTE e3_in(void)
{
	LOGW(TAG, "E3 IN:");
	return((BYTE) 0xff);
}
BYTE e4_in(void)
{
	BYTE val = 0;

    val |= wdi.hd0.brd;
    if (!wdi.hd0._cmd_stb) {
        val |= 0x80; // CMD_AK
    }
    val = (wdi.pio1.data_A & ~wdi.pio1.dir_A) | val;

    LOGD(TAG, "E4 IN: = %02x", val);
	return(val);
}
BYTE e5_in(void)
{
	BYTE val = wdi.hd0._fault << 1;
    val |= wdi.hd0._crc_error << 7;
    val |= wdi.hd0.status.uav << 3;

    val = wdi.pio1.data_B & ~wdi.pio1.dir_B | val;

	LOGD(TAG, "E5 IN: = %02x", val);
	return(val);
}
BYTE e6_in(void)
{
	LOGW(TAG, "E6 IN:");
	return((BYTE) 0xff);
}
BYTE e7_in(void)
{
	LOGW(TAG, "E7 IN:");
	return((BYTE) 0xff);
}
BYTE e8_in(void)
{
    // DMA READ STATUS NOT YET COMPLETE
	LOGD(TAG, "E8 IN: [%d]", wdi.dma.rr_state);
    if (wdi.dma.rr_state == RR_BASE) return wdi.dma.rr0.status; // ####
    else return((BYTE) 0xff);
}
BYTE e9_in(void)
{
	LOGW(TAG, "E9 IN:");
	return((BYTE) 0xff);
}
BYTE ea_in(void)
{
	LOGW(TAG, "EA IN:");
	return((BYTE) 0xff);
}
BYTE eb_in(void)
{
	LOGW(TAG, "EB IN:");
	return((BYTE) 0xff);
}
BYTE ec_in(void)
{
    BYTE val = wdi.ctc.now0;
	// LOGD(TAG, "IN: CTC #0 = %d", val);

    gettimeofday(&t2, NULL);
    tdiff = time_diff(&t1, &t2);
	// LOGI(TAG, "IN: CTC #0 = %d, diff = %d", val, tdiff);
	// LOGI(TAG, "IN: CTC #0 = %d, index = %d", val, wdi_index);

    // if (wdi_index > 4) {
    //     if (val > 0) wdi.ctc.now0--;
	//     LOGD(TAG, "IN: CTC #0 = %d, index = %d", val, wdi_index);
    //     wdi_index = 0;
    // }
    if (tdiff > 16666) {
        if (val > 0) wdi.ctc.now0--;
        gettimeofday(&t1, NULL);
    }
	return(val);
}
BYTE ed_in(void)
{
    BYTE val = wdi.ctc.now1;
	LOGI(TAG, "IN: CTC #1 = %d", wdi.ctc.now1);

    if (val > 0) wdi.ctc.now1--;

	return(val);
}
BYTE ee_in(void)
{
    BYTE val = wdi.ctc.now2;
	LOGD(TAG, "IN: CTC #2 = %d", val);

    // gettimeofday(&t2, NULL);
    // tdiff = time_diff(&t1, &t2);

    // if (tdiff > 100000) {
        if (val > 0) wdi.ctc.now2--;
    //     gettimeofday(&t1, NULL);
    // }

	return(val);
}
BYTE ef_in(void)
{
	LOGW(TAG, "EF IN:");
	return((BYTE) 0xff);
}

void e0_command(void)
{

    int bus = wdi.hd0.bus_addr;
    BYTE data = wdi.pio0.data_A;

    switch (bus)
    {
        case 0:
            wdi.hd0.command.uas = data >> 4;
            wdi.hd0.command.has = (data & 0x0c) >> 2;
            wdi.hd0.command.cas = (wdi.hd0.command.cas & 0xff) | ((data & 0x03) << 8);
            wdi.hd0.status.seeking = 1;

            LOGI(TAG, "DISK COMMAND 0 - UNIT: %02x, HEAD: %02x, CYL: %03x", wdi.hd0.command.uas, wdi.hd0.command.has, wdi.hd0.command.cas);
            break;
        case 1:
            wdi.hd0.command.cas = (wdi.hd0.command.cas & 0xf00) | data;
            wdi.hd0.status.cav = wdi.hd0.command.cas;
            wdi.hd0.status.seeking = 1;

            LOGI(TAG, "DISK COMMAND 1 - CYL: %03x", wdi.hd0.status.cav);
            break;
        case 2:           
            if (data & 0xfc) {
                LOGE(TAG, "DISK COMMAND 2 - %d NOT IMPLEMENTED - %02x", bus, data);
            }
            break;
        case 3:
            if (data & 1) {
                LOGI(TAG, "DISK COMMAND 3 - FAULT CLEAR ");
                wdi.hd0._fault = 1;  // CLEAR FAULT
            }
            if (data & 2) {
                LOGI(TAG, "DISK COMMAND 3 - REZERO");
                // wdi.hd0.command.rezero = 1;

                // wdi.hd0.status.hav = 0;
                // wdi.hd0.status.cav = 0;
                
                wdi.hd0.status.rezeroing = 1;
            }
            break;
        // case 4:
        // case 5:
        // case 6:
        // case 7:
        //     LOGE(TAG, "DISK STATUS %d NOT IMPLEMENTED for COMMAND = %02x", bus, data);
        //     break;
        case 4:
            wdi.pio0.data_B = wdi.hd0.status.unit_rdy;
            wdi.pio0.data_B |= wdi.hd0.status.on_cyl << 1;
            wdi.pio0.data_B |= wdi.hd0.status.seeking << 2;
            wdi.pio0.data_B |= wdi.hd0.status.rezeroing << 3;
            break;
        case 5:
            // LOGE(TAG, "STATUS %d NOT IMPLEMENTED", bus);
	        wdi.pio0.data_B = 0x00;
            break;
        case 6:
            wdi.pio0.data_B = wdi.hd0.status.cav & 0xff;
            break;
        case 7:
            wdi.pio0.data_B = wdi.hd0.status.cav >> 8;
            wdi.pio0.data_B |= wdi.hd0.status.uav << 4;
            wdi.pio0.data_B |= wdi.hd0.status.hav << 2;
            break;
        default:
            break;
    }
}
void e0_out(BYTE data)
{
    int bus = wdi.hd0.bus_addr;

	LOGD(TAG, "E0 OUT: %02x - bus:%d", data, bus);
    wdi.pio0.data_A = data;

    // if (bus == 2) {
        wdi.hd0.command.write_gate = data & 1;
        wdi.hd0.command.read_gate = data & 2;
    // }
}
void e1_out(BYTE data)
{
	LOGD(TAG, "E1 OUT: %02x", data);
    wdi.pio0.data_B = data;

    if (data) LOGE(TAG, "Extended Address non-zero [%02x]", data);
}
void e2_out(BYTE data)
{
	LOGW(TAG, "E2 OUT: %02x", data);
}
void e3_out(BYTE data)
{
	LOGW(TAG, "E3 OUT: %02x", data);
}
void e4_out(BYTE data)
{
	LOGD(TAG, "E4 OUT: %02x", data);
    wdi.pio1.data_A = data;
    wdi.hd0.dsk_id = data & 0x10;
    wdi.hd0.brd = wdi.hd0.dsk_id << 2;
    wdi.hd0._cmd_stb = data & 0x08;
    wdi.hd0._cmd_sel1 = data & 0x04;
    wdi.hd0._cmd_sel0 = data & 0x02;
    wdi.hd0.cmd__r_w = data & 0x01;

    wdi.hd0.bus_addr = (wdi.hd0.cmd__r_w << 2) | (wdi.hd0._cmd_sel1 >> 1) | (wdi.hd0._cmd_sel0 >> 1);
	LOGD(TAG, "OUT - PIO A : %02x - bus: %d", data, wdi.hd0.bus_addr);

    if (wdi.hd0._cmd_stb) {
	    LOGI(TAG, "PIO A - STROBE: %02x - bus: %d", data, wdi.hd0.bus_addr);
        e0_command();

        // wdi.hd0.status.cav = wdi.hd0.command.cas;
        // wdi.hd0.status.uav = wdi.hd0.command.uas;
        // wdi.hd0.status.hav = wdi.hd0.command.has;
    }
}
void e5_out(BYTE data)
{
	LOGD(TAG, "E5 OUT: %02x", data);
    wdi.pio1.data_B = data;
    wdi.hd0.dma_rdy = data & 0x04;
    wdi.hd0._disk_op = data & 0x01;

	LOGI(TAG, "E5 OUT: %02x, DMA RDY: %d, DISK_OP: %d", data, wdi.hd0.dma_rdy, wdi.hd0._disk_op);

    if (wdi.hd0._disk_op) {
        if (wdi.hd0.status.rezeroing) {
            LOGI(TAG, "REZEROING");
            wdi.hd0.status.hav = 0;
            wdi.hd0.status.cav = 0;
            wdi.hd0.status.rezeroing = 0;
        }
        if (wdi.hd0.status.seeking) {
            LOGI(TAG, "SEEKING: UNIT: %02x, HEAD: %02x, CYL: %03x", wdi.hd0.command.uas, wdi.hd0.command.has, wdi.hd0.command.cas);
            wdi.hd0.status.uav = wdi.hd0.command.uas;
            wdi.hd0.status.hav = wdi.hd0.command.has;
            wdi.hd0.status.cav = wdi.hd0.command.cas;
            wdi.hd0.status.seeking = 0;
        }
    }

    if (wdi.hd0.dma_rdy && wdi.hd0.command.write_gate) {
        LOGI(TAG, "DISK WRITE: head: %d, cyl: %x", wdi.hd0.status.hav, wdi.hd0.status.cav);
        LOGI(TAG, "            start: %04x, addr: %04x, len: %d", wdi.dma.wr4.b_start, wdi.dma.wr4.b_addr_counter, wdi.dma.wr0.len);

        for (int i = 0; i <= wdi.dma.wr0.len; i++) {
            buffer[i] = dma_read(wdi.dma.wr4.b_addr_counter++);
        }

        int cyl = (buffer[3] << 8 ) | buffer[2];
        register sum = 0;

        for (int i = 0; i < 512; i++) {
            sum += buffer[i+5];
            hdd[wdi.hd0.status.hav][wdi.hd0.status.cav][buffer[4]][i] = buffer[i+5];
        }
        
        LOGI(TAG, "            SYNC: %02x, HEAD: %02x, CYL: %02x%02x, SEC: %02x, SUM: %d, END: %02x", buffer[0], buffer[1], buffer[3], buffer[2], buffer[4], sum, buffer[517]);

        if ( sum != (512 * 0xe5)) {
            LOGW(TAG, "SECTOR SUM IS DIFFERENT");
            // LOGI(TAG, "            BUF: %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x  %02x %02x %02x %02x", 
            //         buffer[5], buffer[6], buffer[7], buffer[8],
            //         buffer[9], buffer[10], buffer[11], buffer[12],
            //         buffer[13], buffer[14], buffer[15], buffer[16],
            //         buffer[17], buffer[18], buffer[19], buffer[20]);
            char txt[] = "................";
            char *t = txt;
            for (int i = 0; i < 512; i++) {
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

        if (wdi.hd0.status.hav != buffer[1]) {
            LOGE(TAG, "DISK WRITE ERROR - BAD HEAD");
        }
        if (wdi.hd0.status.cav != cyl) {
            LOGE(TAG, "DISK WRITE ERROR - BAD CYLINDER");
        }
    };
    if (wdi.hd0.dma_rdy && wdi.hd0.command.read_gate) {

        register v;
        buffer[0] = wdi.hd0.status.hav;
        buffer[1] = wdi.hd0.status.cav & 0xff;
        buffer[2] = wdi.hd0.status.cav >> 8;
        buffer[3] = wdi.ctc.now1 % 0x14;

        LOGI(TAG, "DISK READ: head: %d, cyl: %d, sec: %d", wdi.hd0.status.hav, wdi.hd0.status.cav, buffer[3]);
        LOGI(TAG, "           start: %04x, addr: %04x, len: %d", wdi.dma.wr4.b_start, wdi.dma.wr4.b_addr_counter, wdi.dma.wr0.len);

        for (int i = 0; i <= wdi.dma.wr0.len; i++) {
            v = buffer[i];
            dma_write(wdi.dma.wr4.b_addr_counter++, v);
            // dma_write(wdi.dma.wr0.a_addr_counter++, v);
        }
    }
}
void e6_out(BYTE data)
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
	                // LOGW(TAG, "E6 OUT: %02x", data);
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
void e7_out(BYTE data)
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
void e8_out(BYTE data)
{
    char *cmd;

    cmd = "";

	LOGD(TAG, "E8 OUT: %02x", data);

    switch (wdi.dma.wr_state) 
    {
        case WR_BASE:

            if (data & 0x80) { // WR4 - WR6

                if ((data & 0x03) == 1) { //WR4
                    wdi.dma.wr4.base = data;
                    if (wdi.dma.wr4.base & 0x1C) {
                        wdi.dma.wr_state = WR4;
                    }
                } else if ((data & 0x03) == 2) { //WR5
                    wdi.dma.wr5.base = data;
                    LOGD(TAG, "WR5 = %02x", data);
                } else if ((data & 0x03) == 3) { //WR6
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
                                    && (wdi.dma.wr0.mode == 3) // TRANSFER/SEARCH
                                    && (wdi.dma.wr1.a_device == MREQ)) {
                                
                                LOGD(TAG, "DMA: MEMORY-TO-MEMORY from: %04x to: %04x length: %04x", 
                                    wdi.dma.wr0.a_addr_counter,
                                    wdi.dma.wr4.b_addr_counter,
                                    wdi.dma.wr0.len
                                );

                                register v;
                                for (int i = 0; i <= wdi.dma.wr0.len; i++) {
                                    v = dma_read(wdi.dma.wr0.a_addr_counter++);
                                    dma_write(wdi.dma.wr4.b_addr_counter++, v);
                                }
                            } else if (!wdi.dma.force_ready
                                    && !wdi.dma.wr0.dest
                                    && (wdi.dma.wr0.mode == 2) // SEARCH
                                    && (wdi.dma.wr1.a_device == MREQ)) {

                                LOGD(TAG, "DMA: R/W-TO/FROM-DISK addr: %04x length: %04x", 
                                    wdi.dma.wr4.b_addr_counter,
                                    wdi.dma.wr0.len
                                );
                                LOGD(TAG, "UNIT: %d, HEAD: %d, CYL: %d", wdi.hd0.status.uav, wdi.hd0.status.hav, wdi.hd0.status.cav);

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
                    LOGD(TAG, "WR6 = %02x - %s", data, cmd);
                } else {
                    LOGW(TAG, "WR [%02x] NOT IMPLEMENTED", data);
                }
            } else { // WR0 - WR3

                if (data & 0x03) { // WR0
                    wdi.dma.wr0.base = data;
                    wdi.dma.wr0.dest = data & 0x04;
                    wdi.dma.wr0.mode = data & 0x03;

                    if (wdi.dma.wr0.base & 0x78) {
                        wdi.dma.wr_state = WR0;
                    }
                } else { // WR1 - WR3

                    if ((data & 0x07) == 4) { //WR1
                        wdi.dma.wr1.base = data;
                        wdi.dma.wr1.a_device = (data & 0x08) >> 3;
                        wdi.dma.wr1.a_fixed = (data & 0x30) >> 4;
                        if (wdi.dma.wr1.base & 0x40) {
                            wdi.dma.wr_state = WR1;
                        }
                    } else if ((data & 0x07) == 0) { //WR2
                        wdi.dma.wr2.base = data;
                        wdi.dma.wr2.b_device = (data & 0x08) >> 3;
                        wdi.dma.wr2.b_fixed = (data & 0x30) >> 4;
                        if (wdi.dma.wr2.base & 0x40) {
                            wdi.dma.wr_state = WR2;
                        }
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
                LOGD(TAG, "WR0 [%02x] A start: %04x len: %02x", wdi.dma.wr0.base, wdi.dma.wr0.a_start, wdi.dma.wr0.len);
            }
            break;
        case WR1:
            wdi.dma.wr1.a_timing = data;
            wdi.dma.wr_state = WR_BASE;
            LOGD(TAG, "WR1 [%02x] A timing: %02x", wdi.dma.wr1.base, wdi.dma.wr1.a_timing);
            break;
        case WR2:
            wdi.dma.wr2.b_timing = data;
            wdi.dma.wr_state = WR_BASE;
            LOGD(TAG, "WR2 [%02x] B timing: %02x", wdi.dma.wr2.base, wdi.dma.wr2.b_timing);
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
                LOGD(TAG, "WR4 [%02x] B start: %04x int: %02x", wdi.dma.wr4.base, wdi.dma.wr4.b_start, wdi.dma.wr4.intr_control);
            }
            break;
        default:
            LOGE(TAG, "WR STATE [%d] NOT IMPLEMENTED", wdi.dma.wr_state);
            break;
    }
}
#define LOG_LOCAL_LEVEL LOG_INFO

void e9_out(BYTE data)
{
	LOGW(TAG, "E9 OUT: %02x", data);
}
void ea_out(BYTE data)
{
	LOGW(TAG, "EA OUT: %02x", data);
}
void eb_out(BYTE data)
{
	LOGW(TAG, "EB OUT: %02x", data);
}
void ec_out(BYTE data)
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
            wdi.ctc.state0 = 0;
            wdi.ctc.time0 = data;
            wdi.ctc.now0 = data;
            gettimeofday(&t1, NULL);
            break;
        default:
        LOGE(TAG, "CTC #0 - unknown state %d", wdi.ctc.state0);
    }
}
void ed_out(BYTE data)
{
    LOGD(TAG, "OUT: CTC #1 - %02x", data);

    switch (wdi.ctc.state1) 
    {
        case 0:
            wdi.ctc.mode1 = data;

            if (data & 0x80 ) LOGE(TAG, "CTC #1 - INT set");
            if (!(data & 0x40)) LOGI(TAG, "CTC #1 - TIMER set");
            if (data & 0x04 ) wdi.ctc.state1 = 1;
            if (data & 0x02 ) wdi.ctc.now1 = 0;
            if (!(data & 0x01)) LOGE(TAG, "CTC #1 - not CONTROL");
            break;
        case 1:
            wdi.ctc.state1 = 0;
            wdi.ctc.time1 = data;
            wdi.ctc.now1 = data;
            // gettimeofday(&t1, NULL);
            break;
        default:
        LOGE(TAG, "CTC #1 - unknown state %d", wdi.ctc.state0);
    }
}
void ee_out(BYTE data)
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
            // gettimeofday(&t1, NULL);
            break;
        default:
        LOGE(TAG, "CTC #2 - unknown state %d", wdi.ctc.state2);
    }
}
void ef_out(BYTE data)
{
	LOGW(TAG, "EF OUT: %02x", data);
}

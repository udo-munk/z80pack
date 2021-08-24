/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2021 by Udo Munk
 *
 * History:
 * 28-SEP-87 Development on TARGON/35 with AT&T Unix System V.3
 * 11-JAN-89 Release 1.1
 * 08-FEB-89 Release 1.2
 * 13-MAR-89 Release 1.3
 * 09-FEB-90 Release 1.4  Ported to TARGON/31 M10/30
 * 20-DEC-90 Release 1.5  Ported to COHERENT 3.0
 * 10-JUN-92 Release 1.6  long casting problem solved with COHERENT 3.2
 *			  and some optimisation
 * 25-JUN-92 Release 1.7  comments in english and ported to COHERENT 4.0
 * 02-OCT-06 Release 1.8  modified to compile on modern POSIX OS's
 * 18-NOV-06 Release 1.9  modified to work with CP/M sources
 * 08-DEC-06 Release 1.10 modified MMU for working with CP/NET
 * 17-DEC-06 Release 1.11 TCP/IP sockets for CP/NET
 * 25-DEC-06 Release 1.12 CPU speed option
 * 19-FEB-07 Release 1.13 various improvements
 * 06-OCT-07 Release 1.14 bug fixes and improvements
 * 06-AUG-08 Release 1.15 many improvements and Windows support via Cygwin
 * 25-AUG-08 Release 1.16 console status I/O loop detection and line discipline
 * 20-OCT-08 Release 1.17 frontpanel integrated and Altair/IMSAI emulations
 * 24-JAN-14 Release 1.18 bug fixes and improvements
 * 02-MAR-14 Release 1.19 source cleanup and improvements
 * 14-MAR-14 Release 1.20 added Tarbell SD FDC and printer port to Altair
 * 29-MAR-14 Release 1.21 many improvements
 * 29-MAY-14 Release 1.22 improved networking and bugfixes
 * 04-JUN-14 Release 1.23 added 8080 emulation
 * 06-SEP-14 Release 1.24 bugfixes and improvements
 * 18-FEB-15 Release 1.25 bugfixes, improvements, added Cromemco Z-1
 * 18-APR-15 Release 1.26 bugfixes and improvements
 * 18-JAN-16 Release 1.27 bugfixes and improvements
 * 05-MAY-16 Release 1.28 improved usability
 * 20-NOV-16 Release 1.29 bugfixes and improvements
 * 15-DEC-16 Release 1.30 improved memory management, machine cycle correct CPUs
 * 28-DEC-16 Release 1.31 improved memory management, reimplemented MMUs
 * 12-JAN-17 Release 1.32 improved configurations, front panel, added IMSAI VIO
 * 07-FEB-17 Release 1.33 bugfixes, improvements, better front panels
 * 16-MAR-17 Release 1.34 improvements, added ProcTec VDM-1
 * 03-AUG-17 Release 1.35 added UNIX sockets, bugfixes, improvements
 * 21-DEC-17 Release 1.36 bugfixes and improvements
 * 06-JAN-21 Release 1.37 bugfixes and improvements
 */

#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "../../frontpanel/frontpanel.h"
#endif
#include "memory.h"

#ifdef WANT_GUI
void check_gui_break(void);
#endif

static int op_nop(void), op_hlt(void), op_stc(void);
static int op_cmc(void), op_cma(void), op_daa(void), op_ei(void), op_di(void);
static int op_out(void), op_in(void);
static int op_mvian(void), op_mvibn(void), op_mvicn(void), op_mvidn(void);
static int op_mvien(void), op_mvihn(void), op_mviln(void);
static int op_ldaxb(void), op_ldaxd(void), op_ldann(void);
static int op_staxb(void), op_staxd(void), op_stann(void);
static int op_movma(void), op_movmb(void), op_movmc(void);
static int op_movmd(void), op_movme(void), op_movmh(void), op_movml(void);
static int op_mvimn(void);
static int op_movaa(void), op_movab(void), op_movac(void), op_movad(void);
static int op_movae(void), op_movah(void), op_moval(void), op_movam(void);
static int op_movba(void), op_movbb(void), op_movbc(void), op_movbd(void);
static int op_movbe(void), op_movbh(void), op_movbl(void), op_movbm(void);
static int op_movca(void), op_movcb(void), op_movcc(void), op_movcd(void);
static int op_movce(void), op_movch(void), op_movcl(void), op_movcm(void);
static int op_movda(void), op_movdb(void), op_movdc(void), op_movdd(void);
static int op_movde(void), op_movdh(void), op_movdl(void), op_movdm(void);
static int op_movea(void), op_moveb(void), op_movec(void), op_moved(void);
static int op_movee(void), op_moveh(void), op_movel(void), op_movem(void);
static int op_movha(void), op_movhb(void), op_movhc(void), op_movhd(void);
static int op_movhe(void), op_movhh(void), op_movhl(void), op_movhm(void);
static int op_movla(void), op_movlb(void), op_movlc(void), op_movld(void);
static int op_movle(void), op_movlh(void), op_movll(void), op_movlm(void);
static int op_lxibnn(void), op_lxidnn(void), op_lxihnn(void);
static int op_lxispnn(void), op_sphl(void), op_lhldnn(void), op_shldnn(void);
static int op_inxb(void), op_inxd(void), op_inxh(void), op_inxsp(void);
static int op_dcxb(void), op_dcxd(void), op_dcxh(void), op_dcxsp(void);
static int op_dadb(void), op_dadd(void), op_dadh(void), op_dadsp(void);
static int op_anaa(void), op_anab(void), op_anac(void), op_anad(void);
static int op_anae(void), op_anah(void), op_anal(void), op_anam(void);
static int op_anin(void);
static int op_oraa(void), op_orab(void), op_orac(void), op_orad(void);
static int op_orae(void), op_orah(void), op_oral(void), op_oram(void);
static int op_orin(void);
static int op_xraa(void), op_xrab(void), op_xrac(void), op_xrad(void);
static int op_xrae(void), op_xrah(void), op_xral(void), op_xram(void);
static int op_xrin(void);
static int op_adda(void), op_addb(void), op_addc(void), op_addd(void);
static int op_adde(void), op_addh(void), op_addl(void), op_addm(void);
static int op_adin(void);
static int op_adca(void), op_adcb(void), op_adcc(void), op_adcd(void);
static int op_adce(void), op_adch(void), op_adcl(void), op_adcm(void);
static int op_acin(void);
static int op_suba(void), op_subb(void), op_subc(void), op_subd(void);
static int op_sube(void), op_subh(void), op_subl(void), op_subm(void);
static int op_suin(void);
static int op_sbba(void), op_sbbb(void), op_sbbc(void), op_sbbd(void);
static int op_sbbe(void), op_sbbh(void), op_sbbl(void), op_sbbm(void);
static int op_sbin(void);
static int op_cmpa(void), op_cmpb(void), op_cmpc(void), op_cmpd(void);
static int op_cmpe(void), op_cmph(void), op_cmpl(void), op_cmpm(void);
static int op_cpin(void);
static int op_inra(void), op_inrb(void), op_inrc(void), op_inrd(void);
static int op_inre(void), op_inrh(void), op_inrl(void), op_inrm(void);
static int op_dcra(void), op_dcrb(void), op_dcrc(void), op_dcrd(void);
static int op_dcre(void), op_dcrh(void), op_dcrl(void), op_dcrm(void);
static int op_rlc(void), op_rrc(void), op_ral(void), op_rar(void);
static int op_xchg(void), op_xthl(void);
static int op_pushpsw(void), op_pushb(void), op_pushd(void), op_pushh(void);
static int op_poppsw(void), op_popb(void), op_popd(void), op_poph(void);
static int op_jmp(void), op_pchl(void), op_call(void), op_ret(void);
static int op_jz(void), op_jnz(void), op_jc(void), op_jnc(void);
static int op_jpe(void), op_jpo(void), op_jm(void), op_jp(void);
static int op_cz(void), op_cnz(void), op_cc(void), op_cnc(void);
static int op_cpe(void), op_cpo(void), op_cm(void), op_cp(void);
static int op_rz(void), op_rnz(void), op_rc(void), op_rnc(void);
static int op_rpe(void), op_rpo(void), op_rm(void), op_rp(void);
static int op_rst0(void), op_rst1(void), op_rst2(void), op_rst3(void);
static int op_rst4(void), op_rst5(void), op_rst6(void), op_rst7(void);
static int op_undoc_nop(void), op_undoc_jmp(void);
static int op_undoc_ret(void), op_undoc_call(void);

/*
 * Function to update address bus LED's during execution of
 * instructions using the 16bit incrementer/decrementer
 * logic, as dicovered by Mike Douglas.
 */
#ifdef FRONTPANEL
static inline void adr_leds(WORD data)
{
	fp_led_address = data;
	fp_clock++;
	fp_sampleData();
}
#endif

/*
 *	This function builds the 8080 central processing unit.
 *	The opcode where PC points to is fetched from the memory
 *	and PC incremented by one. The opcode is used as an
 *	index to an array with function pointers, to execute a
 *	function which emulates this 8080 opcode.
 *
 */
void cpu_8080(void)
{
	extern int time_diff(struct timeval *, struct timeval *);

	static int (*op_sim[256]) (void) = {
		op_nop,				/* 0x00 */
		op_lxibnn,			/* 0x01 */
		op_staxb,			/* 0x02 */
		op_inxb,			/* 0x03 */
		op_inrb,			/* 0x04 */
		op_dcrb,			/* 0x05 */
		op_mvibn,			/* 0x06 */
		op_rlc,				/* 0x07 */
		op_undoc_nop,			/* 0x08 */
		op_dadb,			/* 0x09 */
		op_ldaxb,			/* 0x0a */
		op_dcxb,			/* 0x0b */
		op_inrc,			/* 0x0c */
		op_dcrc,			/* 0x0d */
		op_mvicn,			/* 0x0e */
		op_rrc,				/* 0x0f */
		op_undoc_nop,			/* 0x10 */
		op_lxidnn,			/* 0x11 */
		op_staxd,			/* 0x12 */
		op_inxd,			/* 0x13 */
		op_inrd,			/* 0x14 */
		op_dcrd,			/* 0x15 */
		op_mvidn,			/* 0x16 */
		op_ral,				/* 0x17 */
		op_undoc_nop,			/* 0x18 */
		op_dadd,			/* 0x19 */
		op_ldaxd,			/* 0x1a */
		op_dcxd,			/* 0x1b */
		op_inre,			/* 0x1c */
		op_dcre,			/* 0x1d */
		op_mvien,			/* 0x1e */
		op_rar,				/* 0x1f */
		op_undoc_nop,			/* 0x20 */
		op_lxihnn,			/* 0x21 */
		op_shldnn,			/* 0x22 */
		op_inxh,			/* 0x23 */
		op_inrh,			/* 0x24 */
		op_dcrh,			/* 0x25 */
		op_mvihn,			/* 0x26 */
		op_daa,				/* 0x27 */
		op_undoc_nop,			/* 0x28 */
		op_dadh,			/* 0x29 */
		op_lhldnn,			/* 0x2a */
		op_dcxh,			/* 0x2b */
		op_inrl,			/* 0x2c */
		op_dcrl,			/* 0x2d */
		op_mviln,			/* 0x2e */
		op_cma,				/* 0x2f */
		op_undoc_nop,			/* 0x30 */
		op_lxispnn,			/* 0x31 */
		op_stann,			/* 0x32 */
		op_inxsp,			/* 0x33 */
		op_inrm,			/* 0x34 */
		op_dcrm,			/* 0x35 */
		op_mvimn,			/* 0x36 */
		op_stc,				/* 0x37 */
		op_undoc_nop,			/* 0x38 */
		op_dadsp,			/* 0x39 */
		op_ldann,			/* 0x3a */
		op_dcxsp,			/* 0x3b */
		op_inra,			/* 0x3c */
		op_dcra,			/* 0x3d */
		op_mvian,			/* 0x3e */
		op_cmc,				/* 0x3f */
		op_movbb,			/* 0x40 */
		op_movbc,			/* 0x41 */
		op_movbd,			/* 0x42 */
		op_movbe,			/* 0x43 */
		op_movbh,			/* 0x44 */
		op_movbl,			/* 0x45 */
		op_movbm,			/* 0x46 */
		op_movba,			/* 0x47 */
		op_movcb,			/* 0x48 */
		op_movcc,			/* 0x49 */
		op_movcd,			/* 0x4a */
		op_movce,			/* 0x4b */
		op_movch,			/* 0x4c */
		op_movcl,			/* 0x4d */
		op_movcm,			/* 0x4e */
		op_movca,			/* 0x4f */
		op_movdb,			/* 0x50 */
		op_movdc,			/* 0x51 */
		op_movdd,			/* 0x52 */
		op_movde,			/* 0x53 */
		op_movdh,			/* 0x54 */
		op_movdl,			/* 0x55 */
		op_movdm,			/* 0x56 */
		op_movda,			/* 0x57 */
		op_moveb,			/* 0x58 */
		op_movec,			/* 0x59 */
		op_moved,			/* 0x5a */
		op_movee,			/* 0x5b */
		op_moveh,			/* 0x5c */
		op_movel,			/* 0x5d */
		op_movem,			/* 0x5e */
		op_movea,			/* 0x5f */
		op_movhb,			/* 0x60 */
		op_movhc,			/* 0x61 */
		op_movhd,			/* 0x62 */
		op_movhe,			/* 0x63 */
		op_movhh,			/* 0x64 */
		op_movhl,			/* 0x65 */
		op_movhm,			/* 0x66 */
		op_movha,			/* 0x67 */
		op_movlb,			/* 0x68 */
		op_movlc,			/* 0x69 */
		op_movld,			/* 0x6a */
		op_movle,			/* 0x6b */
		op_movlh,			/* 0x6c */
		op_movll,			/* 0x6d */
		op_movlm,			/* 0x6e */
		op_movla,			/* 0x6f */
		op_movmb,			/* 0x70 */
		op_movmc,			/* 0x71 */
		op_movmd,			/* 0x72 */
		op_movme,			/* 0x73 */
		op_movmh,			/* 0x74 */
		op_movml,			/* 0x75 */
		op_hlt,				/* 0x76 */
		op_movma,			/* 0x77 */
		op_movab,			/* 0x78 */
		op_movac,			/* 0x79 */
		op_movad,			/* 0x7a */
		op_movae,			/* 0x7b */
		op_movah,			/* 0x7c */
		op_moval,			/* 0x7d */
		op_movam,			/* 0x7e */
		op_movaa,			/* 0x7f */
		op_addb,			/* 0x80 */
		op_addc,			/* 0x81 */
		op_addd,			/* 0x82 */
		op_adde,			/* 0x83 */
		op_addh,			/* 0x84 */
		op_addl,			/* 0x85 */
		op_addm,			/* 0x86 */
		op_adda,			/* 0x87 */
		op_adcb,			/* 0x88 */
		op_adcc,			/* 0x89 */
		op_adcd,			/* 0x8a */
		op_adce,			/* 0x8b */
		op_adch,			/* 0x8c */
		op_adcl,			/* 0x8d */
		op_adcm,			/* 0x8e */
		op_adca,			/* 0x8f */
		op_subb,			/* 0x90 */
		op_subc,			/* 0x91 */
		op_subd,			/* 0x92 */
		op_sube,			/* 0x93 */
		op_subh,			/* 0x94 */
		op_subl,			/* 0x95 */
		op_subm,			/* 0x96 */
		op_suba,			/* 0x97 */
		op_sbbb,			/* 0x98 */
		op_sbbc,			/* 0x99 */
		op_sbbd,			/* 0x9a */
		op_sbbe,			/* 0x9b */
		op_sbbh,			/* 0x9c */
		op_sbbl,			/* 0x9d */
		op_sbbm,			/* 0x9e */
		op_sbba,			/* 0x9f */
		op_anab,			/* 0xa0 */
		op_anac,			/* 0xa1 */
		op_anad,			/* 0xa2 */
		op_anae,			/* 0xa3 */
		op_anah,			/* 0xa4 */
		op_anal,			/* 0xa5 */
		op_anam,			/* 0xa6 */
		op_anaa,			/* 0xa7 */
		op_xrab,			/* 0xa8 */
		op_xrac,			/* 0xa9 */
		op_xrad,			/* 0xaa */
		op_xrae,			/* 0xab */
		op_xrah,			/* 0xac */
		op_xral,			/* 0xad */
		op_xram,			/* 0xae */
		op_xraa,			/* 0xaf */
		op_orab,			/* 0xb0 */
		op_orac,			/* 0xb1 */
		op_orad,			/* 0xb2 */
		op_orae,			/* 0xb3 */
		op_orah,			/* 0xb4 */
		op_oral,			/* 0xb5 */
		op_oram,			/* 0xb6 */
		op_oraa,			/* 0xb7 */
		op_cmpb,			/* 0xb8 */
		op_cmpc,			/* 0xb9 */
		op_cmpd,			/* 0xba */
		op_cmpe,			/* 0xbb */
		op_cmph,			/* 0xbc */
		op_cmpl,			/* 0xbd */
		op_cmpm,			/* 0xbe */
		op_cmpa,			/* 0xbf */
		op_rnz,				/* 0xc0 */
		op_popb,			/* 0xc1 */
		op_jnz,				/* 0xc2 */
		op_jmp,				/* 0xc3 */
		op_cnz,				/* 0xc4 */
		op_pushb,			/* 0xc5 */
		op_adin,			/* 0xc6 */
		op_rst0,			/* 0xc7 */
		op_rz,				/* 0xc8 */
		op_ret,				/* 0xc9 */
		op_jz,				/* 0xca */
		op_undoc_jmp,			/* 0xcb */
		op_cz,				/* 0xcc */
		op_call,			/* 0xcd */
		op_acin,			/* 0xce */
		op_rst1,			/* 0xcf */
		op_rnc,				/* 0xd0 */
		op_popd,			/* 0xd1 */
		op_jnc,				/* 0xd2 */
		op_out,				/* 0xd3 */
		op_cnc,				/* 0xd4 */
		op_pushd,			/* 0xd5 */
		op_suin,			/* 0xd6 */
		op_rst2,			/* 0xd7 */
		op_rc,				/* 0xd8 */
		op_undoc_ret,			/* 0xd9 */
		op_jc,				/* 0xda */
		op_in,				/* 0xdb */
		op_cc,				/* 0xdc */
		op_undoc_call,			/* 0xdd */
		op_sbin,			/* 0xde */
		op_rst3,			/* 0xdf */
		op_rpo,				/* 0xe0 */
		op_poph,			/* 0xe1 */
		op_jpo,				/* 0xe2 */
		op_xthl,			/* 0xe3 */
		op_cpo,				/* 0xe4 */
		op_pushh,			/* 0xe5 */
		op_anin,			/* 0xe6 */
		op_rst4,			/* 0xe7 */
		op_rpe,				/* 0xe8 */
		op_pchl,			/* 0xe9 */
		op_jpe,				/* 0xea */
		op_xchg,			/* 0xeb */
		op_cpe,				/* 0xec */
		op_undoc_call,			/* 0xed */
		op_xrin,			/* 0xee */
		op_rst5,			/* 0xef */
		op_rp,				/* 0xf0 */
		op_poppsw,			/* 0xf1 */
		op_jp,				/* 0xf2 */
		op_di,				/* 0xf3 */
		op_cp,				/* 0xf4 */
		op_pushpsw,			/* 0xf5 */
		op_orin,			/* 0xf6 */
		op_rst6,			/* 0xf7 */
		op_rm,				/* 0xf8 */
		op_sphl,			/* 0xf9 */
		op_jm,				/* 0xfa */
		op_ei,				/* 0xfb */
		op_cm,				/* 0xfc */
		op_undoc_call,			/* 0xfd */
		op_cpin,			/* 0xfe */
		op_rst7				/* 0xff */
	};

	register int t = 0;
	register int states;
	struct timeval t1, t2;
	int tdiff;

	gettimeofday(&t1, NULL);

	do {

#ifdef HISIZE
		/* write history */
		his[h_next].h_adr = PC;
		his[h_next].h_af = (A << 8) + F;
		his[h_next].h_bc = BC;
		his[h_next].h_de = DE;
		his[h_next].h_hl = HL;
		his[h_next].h_sp = SP;
		h_next++;
		if (h_next == HISIZE) {
			h_flag = 1;
			h_next = 0;
		}
#endif

#ifdef WANT_TIM
		/* check for start address of runtime measurement */
		if (PC == t_start && !t_flag) {
			t_flag = 1;	/* switch measurement on */
			t_states = 0L;	/* initialise counted T-states */
		}
#endif

		/* CPU interrupt handling */
		if (int_int) {
			if (IFF != 3)
				goto leave;
			if (int_protection)	/* protect first instr */
				goto leave;	/* after EI */

			IFF = 0;

#ifdef BUS_8080
			if (!(cpu_bus & CPU_HLTA)) {
				cpu_bus = CPU_WO | CPU_M1 | CPU_INTA;
#endif
#ifdef FRONTPANEL
				fp_clock += 1000;
				fp_led_data = (int_data != -1) ?
						(BYTE) int_data : 0xff;
				fp_sampleData();
				wait_int_step();
				if (cpu_state & RESET)
					goto leave;
#endif
#ifdef BUS_8080
			}
			cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
			fp_clock++;
			fp_sampleLightGroup(0, 0);
#endif

			memwrt(--SP, PC_H);
			memwrt(--SP, PC);

#ifdef FRONTPANEL
			if (cpu_state & RESET)
				goto leave;
#endif

			switch (int_data) {
			case 0xc7: /* RST 00H */
				PC = 0;
				break;
			case 0xcf: /* RST 08H */
				PC = 8;
				break;
			case 0xd7: /* RST 10H */
				PC = 0x10;
				break;
			case 0xdf: /* RST 18H */
				PC = 0x18;
				break;
			case 0xe7: /* RST 20H */
				PC = 0x20;
				break;
			case 0xef: /* RST 28H */
				PC = 0x28;
				break;
			case 0xf7: /* RST 30H */
				PC = 0x30;
				break;
			case 0xff: /* RST 38H */
				PC = 0x38;
				break;
			case -1: /* no data = 0xff on S100 bus */
				PC = 0x38;
				break;
			default: /* unsupported bus data */
				cpu_error = INTERROR;
				cpu_state = STOPPED;
				continue;
			}
			t += 11;
			int_int = 0;
			int_data = -1;
#ifdef FRONTPANEL
			m1_step = 1;
#endif
		}
leave:

#ifdef BUS_8080
		/* M1 opcode fetch */
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
#endif

		int_protection = 0;
		states = (*op_sim[memrdr(PC++)]) (); /* execute next opcode */
		t += states;

		if (f_flag) {			/* adjust CPU speed */
			if (t >= tmax && !cpu_needed) {
				gettimeofday(&t2, NULL);
				tdiff = time_diff(&t1, &t2);
#ifndef __CYGWIN__
				if ((tdiff > 0) && (tdiff < 10000))
					SLEEP_MS(10 - (tdiff / 1000));
#else
				/* timer resolution seems to be 20ms,
				   sleeps < 10 won't work at all */
				tdiff++; /* avoid compiler warning */
				SLEEP_MS(10);
#endif
				t = 0;
				gettimeofday(&t1, NULL);
			}
		}

		R++;			/* increment refresh register */

					/* do runtime measurement */
#ifdef WANT_TIM
		if (t_flag) {
			t_states += states; /* add T-states for this opcode */
			if (PC == t_end)	/* check for end address */
				t_flag = 0;	/* if reached, switch off */
		}
#endif

#ifdef WANT_GUI
                check_gui_break();
#endif

	} while (cpu_state == CONTIN_RUN);

#ifdef BUS_8080
	if (!(cpu_bus & CPU_INTA))
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
#endif
#ifdef FRONTPANEL
	fp_led_address = PC;
	fp_led_data = getmem(PC);
	fp_clock++;
	fp_sampleData();
#endif
}

static int op_nop(void)                 /* NOP */
{
	return(4);
}

static int op_undoc_nop(void)		/* Undocumented NOP */
{
	if (u_flag) {	/* trap with option -u */
		cpu_error = OPTRAP1;
		cpu_state = STOPPED;
		return(0);
	} else {	/* else NOP */
		return(4);
	}
}

static int op_hlt(void)			/* HLT */
{
#ifdef BUS_8080
	cpu_bus = CPU_WO | CPU_HLTA | CPU_MEMR;
#endif

#ifndef FRONTPANEL
	/* without a frontpanel DI + HALT stops the machine */
	if (IFF == 0)   {
		cpu_error = OPHALT;
		cpu_state = STOPPED;
	} else {
	/* else wait for INT or user interrupt */
		while ((int_int == 0) && (cpu_state == CONTIN_RUN)) {
			SLEEP_MS(1);
			R += 9999;
		}
	}
#ifdef BUS_8080
	if (int_int)
		cpu_bus = CPU_INTA | CPU_WO | CPU_HLTA | CPU_M1;
#endif

	busy_loop_cnt[0] = 0;

#else

	fp_led_address = 0xffff;
	fp_led_data = 0xff;

	/* INT disabled, wait for frontpanel reset or user interrupt */
	if (IFF == 0)   {
		while (!(cpu_state & RESET)) {
			fp_clock++;
			fp_sampleData();
			SLEEP_MS(1);
			R += 9999;
			if (cpu_error != NONE)
				break;
		}
	} else {
	/* else wait for INT, frontpanel reset or user interrupt */
		while ((int_int == 0) && !(cpu_state & RESET)) {
			fp_clock++;
			fp_sampleData();
			SLEEP_MS(1);
			R += 9999;
			if (cpu_error != NONE)
				break;
		}
		if (int_int) {
			cpu_bus = CPU_INTA | CPU_WO | CPU_HLTA | CPU_M1;
			fp_clock++;
			fp_sampleLightGroup(0, 0);
		}
	}
#endif

	return(7);
}

static int op_stc(void)			/* STC */
{
	F |= C_FLAG;
	return(4);
}

static int op_cmc(void)			/* CMC */
{
	if (F & C_FLAG)
		F &= ~C_FLAG;
	else
		F |= C_FLAG;
	return(4);
}

static int op_cma(void)			/* CMA */
{
	A = ~A;
	return(4);
}

static int op_daa(void)			/* DAA */
{
	register int tmp_a = A;

	if (((A & 0xf) > 9) ||	(F & H_FLAG)) {
		((A & 0xf) > 9) ? (F |= H_FLAG) : (F &= ~H_FLAG);
		tmp_a += 6;
	}
	if (((tmp_a & 0x1f0) > 0x90) || (F & C_FLAG)) {
		tmp_a += 0x60;
	}
        if (tmp_a & 0x100)
		(F |= C_FLAG);
        A = tmp_a & 0xff;
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	return(4);
}

static int op_ei(void)			/* EI */
{
	IFF = 3;
	int_protection = 1;		/* protect next instruction */
	return(4);
}

static int op_di(void)			/* DI */
{
	IFF = 0;
	return(4);
}

static int op_in(void)			/* IN n */
{
	BYTE io_in(BYTE, BYTE);
	BYTE addr;

	addr = memrdr(PC++);
	A = io_in(addr, addr);
	return(10);
}

static int op_out(void)			/* OUT n */
{
	BYTE io_out(BYTE, BYTE, BYTE);
	BYTE addr;

	addr = memrdr(PC++);
	io_out(addr, addr, A);
	return(10);
}

static int op_mvian(void)		/* MVI A,n */
{
	A = memrdr(PC++);
	return(7);
}

static int op_mvibn(void)		/* MVI B,n */
{
	B = memrdr(PC++);
	return(7);
}

static int op_mvicn(void)		/* MVI C,n */
{
	C = memrdr(PC++);
	return(7);
}

static int op_mvidn(void)		/* MVI D,n */
{
	D = memrdr(PC++);
	return(7);
}

static int op_mvien(void)		/* MVI E,n */
{
	E = memrdr(PC++);
	return(7);
}

static int op_mvihn(void)		/* MVI H,n */
{
	H = memrdr(PC++);
	return(7);
}

static int op_mviln(void)		/* MVI L,n */
{
	L = memrdr(PC++);
	return(7);
}

static int op_ldaxb(void)		/* LDAX B */
{
	A = memrdr(BC);
	return(7);
}

static int op_ldaxd(void)		/* LDAX D */
{
	A = memrdr(DE);
	return(7);
}

static int op_ldann(void)		/* LDA nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	A = memrdr(i);
	return(13);
}

static int op_staxb(void)		/* STAX B */
{
	memwrt(BC, A);
	return(7);
}

static int op_staxd(void)		/* STAX D */
{
	memwrt(DE, A);
	return(7);
}

static int op_stann(void)		/* STA nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, A);
	return(13);
}

static int op_movma(void)		/* MOV M,A */
{
	memwrt(HL, A);
	return(7);
}

static int op_movmb(void)		/* MOV M,B */
{
	memwrt(HL, B);
	return(7);
}

static int op_movmc(void)		/* MOV M,C */
{
	memwrt(HL, C);
	return(7);
}

static int op_movmd(void)		/* MOV M,D */
{
	memwrt(HL, D);
	return(7);
}

static int op_movme(void)		/* MOV M,E */
{
	memwrt(HL, E);
	return(7);
}

static int op_movmh(void)		/* MOV M,H */
{
	memwrt(HL, H);
	return(7);
}

static int op_movml(void)		/* MOV M,L */
{
	memwrt(HL, L);
	return(7);
}

static int op_mvimn(void)		/* MVI M,n */
{
	memwrt(HL, memrdr(PC++));
	return(10);
}

static int op_movaa(void)		/* MOV A,A */
{
	return(5);
}

static int op_movab(void)		/* MOV A,B */
{
	A = B;
	return(5);
}

static int op_movac(void)		/* MOV A,C */
{
	A = C;
	return(5);
}

static int op_movad(void)		/* MOV A,D */
{
	A = D;
	return(5);
}

static int op_movae(void)		/* MOV A,E */
{
	A = E;
	return(5);
}

static int op_movah(void)		/* MOV A,H */
{
	A = H;
	return(5);
}

static int op_moval(void)		/* MOV A,L */
{
	A = L;
	return(5);
}

static int op_movam(void)		/* MOV A,M */
{
	A = memrdr(HL);
	return(7);
}

static int op_movba(void)		/* MOV B,A */
{
	B = A;
	return(5);
}

static int op_movbb(void)		/* MOV B,B */
{
	return(5);
}

static int op_movbc(void)		/* MOV B,C */
{
	B = C;
	return(5);
}

static int op_movbd(void)		/* MOV B,D */
{
	B = D;
	return(5);
}

static int op_movbe(void)		/* MOV B,E */
{
	B = E;
	return(5);
}

static int op_movbh(void)		/* MOV B,H */
{
	B = H;
	return(5);
}

static int op_movbl(void)		/* MOV B,L */
{
	B = L;
	return(5);
}

static int op_movbm(void)		/* MOV B,M */
{
	B = memrdr(HL);
	return(7);
}

static int op_movca(void)		/* MOV C,A */
{
	C = A;
	return(5);
}

static int op_movcb(void)		/* MOV C,B */
{
	C = B;
	return(5);
}

static int op_movcc(void)		/* MOV C,C */
{
	return(5);
}

static int op_movcd(void)		/* MOV C,D */
{
	C = D;
	return(5);
}

static int op_movce(void)		/* MOV C,E */
{
	C = E;
	return(5);
}

static int op_movch(void)		/* MOV C,H */
{
	C = H;
	return(5);
}

static int op_movcl(void)		/* MOV C,L */
{
	C = L;
	return(5);
}

static int op_movcm(void)		/* MOV C,M */
{
	C = memrdr(HL);
	return(7);
}

static int op_movda(void)		/* MOV D,A */
{
	D = A;
	return(5);
}

static int op_movdb(void)		/* MOV D,B */
{
	D = B;
	return(5);
}

static int op_movdc(void)		/* MOV D,C */
{
	D = C;
	return(5);
}

static int op_movdd(void)		/* MOV D,D */
{
	return(5);
}

static int op_movde(void)		/* MOV D,E */
{
	D = E;
	return(5);
}

static int op_movdh(void)		/* MOV D,H */
{
	D = H;
	return(5);
}

static int op_movdl(void)		/* MOV D,L */
{
	D = L;
	return(5);
}

static int op_movdm(void)		/* MOV D,M */
{
	D = memrdr(HL);
	return(7);
}

static int op_movea(void)		/* MOV E,A */
{
	E = A;
	return(5);
}

static int op_moveb(void)		/* MOV E,B */
{
	E = B;
	return(5);
}

static int op_movec(void)		/* MOV E,C */
{
	E = C;
	return(5);
}

static int op_moved(void)		/* MOV E,D */
{
	E = D;
	return(5);
}

static int op_movee(void)		/* MOV E,E */
{
	return(5);
}

static int op_moveh(void)		/* MOV E,H */
{
	E = H;
	return(5);
}

static int op_movel(void)		/* MOV E,L */
{
	E = L;
	return(5);
}

static int op_movem(void)		/* MOV E,M */
{
	E = memrdr(HL);
	return(7);
}

static int op_movha(void)		/* MOV H,A */
{
	H = A;
	return(5);
}

static int op_movhb(void)		/* MOV H,B */
{
	H = B;
	return(5);
}

static int op_movhc(void)		/* MOV H,C */
{
	H = C;
	return(5);
}

static int op_movhd(void)		/* MOV H,D */
{
	H = D;
	return(5);
}

static int op_movhe(void)		/* MOV H,E */
{
	H = E;
	return(5);
}

static int op_movhh(void)		/* MOV H,H */
{
	return(5);
}

static int op_movhl(void)		/* MOV H,L */
{
	H = L;
	return(5);
}

static int op_movhm(void)		/* MOV H,M */
{
	H = memrdr(HL);
	return(7);
}

static int op_movla(void)		/* MOV L,A */
{
	L = A;
	return(5);
}

static int op_movlb(void)		/* MOV L,B */
{
	L = B;
	return(5);
}

static int op_movlc(void)		/* MOV L,C */
{
	L = C;
	return(5);
}

static int op_movld(void)		/* MOV L,D */
{
	L = D;
	return(5);
}

static int op_movle(void)		/* MOV L,E */
{
	L = E;
	return(5);
}

static int op_movlh(void)		/* MOV L,H */
{
	L = H;
	return(5);
}

static int op_movll(void)		/* MOV L,L */
{
	return(5);
}

static int op_movlm(void)		/* MOV L,M */
{
	L = memrdr(HL);
	return(7);
}

static int op_lxibnn(void)		/* LXI B,nn */
{
	C = memrdr(PC++);
	B = memrdr(PC++);
	return(10);
}

static int op_lxidnn(void)		/* LXI D,nn */
{
	E = memrdr(PC++);
	D = memrdr(PC++);
	return(10);
}

static int op_lxihnn(void)		/* LXI H,nn */
{
	L = memrdr(PC++);
	H = memrdr(PC++);
	return(10);
}

static int op_lxispnn(void)		/* LXI SP,nn */
{
	SP = memrdr(PC++);
	SP += memrdr(PC++) << 8;
	return(10);
}

static int op_sphl(void)		/* SPHL */
{
#ifdef FRONTPANEL
	adr_leds(H << 8 | L);
#endif
	SP = HL;
	return(5);
}

static int op_lhldnn(void)		/* LHLD nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	L = memrdr(i);
	H = memrdr(i + 1);
	return(16);
}

static int op_shldnn(void)		/* SHLD nn */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
	memwrt(i, L);
	memwrt(i + 1, H);
	return(16);
}

static int op_inxb(void)		/* INX B */
{
#ifdef FRONTPANEL
	adr_leds(B << 8 | C);
#endif
	C++;
	if (!C)
		B++;
	return(5);
}

static int op_inxd(void)		/* INX D */
{
#ifdef FRONTPANEL
	adr_leds(D << 8 | E);
#endif
	E++;
	if (!E)
		D++;
	return(5);
}

static int op_inxh(void)		/* INX H */
{
#ifdef FRONTPANEL
	adr_leds(H << 8 | L);
#endif
	L++;
	if (!L)
		H++;
	return(5);
}

static int op_inxsp(void)		/* INX SP */
{
#ifdef FRONTPANEL
	adr_leds(SP);
#endif
	SP++;
	return(5);
}

static int op_dcxb(void)		/* DCX B */
{
#ifdef FRONTPANEL
	adr_leds(B << 8 | C);
#endif
	C--;
	if (C == 0xff)
		B--;
	return(5);
}

static int op_dcxd(void)		/* DCX D */
{
#ifdef FRONTPANEL
	adr_leds(D << 8 | E);
#endif
	E--;
	if (E == 0xff)
		D--;
	return(5);
}

static int op_dcxh(void)		/* DCX H */
{
#ifdef FRONTPANEL
	adr_leds(H << 8 | L);
#endif
	L--;
	if (L == 0xff)
		H--;
	return(5);
}

static int op_dcxsp(void)		/* DCX SP */
{
#ifdef FRONTPANEL
	adr_leds(SP);
#endif
	SP--;
	return(5);
}

static int op_dadb(void)		/* DAD B */
{
	register int carry;

	carry = (L + C > 255) ? 1 : 0;
	L += C;
	(H + B + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += B + carry;
	return(10);
}

static int op_dadd(void)		/* DAD D */
{
	register int carry;

	carry = (L + E > 255) ? 1 : 0;
	L += E;
	(H + D + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += D + carry;
	return(10);
}

static int op_dadh(void)		/* DAD H */
{
	register int carry;

	carry = (L << 1 > 255) ? 1 : 0;
	L <<= 1;
	(H + H + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += H + carry;
	return(10);
}

static int op_dadsp(void)		/* DAD SP */
{
	register int carry;

	BYTE spl = SP & 0xff;
	BYTE sph = SP >> 8;
	
	carry = (L + spl > 255) ? 1 : 0;
	L += spl;
	(H + sph + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	H += sph + carry;
	return(10);
}

static int op_anaa(void)		/* ANA A */
{
	(A & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(4);
}

static int op_anab(void)		/* ANA B */
{
	((A | B) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= B;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(4);
}

static int op_anac(void)		/* ANA C */
{
	((A | C) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= C;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(4);
}

static int op_anad(void)		/* ANA D */
{
	((A | D) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= D;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(4);
}

static int op_anae(void)		/* ANA E */
{
	((A | E) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= E;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(4);
}

static int op_anah(void)		/* ANA H */
{
	((A | H) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= H;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(4);
}

static int op_anal(void)		/* ANA L */
{
	((A | L) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= L;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(4);
}

static int op_anam(void)		/* ANA M */
{
	register BYTE P;

	P = memrdr(HL);
	((A | P) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= P;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(7);
}

static int op_anin(void)		/* ANI n */
{
	register BYTE P;

	P = memrdr(PC++);
	((A | P) & 8) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	A &= P;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~C_FLAG;
	return(7);
}

static int op_oraa(void)		/* ORA A */
{
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(4);
}

static int op_orab(void)		/* ORA B */
{
	A |= B;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(4);
}

static int op_orac(void)		/* ORA C */
{
	A |= C;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(4);
}

static int op_orad(void)		/* ORA D */
{
	A |= D;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(4);
}

static int op_orae(void)		/* ORA E */
{
	A |= E;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(4);
}

static int op_orah(void)		/* ORA H */
{
	A |= H;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(4);
}

static int op_oral(void)		/* ORA L */
{
	A |= L;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(4);
}

static int op_oram(void)		/* ORA M */
{
	A |= memrdr(HL);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(7);
}

static int op_orin(void)		/* ORI n */
{
	A |= memrdr(PC++);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(C_FLAG | H_FLAG);
	return(7);
}

static int op_xraa(void)		/* XRA A */
{
	A = 0;
	F &= ~(S_FLAG |	H_FLAG | C_FLAG);
	F |= Z_FLAG | P_FLAG;
	return(4);
}

static int op_xrab(void)		/* XRA B */
{
	A ^= B;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(4);
}

static int op_xrac(void)		/* XRA C */
{
	A ^= C;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(4);
}

static int op_xrad(void)		/* XRA D */
{
	A ^= D;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(4);
}

static int op_xrae(void)		/* XRA E */
{
	A ^= E;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(4);
}

static int op_xrah(void)		/* XRA H */
{
	A ^= H;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(4);
}

static int op_xral(void)		/* XRA L */
{
	A ^= L;
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(4);
}

static int op_xram(void)		/* XRA M */
{
	A ^= memrdr(HL);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(7);
}

static int op_xrin(void)		/* XRI n */
{
	A ^= memrdr(PC++);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	F &= ~(H_FLAG | C_FLAG);
	return(7);
}

static int op_adda(void)		/* ADD A */
{
	((A & 0xf) + (A & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	((A << 1) > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A << 1;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_addb(void)		/* ADD B */
{
	((A & 0xf) + (B & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + B > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + B;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_addc(void)		/* ADD C */
{
	((A & 0xf) + (C & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + C > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A =  A + C;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_addd(void)		/* ADD D */
{
	((A & 0xf) + (D & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + D > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + D;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adde(void)		/* ADD E */
{
	((A & 0xf) + (E & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + E > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + E;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_addh(void)		/* ADD H */
{
	((A & 0xf) + (H & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + H > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + H;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_addl(void)		/* ADD L */
{
	((A & 0xf) + (L & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + L > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + L;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_addm(void)		/* ADD M */
{
	register BYTE P;

	P = memrdr(HL);
	((A & 0xf) + (P & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_adin(void)		/* ADI n */
{
	register BYTE P;

	P = memrdr(PC++);
	((A & 0xf) + (P & 0xf) > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_adca(void)		/* ADC A */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (A & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	((A << 1) + carry > 255) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = (A << 1) + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adcb(void)		/* ADC B */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (B & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + B + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + B + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adcc(void)		/* ADC C */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (C & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + C + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + C + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adcd(void)		/* ADC D */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (D & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + D + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + D + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adce(void)		/* ADC E */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (E & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + E + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + E + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adch(void)		/* ADC H */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (H & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + H + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + H + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adcl(void)		/* ADC L */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (L & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + L + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + L + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_adcm(void)		/* ADC M */
{
	register int carry;
	register BYTE P;

	P = memrdr(HL);
	carry = (F & C_FLAG) ? 1 : 0;
	((A & 0xf) + (P & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_acin(void)		/* ACI n */
{
	register int carry;
	register BYTE P;

	carry = (F & C_FLAG) ? 1 : 0;
	P = memrdr(PC++);
	((A & 0xf) + (P & 0xf) + carry > 0xf) ?	(F |= H_FLAG) : (F &= ~H_FLAG);
	(A + P + carry > 255) ?	(F |= C_FLAG) : (F &= ~C_FLAG);
	A = A + P + carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_suba(void)		/* SUB A */
{
	A = 0;
	F &= ~(S_FLAG |	C_FLAG);
	F |= Z_FLAG | H_FLAG | P_FLAG;
	return(4);
}

static int op_subb(void)		/* SUB B */
{
	((B & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(B > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - B;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_subc(void)		/* SUB C */
{
	((C & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(C > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - C;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_subd(void)		/* SUB D */
{
	((D & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(D > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - D;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_sube(void)		/* SUB E */
{
	((E & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(E > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - E;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_subh(void)		/* SUB H */
{
	((H & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(H > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - H;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_subl(void)		/* SUB L */
{
	((L & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(L > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - L;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_subm(void)		/* SUB M */
{
	register BYTE P;

	P = memrdr(HL);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_suin(void)		/* SUI n */
{
	register BYTE P;

	P = memrdr(PC++);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_sbba(void)		/* SBB A */
{
	if (F & C_FLAG) {
		F |= S_FLAG | C_FLAG | P_FLAG;
		F &= ~(Z_FLAG | H_FLAG);
		A = 255;
	} else {
		F |= Z_FLAG | H_FLAG | P_FLAG;
		F &= ~(S_FLAG | C_FLAG);
		A = 0;
	}
	return(4);
}

static int op_sbbb(void)		/* SBB B */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((B & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(B + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - B - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_sbbc(void)		/* SBB C */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((C & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(C + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - C - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_sbbd(void)		/* SBB D */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((D & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(D + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - D - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_sbbe(void)		/* SBB E */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((E & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(E + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A =  A - E - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_sbbh(void)		/* SBB H */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((H & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(H + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - H - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_sbbl(void)		/* SBB L */
{
	register int carry;

	carry = (F & C_FLAG) ? 1 : 0;
	((L & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(L + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - L - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_sbbm(void)		/* SBB M */
{
	register int carry;
	register BYTE P;

	P = memrdr(HL);
	carry = (F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_sbin(void)		/* SBI n */
{
	register int carry;
	register BYTE P;

	P = memrdr(PC++);
	carry = (F & C_FLAG) ? 1 : 0;
	((P & 0xf) + carry > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P + carry > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A = A - P - carry;
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_cmpa(void)		/* CMP A */
{
	F &= ~(S_FLAG | C_FLAG);
	F |= Z_FLAG | H_FLAG | P_FLAG;
	return(4);
}

static int op_cmpb(void)		/* CMP B */
{
	register BYTE i;

	((B & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(B > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - B;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_cmpc(void)		/* CMP C */
{
	register BYTE i;

	((C & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(C > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - C;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_cmpd(void)		/* CMP D */
{
	register BYTE i;

	((D & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(D > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - D;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_cmpe(void)		/* CMP E */
{
	register BYTE i;

	((E & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(E > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - E;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_cmph(void)		/* CMP H */
{
	register BYTE i;

	((H & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(H > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - H;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_cmpl(void)		/* CMP L */
{
	register BYTE i;

	((L & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(L > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - L;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(4);
}

static int op_cmpm(void)		/* CMP M */
{
	register BYTE i;
	register BYTE P;

	P = memrdr(HL);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - P;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_cpin(void)		/* CPI n */
{
	register BYTE i;
	register BYTE P;

	P = memrdr(PC++);
	((P & 0xf) > (A & 0xf)) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(P > A) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	i = A - P;
	(parity[i]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(i & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(i) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(7);
}

static int op_inra(void)		/* INR A */
{
	A++;
	((A & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_inrb(void)		/* INR B */
{
	B++;
	((B & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_inrc(void)		/* INR C */
{
	C++;
	((C & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_inrd(void)		/* INR D */
{
	D++;
	((D & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_inre(void)		/* INR E */
{
	E++;
	((E & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_inrh(void)		/* INR H */
{
	H++;
	((H & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_inrl(void)		/* INR L */
{
	L++;
	((L & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_inrm(void)		/* INR M */
{
	register BYTE P;
	WORD addr;

	addr = HL;
	P = memrdr(addr);
        P++;
        memwrt(addr, P);
	((P & 0xf) == 0) ? (F |= H_FLAG) : (F &= ~H_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(10);
}

static int op_dcra(void)		/* DCR A */
{
	A--;
	((A & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(parity[A]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(A & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(A) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_dcrb(void)		/* DCR B */
{
	B--;
	((B & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(parity[B]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(B & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(B) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_dcrc(void)		/* DCR C */
{
	C--;
	((C & 0xf) == 0xf) ? (F &= ~H_FLAG): (F |= H_FLAG);
	(parity[C]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(C & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(C) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_dcrd(void)		/* DCR D */
{
	D--;
	((D & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(parity[D]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(D & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(D) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_dcre(void)		/* DCR E */
{
	E--;
	((E & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(parity[E]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(E & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(E) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_dcrh(void)		/* DCR H */
{
	H--;
	((H & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(parity[H]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(H & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(H) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_dcrl(void)		/* DCR L */
{
	L--;
	((L & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(parity[L]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(L & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(L) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(5);
}

static int op_dcrm(void)		/* DCR M */
{
	register BYTE P;
	WORD addr;

	addr = HL;
	P = memrdr(addr);
	P--;
	memwrt(addr, P);
	((P & 0xf) == 0xf) ? (F &= ~H_FLAG) : (F |= H_FLAG);
	(parity[P]) ? (F &= ~P_FLAG) : (F |= P_FLAG);
	(P & 128) ? (F |= S_FLAG) : (F &= ~S_FLAG);
	(P) ? (F &= ~Z_FLAG) : (F |= Z_FLAG);
	return(10);
}

static int op_rlc(void)			/* RLC */
{
	register int i;

	i = (A & 128) ?	1 : 0;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	A |= i;
	return(4);
}

static int op_rrc(void)			/* RRC */
{
	register int i;

	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (i) A |= 128;
	return(4);
}

static int op_ral(void)			/* RAL */
{
	register int old_c_flag;

	old_c_flag = F & C_FLAG;
	(A & 128) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A <<= 1;
	if (old_c_flag) A |= 1;
	return(4);
}

static int op_rar(void)			/* RAR */
{
	register int i,	old_c_flag;

	old_c_flag = F & C_FLAG;
	i = A & 1;
	(i) ? (F |= C_FLAG) : (F &= ~C_FLAG);
	A >>= 1;
	if (old_c_flag) A |= 128;
	return(4);
}

static int op_xchg(void)		/* XCHG */
{
	register BYTE i;

	i = D;
	D = H;
	H = i;
	i = E;
	E = L;
	L = i;
	return(4);
}

static int op_xthl(void)		/* XTHL */
{
	register BYTE i;

#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	i = memrdr(SP);
	memwrt(SP, L);
	L = i;
	i = memrdr(SP + 1);
	memwrt(SP + 1, H);
	H = i;
	return(18);
}

static int op_pushpsw(void)		/* PUSH PSW */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, A);
	memwrt(--SP, F);
	return(11);
}

static int op_pushb(void)		/* PUSH B */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, B);
	memwrt(--SP, C);
	return(11);
}

static int op_pushd(void)		/* PUSH D */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, D);
	memwrt(--SP, E);
	return(11);
}

static int op_pushh(void)		/* PUSH H */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, H);
	memwrt(--SP, L);
	return(11);
}

static int op_poppsw(void)		/* POP PSW */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	F = memrdr(SP++);
	F &= ~(N2_FLAG | N1_FLAG);
	F |= N_FLAG;
	A = memrdr(SP++);
	return(10);
}

static int op_popb(void)		/* POP B */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	C = memrdr(SP++);
	B = memrdr(SP++);
	return(10);
}

static int op_popd(void)		/* POP D */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	E = memrdr(SP++);
	D = memrdr(SP++);
	return(10);
}

static int op_poph(void)		/* POP H */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	L = memrdr(SP++);
	H = memrdr(SP++);
	return(10);
}

static int op_jmp(void)			/* JMP */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC) << 8;
	PC = i;
	return(10);
}

static int op_undoc_jmp(void)		/* Undocumented JMP */
{
	if (u_flag) {	/* trap with option -u */
		cpu_error = OPTRAP1;
		cpu_state = STOPPED;
		return(0);
	} else {	/* else JMP */
		return(op_jmp());
	}
}

static int op_pchl(void)		/* PCHL */
{
	PC = HL;
	return(5);
}

static int op_call(void)		/* CALL */
{
	register WORD i;

	i = memrdr(PC++);
	i += memrdr(PC++) << 8;
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = i;
	return(17);
}

static int op_undoc_call(void)		/* Undocumented CALL */
{
	if (u_flag) {	/* trap with option -u */
		cpu_error = OPTRAP1;
		cpu_state = STOPPED;
		return(0);
	} else {
		return(op_call());
	}
}

static int op_ret(void)			/* RET */
{
	register WORD i;

#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	i = memrdr(SP++);
	i += memrdr(SP++) << 8;
	PC = i;
	return(10);
}

static int op_undoc_ret(void)		/* Undocumented RET */
{
	if (u_flag) {	/* trap with option -u */
		cpu_error = OPTRAP1;
		cpu_state = STOPPED;
		return(0);
	} else {	/* else RET */
		return(op_ret());
	}
}

static int op_jz(void)			/* JZ nn */
{
	register WORD i;

	if (F & Z_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_jnz(void)			/* JNZ nn */
{
	register WORD i;

	if (!(F & Z_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_jc(void)			/* JC nn */
{
	register WORD i;

	if (F & C_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_jnc(void)			/* JNC nn */
{
	register WORD i;

	if (!(F & C_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_jpe(void)			/* JPE nn */
{
	register WORD i;

	if (F & P_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_jpo(void)			/* JPO nn */
{
	register WORD i;

	if (!(F & P_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_jm(void)			/* JM nn */
{
	register WORD i;

	if (F & S_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_jp(void)			/* JP nn */
{
	register WORD i;

	if (!(F & S_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
		PC = i;
	} else
		PC += 2;
	return(10);
}

static int op_cz(void)			/* CZ nn */
{
	register WORD i;

	if (F & Z_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_cnz(void)			/* CNZ nn */
{
	register WORD i;

	if (!(F & Z_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_cc(void)			/* CC nn */
{
	register WORD i;

	if (F & C_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_cnc(void)			/* CNC nn */
{
	register WORD i;

	if (!(F & C_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_cpe(void)			/* CPE nn */
{
	register WORD i;

	if (F & P_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_cpo(void)			/* CPO nn */
{
	register WORD i;

	if (!(F & P_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_cm(void)			/* CM nn */
{
	register WORD i;

	if (F & S_FLAG) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_cp(void)			/* CP nn */
{
	register WORD i;

	if (!(F & S_FLAG)) {
		i = memrdr(PC++);
		i += memrdr(PC++) << 8;
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		memwrt(--SP, PC_H);
		memwrt(--SP, PC);
		PC = i;
		return(17);
	} else {
		PC += 2;
		return(11);
	}
}

static int op_rz(void)			/* RZ */
{
	register WORD i;

	if (F & Z_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rnz(void)			/* RNZ */
{
	register WORD i;

	if (!(F & Z_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rc(void)			/* RC */
{
	register WORD i;

	if (F & C_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rnc(void)			/* RNC */
{
	register WORD i;

	if (!(F & C_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rpe(void)			/* RPE */
{
	register WORD i;

	if (F & P_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rpo(void)			/* RPO */
{
	register WORD i;

	if (!(F & P_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rm(void)			/* RM */
{
	register WORD i;

	if (F & S_FLAG) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rp(void)			/* RP */
{
	register WORD i;

	if (!(F & S_FLAG)) {
#ifdef BUS_8080
		cpu_bus = CPU_STACK;
#endif
		i = memrdr(SP++);
		i += memrdr(SP++) << 8;
		PC = i;
		return(11);
	} else {
		return(5);
	}
}

static int op_rst0(void)		/* RST 0 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0;
	return(11);
}

static int op_rst1(void)		/* RST 1 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0x08;
	return(11);
}

static int op_rst2(void)		/* RST 2 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0x10;
	return(11);
}

static int op_rst3(void)		/* RST 3 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0x18;
	return(11);
}

static int op_rst4(void)		/* RST 4 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0x20;
	return(11);
}

static int op_rst5(void)		/* RST 5 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0x28;
	return(11);
}

static int op_rst6(void)		/* RST 6 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0x30;
	return(11);
}

static int op_rst7(void)		/* RST 7 */
{
#ifdef BUS_8080
	cpu_bus = CPU_STACK;
#endif
	memwrt(--SP, PC_H);
	memwrt(--SP, PC);
	PC = 0x38;
	return(11);
}

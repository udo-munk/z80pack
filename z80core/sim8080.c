/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "memsim.h"

#ifndef EXCLUDE_I8080

#ifdef UNDOC_INST
#define UNDOC(f) f
#else
#define UNDOC(f) trap_undoc
#endif

#ifdef WANT_GUI
extern void check_gui_break(void);
#endif

static int trap_undoc(void);

#ifndef FAST_INSTR
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

#ifdef UNDOC_INST
static int op_undoc_nop1(void), op_undoc_nop2(void), op_undoc_nop3(void);
static int op_undoc_nop4(void), op_undoc_nop5(void), op_undoc_nop6(void);
static int op_undoc_nop7(void), op_undoc_jmp(void), op_undoc_ret(void);
static int op_undoc_call1(void), op_undoc_call2(void), op_undoc_call3(void);
#endif

#else

/*
 * Precomputed table for fast sign, zero and parity flag calculation
 */
#define _ 0
#define S S_FLAG
#define Z Z_FLAG
#define P P_FLAG
static const BYTE szp_flags[256] = {
/*00*/	Z | P,	_,	_,	P,	_,	P,	P,	_,
/*08*/	_,	P,	P,	_,	P,	_,	_,	P,
/*10*/	_,	P,	P,	_,	P,	_,	_,	P,
/*18*/	P,	_,	_,	P,	_,	P,	P,	_,
/*20*/	_,	P,	P,	_,	P,	_,	_,	P,
/*28*/	P,	_,	_,	P,	_,	P,	P,	_,
/*30*/	P,	_,	_,	P,	_,	P,	P,	_,
/*38*/	_,	P,	P,	_,	P,	_,	_,	P,
/*40*/	_,	P,	P,	_,	P,	_,	_,	P,
/*48*/	P,	_,	_,	P,	_,	P,	P,	_,
/*50*/	P,	_,	_,	P,	_,	P,	P,	_,
/*58*/	_,	P,	P,	_,	P,	_,	_,	P,
/*60*/	P,	_,	_,	P,	_,	P,	P,	_,
/*68*/	_,	P,	P,	_,	P,	_,	_,	P,
/*70*/	_,	P,	P,	_,	P,	_,	_,	P,
/*78*/	P,	_,	_,	P,	_,	P,	P,	_,
/*80*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P,
/*88*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*90*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*98*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P,
/*a0*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*a8*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P,
/*b0*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P,
/*b8*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*c0*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*c8*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P,
/*d0*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P,
/*d8*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*e0*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P,
/*e8*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*f0*/	S | P,	S,	S,	S | P,	S,	S | P,	S | P,	S,
/*f8*/	S,	S | P,	S | P,	S,	S | P,	S,	S,	S | P
};
#undef _
#undef S
#undef Z
#undef P

#define SZP_FLAGS (S_FLAG | Z_FLAG | P_FLAG)

#endif

/*
 * Function to update address bus LED's during execution of
 * instructions using the 16-bit incrementer/decrementer
 * logic, as discovered by Mike Douglas.
 */
#ifdef FRONTPANEL
static inline void addr_leds(WORD data)
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
	extern unsigned long long get_clock_us(void);

#ifndef FAST_INSTR
	static int (*op_sim[256])(void) = {
		op_nop,				/* 0x00 */
		op_lxibnn,			/* 0x01 */
		op_staxb,			/* 0x02 */
		op_inxb,			/* 0x03 */
		op_inrb,			/* 0x04 */
		op_dcrb,			/* 0x05 */
		op_mvibn,			/* 0x06 */
		op_rlc,				/* 0x07 */
		UNDOC(op_undoc_nop1),		/* 0x08 */
		op_dadb,			/* 0x09 */
		op_ldaxb,			/* 0x0a */
		op_dcxb,			/* 0x0b */
		op_inrc,			/* 0x0c */
		op_dcrc,			/* 0x0d */
		op_mvicn,			/* 0x0e */
		op_rrc,				/* 0x0f */
		UNDOC(op_undoc_nop2),		/* 0x10 */
		op_lxidnn,			/* 0x11 */
		op_staxd,			/* 0x12 */
		op_inxd,			/* 0x13 */
		op_inrd,			/* 0x14 */
		op_dcrd,			/* 0x15 */
		op_mvidn,			/* 0x16 */
		op_ral,				/* 0x17 */
		UNDOC(op_undoc_nop3),		/* 0x18 */
		op_dadd,			/* 0x19 */
		op_ldaxd,			/* 0x1a */
		op_dcxd,			/* 0x1b */
		op_inre,			/* 0x1c */
		op_dcre,			/* 0x1d */
		op_mvien,			/* 0x1e */
		op_rar,				/* 0x1f */
		UNDOC(op_undoc_nop4),		/* 0x20 */
		op_lxihnn,			/* 0x21 */
		op_shldnn,			/* 0x22 */
		op_inxh,			/* 0x23 */
		op_inrh,			/* 0x24 */
		op_dcrh,			/* 0x25 */
		op_mvihn,			/* 0x26 */
		op_daa,				/* 0x27 */
		UNDOC(op_undoc_nop5),		/* 0x28 */
		op_dadh,			/* 0x29 */
		op_lhldnn,			/* 0x2a */
		op_dcxh,			/* 0x2b */
		op_inrl,			/* 0x2c */
		op_dcrl,			/* 0x2d */
		op_mviln,			/* 0x2e */
		op_cma,				/* 0x2f */
		UNDOC(op_undoc_nop6),		/* 0x30 */
		op_lxispnn,			/* 0x31 */
		op_stann,			/* 0x32 */
		op_inxsp,			/* 0x33 */
		op_inrm,			/* 0x34 */
		op_dcrm,			/* 0x35 */
		op_mvimn,			/* 0x36 */
		op_stc,				/* 0x37 */
		UNDOC(op_undoc_nop7),		/* 0x38 */
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
		UNDOC(op_undoc_jmp),		/* 0xcb */
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
		UNDOC(op_undoc_ret),		/* 0xd9 */
		op_jc,				/* 0xda */
		op_in,				/* 0xdb */
		op_cc,				/* 0xdc */
		UNDOC(op_undoc_call1),		/* 0xdd */
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
		UNDOC(op_undoc_call2),		/* 0xed */
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
		UNDOC(op_undoc_call3),		/* 0xfd */
		op_cpin,			/* 0xfe */
		op_rst7				/* 0xff */
	};
#endif

	Tstates_t T_max;
	unsigned long long t1, t2;
	int tdiff, t;

	T_max = T + tmax;
	t1 = get_clock_us();

	do {

#ifdef HISIZE
		/* write history */
		his[h_next].h_cpu = I8080;
		his[h_next].h_addr = PC;
		his[h_next].h_af = (A << 8) + F;
		his[h_next].h_bc = (B << 8) + C;
		his[h_next].h_de = (D << 8) + E;
		his[h_next].h_hl = (H << 8) + L;
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
			t_flag = 1;		     /* turn measurement on */
			t_states_s = t_states_e = T; /* initialize markers */
		}
#endif

		/* CPU DMA bus request handling */
		if (bus_mode) {

			if (!bus_request && (bus_mode != BUS_DMA_CONTINUOUS)) {
				if (dma_bus_master) {
					/* hand control to the DMA bus master
					   without BUS_ACK */
					T += (*dma_bus_master)(0);
				}
			}

			if (bus_request) {		/* DMA bus request */
#ifdef FRONTPANEL
				if (F_flag) {
					fp_clock += 1000;
					fp_sampleData();
				}
#endif
				if (dma_bus_master) {
					/* hand control to the DMA bus master
					   with BUS_ACK */
					T += (*dma_bus_master)(1);
				}
				/* FOR NOW -
				   MAY BE NEED A PRIORITY SYSTEM LATER */
				bus_request = 0;
				if (bus_mode == BUS_DMA_CONTINUOUS)	{
					end_bus_request();
				}
#ifdef FRONTPANEL
				if (F_flag) {
					fp_clock += 1000;
					fp_sampleData();
				}
#endif
			}
		}

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
				if (F_flag) {
					fp_clock += 1000;
					fp_led_data = (int_data != -1) ?
						      (BYTE) int_data : 0xff;
					fp_sampleData();
					wait_int_step();
					if (cpu_state & RESET)
						goto leave;
				}
#endif
#ifdef BUS_8080
			}
			cpu_bus = CPU_STACK;
#endif
#ifdef FRONTPANEL
			if (F_flag) {
				fp_clock++;
				fp_sampleLightGroup(0, 0);
			}
#endif

			memwrt(--SP, PC >> 8);
			memwrt(--SP, PC);

#ifdef FRONTPANEL
			if (F_flag && (cpu_state & RESET))
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
			T += 11;
			int_int = 0;
			int_data = -1;
#ifdef FRONTPANEL
			if (F_flag)
				m1_step = 1;
#endif
		}
leave:

#ifdef BUS_8080
		/* M1 opcode fetch */
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
#endif

		int_protection = 0;

#ifndef FAST_INSTR
		t = (*op_sim[memrdr(PC++)])();	/* execute next opcode */
#else
		switch (memrdr(PC++)) {		/* execute next opcode */

#include "sim8080-00.c"

		default:
			t = trap_undoc();
			break;
		}
#endif
		T += t;

		if (f_flag) {		/* adjust CPU speed */
			if (T >= T_max && !cpu_needed) {
				t2 = get_clock_us();
				tdiff = t2 - t1;
				if ((tdiff > 0) && (tdiff < 10000))
					SLEEP_MS(10 - (tdiff / 1000));
				T_max = T + tmax;
				t1 = get_clock_us();
			}
		}

#ifdef WANT_TIM
					/* do runtime measurement */
		if (t_flag) {
			t_states_e = T; /* set end marker for this opcode */
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
	if (F_flag) {
		fp_led_address = PC;
		fp_led_data = getmem(PC);
		fp_clock++;
		fp_sampleData();
	}
#endif
}

#ifndef FAST_INSTR
#include "sim8080-00.c"
#endif

/*
 *	This function traps undocumented opcodes.
 */
static int trap_undoc(void)
{
	cpu_error = OPTRAP1;
	cpu_state = STOPPED;
	return (0);
}

#endif

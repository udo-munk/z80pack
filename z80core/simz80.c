/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1987-2024 by Udo Munk
 * Copyright (C) 2024 by Thomas Eberhardt
 */

/*
 *	This module contains the main Z80 instruction loop ( cpu_z80() )
 *	which handles interrupt requests, DMA bus requests and dispatches
 *	all single byte instructions. It also containes the trap handlers
 *	for undocumented instructions.
 */

#include "sim.h"
#include "simglb.h"
#include "config.h"
#ifdef FRONTPANEL
#include "frontpanel.h"
#endif
#include "memsim.h"

#ifndef EXCLUDE_Z80

#ifdef WANT_GUI
extern void check_gui_break(void);
#endif

static int trap_cb(void), trap_dd(void), trap_ddcb(int);
static int trap_ed(void), trap_fd(void), trap_fdcb(int);

#ifndef FAST_INSTR

#define INSTR(opcode, func)	static int func(void)
#define INSTRD(opcode, func)	static int func(int data)
#define STATES(states)		return (states)

static int op_nop(void), op_halt(void), op_scf(void);
static int op_ccf(void), op_cpl(void), op_daa(void);
static int op_ei(void), op_di(void);
static int op_in(void), op_out(void);
static int op_ldan(void), op_ldbn(void), op_ldcn(void);
static int op_lddn(void), op_lden(void);
static int op_ldhn(void), op_ldln(void);
static int op_ldabc(void), op_ldade(void), op_ldann(void);
static int op_ldbca(void), op_lddea(void), op_ldnna(void);
static int op_ldhla(void), op_ldhlb(void), op_ldhlc(void), op_ldhld(void);
static int op_ldhle(void), op_ldhlh(void), op_ldhll(void), op_ldhl1(void);
static int op_ldaa(void), op_ldab(void), op_ldac(void);
static int op_ldad(void), op_ldae(void);
static int op_ldah(void), op_ldal(void), op_ldahl(void);
static int op_ldba(void), op_ldbb(void), op_ldbc(void);
static int op_ldbd(void), op_ldbe(void);
static int op_ldbh(void), op_ldbl(void), op_ldbhl(void);
static int op_ldca(void), op_ldcb(void), op_ldcc(void);
static int op_ldcd(void), op_ldce(void);
static int op_ldch(void), op_ldcl(void), op_ldchl(void);
static int op_ldda(void), op_lddb(void), op_lddc(void);
static int op_lddd(void), op_ldde(void);
static int op_lddh(void), op_lddl(void), op_lddhl(void);
static int op_ldea(void), op_ldeb(void), op_ldec(void);
static int op_lded(void), op_ldee(void);
static int op_ldeh(void), op_ldel(void), op_ldehl(void);
static int op_ldha(void), op_ldhb(void), op_ldhc(void);
static int op_ldhd(void), op_ldhe(void);
static int op_ldhh(void), op_ldhl(void), op_ldhhl(void);
static int op_ldla(void), op_ldlb(void), op_ldlc(void);
static int op_ldld(void), op_ldle(void);
static int op_ldlh(void), op_ldll(void), op_ldlhl(void);
static int op_ldbcnn(void), op_lddenn(void), op_ldhlnn(void);
static int op_ldspnn(void), op_ldsphl(void);
static int op_ldhlin(void), op_ldinhl(void);
static int op_incbc(void), op_incde(void), op_inchl(void), op_incsp(void);
static int op_decbc(void), op_decde(void), op_dechl(void), op_decsp(void);
static int op_adhlbc(void), op_adhlde(void), op_adhlhl(void), op_adhlsp(void);
static int op_anda(void), op_andb(void), op_andc(void), op_andd(void);
static int op_ande(void), op_andh(void), op_andl(void), op_andhl(void);
static int op_andn(void);
static int op_ora(void), op_orb(void), op_orc(void), op_ord(void);
static int op_ore(void), op_orh(void), op_orl(void), op_orhl(void);
static int op_orn(void);
static int op_xora(void), op_xorb(void), op_xorc(void), op_xord(void);
static int op_xore(void), op_xorh(void), op_xorl(void), op_xorhl(void);
static int op_xorn(void);
static int op_adda(void), op_addb(void), op_addc(void), op_addd(void);
static int op_adde(void), op_addh(void), op_addl(void), op_addhl(void);
static int op_addn(void);
static int op_adca(void), op_adcb(void), op_adcc(void), op_adcd(void);
static int op_adce(void), op_adch(void), op_adcl(void), op_adchl(void);
static int op_adcn(void);
static int op_suba(void), op_subb(void), op_subc(void), op_subd(void);
static int op_sube(void), op_subh(void), op_subl(void), op_subhl(void);
static int op_subn(void);
static int op_sbca(void), op_sbcb(void), op_sbcc(void), op_sbcd(void);
static int op_sbce(void), op_sbch(void), op_sbcl(void), op_sbchl(void);
static int op_sbcn(void);
static int op_cpa(void), op_cpb(void), op_cpc(void), op_cpd(void);
static int op_cpe(void), op_cph(void), op_cplr(void), op_cphl(void);
static int op_cpn(void);
static int op_inca(void), op_incb(void), op_incc(void), op_incd(void);
static int op_ince(void), op_inch(void), op_incl(void), op_incihl(void);
static int op_deca(void), op_decb(void), op_decc(void), op_decd(void);
static int op_dece(void), op_dech(void), op_decl(void), op_decihl(void);
static int op_rlca(void), op_rrca(void), op_rla(void), op_rra(void);
static int op_exdehl(void), op_exafaf(void), op_exx(void), op_exsphl(void);
static int op_pushaf(void), op_pushbc(void), op_pushde(void), op_pushhl(void);
static int op_popaf(void), op_popbc(void), op_popde(void), op_pophl(void);
static int op_jp(void), op_jphl(void), op_jr(void), op_djnz(void);
static int op_call(void), op_ret(void);
static int op_jpz(void), op_jpnz(void), op_jpc(void), op_jpnc(void);
static int op_jppe(void), op_jppo(void), op_jpm(void), op_jpp(void);
static int op_calz(void), op_calnz(void), op_calc(void), op_calnc(void);
static int op_calpe(void), op_calpo(void), op_calm(void), op_calp(void);
static int op_retz(void), op_retnz(void), op_retc(void), op_retnc(void);
static int op_retpe(void), op_retpo(void), op_retm(void), op_retp(void);
static int op_jrz(void), op_jrnz(void), op_jrc(void), op_jrnc(void);
static int op_rst00(void), op_rst08(void), op_rst10(void), op_rst18(void);
static int op_rst20(void), op_rst28(void), op_rst30(void), op_rst38(void);
static int op_cb_handle(void), op_dd_handle(void);
static int op_ed_handle(void), op_fd_handle(void);

/* ------------------------------ op_cb_handle ----------------------------- */

static int op_srla(void), op_srlb(void), op_srlc(void);
static int op_srld(void), op_srle(void);
static int op_srlh(void), op_srll(void), op_srlhl(void);
static int op_slaa(void), op_slab(void), op_slac(void);
static int op_slad(void), op_slae(void);
static int op_slah(void), op_slal(void), op_slahl(void);
static int op_rlra(void), op_rlb(void), op_rlc(void);
static int op_rld(void), op_rle(void);
static int op_rlh(void), op_rll(void), op_rlhl(void);
static int op_rrra(void), op_rrb(void), op_rrc(void);
static int op_rrd(void), op_rre(void);
static int op_rrh(void), op_rrl(void), op_rrhl(void);
static int op_rrcra(void), op_rrcb(void), op_rrcc(void);
static int op_rrcd(void), op_rrce(void);
static int op_rrch(void), op_rrcl(void), op_rrchl(void);
static int op_rlcra(void), op_rlcb(void), op_rlcc(void);
static int op_rlcd(void), op_rlce(void);
static int op_rlch(void), op_rlcl(void), op_rlchl(void);
static int op_sraa(void), op_srab(void), op_srac(void);
static int op_srad(void), op_srae(void);
static int op_srah(void), op_sral(void), op_srahl(void);
static int op_sb0a(void), op_sb1a(void), op_sb2a(void), op_sb3a(void);
static int op_sb4a(void), op_sb5a(void), op_sb6a(void), op_sb7a(void);
static int op_sb0b(void), op_sb1b(void), op_sb2b(void), op_sb3b(void);
static int op_sb4b(void), op_sb5b(void), op_sb6b(void), op_sb7b(void);
static int op_sb0c(void), op_sb1c(void), op_sb2c(void), op_sb3c(void);
static int op_sb4c(void), op_sb5c(void), op_sb6c(void), op_sb7c(void);
static int op_sb0d(void), op_sb1d(void), op_sb2d(void), op_sb3d(void);
static int op_sb4d(void), op_sb5d(void), op_sb6d(void), op_sb7d(void);
static int op_sb0e(void), op_sb1e(void), op_sb2e(void), op_sb3e(void);
static int op_sb4e(void), op_sb5e(void), op_sb6e(void), op_sb7e(void);
static int op_sb0h(void), op_sb1h(void), op_sb2h(void), op_sb3h(void);
static int op_sb4h(void), op_sb5h(void), op_sb6h(void), op_sb7h(void);
static int op_sb0l(void), op_sb1l(void), op_sb2l(void), op_sb3l(void);
static int op_sb4l(void), op_sb5l(void), op_sb6l(void), op_sb7l(void);
static int op_sb0hl(void), op_sb1hl(void), op_sb2hl(void), op_sb3hl(void);
static int op_sb4hl(void), op_sb5hl(void), op_sb6hl(void), op_sb7hl(void);
static int op_rb0a(void), op_rb1a(void), op_rb2a(void), op_rb3a(void);
static int op_rb4a(void), op_rb5a(void), op_rb6a(void), op_rb7a(void);
static int op_rb0b(void), op_rb1b(void), op_rb2b(void), op_rb3b(void);
static int op_rb4b(void), op_rb5b(void), op_rb6b(void), op_rb7b(void);
static int op_rb0c(void), op_rb1c(void), op_rb2c(void), op_rb3c(void);
static int op_rb4c(void), op_rb5c(void), op_rb6c(void), op_rb7c(void);
static int op_rb0d(void), op_rb1d(void), op_rb2d(void), op_rb3d(void);
static int op_rb4d(void), op_rb5d(void), op_rb6d(void), op_rb7d(void);
static int op_rb0e(void), op_rb1e(void), op_rb2e(void), op_rb3e(void);
static int op_rb4e(void), op_rb5e(void), op_rb6e(void), op_rb7e(void);
static int op_rb0h(void), op_rb1h(void), op_rb2h(void), op_rb3h(void);
static int op_rb4h(void), op_rb5h(void), op_rb6h(void), op_rb7h(void);
static int op_rb0l(void), op_rb1l(void), op_rb2l(void), op_rb3l(void);
static int op_rb4l(void), op_rb5l(void), op_rb6l(void), op_rb7l(void);
static int op_rb0hl(void), op_rb1hl(void), op_rb2hl(void), op_rb3hl(void);
static int op_rb4hl(void), op_rb5hl(void), op_rb6hl(void), op_rb7hl(void);
static int op_tb0a(void), op_tb1a(void), op_tb2a(void), op_tb3a(void);
static int op_tb4a(void), op_tb5a(void), op_tb6a(void), op_tb7a(void);
static int op_tb0b(void), op_tb1b(void), op_tb2b(void), op_tb3b(void);
static int op_tb4b(void), op_tb5b(void), op_tb6b(void), op_tb7b(void);
static int op_tb0c(void), op_tb1c(void), op_tb2c(void), op_tb3c(void);
static int op_tb4c(void), op_tb5c(void), op_tb6c(void), op_tb7c(void);
static int op_tb0d(void), op_tb1d(void), op_tb2d(void), op_tb3d(void);
static int op_tb4d(void), op_tb5d(void), op_tb6d(void), op_tb7d(void);
static int op_tb0e(void), op_tb1e(void), op_tb2e(void), op_tb3e(void);
static int op_tb4e(void), op_tb5e(void), op_tb6e(void), op_tb7e(void);
static int op_tb0h(void), op_tb1h(void), op_tb2h(void), op_tb3h(void);
static int op_tb4h(void), op_tb5h(void), op_tb6h(void), op_tb7h(void);
static int op_tb0l(void), op_tb1l(void), op_tb2l(void), op_tb3l(void);
static int op_tb4l(void), op_tb5l(void), op_tb6l(void), op_tb7l(void);
static int op_tb0hl(void), op_tb1hl(void), op_tb2hl(void), op_tb3hl(void);
static int op_tb4hl(void), op_tb5hl(void), op_tb6hl(void), op_tb7hl(void);

static int op_undoc_slla(void);
#ifdef UNDOC_INST
static int op_undoc_sllb(void), op_undoc_sllc(void), op_undoc_slld(void);
static int op_undoc_slle(void), op_undoc_sllh(void), op_undoc_slll(void);
static int op_undoc_sllhl(void);
#endif

/* ------------------------------ op_dd_handle ----------------------------- */

static int op_popix(void), op_pusix(void);
static int op_jpix(void);
static int op_exspx(void);
static int op_ldspx(void);
static int op_ldixnn(void), op_ldixinn(void), op_ldinx(void);
static int op_adaxd(void), op_acaxd(void), op_suaxd(void), op_scaxd(void);
static int op_andxd(void), op_xorxd(void), op_orxd(void), op_cpxd(void);
static int op_decxd(void), op_incxd(void);
static int op_addxb(void), op_addxd(void), op_addxs(void), op_addxx(void);
static int op_incix(void), op_decix(void);
static int op_ldaxd(void), op_ldbxd(void), op_ldcxd(void);
static int op_lddxd(void), op_ldexd(void);
static int op_ldhxd(void), op_ldlxd(void);
static int op_ldxda(void), op_ldxdb(void), op_ldxdc(void);
static int op_ldxdd(void), op_ldxde(void);
static int op_ldxdh(void), op_ldxdl(void), op_ldxdn(void);
static int op_ddcb_handle(void);

#ifdef UNDOC_INST
static int op_undoc_ldaixl(void), op_undoc_ldaixh(void);
static int op_undoc_ldbixl(void), op_undoc_ldbixh(void);
static int op_undoc_ldcixl(void), op_undoc_ldcixh(void);
static int op_undoc_lddixl(void), op_undoc_lddixh(void);
static int op_undoc_ldeixl(void), op_undoc_ldeixh(void);
static int op_undoc_ldixha(void), op_undoc_ldixla(void);
static int op_undoc_ldixhb(void), op_undoc_ldixlb(void);
static int op_undoc_ldixhc(void), op_undoc_ldixlc(void);
static int op_undoc_ldixhd(void), op_undoc_ldixld(void);
static int op_undoc_ldixhe(void), op_undoc_ldixle(void);
static int op_undoc_ldixhixh(void), op_undoc_ldixlixh(void);
static int op_undoc_ldixhixl(void), op_undoc_ldixlixl(void);
static int op_undoc_ldixhn(void), op_undoc_ldixln(void);
static int op_undoc_cpixl(void), op_undoc_cpixh(void);
static int op_undoc_adaixl(void), op_undoc_adaixh(void);
static int op_undoc_acaixl(void), op_undoc_acaixh(void);
static int op_undoc_suaixl(void), op_undoc_suaixh(void);
static int op_undoc_scaixl(void), op_undoc_scaixh(void);
static int op_undoc_oraixl(void), op_undoc_oraixh(void);
static int op_undoc_andixl(void), op_undoc_andixh(void);
static int op_undoc_xorixl(void), op_undoc_xorixh(void);
static int op_undoc_incixl(void), op_undoc_incixh(void);
static int op_undoc_decixl(void), op_undoc_decixh(void);
#endif

/* ----------------------------- op_ddcb_handle ---------------------------- */

static int op_tb0ixd(int), op_tb1ixd(int), op_tb2ixd(int), op_tb3ixd(int);
static int op_tb4ixd(int), op_tb5ixd(int), op_tb6ixd(int), op_tb7ixd(int);
static int op_rb0ixd(int), op_rb1ixd(int), op_rb2ixd(int), op_rb3ixd(int);
static int op_rb4ixd(int), op_rb5ixd(int), op_rb6ixd(int), op_rb7ixd(int);
static int op_sb0ixd(int), op_sb1ixd(int), op_sb2ixd(int), op_sb3ixd(int);
static int op_sb4ixd(int), op_sb5ixd(int), op_sb6ixd(int), op_sb7ixd(int);
static int op_rlcixd(int), op_rrcixd(int), op_rlixd(int), op_rrixd(int);
static int op_slaixd(int), op_sraixd(int), op_srlixd(int);

#ifdef UNDOC_INST
static int op_undoc_sllixd(int);
#ifdef UNDOC_IALL
static int op_undoc_tb0ixd(int), op_undoc_tb1ixd(int), op_undoc_tb2ixd(int);
static int op_undoc_tb3ixd(int), op_undoc_tb4ixd(int), op_undoc_tb5ixd(int);
static int op_undoc_tb6ixd(int), op_undoc_tb7ixd(int);
static int op_undoc_rb0ixda(int), op_undoc_rb1ixda(int), op_undoc_rb2ixda(int);
static int op_undoc_rb3ixda(int), op_undoc_rb4ixda(int), op_undoc_rb5ixda(int);
static int op_undoc_rb6ixda(int), op_undoc_rb7ixda(int);
static int op_undoc_rb0ixdb(int), op_undoc_rb1ixdb(int), op_undoc_rb2ixdb(int);
static int op_undoc_rb3ixdb(int), op_undoc_rb4ixdb(int), op_undoc_rb5ixdb(int);
static int op_undoc_rb6ixdb(int), op_undoc_rb7ixdb(int);
static int op_undoc_rb0ixdc(int), op_undoc_rb1ixdc(int), op_undoc_rb2ixdc(int);
static int op_undoc_rb3ixdc(int), op_undoc_rb4ixdc(int), op_undoc_rb5ixdc(int);
static int op_undoc_rb6ixdc(int), op_undoc_rb7ixdc(int);
static int op_undoc_rb0ixdd(int), op_undoc_rb1ixdd(int), op_undoc_rb2ixdd(int);
static int op_undoc_rb3ixdd(int), op_undoc_rb4ixdd(int), op_undoc_rb5ixdd(int);
static int op_undoc_rb6ixdd(int), op_undoc_rb7ixdd(int);
static int op_undoc_rb0ixde(int), op_undoc_rb1ixde(int), op_undoc_rb2ixde(int);
static int op_undoc_rb3ixde(int), op_undoc_rb4ixde(int), op_undoc_rb5ixde(int);
static int op_undoc_rb6ixde(int), op_undoc_rb7ixde(int);
static int op_undoc_rb0ixdh(int), op_undoc_rb1ixdh(int), op_undoc_rb2ixdh(int);
static int op_undoc_rb3ixdh(int), op_undoc_rb4ixdh(int), op_undoc_rb5ixdh(int);
static int op_undoc_rb6ixdh(int), op_undoc_rb7ixdh(int);
static int op_undoc_rb0ixdl(int), op_undoc_rb1ixdl(int), op_undoc_rb2ixdl(int);
static int op_undoc_rb3ixdl(int), op_undoc_rb4ixdl(int), op_undoc_rb5ixdl(int);
static int op_undoc_rb6ixdl(int), op_undoc_rb7ixdl(int);
static int op_undoc_sb0ixda(int), op_undoc_sb1ixda(int), op_undoc_sb2ixda(int);
static int op_undoc_sb3ixda(int), op_undoc_sb4ixda(int), op_undoc_sb5ixda(int);
static int op_undoc_sb6ixda(int), op_undoc_sb7ixda(int);
static int op_undoc_sb0ixdb(int), op_undoc_sb1ixdb(int), op_undoc_sb2ixdb(int);
static int op_undoc_sb3ixdb(int), op_undoc_sb4ixdb(int), op_undoc_sb5ixdb(int);
static int op_undoc_sb6ixdb(int), op_undoc_sb7ixdb(int);
static int op_undoc_sb0ixdc(int), op_undoc_sb1ixdc(int), op_undoc_sb2ixdc(int);
static int op_undoc_sb3ixdc(int), op_undoc_sb4ixdc(int), op_undoc_sb5ixdc(int);
static int op_undoc_sb6ixdc(int), op_undoc_sb7ixdc(int);
static int op_undoc_sb0ixdd(int), op_undoc_sb1ixdd(int), op_undoc_sb2ixdd(int);
static int op_undoc_sb3ixdd(int), op_undoc_sb4ixdd(int), op_undoc_sb5ixdd(int);
static int op_undoc_sb6ixdd(int), op_undoc_sb7ixdd(int);
static int op_undoc_sb0ixde(int), op_undoc_sb1ixde(int), op_undoc_sb2ixde(int);
static int op_undoc_sb3ixde(int), op_undoc_sb4ixde(int), op_undoc_sb5ixde(int);
static int op_undoc_sb6ixde(int), op_undoc_sb7ixde(int);
static int op_undoc_sb0ixdh(int), op_undoc_sb1ixdh(int), op_undoc_sb2ixdh(int);
static int op_undoc_sb3ixdh(int), op_undoc_sb4ixdh(int), op_undoc_sb5ixdh(int);
static int op_undoc_sb6ixdh(int), op_undoc_sb7ixdh(int);
static int op_undoc_sb0ixdl(int), op_undoc_sb1ixdl(int), op_undoc_sb2ixdl(int);
static int op_undoc_sb3ixdl(int), op_undoc_sb4ixdl(int), op_undoc_sb5ixdl(int);
static int op_undoc_sb6ixdl(int), op_undoc_sb7ixdl(int);
static int op_undoc_rlcixda(int), op_undoc_rlcixdb(int), op_undoc_rlcixdc(int);
static int op_undoc_rlcixdd(int), op_undoc_rlcixde(int), op_undoc_rlcixdh(int);
static int op_undoc_rlcixdl(int);
static int op_undoc_rrcixda(int), op_undoc_rrcixdb(int), op_undoc_rrcixdc(int);
static int op_undoc_rrcixdd(int), op_undoc_rrcixde(int), op_undoc_rrcixdh(int);
static int op_undoc_rrcixdl(int);
static int op_undoc_rlixda(int), op_undoc_rlixdb(int), op_undoc_rlixdc(int);
static int op_undoc_rlixdd(int), op_undoc_rlixde(int), op_undoc_rlixdh(int);
static int op_undoc_rlixdl(int);
static int op_undoc_rrixda(int), op_undoc_rrixdb(int), op_undoc_rrixdc(int);
static int op_undoc_rrixdd(int), op_undoc_rrixde(int), op_undoc_rrixdh(int);
static int op_undoc_rrixdl(int);
static int op_undoc_slaixda(int), op_undoc_slaixdb(int), op_undoc_slaixdc(int);
static int op_undoc_slaixdd(int), op_undoc_slaixde(int), op_undoc_slaixdh(int);
static int op_undoc_slaixdl(int);
static int op_undoc_sraixda(int), op_undoc_sraixdb(int), op_undoc_sraixdc(int);
static int op_undoc_sraixdd(int), op_undoc_sraixde(int), op_undoc_sraixdh(int);
static int op_undoc_sraixdl(int);
static int op_undoc_sllixda(int), op_undoc_sllixdb(int), op_undoc_sllixdc(int);
static int op_undoc_sllixdd(int), op_undoc_sllixde(int), op_undoc_sllixdh(int);
static int op_undoc_sllixdl(int);
static int op_undoc_srlixda(int), op_undoc_srlixdb(int), op_undoc_srlixdc(int);
static int op_undoc_srlixdd(int), op_undoc_srlixde(int), op_undoc_srlixdh(int);
static int op_undoc_srlixdl(int);
#endif /* UNDOC_IALL */
#endif /* UNDOC_INST */

/* ------------------------------ op_ed_handle ----------------------------- */

static int op_im0(void), op_im1(void), op_im2(void);
static int op_reti(void), op_retn(void);
static int op_neg(void);
static int op_inaic(void), op_inbic(void), op_incic(void);
static int op_indic(void), op_ineic(void);
static int op_inhic(void), op_inlic(void);
static int op_outca(void), op_outcb(void), op_outcc(void);
static int op_outcd(void), op_outce(void);
static int op_outch(void), op_outcl(void);
static int op_ini(void), op_inir(void), op_ind(void), op_indr(void);
static int op_outi(void), op_otir(void), op_outd(void), op_otdr(void);
static int op_ldai(void), op_ldar(void), op_ldia(void), op_ldra(void);
static int op_ldbcinn(void), op_lddeinn(void);
static int op_ldhlinn(void), op_ldspinn(void);
static int op_ldinbc(void), op_ldinde(void), op_ldinhl2(void), op_ldinsp(void);
static int op_adchb(void), op_adchd(void), op_adchh(void), op_adchs(void);
static int op_sbchb(void), op_sbchd(void), op_sbchh(void), op_sbchs(void);
static int op_ldi(void), op_ldir(void), op_ldd(void), op_lddr(void);
static int op_cpi(void), op_cpir(void), op_cpdop(void), op_cpdr(void);
static int op_oprld(void), op_oprrd(void);

#ifdef UNDOC_INST
static int op_undoc_outc0(void), op_undoc_infic(void);
#ifdef UNDOC_IALL
static int op_undoc_nop(void);
static int op_undoc_im0(void), op_undoc_im1(void), op_undoc_im2(void);
static int op_undoc_reti(void), op_undoc_retn(void);
static int op_undoc_neg(void);
#endif
#endif

/* ------------------------------ op_fd_handle ----------------------------- */

static int op_popiy(void), op_pusiy(void);
static int op_jpiy(void);
static int op_exspy(void);
static int op_ldspy(void);
static int op_ldiynn(void), op_ldiyinn(void), op_ldiny(void);
static int op_adayd(void), op_acayd(void), op_suayd(void), op_scayd(void);
static int op_andyd(void), op_xoryd(void), op_oryd(void), op_cpyd(void);
static int op_decyd(void), op_incyd(void);
static int op_addyb(void), op_addyd(void), op_addys(void), op_addyy(void);
static int op_inciy(void), op_deciy(void);
static int op_ldayd(void), op_ldbyd(void), op_ldcyd(void);
static int op_lddyd(void), op_ldeyd(void);
static int op_ldhyd(void), op_ldlyd(void);
static int op_ldyda(void), op_ldydb(void), op_ldydc(void);
static int op_ldydd(void), op_ldyde(void);
static int op_ldydh(void), op_ldydl(void), op_ldydn(void);
static int op_fdcb_handle(void);

#ifdef UNDOC_INST
static int op_undoc_ldaiyl(void), op_undoc_ldaiyh(void);
static int op_undoc_ldbiyl(void), op_undoc_ldbiyh(void);
static int op_undoc_ldciyl(void), op_undoc_ldciyh(void);
static int op_undoc_lddiyl(void), op_undoc_lddiyh(void);
static int op_undoc_ldeiyl(void), op_undoc_ldeiyh(void);
static int op_undoc_ldiyha(void), op_undoc_ldiyla(void);
static int op_undoc_ldiyhb(void), op_undoc_ldiylb(void);
static int op_undoc_ldiyhc(void), op_undoc_ldiylc(void);
static int op_undoc_ldiyhd(void), op_undoc_ldiyld(void);
static int op_undoc_ldiyhe(void), op_undoc_ldiyle(void);
static int op_undoc_ldiyhiyh(void), op_undoc_ldiyliyh(void);
static int op_undoc_ldiyhiyl(void), op_undoc_ldiyliyl(void);
static int op_undoc_ldiyhn(void), op_undoc_ldiyln(void);
static int op_undoc_cpiyl(void), op_undoc_cpiyh(void);
static int op_undoc_adaiyl(void), op_undoc_adaiyh(void);
static int op_undoc_acaiyl(void), op_undoc_acaiyh(void);
static int op_undoc_suaiyl(void), op_undoc_suaiyh(void);
static int op_undoc_scaiyl(void), op_undoc_scaiyh(void);
static int op_undoc_oraiyl(void), op_undoc_oraiyh(void);
static int op_undoc_andiyl(void), op_undoc_andiyh(void);
static int op_undoc_xoriyl(void), op_undoc_xoriyh(void);
static int op_undoc_inciyl(void), op_undoc_inciyh(void);
static int op_undoc_deciyl(void), op_undoc_deciyh(void);
#endif

/* ----------------------------- op_fdcb_handle ---------------------------- */

static int op_tb0iyd(int), op_tb1iyd(int), op_tb2iyd(int), op_tb3iyd(int);
static int op_tb4iyd(int), op_tb5iyd(int), op_tb6iyd(int), op_tb7iyd(int);
static int op_rb0iyd(int), op_rb1iyd(int), op_rb2iyd(int), op_rb3iyd(int);
static int op_rb4iyd(int), op_rb5iyd(int), op_rb6iyd(int), op_rb7iyd(int);
static int op_sb0iyd(int), op_sb1iyd(int), op_sb2iyd(int), op_sb3iyd(int);
static int op_sb4iyd(int), op_sb5iyd(int), op_sb6iyd(int), op_sb7iyd(int);
static int op_rlciyd(int), op_rrciyd(int), op_rliyd(int), op_rriyd(int);
static int op_slaiyd(int), op_sraiyd(int), op_srliyd(int);

#ifdef UNDOC_INST
static int op_undoc_slliyd(int);
#ifdef UNDOC_IALL
static int op_undoc_tb0iyd(int), op_undoc_tb1iyd(int), op_undoc_tb2iyd(int);
static int op_undoc_tb3iyd(int), op_undoc_tb4iyd(int), op_undoc_tb5iyd(int);
static int op_undoc_tb6iyd(int), op_undoc_tb7iyd(int);
static int op_undoc_rb0iyda(int), op_undoc_rb1iyda(int), op_undoc_rb2iyda(int);
static int op_undoc_rb3iyda(int), op_undoc_rb4iyda(int), op_undoc_rb5iyda(int);
static int op_undoc_rb6iyda(int), op_undoc_rb7iyda(int);
static int op_undoc_rb0iydb(int), op_undoc_rb1iydb(int), op_undoc_rb2iydb(int);
static int op_undoc_rb3iydb(int), op_undoc_rb4iydb(int), op_undoc_rb5iydb(int);
static int op_undoc_rb6iydb(int), op_undoc_rb7iydb(int);
static int op_undoc_rb0iydc(int), op_undoc_rb1iydc(int), op_undoc_rb2iydc(int);
static int op_undoc_rb3iydc(int), op_undoc_rb4iydc(int), op_undoc_rb5iydc(int);
static int op_undoc_rb6iydc(int), op_undoc_rb7iydc(int);
static int op_undoc_rb0iydd(int), op_undoc_rb1iydd(int), op_undoc_rb2iydd(int);
static int op_undoc_rb3iydd(int), op_undoc_rb4iydd(int), op_undoc_rb5iydd(int);
static int op_undoc_rb6iydd(int), op_undoc_rb7iydd(int);
static int op_undoc_rb0iyde(int), op_undoc_rb1iyde(int), op_undoc_rb2iyde(int);
static int op_undoc_rb3iyde(int), op_undoc_rb4iyde(int), op_undoc_rb5iyde(int);
static int op_undoc_rb6iyde(int), op_undoc_rb7iyde(int);
static int op_undoc_rb0iydh(int), op_undoc_rb1iydh(int), op_undoc_rb2iydh(int);
static int op_undoc_rb3iydh(int), op_undoc_rb4iydh(int), op_undoc_rb5iydh(int);
static int op_undoc_rb6iydh(int), op_undoc_rb7iydh(int);
static int op_undoc_rb0iydl(int), op_undoc_rb1iydl(int), op_undoc_rb2iydl(int);
static int op_undoc_rb3iydl(int), op_undoc_rb4iydl(int), op_undoc_rb5iydl(int);
static int op_undoc_rb6iydl(int), op_undoc_rb7iydl(int);
static int op_undoc_sb0iyda(int), op_undoc_sb1iyda(int), op_undoc_sb2iyda(int);
static int op_undoc_sb3iyda(int), op_undoc_sb4iyda(int), op_undoc_sb5iyda(int);
static int op_undoc_sb6iyda(int), op_undoc_sb7iyda(int);
static int op_undoc_sb0iydb(int), op_undoc_sb1iydb(int), op_undoc_sb2iydb(int);
static int op_undoc_sb3iydb(int), op_undoc_sb4iydb(int), op_undoc_sb5iydb(int);
static int op_undoc_sb6iydb(int), op_undoc_sb7iydb(int);
static int op_undoc_sb0iydc(int), op_undoc_sb1iydc(int), op_undoc_sb2iydc(int);
static int op_undoc_sb3iydc(int), op_undoc_sb4iydc(int), op_undoc_sb5iydc(int);
static int op_undoc_sb6iydc(int), op_undoc_sb7iydc(int);
static int op_undoc_sb0iydd(int), op_undoc_sb1iydd(int), op_undoc_sb2iydd(int);
static int op_undoc_sb3iydd(int), op_undoc_sb4iydd(int), op_undoc_sb5iydd(int);
static int op_undoc_sb6iydd(int), op_undoc_sb7iydd(int);
static int op_undoc_sb0iyde(int), op_undoc_sb1iyde(int), op_undoc_sb2iyde(int);
static int op_undoc_sb3iyde(int), op_undoc_sb4iyde(int), op_undoc_sb5iyde(int);
static int op_undoc_sb6iyde(int), op_undoc_sb7iyde(int);
static int op_undoc_sb0iydh(int), op_undoc_sb1iydh(int), op_undoc_sb2iydh(int);
static int op_undoc_sb3iydh(int), op_undoc_sb4iydh(int), op_undoc_sb5iydh(int);
static int op_undoc_sb6iydh(int), op_undoc_sb7iydh(int);
static int op_undoc_sb0iydl(int), op_undoc_sb1iydl(int), op_undoc_sb2iydl(int);
static int op_undoc_sb3iydl(int), op_undoc_sb4iydl(int), op_undoc_sb5iydl(int);
static int op_undoc_sb6iydl(int), op_undoc_sb7iydl(int);
static int op_undoc_rlciyda(int), op_undoc_rlciydb(int), op_undoc_rlciydc(int);
static int op_undoc_rlciydd(int), op_undoc_rlciyde(int), op_undoc_rlciydh(int);
static int op_undoc_rlciydl(int);
static int op_undoc_rrciyda(int), op_undoc_rrciydb(int), op_undoc_rrciydc(int);
static int op_undoc_rrciydd(int), op_undoc_rrciyde(int), op_undoc_rrciydh(int);
static int op_undoc_rrciydl(int);
static int op_undoc_rliyda(int), op_undoc_rliydb(int), op_undoc_rliydc(int);
static int op_undoc_rliydd(int), op_undoc_rliyde(int), op_undoc_rliydh(int);
static int op_undoc_rliydl(int);
static int op_undoc_rriyda(int), op_undoc_rriydb(int), op_undoc_rriydc(int);
static int op_undoc_rriydd(int), op_undoc_rriyde(int), op_undoc_rriydh(int);
static int op_undoc_rriydl(int);
static int op_undoc_slaiyda(int), op_undoc_slaiydb(int), op_undoc_slaiydc(int);
static int op_undoc_slaiydd(int), op_undoc_slaiyde(int), op_undoc_slaiydh(int);
static int op_undoc_slaiydl(int);
static int op_undoc_sraiyda(int), op_undoc_sraiydb(int), op_undoc_sraiydc(int);
static int op_undoc_sraiydd(int), op_undoc_sraiyde(int), op_undoc_sraiydh(int);
static int op_undoc_sraiydl(int);
static int op_undoc_slliyda(int), op_undoc_slliydb(int), op_undoc_slliydc(int);
static int op_undoc_slliydd(int), op_undoc_slliyde(int), op_undoc_slliydh(int);
static int op_undoc_slliydl(int);
static int op_undoc_srliyda(int), op_undoc_srliydb(int), op_undoc_srliydc(int);
static int op_undoc_srliydd(int), op_undoc_srliyde(int), op_undoc_srliydh(int);
static int op_undoc_srliydl(int);
#endif /* UNDOC_IALL */
#endif /* UNDOC_INST */

#else /* FAST_INSTR */

#define INSTR(opcode, func)	case opcode:
#define INSTRD(opcode, func)	case opcode:
#define STATES(states)		t = states; break

#endif /* FAST_INSTR */

/*
 *	This function builds the Z80 central processing unit.
 *	The opcode where PC points to is fetched from the memory
 *	and PC incremented by one. The opcode is used as an
 *	index to an array with function pointers, to execute a
 *	function which emulates this Z80 opcode.
 */
void cpu_z80(void)
{
	extern unsigned long long get_clock_us(void);

#ifndef FAST_INSTR
	static int (*op_sim[256])(void) = {
		op_nop,				/* 0x00 */
		op_ldbcnn,			/* 0x01 */
		op_ldbca,			/* 0x02 */
		op_incbc,			/* 0x03 */
		op_incb,			/* 0x04 */
		op_decb,			/* 0x05 */
		op_ldbn,			/* 0x06 */
		op_rlca,			/* 0x07 */
		op_exafaf,			/* 0x08 */
		op_adhlbc,			/* 0x09 */
		op_ldabc,			/* 0x0a */
		op_decbc,			/* 0x0b */
		op_incc,			/* 0x0c */
		op_decc,			/* 0x0d */
		op_ldcn,			/* 0x0e */
		op_rrca,			/* 0x0f */
		op_djnz,			/* 0x10 */
		op_lddenn,			/* 0x11 */
		op_lddea,			/* 0x12 */
		op_incde,			/* 0x13 */
		op_incd,			/* 0x14 */
		op_decd,			/* 0x15 */
		op_lddn,			/* 0x16 */
		op_rla,				/* 0x17 */
		op_jr,				/* 0x18 */
		op_adhlde,			/* 0x19 */
		op_ldade,			/* 0x1a */
		op_decde,			/* 0x1b */
		op_ince,			/* 0x1c */
		op_dece,			/* 0x1d */
		op_lden,			/* 0x1e */
		op_rra,				/* 0x1f */
		op_jrnz,			/* 0x20 */
		op_ldhlnn,			/* 0x21 */
		op_ldinhl,			/* 0x22 */
		op_inchl,			/* 0x23 */
		op_inch,			/* 0x24 */
		op_dech,			/* 0x25 */
		op_ldhn,			/* 0x26 */
		op_daa,				/* 0x27 */
		op_jrz,				/* 0x28 */
		op_adhlhl,			/* 0x29 */
		op_ldhlin,			/* 0x2a */
		op_dechl,			/* 0x2b */
		op_incl,			/* 0x2c */
		op_decl,			/* 0x2d */
		op_ldln,			/* 0x2e */
		op_cpl,				/* 0x2f */
		op_jrnc,			/* 0x30 */
		op_ldspnn,			/* 0x31 */
		op_ldnna,			/* 0x32 */
		op_incsp,			/* 0x33 */
		op_incihl,			/* 0x34 */
		op_decihl,			/* 0x35 */
		op_ldhl1,			/* 0x36 */
		op_scf,				/* 0x37 */
		op_jrc,				/* 0x38 */
		op_adhlsp,			/* 0x39 */
		op_ldann,			/* 0x3a */
		op_decsp,			/* 0x3b */
		op_inca,			/* 0x3c */
		op_deca,			/* 0x3d */
		op_ldan,			/* 0x3e */
		op_ccf,				/* 0x3f */
		op_ldbb,			/* 0x40 */
		op_ldbc,			/* 0x41 */
		op_ldbd,			/* 0x42 */
		op_ldbe,			/* 0x43 */
		op_ldbh,			/* 0x44 */
		op_ldbl,			/* 0x45 */
		op_ldbhl,			/* 0x46 */
		op_ldba,			/* 0x47 */
		op_ldcb,			/* 0x48 */
		op_ldcc,			/* 0x49 */
		op_ldcd,			/* 0x4a */
		op_ldce,			/* 0x4b */
		op_ldch,			/* 0x4c */
		op_ldcl,			/* 0x4d */
		op_ldchl,			/* 0x4e */
		op_ldca,			/* 0x4f */
		op_lddb,			/* 0x50 */
		op_lddc,			/* 0x51 */
		op_lddd,			/* 0x52 */
		op_ldde,			/* 0x53 */
		op_lddh,			/* 0x54 */
		op_lddl,			/* 0x55 */
		op_lddhl,			/* 0x56 */
		op_ldda,			/* 0x57 */
		op_ldeb,			/* 0x58 */
		op_ldec,			/* 0x59 */
		op_lded,			/* 0x5a */
		op_ldee,			/* 0x5b */
		op_ldeh,			/* 0x5c */
		op_ldel,			/* 0x5d */
		op_ldehl,			/* 0x5e */
		op_ldea,			/* 0x5f */
		op_ldhb,			/* 0x60 */
		op_ldhc,			/* 0x61 */
		op_ldhd,			/* 0x62 */
		op_ldhe,			/* 0x63 */
		op_ldhh,			/* 0x64 */
		op_ldhl,			/* 0x65 */
		op_ldhhl,			/* 0x66 */
		op_ldha,			/* 0x67 */
		op_ldlb,			/* 0x68 */
		op_ldlc,			/* 0x69 */
		op_ldld,			/* 0x6a */
		op_ldle,			/* 0x6b */
		op_ldlh,			/* 0x6c */
		op_ldll,			/* 0x6d */
		op_ldlhl,			/* 0x6e */
		op_ldla,			/* 0x6f */
		op_ldhlb,			/* 0x70 */
		op_ldhlc,			/* 0x71 */
		op_ldhld,			/* 0x72 */
		op_ldhle,			/* 0x73 */
		op_ldhlh,			/* 0x74 */
		op_ldhll,			/* 0x75 */
		op_halt,			/* 0x76 */
		op_ldhla,			/* 0x77 */
		op_ldab,			/* 0x78 */
		op_ldac,			/* 0x79 */
		op_ldad,			/* 0x7a */
		op_ldae,			/* 0x7b */
		op_ldah,			/* 0x7c */
		op_ldal,			/* 0x7d */
		op_ldahl,			/* 0x7e */
		op_ldaa,			/* 0x7f */
		op_addb,			/* 0x80 */
		op_addc,			/* 0x81 */
		op_addd,			/* 0x82 */
		op_adde,			/* 0x83 */
		op_addh,			/* 0x84 */
		op_addl,			/* 0x85 */
		op_addhl,			/* 0x86 */
		op_adda,			/* 0x87 */
		op_adcb,			/* 0x88 */
		op_adcc,			/* 0x89 */
		op_adcd,			/* 0x8a */
		op_adce,			/* 0x8b */
		op_adch,			/* 0x8c */
		op_adcl,			/* 0x8d */
		op_adchl,			/* 0x8e */
		op_adca,			/* 0x8f */
		op_subb,			/* 0x90 */
		op_subc,			/* 0x91 */
		op_subd,			/* 0x92 */
		op_sube,			/* 0x93 */
		op_subh,			/* 0x94 */
		op_subl,			/* 0x95 */
		op_subhl,			/* 0x96 */
		op_suba,			/* 0x97 */
		op_sbcb,			/* 0x98 */
		op_sbcc,			/* 0x99 */
		op_sbcd,			/* 0x9a */
		op_sbce,			/* 0x9b */
		op_sbch,			/* 0x9c */
		op_sbcl,			/* 0x9d */
		op_sbchl,			/* 0x9e */
		op_sbca,			/* 0x9f */
		op_andb,			/* 0xa0 */
		op_andc,			/* 0xa1 */
		op_andd,			/* 0xa2 */
		op_ande,			/* 0xa3 */
		op_andh,			/* 0xa4 */
		op_andl,			/* 0xa5 */
		op_andhl,			/* 0xa6 */
		op_anda,			/* 0xa7 */
		op_xorb,			/* 0xa8 */
		op_xorc,			/* 0xa9 */
		op_xord,			/* 0xaa */
		op_xore,			/* 0xab */
		op_xorh,			/* 0xac */
		op_xorl,			/* 0xad */
		op_xorhl,			/* 0xae */
		op_xora,			/* 0xaf */
		op_orb,				/* 0xb0 */
		op_orc,				/* 0xb1 */
		op_ord,				/* 0xb2 */
		op_ore,				/* 0xb3 */
		op_orh,				/* 0xb4 */
		op_orl,				/* 0xb5 */
		op_orhl,			/* 0xb6 */
		op_ora,				/* 0xb7 */
		op_cpb,				/* 0xb8 */
		op_cpc,				/* 0xb9 */
		op_cpd,				/* 0xba */
		op_cpe,				/* 0xbb */
		op_cph,				/* 0xbc */
		op_cplr,			/* 0xbd */
		op_cphl,			/* 0xbe */
		op_cpa,				/* 0xbf */
		op_retnz,			/* 0xc0 */
		op_popbc,			/* 0xc1 */
		op_jpnz,			/* 0xc2 */
		op_jp,				/* 0xc3 */
		op_calnz,			/* 0xc4 */
		op_pushbc,			/* 0xc5 */
		op_addn,			/* 0xc6 */
		op_rst00,			/* 0xc7 */
		op_retz,			/* 0xc8 */
		op_ret,				/* 0xc9 */
		op_jpz,				/* 0xca */
		op_cb_handle,			/* 0xcb */
		op_calz,			/* 0xcc */
		op_call,			/* 0xcd */
		op_adcn,			/* 0xce */
		op_rst08,			/* 0xcf */
		op_retnc,			/* 0xd0 */
		op_popde,			/* 0xd1 */
		op_jpnc,			/* 0xd2 */
		op_out,				/* 0xd3 */
		op_calnc,			/* 0xd4 */
		op_pushde,			/* 0xd5 */
		op_subn,			/* 0xd6 */
		op_rst10,			/* 0xd7 */
		op_retc,			/* 0xd8 */
		op_exx,				/* 0xd9 */
		op_jpc,				/* 0xda */
		op_in,				/* 0xdb */
		op_calc,			/* 0xdc */
		op_dd_handle,			/* 0xdd */
		op_sbcn,			/* 0xde */
		op_rst18,			/* 0xdf */
		op_retpo,			/* 0xe0 */
		op_pophl,			/* 0xe1 */
		op_jppo,			/* 0xe2 */
		op_exsphl,			/* 0xe3 */
		op_calpo,			/* 0xe4 */
		op_pushhl,			/* 0xe5 */
		op_andn,			/* 0xe6 */
		op_rst20,			/* 0xe7 */
		op_retpe,			/* 0xe8 */
		op_jphl,			/* 0xe9 */
		op_jppe,			/* 0xea */
		op_exdehl,			/* 0xeb */
		op_calpe,			/* 0xec */
		op_ed_handle,			/* 0xed */
		op_xorn,			/* 0xee */
		op_rst28,			/* 0xef */
		op_retp,			/* 0xf0 */
		op_popaf,			/* 0xf1 */
		op_jpp,				/* 0xf2 */
		op_di,				/* 0xf3 */
		op_calp,			/* 0xf4 */
		op_pushaf,			/* 0xf5 */
		op_orn,				/* 0xf6 */
		op_rst30,			/* 0xf7 */
		op_retm,			/* 0xf8 */
		op_ldsphl,			/* 0xf9 */
		op_jpm,				/* 0xfa */
		op_ei,				/* 0xfb */
		op_calm,			/* 0xfc */
		op_fd_handle,			/* 0xfd */
		op_cpn,				/* 0xfe */
		op_rst38			/* 0xff */
	};
#endif /* !FAST_INSTR */

	Tstates_t T_max;
	unsigned long long t1, t2;
	int tdiff, t;
	WORD p;

	T_max = T + tmax;
	t1 = get_clock_us();

	do {

#ifdef HISIZE
		/* write history */
		his[h_next].h_cpu = Z80;
		his[h_next].h_addr = PC;
		his[h_next].h_af = (A << 8) + F;
		his[h_next].h_bc = (B << 8) + C;
		his[h_next].h_de = (D << 8) + E;
		his[h_next].h_hl = (H << 8) + L;
		his[h_next].h_ix = IX;
		his[h_next].h_iy = IY;
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
		if (int_nmi) {		/* non-maskable interrupt */
			IFF <<= 1 & 3;
			memwrt(--SP, PC >> 8);
			memwrt(--SP, PC);
			PC = 0x66;
#ifdef UNDOC_FLAGS
			WZ = PC;
#endif
			int_nmi = 0;
			T += 11;
			R++;		/* increment refresh register */
		}

		if (int_int) {		/* maskable interrupt */
			if (IFF != 3)
				goto leave;
			if (int_protection)	/* protect first instr */
				goto leave;

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
			cpu_bus = 0;
#endif

			switch (int_mode) {
			case 0:		/* IM 0 */
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
				T += 13;
				break;
			case 1:		/* IM 1 */
				memwrt(--SP, PC >> 8);
				memwrt(--SP, PC);
#ifdef FRONTPANEL
				if (F_flag && (cpu_state & RESET))
					goto leave;
#endif
				PC = 0x38;
				T += 13;
				break;
			case 2:		/* IM 2 */
				memwrt(--SP, PC >> 8);
				memwrt(--SP, PC);
#ifdef FRONTPANEL
				if (F_flag && (cpu_state & RESET))
					goto leave;
#endif
				p = (I << 8) + (int_data & 0xff);
				PC = memrdr(p++);
				PC += memrdr(p) << 8;
				T += 19;
				break;
			}
#ifdef UNDOC_FLAGS
			WZ = PC;
#endif
			int_int = 0;
			int_data = -1;
#ifdef FRONTPANEL
			if (F_flag)
				m1_step = 1;
#endif
			R++;		/* increment refresh register */
		}
leave:

#ifdef BUS_8080
		/* M1 opcode fetch */
		cpu_bus = CPU_WO | CPU_M1 | CPU_MEMR;
#endif

		R++;			/* increment refresh register */

#ifdef UNDOC_FLAGS
		pmodF = modF;
		modF = 0;
#endif

		int_protection = 0;

#ifndef FAST_INSTR
		t = (*op_sim[memrdr(PC++)])();	/* execute next opcode */
#else
		switch (memrdr(PC++)) {		/* execute next opcode */

#include "simz80-00.c"

		default:
			t = 0;		/* silence compiler */
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
#include "simz80-00.c"
#endif

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xcb of a multi byte opcode.
 */
static int trap_cb(void)
{
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return (0);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xdd of a multi byte opcode.
 */
static int trap_dd(void)
{
#ifdef UNDOC_INST
	if (!u_flag) {
		/* Treat 0xdd prefix as NOP on non IX-instructions */
		PC--;
		R--;
		return (4);
	}
#endif
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return (0);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xdd 0xcb of a 4 byte opcode.
 */
static int trap_ddcb(int data)
{
	UNUSED(data);

	cpu_error = OPTRAP4;
	cpu_state = STOPPED;
	return (0);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xed of a multi byte opcode.
 */
static int trap_ed(void)
{
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return (0);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xfd of a multi byte opcode.
 */
static int trap_fd(void)
{
#ifdef UNDOC_INST
	if (!u_flag) {
		/* Treat 0xfd prefix as NOP on non IY-instructions */
		PC--;
		R--;
		return (4);
	}
#endif
	cpu_error = OPTRAP2;
	cpu_state = STOPPED;
	return (0);
}

/*
 *	This function traps undocumented opcodes following the
 *	initial 0xfd 0xcb of a 4 byte opcode.
 */
static int trap_fdcb(int data)
{
	UNUSED(data);

	cpu_error = OPTRAP4;
	cpu_state = STOPPED;
	return (0);
}

#endif /* !EXCLUDE_Z80 */

/*
 * Z80SIM  -  a Z80-CPU simulator
 *
 * Copyright (C) 1989-2021 by Udo Munk
 * Parts Copyright (C) 2008 by Justin Clancy
 * 8080 disassembler Copyright (C) 2018 by Christophe Staiesse
 * Copyright (c) 2022 by Thomas Eberhardt
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

/*
 *	This module is a Z80 and 8080 disassembler for the Z80-CPU simulator
 */

#include <stdio.h>
#include <string.h>
#include "sim.h"
#include "simglb.h"
#include "memory.h"

/*
 *	Forward declarations
 */
static int opout(const char *, WORD);
static int nout(const char *, WORD);
static int iout(const char *, WORD);
static int rout(const char *, WORD);
static int nnout(const char *, WORD);
static int inout(const char *, WORD);
static int cbop(const char *, WORD);
static int edop(const char *, WORD);
static int ddfd(const char *, WORD);

/*
 *	Op-code tables
 */
struct opt {
	int (*fun) (const char *, WORD);
	const char *text;
};

static struct opt optabz80_01[64] = {
	{ opout,  "NOP"			},	/* 0x00 */
	{ nnout,  "LD\tBC,"		},	/* 0x01 */
	{ opout,  "LD\t(BC),A"		},	/* 0x02 */
	{ opout,  "INC\tBC"		},	/* 0x03 */
	{ opout,  "INC\tB"		},	/* 0x04 */
	{ opout,  "DEC\tB"		},	/* 0x05 */
	{ nout,   "LD\tB,"		},	/* 0x06 */
	{ opout,  "RLCA"		},	/* 0x07 */
	{ opout,  "EX\tAF,AF'"		},	/* 0x08 */
	{ opout,  "ADD\tHL,BC"		},	/* 0x09 */
	{ opout,  "LD\tA,(BC)"		},	/* 0x0a */
	{ opout,  "DEC\tBC"		},	/* 0x0b */
	{ opout,  "INC\tC"		},	/* 0x0c */
	{ opout,  "DEC\tC"		},	/* 0x0d */
	{ nout,   "LD\tC,"		},	/* 0x0e */
	{ opout,  "RRCA"		},	/* 0x0f */
	{ rout,   "DJNZ\t"		},	/* 0x10 */
	{ nnout,  "LD\tDE,"		},	/* 0x11 */
	{ opout,  "LD\t(DE),A"		},	/* 0x12 */
	{ opout,  "INC\tDE"		},	/* 0x13 */
	{ opout,  "INC\tD"		},	/* 0x14 */
	{ opout,  "DEC\tD"		},	/* 0x15 */
	{ nout,   "LD\tD,"		},	/* 0x16 */
	{ opout,  "RLA"			},	/* 0x17 */
	{ rout,   "JR\t"		},	/* 0x18 */
	{ opout,  "ADD\tHL,DE"		},	/* 0x19 */
	{ opout,  "LD\tA,(DE)"		},	/* 0x1a */
	{ opout,  "DEC\tDE"		},	/* 0x1b */
	{ opout,  "INC\tE"		},	/* 0x1c */
	{ opout,  "DEC\tE"		},	/* 0x1d */
	{ nout,   "LD\tE,"		},	/* 0x1e */
	{ opout,  "RRA"			},	/* 0x1f */
	{ rout,   "JR\tNZ,"		},	/* 0x20 */
	{ nnout,  "LD\tHL,"		},	/* 0x21 */
	{ inout,  "LD\t(%04X),HL"	},	/* 0x22 */
	{ opout,  "INC\tHL"		},	/* 0x23 */
	{ opout,  "INC\tH"		},	/* 0x24 */
	{ opout,  "DEC\tH"		},	/* 0x25 */
	{ nout,   "LD\tH,"		},	/* 0x26 */
	{ opout,  "DAA"			},	/* 0x27 */
	{ rout,   "JR\tZ,"		},	/* 0x28 */
	{ opout,  "ADD\tHL,HL"		},	/* 0x29 */
	{ inout,  "LD\tHL,(%04X)"	},	/* 0x2a */
	{ opout,  "DEC\tHL"		},	/* 0x2b */
	{ opout,  "INC\tL"		},	/* 0x2c */
	{ opout,  "DEC\tL"		},	/* 0x2d */
	{ nout,   "LD\tL,"		},	/* 0x2e */
	{ opout,  "CPL"			},	/* 0x2f */
	{ rout,   "JR\tNC,"		},	/* 0x30 */
	{ nnout,  "LD\tSP,"		},	/* 0x31 */
	{ inout,  "LD\t(%04X),A"	},	/* 0x32 */
	{ opout,  "INC\tSP"		},	/* 0x33 */
	{ opout,  "INC\t(HL)"		},	/* 0x34 */
	{ opout,  "DEC\t(HL)"		},	/* 0x35 */
	{ nout,   "LD\t(HL),"		},	/* 0x36 */
	{ opout,  "SCF"			},	/* 0x37 */
	{ rout,   "JR\tC,"		},	/* 0x38 */
	{ opout,  "ADD\tHL,SP"		},	/* 0x39 */
	{ inout,  "LD\tA,(%04X)"	},	/* 0x3a */
	{ opout,  "DEC\tSP"		},	/* 0x3b */
	{ opout,  "INC\tA"		},	/* 0x3c */
	{ opout,  "DEC\tA"		},	/* 0x3d */
	{ nout,   "LD\tA,"		},	/* 0x3e */
	{ opout,  "CCF"			}	/* 0x3f */
};

static struct opt optabz80_67[64] = {
	{ opout,  "RET\tNZ"		},	/* 0xc0 */
	{ opout,  "POP\tBC"		},	/* 0xc1 */
	{ nnout,  "JP\tNZ,"		},	/* 0xc2 */
	{ nnout,  "JP\t"		},	/* 0xc3 */
	{ nnout,  "CALL\tNZ,"		},	/* 0xc4 */
	{ opout,  "PUSH\tBC"		},	/* 0xc5 */
	{ nout,   "ADD\tA,"		},	/* 0xc6 */
	{ opout,  "RST\t0"		},	/* 0xc7 */
	{ opout,  "RET\tZ"		},	/* 0xc8 */
	{ opout,  "RET"			},	/* 0xc9 */
	{ nnout,  "JP\tZ,"		},	/* 0xca */
	{ cbop,   ""			},	/* 0xcb */
	{ nnout,  "CALL\tZ,"		},	/* 0xcc */
	{ nnout,  "CALL\t"		},	/* 0xcd */
	{ nout,   "ADC\tA,"		},	/* 0xce */
	{ opout,  "RST\t8"		},	/* 0xcf */
	{ opout,  "RET\tNC"		},	/* 0xd0 */
	{ opout,  "POP\tDE"		},	/* 0xd1 */
	{ nnout,  "JP\tNC,"		},	/* 0xd2 */
	{ iout,   "OUT\t(%02X),A"	},	/* 0xd3 */
	{ nnout,  "CALL\tNC,"		},	/* 0xd4 */
	{ opout,  "PUSH\tDE"		},	/* 0xd5 */
	{ nout,   "SUB\t"		},	/* 0xd6 */
	{ opout,  "RST\t10"		},	/* 0xd7 */
	{ opout,  "RET\tC"		},	/* 0xd8 */
	{ opout,  "EXX"			},	/* 0xd9 */
	{ nnout,  "JP\tC,"		},	/* 0xda */
	{ iout,   "IN\tA,(%02X)"	},	/* 0xdb */
	{ nnout,  "CALL\tC,"		},	/* 0xdc */
	{ ddfd,   ""			},	/* 0xdd */
	{ nout,   "SBC\tA,"		},	/* 0xde */
	{ opout,  "RST\t18"		},	/* 0xdf */
	{ opout,  "RET\tPO"		},	/* 0xe0 */
	{ opout,  "POP\tHL"		},	/* 0xe1 */
	{ nnout,  "JP\tPO,"		},	/* 0xe2 */
	{ opout,  "EX\t(SP),HL"		},	/* 0xe3 */
	{ nnout,  "CALL\tPO,"		},	/* 0xe4 */
	{ opout,  "PUSH\tHL"		},	/* 0xe5 */
	{ nout,   "AND\t"		},	/* 0xe6 */
	{ opout,  "RST\t20"		},	/* 0xe7 */
	{ opout,  "RET\tPE"		},	/* 0xe8 */
	{ opout,  "JP\t(HL)"		},	/* 0xe9 */
	{ nnout,  "JP\tPE,"		},	/* 0xea */
	{ opout,  "EX\tDE,HL"		},	/* 0xeb */
	{ nnout,  "CALL\tPE,"		},	/* 0xec */
	{ edop,   ""			},	/* 0xed */
	{ nout,   "XOR\t"		},	/* 0xee */
	{ opout,  "RST\t28"		},	/* 0xef */
	{ opout,  "RET\tP"		},	/* 0xf0 */
	{ opout,  "POP\tAF"		},	/* 0xf1 */
	{ nnout,  "JP\tP,"		},	/* 0xf2 */
	{ opout,  "DI"			},	/* 0xf3 */
	{ nnout,  "CALL\tP,"		},	/* 0xf4 */
	{ opout,  "PUSH\tAF"		},	/* 0xf5 */
	{ nout,   "OR\t"		},	/* 0xf6 */
	{ opout,  "RST\t30"		},	/* 0xf7 */
	{ opout,  "RET\tM"		},	/* 0xf8 */
	{ opout,  "LD\tSP,HL"		},	/* 0xf9 */
	{ nnout,  "JP\tM,"		},	/* 0xfa */
	{ opout,  "EI"			},	/* 0xfb */
	{ nnout,  "CALL\tM,"		},	/* 0xfc */
	{ ddfd,   ""			},	/* 0xfd */
	{ nout,   "CP\t"		},	/* 0xfe */
	{ opout,  "RST\t38"		}	/* 0xff */
};

static struct opt optabed_23[64] = {
	{ opout,  "IN\tB,(C)"		},	/* 0x40 */
	{ opout,  "OUT\t(C),B"		},	/* 0x41 */
	{ opout,  "SBC\tHL,BC"		},	/* 0x42 */
	{ inout,  "LD\t(%04X),BC"	},	/* 0x43 */
	{ opout,  "NEG"			},	/* 0x44 */
	{ opout,  "RETN"		},	/* 0x45 */
	{ opout,  "IM\t0"		},	/* 0x46 */
	{ opout,  "LD\tI,A"		},	/* 0x47 */
	{ opout,  "IN\tC,(C)"		},	/* 0x48 */
	{ opout,  "OUT\t(C),C"		},	/* 0x49 */
	{ opout,  "ADC\tHL,BC"		},	/* 0x4a */
	{ inout,  "LD\tBC,(%04X)"	},	/* 0x4b */
	{ opout,  "NEG*"		},	/* 0x4c */ /* undocumented */
	{ opout,  "RETI"		},	/* 0x4d */
	{ opout,  "IM*\t0"		},	/* 0x4e */ /* undocumented */
	{ opout,  "LD\tR,A"		},	/* 0x4f */
	{ opout,  "IN\tD,(C)"		},	/* 0x50 */
	{ opout,  "OUT\t(C),D"		},	/* 0x51 */
	{ opout,  "SBC\tHL,DE"		},	/* 0x52 */
	{ inout,  "LD\t(%04X),DE"	},	/* 0x53 */
	{ opout,  "NEG*"		},	/* 0x54 */ /* undocumented */
	{ opout,  "RETN*"		},	/* 0x55 */ /* undocumented */
	{ opout,  "IM\t1"		},	/* 0x56 */
	{ opout,  "LD\tA,I"		},	/* 0x57 */
	{ opout,  "IN\tE,(C)"		},	/* 0x58 */
	{ opout,  "OUT\t(C),E"		},	/* 0x59 */
	{ opout,  "ADC\tHL,DE"		},	/* 0x5a */
	{ inout,  "LD\tDE,(%04X)"	},	/* 0x5b */
	{ opout,  "NEG*"		},	/* 0x5c */ /* undocumented */
	{ opout,  "RETI*"		},	/* 0x5d */ /* undocumented */
	{ opout,  "IM\t2"		},	/* 0x5e */
	{ opout,  "LD\tA,R"		},	/* 0x5f */
	{ opout,  "IN\tH,(C)"		},	/* 0x60 */
	{ opout,  "OUT\t(C),H"		},	/* 0x61 */
	{ opout,  "SBC\tHL,HL"		},	/* 0x62 */
	{ inout,  "LD*\t(%04X),HL"	},	/* 0x63 */ /* undocumented */
	{ opout,  "NEG*"		},	/* 0x64 */ /* undocumented */
	{ opout,  "RETN*"		},	/* 0x65 */ /* undocumented */
	{ opout,  "IM*\t0"		},	/* 0x66 */ /* undocumented */
	{ opout,  "RRD"			},	/* 0x67 */
	{ opout,  "IN\tL,(C)"		},	/* 0x68 */
	{ opout,  "OUT\t(C),L"		},	/* 0x69 */
	{ opout,  "ADC\tHL,HL"		},	/* 0x6a */
	{ inout,  "LD*\tHL,(%04X)"	},	/* 0x6b */ /* undocumented */
	{ opout,  "NEG*"		},	/* 0x6c */ /* undocumented */
	{ opout,  "RETI*"		},	/* 0x6d */ /* undocumented */
	{ opout,  "IM*\t0"		},	/* 0x6e */ /* undocumented */
	{ opout,  "RLD"			},	/* 0x6f */
	{ opout,  "IN*\tF,(C)"		},	/* 0x70 */ /* undocumented */
	{ opout,  "OUT*\t(C),0"		},	/* 0x71 */ /* undocumented */
	{ opout,  "SBC\tHL,SP"		},	/* 0x72 */
	{ inout,  "LD\t(%04X),SP"	},	/* 0x73 */
	{ opout,  "NEG*"		},	/* 0x74 */ /* undocumented */
	{ opout,  "RETN*"		},	/* 0x75 */ /* undocumented */
	{ opout,  "IM*\t1"		},	/* 0x76 */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0x77 */ /* undocumented */
	{ opout,  "IN\tA,(C)"		},	/* 0x78 */
	{ opout,  "OUT\t(C),A"		},	/* 0x79 */
	{ opout,  "ADC\tHL,SP"		},	/* 0x7a */
	{ inout,  "LD\tSP,(%04X)"	},	/* 0x7b */
	{ opout,  "NEG*"		},	/* 0x7c */ /* undocumented */
	{ opout,  "RETI*"		},	/* 0x7d */ /* undocumented */
	{ opout,  "IM*\t2"		},	/* 0x7e */ /* undocumented */
	{ opout,  "NOP*"		}	/* 0x7f */ /* undocumented */
};

static struct opt optabed_5[32] = {
	{ opout,  "LDI"			},	/* 0xa0 */
	{ opout,  "CPI"			},	/* 0xa1 */
	{ opout,  "INI"			},	/* 0xa2 */
	{ opout,  "OUTI"		},	/* 0xa3 */
	{ opout,  "NOP*"		},	/* 0xa4 */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xa5 */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xa6 */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xa7 */ /* undocumented */
	{ opout,  "LDD"			},	/* 0xa8 */
	{ opout,  "CPD"			},	/* 0xa9 */
	{ opout,  "IND"			},	/* 0xaa */
	{ opout,  "OUTD"		},	/* 0xab */
	{ opout,  "NOP*"		},	/* 0xac */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xad */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xae */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xaf */ /* undocumented */
	{ opout,  "LDIR"		},	/* 0xb0 */
	{ opout,  "CPIR"		},	/* 0xb1 */
	{ opout,  "INIR"		},	/* 0xb2 */
	{ opout,  "OTIR"		},	/* 0xb3 */
	{ opout,  "NOP*"		},	/* 0xb4 */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xb5 */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xb6 */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xb7 */ /* undocumented */
	{ opout,  "LDDR"		},	/* 0xb8 */
	{ opout,  "CPDR"		},	/* 0xb9 */
	{ opout,  "INDR"		},	/* 0xba */
	{ opout,  "OTDR"		},	/* 0xbb */
	{ opout,  "NOP*"		},	/* 0xbc */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xbd */ /* undocumented */
	{ opout,  "NOP*"		},	/* 0xbe */ /* undocumented */
	{ opout,  "NOP*"		}	/* 0xbf */ /* undocumented */
};

static struct opt optabi8080_01[64] = {
	{ opout,  "NOP"			},	/* 0x00 */
	{ nnout,  "LXI\tB,"		},	/* 0x01 */
	{ opout,  "STAX\tB"		},	/* 0x02 */
	{ opout,  "INX\tB"		},	/* 0x03 */
	{ opout,  "INR\tB"		},	/* 0x04 */
	{ opout,  "DCR\tB"		},	/* 0x05 */
	{ nout,   "MVI\tB,"		},	/* 0x06 */
	{ opout,  "RLC"			},	/* 0x07 */
	{ opout,  "NOP*"		},	/* 0x08 */ /* undocumented */
	{ opout,  "DAD\tB"		},	/* 0x09 */
	{ opout,  "LDAX\tB"		},	/* 0x0a */
	{ opout,  "DCX\tB"		},	/* 0x0b */
	{ opout,  "INR\tC"		},	/* 0x0c */
	{ opout,  "DCR\tC"		},	/* 0x0d */
	{ nout,   "MVI\tC,"		},	/* 0x0e */
	{ opout,  "RRC"			},	/* 0x0f */
	{ opout,  "NOP*"		},	/* 0x10 */ /* undocumented */
	{ nnout,  "LXI\tD,"		},	/* 0x11 */
	{ opout,  "STAX\tD"		},	/* 0x12 */
	{ opout,  "INX\tD"		},	/* 0x13 */
	{ opout,  "INR\tD"		},	/* 0x14 */
	{ opout,  "DCR\tD"		},	/* 0x15 */
	{ nout,   "MVI\tD,"		},	/* 0x16 */
	{ opout,  "RAL"			},	/* 0x17 */
	{ opout,  "NOP*"		},	/* 0x18 */ /* undocumented */
	{ opout,  "DAD\tD"		},	/* 0x19 */
	{ opout,  "LDAX\tD"		},	/* 0x1a */
	{ opout,  "DCX\tD"		},	/* 0x1b */
	{ opout,  "INR\tE"		},	/* 0x1c */
	{ opout,  "DCR\tE"		},	/* 0x1d */
	{ nout,   "MVI\tE,"		},	/* 0x1e */
	{ opout,  "RAR"			},	/* 0x1f */
	{ opout,  "NOP*"		},	/* 0x20 */ /* undocumented */
	{ nnout,  "LXI\tH,"		},	/* 0x21 */
	{ nnout,  "SHLD\t"		},	/* 0x22 */
	{ opout,  "INX\tH"		},	/* 0x23 */
	{ opout,  "INR\tH"		},	/* 0x24 */
	{ opout,  "DCR\tH"		},	/* 0x25 */
	{ nout,   "MVI\tH,"		},	/* 0x26 */
	{ opout,  "DAA"			},	/* 0x27 */
	{ opout,  "NOP*"		},	/* 0x28 */ /* undocumented */
	{ opout,  "DAD\tH"		},	/* 0x29 */
	{ nnout,  "LHLD\t"		},	/* 0x2a */
	{ opout,  "DCX\tH"		},	/* 0x2b */
	{ opout,  "INR\tL"		},	/* 0x2c */
	{ opout,  "DCR\tL"		},	/* 0x2d */
	{ nout,   "MVI\tL,"		},	/* 0x2e */
	{ opout,  "CMA"			},	/* 0x2f */
	{ opout,  "NOP*"		},	/* 0x30 */ /* undocumented */
	{ nnout,  "LXI\tSP,"		},	/* 0x31 */
	{ nnout,  "STA\t"		},	/* 0x32 */
	{ opout,  "INX\tSP"		},	/* 0x33 */
	{ opout,  "INR\tM"		},	/* 0x34 */
	{ opout,  "DCR\tM"		},	/* 0x35 */
	{ nout,   "MVI\tM,"		},	/* 0x36 */
	{ opout,  "STC"			},	/* 0x37 */
	{ opout,  "NOP*"		},	/* 0x38 */ /* undocumented */
	{ opout,  "DAD\tSP"		},	/* 0x39 */
	{ nnout,  "LDA\t"		},	/* 0x3a */
	{ opout,  "DCX\tSP"		},	/* 0x3b */
	{ opout,  "INR\tA"		},	/* 0x3c */
	{ opout,  "DCR\tA"		},	/* 0x3d */
	{ nout,   "MVI\tA,"		},	/* 0x3e */
	{ opout,  "CMC"			}	/* 0x3f */
};

static struct opt optabi8080_67[64] = {
	{ opout,  "RNZ"			},	/* 0xc0 */
	{ opout,  "POP\tB"		},	/* 0xc1 */
	{ nnout,  "JNZ\t"		},	/* 0xc2 */
	{ nnout,  "JMP\t"		},	/* 0xc3 */
	{ nnout,  "CNZ\t"		},	/* 0xc4 */
	{ opout,  "PUSH\tB"		},	/* 0xc5 */
	{ nout,   "ADI\t"		},	/* 0xc6 */
	{ opout,  "RST\t0"		},	/* 0xc7 */
	{ opout,  "RZ"			},	/* 0xc8 */
	{ opout,  "RET"			},	/* 0xc9 */
	{ nnout,  "JZ\t"		},	/* 0xca */
	{ nnout,  "JMP*\t"		},	/* 0xcb */ /* undocumented */
	{ nnout,  "CZ\t"		},	/* 0xcc */
	{ nnout,  "CALL\t"		},	/* 0xcd */
	{ nout,   "ACI\t"		},	/* 0xce */
	{ opout,  "RST\t1"		},	/* 0xcf */
	{ opout,  "RNC"			},	/* 0xd0 */
	{ opout,  "POP\tD"		},	/* 0xd1 */
	{ nnout,  "JNC\t"		},	/* 0xd2 */
	{ nout,   "OUT\t"		},	/* 0xd3 */
	{ nnout,  "CNC\t"		},	/* 0xd4 */
	{ opout,  "PUSH\tD"		},	/* 0xd5 */
	{ nout,   "SUI\t"		},	/* 0xd6 */
	{ opout,  "RST\t2"		},	/* 0xd7 */
	{ opout,  "RC"			},	/* 0xd8 */
	{ opout,  "RET*"		},	/* 0xd9 */ /* undocumented */
	{ nnout,  "JC\t"		},	/* 0xda */
	{ nout,   "IN\t"		},	/* 0xdb */
	{ nnout,  "CC\t"		},	/* 0xdc */
	{ nnout,  "CALL*\t"		},	/* 0xdd */ /* undocumented */
	{ nout,   "SBI\t"		},	/* 0xde */
	{ opout,  "RST\t3"		},	/* 0xdf */
	{ opout,  "RPO"			},	/* 0xe0 */
	{ opout,  "POP\tH"		},	/* 0xe1 */
	{ nnout,  "JPO\t"		},	/* 0xe2 */
	{ opout,  "XTHL"		},	/* 0xe3 */
	{ nnout,  "CPO\t"		},	/* 0xe4 */
	{ opout,  "PUSH\tH"		},	/* 0xe5 */
	{ nout,   "ANI\t"		},	/* 0xe6 */
	{ opout,  "RST\t4"		},	/* 0xe7 */
	{ opout,  "RPE"			},	/* 0xe8 */
	{ opout,  "PCHL"		},	/* 0xe9 */
	{ nnout,  "JPE\t"		},	/* 0xea */
	{ opout,  "XCHG"		},	/* 0xeb */
	{ nnout,  "CPE\t"		},	/* 0xec */
	{ nnout,  "CALL*\t"		},	/* 0xed */ /* undocumented */
	{ nout,   "XRI\t"		},	/* 0xee */
	{ opout,  "RST\t5"		},	/* 0xef */
	{ opout,  "RP"			},	/* 0xf0 */
	{ opout,  "POP\tPSW"		},	/* 0xf1 */
	{ nnout,  "JP\t"		},	/* 0xf2 */
	{ opout,  "DI"			},	/* 0xf3 */
	{ nnout,  "CP\t"		},	/* 0xf4 */
	{ opout,  "PUSH\tPSW"		},	/* 0xf5 */
	{ nout,   "ORI\t"		},	/* 0xf6 */
	{ opout,  "RST\t6"		},	/* 0xf7 */
	{ opout,  "RM"			},	/* 0xf8 */
	{ opout,  "SPHL"		},	/* 0xf9 */
	{ nnout,  "JM\t"		},	/* 0xfa */
	{ opout,  "EI"			},	/* 0xfb */
	{ nnout,  "CM\t"		},	/* 0xfc */
	{ nnout,  "CALL*\t"		},	/* 0xfd */ /* undocumented */
	{ nout,   "CPI\t"		},	/* 0xfe */
	{ opout,  "RST\t7"		}	/* 0xff */
};

static const char *reg[]     = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
static const char *regix[]   = { "B", "C", "D", "E", "IXH", "IXL", "IX", "A" };
static const char *regiy[]   = { "B", "C", "D", "E", "IYH", "IYL", "IY", "A" };
static const char *aluins[]  = {"ADD\tA,", "ADC\tA,", "SUB\t", "SBC\tA,",
				"AND\t", "XOR\t", "OR\t", "CP\t"};
static const char *aluinsu[] = {"ADD*\tA,", "ADC*\tA,", "SUB*\t", "SBC*\tA,",
				"AND*\t", "XOR*\t", "OR*\t", "CP*\t"};
static const char *rsins[]   = {"RLC", "RRC", "RL", "RR",
				"SLA", "SRA", "SLL*", "SRL"};
static const char *rsinsu[]  = {"RLC*", "RRC*", "RL*", "RR*",
				"SLA*", "SRA*", "SLL*", "SRL*"};
static const char *bitins[]  = {"", "BIT", "RES", "SET"};
static const char *regi8080[]	 = { "B", "C", "D", "E", "H", "L", "M", "A" };
static const char *aluinsi8080[] = {"ADD", "ADC", "SUB", "SBB",
				    "ANA", "XRA", "ORA", "CMP"};

/* globals for passing disassembled code to anyone else who's interested */

char Disass_Str[64];
char Opcode_Str[64];

/* Set up machine code hex in Opcode_Str for disassembly */

static void get_opcodes(WORD addr, int len)
{
	switch (len) {
	case 1:
		sprintf(Opcode_Str, "%02X         ", getmem(addr));
		break;
	case 2:
		sprintf(Opcode_Str, "%02X %02X      ",
			getmem(addr), getmem(addr + 1));
		break;
	case 3:
		sprintf(Opcode_Str, "%02X %02X %02X   ",
			getmem(addr), getmem(addr + 1),
			getmem(addr + 2));
		break;
	case 4:
		sprintf(Opcode_Str, "%02X %02X %02X %02X",
			getmem(addr), getmem(addr + 1),
			getmem(addr + 2), getmem(addr + 3));
		break;
	default:
		sprintf(Opcode_Str, "xx OW OW xx");
	}
}

/*
 *	The function disass() is the only global function of
 *	this module. The first argument specifies the
 *	instruction set to use. The second argument is a
 *	pointer to a WORD, which hold the address of the
 *	op-code to disassemble. The output of the disassembly
 *	goes to stdout, terminated by a newline. After the
 *	disassembly the pointer to the address of the op-code
 *	will be increased by the size of the op-code, so that
 *	disass() can be called again.
 *
 *	At most four bytes will be read from memory using
 *	getmem().
 *
 */
void disass(int cpu, WORD *addr)
{
	register BYTE op;
	register int len = 1;
	struct opt *optp;

	op = getmem(*addr);
	if (cpu == Z80) {
		if (op < 0x40) {
			optp = &optabz80_01[op];
			len = (*optp->fun)(optp->text, *addr);
		} else if (op < 0x80) {
			if (op == 0x76)
				strcpy(Disass_Str, "HALT");
			else
				sprintf(Disass_Str, "LD\t%s,%s",
					reg[(op >> 3) & 7], reg[op & 7]);
		} else if (op < 0xc0)
			sprintf(Disass_Str, "%s%s",
				aluins[(op >> 3) & 7], reg[op & 7]);
		else {
			optp = &optabz80_67[op & 0x3f];
			len = (*optp->fun)(optp->text, *addr);
		}
	} else {
		if (op < 0x40) {
			optp = &optabi8080_01[op];
			len = (*optp->fun)(optp->text, *addr);
		} else if (op < 0x80) {
			if (op == 0x76)
				strcpy(Disass_Str, "HLT");
			else
				sprintf(Disass_Str, "MOV\t%s,%s",
					regi8080[(op >> 3) & 7],
					regi8080[op & 7]);
		} else if (op < 0xc0)
			sprintf(Disass_Str, "%s\t%s",
				aluinsi8080[(op >> 3) & 7],
				regi8080[op & 7]);
		else {
			optp = &optabi8080_67[op & 0x3f];
			len = (*optp->fun)(optp->text, *addr);
		}
	}
	strcat(Disass_Str, "\n");

	get_opcodes(*addr, len);
#ifndef WANT_GUI
	fputs(Opcode_Str, stdout);
	putchar('\t');
	fputs(Disass_Str, stdout);
#endif

	*addr += len;
}

/*
 *	disassemble 1 byte op-codes
 */
static int opout(const char *s, WORD a)
{
	UNUSED(a);

	strcpy(Disass_Str, s);
	return(1);
}

/*
 *	disassemble 2 byte op-codes of type "Op n"
 */
static int nout(const char *s, WORD a)
{
	sprintf(Disass_Str, "%s%02X", s, getmem(a + 1));
	return(2);
}

/*
 *	disassemble 2 byte op-codes with indirect addressing
 */
static int iout(const char *s, WORD a)
{
	sprintf(Disass_Str, s, getmem(a + 1));
	return(2);
}

/*
 *	disassemble 2 byte op-codes with relative addressing
 */
static int rout(const char *s, WORD a)
{
	sprintf(Disass_Str, "%s%04X", s, a + (signed char) getmem(a + 1) + 2);
	return(2);
}

/*
 *	disassemble 3 byte op-codes of type "Op nn"
 */
static int nnout(const char *s, WORD a)
{
	register int i;

	i = getmem(a + 1) + (getmem(a + 2) << 8);
	sprintf(Disass_Str, "%s%04X", s, i);
	return(3);
}

/*
 *	disassemble 3 byte op-codes with indirect addressing
 */
static int inout(const char *s, WORD a)
{
	register int i;

	i = getmem(a + 1) + (getmem(a + 2) << 8);
	sprintf(Disass_Str, s, i);
	return(3);
}

/*
 *	disassemble multi byte op-codes with prefix 0xcb
 */
static int cbop(const char *s, WORD a)
{
	register BYTE b2;

	UNUSED(s);

	b2 = getmem(a + 1);
	if (b2 < 0x40)
		sprintf(Disass_Str, "%s\t%s",
			rsins[b2 >> 3], reg[b2 & 7]);
	else
		sprintf(Disass_Str, "%s\t%c,%s",
			bitins[b2 >> 6], ((b2 >> 3) & 7) + '0', reg[b2 & 7]);
	return(2);
}

/*
 *	disassemble multi byte op-codes with prefix 0xed
 */
static int edop(const char *s, WORD a)
{
	register BYTE b2;
	int len = 1;

	UNUSED(s);

	b2 = getmem(a + 1);
	if (b2 < 0x40)					/* undocumented */
		strcpy(Disass_Str, "NOP*");
	else if (b2 < 0x80) {
		b2 &= 0x3f;
		len = (*optabed_23[b2].fun)(optabed_23[b2].text, a + 1);
	} else if (b2 < 0xa0)				/* undocumented */
		strcpy(Disass_Str, "NOP*");
	else if (b2 < 0xc0) {
		b2 &= 0x1f;
		len = (*optabed_5[b2].fun)(optabed_5[b2].text, a + 1);
	} else						/* undocumented */
		strcpy(Disass_Str, "NOP*");
	return(len + 1);
}

/*
 *	disassemble multi byte op-codes with prefix 0xdd and 0xfd
 */
static int ddfd(const char *s, WORD a)
{
	register BYTE b2, b4;
	register int r1, r2;
	register const char **ireg;

	UNUSED(s);

	if (getmem(a) == 0xdd)
		ireg = regix;
	else
		ireg = regiy;
	b2 = getmem(a + 1);
	r1 = (b2 >> 3) & 7;
	r2 = b2 & 7;
	if (b2 < 0x40) {
		switch (b2) {
		case 0x09:
			sprintf(Disass_Str, "ADD\t%s,BC", ireg[6]);
			return(2);
		case 0x19:
			sprintf(Disass_Str, "ADD\t%s,DE", ireg[6]);
			return(2);
		case 0x21:
			sprintf(Disass_Str, "LD\t%s,%04X", ireg[6],
				getmem(a + 2) + (getmem(a + 3) << 8));
			return(4);
		case 0x22:
			sprintf(Disass_Str, "LD\t(%04X),%s",
				getmem(a + 2) + (getmem(a + 3) << 8), ireg[6]);
			return(4);
		case 0x23:
			sprintf(Disass_Str, "INC\t%s", ireg[6]);
			return(2);
		case 0x24:				/* undocumented */
			sprintf(Disass_Str, "INC*\t%sH", ireg[6]);
			return(2);
		case 0x25:				/* undocumented */
			sprintf(Disass_Str, "DEC*\t%sH", ireg[6]);
			return(2);
		case 0x26:				/* undocumented */
			sprintf(Disass_Str, "LD*\t%sH,%02X", ireg[6],
				getmem(a + 2));
			return(3);
		case 0x29:
			sprintf(Disass_Str, "ADD\t%s,%s", ireg[6], ireg[6]);
			return(2);
		case 0x2a:
			sprintf(Disass_Str, "LD\t%s,(%04X)", ireg[6],
				getmem(a + 2) + (getmem(a + 3) << 8));
			return(4);
		case 0x2b:
			sprintf(Disass_Str, "DEC\t%s", ireg[6]);
			return(2);
		case 0x2c:				/* undocumented */
			sprintf(Disass_Str, "INC*\t%sL", ireg[6]);
			return(2);
		case 0x2d:				/* undocumented */
			sprintf(Disass_Str, "DEC*\t%sL", ireg[6]);
			return(2);
		case 0x2e:				/* undocumented */
			sprintf(Disass_Str, "LD*\t%sL,%02X", ireg[6],
				getmem(a + 2));
			return(3);
		case 0x34:
			sprintf(Disass_Str, "INC\t(%s+%02X)", ireg[6],
				getmem(a + 2));
			return(3);
		case 0x35:
			sprintf(Disass_Str, "DEC\t(%s+%02X)", ireg[6],
				getmem(a + 2));
			return(3);
		case 0x36:
			sprintf(Disass_Str, "LD\t(%s+%02X),%02X", ireg[6],
				getmem(a + 2), getmem(a + 3));
			return(4);
		case 0x39:
			sprintf(Disass_Str, "ADD\t%s,SP", ireg[6]);
			return(2);
		default:				/* undocumented */
			strcpy(Disass_Str, "NOP*");
			return(1);
		}
	} else if (b2 < 0x80) {
		if (((r1 < 4 || r1 > 6) && (r2 < 4 || r2 > 6))
		    || (r1 == 6 && r2 == 6)) {		/* undocumented */
			strcpy(Disass_Str, "NOP*");
			return(1);
		} else if (r1 == 6) {
			sprintf(Disass_Str, "LD\t(%s+%02X),%s", ireg[r1],
				getmem(a + 2), reg[r2]);
			return(3);
		} else if (r2 == 6) {
			sprintf(Disass_Str, "LD\t%s,(%s+%02X)", reg[r1],
				ireg[r2], getmem(a + 2));
			return(3);
		} else {				/* undocumented */
			sprintf(Disass_Str, "LD*\t%s,%s", ireg[r1], ireg[r2]);
			return(2);
		}
	} else if (b2 < 0xc0) {
		if (r2 < 4 || r2 > 6) {			/* undocumented */
			strcpy(Disass_Str, "NOP*");
			return(1);
		} else if (r2 == 6) {
			sprintf(Disass_Str, "%s(%s+%02X)", aluins[r1],
				ireg[r2], getmem(a + 2));
			return(3);
		} else {				/* undocumented */
			sprintf(Disass_Str, "%s%s", aluinsu[r1], ireg[r2]);
			return(2);
		}
	} else {
		switch (b2) {
		case 0xcb:
			b4 = getmem(a + 3);
			if ((b4 & 7) == 6) {
				if (b4 < 0x40)
					sprintf(Disass_Str, "%s\t(%s+%02X)",
						rsins[b4 >> 3], ireg[6],
						getmem(a + 2));
				else
					sprintf(Disass_Str, "%s\t%c,(%s+%02X)",
						bitins[b4 >> 6],
						((b4 >> 3) & 7) + '0',
						ireg[6], getmem(a + 2));
			} else {
				if (b4 < 0x40)		/* undocumented */
					sprintf(Disass_Str, "%s\t(%s+%02X),%s",
						rsinsu[b4 >> 3], ireg[6],
						getmem(a + 2), reg[b4 & 7]);
				else if (b4 < 0x80)	/* undocumented */
					sprintf(Disass_Str,
						"%s*\t%c,(%s+%02X)",
						bitins[b4 >> 6],
						((b4 >> 3) & 7) + '0',
						ireg[6], getmem(a + 2));
				else			/* undocumented */
					sprintf(Disass_Str,
						"%s*\t%c,(%s+%02X),%s",
						bitins[b4 >> 6],
						((b4 >> 3) & 7) + '0',
						ireg[6], getmem(a + 2),
						reg[b4 & 7]);
			}
			return(4);
		case 0xe1:
			sprintf(Disass_Str, "POP\t%s", ireg[6]);
			return(2);
		case 0xe3:
			sprintf(Disass_Str, "EX\t(SP),%s", ireg[6]);
			return(2);
		case 0xe5:
			sprintf(Disass_Str, "PUSH\t%s", ireg[6]);
			return(2);
		case 0xe9:
			sprintf(Disass_Str, "JP\t(%s)", ireg[6]);
			return(2);
		case 0xf9:
			sprintf(Disass_Str, "LD\tSP,%s", ireg[6]);
			return(2);
		default:				/* undocumented */
			strcpy(Disass_Str, "NOP*");
			return(1);
		}
	}
}
/*
 * Z80 and 8080 disassembler for Z80-CPU simulator
 *
 * Copyright (C) 1989-2021 by Udo Munk
 * Parts Copyright (C) 2008 by Justin Clancy
 * 8080 disassembler Copyright (C) 2018 by Christophe Staiesse
 * Copyright (c) 2022 Thomas Eberhardt
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

#include <stdio.h>
#include <string.h>
#include "sim.h"

/*
 *	Forward declarations
 */
static int opout(char *, char **);
static int nout(char *, unsigned char **);
static int iout(char *, unsigned char **);
static int rout(char *, char **);
static int nnout(char *, unsigned char **);
static int inout(char *, unsigned char **);
static int cbop(char *, unsigned char **);
static int edop(char *, unsigned char **);
static int ddfd(char *, unsigned char **);

/*
 *	Op-code tables
 */
struct opt {
	int (*fun) ();
	char *text;
};

static struct opt optabz80[256] = {
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
	{ opout,  "CCF"			},	/* 0x3f */
	{ opout,  "LD\tB,B"		},	/* 0x40 */
	{ opout,  "LD\tB,C"		},	/* 0x41 */
	{ opout,  "LD\tB,D"		},	/* 0x42 */
	{ opout,  "LD\tB,E"		},	/* 0x43 */
	{ opout,  "LD\tB,H"		},	/* 0x44 */
	{ opout,  "LD\tB,L"		},	/* 0x45 */
	{ opout,  "LD\tB,(HL)"		},	/* 0x46 */
	{ opout,  "LD\tB,A"		},	/* 0x47 */
	{ opout,  "LD\tC,B"		},	/* 0x48 */
	{ opout,  "LD\tC,C"		},	/* 0x49 */
	{ opout,  "LD\tC,D"		},	/* 0x4a */
	{ opout,  "LD\tC,E"		},	/* 0x4b */
	{ opout,  "LD\tC,H"		},	/* 0x4c */
	{ opout,  "LD\tC,L"		},	/* 0x4d */
	{ opout,  "LD\tC,(HL)"		},	/* 0x4e */
	{ opout,  "LD\tC,A"		},	/* 0x4f */
	{ opout,  "LD\tD,B"		},	/* 0x50 */
	{ opout,  "LD\tD,C"		},	/* 0x51 */
	{ opout,  "LD\tD,D"		},	/* 0x52 */
	{ opout,  "LD\tD,E"		},	/* 0x53 */
	{ opout,  "LD\tD,H"		},	/* 0x54 */
	{ opout,  "LD\tD,L"		},	/* 0x55 */
	{ opout,  "LD\tD,(HL)"		},	/* 0x56 */
	{ opout,  "LD\tD,A"		},	/* 0x57 */
	{ opout,  "LD\tE,B"		},	/* 0x58 */
	{ opout,  "LD\tE,C"		},	/* 0x59 */
	{ opout,  "LD\tE,D"		},	/* 0x5a */
	{ opout,  "LD\tE,E"		},	/* 0x5b */
	{ opout,  "LD\tE,H"		},	/* 0x5c */
	{ opout,  "LD\tE,L"		},	/* 0x5d */
	{ opout,  "LD\tE,(HL)"		},	/* 0x5e */
	{ opout,  "LD\tE,A"		},	/* 0x5f */
	{ opout,  "LD\tH,B"		},	/* 0x60 */
	{ opout,  "LD\tH,C"		},	/* 0x61 */
	{ opout,  "LD\tH,D"		},	/* 0x62 */
	{ opout,  "LD\tH,E"		},	/* 0x63 */
	{ opout,  "LD\tH,H"		},	/* 0x64 */
	{ opout,  "LD\tH,L"		},	/* 0x65 */
	{ opout,  "LD\tH,(HL)"		},	/* 0x66 */
	{ opout,  "LD\tH,A"		},	/* 0x67 */
	{ opout,  "LD\tL,B"		},	/* 0x68 */
	{ opout,  "LD\tL,C"		},	/* 0x69 */
	{ opout,  "LD\tL,D"		},	/* 0x6a */
	{ opout,  "LD\tL,E"		},	/* 0x6b */
	{ opout,  "LD\tL,H"		},	/* 0x6c */
	{ opout,  "LD\tL,L"		},	/* 0x6d */
	{ opout,  "LD\tL,(HL)"		},	/* 0x6e */
	{ opout,  "LD\tL,A"		},	/* 0x6f */
	{ opout,  "LD\t(HL),B"		},	/* 0x70 */
	{ opout,  "LD\t(HL),C"		},	/* 0x71 */
	{ opout,  "LD\t(HL),D"		},	/* 0x72 */
	{ opout,  "LD\t(HL),E"		},	/* 0x73 */
	{ opout,  "LD\t(HL),H"		},	/* 0x74 */
	{ opout,  "LD\t(HL),L"		},	/* 0x75 */
	{ opout,  "HALT"		},	/* 0x76 */
	{ opout,  "LD\t(HL),A"		},	/* 0x77 */
	{ opout,  "LD\tA,B"		},	/* 0x78 */
	{ opout,  "LD\tA,C"		},	/* 0x79 */
	{ opout,  "LD\tA,D"		},	/* 0x7a */
	{ opout,  "LD\tA,E"		},	/* 0x7b */
	{ opout,  "LD\tA,H"		},	/* 0x7c */
	{ opout,  "LD\tA,L"		},	/* 0x7d */
	{ opout,  "LD\tA,(HL)"		},	/* 0x7e */
	{ opout,  "LD\tA,A"		},	/* 0x7f */
	{ opout,  "ADD\tA,B"		},	/* 0x80 */
	{ opout,  "ADD\tA,C"		},	/* 0x81 */
	{ opout,  "ADD\tA,D"		},	/* 0x82 */
	{ opout,  "ADD\tA,E"		},	/* 0x83 */
	{ opout,  "ADD\tA,H"		},	/* 0x84 */
	{ opout,  "ADD\tA,L"		},	/* 0x85 */
	{ opout,  "ADD\tA,(HL)"		},	/* 0x86 */
	{ opout,  "ADD\tA,A"		},	/* 0x87 */
	{ opout,  "ADC\tA,B"		},	/* 0x88 */
	{ opout,  "ADC\tA,C"		},	/* 0x89 */
	{ opout,  "ADC\tA,D"		},	/* 0x8a */
	{ opout,  "ADC\tA,E"		},	/* 0x8b */
	{ opout,  "ADC\tA,H"		},	/* 0x8c */
	{ opout,  "ADC\tA,L"		},	/* 0x8d */
	{ opout,  "ADC\tA,(HL)"		},	/* 0x8e */
	{ opout,  "ADC\tA,A"		},	/* 0x8f */
	{ opout,  "SUB\tB"		},	/* 0x90 */
	{ opout,  "SUB\tC"		},	/* 0x91 */
	{ opout,  "SUB\tD"		},	/* 0x92 */
	{ opout,  "SUB\tE"		},	/* 0x93 */
	{ opout,  "SUB\tH"		},	/* 0x94 */
	{ opout,  "SUB\tL"		},	/* 0x95 */
	{ opout,  "SUB\t(HL)"		},	/* 0x96 */
	{ opout,  "SUB\tA"		},	/* 0x97 */
	{ opout,  "SBC\tA,B"		},	/* 0x98 */
	{ opout,  "SBC\tA,C"		},	/* 0x99 */
	{ opout,  "SBC\tA,D"		},	/* 0x9a */
	{ opout,  "SBC\tA,E"		},	/* 0x9b */
	{ opout,  "SBC\tA,H"		},	/* 0x9c */
	{ opout,  "SBC\tA,L"		},	/* 0x9d */
	{ opout,  "SBC\tA,(HL)"		},	/* 0x9e */
	{ opout,  "SBC\tA,A"		},	/* 0x9f */
	{ opout,  "AND\tB"		},	/* 0xa0 */
	{ opout,  "AND\tC"		},	/* 0xa1 */
	{ opout,  "AND\tD"		},	/* 0xa2 */
	{ opout,  "AND\tE"		},	/* 0xa3 */
	{ opout,  "AND\tH"		},	/* 0xa4 */
	{ opout,  "AND\tL"		},	/* 0xa5 */
	{ opout,  "AND\t(HL)"		},	/* 0xa6 */
	{ opout,  "AND\tA"		},	/* 0xa7 */
	{ opout,  "XOR\tB"		},	/* 0xa8 */
	{ opout,  "XOR\tC"		},	/* 0xa9 */
	{ opout,  "XOR\tD"		},	/* 0xaa */
	{ opout,  "XOR\tE"		},	/* 0xab */
	{ opout,  "XOR\tH"		},	/* 0xac */
	{ opout,  "XOR\tL"		},	/* 0xad */
	{ opout,  "XOR\t(HL)"		},	/* 0xae */
	{ opout,  "XOR\tA"		},	/* 0xaf */
	{ opout,  "OR\tB"		},	/* 0xb0 */
	{ opout,  "OR\tC"		},	/* 0xb1 */
	{ opout,  "OR\tD"		},	/* 0xb2 */
	{ opout,  "OR\tE"		},	/* 0xb3 */
	{ opout,  "OR\tH"		},	/* 0xb4 */
	{ opout,  "OR\tL"		},	/* 0xb5 */
	{ opout,  "OR\t(HL)"		},	/* 0xb6 */
	{ opout,  "OR\tA"		},	/* 0xb7 */
	{ opout,  "CP\tB"		},	/* 0xb8 */
	{ opout,  "CP\tC"		},	/* 0xb9 */
	{ opout,  "CP\tD"		},	/* 0xba */
	{ opout,  "CP\tE"		},	/* 0xbb */
	{ opout,  "CP\tH"		},	/* 0xbc */
	{ opout,  "CP\tL"		},	/* 0xbd */
	{ opout,  "CP\t(HL)"		},	/* 0xbe */
	{ opout,  "CP\tA"		},	/* 0xbf */
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

static struct opt optabi8080[256] = {
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
	{ opout,  "CMC"			},	/* 0x3f */
	{ opout,  "MOV\tB,B"		},	/* 0x40 */
	{ opout,  "MOV\tB,C"		},	/* 0x41 */
	{ opout,  "MOV\tB,D"		},	/* 0x42 */
	{ opout,  "MOV\tB,E"		},	/* 0x43 */
	{ opout,  "MOV\tB,H"		},	/* 0x44 */
	{ opout,  "MOV\tB,L"		},	/* 0x45 */
	{ opout,  "MOV\tB,M"		},	/* 0x46 */
	{ opout,  "MOV\tB,A"		},	/* 0x47 */
	{ opout,  "MOV\tC,B"		},	/* 0x48 */
	{ opout,  "MOV\tC,C"		},	/* 0x49 */
	{ opout,  "MOV\tC,D"		},	/* 0x4a */
	{ opout,  "MOV\tC,E"		},	/* 0x4b */
	{ opout,  "MOV\tC,H"		},	/* 0x4c */
	{ opout,  "MOV\tC,L"		},	/* 0x4d */
	{ opout,  "MOV\tC,M"		},	/* 0x4e */
	{ opout,  "MOV\tC,A"		},	/* 0x4f */
	{ opout,  "MOV\tD,B"		},	/* 0x50 */
	{ opout,  "MOV\tD,C"		},	/* 0x51 */
	{ opout,  "MOV\tD,D"		},	/* 0x52 */
	{ opout,  "MOV\tD,E"		},	/* 0x53 */
	{ opout,  "MOV\tD,H"		},	/* 0x54 */
	{ opout,  "MOV\tD,L"		},	/* 0x55 */
	{ opout,  "MOV\tD,M"		},	/* 0x56 */
	{ opout,  "MOV\tD,A"		},	/* 0x57 */
	{ opout,  "MOV\tE,B"		},	/* 0x58 */
	{ opout,  "MOV\tE,C"		},	/* 0x59 */
	{ opout,  "MOV\tE,D"		},	/* 0x5a */
	{ opout,  "MOV\tE,E"		},	/* 0x5b */
	{ opout,  "MOV\tE,H"		},	/* 0x5c */
	{ opout,  "MOV\tE,L"		},	/* 0x5d */
	{ opout,  "MOV\tE,M"		},	/* 0x5e */
	{ opout,  "MOV\tE,A"		},	/* 0x5f */
	{ opout,  "MOV\tH,B"		},	/* 0x60 */
	{ opout,  "MOV\tH,C"		},	/* 0x61 */
	{ opout,  "MOV\tH,D"		},	/* 0x62 */
	{ opout,  "MOV\tH,E"		},	/* 0x63 */
	{ opout,  "MOV\tH,H"		},	/* 0x64 */
	{ opout,  "MOV\tH,L"		},	/* 0x65 */
	{ opout,  "MOV\tH,M"		},	/* 0x66 */
	{ opout,  "MOV\tH,A"		},	/* 0x67 */
	{ opout,  "MOV\tL,B"		},	/* 0x68 */
	{ opout,  "MOV\tL,C"		},	/* 0x69 */
	{ opout,  "MOV\tL,D"		},	/* 0x6a */
	{ opout,  "MOV\tL,E"		},	/* 0x6b */
	{ opout,  "MOV\tL,H"		},	/* 0x6c */
	{ opout,  "MOV\tL,L"		},	/* 0x6d */
	{ opout,  "MOV\tL,M"		},	/* 0x6e */
	{ opout,  "MOV\tL,A"		},	/* 0x6f */
	{ opout,  "MOV\tM,B"		},	/* 0x70 */
	{ opout,  "MOV\tM,C"		},	/* 0x71 */
	{ opout,  "MOV\tM,D"		},	/* 0x72 */
	{ opout,  "MOV\tM,E"		},	/* 0x73 */
	{ opout,  "MOV\tM,H"		},	/* 0x74 */
	{ opout,  "MOV\tM,L"		},	/* 0x75 */
	{ opout,  "HLT"			},	/* 0x76 */
	{ opout,  "MOV\tM,A"		},	/* 0x77 */
	{ opout,  "MOV\tA,B"		},	/* 0x78 */
	{ opout,  "MOV\tA,C"		},	/* 0x79 */
	{ opout,  "MOV\tA,D"		},	/* 0x7a */
	{ opout,  "MOV\tA,E"		},	/* 0x7b */
	{ opout,  "MOV\tA,H"		},	/* 0x7c */
	{ opout,  "MOV\tA,L"		},	/* 0x7d */
	{ opout,  "MOV\tA,M"		},	/* 0x7e */
	{ opout,  "MOV\tA,A"		},	/* 0x7f */
	{ opout,  "ADD\tB"		},	/* 0x80 */
	{ opout,  "ADD\tC"		},	/* 0x81 */
	{ opout,  "ADD\tD"		},	/* 0x82 */
	{ opout,  "ADD\tE"		},	/* 0x83 */
	{ opout,  "ADD\tH"		},	/* 0x84 */
	{ opout,  "ADD\tL"		},	/* 0x85 */
	{ opout,  "ADD\tM"		},	/* 0x86 */
	{ opout,  "ADD\tA"		},	/* 0x87 */
	{ opout,  "ADC\tB"		},	/* 0x88 */
	{ opout,  "ADC\tC"		},	/* 0x89 */
	{ opout,  "ADC\tD"		},	/* 0x8a */
	{ opout,  "ADC\tE"		},	/* 0x8b */
	{ opout,  "ADC\tH"		},	/* 0x8c */
	{ opout,  "ADC\tL"		},	/* 0x8d */
	{ opout,  "ADC\tM"		},	/* 0x8e */
	{ opout,  "ADC\tA"		},	/* 0x8f */
	{ opout,  "SUB\tB"		},	/* 0x90 */
	{ opout,  "SUB\tC"		},	/* 0x91 */
	{ opout,  "SUB\tD"		},	/* 0x92 */
	{ opout,  "SUB\tE"		},	/* 0x93 */
	{ opout,  "SUB\tH"		},	/* 0x94 */
	{ opout,  "SUB\tL"		},	/* 0x95 */
	{ opout,  "SUB\tM"		},	/* 0x96 */
	{ opout,  "SUB\tA"		},	/* 0x97 */
	{ opout,  "SBB\tB"		},	/* 0x98 */
	{ opout,  "SBB\tC"		},	/* 0x99 */
	{ opout,  "SBB\tD"		},	/* 0x9a */
	{ opout,  "SBB\tE"		},	/* 0x9b */
	{ opout,  "SBB\tH"		},	/* 0x9c */
	{ opout,  "SBB\tL"		},	/* 0x9d */
	{ opout,  "SBB\tM"		},	/* 0x9e */
	{ opout,  "SBB\tA"		},	/* 0x9f */
	{ opout,  "ANA\tB"		},	/* 0xa0 */
	{ opout,  "ANA\tC"		},	/* 0xa1 */
	{ opout,  "ANA\tD"		},	/* 0xa2 */
	{ opout,  "ANA\tE"		},	/* 0xa3 */
	{ opout,  "ANA\tH"		},	/* 0xa4 */
	{ opout,  "ANA\tL"		},	/* 0xa5 */
	{ opout,  "ANA\tM"		},	/* 0xa6 */
	{ opout,  "ANA\tA"		},	/* 0xa7 */
	{ opout,  "XRA\tB"		},	/* 0xa8 */
	{ opout,  "XRA\tC"		},	/* 0xa9 */
	{ opout,  "XRA\tD"		},	/* 0xaa */
	{ opout,  "XRA\tE"		},	/* 0xab */
	{ opout,  "XRA\tH"		},	/* 0xac */
	{ opout,  "XRA\tL"		},	/* 0xad */
	{ opout,  "XRA\tM"		},	/* 0xae */
	{ opout,  "XRA\tA"		},	/* 0xaf */
	{ opout,  "ORA\tB"		},	/* 0xb0 */
	{ opout,  "ORA\tC"		},	/* 0xb1 */
	{ opout,  "ORA\tD"		},	/* 0xb2 */
	{ opout,  "ORA\tE"		},	/* 0xb3 */
	{ opout,  "ORA\tH"		},	/* 0xb4 */
	{ opout,  "ORA\tL"		},	/* 0xb5 */
	{ opout,  "ORA\tM"		},	/* 0xb6 */
	{ opout,  "ORA\tA"		},	/* 0xb7 */
	{ opout,  "CMP\tB"		},	/* 0xb8 */
	{ opout,  "CMP\tC"		},	/* 0xb9 */
	{ opout,  "CMP\tD"		},	/* 0xba */
	{ opout,  "CMP\tE"		},	/* 0xbb */
	{ opout,  "CMP\tH"		},	/* 0xbc */
	{ opout,  "CMP\tL"		},	/* 0xbd */
	{ opout,  "CMP\tM"		},	/* 0xbe */
	{ opout,  "CMP\tA"		},	/* 0xbf */
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

static int addr;
static char *unknown = "???";
static char *reg[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
static char *regix = "IX";
static char *regiy = "IY";

/* globals for passing disassembled code to anyone else who's interested */

char Disass_Str[64];
char Opcode_Str[64];

/* Set up machine code hex in Opcode_Str for disassembly */

static void get_opcodes(unsigned char **p, int len)
{
	switch (len) {
	case 1:
		sprintf(Opcode_Str, "%02X         ", (**p & 0xff));
		break;
	case 2:
		sprintf(Opcode_Str, "%02X %02X      ",
			(**p & 0xff), *(*p + 1) & 0xff);
		break;
	case 3:
		sprintf(Opcode_Str, "%02X %02X %02X   ",
			(**p & 0xff), *(*p + 1) & 0xff,
			*(*p + 2) & 0xff);
		break;
	case 4:
		sprintf(Opcode_Str, "%02X %02X %02X %02X",
			(**p & 0xff), *(*p + 1) & 0xff,
			*(*p + 2) & 0xff, *(*p + 3) & 0xff);
		break;
	default:
		sprintf(Opcode_Str, "xx OW OW xx");
	}
}

/*
 *	The function disass() is the only global function of
 *	this module. The first argument is a pointer to a
 *	unsigned char pointer, which points to the op-code
 *	to disassemble. The output of the disassembly goes
 *	to stdout, terminated by a newline. After the
 *	disassembly the pointer to the op-code will be
 *	increased by the size of the op-code, so that
 *	disass() can be called again.
 *	The second argument is the (Z80) address of the
 *	op-code to disassemble. It is used to calculate the
 *	destination address of relative jumps.
 *
 *	At most four bytes will be read from the buffer.
 *
 *	To handle memory wrap around from high memory to low
 *	memory addresses, set base to a non-null pointer
 *	designating the base of a 64K memory block. *p should
 *	point into this block.
 */
void disass(int cpu, unsigned char **p, int adr, unsigned char *base)
{
	register int len;
	unsigned char buf[4];
	size_t ofs;
	int i;

	addr = adr;

	if (base != NULL) {
		ofs = *p - base;
		for (i = 0; i < 4; i++)
			buf[i] = base[(ofs + i) & 0xffff];
		*p = buf;
	}

	if (cpu == Z80)
		len = (*optabz80[**p].fun) (optabz80[**p].text, p);
	else
		len = (*optabi8080[**p].fun) (optabi8080[**p].text, p);

	get_opcodes(p, len);
#ifndef WANT_GUI
	fputs(Opcode_Str, stdout);
	putchar('\t');
	fputs(Disass_Str, stdout);
#endif

	if (base != NULL)
		*p = base + ((ofs + len) & 0xffff);
	else
		*p += len;
}

/*
 *	disassemble 1 byte op-codes
 */
static int opout(char *s, char **p)
{
	p = p;	/* to avoid compiler warning */

	sprintf(Disass_Str, "%s\n", s);
	return(1);
}

/*
 *	disassemble 2 byte op-codes of type "Op n"
 */
static int nout(char *s, unsigned char **p)
{
	sprintf(Disass_Str, "%s%02X\n", s, *(*p + 1));
	return(2);
}

/*
 *	disassemble 2 byte op-codes with indirect addressing
 */
static int iout(char *s, unsigned char **p)
{
	sprintf(Disass_Str, s, *(*p + 1));
	strcat(Disass_Str, "\n");
	return(2);
}

/*
 *	disassemble 2 byte op-codes with relative addressing
 */
static int rout(char *s, char **p)
{
	sprintf(Disass_Str, "%s%04X\n", s, (addr + *(*p + 1) + 2) & 0xffff);
	return(2);
}

/*
 *	disassemble 3 byte op-codes of type "Op nn"
 */
static int nnout(char *s, unsigned char **p)
{
	register int i;

	i = *(*p + 1) + (*(*p + 2) << 8);
	sprintf(Disass_Str, "%s%04X\n", s, i);
	return(3);
}

/*
 *	disassemble 3 byte op-codes with indirect addressing
 */
static int inout(char *s, unsigned char **p)
{
	register int i;

	i = *(*p + 1) + (*(*p + 2) << 8);
	sprintf(Disass_Str, s, i);
	strcat(Disass_Str, "\n");
	return(3);
}

/*
 *	disassemble multi byte op-codes with prefix 0xcb
 */
static int cbop(char *s, unsigned char **p)
{
	register int b2;

	s = s;	/* to avoid compiler warning */

	b2 = *(*p + 1);
	if (b2 >= 0x00 && b2 <= 0x07) {
		sprintf(Disass_Str, "RLC\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x08 && b2 <= 0x0f) {
		sprintf(Disass_Str, "RRC\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x10 && b2 <= 0x17) {
		sprintf(Disass_Str, "RL\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x18 && b2 <= 0x1f) {
		sprintf(Disass_Str, "RR\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x20 && b2 <= 0x27) {
		sprintf(Disass_Str, "SLA\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x28 && b2 <= 0x2f) {
		sprintf(Disass_Str, "SRA\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x30 && b2 <= 0x37) {		/* undocumented */
		sprintf(Disass_Str, "SLL*\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x38 && b2 <= 0x3f) {
		sprintf(Disass_Str, "SRL\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x40 && b2 <= 0x7f) {
		sprintf(Disass_Str, "BIT\t%c,%s\n",
			((b2 >> 3) & 7) + '0', reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x80 && b2 <= 0xbf) {
		sprintf(Disass_Str, "RES\t%c,%s\n",
			((b2 >> 3) & 7) + '0', reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0xc0) {
		sprintf(Disass_Str, "SET\t%c,%s\n",
			((b2 >> 3) & 7) + '0', reg[b2 & 7]);
		return(2);
	}
	sprintf(Disass_Str, "%s\n", unknown);
	return(2);
}

/*
 *	disassemble multi byte op-codes with prefix 0xed
 */
static int edop(char *s, unsigned char **p)
{
	register int b2, i;
	int len = 2;

	s = s;	/* to avoid compiler warning */

	Disass_Str[0] = 0;
	b2 = *(*p + 1);
	switch (b2) {
	case 0x40:
		strcat(Disass_Str, "IN\tB,(C)\n");
		break;
	case 0x41:
		strcat(Disass_Str, "OUT\t(C),B\n");
		break;
	case 0x42:
		strcat(Disass_Str, "SBC\tHL,BC\n");
		break;
	case 0x43:
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD\t(%04X),BC\n", i);
		len = 4;
		break;
	case 0x44:
		strcat(Disass_Str, "NEG\n");
		break;
	case 0x45:
		strcat(Disass_Str, "RETN\n");
		break;
	case 0x46:
		strcat(Disass_Str, "IM\t0\n");
		break;
	case 0x47:
		strcat(Disass_Str, "LD\tI,A\n");
		break;
	case 0x48:
		strcat(Disass_Str, "IN\tC,(C)\n");
		break;
	case 0x49:
		strcat(Disass_Str, "OUT\t(C),C\n");
		break;
	case 0x4a:
		strcat(Disass_Str, "ADC\tHL,BC\n");
		break;
	case 0x4b:
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD\tBC,(%04X)\n", i);
		len = 4;
		break;
	case 0x4c:				/* undocumented */
	case 0x54:				/* undocumented */
	case 0x5c:				/* undocumented */
	case 0x64:				/* undocumented */
	case 0x6c:				/* undocumented */
	case 0x74:				/* undocumented */
	case 0x7c:				/* undocumented */
		strcat(Disass_Str, "NEG*\n");
		break;
	case 0x4d:
		strcat(Disass_Str, "RETI\n");
		break;
	case 0x4e:				/* undocumented */
	case 0x66:				/* undocumented */
	case 0x6e:				/* undocumented */
		strcat(Disass_Str, "IM*\t0\n");
		break;
	case 0x4f:
		strcat(Disass_Str, "LD\tR,A\n");
		break;
	case 0x50:
		strcat(Disass_Str, "IN\tD,(C)\n");
		break;
	case 0x51:
		strcat(Disass_Str, "OUT\t(C),D\n");
		break;
	case 0x52:
		strcat(Disass_Str, "SBC\tHL,DE\n");
		break;
	case 0x53:
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD\t(%04X),DE\n", i);
		len = 4;
		break;
	case 0x55:				/* undocumented */
	case 0x65:				/* undocumented */
	case 0x75:				/* undocumented */
		strcat(Disass_Str, "RETN*\n");
		break;
	case 0x5d:				/* undocumented */
	case 0x6d:				/* undocumented */
	case 0x7d:				/* undocumented */
		strcat(Disass_Str, "RETI*\n");
		break;
	case 0x56:
		strcat(Disass_Str, "IM\t1\n");
		break;
	case 0x57:
		strcat(Disass_Str, "LD\tA,I\n");
		break;
	case 0x58:
		strcat(Disass_Str, "IN\tE,(C)\n");
		break;
	case 0x59:
		strcat(Disass_Str, "OUT\t(C),E\n");
		break;
	case 0x5a:
		strcat(Disass_Str, "ADC\tHL,DE\n");
		break;
	case 0x5b:
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD\tDE,(%04X)\n", i);
		len = 4;
		break;
	case 0x5e:
		strcat(Disass_Str, "IM\t2\n");
		break;
	case 0x5f:
		strcat(Disass_Str, "LD\tA,R\n");
		break;
	case 0x60:
		strcat(Disass_Str, "IN\tH,(C)\n");
		break;
	case 0x61:
		strcat(Disass_Str, "OUT\t(C),H\n");
		break;
	case 0x62:
		strcat(Disass_Str, "SBC\tHL,HL\n");
		break;
	case 0x63:				/* undocumented */
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD*\t(%04X),HL\n", i);
		len = 4;
		break;
	case 0x67:
		strcat(Disass_Str, "RRD\n");
		break;
	case 0x68:
		strcat(Disass_Str, "IN\tL,(C)\n");
		break;
	case 0x69:
		strcat(Disass_Str, "OUT\t(C),L\n");
		break;
	case 0x6a:
		strcat(Disass_Str, "ADC\tHL,HL\n");
		break;
	case 0x6b:				/* undocumented */
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD*\tHL,(%04X)\n", i);
		len = 4;
		break;
	case 0x6f:
		strcat(Disass_Str, "RLD\n");
		break;
	case 0x70:				/* undocumented */
		strcat(Disass_Str, "IN*\tF,(C)\n");
		break;
	case 0x71:				/* undocumented */
		strcat(Disass_Str, "OUT*\t(C),0\n");
		break;
	case 0x72:
		strcat(Disass_Str, "SBC\tHL,SP\n");
		break;
	case 0x73:
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD\t(%04X),SP\n", i);
		len = 4;
		break;
	case 0x76:				/* undocumented */
		strcat(Disass_Str, "IM*\t1\n");
		break;
	case 0x78:
		strcat(Disass_Str, "IN\tA,(C)\n");
		break;
	case 0x79:
		strcat(Disass_Str, "OUT\t(C),A\n");
		break;
	case 0x7a:
		strcat(Disass_Str, "ADC\tHL,SP\n");
		break;
	case 0x7b:
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD\tSP,(%04X)\n", i);
		len = 4;
		break;
	case 0x7e:				/* undocumented */
		strcat(Disass_Str, "IM*\t2\n");
		break;
	case 0xa0:
		strcat(Disass_Str, "LDI\n");
		break;
	case 0xa1:
		strcat(Disass_Str, "CPI\n");
		break;
	case 0xa2:
		strcat(Disass_Str, "INI\n");
		break;
	case 0xa3:
		strcat(Disass_Str, "OUTI\n");
		break;
	case 0xa8:
		strcat(Disass_Str, "LDD\n");
		break;
	case 0xa9:
		strcat(Disass_Str, "CPD\n");
		break;
	case 0xaa:
		strcat(Disass_Str, "IND\n");
		break;
	case 0xab:
		strcat(Disass_Str, "OUTD\n");
		break;
	case 0xb0:
		strcat(Disass_Str, "LDIR\n");
		break;
	case 0xb1:
		strcat(Disass_Str, "CPIR\n");
		break;
	case 0xb2:
		strcat(Disass_Str, "INIR\n");
		break;
	case 0xb3:
		strcat(Disass_Str, "OTIR\n");
		break;
	case 0xb8:
		strcat(Disass_Str, "LDDR\n");
		break;
	case 0xb9:
		strcat(Disass_Str, "CPDR\n");
		break;
	case 0xba:
		strcat(Disass_Str, "INDR\n");
		break;
	case 0xbb:
		strcat(Disass_Str, "OTDR\n");
		break;
	default:				/* undocumented */
		strcat(Disass_Str, "NOP*\n");
	}
	return(len);
}

/*
 *	disassemble multi byte op-codes with prefix 0xdd and 0xfd
 */
static int ddfd(char *s, unsigned char **p)
{
	register int b2, b4;
	register char *ireg;
	int len = 3;

	s = s;	/* to avoid compiler warning */

	if (**p == 0xdd)
		ireg = regix;
	else
		ireg = regiy;
	b2 = *(*p + 1);
	if (b2 >= 0x70 && b2 <= 0x77 && b2 != 0x76) {
		sprintf(Disass_Str, "LD\t(%s+%02X),%s\n", ireg, *(*p + 2),
			reg[b2 & 7]);
		return(3);
	}
	switch (b2) {
	case 0x09:
		sprintf(Disass_Str, "ADD\t%s,BC\n", ireg);
		len = 2;
		break;
	case 0x19:
		sprintf(Disass_Str, "ADD\t%s,DE\n", ireg);
		len = 2;
		break;
	case 0x21:
		sprintf(Disass_Str, "LD\t%s,%04X\n", ireg, *(*p + 2) + (*(*p + 3) << 8));
		len = 4;
		break;
	case 0x22:
		sprintf(Disass_Str, "LD\t(%04X),%s\n", *(*p + 2) + (*(*p + 3) << 8), ireg);
		len = 4;
		break;
	case 0x23:
		sprintf(Disass_Str, "INC\t%s\n", ireg);
		len = 2;
		break;
	case 0x24:				/* undocumented */
		sprintf(Disass_Str, "INC*\t%sH\n", ireg);
		len = 2;
		break;
	case 0x25:				/* undocumented */
		sprintf(Disass_Str, "DEC*\t%sH\n", ireg);
		len = 2;
		break;
	case 0x26:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,%02X\n", ireg, *(*p + 2));
		break;
	case 0x29:
		if (**p == 0xdd)
			sprintf(Disass_Str, "ADD\tIX,IX\n");
		else
			sprintf(Disass_Str, "ADD\tIY,IY\n");
		len = 2;
		break;
	case 0x2a:
		sprintf(Disass_Str, "LD\t%s,(%04X)\n", ireg, *(*p + 2) + (*(*p + 3) << 8));
		len = 4;
		break;
	case 0x2b:
		sprintf(Disass_Str, "DEC\t%s\n", ireg);
		len = 2;
		break;
	case 0x2c:				/* undocumented */
		sprintf(Disass_Str, "INC*\t%sL\n", ireg);
		len = 2;
		break;
	case 0x2d:				/* undocumented */
		sprintf(Disass_Str, "DEC*\t%sL\n", ireg);
		len = 2;
		break;
	case 0x2e:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,%02X\n", ireg, *(*p + 2));
		break;
	case 0x34:
		sprintf(Disass_Str, "INC\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x35:
		sprintf(Disass_Str, "DEC\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x36:
		sprintf(Disass_Str, "LD\t(%s+%02X),%02X\n", ireg, *(*p + 2), *(*p + 3));
		len = 4;
		break;
	case 0x39:
		sprintf(Disass_Str, "ADD\t%s,SP\n", ireg);
		len = 2;
		break;
	case 0x44:				/* undocumented */
		sprintf(Disass_Str, "LD*\tB,%sH\n", ireg);
		len = 2;
		break;
	case 0x45:				/* undocumented */
		sprintf(Disass_Str, "LD*\tB,%sL\n", ireg);
		len = 2;
		break;
	case 0x46:
		sprintf(Disass_Str, "LD\tB,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x4c:				/* undocumented */
		sprintf(Disass_Str, "LD*\tC,%sH\n", ireg);
		len = 2;
		break;
	case 0x4d:				/* undocumented */
		sprintf(Disass_Str, "LD*\tC,%sL\n", ireg);
		len = 2;
		break;
	case 0x4e:
		sprintf(Disass_Str, "LD\tC,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x54:				/* undocumented */
		sprintf(Disass_Str, "LD*\tD,%sH\n", ireg);
		len = 2;
		break;
	case 0x55:				/* undocumented */
		sprintf(Disass_Str, "LD*\tD,%sL\n", ireg);
		len = 2;
		break;
	case 0x56:
		sprintf(Disass_Str, "LD\tD,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x5c:				/* undocumented */
		sprintf(Disass_Str, "LD*\tE,%sH\n", ireg);
		len = 2;
		break;
	case 0x5d:				/* undocumented */
		sprintf(Disass_Str, "LD*\tE,%sL\n", ireg);
		len = 2;
		break;
	case 0x5e:
		sprintf(Disass_Str, "LD\tE,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x60:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,B\n", ireg);
		len = 2;
		break;
	case 0x61:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,C\n", ireg);
		len = 2;
		break;
	case 0x62:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,D\n", ireg);
		len = 2;
		break;
	case 0x63:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,E\n", ireg);
		len = 2;
		break;
	case 0x64:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,%sH\n", ireg, ireg);
		len = 2;
		break;
	case 0x65:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,%sL\n", ireg, ireg);
		len = 2;
		break;
	case 0x66:
		sprintf(Disass_Str, "LD\tH,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x67:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sH,A\n", ireg);
		len = 2;
		break;
	case 0x68:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,B\n", ireg);
		len = 2;
		break;
	case 0x69:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,C\n", ireg);
		len = 2;
		break;
	case 0x6a:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,D\n", ireg);
		len = 2;
		break;
	case 0x6b:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,E\n", ireg);
		len = 2;
		break;
	case 0x6c:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,%sH\n", ireg, ireg);
		len = 2;
		break;
	case 0x6d:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,%sL\n", ireg, ireg);
		len = 2;
		break;
	case 0x6e:
		sprintf(Disass_Str, "LD\tL,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x6f:				/* undocumented */
		sprintf(Disass_Str, "LD*\t%sL,A\n", ireg);
		len = 2;
		break;
	case 0x7c:				/* undocumented */
		sprintf(Disass_Str, "LD*\tA,%sH\n", ireg);
		len = 2;
		break;
	case 0x7d:				/* undocumented */
		sprintf(Disass_Str, "LD*\tA,%sL\n", ireg);
		len = 2;
		break;
	case 0x7e:
		sprintf(Disass_Str, "LD\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x84:				/* undocumented */
		sprintf(Disass_Str, "ADD*\tA,%sH\n", ireg);
		len = 2;
		break;
	case 0x85:				/* undocumented */
		sprintf(Disass_Str, "ADD*\tA,%sL\n", ireg);
		len = 2;
		break;
	case 0x86:
		sprintf(Disass_Str, "ADD\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x8c:				/* undocumented */
		sprintf(Disass_Str, "ADC*\tA,%sH\n", ireg);
		len = 2;
		break;
	case 0x8d:				/* undocumented */
		sprintf(Disass_Str, "ADC*\tA,%sL\n", ireg);
		len = 2;
		break;
	case 0x8e:
		sprintf(Disass_Str, "ADC\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x94:				/* undocumented */
		sprintf(Disass_Str, "SUB*\t%sH\n", ireg);
		len = 2;
		break;
	case 0x95:				/* undocumented */
		sprintf(Disass_Str, "SUB*\t%sL\n", ireg);
		len = 2;
		break;
	case 0x96:
		sprintf(Disass_Str, "SUB\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x9c:				/* undocumented */
		sprintf(Disass_Str, "SBC*\tA,%sH\n", ireg);
		len = 2;
		break;
	case 0x9d:				/* undocumented */
		sprintf(Disass_Str, "SBC*\tA,%sL\n", ireg);
		len = 2;
		break;
	case 0x9e:
		sprintf(Disass_Str, "SBC\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xa4:				/* undocumented */
		sprintf(Disass_Str, "AND*\t%sH\n", ireg);
		len = 2;
		break;
	case 0xa5:				/* undocumented */
		sprintf(Disass_Str, "AND*\t%sL\n", ireg);
		len = 2;
		break;
	case 0xa6:
		sprintf(Disass_Str, "AND\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xac:				/* undocumented */
		sprintf(Disass_Str, "XOR*\t%sH\n", ireg);
		len = 2;
		break;
	case 0xad:				/* undocumented */
		sprintf(Disass_Str, "XOR*\t%sL\n", ireg);
		len = 2;
		break;
	case 0xae:
		sprintf(Disass_Str, "XOR\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xb4:				/* undocumented */
		sprintf(Disass_Str, "OR*\t%sH\n", ireg);
		len = 2;
		break;
	case 0xb5:				/* undocumented */
		sprintf(Disass_Str, "OR*\t%sL\n", ireg);
		len = 2;
		break;
	case 0xb6:
		sprintf(Disass_Str, "OR\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xbc:				/* undocumented */
		sprintf(Disass_Str, "CP*\t%sH\n", ireg);
		len = 2;
		break;
	case 0xbd:				/* undocumented */
		sprintf(Disass_Str, "CP*\t%sL\n", ireg);
		len = 2;
		break;
	case 0xbe:
		sprintf(Disass_Str, "CP\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xcb:
		b4 = *(*p + 3);
		switch (b4) {
		case 0x06:
			sprintf(Disass_Str, "RLC\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x0e:
			sprintf(Disass_Str, "RRC\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x16:
			sprintf(Disass_Str, "RL\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x1e:
			sprintf(Disass_Str, "RR\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x26:
			sprintf(Disass_Str, "SLA\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x2e:
			sprintf(Disass_Str, "SRA\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x36:			/* undocumented */
			sprintf(Disass_Str, "SLL*\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x3e:
			sprintf(Disass_Str, "SRL\t(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x46:
			sprintf(Disass_Str, "BIT\t0,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x4e:
			sprintf(Disass_Str, "BIT\t1,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x56:
			sprintf(Disass_Str, "BIT\t2,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x5e:
			sprintf(Disass_Str, "BIT\t3,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x66:
			sprintf(Disass_Str, "BIT\t4,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x6e:
			sprintf(Disass_Str, "BIT\t5,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x76:
			sprintf(Disass_Str, "BIT\t6,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x7e:
			sprintf(Disass_Str, "BIT\t7,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x86:
			sprintf(Disass_Str, "RES\t0,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x8e:
			sprintf(Disass_Str, "RES\t1,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x96:
			sprintf(Disass_Str, "RES\t2,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0x9e:
			sprintf(Disass_Str, "RES\t3,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xa6:
			sprintf(Disass_Str, "RES\t4,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xae:
			sprintf(Disass_Str, "RES\t5,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xb6:
			sprintf(Disass_Str, "RES\t6,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xbe:
			sprintf(Disass_Str, "RES\t7,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xc6:
			sprintf(Disass_Str, "SET\t0,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xce:
			sprintf(Disass_Str, "SET\t1,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xd6:
			sprintf(Disass_Str, "SET\t2,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xde:
			sprintf(Disass_Str, "SET\t3,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xe6:
			sprintf(Disass_Str, "SET\t4,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xee:
			sprintf(Disass_Str, "SET\t5,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xf6:
			sprintf(Disass_Str, "SET\t6,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		case 0xfe:
			sprintf(Disass_Str, "SET\t7,(%s+%02X)\n", ireg, *(*p + 2));
			break;
		default:			/* undocumented */
			if (b4 >= 0x00 && b4 <= 0x07) {
				sprintf(Disass_Str, "RLC*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x08 && b4 <= 0x0f) {
				sprintf(Disass_Str, "RRC*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x10 && b4 <= 0x17) {
				sprintf(Disass_Str, "RL*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x18 && b4 <= 0x1f) {
				sprintf(Disass_Str, "RR*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x20 && b4 <= 0x27) {
				sprintf(Disass_Str, "SLA*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x28 && b4 <= 0x2f) {
				sprintf(Disass_Str, "SRA*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x30 && b4 <= 0x37) {
				sprintf(Disass_Str, "SLL*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x38 && b4 <= 0x3f) {
				sprintf(Disass_Str, "SRL*\t(%s+%02X),%s\n",
					ireg, *(*p + 2), reg[b4 & 7]);
			} else if (b4 >= 0x40 && b4 <= 0x7f) {
				sprintf(Disass_Str, "BIT*\t%c,(%s+%02X)\n",
					((b4 >> 3) & 7) + '0', ireg, *(*p + 2));
			} else if (b4 >= 0x80 && b4 <= 0xbf) {
				sprintf(Disass_Str, "RES*\t%c,(%s+%02X),%s\n",
					((b4 >> 3) & 7) + '0', ireg, *(*p + 2),
					reg[b4 & 7]);
			} else if (b4 >= 0xc0) {
				sprintf(Disass_Str, "SET*\t%c,(%s+%02X),%s\n",
					((b4 >> 3) & 7) + '0', ireg, *(*p + 2),
					reg[b4 & 7]);
			}
		}
		len = 4;
		break;
	case 0xe1:
		sprintf(Disass_Str, "POP\t%s\n", ireg);
		len = 2;
		break;
	case 0xe3:
		sprintf(Disass_Str, "EX\t(SP),%s\n", ireg);
		len = 2;
		break;
	case 0xe5:
		sprintf(Disass_Str, "PUSH\t%s\n", ireg);
		len = 2;
		break;
	case 0xe9:
		sprintf(Disass_Str, "JP\t(%s)\n", ireg);
		len = 2;
		break;
	case 0xf9:
		sprintf(Disass_Str, "LD\tSP,%s\n", ireg);
		len = 2;
		break;
	default:
		Disass_Str[0] = 0;
		strcat(Disass_Str, "NOP*\n");
		len = 1;
	}
	return(len);
}

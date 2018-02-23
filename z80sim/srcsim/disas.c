/*
 * Z80 disassembler for Z80-CPU simulator
 *
 * Copyright (C) 1989-2017 by Udo Munk
 * Parts Copyright (C) 2008 by Justin Clancy
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

static struct opt optab[256] = {
	{ opout,  "NOP"			},	/* 0x00	*/
	{ nnout,  "LD\tBC,"		},	/* 0x01	*/
	{ opout,  "LD\t(BC),A"		},	/* 0x02	*/
	{ opout,  "INC\tBC"		},	/* 0x03	*/
	{ opout,  "INC\tB"		},	/* 0x04	*/
	{ opout,  "DEC\tB"		},	/* 0x05	*/
	{ nout,	  "LD\tB,"		},	/* 0x06	*/
	{ opout,  "RLCA"		},	/* 0x07	*/
	{ opout,  "EX\tAF,AF'"		},	/* 0x08	*/
	{ opout,  "ADD\tHL,BC"		},	/* 0x09	*/
	{ opout,  "LD\tA,(BC)"		},	/* 0x0a */
	{ opout,  "DEC\tBC"		},	/* 0x0b */
	{ opout,  "INC\tC"		},	/* 0x0c */
	{ opout,  "DEC\tC"		},	/* 0x0d */
	{ nout,	  "LD\tC,"		},	/* 0x0e */
	{ opout,  "RRCA"		},	/* 0x0f */
	{ rout,	  "DJNZ\t"		},	/* 0x10	*/
	{ nnout,  "LD\tDE,"		},	/* 0x11	*/
	{ opout,  "LD\t(DE),A"		},	/* 0x12	*/
	{ opout,  "INC\tDE"		},	/* 0x13	*/
	{ opout,  "INC\tD"		},	/* 0x14	*/
	{ opout,  "DEC\tD"		},	/* 0x15	*/
	{ nout,	  "LD\tD,"		},	/* 0x16	*/
	{ opout,  "RLA"			},	/* 0x17	*/
	{ rout,	  "JR\t"		},	/* 0x18	*/
	{ opout,  "ADD\tHL,DE"		},	/* 0x19	*/
	{ opout,  "LD\tA,(DE)"		},	/* 0x1a */
	{ opout,  "DEC\tDE"		},	/* 0x1b */
	{ opout,  "INC\tE"		},	/* 0x1c */
	{ opout,  "DEC\tE"		},	/* 0x1d */
	{ nout,	  "LD\tE,"		},	/* 0x1e */
	{ opout,  "RRA"			},	/* 0x1f */
	{ rout,	  "JR\tNZ,"		},	/* 0x20	*/
	{ nnout,  "LD\tHL,"		},	/* 0x21	*/
	{ inout,  "LD\t(%04X),HL"	},	/* 0x22	*/
	{ opout,  "INC\tHL"		},	/* 0x23	*/
	{ opout,  "INC\tH"		},	/* 0x24	*/
	{ opout,  "DEC\tH"		},	/* 0x25	*/
	{ nout,	  "LD\tH,"		},	/* 0x26	*/
	{ opout,  "DAA"			},	/* 0x27	*/
	{ rout,	  "JR\tZ,"		},	/* 0x28	*/
	{ opout,  "ADD\tHL,HL"		},	/* 0x29	*/
	{ inout,  "LD\tHL,(%04X)"	},	/* 0x2a */
	{ opout,  "DEC\tHL"		},	/* 0x2b */
	{ opout,  "INC\tL"		},	/* 0x2c */
	{ opout,  "DEC\tL"		},	/* 0x2d */
	{ nout,	  "LD\tL,"		},	/* 0x2e */
	{ opout,  "CPL"			},	/* 0x2f */
	{ rout,	  "JR\tNC,"		},	/* 0x30	*/
	{ nnout,  "LD\tSP,"		},	/* 0x31	*/
	{ inout,  "LD\t(%04X),A"	},	/* 0x32	*/
	{ opout,  "INC\tSP"		},	/* 0x33	*/
	{ opout,  "INC\t(HL)"		},	/* 0x34	*/
	{ opout,  "DEC\t(HL)"		},	/* 0x35	*/
	{ nout,	  "LD\t(HL),"		},	/* 0x36	*/
	{ opout,  "SCF"			},	/* 0x37	*/
	{ rout,	  "JR\tC,"		},	/* 0x38	*/
	{ opout,  "ADD\tHL,SP"		},	/* 0x39	*/
	{ inout,  "LD\tA,(%04X)"	},	/* 0x3a */
	{ opout,  "DEC\tSP"		},	/* 0x3b */
	{ opout,  "INC\tA"		},	/* 0x3c */
	{ opout,  "DEC\tA"		},	/* 0x3d */
	{ nout,	  "LD\tA,"		},	/* 0x3e */
	{ opout,  "CCF"			},	/* 0x3f */
	{ opout,  "LD\tB,B"		},	/* 0x40	*/
	{ opout,  "LD\tB,C"		},	/* 0x41	*/
	{ opout,  "LD\tB,D"		},	/* 0x42	*/
	{ opout,  "LD\tB,E"		},	/* 0x43	*/
	{ opout,  "LD\tB,H"		},	/* 0x44	*/
	{ opout,  "LD\tB,L"		},	/* 0x45	*/
	{ opout,  "LD\tB,(HL)"		},	/* 0x46	*/
	{ opout,  "LD\tB,A"		},	/* 0x47	*/
	{ opout,  "LD\tC,B"		},	/* 0x48	*/
	{ opout,  "LD\tC,C"		},	/* 0x49	*/
	{ opout,  "LD\tC,D"		},	/* 0x4a */
	{ opout,  "LD\tC,E"		},	/* 0x4b */
	{ opout,  "LD\tC,H"		},	/* 0x4c */
	{ opout,  "LD\tC,L"		},	/* 0x4d */
	{ opout,  "LD\tC,(HL)"		},	/* 0x4e */
	{ opout,  "LD\tC,A"		},	/* 0x4f */
	{ opout,  "LD\tD,B"		},	/* 0x50	*/
	{ opout,  "LD\tD,C"		},	/* 0x51	*/
	{ opout,  "LD\tD,D"		},	/* 0x52	*/
	{ opout,  "LD\tD,E"		},	/* 0x53	*/
	{ opout,  "LD\tD,H"		},	/* 0x54	*/
	{ opout,  "LD\tD,L"		},	/* 0x55	*/
	{ opout,  "LD\tD,(HL)"		},	/* 0x56	*/
	{ opout,  "LD\tD,A"		},	/* 0x57	*/
	{ opout,  "LD\tE,B"		},	/* 0x58	*/
	{ opout,  "LD\tE,C"		},	/* 0x59	*/
	{ opout,  "LD\tE,D"		},	/* 0x5a */
	{ opout,  "LD\tE,E"		},	/* 0x5b */
	{ opout,  "LD\tE,H"		},	/* 0x5c */
	{ opout,  "LD\tE,L"		},	/* 0x5d */
	{ opout,  "LD\tE,(HL)"		},	/* 0x5e */
	{ opout,  "LD\tE,A"		},	/* 0x5f */
	{ opout,  "LD\tH,B"		},	/* 0x60	*/
	{ opout,  "LD\tH,C"		},	/* 0x61	*/
	{ opout,  "LD\tH,D"		},	/* 0x62	*/
	{ opout,  "LD\tH,E"		},	/* 0x63	*/
	{ opout,  "LD\tH,H"		},	/* 0x64	*/
	{ opout,  "LD\tH,L"		},	/* 0x65	*/
	{ opout,  "LD\tH,(HL)"		},	/* 0x66	*/
	{ opout,  "LD\tH,A"		},	/* 0x67	*/
	{ opout,  "LD\tL,B"		},	/* 0x68	*/
	{ opout,  "LD\tL,C"		},	/* 0x69	*/
	{ opout,  "LD\tL,D"		},	/* 0x6a */
	{ opout,  "LD\tL,E"		},	/* 0x6b */
	{ opout,  "LD\tL,H"		},	/* 0x6c */
	{ opout,  "LD\tL,L"		},	/* 0x6d */
	{ opout,  "LD\tL,(HL)"		},	/* 0x6e */
	{ opout,  "LD\tL,A"		},	/* 0x6f */
	{ opout,  "LD\t(HL),B"		},	/* 0x70	*/
	{ opout,  "LD\t(HL),C"		},	/* 0x71	*/
	{ opout,  "LD\t(HL),D"		},	/* 0x72	*/
	{ opout,  "LD\t(HL),E"		},	/* 0x73	*/
	{ opout,  "LD\t(HL),H"		},	/* 0x74	*/
	{ opout,  "LD\t(HL),L"		},	/* 0x75	*/
	{ opout,  "HALT"		},	/* 0x76	*/
	{ opout,  "LD\t(HL),A"		},	/* 0x77	*/
	{ opout,  "LD\tA,B"		},	/* 0x78	*/
	{ opout,  "LD\tA,C"		},	/* 0x79	*/
	{ opout,  "LD\tA,D"		},	/* 0x7a */
	{ opout,  "LD\tA,E"		},	/* 0x7b */
	{ opout,  "LD\tA,H"		},	/* 0x7c */
	{ opout,  "LD\tA,L"		},	/* 0x7d */
	{ opout,  "LD\tA,(HL)"		},	/* 0x7e */
	{ opout,  "LD\tA,A"		},	/* 0x7f */
	{ opout,  "ADD\tA,B"		},	/* 0x80	*/
	{ opout,  "ADD\tA,C"		},	/* 0x81	*/
	{ opout,  "ADD\tA,D"		},	/* 0x82	*/
	{ opout,  "ADD\tA,E"		},	/* 0x83	*/
	{ opout,  "ADD\tA,H"		},	/* 0x84	*/
	{ opout,  "ADD\tA,L"		},	/* 0x85	*/
	{ opout,  "ADD\tA,(HL)"		},	/* 0x86	*/
	{ opout,  "ADD\tA,A"		},	/* 0x87	*/
	{ opout,  "ADC\tA,B"		},	/* 0x88	*/
	{ opout,  "ADC\tA,C"		},	/* 0x89	*/
	{ opout,  "ADC\tA,D"		},	/* 0x8a */
	{ opout,  "ADC\tA,E"		},	/* 0x8b */
	{ opout,  "ADC\tA,H"		},	/* 0x8c */
	{ opout,  "ADC\tA,L"		},	/* 0x8d */
	{ opout,  "ADC\tA,(HL)"		},	/* 0x8e */
	{ opout,  "ADC\tA,A"		},	/* 0x8f */
	{ opout,  "SUB\tB"		},	/* 0x90	*/
	{ opout,  "SUB\tC"		},	/* 0x91	*/
	{ opout,  "SUB\tD"		},	/* 0x92	*/
	{ opout,  "SUB\tE"		},	/* 0x93	*/
	{ opout,  "SUB\tH"		},	/* 0x94	*/
	{ opout,  "SUB\tL"		},	/* 0x95	*/
	{ opout,  "SUB\t(HL)"		},	/* 0x96	*/
	{ opout,  "SUB\tA"		},	/* 0x97	*/
	{ opout,  "SBC\tA,B"		},	/* 0x98	*/
	{ opout,  "SBC\tA,C"		},	/* 0x99	*/
	{ opout,  "SBC\tA,D"		},	/* 0x9a */
	{ opout,  "SBC\tA,E"		},	/* 0x9b */
	{ opout,  "SBC\tA,H"		},	/* 0x9c */
	{ opout,  "SBC\tA,L"		},	/* 0x9d */
	{ opout,  "SBC\tA,(HL)"		},	/* 0x9e */
	{ opout,  "SBC\tA,A"		},	/* 0x9f */
	{ opout,  "AND\tB"		},	/* 0xa0	*/
	{ opout,  "AND\tC"		},	/* 0xa1	*/
	{ opout,  "AND\tD"		},	/* 0xa2	*/
	{ opout,  "AND\tE"		},	/* 0xa3	*/
	{ opout,  "AND\tH"		},	/* 0xa4	*/
	{ opout,  "AND\tL"		},	/* 0xa5	*/
	{ opout,  "AND\t(HL)"		},	/* 0xa6	*/
	{ opout,  "AND\tA"		},	/* 0xa7	*/
	{ opout,  "XOR\tB"		},	/* 0xa8	*/
	{ opout,  "XOR\tC"		},	/* 0xa9	*/
	{ opout,  "XOR\tD"		},	/* 0xaa */
	{ opout,  "XOR\tE"		},	/* 0xab */
	{ opout,  "XOR\tH"		},	/* 0xac */
	{ opout,  "XOR\tL"		},	/* 0xad */
	{ opout,  "XOR\t(HL)"		},	/* 0xae */
	{ opout,  "XOR\tA"		},	/* 0xaf */
	{ opout,  "OR\tB"		},	/* 0xb0	*/
	{ opout,  "OR\tC"		},	/* 0xb1	*/
	{ opout,  "OR\tD"		},	/* 0xb2	*/
	{ opout,  "OR\tE"		},	/* 0xb3	*/
	{ opout,  "OR\tH"		},	/* 0xb4	*/
	{ opout,  "OR\tL"		},	/* 0xb5	*/
	{ opout,  "OR\t(HL)"		},	/* 0xb6	*/
	{ opout,  "OR\tA"		},	/* 0xb7	*/
	{ opout,  "CP\tB"		},	/* 0xb8	*/
	{ opout,  "CP\tC"		},	/* 0xb9	*/
	{ opout,  "CP\tD"		},	/* 0xba */
	{ opout,  "CP\tE"		},	/* 0xbb */
	{ opout,  "CP\tH"		},	/* 0xbc */
	{ opout,  "CP\tL"		},	/* 0xbd */
	{ opout,  "CP\t(HL)"		},	/* 0xbe */
	{ opout,  "CP\tA"		},	/* 0xbf */
	{ opout,  "RET\tNZ"		},	/* 0xc0	*/
	{ opout,  "POP\tBC"		},	/* 0xc1	*/
	{ nnout,  "JP\tNZ,"		},	/* 0xc2	*/
	{ nnout,  "JP\t"		},	/* 0xc3	*/
	{ nnout,  "CALL\tNZ,"		},	/* 0xc4	*/
	{ opout,  "PUSH\tBC"		},	/* 0xc5	*/
	{ nout,	  "ADD\tA,"		},	/* 0xc6	*/
	{ opout,  "RST\t0"		},	/* 0xc7	*/
	{ opout,  "RET\tZ"		},	/* 0xc8	*/
	{ opout,  "RET"			},	/* 0xc9	*/
	{ nnout,  "JP\tZ,"		},	/* 0xca */
	{ cbop,	  ""			},	/* 0xcb */
	{ nnout,  "CALL\tZ,"		},	/* 0xcc */
	{ nnout,  "CALL\t"		},	/* 0xcd */
	{ nout,	  "ADC\tA,"		},	/* 0xce */
	{ opout,  "RST\t8"		},	/* 0xcf */
	{ opout,  "RET\tNC"		},	/* 0xd0	*/
	{ opout,  "POP\tDE"		},	/* 0xd1	*/
	{ nnout,  "JP\tNC,"		},	/* 0xd2	*/
	{ iout,	  "OUT\t(%02X),A"	},	/* 0xd3	*/
	{ nnout,  "CALL\tNC,"		},	/* 0xd4	*/
	{ opout,  "PUSH\tDE"		},	/* 0xd5	*/
	{ nout,	  "SUB\t"		},	/* 0xd6	*/
	{ opout,  "RST\t10"		},	/* 0xd7	*/
	{ opout,  "RET\tC"		},	/* 0xd8	*/
	{ opout,  "EXX"			},	/* 0xd9	*/
	{ nnout,  "JP\tC,"		},	/* 0xda */
	{ iout,	  "IN\tA,(%02X)"	},	/* 0xdb */
	{ nnout,  "CALL\tC,"		},	/* 0xdc */
	{ ddfd,	  ""			},	/* 0xdd */
	{ nout,	  "SBC\tA,"		},	/* 0xde */
	{ opout,  "RST\t18"		},	/* 0xdf */
	{ opout,  "RET\tPO"		},	/* 0xe0	*/
	{ opout,  "POP\tHL"		},	/* 0xe1	*/
	{ nnout,  "JP\tPO,"		},	/* 0xe2	*/
	{ opout,  "EX\t(SP),HL"		},	/* 0xe3	*/
	{ nnout,  "CALL\tPO,"		},	/* 0xe4	*/
	{ opout,  "PUSH\tHL"		},	/* 0xe5	*/
	{ nout,	  "AND\t"		},	/* 0xe6	*/
	{ opout,  "RST\t20"		},	/* 0xe7	*/
	{ opout,  "RET\tPE"		},	/* 0xe8	*/
	{ opout,  "JP\t(HL)"		},	/* 0xe9	*/
	{ nnout,  "JP\tPE,"		},	/* 0xea */
	{ opout,  "EX\tDE,HL"		},	/* 0xeb */
	{ nnout,  "CALL\tPE,"		},	/* 0xec */
	{ edop,	  ""			},	/* 0xed */
	{ nout,	  "XOR\t"		},	/* 0xee */
	{ opout,  "RST\t28"		},	/* 0xef */
	{ opout,  "RET\tP"		},	/* 0xf0	*/
	{ opout,  "POP\tAF"		},	/* 0xf1	*/
	{ nnout,  "JP\tP,"		},	/* 0xf2	*/
	{ opout,  "DI"			},	/* 0xf3	*/
	{ nnout,  "CALL\tP,"		},	/* 0xf4	*/
	{ opout,  "PUSH\tAF"		},	/* 0xf5	*/
	{ nout,	  "OR\t"		},	/* 0xf6	*/
	{ opout,  "RST\t30"		},	/* 0xf7	*/
	{ opout,  "RET\tM"		},	/* 0xf8	*/
	{ opout,  "LD\tSP,HL"		},	/* 0xf9	*/
	{ nnout,  "JP\tM,"		},	/* 0xfa */
	{ opout,  "EI"			},	/* 0xfb */
	{ nnout,  "CALL\tM,"		},	/* 0xfc */
	{ ddfd,	  ""			},	/* 0xfd */
	{ nout,	  "CP\t"		},	/* 0xfe */
	{ opout,  "RST\t38"		}	/* 0xff */
};

static int addr;
static char *unkown = "???";
static char *reg[] = { "B", "C", "D", "E", "H", "L", "(HL)", "A" };
static char *regix = "IX";
static char *regiy = "IY";

/* globals for passing disassembled code to anyone else who's interested */

char Disass_Str[64];
char Opcode_Str[64];

#ifdef WANT_GUI

/* Set up machine code hex in Opcode_Str for GUI disassembly */

void get_opcodes(unsigned char **p, int len)
{
  switch (len)
  {
    case 1:
      sprintf(Opcode_Str, "%02X         ",
                (**p & 0xff));
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
#endif

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
 */
void disass(unsigned char **p, int adr)
{
	register int len;

	addr = adr;
	len = (*optab[**p].fun)	(optab[**p].text, p);
#ifndef WANT_GUI
	printf(Disass_Str);
#endif
#ifdef WANT_GUI
        get_opcodes(p, len);
#endif
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
	sprintf(Disass_Str, "%s%04X\n", s, addr + *(*p + 1) + 2);
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
	if (b2 >= 0x38 && b2 <= 0x3f) {
		sprintf(Disass_Str, "SRL\t%s\n",
			reg[b2 & 7]);
		return(2);
	}
	if (b2 >= 0x40 && b2 <= 0x7f) {
		sprintf(Disass_Str, "BIT\t%c,%s\n",
			((b2 >> 3) & 7) + '0', reg[b2 &	7]);
		return(2);
	}
	if (b2 >= 0x80 && b2 <= 0xbf) {
		sprintf(Disass_Str, "RES\t%c,%s\n",
			((b2 >> 3) & 7) + '0', reg[b2 &	7]);
		return(2);
	}
	if (b2 >= 0xc0)	{
		sprintf(Disass_Str, "SET\t%c,%s\n",
			((b2 >> 3) & 7) + '0', reg[b2 &	7]);
		return(2);
	}
	strcat(Disass_Str, unkown);
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
	case 0x4d:
		strcat(Disass_Str, "RETI\n");
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
	case 0x6f:
		strcat(Disass_Str, "RLD\n");
		break;
	case 0x72:
		strcat(Disass_Str, "SBC\tHL,SP\n");
		break;
	case 0x73:
		i = *(*p + 2) + (*(*p + 3) << 8);
		sprintf(Disass_Str, "LD\t(%04X),SP\n", i);
		len = 4;
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
	default:
		strcat(Disass_Str, unkown);
	}
	return(len);
}

/*
 *	disassemble multi byte op-codes with prefix 0xdd and 0xfd
 */
static int ddfd(char *s, unsigned char **p)
{
	register int b2;
	register char *ireg;
	int len = 3;

	s = s;	/* to avoid compiler warning */

	if (**p == 0xdd)
		ireg = regix;
	else
		ireg = regiy;
	b2 = *(*p + 1);
	if (b2 >= 0x70 && b2 <= 0x77) {
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
	case 0x46:
		sprintf(Disass_Str, "LD\tB,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x4e:
		sprintf(Disass_Str, "LD\tC,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x56:
		sprintf(Disass_Str, "LD\tD,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x5e:
		sprintf(Disass_Str, "LD\tE,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x66:
		sprintf(Disass_Str, "LD\tH,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x6e:
		sprintf(Disass_Str, "LD\tL,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x7e:
		sprintf(Disass_Str, "LD\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x86:
		sprintf(Disass_Str, "ADD\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x8e:
		sprintf(Disass_Str, "ADC\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x96:
		sprintf(Disass_Str, "SUB\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0x9e:
		sprintf(Disass_Str, "SBC\tA,(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xa6:
		sprintf(Disass_Str, "AND\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xae:
		sprintf(Disass_Str, "XOR\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xb6:
		sprintf(Disass_Str, "OR\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xbe:
		sprintf(Disass_Str, "CP\t(%s+%02X)\n", ireg, *(*p + 2));
		break;
	case 0xcb:
		switch (*(*p + 3)) {
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
		default:
			strcat(Disass_Str, unkown);
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
		strcat(Disass_Str, unkown);
	}
	return(len);
}

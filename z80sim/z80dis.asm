	TITLE	'Z80-Disassembler'

;	Hardware independent ROMable Z80-Disassembler
;
;	The address from which to begin the disassembly should be stored
;	into the 16-bit memory location DADDR. One of the entry points
;	DISSCR (output of one screen page) or DISLIN (output of one
;	line) can then be called. The program will also store the next
;	address in the memory location DADDR, which makes multiple calls
;	without loading new addresses possible.
;	A subroutine with the name PRTSTR must be written, which will
;	be called with the address of a zero-terminated string in HL.
;
;	27-JUN-89	Udo Munk

LPP	EQU	15		; number of lines per screen page
MNEPOS	EQU	11H		; offset of mnemonics into output workspace

	; disassemble one screen page
DISSCR:	LD	B,LPP		; disassemble one screen page with LPP lines
$DLP1:	PUSH	BC
	CALL	DISLIN
	POP	BC
	DJNZ	$DLP1
	RET

	; disassemble one line
DISLIN:	CALL	CLWO		; clear workspace for one line output
	LD	HL,WRKS		; address of output workspace -> HL
	LD	DE,(DADDR)	; disassembly address -> DE
	CALL	PRADDR		; output address in DE
	INC	HL		; output blank space
	LD	(PRTMP),HL	; save opcode space print position
	LD	C,0		; clear control flags
	DEC	DE		; current addr.-1 -> DE
$DL13:	CALL	PRNBO		; output current byte
	LD	A,(DE)		; current byte -> A
	LD	B,A		; and save in B
	CP	0EDH		; prefix ED ?
	JR	NZ,$DL14
	SET	4,C		; yes, set ED-flag
	JR	$DL13		; and process next byte
$DL14:	CP	0FDH		; prefix FD ?
	JR	NZ,$DL15
	SET	6,C		; yes, set FD-flag
	JR	$DL16		; and index-flag
$DL15:	CP	0DDH		; prefix DD ?
	JR	NZ,$DL17
$DL16:	SET	5,C		; set index-flag for IX/IY-addressing
	LD	HL,(PRTMP)	; opcode space print position -> HL
	JR	$DL13		; process next byte
$DL17:	LD	HL,WRKS+MNEPOS	; set HL to operator position

	; after prefix CB
CB:	LD	A,B		; get opcode byte from B
	CP	0CBH		; prefix CB ?
	JP	NZ,WITHOU
	INC	DE		; yes, set pointer to next byte
	BIT	5,C		; IX/IY-flag ?
	JR	Z,$DL18
	INC	DE		; yes, set pointer to next byte
$DL18:	LD	A,(DE)		; next byte -> A
	LD	B,A		; and save in B
	PUSH	DE		; save disassembly addr.
	LD	D,MNETAB >> 8	; high byte of operator table -> D
	LD	E,0E8H		; DE = pointer to "SET"
	CP	0C0H		; SET ?
	JR	NC,$DL19
	LD	E,0E4H		; no, DE = pointer to "RES"
	CP	80H		; RES ?
	JR	NC,$DL19
	LD	E,0E0H		; no, DE = pointer to "BIT"
	CP	40H		; BIT ?
	JR	NC,$DL19
	AND	38H		; clear bits 0..2 and 6..7
	RRCA			; division by 2
	ADD	A,CBMTAB & 0FFH ; add to base of CB-mnemonics
	LD	E,A
	LD	D,CBMTAB >> 8	; DE = pointer to CB-mnemonic
$DL19:	CALL	PRMNE		; output mnemonic
	POP	DE		; restore current disassembly addr. -> DE
	LD	A,B		; restore byte -> A
	BIT	5,C		; IX/IY-flag ?
	JR	Z,$DL20
	DEC	DE		; one back if IX/IY-addressing
$DL20:	DEC	DE		; pointer again to CB-prefix
	CP	40H		; CB-opcode < 40H ?
	JR	C,$DL21
	AND	38H		; no, clear bits 0..2 and 6..7
	RRCA			; division by 8 -> 1. operand
	RRCA
	RRCA
	CALL	PRO1		; output 1. operand
	LD	A,B		; restore opcode byte to A
	SET	7,C		; set comma-flag
$DL21:	AND	7		; clear bits 3..7 -> 2. operand
	SET	4,A		; set letters-flag
	CALL	PRO1		; output 2. operand
	CALL	PRNBO		; output opcode byte to opcode space
	JP	INADDR		; done, save address and output workspace

	; without prefix CB/ED
WITHOU:	PUSH	DE		; save disassembly addr.
	BIT	4,C		; ED-flag ?
	JP	NZ,ED
	CP	40H		; no, < 40H ?
	JR	C,$DL25
	CP	80H		; no, > 80H ?
	JR	NC,$DL23
	LD	E,50H		; no, DE = pointer to "LD"
	CP	76H		; HALT ?
	JR	NZ,$DL22
	LD	E,5CH		; yes, DE = pointer to "HALT"
$DL22:	JR	$DL26		; output mnemonic
$DL23:	CP	0C0H		; > C0H ?
	JR	NC,$DL24
	AND	38H		; yes, clear bits 0..2 and 6..7
	RRCA			; division by 2 -> operator
	LD	E,A		; operator -> E
	JR	$DL26		; output mnemonic
$DL24:	SUB	80H		; if > C0H, -80 -> table operator
$DL25:	LD	E,A		; operator -> E
	LD	D,CODTAB >> 8	; high byte operator table -> D
	LD	A,(DE)		; LSB mnemonic address -> A
	LD	E,A		; and to E
$DL26:	LD	D,MNETAB >> 8	; MSB mnemonic address -> D
	CALL	PRMNE		; output mnemonic
	POP	DE		; restore current disassembly addr. -> DE
	LD	A,B		; restore opcode byte -> A
	PUSH	DE		; save disassembly addr.
	CP	40H		; byte < 40 ?
	JR	C,$DL30
	CP	80H		; no, > 80 ?
	JR	NC,$DL28
	CP	76H		; no, HALT ?
	JR	NZ,$DL27
	LD	A,0FFH		; yes, empty operand -> A
	JR	$DL31		; output operands
$DL27:	AND	38H		; clear bits 0..2 and 6..7
	RRCA			; division by 8 -> 1. operand
	RRCA
	RRCA
	SET	4,A		; set letters-flag
	JR	$DL31		; output operands
$DL28:	CP	0C0H		; > C0 ?
	JR	NC,$DL29
	CP	90H		; > 90 ?
	JR	C,$DL51
	AND	0F8H		; yes, clear register bits
	CP	98H		; "SBC" ?
	JR	Z,$DL51
	LD	A,B		; restore opcode byte to A
	AND	7		; leave only register bits
	SET	4,A		; set letters-flag
	JR	$DL52
$DL51:	LD	A,17H		; yes, 17 = output register A
	JR	$DL31		; output operands
$DL29:	SUB	80H		; if > C0, -80 -> operands table
$DL30:	LD	E,A		; LSB operands table -> E
	LD	D,OPETAB >> 8	; MSB operands table -> D
	LD	A,(DE)		; 1. operand -> A
$DL31:	POP	DE		; restore curr. disassembly addr. -> DE
	CALL	PRO1		; output 1. operand
	LD	A,B		; restore opcode byte -> A
	PUSH	DE		; save curr. disassembly addr.
	CP	40H		; < 40 ?
	JR	C,$DL34
	CP	0C0H		; no, < C0 ?
	JR	NC,$DL33
	CP	76H		; yes, HALT ?
	JR	NZ,$DL32
	LD	A,0FFH		; yes, again empty operand -> A
	JR	$DL35		; output operands
$DL32:	AND	7		; clear bits 3..7, -> 2. operand
	SET	4,A		; set letters-flag
	JR	$DL35		; output operands
$DL33:	SUB	80H		; if > C0 : subtract 80
$DL34:	ADD	A,80H		; LSB operands table -> A
	LD	E,A		; and -> E
	LD	D,OPETAB >> 8	; MSB operands table -> D
	LD	A,(DE)		; 2. operand -> A
$DL35:	POP	DE		; restore curr. disassembly addr. -> DE
	SET	7,C		; set comma-flag
	CALL	PRO1		; output 2. operand
	JP	INADDR		; done, save address and output workspace

	; after prefix ED
ED:	SUB	40H		; subtract 40 from 2. byte
	JP	C,ERRO		; error when carry
	CP	60H		; 2. byte < A0 ?
	JR	NC,$DL36
	CP	40H		; yes, >= 60 ?
	JP	NC,ERRO		; yes, error
	JR	$DL37		; no, continue
$DL36:	SUB	20H		; convert 60..7F to 00..20
$DL37:	ADD	A,80H		; LSB operator table -> A
	LD	E,A		; and -> E
	LD	D,CODTAB >> 8	; MSB operator table -> D
	LD	A,(DE)		; LSB mnemonic address -> A
	CP	0FFH		; empty ?
	JP	Z,ERRO		; yes, error
	LD	E,A		; no, -> E
	LD	D,MNETAB >> 8	; MSB mnemonic address -> D
	CALL	PRMNE		; output mnemonic
	POP	DE		; restore disassembly addr. -> DE
	LD	A,B		; restore opcode byte -> A
	CP	80H		; < 80 ?
	JP	NC,INADDR	; no, output workspace and done
	PUSH	DE		; save disassembly addr.
	SUB	40H		; LSB 1. operand in A
	LD	E,A		; and -> E
	LD	D,OP2TAB >> 8	; MSB 2. operand -> D
	LD	A,(DE)		; 1. operand -> A
	POP	DE		; restore curr. disassembly addr. -> DE
	CALL	PRO1		; output 1. operand
	LD	A,B		; restore opcode byte -> A
	CP	80H		; < 80 ?
	JP	NC,INADDR	; yes, output workspace and done
	PUSH	DE		; save curr. disassembly addr.
	LD	E,A		; LSB operands table -> E
	LD	D,OP2TAB >> 8	; MSB operands table -> D
	LD	A,(DE)		; 2. operand -> A
	SET	7,C		; set comma-flag
$DL52:	POP	DE		; restore curr. disassembly addr. -> DE
	CALL	PRO1		; output 2. operand
	JP	INADDR		; done, save address and output workspace

	; output operand 1
PRO1:	CP	0FFH		; empty operand ?
	RET	Z		; yes, done
	CP	17H		; no, output register "A" ?
	JR	NZ,$DL01
	LD	A,18H		; yes, recode
$DL01:	CP	16H		; output register "(HL)" ?
	JR	NZ,$DL02
	LD	A,0B4H		; yes, recode
$DL02:	BIT	7,C		; 2. operand ?
	JR	Z,$DL03
	LD	(HL),','	; yes, output ","
	INC	HL		; next print position -> HL
$DL03:	BIT	7,A		; "(...)" ?
	JR	Z,$DL04
	LD	(HL),'('	; yes, output "("
	INC	HL		; next print position -> HL
$DL04:	BIT	4,A		; letters ?
	JR	NZ,PRLET	; yes, output letters
	BIT	6,A		; no, bit number/RST address ?
	JR	NZ,DIST		; yes, output distance
	BIT	5,A		; no, bit number ?
	JR	NZ,PRO2		; yes, output RST
	AND	7		; no, clear bits 3..7
	CALL	PRNIBB		; output hex byte
	RET

	; output RST
PRO2:	PUSH	AF		; save A
	AND	6		; clear bits 0 and 4..7
	RRCA			; division by 2
	CALL	PRNIBB		; output upper nibble
	POP	AF		; restore A
	BIT	0,A		; RST x8 ?
	LD	A,'0'		; no, "0" -> A
	JR	Z,$DL05
	LD	A,'8'		; yes, "8" -> A
$DL05:	LD	(HL),A		; output "0" or "8"
	INC	HL		; next print position -> HL
	RET

	; output distance
DIST:	BIT	0,A		; relative distance ?
	JR	Z,PR_N		; no, output N
	CALL	PRNBO		; output byte to opcode space
	PUSH	DE		; save curr. disassembly addr.
	LD	A,(DE)		; distance byte -> A
	INC	DE		; increment disassembly addr.
	RLCA			; distance byte bit 7 -> carry
	RRCA
	JR	NC,$DL06	; forward jump
	SET	0,C		; set flag for backward jump
$DL06:	ADD	A,E		; add distance to PC
	LD	E,A
	BIT	0,C		; test flag
	JR	NC,$DL07	; no overflow
	JR	NZ,$DL08	; backward jump
	INC	D		; increment MSB PC
	JR	$DL08		; output destination address
$DL07:	JR	Z,$DL08		; if forward jump output destination address
	DEC	D		; else decrement MSB PC
$DL08:	CALL	PRADDR		; output destination address in DE
	POP	DE		; restore curr. disassembly addr. -> DE
	RET

	; output N
PR_N:	PUSH	AF		; save A
	BIT	1,A		; N ?
	JR	Z,PRNN		; no, output NN
	CALL	PRNBOI		; yes, output byte into opcode and instr. space
	JR	$DL12		; process ")"

	; output NN
PRNN:	CALL	PRNBO		; output byte into opcode space
	CALL	PRNBOI		; output byte into opcode and instr. space
	DEC	DE		; DE -> LSB of NN
	CALL	PRBI		; output byte into instr. space
	INC	DE		; restore curr. disassembly addr. -> DE
	JR	$DL12		; process ")"

	; output letters
PRLET:	PUSH	AF		; save A
	PUSH	BC		; save flags in C
	PUSH	DE		; save curr. disassembly addr.
	LD	B,1		; count = 1
	LD	DE,REGTAB	; DE points to register names
	BIT	5,A		; 2 letters ?
	JR	Z,$DL09
	INC	B		; yes, increment count
$DL09:	BIT	6,A		; jump condition ?
	JR	Z,$DL10
	LD	DE,SPRTAB	; yes, DE points to condition names
$DL10:	RES	7,A		; clear parentheses-bit
	CP	34H		; "(HL)" ?
	JR	NZ,$DL11
	BIT	5,C		; yes, index register ?
	JR	Z,$DL11
	LD	A,0AH		; yes, A -> IX register
	BIT	6,C		; IY register ?
	JR	Z,$DL11
	LD	A,0CH		; yes, A -> IY register
$DL11:	AND	0FH		; clear upper nibble
	ADD	A,E		; and add to base in DE
	LD	E,A
$DL50:	LD	A,(DE)		; character -> A
	LD	(HL),A		; output character
	INC	DE		; increment table address
	INC	HL		; next print position
	DJNZ	$DL50		; next character
	POP	DE		; restore registers
	POP	BC
	POP	AF
	PUSH	AF		; save A
	CP	0B4H		; "(HL)" ?
	JR	NZ,$DL12
	BIT	5,C		; no, index register ?
	JR	Z,$DL12
	LD	A,(DE)		; yes, opcode byte after DD/FD -> A
	CP	0E9H		; "JP (IX/IY)" ?
	JR	Z,$DL12
	LD	(HL),'+'	; no, output "+"
	INC	HL		; next print position
	CALL	PRNBOI		; output offset
$DL12:	POP	AF		; restore A
	BIT	7,A		; "()" ?
	RET	Z		; no, done
	LD	(HL),')'	; yes, output ")"
	INC	HL		; next print position
	RET

	; error
ERRO:	LD	DE,CBMTAB+24	; "????" pointer -> DE
	CALL	PRMNE		; output as mnemonic
	POP	DE		; get curr. disassembly addr. from stack

	; increment and store disassembly address
INADDR:	INC	DE
	LD	(DADDR),DE

	; output workspace
PRWO:	PUSH	AF		; save registers
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	HL,WRKS		; workspace address -> HL
	CALL	PRTSTR		; output workspace on terminal
	LD	HL,NL		; newline string address -> HL
	CALL	PRTSTR		; output newline
	POP	HL		; restore registers
	POP	DE
	POP	BC
	POP	AF
	RET

	; clear workspace
CLWO:	LD	HL,WRKS		; fill workspace with blank spaces
	LD	DE,WRKS+1	; and terminate with zero
	LD	(HL),' '
	LD	BC,32
	LDIR
	XOR	A
	LD	(DE),A
	RET

	; output mnemonic
PRMNE:	PUSH	BC		; save BC
	LD	BC,4		; 4 bytes
	EX	DE,HL		; DE=print positiion, HL=mnemonic
	LDIR			; transfer bytes
	EX	DE,HL		; HL is print position again
	POP	BC		; restore BC
	INC	HL		; blank space
	RET

	; output next byte into opcode and instr. space
PRNBOI:	CALL	PRNBO		; output next byte into opcode space

	; output byte into instr. space
PRBI:	PUSH	AF		; save A
	LD	A,(DE)		; byte -> A
	CALL	PRACC		; output A
	POP	AF		; restore A
	RET

	; output next byte into opcode space
PRNBO:	PUSH	AF		; save A
	INC	DE		; set DE to next byte
	PUSH	HL		; save current print position
	LD	HL,(PRTMP)	; opcode space print position -> HL
	LD	A,(DE)		; byte -> A
	CALL	PRACC		; output A
	INC HL			; blank space
	LD	(PRTMP),HL	; save opcode print position
	POP	HL		; restore current print position
	POP	AF		; restore A
	RET

	; output DE
PRADDR:	LD	A,D		; MSB -> A
	CALL	PRACC		; output A
	LD	A,E		; LSB -> A

	; output A
PRACC:	PUSH	AF		; save A
	RRCA			; shift upper nibble into lower one
	RRCA
	RRCA
	RRCA
	CALL	PRNIBB		; output upper nibble
	POP	AF		; restore A
	CALL	PRNIBB		; output lower nibble
	RET

	; convert lower nibble to ASCII hex and write it into the workspace
PRNIBB:	AND	0FH
	ADD	A,90H
	DAA
	ADC	A,40H
	DAA
	LD	(HL),A		; write ASCII digit into the workspace
	INC	HL		; increment print position
	RET

	; The following tables must be at the beginning of a memory page
	; and the order of the tables must not be changed, because
	; the LSB of the table addresses is calculated with arithmetic
	; operations on the opcodes !!!

	DEFS	256 - ($ & 0FFH)

MNETAB:		; Z80 mnemonics table
	DEFM	'ADD ADC '
	DEFM	'SUB SBC '
	DEFM	'AND XOR '
	DEFM	'OR  CP  '
	DEFM	'JR  NOP '
	DEFM	'DJNZEX  '
	DEFM	'RLCARLA '
	DEFM	'DAA SCF '
	DEFM	'RRCARRA '
	DEFM	'CPL CCF '
	DEFM	'LD  INC '
	DEFM	'DEC HALT'
	DEFM	'RET POP '
	DEFM	'JP  OUT '
	DEFM	'EX  DI  '
	DEFM	'CALLPUSH'
	DEFM	'RST EXX '
	DEFM	'IN  EX  '
	DEFM	'EI  LDI '
	DEFM	'LDIRINI '
	DEFM	'INIROUTI'
	DEFM	'OTIRNEG '
	DEFM	'RETNRRD '
	DEFM	'LDD LDDR'
	DEFM	'CPD CPDR'
	DEFM	'IND INDR'
	DEFM	'OTDROUTD'
	DEFM	'RETIRLD '
	DEFM	'BIT RES '
	DEFM	'SET ????'
	DEFM	'CPI CPIR'
	DEFM	'IM  ----'

CODTAB:		; LSB addresses of mnemonics in MNETAB for the
		; opcodes 00..3F without prefix ED/CB

	DEFB	024H,050H,050H,054H,054H,058H,050H,030H	; NOP  LD   LD   INC  INC  DEC  LD   RLCA
	DEFB	070H,000H,050H,058H,054H,058H,050H,040H	; EX   ADD  LD   DEC  INC  DEC  LD   RRCA
	DEFB	028H,050H,050H,054H,054H,058H,050H,034H	; DJNZ LD   LD   INC  INC  DEC  LD   RLA
	DEFB	020H,000H,050H,058H,054H,058H,050H,044H	; JR   ADD  LD   DEC  INC  DEC  LD   RRA
	DEFB	020H,050H,050H,054H,054H,058H,050H,038H	; JR   LD   LD   INC  INC  DEC  LD   DAA
	DEFB	020H,000H,050H,058H,054H,058H,050H,048H	; JR   ADD  LD   DEC  INC  DEC  LD   CPL
	DEFB	020H,050H,050H,054H,054H,058H,050H,03CH	; JR   LD   LD   INC  INC  DEC  LD   SCF
	DEFB	020H,000H,050H,058H,054H,058H,050H,04CH	; JR   ADD  LD   DEC  INC  DEC  LD   CCF

		; LSB addresses of mnemonics in MNETAB for the
		; opcodes C0..FF without prefix ED/CB

	DEFB	060H,064H,068H,068H,078H,07CH,000H,080H	; RET  POP  JP   JP   CALL PUSH ADD  RST
	DEFB	060H,060H,068H,0F1H,078H,078H,004H,080H	; RET  RET  JP   (CB) CALL CALL ADC  RST
	DEFB	060H,064H,068H,06CH,078H,07CH,008H,080H	; RET  POP  JP   OUT  CALL PUSH SUB  RST
	DEFB	060H,084H,068H,088H,078H,0F0H,00CH,080H	; RET  EXX  JP   IN   CALL (DD) SBC  RST
	DEFB	060H,064H,068H,070H,078H,07CH,010H,080H	; RET  POP  JP   EX   CALL PUSH AND  RST
	DEFB	060H,068H,068H,02CH,078H,0F2H,014H,080H	; RET  JP   JP   EX   CALL (ED) XOR  RST
	DEFB	060H,064H,068H,074H,078H,07CH,018H,080H	; RET  POP  JP   DI   CALL PUSH OR   RST
	DEFB	060H,050H,068H,090H,078H,0F8H,01CH,080H	; RET  LD   JP   EI   CALL (FD) CP   RST

		; LSB addresses of mnemonics in MNETAB for the
		; opcodes 40..7F with prefix ED

	DEFB	088H,06CH,00CH,050H,0ACH,0B0H,0F8H,050H	; IN   OUT  SBC  LD   NEG  RETN IM   LD
	DEFB	088H,06CH,004H,050H,0FFH,0D8H,0FFH,050H	; IN   OUT  ADC  LD        RETI      LD
	DEFB	088H,06CH,00CH,050H,0FFH,0FFH,0F8H,050H	; IN   OUT  SBC  LD             IM   LD
	DEFB	088H,06CH,004H,050H,0FFH,0FFH,0F8H,050H	; IN   OUT  ADC  LD             IM   LD
	DEFB	088H,06CH,00CH,0FFH,0FFH,0FFH,0FFH,0B4H	; IN   OUT  SBC                      RRD
	DEFB	088H,06CH,004H,0FFH,0FFH,0FFH,0FFH,0DCH	; IN   OUT  ADC                      RLD
	DEFB	0FFH,0FFH,00CH,050H,0FFH,0FFH,0FFH,0FFH	;           SBC  LD
	DEFB	088H,06CH,004H,050H,0FFH,0FFH,0FFH,0FFH	; IN   OUT  ADC  LD

		; LSB addresses of mnemonics in MNETAB for the
		; opcodes A0..BF with prefix ED

	DEFB	094H,0F0H,09CH,0A4H,0FFH,0FFH,0FFH,0FFH	; LDI  CPI  INI  OUTI
	DEFB	0B8H,0C0H,0C8H,0D4H,0FFH,0FFH,0FFH,0FFH	; LDD  CPD  IND  OUTD
	DEFB	098H,0F4H,0A0H,0A8H,0FFH,0FFH,0FFH,0FFH	; LDIR CPIR INIR OTIR
	DEFB	0BCH,0C4H,0CCH,0D0H,0FFH,0FFH,0FFH,0FFH	; LDDR CPDR INDR OTDR

SPRTAB:		; jump conditions table

	DEFM	'NZNCPOPEPM'
	DEFB	0FFH,0FFH,0FFH,0FFH,0FFH,0FFH

REGTAB:		; registers table

	DEFM	'BCDEHLSPAFIXIYIR'

OPETAB:		; operands table:
		;	bit 7: number/letter
		;	bit 6: single/double
		;	bit 5: register/jump condition
		;	bit 4: without/with parentheses
		;	bit 0..3: offset into the register names table

		; opcodes 00..3F without prefix ED/CB
		; 1. operand

	DEFB	0FFH,030H,0B0H,030H,010H,010H,010H,0FFH	; -    BC   (BC) BC   B    B    B    -
	DEFB	038H,034H,017H,030H,011H,011H,011H,0FFH	; AF   HL   A    BC   C    C    C    -
	DEFB	041H,032H,0B2H,032H,012H,012H,012H,0FFH	; DIS  DE   (DE) DE   D    D    D    -
	DEFB	041H,034H,017H,032H,013H,013H,013H,0FFH	; DIS  HL   A    DE   E    E    E    -
	DEFB	070H,034H,0C4H,034H,014H,014H,014H,0FFH	; NZ   HL   (NN) HL   H    H    H    -
	DEFB	051H,034H,034H,034H,015H,015H,015H,0FFH	; Z    HL   HL   HL   L    L    L    -
	DEFB	072H,036H,0C4H,036H,016H,016H,016H,0FFH	; NC   SP   (NN) SP   (HL) (HL) (HL) -
	DEFB	011H,034H,017H,036H,017H,017H,017H,0FFH	; C    HL   A    SP   A    A    A    -

		; opcodes C0..FF without prefix ED/CB
		; 1. operand

	DEFB	070H,030H,070H,044H,070H,030H,017H,020H	; NZ   BC   NZ   NN   NZ   BC   A    00
	DEFB	051H,0FFH,051H,0F1H,051H,044H,017H,021H	; Z    -    Z    *CB  Z    NN   A    08
	DEFB	072H,032H,072H,0C2H,072H,032H,042H,022H	; NC   DE   NC   (N)  NC   DE   N    10
	DEFB	053H,0FFH,053H,017H,053H,0F2H,017H,023H	; C    -    C    A    C    *DD  A    18
	DEFB	074H,034H,074H,0B6H,074H,034H,042H,024H	; PO   HL   PO   (SP) PO   HL   N    20
	DEFB	076H,016H,076H,032H,076H,0F4H,042H,025H	; PE   (HL) PE   DE   PE   *ED  N    28
	DEFB	058H,038H,058H,0FFH,058H,038H,042H,026H	; P    AF   P    -    P    AF   N    30
	DEFB	059H,036H,059H,0FFH,059H,0F8H,042H,027H	; M    SP   M    -    M    *FD  N    38

		; opcodes 00..3F without prefix ED/CB
		; 2. operand

	DEFB	0FFH,044H,017H,0FFH,0FFH,0FFH,042H,0FFH	; -    NN   A    -    -    -    N    -
	DEFB	038H,030H,0B0H,0FFH,0FFH,0FFH,042H,0FFH	; AF   BC   (BC) -    -    -    N    -
	DEFB	0FFH,044H,017H,0FFH,0FFH,0FFH,042H,0FFH	; -    NN   A    -    -    -    N    -
	DEFB	0FFH,032H,0B2H,0FFH,0FFH,0FFH,042H,0FFH	; -    DE   (DE) -    -    -    N    -
	DEFB	041H,044H,034H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  NN   HL   -    -    -    N    -
	DEFB	041H,034H,0C4H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  HL   (NN) -    -    -    N    -
	DEFB	041H,044H,017H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  NN   A    -    -    -    N    -
	DEFB	041H,036H,0C4H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  SP   (NN) -    -    -    N    -

		; opcodes C0..FF without prefix ED/CB
		; 2. operand

	DEFB	0FFH,0FFH,044H,0FFH,044H,0FFH,042H,0FFH	; -    -    NN   -    NN   -    N    -
	DEFB	0FFH,0FFH,044H,0FFH,044H,0FFH,042H,0FFH	; -    -    NN   *CB  NN   -    N    -
	DEFB	0FFH,0FFH,044H,017H,044H,0FFH,0FFH,0FFH	; -    -    NN   A    NN   -    -    -
	DEFB	0FFH,0FFH,044H,0C2H,044H,0FFH,042H,0FFH	; -    -    NN   (N)  NN   *DD  N    -
	DEFB	0FFH,0FFH,044H,034H,044H,0FFH,0FFH,0FFH	; -    -    NN   HL   NN   -    -    -
	DEFB	0FFH,0FFH,044H,034H,044H,0FFH,0FFH,0FFH	; -    -    NN   HL   NN   *ED  -    -
	DEFB	0FFH,0FFH,044H,0FFH,044H,0FFH,0FFH,0FFH	; -    -    NN   -    NN   -    -    -
	DEFB	0FFH,034H,044H,0FFH,044H,0FFH,0FFH,0FFH	; -    HL   NN   -    NN   *FD  -    -

OP2TAB:		; opcodes 40..7F with prefix ED
		; 1. operand

	DEFB	010H,091H,034H,0C4H,0FFH,0FFH,000H,01EH	; B    (C)  HL   (NN) -    -    0    I
	DEFB	011H,091H,034H,030H,0FFH,0FFH,0FFH,01FH	; C    (C)  HL   BC   -    -    -    R
	DEFB	012H,091H,034H,0C4H,0FFH,0FFH,001H,017H	; D    (C)  HL   (NN) -    -    1    A
	DEFB	013H,091H,034H,032H,0FFH,0FFH,002H,017H	; E    (C)  HL   DE   -    -    2    A
	DEFB	014H,091H,034H,0C4H,0FFH,0FFH,076H,0FFH	; H    (C)  HL   -    -    -    -    -
	DEFB	015H,091H,034H,0FFH,0FFH,0FFH,0FFH,0FFH	; L    (C)  HL   -    -    -    -    -
	DEFB	0FFH,0FFH,034H,0C4H,0FFH,0FFH,0FFH,0FFH	; -    -    HL   (NN) -    -    -    -
	DEFB	017H,091H,034H,036H,0FFH,0FFH,0FFH,0FFH	; A    (C)  HL   SP   -    -    -    -

		; opcodes 40..7F with prefix ED
		; 2. operand

	DEFB	091H,010H,030H,030H,0FFH,0FFH,0FFH,017H	; (C)  B    BC   BC   -    -    -    A
	DEFB	091H,011H,030H,0C4H,0FFH,0FFH,0FFH,017H	; (C)  C    BC   (NN) -    -    -    A
	DEFB	091H,012H,032H,032H,0FFH,0FFH,0FFH,01EH	; (C)  D    DE   DE   -    -    -    I
	DEFB	091H,013H,032H,0C4H,0FFH,0FFH,0FFH,01FH	; (C)  E    DE   (NN) -    -    -    R
	DEFB	091H,014H,034H,034H,0FFH,0FFH,0FFH,0FFH	; (C)  H    HL   -    -    -    -    -
	DEFB	091H,015H,034H,0FFH,0FFH,0FFH,0FFH,0FFH	; (C)  L    HL   -    -    -    -    -
	DEFB	0FFH,0FFH,036H,036H,0FFH,0FFH,0FFH,0FFH	; -    -    SP   SP   -    -    -    -
	DEFB	091H,017H,036H,0C4H,0FFH,0FFH,0FFH,0FFH	; (C)  A    SP   (NN) -    -    -    -

CBMTAB:		; mnemonics table for prefix CB
	DEFM	'RLC RRC '
	DEFM	'RL  RR  '
	DEFM	'SLA SRA '
	DEFM	'????SRL '

NL:		; zero-terminated newline for the terminal
	DEFB	10,13,0

WRKS:	DEFS	34		; workspace for the preparation of an output line
PRTMP:	DEFS	2		; temporary storage for the print position
DADDR:	DEFS	2		; disassembly address

;
;	UCSD p-System IV CBIOS for IMSAI 8080
;
;	Copyright (C) 2017 by Udo Munk
;
;	Use 8080 instructions only.
;
MSIZE	EQU	54		;memory size in kilobytes
BIAS	EQU	(MSIZE*1024)-01900H
BIOS	EQU	1500H+BIAS	;base of bios
;
;	I/O ports
;
TTY	EQU	02H		;tty data port
TTYS	EQU	03H		;tty status port
PRINTER	EQU	0F6H		;IMSAI PTR-300 line printer
FDC	EQU	0FDH		;IMSAI FIF FDC
;
;	disk descriptor for FIF
;
FIF	EQU	80H
DDCMD	EQU	80H		;unit/command
DDRES	EQU	81H		;result code
DDHTRK	EQU	82H		;track high
DDLTRK	EQU	83H		;track low
DDSEC	EQU	84H		;sector
DDLDMA	EQU	85H		;DMA address low
DDHDMA	EQU	86H		;DMA address high
;
	ORG	BIOS		;origin of this program
;
;	jump vector for individual subroutines
;
	JP	BOOT		;cold start
WBOOTE: JP	WBOOT		;warm start
	JP	CONST		;console status
	JP	CONIN		;console character in
	JP	CONOUT		;console character out
	JP	LIST		;list character out
	JP	PUNCH		;punch character out
	JP	READER		;reader character out
	JP	HOME		;move head to home position
	JP	SELDSK		;select disk
	JP	SETTRK		;set track number
	JP	SETSEC		;set sector number
	JP	SETDMA		;set dma address
	JP	READ		;read disk
	JP	WRITE		;write disk
	JP	LISTST		;return list status
	JP	SECTRAN		;sector translate
;
;	copyright text
;
	DEFM	'54K UCSD p-System IV.0 CBIOS V1.0 for IMSAI 8080, '
	DEFM	'Copyright 2017 by Udo Munk'
	DEFB	13,10,0
;
;	individual subroutines to perform each function
;	simplest case is to just perform parameter initialization
;
BOOT:
	RET
;
WBOOT:
	DI
	HALT
	RET
;
;	simple i/o handlers
;
;	console status, return 0ffh if character ready, 00h if not
;
CONST:
	IN	A,(TTYS)	;get tty status
	AND	02H		;mask bit
	RET	Z		;not ready
	LD	A,0FFH		;ready, set flag
	RET
;
;	console character into register a
;
CONIN:
	IN	A,(TTYS)	;get tty status
	AND	02H		;mask bit
	JP	Z,CONIN		;wait until character available
	IN	A,(TTY)		;get character
	RET
;
;	console character output from register c
;
CONOUT:
	IN	A,(TTYS)	;get status
	RRCA			;test bit 0
	JP	NC,CONOUT	;wait until transmitter ready
	LD	A,C		;get to accumulator
	OUT	(TTY),A		;send to tty
	CP	27		;ESC character?
	RET	NZ		;no, done
	LD	C,'['		;send second lead in for ANSI terminals
	JP	CONOUT
;
;	list character from register c
;
LIST:
	IN	A,(PRINTER)	;get printer status
	CP	0FFH		;no printer
	RET	Z
	AND	04H		;check ready bit
	JP	Z,LIST		;not ready, try again
	LD	A,C		;character to register a
	OUT	(PRINTER),A	;output it
	RET
;
;	return list status (00h if not ready, 0ffh if ready)
;
LISTST:
	IN	A,(PRINTER)	;get printer status
	CP	0FFH		;no printer
	JP	Z,LST1
	AND	04H		;check ready bit
	JP	Z,LST1		;not ready
	LD	A,0FFH		;else return with ready status
	RET
LST1:	XOR	A		;printer not ready
	RET
;
;	punch character from register c
;
PUNCH:
	RET
;
;	read character into register a from reader device
;
READER:
	LD	A,01AH		;return EOF
	RET
;
;
;	i/o drivers for the disk follow
;
;	move to the track 00 position of current drive
;	translate this call into a settrk call with parameter 00
;
HOME:
	LD	C,0		;select track 0
	JP	SETTRK
;
;	select disk given by register C
;
SELDSK:
	LD	HL,0000H	;return code
	LD	A,C
	CP	0		;disk drive 0?
	JP	Z,SEL0
	CP	1		;disk drive 1?
	JP	Z,SEL1
	CP	2		;disk drive 2?
	JP	Z,SEL2
	CP	3		;disk drive 3?
	JP	Z,SEL3
	RET
SEL0:	LD	A,1		;unit 1
	JP	SELX
SEL1:	LD	A,2		;unit 2
	JP	SELX
SEL2:	LD	A,4		;unit 3
	JP	SELX
SEL3:	LD	A,8		;unit 4
SELX:	LD	(DDCMD),A	;set disk unit in FIF DD
	RET
;
;	set track given by register c
;
SETTRK:
	LD	A,C		;get to accumulator
	LD	(DDLTRK),A	;set in FIF DD
	RET
;
;	set sector given by register c
;
SETSEC:
	LD	A,C		;get to accumulator
	LD	(DDSEC),A	;set in FIF DD
	RET
;
;	translate the sector given by BC using the
;	translate table given by DE
;
SECTRAN:
	LD	L,C		;return untranslated
	LD	H,B		;in HL
	INC	L		;sector no. start with 1
	RET
;
;	set dma address given by registers b and c
;
SETDMA:
	LD	A,C		;low order address
	LD	(DDLDMA),A
	LD	A,B		;high order address
	LD	(DDHDMA),A
	RET
;
;	perform read operation
;
READ:
	LD	A,(DDCMD)	;get unit/command
	AND	0FH		;mask out command
	OR	20H		;mask in read command
	LD	(DDCMD),A	;set in FIF DD
	JP	DOIO		;do I/O operation
;
;	perform a write operation
;
WRITE:
	LD	A,(DDCMD)	;get unit/command
	AND	0FH		;mask out command
	OR	10H		;mask in write command
	LD	(DDCMD),A	;set in FIF DD
;
;	enter here from read and write to perform the actual i/o
;	operation.  return a 00h in register a if the operation completes
;	properly, and 01h if an error occurs during the read or write
;
DOIO:
	XOR	A		;zero accumulator
	LD	(DDRES),A	;reset result code
	OUT	(FDC),A		;ask FDC to execute the DD
IO1:	LD	A,(DDRES)	;wait for FDC
	OR	A
	JP	Z,IO1		;not done yet
	AND	0FEH		;status = 1 ?
	RET	Z		;if yes return OK
	LD	A,(DDRES)	;get status again
	RET			;and return with error
;
	END

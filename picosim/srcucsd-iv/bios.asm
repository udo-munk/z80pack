;
;	UCSD p-System IV CBIOS for z80pack machines using SD-FDC
;
;	Copyright (C) 2024 by Udo Munk
;
;	Use 8080 instructions only.
;
MSIZE	EQU	62		;memory size in kilobytes
BIAS	EQU	(MSIZE*1024)-01900H
BIOS	EQU	1500H+BIAS	;base of bios
;
;	I/O ports
;
TTY	EQU	01H		;tty data port
TTYS	EQU	00H		;tty status port
FDC	EQU	04H		;FDC port
;
;	disk command bytes for FDC
;
FDCMD	EQU	80H		;FDC command
FDTRK	EQU	80H		;track
FDSEC	EQU	81H		;sector
FDLDMA	EQU	82H		;DMA address low
FDHDMA	EQU	83H		;DMA address high
;
DISKNO	EQU	84H		;selected disk
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
	DEFM	'62K UCSD p-System IV.0 CBIOS V1.0 for picosim, '
	DEFM	'Copyright 2024 by Udo Munk'
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
	RRA			;test  bit 0
	JP	C,CONST1	;not ready
	LD	A,0FFH		;ready, set flag
	RET
CONST1:
	XOR	A		;zero A
	RET
;
;	console character into register a
;
CONIN:
	IN	A,(TTYS)	;get tty status
	RRA			;test bit 0
	JP	C,CONIN		;wait until character available
	IN	A,(TTY)		;get character
	RET
;
;	console character output from register c
;
CONOUT:
	IN	A,(TTYS)	;get status
	RLA			;test bit 7
	JP	C,CONOUT	;wait until transmitter ready
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
	RET
;
;	return list status (00h if not ready, 0ffh if ready)
;
LISTST:
	XOR	A
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
	LD	A,C		;get drive #
	CP	2		;< 2 ?
	JP	C,SEL		;if yes take it
	RET			;no thanks
SEL:
	LD	(DISKNO),A
	RET
;
;	set track given by register c
;
SETTRK:
	LD	A,C		;get to accumulator
	LD	(FDTRK),A	;set in FDC command
	RET
;
;	set sector given by register c
;
SETSEC:
	LD	A,C		;get to accumulator
	LD	(FDSEC),A	;set in FDC command
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
	LD	(FDLDMA),A
	LD	A,B		;high order address
	LD	(FDHDMA),A
	RET
;
;	perform read operation
;
READ:
	LD	A,(DISKNO)	;get drive #
	OR	20H		;mask in read command
	JP	DOIO		;do I/O operation
;
;	perform a write operation
;
WRITE:
	LD	A,(DISKNO)	;get drive #
	OR	40H		;mask in write command
;
;	enter here from read and write to perform the actual i/o
;	operation.  return a 00h in register a if the operation completes
;	properly, and 01h if an error occurs during the read or write
;
DOIO:
	OUT	(FDC),A		;ask FDC to execute the command
	IN	A,(FDC)		;get result
	RET
;
	END			;of BIOS

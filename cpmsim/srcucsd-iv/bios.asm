;	UCSD p-System IV CBIOS for Z80-Simulator
;
;	Copyright (C) 2008 by Udo Munk
;
MSIZE	EQU	64		;memory size in kilobytes
BIAS	EQU	(MSIZE*1024)-01900H
BIOS	EQU	1500H+BIAS	;base of bios
;
;	I/O ports
;
CONSTA	EQU	0		;console status port
CONDAT	EQU	1		;console data port
PRTSTA	EQU	2		;printer status port
PRTDAT	EQU	3		;printer data port
AUXDAT	EQU	5		;auxiliary data port
FDCD	EQU	10		;fdc-port: # of drive
FDCT	EQU	11		;fdc-port: # of track
FDCS	EQU	12		;fdc-port: # of sector
FDCOP	EQU	13		;fdc-port: command
FDCST	EQU	14		;fdc-port: status
DMAL	EQU	15		;dma-port: dma address low
DMAH	EQU	16		;dma-port: dma address high
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
	DEFM	'64K UCSD p-System IV.0 CBIOS V1.1 for Z80SIM, '
	DEFM	'Copyright 2008 by Udo Munk'
	DEFB	13,10,0
;
;	individual subroutines to perform each function
;	simplest case is to just perform parameter initialization
;
BOOT:
	RET

WBOOT:
	DI
	HALT
	RET
;
;
;	simple i/o handlers
;
;	console status, return 0ffh if character ready, 00h if not
;
CONST:	IN	A,(CONSTA)	;get console status
	RET
;
;	console character into register a
;
CONIN:	IN	A,(CONDAT)	;get character from console
	RET
;
;	console character output from register c
;
CONOUT: LD	A,C		;get to accumulator
	OUT	(CONDAT),A	;send character to console
	CP	27		;ESC character?
	RET	NZ		;no, done
	LD	A,'['		;send second lead in for ANSI terminals
	OUT	(CONDAT),A
	RET
;
;	list character from register c
;
LIST:	LD	A,C		;character to register a
	OUT	(PRTDAT),A
	RET
;
;	return list status (00h if not ready, 0ffh if ready)
;
LISTST: IN	A,(PRTSTA)
	RET
;
;	punch character from register c
;
PUNCH:	LD	A,C		;character to register a
	OUT	(AUXDAT),A
	RET
;
;	read character into register a from reader device
;
READER: IN	A,(AUXDAT)
	RET
;
;
;	i/o drivers for the disk follow
;
;	move to the track 00 position of current drive
;	translate this call into a settrk call with parameter 00
;
HOME:	LD	C,0		;select track 0
	JP	SETTRK		;we will move to 00 on first read/write
;
;	select disk given by register C
;
SELDSK: LD	HL,0000H	;return code
	LD	A,C
	OUT	(FDCD),A	;selekt disk drive
	RET
;
;	set track given by register c
;
SETTRK: LD	A,C
	OUT	(FDCT),A
	RET
;
;	set sector given by register c
;
SETSEC: LD	A,C
	OUT	(FDCS),A
	RET
;
;	translate the sector given by BC using the
;	translate table given by DE
;
SECTRAN:
	LD	L,C		;return untranslated
	LD	H,B		;in HL
	INC	L		;sector no. start with 1
	RET	NZ
	INC	H
	RET
;
;	set dma address given by registers b and c
;
SETDMA: LD	A,C		;low order address
	OUT	(DMAL),A
	LD	A,B		;high order address
	OUT	(DMAH),A	;in dma
	RET
;
;	perform read operation
;
READ:	XOR	A		;read command -> A
	JP	WAITIO		;to perform the actual i/o
;
;	perform a write operation
;
WRITE:	LD	A,1		;write command -> A
;
;	enter here from read and write to perform the actual i/o
;	operation.  return a 00h in register a if the operation completes
;	properly, and 01h if an error occurs during the read or write
;
WAITIO: OUT	(FDCOP),A	;start i/o operation
	IN	A,(FDCST)	;status of i/o operation -> A
	RET
;
	END			;of BIOS

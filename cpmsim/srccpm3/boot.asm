;	CP/M 3 boot-loader for Z80-Simulator
;
;	Copyright (C) 1989-2007 by Udo Munk
;
	ORG	0		;mem base of boot
;
BOOT	EQU	0100H		;cpmldr runs at 0100H
SECTS	EQU	25		;# of sectors to load
;
;	I/O ports
;
CONDAT	EQU	1		;console data port
DRIVE   EQU	10		;fdc-port: # of drive
TRACK   EQU	11		;fdc-port: # of track
SECTOR  EQU	12		;fdc-port: # of sector
FDCOP   EQU	13		;fdc-port: command
FDCST   EQU	14		;fdc-port: status
DMAL    EQU	15		;dma-port: dma address low
DMAH    EQU	16		;dma-port: dma address high
;
	JP	COLD
;
ERRMSG: DEFM	'BOOT: error booting'
	DEFB	13,10,0
;
;	begin the load operation
;
COLD:	LD	C,2		;c=sector 2
	LD	D,SECTS		;d=# sectors to load
	LD	HL,BOOT		;base transfer address
	LD	A,0		;select drive A
	OUT	(DRIVE),A
	OUT	(TRACK),A	;set track 0
;
;	load the next sector
;
LSECT:	LD	A,C		;set sector
	OUT	(SECTOR),A
	LD	A,L		;set dma address low
	OUT	(DMAL),A
	LD	A,H		;set dma address high
	OUT	(DMAH),A
	XOR	A		;read sector
	OUT	(FDCOP),A
	IN	A,(FDCST)	;get status of fdc
	OR	A		;read successful ?
	JP	Z,CONT		;yes, continue
	LD	HL,ERRMSG	;no, print error
PRTMSG:	LD	A,(HL)
	OR	A
	JP	Z,STOP
	OUT	(CONDAT),A
	INC	HL
	JP	PRTMSG
STOP:	DI
	HALT			;and halt cpu
;
CONT:
				;go to next sector if load is incomplete
	DEC	D		;sects=sects-1
	JP	Z,BOOT		;head for cpmldr
;
;	more sectors to load
;
;	we aren't using a stack, so use <sp> as scratch register
;	to hold the load address increment
;
	LD	SP,128		;128 bytes per sector
	ADD	HL,SP		;<hl> = <hl> + 128
	INC	C		;sector = sector + 1
	JP	LSECT		;go read another
;
	END			;of boot loader

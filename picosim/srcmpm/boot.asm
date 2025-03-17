;
;	MP/M 2 boot-loader for z80pack machines using SD-FDC
;
;	Copyright (C) 1989-2024 by Udo Munk
;	Copyright (C) 2025 by Thomas Eberhardt
;
; History:
; 12-MAR-2025 first public release based on cpmsim/srcmpm and RP2xxx/srccpm3
;
	ORG	0		;mem base of boot
;
BOOT	EQU	0100H		;cpmldr runs at 0100H
SECTS	EQU	51		;# of sectors to load (26 * 2 - 1)
;
;	I/O ports
;
CONDAT	EQU	1		;console data port
FDC	EQU	4		;FDC
;
	JP	COLD
;
ERRMSG:	DEFM	'BOOT: error booting'
	DEFB	13,10,0
;
;	begin the load operation
;
COLD:	LD	A,10H		;setup command for FDC
	OUT	(FDC),A
	LD	A,CMD AND 0FFH
	OUT	(FDC),A
	LD	A,CMD SHR 8
	OUT	(FDC),A
	LD	BC,2		;b=track 0, c=sector 2
	LD	D,SECTS		;d=# sectors to load
;
;	load the next sector
;
LSECT	LD	A,20H		;tell FDC to read sector on drive 0
	OUT	(FDC),A
	IN	A,(FDC)		;get status of FDC
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
	JP	Z,BOOT		;head for the bios

;
;	more sectors to load
;
;	we aren't using a stack, so use <sp> as scratch register
;	to hold the load address increment
;
	LD	HL,(CMD+2)	;get DMA address
	LD	SP,128		;128 bytes per sector
	ADD	HL,SP		;<hl> = <hl> + 128
	LD	(CMD+2),HL	;set new DMA address
;
	INC	C		;sector = sector + 1
	LD	A,C
	LD	(CMD+1),A	;save sector
	CP	27		;last sector of track ?
	JP	C,LSECT		;no, go read another
;
;	end of track, increment to next track
;
	LD	C,1		;sector = 1
	LD	A,C
	LD	(CMD+1),A	;save sector
	INC	B		;track = track + 1
	LD	A,B
	LD	(CMD),A		;save track
	JP	LSECT		;for another group
;
;	command bytes for the FDC
;
CMD	DEFB	00H		;track 0
	DEFB	02H		;sector 2
	DEFB	BOOT AND 0FFH	;DMA address low
	DEFB	BOOT SHR 8	;DMA address high
;
	END			;of boot loader

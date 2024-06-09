;
;	boot ROM for z80pack machines using SD-FDC
;	use 8080 instructions only, so that it runs
;	on all our CPU's
;
;	Copyright (C) 2024 by Udo Munk
;
	ORG	0FF00H		;ROM in upper most memory page
;
;	I/O ports
;
FDC	EQU	4		;FDC port
;
FDCMD	EQU	0F000H		;command bytes for the FDC
;
	DI			;disable interrupts
;
	XOR	A		;zero A
	LD	(FDCMD),A	;track 0 
	LD	(FDCMD+2),A	;DMA address low
	LD	(FDCMD+3),A	;DMA address high
	LD	A,1		;sector 1
	LD	(FDCMD+1),A
;
	LD	A,10H		;setup command for FDC
	OUT	(FDC),A
	LD	A,FDCMD AND 0FFH
	OUT	(FDC),A
	LD	A,FDCMD SHR 8
	OUT	(FDC),A
;
	LD	A,20H		;read sector 1 on track 0 from drive 0
	OUT	(FDC),A
	IN	A,(FDC)		;get FDC result
	OR	A		;zero ?
	JP	Z,0		;if yes jump to boot code loaded @ 0
;
	HALT			;some problem, reason still in A
;
	END			;of ROM

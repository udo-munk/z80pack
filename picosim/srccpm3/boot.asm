;
;	CP/M 3 boot-loader for z80pack machines using SD-FDC
;
;	Copyright (C) 2024 by Udo Munk
;
; History:
; 30-JUN-2024 first public release
;
	ORG	0		; memory base of boot
;
BOOT	EQU	0100H		; cpmldr runs at 0100H
SECTS	EQU	25		; # of sectors to load
;
;	I/O ports
;
FDC	EQU	4		;FDC port
;
	DI			;disable interrupts
	LXI	SP,0FFH		;some space for stack
	MVI	A,10H		;setup command for FDC
	OUT	FDC
	MVI	A,CMD AND 0FFH
	OUT	FDC
	MVI	A,CMD SHR 8
	OUT	FDC
	LXI	B,2		;B=track 0, C=sector 2
	MVI	D,SECTS		;D=# sectors to load
;
; load the next sector
;
LSECT	MVI	A,20H		;tell FDC to read sector on drive 0
	OUT	FDC
	IN	FDC		;get result from FDC
	ORA	A
	JZ	BOOT1
	HLT			;read error, halt CPU
BOOT1	DCR	D		;SECTS=SECTS-1
	JZ	BOOT		;all done, head for cpmldr
	INR	C		;sector = sector + 1
	MOV	A,C
	STA	CMD+1		;save sector
	PUSH	D
	LHLD	CMD+2		;get DMA address
	LXI	D,80H		;and increase it by 128
	DAD	D
	POP	D
	SHLD	CMD+2		;set new dma address
	JMP	LSECT		;for next sector
;
; command bytes for the FDC
CMD	DB	00H		;track 0
	DB	02H		;sector 2
	DB	BOOT AND 0FFH	;DMA address low
	DB	BOOT SHR 8	;DMA address high

	END			;of boot loader

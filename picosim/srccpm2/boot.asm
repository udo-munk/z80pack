;
;	CP/M 2.2 boot-loader for z80pack machines using SD-FDC
;
;	Copyright (C) 2024 by Udo Munk
;
	ORG	0		;mem base of boot
;
MSIZE	EQU	64		;mem size in kbytes
;
BIAS	EQU	(MSIZE-20)*1024	;offset from 20k system
CCP	EQU	3400H+BIAS	;base of the ccp
CPMB	EQU	BIAS+3400H	;start of CP/M
BOOTE	EQU	CPMB+1600H	;cold boot entry point
SECTS	EQU	51		;# of sectors to load (26 * 2 - 1)
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
	JZ	BOOTE		;go to CP/M if all sectors done
	INR	C		;sector = sector + 1
	MOV	A,C
	CPI	27		;last sector of track?
	JC	BOOT2		;no, do next sector
	MVI	C,1		;sector = 1
	INR	B		;track = track + 1
BOOT2	MOV	A,B		;setup command
	STA	CMD+0		;save track
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
	DB	CPMB AND 0FFH	;DMA address low
	DB	CPMB SHR 8	;DMA address high

	END			;of boot loader

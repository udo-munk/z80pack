	TITLE	'CP/M 3 boot loader for IMSAI 8080'
;
;	Copyright (C) 2019 by Udo Munk
;
; History:
; 23-OCT-2019 first public release
;
	ORG	0		; memory base of boot
;
BOOT	EQU	0100H		; cpmldr runs at 0100H
SECTS	EQU	25		; # of sectors to load
;
;	I/O ports
;
MEMCTL	EQU	0F3H		; memory control MPU-B
FDC	EQU	0FDH		; IMSAI FIF FDC
LEDS	EQU	0FFH		; programmed output LED's
;
	JMP	COLD
;
FIF:	DB	21H		; FDC command/unit: read sector first drive
	DB	00H		; FDC result code
	DB	00H		; disk track high
	DB	00H		; disk track low
	DB	02H		; disk sector 2
	DB	BOOT AND 0FFH	; DMA address low
	DB	BOOT SHR 8	; DMA address high
;
;	begin the load operation
;
COLD:	LXI	SP,0FFH		; some space for the stack
	MVI	A,0C0H		; remove MPU-B address space
	OUT	MEMCTL
	MVI	A,10H		; setup FDC disk descriptor
	OUT	FDC
	MVI	A,FIF AND 0FFH
	OUT	FDC
	MVI	A,FIF SHR 8
	OUT	FDC
	LXI	B,2		; B = track 0, C = sector 2
	MVI	D,SECTS		; D = # of sectors to load
;
;	load the next sector
;
LSECT:	XRA	A		; tell FDC to execute the descriptor
	OUT	FDC
LS1:	LDA	FIF+1		; wait for FDC
	ORA	A
	JZ	LS1		; not done yet
	CPI	1		; result = 1?
	JZ	LS2		; yes, continue
	CMA			; no, complement error code
	OUT	LEDS		; output to LED's
	HLT			; and halt CPU
LS2:	DCR	D		; SECTS = SECTS - 1
	JZ	BOOT		; all done, head for cpmldr
	INR	C		; sector = sector + 1
	MOV	A,C
	STA	FIF+4		; set sector in disk descriptor
	XRA	A
	STA	FIF+1		; reset result
	PUSH	D
	LHLD	FIF+5		; get DMA address
	LXI	D,80H		; and increase it by 128
	DAD	D
	POP	D
	SHLD	FIF+5		; set new DMA address
	JMP	LSECT		; for next sector
;
	END			; of boot loader

;
;	CP/M 3 LDRBIOS for z80pack machines using SD-FDC
;
;	Copyright (C) 2024 by Udo Munk
;
; History:
; 30-JUN-2024 first public release
;
	CSEG
;
;	I/O ports
;
CONSTA	EQU	0		; console status port
CONDAT	EQU	1		; console data port
FDC	EQU	4		; port for the FDC
;
;	jump vector for individual subroutines
;	* needs to be implemented in loader BIOS
;
	JMP	BOOT		; * perform cold start initialization
	JMP	WBOOT		;   perform warm start initialization
	JMP	CONST		;   check for console input char ready
	JMP	CONIN		;   read console character in
	JMP	CONOUT		; * write console character out
	JMP	LIST		;   write list character out
	JMP	AUXOUT		;   write auxiliary character out
	JMP	AUXIN		;   read auxiliary character in
	JMP	HOME		; * move head to track 0 on selected disk
	JMP	SELDSK		; * select disk drive
	JMP	SETTRK		; * set track number
	JMP	SETSEC		; * set sector number
	JMP	SETDMA		; * set DMA address
	JMP	READ		; * read specified sector
	JMP	WRITE		;   write specified sector
	JMP	LISTST		;   return list status
	JMP	SECTRAN		; * translate logical to physical sector
	JMP	CONOST		;   return output status of console
	JMP	AUXIST		;   return input status of auxiliary
	JMP	AUXOST		;   return output status of auxiliary
	JMP	DEVTBL		;   return address of character I/O table
	JMP	DEVINI		;   initialize character I/O devices
	JMP	DRVTBL		;   return address of disk drive table
	JMP	MULTIO		;   set number of sectors to read/write
	JMP	FLUSH		;   flush deblocking buffers
	JMP	MOVE		;   memory to memory move
	JMP	TIME		;   time set/get signal
	JMP	SELMEM		;   select bank of memory
	JMP	SETBNK		;   specify bank for DMA operation
	JMP	XMOVE		;   set bank for memory DMA transfer
	JMP	0		;   reserved for system implementer
	JMP	0		;   reserved for future use
	JMP	0		;   reserved for future use
;
;	fixed data tables for IBM 3740 8" SD disk
;
;	disk parameter header
;
DPH0:	DW	TRANS		; sector translate table
	DB	0,0,0,0		; BDOS scratch area
	DB	0,0,0,0,0
	DB	0		; media flag
	DW	DPB0		; disk parameter block
	DW	0FFFEH		; checksum vector
	DW	0FFFEH		; allocation vector
	DW	0FFFEH		; directory buffer control block
	DW	0FFFFH		; DTABCB not used
	DW	0FFFFH		; hashing not used
	DB	0		; hash bank
;
;	sector translate table for IBM 3740 8" SD disk
;
TRANS:	DB	1,7,13,19	; sectors 1,2,3,4
	DB	25,5,11,17	; sectors 5,6,7,8
	DB	23,3,9,15	; sectors 9,10,11,12
	DB	21,2,8,14	; sectors 13,14,15,16
	DB	20,26,6,12	; sectors 17,18,19,20
	DB	18,24,4,10	; sectors 21,22,23,24
	DB	16,22		; sectors 25,26
;
;	disk parameter block for IBM 3740 8" SD disk
;
DPB0:	DW	26		; sectors per track
	DB	3		; block shift factor
	DB	7		; block mask
	DB	0		; extend mask
	DW	242		; disk size - 1
	DW	63		; directory max
	DB	192		; alloc 0
	DB	0		; alloc 1
	DW	16		; check size
	DW	2		; track offset
	DB	0,0		; physical sector size and shift
;
;	end of fixed tables
;
;	FDC command bytes
;
CMD:	DS	4
DDTRK	EQU	CMD+0		; track
DDSEC	EQU	CMD+1		; sector
DDLDMA	EQU	CMD+2		; DMA address low
DDHDMA	EQU	CMD+3		; DMA address high
;
;
BOOT:	MVI	A,10H		; setup FDC command
	OUT	FDC
	LXI	H,CMD
	MOV	A,L
	OUT	FDC
	MOV	A,H
	OUT	FDC
	RET
;
;	these are not implemented in a loader BIOS
;
WBOOT:
CONST:
CONIN:
LIST:
AUXOUT:
AUXIN:
WRITE:
LISTST:
CONOST:
AUXIST:
AUXOST:
DEVTBL:
DEVINI:
DRVTBL:
MULTIO:
FLUSH:
TIME:
SELMEM:
SETBNK:
MOVE:
XMOVE:
	RET
;
;	console character output from register C
;
CONOUT:	IN	CONSTA		; get status
	RLC			; test bit 7
	JC	CONOUT		; wait until transmitter ready
	MOV	A,C		; get character into accumulator
	OUT	CONDAT		; send to console
	RET
;
;	move to track 0 position on current disk
;
HOME:	MVI	C,0		; select track 0
	JMP	SETTRK
;
;	select disk given by register C
;
SELDSK:	LXI	H,0		; HL = error return code
	MOV	A,C		; get disk # to A
	ORA	A		; we boot from drive 0 only
	RNZ			; return error
	LXI	H,DPH0		; HL = disk parameter header
	RET
;
;	set track given by register C
;
SETTRK:	MOV	A,C		; get track to A
	STA	DDTRK		; and set in FDC command bytes
	RET
;
;	set sector given by register C
;
SETSEC:	MOV	A,C		; get sector to A
	STA	DDSEC		; and set in FDC command bytes
	RET
;
;	translate the sector given by BC using the
;	translate table given by DE
;
SECTRAN:XCHG			; HL = .TRANS
	DAD	B		; HL = .TRANS(sector)
	XCHG
	LDAX	D
	MOV	L,A		; L = TRANS(sector)
	MVI	H,0		; HL = TRANS(sector)
	RET
;
;	set DMA address given by register BC
;
SETDMA:	MOV	A,C		; low order address
	STA	DDLDMA		; set in FDC command bytes
	MOV	A,B		; high order address
	STA	DDHDMA		; set in FDC command bytes
	RET
;
;	perform read operation
;
READ:	MVI	A,20H		; read from drive 0
	OUT	FDC		; ask FDC to execute the command
	IN	FDC		; get status from FDC
	RET
;
	END			; of BIOS

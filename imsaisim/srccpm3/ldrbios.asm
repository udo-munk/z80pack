	TITLE	'CP/M 3 LDRBIOS for IMSAI 8080'
;
;	Copyright (C) 2019 by Udo Munk
;
; History:
; 23-OCT-2019 first public release
; 25-OCT-2019 initialize all character devices
;
	CSEG
;
;	I/O ports
;
TTY	EQU	02H		; tty data port
TTYS	EQU	03H		; tty status port
KBDS	EQU	05H		; keyboard status port
AUXS	EQU	23H		; auxiliary status port
USRS	EQU	25H		; user status port
SIO1C	EQU	08H		; control port for first SIO board
SIO2C	EQU	28H		; control port for second SIO board
PRINTER	EQU	0F6H		; IMSAI PTR-300 line printer
PIC8	EQU	0F7H		; interrupt priority controller
FDC	EQU	0FDH		; IMSAI FIF FDC
LEDS	EQU	0FFH		; programmed output LED's
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
;	IMSAI SIO-2 initialization
;
SIOSTR:	DB	0AEH,40H,0AEH,37H,0
;
;	end of fixed tables
;
;	IMSAI FIF
;
FIF:	DS	7		; storage for FIF disk descriptor
DDCMD	EQU	FIF+0		; unit/command
DDRES	EQU	FIF+1		; result code
DDHTRK	EQU	FIF+2		; track high
DDLTRK	EQU	FIF+3		; track low
DDSEC	EQU	FIF+4		; sector
DDLDMA	EQU	FIF+5		; DMA address low
DDHDMA	EQU	FIF+6		; DMA address high
;
;
BOOT:	LXI	H,SIOSTR	; initialize all SIO's
	MOV	A,M		; get next byte from init string
BO1:	OUT	TTYS		; output to SIO status ports
	OUT	KBDS
	OUT	AUXS
	OUT	USRS
	INX	H		; next one
	MOV	A,M
	ORA	A		; is it zero ?
	JNZ	BO1		; no, one more
	XRA	A		; turn off interrupts and
	OUT	SIO1C		; carrier detect on all SIO channels
	OUT	SIO2C
	STA	DDHTRK		; set track high in disk descriptor to 0
	MVI	A,00001001B	; disable all interrupts but channel 7
	OUT	PIC8
	IN	PRINTER		; get printer status, F4 is OK
	ANI	04H		; mask the telling bit
	JZ	B02		; skip init if bit off, might hang us
	MVI	A,80H		; else send init command
	OUT	PRINTER
B02:	MVI	A,10H		; setup FIF disk descriptor
	OUT	FDC
	LXI	H,FIF
	MOV	A,L
	OUT	FDC
	MOV	A,H
	OUT	FDC
	RET
;
;	these are not implemented in loader BIOS
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
CONOUT:	IN	TTYS		; get status
	RRC			; test bit 0
	JNC	CONOUT		; wait until transmitter ready
	MOV	A,C		; get character to A
	OUT	TTY		; send to tty
	RET
;
;	move to track 0 position of current drive
;
HOME:
	MVI	C,0		; select track 0
	JMP	SETTRK
;
;	select disk given by register C
;
SELDSK:	LXI	H,0		; HL = error return code
	MOV	A,C		; get disk # to A
	ORA	A		; we boot from drive 0 only
	RNZ			; return error
	INR	A		; disk unit 1
	STA	DDCMD		; set disk unit in FIF disk descriptor
	LXI	H,DPH0		; HL = disk parameter header
	RET
;
;	set track given by register C
;
SETTRK:	MOV	A,C		; get track to A
	STA	DDLTRK		; and set in FIF disk descriptor
	RET
;
;	set sector given by register C
;
SETSEC:	MOV	A,C		; get sector to A
	STA	DDSEC		; and set in FIF disk descriptor
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
	STA	DDLDMA		; set in FIF disk descriptor
	MOV	A,B		; high order address
	STA	DDHDMA		; set in FIF disk descriptor
	RET
;
;	perform read operation
;
READ:	LDA	DDCMD		; get unit/command
	ANI	0FH		; mask out command
	ORI	20H		; mask in read command
	STA	DDCMD		; set in FIF disk descriptor
	XRA	A		; zero A
	STA	DDRES		; reset result code
	OUT	FDC		; ask FDC to execute the disk descriptor
RD1:	LDA	DDRES		; wait for FDC
	ORA	A
	JZ	RD1		; not done yet
	ANI	0FEH		; status = 1 ?
	RZ			; if yes, return OK
	LDA	DDRES		; get status again
	CMA			; complement for LED's
	OUT	LEDS		; display the error code
	RET			; and return with error
;
	END			; of BIOS

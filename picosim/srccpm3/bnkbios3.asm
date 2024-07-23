;
;	CP/M 3 banked BIOS for z80pack machines using SD-FDC
;
;	Copyright (C) 2024 by Udo Munk
;
; History:
; 03-JUL-2024 first public release
; 07-JUL-2024 added RTC
; 14-JUL-2024 fixed bug, FCB one byte short
;
WARM	EQU	0		; BIOS warm start
BDOS	EQU	5		; BDOS entry
;
; 	BDOS functions
;
PRINT	EQU	9		; print string
OPEN	EQU	15		; open file
READS	EQU	20		; read sequential
DMA	EQU	26		; setup DMA
MULTI	EQU	44		; multi sector I/O
;
;	RTC commands
;
GETSEC	EQU	0		; get seconds from RTC
GETMIN	EQU	1		; get minutes from RTC
GETHOU	EQU	2		; get hours from RTC
GETDAL	EQU	3		; get days low from RTC
GETDAH	EQU	4		; get days high from RTC
;
;	character devices mode byte
;
mb$in	EQU	00000001B	; device may do input
mb$out	EQU	00000010B	; device may do output
mb$inou	EQU	mb$in+mb$out	; device may do both
mb$soft	EQU	00000100B	; software selectable baud rate
mb$ser	EQU	00001000B	; device may use protocol
mb$xon	EQU	00010000B	; XON/XOFF protocol enabled
;
;	character devices baud rate byte
;
baud$0	EQU	0		; Altair 88-SIO baudrate not software selectable
;
TPA	EQU	0100H		; start of TPA
;
;	I/O ports
;
TTY1	EQU	01H		; tty 1 data
TTY1S	EQU	00H		; tty 1 status
FDC	EQU	04H		; FDC
MMUSEL	EQU	40H		; MMU bank select
CLKCMD	EQU	41H		; RTC command
CLKDAT	EQU	42H		; RTC data
LEDS	EQU	0FFH		; frontpanel LED's
;
;	external references in SCB
;
	EXTRN	@civec, @covec, @aovec, @aivec, @lovec, @bnkbf
	EXTRN	@crdma, @crdsk, @fx, @resel, @vinfo, @usrcd
	EXTRN	@ermde, @date, @hour, @min, @sec, @mxtpa
;
	CSEG
;
;	jump vector for individual subroutines
;
	JMP	BOOT		; perform cold start initialization
WBOOTE:	JMP	WBOOT		; perform warm start initialization
	JMP	CONST		; check for console input char ready
	JMP	CONIN		; read console character in
	JMP	CONOUT		; write console character out
	JMP	LIST		; write list character out
	JMP	AUXOUT		; write auxiliary character out
	JMP	AUXIN		; read auxiliary character in
	JMP	HOME		; move head to track 0 on selected disk
	JMP	SELDSK		; select disk drive
	JMP	SETTRK		; set track number
	JMP	SETSEC		; set sector number
	JMP	SETDMA		; set DMA address
	JMP	READ		; read specified sector
	JMP	WRITE		; write specified sector
	JMP	LISTST		; return list status
	JMP	SECTRAN		; translate logical to physical sector
	JMP	CONOST		; return output status of console
	JMP	AUXIST		; return input status of auxiliary
	JMP	AUXOST		; return output status of auxiliary
	JMP	DEVTBL		; return address of character I/O table
	JMP	DEVINI		; initialize character I/O devices
	JMP	DRVTBL		; return address of disk drive table
	JMP	MULTIO		; set number of sectors to read/write
	JMP	FLUSH		; flush deblocking buffers
	JMP	MOVE		; memory to memory move
	JMP	TIME		; time set/get signal
	JMP	SELMEM		; select bank of memory
	JMP	SETBNK		; specify bank for DMA operation
	JMP	XMOVE		; set bank for memory DMA transfer
	JMP	0		; reserved for system implementer
	JMP	0		; reserved for future use
	JMP	0		; reserved for future use
;
;	drive table
;
DRIVES:	DW	DPH0
	DW	DPH1
	DW	DPH2
	DW	DPH3
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
	DW	0
;
;	disk parameter headers for IBM 3740 8" SD disks
;
DPH0:	DW	TRANS		; sector translate table
	DB	0,0,0,0		; BDOS scratch area
	DB	0,0,0,0,0
	DB	0		; media flag
	DW	DPBFD		; disk parameter block
	DW	0FFFEH		; checksum vector
	DW	0FFFEH		; allocation vector
	DW	0FFFEH		; directory buffer control block
	DW	0FFFFH		; DTABCB not used
	DW	0FFFFH		; hashing not used
	DB	0		; hash bank
DPH1:	DW	TRANS		; sector translate table
	DB	0,0,0,0		; BDOS scratch area
	DB	0,0,0,0,0
	DB	0		; media flag
	DW	DPBFD		; disk parameter block
	DW	0FFFEH		; checksum vector
	DW	0FFFEH		; allocation vector
	DW	0FFFEH		; directory buffer control block
	DW	0FFFFH		; DTABCB not used
	DW	0FFFFH		; hashing not used
	DB	0		; hash bank
DPH2:	DW	TRANS		; sector translate table
	DB	0,0,0,0		; BDOS scratch area
	DB	0,0,0,0,0
	DB	0		; media flag
	DW	DPBFD		; disk parameter block
	DW	0FFFEH		; checksum vector
	DW	0FFFEH		; allocation vector
	DW	0FFFEH		; directory buffer control block
	DW	0FFFFH		; DTABCB not used
	DW	0FFFFH		; hashing not used
	DB	0		; hash bank
DPH3:	DW	TRANS		; sector translate table
	DB	0,0,0,0		; BDOS scratch area
	DB	0,0,0,0,0
	DB	0		; media flag
	DW	DPBFD		; disk parameter block
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
DPBFD:	DW	26		; sectors per track
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
;	FDC command bytes
;
CMD:	DS	4
DDTRK	EQU	CMD+0		; track
DDSEC	EQU	CMD+1		; sector
DDLDMA	EQU	CMD+2		; DMA address low
DDHDMA	EQU	CMD+3		; DMA address high
;
;	character device table
;
CHRTBL:	DB	'TTY1  '
	DB	mb$inou+mb$ser
	DB	baud$0
;
	DB	0
;
;	FCB for loading CCP
;
CCPFCB:	DB	1,'CCP     COM',0,0,0,0
	DB	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
CCPCR:	DB	0
CCPREC:	DB	0,0,0
;
;	BIOS error messages
;
OPNERR:	DB	'Open error CCP.COM',13,10,'$'
RDRERR:	DB	'Read error CCP.COM',13,10,'$'
;
BANK:	DB	0		; bank to select for DMA
SDISK:	DB	0		; selected disk
;
	DS	32		; small stack
STACK:
;
	DSEG
;
SIGNON:	DB	13,10
	DB	'Banked BIOS V1.2',13,10
	DB	'Copyright (C) 2024 Udo Munk',13,10,13,10
	DB	0
;
;	get control from cold start loader
;	and initialize system
;
BOOT:	LXI	SP,STACK
;
	LXI	H,8000H		; device 0
	SHLD	@civec		; CONIN:=TTY1
	SHLD	@covec		; CONOUT:=TTY1
	LXI	H,0
	SHLD	@aivec		; AUXIN:=NULL
	SHLD	@aovec		; AUXOUT:=NULL
	SHLD	@lovec		; LST:=NULL
;
	MVI	A,10H		; setup FDC command
	OUT	FDC
	LXI	H,CMD
	MOV	A,L
	OUT	FDC
	MOV	A,H
	OUT	FDC
;
	LXI	H,SIGNON	; print signon
BOOT1:	MOV	A,M		; get next message bye
	ORA	A		; is it zero ?
	JZ	BOOT2		; yes, done
	MOV	C,A		; no, print character on console
	PUSH	H
	CALL	CONOUT
	POP	H
	INX	H		; and do next
	JMP	BOOT1
;
BOOT2:	JMP	WBOOT
;
	CSEG
;
;	get control when a warm start occurs
;
WBOOT:	LXI	SP,STACK
	MVI	A,1		; select memory bank 1
	CALL	SELMEM
;
	MVI	A,0C3H		; JMP instruction
	STA	WARM
	LXI	H,WBOOTE	; warm boot entry point
	SHLD	WARM+1
	STA	BDOS
	LHLD	@mxtpa		; BDOS entry point
	SHLD	BDOS+1
;
	XRA	A		; initialize FCB for CCP
	STA	CCPFCB+12
	STA	CCPFCB+15
	STA	CCPCR
	STA	CCPREC
	STA	CCPREC+1
	STA	CCPREC+2
	LXI	D,CCPFCB	; open file CCP.COM
	MVI	C,OPEN
	CALL	BDOS
	LXI	D,OPNERR
	INR	A
	JZ	CPPERR		; file open error
	LXI	D,TPA		; setup DMA to TPA
	MVI	C,DMA
	CALL	BDOS
	LXI	D,128		; read up to 16 KB
	MVI	C,MULTI
	CALL	BDOS
	LXI	D,CCPFCB	; read the CCP into memory
	MVI	C,READS
	CALL	BDOS
	LXI	D,RDRERR
	INR	A
	JNZ	TPA		; start CCP
;
CPPERR:	MVI	C,PRINT		; error loading CCP, print error message
	CALL	BDOS
	HLT			; and halt the machine
;
;	I/O drivers for character devices
;
DEVTBL:	LXI	H,CHRTBL	; return address of character I/O table
	RET
;
;	initialize character I/O devices
;
DEVINI: RET
;
;	console input status
;
CONST:	LHLD	@civec		; get console in vector
	MVI	A,80H		; test for device 0
	ANA	H
	JZ	CONS1		; if not set try next
	CALL	TTY1IS		; test tty 1 input status
	JZ	DEVRDY		; done if device ready
CONS1:	XRA	A		; no device ready
	RET
;
;	console input
;
CONIN:	LHLD	@civec		; get console in vector
	MVI	A,80H		; test for device 0
	ANA	H
	JZ	CONI1		; if not set try next
	CALL	TTY1IS		; test tty 1 input status
	JZ	TTY1IN		; ready, get input from tty 1
CONI1:	JMP	CONIN		; no device ready, try again
;
;	console output status
;
CONOST:	LHLD	@covec		; get console out vector
	MVI	A,80H		; test for device 0
	ANA	H
	JNZ	CONOS1		; if not set try next
	CALL	TTY1OS		; test tty 1 output status
	JNZ	DEVNRY		; if device not ready
CONOS1:	MVI	A,0FFH		; all output devices ready
	RET
;
;	console output
;
CONOUT: LHLD	@covec		; get console out vector
	MVI	A,80H		; test for device 0
	ANA	H
	CNZ	TTY1OU		; if set call tty 1 output
	RET
;
;	list output status
;
LISTST:	JMP	DEVNRY		; no printer, never ready
;
;	list output
;
LIST:	RET			; no printer
;
;	auxiliary input status
;
AUXIST:	JMP	DEVNRY		; no auxiliary, never ready
;
;	auxiliary input
;
AUXIN:	MVI	A,01AH		; we have no auxiliary
	RET			; so return CTL-Z
;
;	auxiliary output status
;
AUXOST:	JMP	DEVNRY		; no auxiliary, never ready
;
;	auxiliary output
;
AUXOUT: RET			; no auxiliary
;
;	tty 1 input status
;
TTY1IS:	IN	TTY1S		; get status
	ANI	01H		; mask bit
	RET
;
;	tty 1 input
;
TTY1IN:	IN	TTY1		; get character
	RET
;
;	tty 1 output status
;
TTY1OS:	IN	TTY1S		; get status
	ANI	80H		; mask bit
	RET
;
;	tty 1 output
;
TTY1OU:	IN	TTY1S		; get status
	RLC			; test bit 7
	JC	TTY1OU		; wait until transmitter ready
	MOV	A,C		; get character to A
	OUT	TTY1		; send to tty 1
	RET
;
;	return with device ready
;
DEVRDY:	MVI	A,0FFH
	RET
;
;	return with device not ready
;
DEVNRY:	XRA	A
	RET
;
;	I/O drivers for disks
;
DRVTBL:	LXI	H,DRIVES	; return address of disk drive table
	RET
;
	DSEG
;
;	move to track 0 position of current drive
;
HOME:	MVI	C,0		; select track 0
	JMP	SETTRK
;
	CSEG
;
;	select disk given by register C
;
SELDSK: LXI	H,0		; HL = error return code
	MOV	A,C		; get disk # to A
	CPI	0		; disk 0 ?
	JZ	SEL0		; select disk 0
	CPI	1		; disk 1 ?
	JZ	SEL1		; select disk 1
	CPI	2		; disk 2 ?
	JZ	SEL2		; select disk 2
	CPI	3		; disk 3 ?
	JZ	SEL3		; select disk 3
	RET			; else return with error
SEL0:	STA	SDISK
	LXI	H,DPH0		; HL = disk parameter header disk 0
	RET
SEL1:	STA	SDISK
	LXI	H,DPH1		; HL = disk parameter header disk 1
	RET
SEL2:	STA	SDISK
	LXI	H,DPH2		; HL = disk parameter header disk 2
	RET
SEL3:	STA	SDISK
	LXI	H,DPH3		; HL = disk parameter header disk 3
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
	DSEG
;
;	translate the sector given by BC using
;	the translation table given by DE
;
SECTRAN:MOV	A,D		; do we have a translation table ?
	ORA	E
	JNZ	SECT1		; yes, translate
	MOV	L,C		; no, return untranslated
	MOV	H,B
	INR	L		; sector no. start with 1
	RET
SECT1:	XCHG			; HL = .TRANS
	DAD	B		; HL = .TRANS(sector)
	XCHG
	LDAX	D
	MOV	L,A		; L = TRANS(sector)
	MVI	H,0		; HL = TRANS(sector)
	RET			; with value in HL
;
	CSEG
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
READ:	LDA	BANK		; switch to saved bank
	OUT	MMUSEL
	LDA	SDISK		; get disk
	ORI	20H		; mask in read command
	OUT	FDC		; ask FDC to execute the command
	XRA	A		; reselect bank 0
	OUT	MMUSEL
	IN	FDC		; get FDC status
	ORA	A		; is it zero ?
	RZ			; return if OK
	CMA			; complement for LED's
	OUT	LEDS		; display the error code
	MVI	A,1		; nonrecoverable error
	RET			; return with error
;
;	perform write operation
;
WRITE:	LDA	BANK		; switch to saved bank
	OUT	MMUSEL
	LDA	SDISK		; get disk
	ORI	40H		; mask in write command
	OUT	FDC		; ask FDC to execute the command
	XRA	A		; reselect bank 0
	OUT	MMUSEL
	IN	FDC		; get FDC status
	ORA	A		; is it zero ?
	RZ			; return if OK
	CMA			; complement for LED's
	OUT	LEDS		; display the error code
	MVI	A,1		; nonrecoverable error
	RET			; return with error
;
;	set count of consecutive sectors
;	for read or write
;
MULTIO:	XRA	A		; do nothing
	RET
;
;	force physical buffer flushing
;
FLUSH:	XRA	A		; no user deblocking
	RET
;
;	memory to memory block move
;	HL = destination address
;	DE = source address
;	BC = count
;
MOVE:	LDAX	D
	MOV	M,A
	INX	D
	INX	H
	DCX	B
	MOV	A,B
	ORA	C
	JNZ	MOVE
	RET
;
;	select memory bank
;
SELMEM:	OUT	MMUSEL
	RET
;
;	specify memory bank for DMA operation
;
SETBNK:	STA	BANK
	RET
;
;	set banks for following MOVE
;
XMOVE: RET			; no memory to memory DMA available
;
;	get/set time
;
TIME:	MOV	A,C
	CPI	0FFH
	RZ			; can't set time
	MVI	A,GETSEC	; get seconds
	OUT	CLKCMD
	IN	CLKDAT
	STA	@sec
	MVI	A,GETMIN	; get minutes
	OUT	CLKCMD
	IN	CLKDAT
	STA	@min
	MVI	A,GETHOU	; get hours
	OUT	CLKCMD
	IN	CLKDAT
	STA	@hour
	MVI	A,GETDAL	; get day
	OUT	CLKCMD
	IN	CLKDAT
	STA	@date
	MVI	A,GETDAH
	OUT	CLKCMD
	IN	CLKDAT
	STA	@date+1
	RET
;
	END			; of BIOS

	TITLE	'CP/M 3 banked BIOS for IMSAI 8080'
;
;	Copyright (C) 2019-2024 by Udo Munk
;
; History:
; 23-OCT-2019 first public release
; 24-Oct-2019 get time/date from RTC
; 25-OCT-2019 add character device table
; 28-OCT-2019 add all character devices with I/O redirection
; 01-NOV-2019 add more complete character device mode byte
; 17-NOV-2019 handle result codes from FIF FDC
; 01-APR-2020 moved RTC ports
; 15-JUL-2024 fixed bug, FCB one byte short
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
baud$0	EQU	0		; IMSAI SIO-2 baudrate not software selectable
;
TPA	EQU	0100H		; start of TPA
;
;	I/O ports
;
TTY1	EQU	02H		; tty 1 data
TTY1S	EQU	03H		; tty 1 status
KBD	EQU	04H		; keyboard data
KBDS	EQU	05H		; keyboard status
TTY2	EQU	22H		; tty 2 data
TTY2S	EQU	23H		; tty 2 status
MODEM	EQU	24H		; modem data
MODEMS	EQU	25H		; modem status
MMUSEL	EQU	40H		; MMU bank select
CLKCMD	EQU	41H		; RTC command
CLKDAT	EQU	42H		; RTC data
PRINTER	EQU	0F6H		; IMSAI PTR-300 line printer
FDC	EQU	0FDH		; IMSAI FIF FDC
LEDS	EQU	0FFH		; programmed output LED's
;
;	IMSAI VIO
;
VIOINIT	EQU	0F800H		; init call in VIO firmware
VIOOUT	EQU	0F803H		; character output call in VIO firmware
VIOTST	EQU	0FFFDH		; firmware signature with string VI0
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
	DW	DPH8
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
;	disk parameter header for 4 MB harddisk
;
DPH8:	DW	0		; sector translation table
	DB	0,0,0,0		; bdos scratch area
	DB	0,0,0,0,0
	DB	0		; media flag
	DW	DPBHD		; disk parameter block
	DW	0		; checksum vector
	DW	0FFFEH		; allocation vector
	DW	0FFFEH		; directory buffer control block
	DW	0FFFFH		; dtabcb not used
	DW	0FFFEH		; hashing
	DB	0		; hash bank
;
;	disk parameter block for 4 MB harddisk
;
DPBHD:	DW	128		; sectors per track
	DB	4		; block shift factor
	DB	15		; block mask
	DB	0		; extent mask
	DW	2039		; disk size - 1
	DW	1023		; directory max
	DB	255		; alloc 0
	DB	255		; alloc 1
	DW	8000H		; check size
	DW	0		; track offset
	DB	0,0		; physical sector size and shift
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
;	character device table
;
CHRTBL:	DB	'TTY1  '
	DB	mb$inou+mb$ser
	DB	baud$0
;
	DB	'TTY2  '
	DB	mb$inou+mb$ser
	DB	baud$0
;
	DB	'CRT   '
	DB	mb$inou
	DB	baud$0
;
	DB	'MODEM '
	DB	mb$inou+mb$ser
	DB	baud$0
;
	DB	'LPT   '
	DB	mb$out
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
VIOF:	DB	0		; flag if VIO board available
;
	DS	32		; small stack
STACK:
;
	DSEG
;
SIGNON:	DB	13,10
	DB	'IMSAI 8080 banked BIOS V1.8,',13,10
	DB	'Copyright (C) 2019-2024 Udo Munk',13,10,13,10
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
	LXI	H,4000H		; device 1
	SHLD	@aivec		; AUXIN:=TTY2
	SHLD	@aovec		; AUXOUT:=TTY2
	LXI	H,0800H		; device 4
	SHLD	@lovec		; LST:=LPT
;
	LXI	H,VIOTST	; check if VIO board available
	MOV	A,M		; verify first signature byte
	CPI	'V'
	JNZ	BOOT1
	INX	H		; verify second signature byte
	MOV	A,M
	CPI	'I'
	JNZ	BOOT1
	INX	H		; verify third signature byte
	MOV	A,M
	CPI	'0'
	JNZ	BOOT1
	MVI	A,0FFH		; VIO firmware found, set flag
	STA	VIOF
	CALL	VIOINIT		; initialize the VIO board
;	LXI	H,2000H		; device 2
;	SHLD	@civec		; CONIN:=CRT
;	SHLD	@covec		; CONOUT:=CRT
;
BOOT1:	XRA	A		; set track high in disk descriptor to 0
	STA	DDHTRK
	MVI	A,10H		; setup FIF disk descriptor
	OUT	FDC
	LXI	H,FIF
	MOV	A,L
	OUT	FDC
	MOV	A,H
	OUT	FDC
;
	LXI	H,SIGNON	; print signon
BOOT2:	MOV	A,M		; get next message bye
	ORA	A		; is it zero ?
	JZ	BOOT3		; yes, done
	MOV	C,A		; no, print character on console
	PUSH	H
	CALL	CONOUT
	POP	H
	INX	H		; and do next
	JMP	BOOT2
;
BOOT3:	JMP	WBOOT
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
	JNZ	DEVRDY		; done if device ready
CONS1:	MVI	A,40H		; test for device 1
	ANA	H
	JZ	CONS2		; if not set try next
	CALL	TTY2IS		; test tty 2 input status
	JNZ	DEVRDY		; done if device ready
CONS2:	MVI	A,20H		; test for device 2
	ANA	H
	JZ	CONS3		; if not set try next
	CALL	KBDIS		; test keyboard input status
	JNZ	DEVRDY		; done if device ready
CONS3:	MVI	A,10H		; test for device 3
	ANA	H
	JZ	CONS4		; if not set try next
	CALL	MODIS		; test modem input status
	JNZ	DEVRDY
CONS4:	XRA	A		; no device ready
	RET
;
;	console input
;
CONIN:	LHLD	@civec		; get console in vector
	MVI	A,80H		; test for device 0
	ANA	H
	JZ	CONI1		; if not set try next
	CALL	TTY1IS		; test tty 1 input status
	JNZ	TTY1IN		; ready, get input from tty 1
CONI1:	MVI	A,40H		; test for device 1
	ANA	H
	JZ	CONI2		; if not set try next
	CALL	TTY2IS		; test tty 2 input status
	JNZ	TTY2IN		; ready, get input from tty 2
CONI2:	MVI	A,20H		; test for device 2
	ANA	H
	JZ	CONI3		; if not set try next
	CALL	KBDIS		; test keyboard input status
	JNZ	KBDIN		; read, get input status from keyboard
CONI3:	MVI	A,10H		; test for device 3
	ANA	H
	JZ	CONI4		; if not set try next
	CALL	MODIS		; test modem input status
	JNZ	MODIN		; ready, get input from modem
CONI4:	JMP	CONIN		; no device ready, try again
;
;	console output status
;
CONOST:	LHLD	@covec		; get console out vector
	MVI	A,80H		; test for device 0
	ANA	H
	JNZ	CONOS1		; if not set try next
	CALL	TTY1OS		; test tty 1 output status
	JZ	DEVNRY		; if device not ready
CONOS1:	MVI	A,40H		; test for device 1
	ANA	H
	JNZ	CONOS2		; if not set try next
	CALL	TTY2OS		; test tty 2 output status
	JZ	DEVNRY		; if device not ready
CONOS2:	MVI	A,10H		; test for device 3
	ANA	H
	JNZ	CONOS3		; if not set try next
	CALL	MODOS		; test modem output status
	JZ	DEVNRY		; if device not ready
CONOS3:	MVI	A,08H		; test for device 4
	ANA	H
	JNZ	CONOS4		; if not set try next
	CALL	LPTST		; test printer output status
	JZ	DEVNRY		; if device not ready
CONOS4:	MVI	A,0FFH		; all output devices ready
	RET
;
;	console output
;
CONOUT: LHLD	@covec		; get console out vector
	MVI	A,80H		; test for device 0
	ANA	H
	CNZ	TTY1OU		; if set call tty 1 output
	MVI	A,40H		; test for device 1
	ANA	H
	CNZ	TTY2OU		; if set call tty 2 output
	MVI	A,20H		; test for device 2
	ANA	H
	CNZ	CRTOUT		; if set call crt output
	MVI	A,10H		; test for device 3
	ANA	H
	CNZ	MODOUT		; if set call modem output
	MVI	A,08H		; test for device 4
	ANA	H
	CNZ	LPTOUT		; if set call lpt output
	RET
;
;	list output status
;
LISTST:	LHLD	@lovec		; get list out vector
	MVI	A,80H		; test for device 0
	ANA	H
	JNZ	LISTS1		; if not set try next
	CALL	TTY1OS		; test tty 1 output status
	JZ	DEVNRY		; if device not ready
LISTS1:	MVI	A,40H		; test for device 1
	ANA	H
	JNZ	LISTS2		; if not set try next
	CALL	TTY2OS		; test tty 2 output status
	JZ	DEVNRY		; if device not ready
LISTS2:	MVI	A,10H		; test for device 3
	ANA	H
	JNZ	LISTS3		; if not set try next
	CALL	MODOS		; test modem output status
	JZ	DEVNRY		; if device not ready
LISTS3:	MVI	A,08H		; test for device 4
	ANA	H
	JNZ	LISTS4		; if not set try next
	CALL	LPTST		; test printer output status
	JZ	DEVNRY		; if device not ready
LISTS4:	MVI	A,0FFH		; all output devices ready
	RET
;
;	list output
;
LIST:	LHLD	@lovec		; get list out vector
	MVI	A,80H		; test for device 0
	ANA	H
	CNZ	TTY1OU		; if set call tty 1 output
	MVI	A,40H		; test for device 1
	ANA	H
	CNZ	TTY2OU		; if set call tty 2 output
	MVI	A,20H		; test for device 2
	ANA	H
	CNZ	CRTOUT		; if set call crt output
	MVI	A,10H		; test for device 3
	ANA	H
	CNZ	MODOUT		; if set call modem output
	MVI	A,08H		; test for device 4
	ANA	H
	CNZ	LPTOUT		; if set call lpt output
	RET
;
;	auxiliary input status
;
AUXIST:	LHLD	@aivec		; get auxiliary in vector
	MVI	A,80H		; test for device 0
	ANA	H
	JZ	AUXS1		; if not set try next
	CALL	TTY1IS		; test tty 1 input status
	JNZ	DEVRDY		; done if device ready
AUXS1:	MVI	A,40H		; test for device 1
	ANA	H
	JZ	AUXS2		; if not set try next
	CALL	TTY2IS		; test tty 2 input status
	JNZ	DEVRDY		; done if device ready
AUXS2:	MVI	A,20H		; test for device 2
	ANA	H
	JZ	AUXS3		; if not set try next
	CALL	KBDIS		; test keyboard input status
	JNZ	DEVRDY		; done if device ready
AUXS3:	MVI	A,10H		; test for device 3
	ANA	H
	JZ	AUXS4		; if not set try next
	CALL	MODIS		; test modem input status
	JNZ	DEVRDY
AUXS4:	XRA	A		; no device ready
	RET
;
;	auxiliary input
;
AUXIN:	LHLD	@aivec		; get auxiliary in vector
	MVI	A,80H		; test for device 0
	ANA	H
	JZ	AUXI1		; if not set try next
	CALL	TTY1IS		; test tty 1 input status
	JNZ	TTY1IN		; ready, get input from tty 1
AUXI1:	MVI	A,40H		; test for device 1
	ANA	H
	JZ	AUXI2		; if not set try next
	CALL	TTY2IS		; test tty 2 input status
	JNZ	TTY2IN		; ready, get input from tty 2
AUXI2:	MVI	A,20H		; test for device 2
	ANA	H
	JZ	AUXI3		; if not set try next
	CALL	KBDIS		; test keyboard input status
	JNZ	KBDIN		; read, get input status from keyboard
AUXI3:	MVI	A,10H		; test for device 3
	ANA	H
	JZ	AUXI4		; if not set try next
	CALL	MODIS		; test modem input status
	JNZ	MODIN		; ready, get input from modem
AUXI4:	JMP	AUXIN		; no device ready, try again
;
;	auxiliary output status
;
AUXOST:	LHLD	@aovec		; get auxiliary out vector
	MVI	A,80H		; test for device 0
	ANA	H
	JNZ	AUXOS1		; if not set try next
	CALL	TTY1OS		; test tty 1 output status
	JZ	DEVNRY		; if device not ready
AUXOS1:	MVI	A,40H		; test for device 1
	ANA	H
	JNZ	AUXOS2		; if not set try next
	CALL	TTY2OS		; test tty 2 output status
	JZ	DEVNRY		; if device not ready
AUXOS2:	MVI	A,10H		; test for device 3
	ANA	H
	JNZ	AUXOS3		; if not set try next
	CALL	MODOS		; test modem output status
	JZ	DEVNRY		; if device not ready
AUXOS3:	MVI	A,08H		; test for device 4
	ANA	H
	JNZ	AUXOS4		; if not set try next
	CALL	LPTST		; test printer output status
	JZ	DEVNRY		; if device not ready
AUXOS4:	MVI	A,0FFH		; all output devices ready
	RET
;
;	auxiliary output
;
AUXOUT: LHLD	@aovec		; get auxiliary out vector
	MVI	A,80H		; test for device 0
	ANA	H
	CNZ	TTY1OU		; if set call tty 1 output
	MVI	A,40H		; test for device 1
	ANA	H
	CNZ	TTY2OU		; if set call tty 2 output
	MVI	A,20H		; test for device 2
	ANA	H
	CNZ	CRTOUT		; if set call crt output
	MVI	A,10H		; test for device 3
	ANA	H
	CNZ	MODOUT		; if set call modem output
	MVI	A,08H		; test for device 4
	ANA	H
	CNZ	LPTOUT		; if set call lpt output
	RET
;
;	tty 1 input status
;
TTY1IS:	IN	TTY1S		; get status
	ANI	02H		; mask bit
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
	ANI	01H		; mask bit
	RET
;
;	tty 1 output
;
TTY1OU:	IN	TTY1S		; get status
	RRC			; test bit 0
	JNC	TTY1OU		; wait until transmitter ready
	MOV	A,C		; get character to A
	OUT	TTY1		; send to tty 1
	RET
;
;	tty 2 input status
;
TTY2IS:	IN	TTY2S		; get status
	ANI	02H		; mask bit
	RET
;
;	tty 2 input
;
TTY2IN:	IN	TTY2		; get character
	RET
;
;	tty 2 output status
;
TTY2OS:	IN	TTY2S		; get status
	ANI	01H		; mask bit
	RET
;
;	tty 2 output
;
TTY2OU:	IN	TTY2S		; get status
	RRC			; test bit 0
	JNC	TTY2OU		; wait until transmitter ready
	MOV	A,C		; get character to A
	OUT	TTY2		; send to tty 2
	RET
;
;	keyboard input status
;
KBDIS:	IN	KBDS		; get status
	ANI	02H		; mask bit
	RET
;
;	keyboard input
;
KBDIN:	IN	KBD		; get character
	RET
;
;	crt output
;
CRTOUT:	LDA	VIOF		; VIO board available ?
	ORA	A
	RZ			; if not do nothing
	MOV	A,C		; get character into A
	CALL	VIOOUT		; and output via VIO firmware
	RET
;
;	modem input status
;
MODIS:	IN	MODEMS		; get status
	ANI	02H		; mask bit
	RET
;
;	modem input
;
MODIN:	IN	MODEM		; get character
	RET
;
;	modem output status
;
MODOS:	IN	MODEMS		; get status
	ANI	01H		; mask bit
	RET
;
;	modem output
;
MODOUT:	IN	MODEMS		; get status
	RRC			; test bit 0
	JNC	MODOUT		; wait until transmitter ready
	MOV	A,C		; get character to A
	OUT	MODEM		; send to modem
	RET
;
;	lpt output status
;
LPTST:	IN	PRINTER		; get printer status
	CPI	0FFH		; no printer ?
	RZ			; return not ready
	ANI	04H		; check ready bit
	RET
;
;	lpt output
;
LPTOUT:	IN	PRINTER		; get printer status
	CPI	0FFH		; no printer ?
	RZ
	ANI	04H		; check ready bit
	JZ	LPTOUT		; not ready, try again
	MOV	A,C		; get character into A
	OUT	PRINTER		; send to printer
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
	CPI	8		; harddisk ?
	JZ	SEL8		; select harddisk
	RET			; else return with error
SEL0:	MVI	A,1		; disk unit 1
	STA	DDCMD		; set disk unit in FIF disk descriptor
	LXI	H,DPH0		; HL = disk parameter header disk 0
	RET
SEL1:	MVI	A,2		; disk unit 2
	STA	DDCMD		; set disk unit in FIF disk descriptor
	LXI	H,DPH1		; HL = disk parameter header disk 1
	RET
SEL2:	MVI	A,4		; disk unit 3
	STA	DDCMD		; set disk unit in FIF disk descriptor
	LXI	H,DPH2		; HL = disk parameter header disk 2
	RET
SEL3:	MVI	A,8		; disk unit 4
	STA	DDCMD		; set disk unit in FIF disk descriptor
	LXI	H,DPH3		; HL = disk parameter header disk 3
	RET
SEL8:	MVI	A,15		; disk unit 15
	STA	DDCMD		; set disk unit in FIF disk descriptor
	LXI	H,DPH8		; HL = disk parameter header harddisk
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
	STA	DDLDMA		; set in FIF disk descriptor
	MOV	A,B		; high order address
	STA	DDHDMA		; set in FIF disk descriptor
	RET
;
;	perform read operation
;
READ:	LDA	BANK		; switch to saved bank
	OUT	MMUSEL
	LDA	DDCMD		; get unit/command
	ANI	0FH		; mask out command
	ORI	20H		; mask in read command
	STA	DDCMD		; set in FIF disk descriptor
	XRA	A		; zero A
	STA	DDRES		; reset result code
	OUT	FDC		; ask FDC to execute the disk descriptor
RD1:	LDA	DDRES		; wait for FDC
	ORA	A
	JZ	RD1		; not done yet
	XRA	A		; reselect bank 0
	OUT	MMUSEL
	LDA	DDRES		; get status again
	ANI	0FEH		; status = 1 ?
	RZ			; if yes, return OK
	LDA	DDRES		; get status again
	CMA			; complement for LED's
	OUT	LEDS		; display the error code
	MVI	A,1		; nonrecoverable error
	RET			; return with error
;
;	perform write operation
;
WRITE:	LDA	BANK		; switch to saved bank
	OUT	MMUSEL
	LDA	DDCMD		; get unit/command
	ANI	0FH		; mask out command
	ORI	10H		; mask in write command
	STA	DDCMD		; set in FIF disk descriptor
	XRA	A		; zero A
	STA	DDRES		; reset result code
	OUT	FDC		; ask FDC to execute the disk descriptor
WR1:	LDA	DDRES		; wait for FDC
	ORA	A
	JZ	WR1		; not done yet
	XRA	A		; reselect bank 0
	OUT	MMUSEL
	LDA	DDRES		; get status again
	ANI	0FEH		; status = 1 ?
	RZ			; if yes, return OK
	LDA	DDRES		; get status again
	CMA			; complement for LED's
	OUT	LEDS		; display the error code
	LDA	DDRES		; get status again
	CPI	0A2H		; hardware write protected ?
	JZ	DISKRO
	CPI	0A3H		; software write protected ?
	JZ	DISKRO
	MVI	A,1		; no, nonrecoverable error
	RET			; return with error
DISKRO:	MVI	A,2		; disk R/O error
	RET
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

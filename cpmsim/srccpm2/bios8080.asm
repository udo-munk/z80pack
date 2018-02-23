;	8080 CBIOS for Z80-Simulator
;
;	Copyright (C) 1988-2007 by Udo Munk
;
MSIZE	EQU	64		;cp/m version memory size in kilobytes
;
;	"bias" is address offset from 3400H for memory systems
;	than 16K (referred to as "b" throughout the text).
;
BIAS	EQU	(MSIZE-20)*1024
CCP	EQU	3400H+BIAS	;base of ccp
BDOS	EQU	CCP+806H	;base of bdos
BIOS	EQU	CCP+1600H	;base of bios
NSECTS	EQU	(BIOS-CCP)/128	;warm start sector count
CDISK	EQU	0004H		;current disk number 0=A,...,15=P
IOBYTE	EQU	0003H		;intel i/o byte
;
;	I/O ports
;
CONSTA	EQU	0		;console status port
CONDAT	EQU	1		;console data port
PRTSTA	EQU	2		;printer status port
PRTDAT	EQU	3		;printer data port
AUXDAT	EQU	5		;auxiliary data port
FDCD	EQU	10		;fdc-port: # of drive
FDCT	EQU	11		;fdc-port: # of track
FDCS	EQU	12		;fdc-port: # of sector
FDCOP	EQU	13		;fdc-port: command
FDCST	EQU	14		;fdc-port: status
DMAL	EQU	15		;dma-port: dma address low
DMAH	EQU	16		;dma-port: dma address high
;
	ORG	1600H		;origin of this program
;
;	jump vector for individual subroutines
;
	JMP	BOOT		;cold start
WBOOTE: JMP	WBOOT		;warm start
	JMP	CONST		;console status
	JMP	CONIN		;console character in
	JMP	CONOUT		;console character out
	JMP	LIST		;list character out
	JMP	PUNCH		;punch character out
	JMP	READER		;reader character in
	JMP	HOME		;move head to home position
	JMP	SELDSK		;select disk
	JMP	SETTRK		;set track number
	JMP	SETSEC		;set sector number
	JMP	SETDMA		;set dma address
	JMP	READ		;read disk
	JMP	WRITE		;write disk
	JMP	LISTST		;return list status
	JMP	SECTRAN		;sector translate
;
;	fixed data tables for four-drive standard
;	IBM-compatible 8" SD disks
;
;	disk parameter header for disk 00
DPBASE:	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK00,ALL00
;	disk parameter header for disk 01
	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK01,ALL01
;	disk parameter header for disk 02
	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK02,ALL02
;	disk parameter header for disk 03
	DW	TRANS,0000H
	DW	0000H,0000H
	DW	DIRBF,DPBLK
	DW	CHK03,ALL03
;
;	sector translate vector for the IBM 8" SD disks
;
TRANS:	DB	1,7,13,19	;sectors 1,2,3,4
	DB	25,5,11,17	;sectors 5,6,7,8
	DB	23,3,9,15	;sectors 9,10,11,12
	DB	21,2,8,14	;sectors 13,14,15,16
	DB	20,26,6,12	;sectors 17,18,19,20
	DB	18,24,4,10	;sectors 21,22,23,24
	DB	16,22		;sectors 25,26
;
;	disk parameter block, common to all IBM 8" SD disks
;
DPBLK:  DW	26		;sectors per track
	DB	3		;block shift factor
	DB	7		;block mask
	DB	0		;extent mask
	DW	242		;disk size-1
	DW	63		;directory max
	DB	192		;alloc 0
	DB	0		;alloc 1
	DW	16		;check size
	DW	2		;track offset
;
;	fixed data tables for 4MB harddisks
;
;	disk parameter header
HDB1:	DW	0000H,0000H
	DW	0000H,0000H
	DW	DIRBF,HDBLK
	DW	CHKHD1,ALLHD1
HDB2:	DW	0000H,0000H
	DW	0000H,0000H
	DW	DIRBF,HDBLK
	DW	CHKHD2,ALLHD2
;
;       disk parameter block for harddisk
;
HDBLK:  DW	128		;sectors per track
	DB	4		;block shift factor
	DB	15		;block mask
	DB	0		;extent mask
	DW	2039		;disk size-1
	DW	1023		;directory max
	DB	255		;alloc 0
	DB	255		;alloc 1
	DW	0		;check size
	DW	0		;track offset
;
;	message
;
SIGNON: DB	'64K CP/M Vers. 2.2 (8080 CBIOS V1.2 for Z80SIM, '
	DB	'Copyright 1988-2007 by Udo Munk)'
	DB	13,10,0
;
LDERR:	DB	'Error booting'
	DB	13,10,0
;
;	end of fixed tables
;
;	utility functions
;
;	print a 0 terminated string to console device
;	pointer to string in HL
;
PRTMSG:	XCHG
	LDAX	D
	XCHG
	ORA	A
	RZ
	MOV	C,A
	CALL	CONOUT
	INX	H
	JMP	PRTMSG
;
;	individual subroutines to perform each function
;	simplest case is to just perform parameter initialization
;
BOOT:   LXI	SP,80H		;use space below buffer for stack
	LXI	H,SIGNON	;print message
	CALL	PRTMSG
	XRA	A		;zero in the accum
	STA	IOBYTE		;clear the iobyte
	STA	CDISK		;select disk zero
	JMP	GOCPM		;initialize and go to cp/m
;
;	simplest case is to read the disk until all sectors loaded
;
WBOOT:  LXI	SP,80H		;use space below buffer for stack
	MVI	C,0		;select disk 0
	CALL	SELDSK
	CALL	HOME		;go to track 00
;
	MVI	B,NSECTS	;b counts # of sectors to load
	MVI	C,0		;c has the current track number
	MVI	D,2		;d has the next sector to read
;	note that we begin by reading track 0, sector 2 since sector 1
;	contains the cold start loader, which is skipped in a warm start
	LXI	H,CCP		;base of cp/m (initial load point)
LOAD1:				;load one more sector
	PUSH	B		;save sector count, current track
	PUSH	D		;save next sector to read
	PUSH	H		;save dma address
	MOV	C,D		;get sector address to register c
	CALL	SETSEC		;set sector address from register c
	POP	B		;recall dma address to b,c
	PUSH	B		;replace on stack for later recall
	CALL	SETDMA		;set dma address from b,c
;	drive set to 0, track set, sector set, dma address set
	CALL	READ
	ORA	A		;any errors?
	JZ	LOAD2		;no, continue
	LXI	H,LDERR		;error, print message
	CALL	PRTMSG
	DI			;and halt the machine
	HLT
;	no error, move to next sector
LOAD2:	POP	H		;recall dma address
	LXI	D,128		;dma=dma+128
	DAD	D		;new dma address is in h,l
	POP	D		;recall sector address
	POP	B		;recall number of sectors remaining,
				;and current trk
	DCR	B		;sectors=sectors-1
	JZ	GOCPM		;transfer to cp/m if all have been loaded
;	more sectors remain to load, check for track change
	INR	D
	MOV	A,D		;sector=27?, if so, change tracks
	CPI	27
	JC	LOAD1		;carry generated if sector<27
;	end of current track, go to next track
	MVI	D,1		;begin with first sector of next track
	INR	C		;track=track+1
;	save register state, and change tracks
	CALL	SETTRK		;track address set from register c
	JMP	LOAD1		;for another sector
;	end of load operation, set parameters and go to cp/m
GOCPM:
	MVI	A,0C3H		;c3 is a jmp instruction
	STA	0		;for jmp to wboot
	LXI	H,WBOOTE	;wboot entry point
	SHLD	1		;set address field for jmp at 0
;
	STA	5		;for jmp to bdos
	LXI	H,BDOS		;bdos entry point
	SHLD	6		;address field of jump at 5 to bdos
;
	LXI	B,80H		;default dma address is 80h
	CALL	SETDMA
;
	LDA	CDISK		;get current disk number
	MOV	C,A		;send to the ccp
	JMP	CCP		;go to cp/m for further processing
;
;
;	simple i/o handlers
;
;	console status, return 0ffh if character ready, 00h if not
;
CONST:	IN	CONSTA		;get console status
	RET
;
;	console character into register a
;
CONIN:	IN	CONDAT		;get character from console
	RET
;
;	console character output from register c
;
CONOUT: MOV	A,C		;get to accumulator
	OUT	CONDAT		;send character to console
	RET
;
;	list character from register c
;
LIST:	MOV	A,C		;character to register a
	OUT	PRTDAT
	RET
;
;	return list status (00h if not ready, 0ffh if ready)
;
LISTST: IN	PRTSTA
	RET
;
;	punch character from register c
;
PUNCH:	MOV	A,C		;character to register a
	OUT	AUXDAT
	RET
;
;	read character into register a from reader device
;
READER: IN	AUXDAT
	RET
;
;
;	i/o drivers for the disk follow
;
;	move to the track 00 position of current drive
;	translate this call into a settrk call with parameter 00
;
HOME:	MVI	C,0		;select track 0
	JMP	SETTRK		;we will move to 00 on first read/write
;
;	select disk given by register C
;
SELDSK: LXI	H,0000H		;error return code
	MOV	A,C
	CPI	4		;FD drive 0-3?
	JC	SELFD		;go
	CPI	8		;harddisk 1?
	JZ	SELHD1		;go
	CPI	9		;harddisk 2?
	JZ	SELHD2		;go
	RET			;no, error
;	disk number is in the proper range
;	compute proper disk parameter header address
SELFD:	OUT	FDCD		;selekt disk drive
	MOV	L,A		;L=disk number 0,1,2,3
	DAD	H		;*2
	DAD	H		;*4
	DAD	H		;*8
	DAD	H		;*16 (size of each header)
	LXI	D,DPBASE
	DAD	D		;HL=.dpbase(diskno*16)
	RET
SELHD1:	LXI	H,HDB1		;dph harddisk 1
	JMP	SELHD
SELHD2:	LXI	H,HDB2		;dph harddisk 2
SELHD:	OUT	FDCD		;select darddisk drive
	RET
;
;	set track given by register c
;
SETTRK: MOV	A,C
	OUT	FDCT
	RET
;
;	set sector given by register c
;
SETSEC: MOV	A,C
	OUT	FDCS
	RET
;
;	translate the sector given by BC using the
;	translate table given by DE
;
SECTRAN:
	MOV	A,D		;do we have a translation table?
	ORA	E
	JNZ	SECT1		;yes, translate
	MOV	L,C		;no, return untranslated
	MOV	H,B		;in HL
	INR	L		;sector no. start with 1
	RNZ
	INR	H
	RET
SECT1:	XCHG			;HL=.trans
	DAD	B		;HL=.trans(sector)
	XCHG
	LDAX	D
	MOV	L,A		;L = trans(sector)
	MVI	H,0		;HL= trans(sector)
	RET			;with value in HL
;
;	set dma address given by registers b and c
;
SETDMA: MOV	A,C		;low order address
	OUT	DMAL
	MOV	A,B		;high order address
	OUT	DMAH		;in dma
	RET
;
;	perform read operation
;
READ:	XRA	A		;read command -> A
	JMP	WAITIO		;to perform the actual i/o
;
;	perform a write operation
;
WRITE:	MVI	A,1		;write command -> A
;
;	enter here from read and write to perform the actual i/o
;	operation.  return a 00h in register a if the operation completes
;	properly, and 01h if an error occurs during the read or write
;
WAITIO: OUT	FDCOP		;start i/o operation
	IN	FDCST		;status of i/o operation -> A
	RET
;
;	the remainder of the CBIOS is reserved uninitialized
;	data area, and does not need to be a part of the
;	system memory image (the space must be available,
;	however, between "begdat" and "enddat").
;
;	scratch ram area for BDOS use
;
BEGDAT	EQU	$		;beginning of data area
DIRBF:	DS	128		;scratch directory area
ALL00:	DS	31		;allocation vector 0
ALL01:	DS	31		;allocation vector 1
ALL02:	DS	31		;allocation vector 2
ALL03:	DS	31		;allocation vector 3
ALLHD1:	DS	255		;allocation vector harddisk 1
ALLHD2:	DS	255		;allocation vector harddisk 2
CHK00:	DS	16		;check vector 0
CHK01:	DS	16		;check vector 1
CHK02:	DS	16		;check vector 2
CHK03:	DS	16		;check vector 3
CHKHD1:	DS	0		;check vector harddisk 1
CHKHD2:	DS	0		;check vector harddisk 2
;
ENDDAT	EQU	$		;end of data area
DATSIZ	EQU	$-BEGDAT	;size of data area
;
	END			;of BIOS

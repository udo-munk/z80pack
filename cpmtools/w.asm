;*****************************************************************************
;
;  W.ASM - CP/M program to write a file from the CP/M file system to the
;       host PC file system. Designed to run under the z80pack simulator.
;       Requires the simbdos.c module in z80pack to support host file I/O. 
;
;	Rev	 Date	  Desc
;	1.0	10/2/19   Mike Douglas, Original
;
;*****************************************************************************

; BDOS equates

BDOS	equ	5		;BDOS entry point
PRINT	equ	9		;BDOS write string to console
OPENF	equ	15		;BDOS open file
CLOSEF	equ	16		;BDOS close file
DELETEF	equ	19		;BDOS delete file
READF	equ	20		;BDOS read file
WRITEF	equ	21		;BDOS write file
MAKEF	equ	22		;BDOS make file
SETDMA	equ	26		;BDOS set DMA address

; CP/M default File Control Block (FCB)

FCB	equ	5Ch		;location of default CP/M FCB
FCBFN	equ	FCB+1		;location of file name
FCBEXT	equ	FCB+9		;location of file extension
FCBCR	equ	FCB+32		;current record
DEFDMA	equ	80h		;CP/M default DMA address

; Misc equates

PCPORT	equ	161		;OUT port for file hook into simulator
CR	equ	13		;ASCII for carriage return
LF	equ	10		;ASCII for line feed

	org	0100h		;CP/M load and entry address
;-----------------------------------------------------------------------------
; Main program
;-----------------------------------------------------------------------------
	call	vfyFcb		;verify the FCB from command line
	
	lxi	d,mFile		;display file name
	mvi	c,PRINT
	call	BDOS

	call	opnFile		;open the files
	call	cpyFile		;perform the copy

	lxi	d,mDone		;DE->success message
	jmp	exitM2F		;close files, display exit message and exit
			
;-----------------------------------------------------------------------------	
; vfyFcb - Verify a valid file to copy has been specified on the command
;    line. Copy the default FCB containing the file name to outFcb. Also
;    copy the file name with space supression into mFName.
;-----------------------------------------------------------------------------
vfyFcb	lxi	d,mHelp		;DE->help message
	lda	FCBFN		;look at 1st character of 1st file name
	cpi	' '		;anything there?
	jz	exitMsg		;no, display help message and exit

	lxi	h,FCB		;HL->FCB made by command processor
	lxi	d,outFcb	;DE->copy of FCB
	mvi	b,16		;B=number of bytes to move

copyFcb	mov	a,m		;copy command line FCB to outFcb
	stax	d
	inx	h
	inx	d
	dcr	b
	jnz	copyFcb

; Copy space suppressed filename to mFName and verify no wildcards

	lxi	h,FCBFN		;HL->filename in FCB
	lxi	d,mFName	;DE->filename in output message
	mvi	b,8		;B=number of bytes to move
	call	copyFn		;copy filename
	
	mvi	a,'.'		;insert '.' before extension
	stax	d
	inx	d
	
	lxi	h,FCBEXT	;copy extension
	mvi	b,3
	call	copyFn
	
	dcx	d		;if no extension, get rid of '.'
	ldax	d
	cpi	'.'
	jz	term$		;no extension, insert terminator
	
	inx	d		;bump past end of extension

term$	mvi	a,'$'		;add trailing '$' terminator
	stax	d
	ret

;-----------------------------------------------------------------------------	
; opnFile - Open the input and output files
;-----------------------------------------------------------------------------
; Set the buffer address for disk reads and writes

opnFile	lxi	d,DEFDMA	;use the CP/M default buffer
	mvi	c,SETDMA
	call	pcBdos		;set buffer address

	lxi	d,DEFDMA	;use the CP/M default buffer
	mvi	c,SETDMA
	call	BDOS		;set buffer address
	
; Open the input file on the CP/M machine

	xra	a		;zero current record the FCB
	sta	FCBCR

	lxi	d,FCB		;DE->FCB for input from CP/M
	mvi	c,OPENF		;C=open file command
	call	BDOS		;open input file

	lxi	d,mNoFile	;DE->file not found message
	inr	a		;test for FFh error
	jz	exitMsg		;file not found, exit

; Open the output file on the PC. Unlike CP/M, the MAKEF command
;    also opens the file for write on the PC.

	lxi	d,outFcb	;DE->destination FCB for PC
	mvi	c,MAKEF		;C=make file command
	call	pcBdos		;open destination file
	
	lxi	d,mMakErr	;DE->can't create file message
	inr	a		;test for FF (make file fail)
	jz	exitM1F		;create failed, close one file & exit
	
	ret
	
;-----------------------------------------------------------------------------	
; cpyFile - Copy the input file from CP/M to the output file on the PC
;-----------------------------------------------------------------------------
cpyFile	lxi	d,FCB		;DE->input file FCB
	mvi	c,READF		;C=read file sequential
	call	BDOS
	ora	a		;end of file?
	rnz			;yes, file copy is done

	lxi	d,outFcb	;DE->output file FCB
	mvi	c,WRITEF	;C=write file sequential
	call	pcBdos		;write destination file
	ora	a		;test for write error
	jz	cpyFile		;no error, do next record

	lxi	d,mWrtErr	;DE->write error message
	jmp	exitM2F		;display error, close two files & exit

;-----------------------------------------------------------------------------
; exitMsg - Display message pointed to by DE and exit to CP/M
; exitM1F - also closes the input file
; exitM2F - also closes the input and output files
;-----------------------------------------------------------------------------
exitM2F	push	d		;save message pointer
	lxi	d,outFcb	;DE->destination file FCB
	mvi	c,CLOSEF	;C=close file command
	call	pcBdos
	pop	d		;DE->exit message

exitM1F	push	d		;save message pointer
	lxi	d,FCB		;DE->source file FCB
	mvi	c,CLOSEF	;C=close file command
	call	BDOS
	pop	d		;DE->exit message

exitMsg	mvi	c,PRINT		;display message passed in DE
	call	BDOS
	jmp	0		;exit to CP/M

;-----------------------------------------------------------------------------
; pcBdos - Call the PC "BDOS" by putting the complement of the command
;     (passed in C) into A, then doing an OUT to PCPORT to call the 
;     simulator hook for PC file I/O.
;-----------------------------------------------------------------------------
pcBdos	mov	a,c		;set A = not C
	cma
	out	PCPORT		;call the PC "BDOS"
	ret

;-----------------------------------------------------------------------------	
; copyFn - Copy from (HL) to (DE). Abort with error if '?' found which
;     indicates a wildcard was used (not supported). Stop copying when
;     space is found or length in B has been copied.
;-----------------------------------------------------------------------------
copyFn	mov	a,m		;copy file name byte
	stax	d
	
	cpi	'?'		;wildcard used?
	jz	wildErr		;yes, not supported
	
	cpi	' '		;space?
	rz			;yes, done
	
	inx  	h		;bump pointers
	inx	d
	dcr	b
	jnz	copyFn		;copy until B reaches zero
	
	ret

wildErr	lxi	d,mNoWild	;DE->no wildcard message
	jmp	exitMsg		;display message and exit

;-----------------------------------------------------------------------------
;  String Constants
;-----------------------------------------------------------------------------
mHelp	db	CR,LF
 	db	'W.COM v1.0 - Write file to host PC from CP/M',CR,LF
	db	LF
 	db	'Usage: W <filename>',CR,LF,'$'

mNoWild	db	CR,LF,'Wildcards not supported',CR,LF,'$'

mFile	db	CR,LF,'File '
mFName	ds	16

mNoFile	db	' not found',CR,LF,'$'
mMakErr	db	' - cannot create file',CR,LF,'$'
mWrtErr	db	' - write error',CR,LF,'$'
mDone	db	' written to host PC',CR,LF,'$'

;-----------------------------------------------------------------------------
;  Data Area
;-----------------------------------------------------------------------------
outFcb	dw	0,0,0,0,0,0,0,0,0	;36 byte FCB
	dw	0,0,0,0,0,0,0,0,0
	
	end

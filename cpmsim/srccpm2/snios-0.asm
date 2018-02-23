	title	'Slave Network I/O System'

;***************************************************************
;***************************************************************
;**                                                           **
;**      S l a v e   N e t w o r k   I / O   S y s t e m      **
;**                                                           **
;***************************************************************
;***************************************************************

;/*
;  Copyright (C) 1980
;  Digital Research
;  P.O. Box 579
;  Pacific Grove, CA 93950
;
;  Modified September 2014 for Z80SIM by Udo Munk
;*/

false	equ	0
true	equ	not false

always$retry	equ	false	; force continuous retries

debug	equ	false

;	Jump vector for SNIOS entry points
	jmp	ntwrkinit	; network initialization
	jmp	ntwrksts	; network status
	jmp	cnfgtbladr	; return config table addr
	jmp	sendmsg		; send message on network
	jmp	receivemsg	; receive message from network
	jmp	ntwrkerror	; network error
	jmp	ntwrkwboot	; network warm boot

slave$ID	equ	11h	; slave processor ID number

;	Slave Configuration Table
configtbl:

Network$status:
	db	0000$0000b	; network status byte
	db	slave$ID	; slave processor ID number
	db	0,0		; A:  Disk device
	db	0,0		; B:   "
	db	0,0		; C:   "
	db	0,0		; D:   "
	db	0,0		; E:   "
	db	0,0		; F:   "
	db	0,0		; G:   "
	db	0,0		; H:   "
	db	0,0		; I:   "
	db	0,0		; J:   "
	db	0,0		; K:   "
	db	0,0		; L:   "
	db	0,0		; M:   "
	db	0,0		; N:   "
	db	0,0		; O:   "
	db	0,0		; P:   "

	db	0,0		; console device

	db	0,0		; list device:
	db	0		;	buffer index
	db	0		;	FMT
	db	0		;	DID
	db	slave$ID	;	SID
	db	5		;	FNC
	db	0		;	SIZ
	ds	1		;	MSG(0)  List number
	ds	128		;	MSG(1) ... MSG(128)

;	Local Data Segment
;
Binary$ASCII:
	db	0ffh		; 0=7 bit ASCII, FF=8 bit binary
msg$adr:
	dw	0		; message address

timeout$retries equ 100		; timeout a max of 100 times
max$retries equ	10		; send message max of 10 times
retry$count:
	db	0

;	Network Status Byte Equates
;
active		equ	0001$0000b	; slave logged in on network
rcverr		equ	0000$0010b	; error in received message
senderr		equ	0000$0001b	; unable to send message

;	General Equates
;
SOH	equ	01h		; Start of Header
STX	equ	02h		; Start of Data
ETX	equ	03h		; End of Data
EOT	equ	04h		; End of Transmission
ENQ	equ	05h		; Enquire
ACK	equ	06h		; Acknowledge
LF	equ	0ah		; Line Feed
CR	equ	0dh		; Carriage Return
NAK	equ	15h		; Negative Acknowledge

login	equ	64		; Login NDOS function

;	I/O Equates
;
stati	equ	50
mski	equ	01
dprti	equ	51

stato	equ	50
msko	equ	02
dprto	equ	51

;	Utility Procedures
;
delay:	;delay for c[a] * 0.5 milliseconds
	mvi	a,16
delay1:
	mvi	c,86h
delay2:
	dcr	c
	jnz	delay2
	dcr	a
	jnz	delay1
	ret

Nib$out:			; A = nibble to be transmitted in ASCII
	cpi	10
	jnc	nibAtoF		; jump if A-F
	adi	'0'
	mov	c,a
	jmp	Char$out
nibAtoF:
	adi	'A'-10
	mov	c,a
	jmp	Char$out

Pre$Char$out:
	mov	a,d
	add	c
	mov	d,a		; update the checksum in D

Char$out:			; C = byte to be transmitted
	in	stato
	ani	msko
	jz	Char$out

	mov	a,c
	out	dprto
;	jmp	delay		; delay after each Char sent to Mstr
	ret

Nib$in:				; return nibble in A register
	call	Char$in
	rc
	ani	7fh
	sui	'0'
	cpi	10
	jc	Nib$in$rtn 	; must be 0-9
	adi	('0'-'A'+10) and 0ffh
	cpi	16
	jc	Nib$in$rtn 	; must be 10-15
	lda	network$status
	ori	rcverr
	sta	network$status
	mvi	a,0
	stc			; carry set indicating err cond
	ret

Nib$in$rtn:
	ora	a		; clear carry & return
	ret

Char$in:			; return byte in A register
				;  carry set on rtn if timeout
	mvi	b,50		; 50 ms = 50 chars @ 9600 baud
Char$in0:
	mvi	c,5ah
Char$in1:
	in	stati		; busy wait forever, no reasonable
	ani	mski		; delay implemented yet
	jz	Char$in1
;	in	stati
;	ani	mski
;	jnz	Char$in2
;	dcr	c
;	jnz	Char$in1
;	dcr	b
;	jnz	Char$in0
;	stc			; carry set for err cond = timeout
;	ret
Char$in2:
	in	dprti
	ret			; rtn with raw char and carry cleared

Net$out:			; C = byte to be transmitted
				; D = checksum
	mov	a,d
	add	c
	mov	d,a
	lda	Binary$ASCII
	ora	a
	jnz	Char$out	; transmit byte in Binary mode
	mov	a,c
	mov	b,a
	rar
	rar
	rar
	rar
	ani	0FH		; mask HO nibble to LO nibble
	call	Nib$out
	mov	a,b
	ani	0FH
	jmp	Nib$out

Msg$in:				; HL = destination address
				; E  = # bytes to input
	call	Net$in
	rc
	mov	m,a
	inx	h
	dcr	e
	jnz	Msg$in
	ret

Net$in:				; byte returned in A register
				; D  = checksum accumulator
	lda	Binary$ASCII
	ora	a
	jz	ASCII$in
	call	Char$in		;receive byte in Binary mode
	rc
	jmp	chks$in
ASCII$in:
	call	Nib$in
	rc
	add	a
	add	a
	add	a
	add	a
	push	psw
	call	Nib$in
	pop	b
	rc
	ora	b
chks$in:
	mov	b,a
	add	d		; add & update checksum accum.
	mov	d,a
	ora	a		; set cond code from checksum
	mov	a,b
	ret

Msg$out:			; HL = source address
				; E  = # bytes to output
				; D  = checksum
				; C  = preamble byte
	mvi	d,0		; initialize the checksum
	call	Pre$Char$out	; send the preamble character
Msg$out$loop:
	mov	c,m
	inx	h
	call	Net$out
	dcr	e
	jnz	Msg$out$loop
	ret

;	Network Initialization
ntwrkinit:
;	device initialization, as required

initok:
	xra	a		;return code is 0=success
	ret

;	Network Status
ntwrksts:
	lda	network$status
	mov	b,a
	ani	not (rcverr+senderr)
	sta	network$status
	mov	a,b
	ret

;	Return Configuration Table Address
cnfgtbladr:
	lxi	h,configtbl
	ret

;	Send Message on Network
sendmsg:			; BC = message addr
	mov	h,b
	mov	l,c		; HL = message address
	shld	msg$adr
re$sendmsg:
	mvi	a,max$retries
	sta	retry$count	; initialize retry count
send:
	lhld	msg$adr
	mvi	c,ENQ
	call	Char$out	; send ENQ to master
	mvi	d,timeout$retries
ENQ$response:
	call	Char$in
	jnc	got$ENQ$response
	dcr	d
	jnz	ENQ$response
	jmp	Char$in$timeout
got$ENQ$response:
	call	get$ACK0
	mvi	c,SOH
	mvi	e,5
	call	Msg$out		; send SOH FMT DID SID FNC SIZ
	xra	a
	sub	d
	mov	c,a
	call	net$out		; send HCS (header checksum)
	call	get$ACK
	dcx	h
	mov	e,m
	inx	h
	inr	e
	mvi	c,STX
	call	Msg$out		; send STX DB0 DB1 ...
	mvi	c,ETX
	call	Pre$Char$out	; send ETX
	xra	a
	sub	d
	mov	c,a
	call	Net$out		; send the checksum
	mvi	c,EOT
	call	Char$out	; send EOT
	call	get$ACK		; (leave these
	ret			;              two instructions)

get$ACK:
	call	Char$in
	jc	send$retry 	; jump if timeout
get$ACK0:
	ani	7fh
	sui	ACK
	rz
send$retry:
	pop	h		; discard return address
	lxi	h,retry$count
	dcr	m
	jnz	send		; send again unles max retries
Char$in$timeout:
	mvi	a,senderr

	if	always$retry
	call	error$return
	jmp	re$sendmsg
	else
	jmp	error$return
	endif

;	Receive Message from Network
receivemsg:			; BC = message addr
	mov	h,b
	mov	l,c		; HL = message address
	shld	msg$adr
re$receivemsg:
	mvi	a,max$retries
	sta	retry$count	; initialize retry count
re$call:
	call	receive		; rtn from receive is receive error

receive$retry:
	lxi	h,retry$count
	dcr	m
	jnz	re$call
receive$timeout:
	mvi	a,rcverr

	if	always$retry
	call	error$return
	jmp	re$receivemsg
	else
	jmp	error$return
	endif

receive:
	lhld	msg$adr
	mvi	d,timeout$retries
receive$firstchar:
	call	charin
	jnc	got$firstchar
	dcr	d
	jnz	receive$firstchar
	pop	h		; discard receive$retry rtn adr
	jmp	receive$timeout
got$firstchar:
	ani	7fh
	cpi	ENQ		; Enquire?
	jnz	receive

	mvi	c,ACK
	call	Char$out 	; acknowledge ENQ with an ACK

	call	Char$in
	rc			; return to receive$retry
	ani	7fh
	cpi	SOH		; Start of Header ?
	rnz			; return to receive$retry
	mov	d,a		; initialize the HCS
	mvi	e,5
	call	Msg$in
	rc			; return to receive$retry
	call	Net$in
	rc			; return to receive$retry
	jnz	bad$checksum
	call	send$ACK
	call	Char$in
	rc			; return to receive$retry
	ani	7fh
	cpi	STX		; Start of Data ?
	rnz			; return to receive$retry
	mov	d,a		; initialize the CKS
	dcx	h
	mov	e,m
	inx	h
	inr	e
	call	msg$in		; get DB0 DB1 ...
	rc			; return to receive$retry
	call	Char$in		; get the ETX
	rc			; return to receive$retry
	ani	7fh
	cpi	ETX
	rnz			; return to receive$retry
	add	d
	mov	d,a		; update CKS with ETX
	call	Net$in		; get CKS
	rc			; return to receive$retry
	call	Char$in		; get EOT
	rc			; return to receive$retry
	ani	7fh
	cpi	EOT
	rnz			; return to receive$retry
	mov	a,d
	ora	a		; test CKS
	jnz	bad$checksum
	pop	h		; discard receive$retry rtn adr
	lhld	msg$adr
	inx	h
	lda	configtbl+1
	sub	m
	jz	send$ACK 	; jump with A=0 if DID ok
	mvi	a,0ffh		; return code shows bad DID
send$ACK:
	push	psw		; save return code
	mvi	c,ACK
	call	Char$out 	; send ACK if checksum ok
	pop	psw		; restore return code
	ret

bad$DID:
bad$checksum:
	mvi	c,NAK
	jmp	Char$out  	; send NAK on bad chksm & not max retries
;	ret

error$return:
	lxi	h,network$status
	ora	m
	mov	m,a
	call	ntwrkerror 	; perform any required device re-init.
	mvi	a,0ffh
	ret

;
ntwrkerror:
				;  perform any required device re-initialization
	ret

;
ntwrkwboot:
	; This procedure is called each time the CCP is
	;  reloaded from disk.  At this point you could
	;  possibly check for any mail at the master or
	;  anything else, this is just a dummy proc.
	mvi	c,9
	lxi	d,wboot$msg
	call	0005h
	ret

wboot$msg:
	db	'<Warm Boot>'
	db	'$'

	end

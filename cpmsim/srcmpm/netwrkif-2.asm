	title	'Master Network I/F Module'
	page	54

;***************************************************************
;***************************************************************
;**                                                           **
;**      S e r v e r   N e t w o r k   I / F   M o d u l e    **
;**                                                           **
;***************************************************************
;***************************************************************

;/*
;  Copyright (C) 1980
;  Digital Research
;  P.O. Box 579
;  Pacific Grove, CA 93950
;
;  Modified October 5, 1982
;
;  Modified December 2006 and May 2014 for Z80SIM by Udo Munk
;*/

	
false	equ	0
true	equ	not false
debug	equ	false

WtchDg	equ	true		; include watch dog timer

mutexin	equ	false		; provide mutual exclusion on input
mutexout equ	false		; provide mutual exclusion on output

NmbSlvs	equ	2		; Number of slaves

Console1$status	equ	40
Console2$status	equ	42
Console3$status	equ	44
Console4$status	equ	46

	if	debug
	lxi	sp,NtwrkIS0+2eh
	mvi	c,145
	mvi	e,64
	call	bdos		; set priority to 64
	lxi	h,UQCBNtwrkQI0	; initialize reentrant variables
	lxi	d,UQCBNtwrkQO0
	lxi	b,BufferQ0
	mvi	a,00h
	ret
	endif

bdosadr:
	if	debug
	dw	0005h
	else
	dw	$-$		; XDOS entry point for RSP version
	endif

;  Network Interface Process #0

NtwrkIP0:
	dw	0		; link
	db	0		; status
	db	64		; priority
	dw	NtwrkIS0+46	; stack pointer
	db	'NtwrkIP0'	; name
	db	0		; console
	db	0ffh		; memseg
	ds	2		; b
	ds	2		; thread
	ds	2		; buff
	ds	1		; user code & disk slct
	ds	2		; dcnt
	ds	1		; searchl
	ds	2		; searcha
	ds	2		; active drives
	dw	0		; HL'
	dw	0		; DE'
	dw	0		; BC'
	dw	0		; AF'
	dw	0		; IY
	dw	0		; IX
	dw	UQCBNtwrkQI0	; HL
	dw	UQCBNtwrkQO0	; DE
	dw	BufferQ0	; BC
	dw	0200H		; AF, A = ntwkif console dev #
	ds	2		; scratch

NtwrkIS0:
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h
	dw	setup

QCBNtwrkQI0:
	ds	2		; link
	db	'NtwrkQI0'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQI0:
	dw	QCBNtwrkQI0	; pointer
	dw	BufferQI0Addr	; msgadr
BufferQI0Addr:
	dw	BufferQ0

QCBNtwrkQO0:
	ds	2		; link
	db	'NtwrkQO0'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQO0:
	dw	QCBNtwrkQO0	; pointer
	dw	BufferQO0Addr	; msgadr
BufferQO0Addr:
	ds	2

BufferQ0:
	ds	1		; FMT
	ds	1		; DID
	ds	1		; SID
	ds	1		; FNC
	ds	1		; SIZ
	ds	257		; MSG

;	Network Interface Process #1

	if	NmbSlvs GE 2
NtwrkIP1:

	if	NmbSlvs GE 3
	dw	NtwrkIP2	; link
	else
	dw	0		; link
	endif

	db	0		; status
	db	64		; priority
	dw	NtwrkIS1+46	; stack pointer
	db	'NtwrkIP1'	; name
	db	0		; console
	db	0ffh		; memseg
	ds	2		; b
	ds	2		; thread
	ds	2		; buff
	ds	1		; user code & disk slct
	ds	2		; dcnt
	ds	1		; searchl
	ds	2		; searcha
	ds	2		; active drives
	dw	0		; HL'
	dw	0		; DE'
	dw	0		; BC'
	dw	0		; AF'
	dw	0		; IY
	dw	0		; IX
	dw	UQCBNtwrkQI1	; HL
	dw	UQCBNtwrkQO1	; DE
	dw	BufferQ1	; BC
	dw	0300h		; AF, A = ntwkif console dev #
	ds	2		; scratch

NtwrkIS1:
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h
	dw	init

QCBNtwrkQI1:
	ds	2		; link
	db	'NtwrkQI1'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQI1:
	dw	QCBNtwrkQI1	; pointer
	dw	BufferQI1Addr	; msgadr
BufferQI1Addr:
	dw	BufferQ1

QCBNtwrkQO1:
	ds	2		; link
	db	'NtwrkQO1'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQO1:
	dw	QCBNtwrkQO1	; pointer
	dw	BufferQO1Addr	; msgadr
BufferQO1Addr:
	ds	2

BufferQ1:
	ds	1		; FMT
	ds	1		; DID
	ds	1		; SID
	ds	1		; FNC
	ds	1		; SIZ
	ds	257		; MSG
	endif

;	Network Interface Process #2

	if	NmbSlvs GE 3
NtwrkIP2:

	if	NmbSlvs GE 4
	dw	NtwrkIP3	; link
	else
	dw	0		; link
	endif

	db	0		; status
	db	64		; priority
	dw	NtwrkIS2+46	; stack pointer
	db	'NtwrkIP2'	; name
	db	0		; console
	db	0ffh		; memseg
	ds	2		; b
	ds	2		; thread
	ds	2		; buff
	ds	1		; user code & disk slct
	ds	2		; dcnt
	ds	1		; searchl
	ds	2		; searcha
	ds	2		; active drives
	dw	0		; HL'
	dw	0		; DE'
	dw	0		; BC'
	dw	0		; AF'
	dw	0		; IY
	dw	0		; IX
	dw	UQCBNtwrkQI2	; HL
	dw	UQCBNtwrkQO2	; DE
	dw	BufferQ2	; BC
	dw	0200h		; AF, A = ntwkif console dev #
	ds	2		; scratch

NtwrkIS2:
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h
	dw	init

QCBNtwrkQI2:
	ds	2		; link
	db	'NtwrkQI2'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQI2:
	dw	QCBNtwrkQI2	; pointer
	dw	BufferQI2Addr	; msgadr
BufferQI2Addr:
	dw	BufferQ2

QCBNtwrkQO2:
	ds	2		; link
	db	'NtwrkQO2'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQO2:
	dw	QCBNtwrkQO2	; pointer
	dw	BufferQO2Addr	; msgadr
BufferQO2Addr:
	ds	2

BufferQ2:
	ds	1		; FMT
	ds	1		; DID
	ds	1		; SID
	ds	1		; FNC
	ds	1		; SIZ
	ds	257		; MSG
	endif

;	Network Interface Process #3

	if	NmbSlvs GE 4
NtwrkIP3:
	dw	0		; link
	db	0		; status
	db	64		; priority
	dw	NtwrkIS3+46	; stack pointer
	db	'NtwrkIP3'	; name
	db	0		; console
	db	0ffh		; memseg
	ds	2		; b
	ds	2		; thread
	ds	2		; buff
	ds	1		; user code & disk slct
	ds	2		; dcnt
	ds	1		; searchl
	ds	2		; searcha
	ds	2		; active drives
	dw	0		; HL'
	dw	0		; DE'
	dw	0		; BC'
	dw	0		; AF'
	dw	0		; IY
	dw	0		; IX
	dw	UQCBNtwrkQI3	; HL
	dw	UQCBNtwrkQO3	; DE
	dw	BufferQ3	; BC
	dw	0300h		; AF, A = ntwkif console dev #
	ds	2		; scratch

NtwrkIS3:
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h
	dw	init

QCBNtwrkQI3:
	ds	2		; link
	db	'NtwrkQI3'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQI3:
	dw	QCBNtwrkQI3	; pointer
	dw	BufferQI3Addr	; msgadr
BufferQI3Addr:
	dw	BufferQ3

QCBNtwrkQO3:
	ds	2		; link
	db	'NtwrkQO3'	; name
	dw	2		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer

UQCBNtwrkQO3:
	dw	QCBNtwrkQO3	; pointer
	dw	BufferQO3Addr	; msgadr
BufferQO3Addr:
	ds	2

BufferQ3:
	ds	1		; FMT
	ds	1		; DID
	ds	1		; SID
	ds	1		; FNC
	ds	1		; SIZ
	ds	257		; MSG
	endif


	if	WtchDg
;  Watchdog Timer Process
;
WatchDogPD:

	if	NmbSlvs GT 1
	dw	NtwrkIP1	; link to the remaining NETWRKIF PD's
	else
	dw	0		; link
	endif

	db	0		; status
	db	64		; priority
	dw	WatchDogSTK+46	; stack pointer
	db	'WatchDog'	; name
	db	0		; console
	db	0ffh		; memseg
	ds	2		; b
	ds	2		; thread
	ds	2		; buff
	ds	1		; user code & disk slct
	ds	2		; dcnt
	ds	1		; searchl
	ds	2		; searcha
	ds	2		; active drives
	dw	0		; HL'
	dw	0		; DE'
	dw	0		; BC'
	dw	0		; AF'
	dw	0		; IY
	dw	0		; IX
	dw	0		; HL
	dw	0		; DE
	dw	0		; BC
	dw	0		; AF
	ds	2		; scratch

WatchDogSTK:
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h,0c7c7h
	dw	0c7c7h,0c7c7h,0c7c7h
	dw	WatchDog

WatchDogTime:
	dw	$-$	; one-second counter

WatchDogTable:
;	     Waiting Timeout   Start	Flag  Requester
	db	0,	0,	0,0,	0ah	; #0
	db	0,	0,	0,0,	0bh	; #1
	db	0,	0,	0,0,	0fh	; #2
	db	0,	0,	0,0,	0dh	; #3
	endif

	if	mutexin or mutexout
QCBMXSXmitq:			; MX queue for requester transmitting

	ds	2		; link
	db	'MXSXmitq'	; name
	dw	0		; msglen
	dw	1		; nmbmsgs
	ds	2		; dqph
	ds	2		; nqph
	ds	2		; msgin
	ds	2		; msgout
	ds	2		; msgcnt
	ds	2		; buffer (owner PD)

UQCBMXSXmitq:
	dw	QCBMXSXmitq
;	dw	0		; no message, since it's an MX queue
;	db	'MXSXmitq'	; no name, since the QCB pointer is resolved
	endif

;	Server Configuration Table

configtbl:
	db	0		; Server status byte
	db	0		; Server ID
	db	NmbSlvs		; Maximum number of requesters supported
	db	0		; Number of requesters currently logged-in
	dw	0000h		; 16 bit vector of logged in requesters
	ds	16		; Requester ID's currently logged-in
	db	'PASSWORD' 	; login password

nmsg		equ	1	; number of messages buffered
slave$stk$len	equ	96h	; server process stack size

	if	NmbSlvs GE 2
slave1$stk:
	ds	slave$stk$len-2		
	dw	Slave1			
					
	endif

	if	NmbSlvs GE 3
slave2$stk:
	ds	slave$stk$len-2
	dw	Slave2
	endif

	if	NmbSlvs GE 4
slave3$stk:
	ds	slave$stk$len-2
	dw	Slave3
	endif

	if	NmbSlvs GE 2
Slave1:
	ds	52		; SERVR1PR processor descriptor
	endif

	if	NmbSlvs GE 3
Slave2:
	ds	52		; SERVR2PR processor descriptor
	endif

	if	NmbSlvs GE 4
Slave3:
	ds	52		; SERVR3PR processor descriptor
	endif

;	Local Data Segment

BinaryASCII:
	db	0ffh		; Requester #0: 0=7 bit ASCII, FF=8 bit binary
	db	0ffh		;       #1
	db	0ffh		;       #2
	db	0ffh		;       #3

Networkstatus:
	db	0		; Slave #0 network status byte
	db	0		;       #1
	db	0		;       #2
	db	0		;       #3

conin:	dw	$-$		; save area for XIOS routine address

max$retries	equ	10	; maximum send message retries

chariotbl:			; Relationship between requesters and consoles
	db	1
	db	2
	db	3
	db	4

;	Network Status Byte Equates

ntwrktxrdy	equ	10000000b	; NETWRKIF ready to send msg
active		equ	00010000b	; requester logged into network
msgerr		equ	00001000b	; error in received message
ntwrk		equ	00000100b	; network alive
msgovr		equ	00000010b	; message overrun
ntwrkrxrdy	equ	00000001b	; NETWRKIF has rcvd msg

;	BDOS and XDOS Equates

flagset	equ	133		; flag set
makeq	equ	134		; make queue
readq	equ	137		; read queue
writeq	equ	139		; write queue
delay	equ	141		; delay
dsptch	equ	142		; dispatch
createp	equ	144		; create process
sydatad	equ	154		; system data page address
poll	equ	083h		; Poll device

;	General Equates

SOH	equ	01h		; Start of Header
STX	equ	02h		; Start of Data
ETX	equ	03h		; End of Data
EOT	equ	04h		; End of Transmission
ENQ	equ	05h		; Enquire
ACK	equ	06h		; Acknowledge
LF	equ	0ah		; Line Feed
CR	equ	0dh		; Carriage Return
NAK	equ	15h		; Negative Acknowledge

;	Utility Procedures

bdos:
	lhld	bdosadr		; get XDOS entry point from RSP start
	pchl

Nibout:				; A = nibble to be transmitted in ASCII
	cpi	10
	jnc	nibatof		; jump if A-F
	adi	'0'
	mov	c,a
	jmp	Charout
nibatof:
	adi	'A'-10
	mov	c,a
	jmp	Charout

PreCharout:
	mov	a,d
	add	c
	mov	d,a		; update the checksum

char$out:

;	Character output routine for network I/O
;	using Z80SIM SIO ports
;
;	8080 version: This has to dispatch and then use direct port I/O
;	--extremely messy to do reentrantly
;
;	Entry:	C = character to transmit
;		B = slave id byte

	push 	h
	push	d
	push	b

	lxi	d,out0		; dispatch address = 
	mov	l,b		;   out0 + slaveid*16
	mvi	h,0
	dad	h
	dad	h
	dad	h
	dad	h
	dad	d
	mvi	a,10h		; load "get transmit status" value
	pchl			; dispatch 

out0:
	in	Console1$status
	ani	2
	jz	out0

	mov	a,c
	out	Console1$status+1	; write the character
	pop	b
	pop	d
	pop	h
	ret
	nop			; filler to get 16 bytes, see dispatcher above
	nop

out1:
	in	Console2$status
	ani	2
	jz	out1

	mov	a,c
	out	Console2$status+1
	pop	b
	pop	d
	pop	h
	ret
	nop			; filler to get 16 bytes, see dispatcher above
	nop

out2:
	in	Console3$status
	ani	2
	jz	out2

	mov	a,c
	out	Console3$status+1
	pop	b
	pop	d
	pop	h
	ret
	nop			; filler to get 16 bytes, see dispatcher above
	nop

out3:
	in	Console4$status
	ani	2
	jz	out3

	mov	a,c
	out	Console4$status+1
	pop	b
	pop	d
	pop	h
	ret
	nop			; filler to get 16 bytes, see dispatcher above
	nop

Nibin:				; return nibble in A register
	call	Charin
	rc
	ani	07fh
	sui	'0'
	cpi	10
	jc	Nibin$return 	; must be 0-9
	adi	('0'-'A'+10) and 0ffh
	cpi	16
	jc	Nibin$return 	; must be 10-15
	lda	networkstatus
	ori	msgerr
	sta	networkstatus
	mvi	a,0
	stc
	ret

Nibin$return:
	ora	a
	ret

xChar$in:			; Get the first character using polled 
				; console I/O. Note that the rest of the 
				; message will be received using direct
				; port I/O with interrupts disabled.
				; OVERRUNS ARE NOT POSSIBLE USING THIS SCHEME

	push	h
	push	b
	lxi	h, Charin$return
	push	h
	mov	c,b
	mvi	b,0
	lxi	h, chariotbl
	dad	b
	mov	d, m		; Get the console number
	lhld	conin
	pchl			; vector off

Charin$return:
	pop	b
	pop	h
	ret


char$in:

;	Character input routine for network I/O
;	using Z80SIM SIO ports
;
;	8080 Version uses same nasty dispatch mechanism that the output
;	routine used
;
;	Entry:	B = Slave ID
;	Exit:	A = character input

	push 	h
	push	d
	push	b
	lxi	d,in0		; HL = in0 + 17*slaveid
	mov	l,b
	mvi	h,0
	xchg
	dad	d
	xchg
	dad	h
	dad	h
	dad 	h
	dad	h
	dad	d

	mvi	c,80		; load status retry count
	pchl			; dispatch

in0:
				; we can't use busy waiting with a loop
				; on 500MHz CPU's, this needs to be handled
				; different
;	dcr	c
;	jz	retout		; error return if retry timeout

	in	Console1$status	; wait for RXready
	ani	1
	jz	in0

	in	Console1$status+1	; get the character
	pop	b
	pop	d
	pop	h
	ret

	nop			; filler for retry timeout
	nop
	nop
	nop

in1:
;	dcr	c
;	jz	retout			

	in	Console2$status		
	ani	1
	jz	in1

	in	Console2$status+1	
	pop	b
	pop	d
	pop	h
	ret

	nop			; filler for retry timeout
	nop
	nop
	nop

in2:
;	dcr	c
;	jz	retout			

	in	Console3$status		
	ani	1
	jz	in2

	in	Console3$status+1	
	pop	b
	pop	d
	pop	h
	ret

	nop			; filler for retry timeout
	nop
	nop
	nop

in3:
;	dcr	c
;	jz	retout			

	in	Console4$status		
	ani	1
	jz	in3

	in	Console4$status+1	
	pop	b
	pop	d
	pop	h
	ret

	nop			; filler for retry timeout
	nop
	nop
	nop

retout:				; error return (carry=1)
	stc
	pop	b
	pop	d
	pop	h
	ret

Netout:				; C = byte to be transmitted
	mov	a,d
	add	c
	mov	d,a
	lda	BinaryASCII
	ora	a
	jnz	Charout		; transmit byte in Binary mode
	mov	a,c
	push	psw
	rar
	rar
	rar
	rar
	ani	0FH		; Shift HI nibble to LO nibble
	call	Nibout
	pop	psw
	ani	0FH
	jmp	Nibout

Netin:				; byte returned in A register
				; D  = checksum accumulator
	lda	BinaryASCII
	ora	a
	jz	ASCIIin
	call	charin		; receive byte in Binary mode
	rc
	jmp	chksin

ASCIIin:
	call	Nibin
	rc
	add	a
	add	a
	add	a
	add	a
	push	psw
	call	Nibin
	rc
	xthl
	ora	h
	pop	h
chksin:
	ora	a
	push	psw
	add	d		; add & update checksum accum.
	mov	d,a
	pop	psw
	ret

Msgin:				; HL = destination address
				; E  = # bytes to input
	call	Netin
	rc
	mov	m,a
	inx	h
	dcr	e
	jnz	Msgin
	ret

Msgout:				; HL = source address
				; E  = # bytes to output
				; D  = checksum
				; C  = preamble character
	mvi	d,0
	call	PreCharout

Msgoutloop:
	mov	c,m
	inx	h
	call	Netout
	dcr	e
	jnz	Msgoutloop
	ret

;	Network Initialization

nwinit:

;	device initialization, as required
;
;
;	Find address of XIOS console output routine

	lhld	0001h		; get warmstart entry in the XIOS jump table
	inx	h
	mov	e, m
	inx	h
	mov	d, m
	lxi	h, 0006h	; Offset for conin routine
	dad	d
	shld	conin		; save the address
	xra	a		; return code is 0=success
	ret


;	Network Status

nwstat:				; C = Slave #
	mvi	b,0
	lxi	h,networkstatus
	dad	b
	mov	a,m
	mov	b,a
	ani	not (msgerr+msgovr)
	mov	m,a
	mov	a,b
	ret


;	Return Configuration Table Address

cfgadr:
	lxi	h,configtbl
	ret

;	Send Message on Network

sndmsg:				; DE = message addr
				;  C = Slave #
	mov	b,c
	mvi	a,max$retries	; A = max$retries

send:
	push	psw

	if	mutexout

;	Use mutual exclusion if it is possible for some unsolicited input
;	to stomp on your output (This is nice if you;re running some sort
;	of multi-drop protocol)

	push	b		
	push	d
	mvi	c,readq
	lxi	d,UQCBMXSXmitq
	call	bdos		; obtain mutual exclusion token
	pop	d
	pop	b
	endif

	xchg
	push	h
	di			; disable interrupts to avoid underrun
	mvi	c,ENQ
	call	Charout		; send ENQ
	call	getACK		; won't return on an error
	mvi	e,5
	mvi	c,SOH
	call	Msgout		; send SOH FMT DID SID FNC SIZ
	xra	a
	sub	d
	mov	c,a
	call	Netout		; send HCS (header checksum)
	call	getACK		; won't return on an error
	dcx	h
	mov	e,m
	inx	h
	inr	e
	mvi	c,STX
	call	Msgout		; send STX DB0 DB1 ...
	mvi	c,ETX
	call	PreCharout	; send ETX
	xra	a
	sub	d
	mov	c,a
	call	Netout		; send CKS
	mvi	c,EOT
	call	PreCharout	; send EOT
	call	getACK		; won't return on an error
	pop	d		; discard message address
	pop	psw		; discard retry counter

	if	mutexout
	call	release$MX
	endif

	ei			; return from suspended animation
	xra	a
	ret			; A = 0, successful send message

getACK:
	call	Charin
	jc	getACK$timeout	; receive timeout-->start error recovery
	ani	7fh
	sui	ACK
	rz

getACK$timeout:
	pop	d		; discard return address

	if	mutexout
	push	b
	call	release$MX
	pop	b
	endif

	pop	d		; DE = message address
	pop	psw		; A = retry count
	dcr	a
	jnz	send		; continue if retry count non-zero
	dcr	a		; else-->we're dead-->A = 0ffh
	ret			; failed to send message

	if	mutexin or mutexout

release$MX:			; send back requester transmit MX message
	mvi	c,writeq
	lxi	d,UQCBMXSXmitq
	jmp	bdos
	endif

;	Receive Message from Network
rcvmsg:				; DE = message addr
				;  C = Slave #
	mov	b,c
receive:
	xchg
	push	h
	call	get$ENQ

; 	a return to this point indicates an error
receive$retry:
	ei			; re-enable other processes

	if	mutexin
	push	b
	call	release$MX
	pop	b
	endif

	pop	d
	jmp	receive

get$ENQ:			; get first character of message using
				; polled console I/O
	call	xCharin
	jc	get$ENQ
	ani	7fh
	cpi	ENQ		; Start of Message ?
	jnz	get$ENQ

	if	mutexin

;	Don't get too involved with receiving a message if some other
;	NETWRKIF process is going to stomp you by sending a message along
;	the same line

	push	b
	push	h
	mvi	c,readq
	lxi	d,UQCBMXSXmitq
	call	bdos
	pop	h
	pop	b
	endif

	mvi	c,ACK
	di			; requester in gear now serve only him

	call	charout		; send ACK to requester, allowing transmit
	call	Charin
	rc
	ani	7fh
	cpi	SOH
	rnz
	mov	d,a		; initialize the HCS
	mvi	e,5
	call	Msgin
	cnc	Netin
	rc
	mov	a,d
	ora	a
	jnz	sendNAK		; jmp & send NAK if HCS <> 0
	mvi	c,ACK
	call	Charout
	call	Charin
	rc
	ani	7fh
	cpi	STX
	rnz
	mov	d,a		; initialize the CKS
	dcx	h
	mov	e,m
	inx	h
	inr	e
	call	msgin
	cnc	Charin
	rc
	ani	7fh
	cpi	ETX
	rnz
	add	d
	mov	d,a
	call	Netin		; get Checksum byte
	rc
	mov	a,d
	ora	a		; should be zero
	jz	sendACK		; jump if checksum OK

sendNAK:			; else-->refuse the message
	mvi	c,NAK
	jmp	Charout		; send NAK and return to receive$retry

sendACK:			; come here if message was received properly
	call	Charin		; get EOT
	rc
	ani	7fh
	cpi	EOT
	rnz
	mvi	c,ACK
	call	Charout		; send ACK if checksum ok
	pop	d		; discard return address
	pop	d		; discard message address
	ei			; Dispense with the Rip Van Winkle act

	if	mutexin
	call	release$MX
	endif

	xra	a
	ret


restore:			

; 	This routine allows N copies of NtwrkIPx to run reentrantly.  
;	It takes the values that were pre-initialized in the process
;	descriptor and later saved on the stack and loads them into
;	the registers, leaving the stack image untouched.  All variables
;	intrinsic to the process therefore always reside on the 
;	process-dependent stack

	di			; this is a real critical region
	pop	h
	shld	rtnadr
	pop	h
	pop	d
	pop	b
	pop	psw
	push	psw
	push	b
	push	d
	push	h
	push	h
	lhld	rtnadr
	xthl
	ei
	ret

rtnadr:	ds	2

	if	WtchDg

;	WatchDog Timer Process
;	This process needs adjunct processes to handle the timeout flags
;	that it sets.  They might possibly abort the offending NtwrkIPx
;	process, recreate it, and allow it to re-initialize its queues

WatchDog:
	mvi	c,Delay
	lxi	d,60		; delay for 1 second
	call	bdos
	lhld	WatchDogTime
	inx	h
	shld	WatchDogTime
	lxi	h,WatchDogTable-5
	mvi	c,NmbSlvs

WatchDogLoop:
	lxi	d,0005h
	dad	d
	mov	a,m
	ora	a
	jz	WatchDogDec
	inx	h
	ana	m
	dcx	h
	jnz	WatchDogDec	; waiting & timeout set
	push	h		; save HL -> WDT.waiting
	inx	h
	inx	h
	di
	mov	e,m
	inx	h
	mov	d,m
	ei
	lhld	WatchDogTime
	mov	a,l
	sub	e
	mov	l,a
	mov	a,h
	sbb	d
	mov	h,a
	mvi	a,10		; # seconds since started Charin
	sub	l
	mvi	a,0
	sbb	h
	pop	h
	jnc	WatchDogDec
	push	h
	inx	h
	mvi	m,0ffh		; WDT.timeout = 0ffh
	inx	h
	inx	h
	inx	h
	push	b
	mov	e,m		; E = Flag #
	mvi	c,Flagset
	call	bdos
	pop	b
	pop	h

WatchDogDec:
	dcr	c
	jnz	WatchDogLoop

	jmp	WatchDog
	endif


;	Setup code for Network Interface Procedures

Setup:
	push	psw		; create stack image of all reentrant variables
	push	b
	push	d
	push	h
	call	nwinit

	if	mutexin or mutexout
	mvi	c,makeq		; make the mutual exclusion queue
	lxi	d,QCBMXSXmitq
	call	bdos

	mvi	c,writeq	; leave a token in the queue
	lxi	d,UQCBMXSXmitq
	call	bdos
	endif

	if	WtchDg
	lxi	d,WatchDogPD	; since this process is linked to all other
				; NtwrkIPx processes, creating it creates all
				; of the others
	mvi	c,createp
	call	bdos

	else

	if	NmbSlvs GE 2
	lxi	d,NtwrkIP1	; this will create all the other NtwrkIPx
				; processes if there's no watchdog
	mvi	c,createp
	call	bdos
	endif
	endif

	mvi	c,dsptch	; give everything a chance to create its queues
	call	bdos

	mvi	c,sydatad
	call	bdos
	lxi	d,9
	dad	d
	lxi	d,configtbl
	mov	m,e
	inx	h
	mov	m,d		; sysdatpage(9&10) = co.configtbl
				; filling in the config tbl address is the
				; the server processes' cue to start

	pop	h
	pop	d
	pop	b
	pop	psw

;  Network Interface Reentrant Procedure

Init:
	push	psw	; A = network i/f console dev #
	push	B	; BC= buffer address
	push	D	; DE= UQCB ntwrk queue out
	push	H	; HL= UQCB ntwrk queue in
	mov	e,m
	inx	h
	mov	d,m
	mvi	c,makeq
	call	bdos	; make the ntwrk queue in
	call	restore
	xchg
	mov	e,m
	inx	h
	mov	d,m
	mvi	c,makeq
	call	bdos	; make the ntwrk queue out

Loop:
	call	restore
	mov	d,b
	mov	e,c

	mov	c,a
	call	rcvmsg

	call	restore
	xchg
	mvi	c,writeq
	call	bdos

	call	restore
	mvi	c,readq
	call	bdos

	call	restore
	mov	d,b
	mov	e,c

	mov	c,a
	call	sndmsg

	jmp	Loop

	end

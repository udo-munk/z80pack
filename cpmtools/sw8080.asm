;	Switch to 8080 mode tool
;
;	Copyright (C) 2024 by Thomas Eberhardt
;
	title	'Switch to 8080 mode tool'

	.8080
	aseg
	org	100h

bdos	equ	5
hwctl	equ	0a0h
hwunlk	equ	0aah
set8080	equ	16

	xra	a
	dcr	a
	jpe	a8080		; even parity -> already in 8080 mode
	in	hwctl		; check if hardware control port is unlocked
	ora	a
	jz	unlckd
	mvi	a,hwunlk	; unlock hardware control port
	out	hwctl
unlckd:	mvi	a,set8080	; switch to 8080 mode
	out	hwctl
	lxi	d,sw8080
	jmp	domsg
a8080:	lxi	d,is8080
domsg:	mvi	c,9
	jmp	bdos

is8080:	db	'Already in 8080 mode$'
sw8080:	db	'Switched to 8080 mode$'

	end

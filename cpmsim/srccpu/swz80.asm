;	Switch to Z80 mode tool
;
;	Copyright (C) 2024 by Thomas Eberhardt
;
	title	'Switch to Z80 mode tool'

	.8080
	aseg
	org	100h

bdos	equ	5
wboot	equ	0
hwctl	equ	0a0h
hwunlk	equ	0aah
setz80	equ	32

	xra	a
	dcr	a
	jpo	az80		; odd parity -> already in Z80 mode
	in	hwctl		; check if hardware control port is unlocked
	ora	a
	jz	unlckd
	mvi	a,hwunlk	; unlock hardware control port
	out	hwctl
unlckd:	mvi	a,setz80	; switch to Z80 mode
	out	hwctl
	lxi	d,swz80
	jmp	domsg
az80:	lxi	d,isz80
domsg:	mvi	c,9
	call	bdos
	jmp	wboot

isz80:	db	'Already in Z80 mode$'
swz80:	db	'Switched to Z80 mode$'

	TITLE	'Test program for Z80-Disassembler'

;==========================================================================
;	Test program for Z80-Disassembler
;==========================================================================

	LD	SP,STACK	; initialize stack for simulator
	LD	HL,Z80OPS	; start address for disassembler
	LD	(DADDR),HL
LOOP:
	CALL	DISSCR		; disassemble one screen full
	HALT			; stop simulation
	JP	LOOP		; next run

PRTSTR:				; print zero terminated string
	LD	A,(HL)		; next char -> A
	OR	A		; zero ?
	RET	Z		; yes, done
	OUT	(1),A		; no, print it
	INC	HL		; increase pointer to string
	JP	PRTSTR		; process next char

	INCLUDE z80dis.asm
	INCLUDE z80ops.asm

	DEFS	100H
STACK:
	END

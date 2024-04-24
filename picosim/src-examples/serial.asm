;
; Example program for the Pico Z80
; Do some serial I/O.
;
; Udo Munk, April 2024
;
	ORG	0000H			; starting at memory location 0

CR	EQU	13			; carriage return
LF	EQU	10			; line feed

TTYDAT	EQU	1			; tty data port

	LD	SP,0F000H		; setup stack in upper memory
	LD	HL,TEXT			; load HL with text to send
LOOP:	LD	A,(HL)			; get next character
	OR	A			; is it 0 termination?
	JR	Z,DONE			; if yes done
	OUT	(TTYDAT),A		; output it to tty port
	INC	HL			; point to next character
	JR	LOOP			; and again
DONE:	HALT				; done

TEXT:	DEFM	"Hello from the Z80 CPU"
	DEFB	CR,LF,0

	END

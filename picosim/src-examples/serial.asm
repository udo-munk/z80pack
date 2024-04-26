;
; Example program for the Pico Z80
; Do some serial I/O.
;
; Udo Munk, April 2024
;
	ORG	0000H			; starting at memory location 0

CR	EQU	13			; carriage return
LF	EQU	10			; line feed

TTYSTA	EQU	0			; tty status port
TTYDAT	EQU	1			; tty data port

	DI				; disable interrupts
	LD	SP,0F000H		; setup stack in upper memory

					; print instructions
	LD	HL,TEXT			; HL points to text to send
LOOP:	LD	A,(HL)			; get next character into A
	OR	A			; is it 0 termination?
	JR	Z,ECHO			; if yes done
	CALL	OUTCH			; output character to tty
	INC	HL			; point to next character
	JR	LOOP			; and again

					; now echo what is typed
ECHO:	IN	A,(TTYSTA)		; get tty status
	RRCA
	JR	C,ECHO			; wait if not ready
	IN	A,(TTYDAT)		; get character from tty into A
	CALL	OUTCH			; echo it
	CP	'X'			; is it a X?
	JR	Z,DONE			; done if so
	CP	CR			; is it a carriage return?
	JR	NZ,ECHO			; no, go on
	LD	A,LF			; else also send line feed
	CALL	OUTCH
	JR	ECHO			; repeat

DONE:	HALT				; done

					; output character in A to tty
OUTCH:	PUSH	AF			; save character in A
OUTCH1:	IN	A,(TTYSTA)		; get tty status
	RLCA
	JR	C,OUTCH1		; wait if not ready
	POP	AF			; get character back into A
	OUT	(TTYDAT),A		; send character to tty
	RET

TEXT:	DEFM	"Hello from the Z80 CPU"
	DEFB	CR,LF
	DEFM	"I will echo what you type, type X if done"
	DEFB	CR,LF,0

	END

;
;	Program to reset z80pack systems via hardware control port
;
;	May 2018, Udo Munk
;
	ORG	0100H

HWCTL	EQU	0A0H		;hardware control port

	IN	HWCTL		;is the port locked?
	ORA	A
	JZ	UNLCK		;continue if not
	MVI	A,0AAH		;is locked, unlock with magic number
	OUT	HWCTL
UNLCK:	MVI	A,040H		;now reset system via hardware control
	OUT	HWCTL

	RET			;if that didn't work return to CP/M

	END

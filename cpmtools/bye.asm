;
;	Program to shutdown z80pack systems via hardware control port
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
UNLCK:	MVI	A,080H		;now stop system via hardware control
	OUT	HWCTL

	DI			;if that didn't work halt system
	HLT

	RET			;if it gets out of halted state return to CP/M

	END

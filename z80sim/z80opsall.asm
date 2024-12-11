	TITLE	'Z80 Instr. in order (incl. undoc''d and all index variants)'
IND	EQU	5
M	EQU	10H
N	EQU	20H
DIS	EQU	30H

Z80OPS:
	ADC	A,(HL)
	ADC	A,(IX)
	ADC	A,(IX+IND)
	ADC	A,(IX-IND)
	ADC	A,(IY)
	ADC	A,(IY+IND)
	ADC	A,(IY-IND)
	ADC	A,A
	ADC	A,B
	ADC	A,C
	ADC	A,D
	ADC	A,E
	ADC	A,H
	ADC	A,IXH			; Undocumented
	ADC	A,IXL			; Undocumented
	ADC	A,IYH			; Undocumented
	ADC	A,IYL			; Undocumented
	ADC	A,L
	ADC	A,N
	ADC	HL,BC
	ADC	HL,DE
	ADC	HL,HL
	ADC	HL,SP
	ADD	A,(HL)
	ADD	A,(IX)
	ADD	A,(IX+IND)
	ADD	A,(IX-IND)
	ADD	A,(IY)
	ADD	A,(IY+IND)
	ADD	A,(IY-IND)
	ADD	A,A
	ADD	A,B
	ADD	A,C
	ADD	A,D
	ADD	A,E
	ADD	A,H
	ADD	A,IXH			; Undocumented
	ADD	A,IXL			; Undocumented
	ADD	A,IYH			; Undocumented
	ADD	A,IYL			; Undocumented
	ADD	A,L
	ADD	A,N
	ADD	HL,BC
	ADD	HL,DE
	ADD	HL,HL
	ADD	HL,SP
	ADD	IX,BC
	ADD	IX,DE
	ADD	IX,IX
	ADD	IX,SP
	ADD	IY,BC
	ADD	IY,DE
	ADD	IY,IY
	ADD	IY,SP
	AND	(HL)
	AND	(IX)
	AND	(IX+IND)
	AND	(IX-IND)
	AND	(IY)
	AND	(IY+IND)
	AND	(IY-IND)
	AND	A
	AND	B
	AND	C
	AND	D
	AND	E
	AND	H
	AND	IXH			; Undocumented
	AND	IXL			; Undocumented
	AND	IYH			; Undocumented
	AND	IYL			; Undocumented
	AND	L
	AND	N
	BIT	0,(HL)
	BIT	0,(IX)
	BIT	0,(IX+IND)
	BIT	0,(IX-IND)
	BIT	0,(IY)
	BIT	0,(IY+IND)
	BIT	0,(IY-IND)
	BIT	0,A
	BIT	0,B
	BIT	0,C
	BIT	0,D
	BIT	0,E
	BIT	0,H
	BIT	0,L
	BIT	1,(HL)
	BIT	1,(IX)
	BIT	1,(IX+IND)
	BIT	1,(IX-IND)
	BIT	1,(IY)
	BIT	1,(IY+IND)
	BIT	1,(IY-IND)
	BIT	1,A
	BIT	1,B
	BIT	1,C
	BIT	1,D
	BIT	1,E
	BIT	1,H
	BIT	1,L
	BIT	2,(HL)
	BIT	2,(IX)
	BIT	2,(IX+IND)
	BIT	2,(IX-IND)
	BIT	2,(IY)
	BIT	2,(IY+IND)
	BIT	2,(IY-IND)
	BIT	2,A
	BIT	2,B
	BIT	2,C
	BIT	2,D
	BIT	2,E
	BIT	2,H
	BIT	2,L
	BIT	3,(HL)
	BIT	3,(IX)
	BIT	3,(IX+IND)
	BIT	3,(IX-IND)
	BIT	3,(IY)
	BIT	3,(IY+IND)
	BIT	3,(IY-IND)
	BIT	3,A
	BIT	3,B
	BIT	3,C
	BIT	3,D
	BIT	3,E
	BIT	3,H
	BIT	3,L
	BIT	4,(HL)
	BIT	4,(IX)
	BIT	4,(IX+IND)
	BIT	4,(IX-IND)
	BIT	4,(IY)
	BIT	4,(IY+IND)
	BIT	4,(IY-IND)
	BIT	4,A
	BIT	4,B
	BIT	4,C
	BIT	4,D
	BIT	4,E
	BIT	4,H
	BIT	4,L
	BIT	5,(HL)
	BIT	5,(IX)
	BIT	5,(IX+IND)
	BIT	5,(IX-IND)
	BIT	5,(IY)
	BIT	5,(IY+IND)
	BIT	5,(IY-IND)
	BIT	5,A
	BIT	5,B
	BIT	5,C
	BIT	5,D
	BIT	5,E
	BIT	5,H
	BIT	5,L
	BIT	6,(HL)
	BIT	6,(IX)
	BIT	6,(IX+IND)
	BIT	6,(IX-IND)
	BIT	6,(IY)
	BIT	6,(IY+IND)
	BIT	6,(IY-IND)
	BIT	6,A
	BIT	6,B
	BIT	6,C
	BIT	6,D
	BIT	6,E
	BIT	6,H
	BIT	6,L
	BIT	7,(HL)
	BIT	7,(IX)
	BIT	7,(IX+IND)
	BIT	7,(IX-IND)
	BIT	7,(IY)
	BIT	7,(IY+IND)
	BIT	7,(IY-IND)
	BIT	7,A
	BIT	7,B
	BIT	7,C
	BIT	7,D
	BIT	7,E
	BIT	7,H
	BIT	7,L
	CALL	C,NN
	CALL	M,NN
	CALL	NC,NN
	CALL	NN
	CALL	NZ,NN
	CALL	P,NN
	CALL	PE,NN
	CALL	PO,NN
	CALL	Z,NN
	CCF
	CP	(HL)
	CP	(IX)
	CP	(IX+IND)
	CP	(IX-IND)
	CP	(IY)
	CP	(IY+IND)
	CP	(IY-IND)
	CP	A
	CP	B
	CP	C
	CP	D
	CP	E
	CP	H
	CP	IXH			; Undocumented
	CP	IXL			; Undocumented
	CP	IYH			; Undocumented
	CP	IYL			; Undocumented
	CP	L
	CP	N
	CPD
	CPDR
	CPI
	CPIR
	CPL
	DAA
	DEC	(HL)
	DEC	(IX)
	DEC	(IX+IND)
	DEC	(IX-IND)
	DEC	(IY)
	DEC	(IY+IND)
	DEC	(IY-IND)
	DEC	A
	DEC	B
	DEC	BC
	DEC	C
	DEC	D
	DEC	DE
	DEC	E
	DEC	H
	DEC	HL
	DEC	IX
	DEC	IXH			; Undocumented
	DEC	IXL			; Undocumented
	DEC	IY
	DEC	IYH			; Undocumented
	DEC	IYL			; Undocumented
	DEC	L
	DEC	SP
	DI
	DJNZ	$+DIS
	EI
	EX	(SP),HL
	EX	(SP),IX
	EX	(SP),IY
	EX	AF,AF'
	EX	DE,HL
	EXX
	HALT
	IM	0
	IM	1
	IM	2
	IN	A,(C)
	IN	A,(N)
	IN	B,(C)
	IN	C,(C)
	IN	D,(C)
	IN	E,(C)
	IN	F,(C)			; Undocumented
	IN	H,(C)
	IN	L,(C)
	INC	(HL)
	INC	(IX)
	INC	(IX+IND)
	INC	(IX-IND)
	INC	(IY)
	INC	(IY+IND)
	INC	(IY-IND)
	INC	A
	INC	B
	INC	BC
	INC	C
	INC	D
	INC	DE
	INC	E
	INC	H
	INC	HL
	INC	IX
	INC	IXH			; Undocumented
	INC	IXL			; Undocumented
	INC	IY
	INC	IYH			; Undocumented
	INC	IYL			; Undocumented
	INC	L
	INC	SP
	IND
	INDR
	INI
	INIR
	JP	(HL)
	JP	(IX)
	JP	(IY)
	JP	C,NN
	JP	M,NN
	JP	NC,NN
	JP	NN
	JP	NZ,NN
	JP	P,NN
	JP	PE,NN
	JP	PO,NN
	JP	Z,NN
	JR	$+DIS
	JR	C,$+DIS
	JR	NC,$+DIS
	JR	NZ,$+DIS
	JR	Z,$+DIS
	LD	(BC),A
	LD	(DE),A
	LD	(HL),A
	LD	(HL),B
	LD	(HL),C
	LD	(HL),D
	LD	(HL),E
	LD	(HL),H
	LD	(HL),L
	LD	(HL),N
	LD	(IX),A
	LD	(IX),B
	LD	(IX),C
	LD	(IX),D
	LD	(IX),E
	LD	(IX),H
	LD	(IX),L
	LD	(IX),N
	LD	(IX+IND),A
	LD	(IX+IND),B
	LD	(IX+IND),C
	LD	(IX+IND),D
	LD	(IX+IND),E
	LD	(IX+IND),H
	LD	(IX+IND),L
	LD	(IX+IND),N
	LD	(IX-IND),A
	LD	(IX-IND),B
	LD	(IX-IND),C
	LD	(IX-IND),D
	LD	(IX-IND),E
	LD	(IX-IND),H
	LD	(IX-IND),L
	LD	(IX-IND),N
	LD	(IY),A
	LD	(IY),B
	LD	(IY),C
	LD	(IY),D
	LD	(IY),E
	LD	(IY),H
	LD	(IY),L
	LD	(IY),N
	LD	(IY+IND),A
	LD	(IY+IND),B
	LD	(IY+IND),C
	LD	(IY+IND),D
	LD	(IY+IND),E
	LD	(IY+IND),H
	LD	(IY+IND),L
	LD	(IY+IND),N
	LD	(IY-IND),A
	LD	(IY-IND),B
	LD	(IY-IND),C
	LD	(IY-IND),D
	LD	(IY-IND),E
	LD	(IY-IND),H
	LD	(IY-IND),L
	LD	(IY-IND),N
	LD	(NN),A
	LD	(NN),BC
	LD	(NN),DE
	LD	(NN),HL
	LD	(NN),IX
	LD	(NN),IY
	LD	(NN),SP
	LD	A,(BC)
	LD	A,(DE)
	LD	A,(HL)
	LD	A,(IX)
	LD	A,(IX+IND)
	LD	A,(IX-IND)
	LD	A,(IY)
	LD	A,(IY+IND)
	LD	A,(IY-IND)
	LD	A,(NN)
	LD	A,A
	LD	A,B
	LD	A,C
	LD	A,D
	LD	A,E
	LD	A,H
	LD	A,I
	LD	A,IXH			; Undocumented
	LD	A,IXL			; Undocumented
	LD	A,IYH			; Undocumented
	LD	A,IYL			; Undocumented
	LD	A,L
	LD	A,N
	LD	B,(HL)
	LD	B,(IX)
	LD	B,(IX+IND)
	LD	B,(IX-IND)
	LD	B,(IY)
	LD	B,(IY+IND)
	LD	B,(IY-IND)
	LD	B,A
	LD	B,B
	LD	B,C
	LD	B,D
	LD	B,E
	LD	B,H
	LD	B,IXH			; Undocumented
	LD	B,IXL			; Undocumented
	LD	B,IYH			; Undocumented
	LD	B,IYL			; Undocumented
	LD	B,L
	LD	B,N
	LD	BC,(NN)
	LD	BC,NN
	LD	C,(HL)
	LD	C,(IX)
	LD	C,(IX+IND)
	LD	C,(IX-IND)
	LD	C,(IY)
	LD	C,(IY+IND)
	LD	C,(IY-IND)
	LD	C,A
	LD	C,B
	LD	C,C
	LD	C,D
	LD	C,E
	LD	C,H
	LD	C,IXH			; Undocumented
	LD	C,IXL			; Undocumented
	LD	C,IYH			; Undocumented
	LD	C,IYL			; Undocumented
	LD	C,L
	LD	C,N
	LD	D,(HL)
	LD	D,(IX)
	LD	D,(IX+IND)
	LD	D,(IX-IND)
	LD	D,(IY)
	LD	D,(IY+IND)
	LD	D,(IY-IND)
	LD	D,A
	LD	D,B
	LD	D,C
	LD	D,D
	LD	D,E
	LD	D,H
	LD	D,IXH			; Undocumented
	LD	D,IXL			; Undocumented
	LD	D,IYH			; Undocumented
	LD	D,IYL			; Undocumented
	LD	D,L
	LD	D,N
	LD	DE,(NN)
	LD	DE,NN
	LD	E,(HL)
	LD	E,(IX)
	LD	E,(IX+IND)
	LD	E,(IX-IND)
	LD	E,(IY)
	LD	E,(IY+IND)
	LD	E,(IY-IND)
	LD	E,A
	LD	E,B
	LD	E,C
	LD	E,D
	LD	E,E
	LD	E,H
	LD	E,IXH			; Undocumented
	LD	E,IXL			; Undocumented
	LD	E,IYH			; Undocumented
	LD	E,IYL			; Undocumented
	LD	E,L
	LD	E,N
	LD	H,(HL)
	LD	H,(IX)
	LD	H,(IX+IND)
	LD	H,(IX-IND)
	LD	H,(IY)
	LD	H,(IY+IND)
	LD	H,(IY-IND)
	LD	H,A
	LD	H,B
	LD	H,C
	LD	H,D
	LD	H,E
	LD	H,H
	LD	H,L
	LD	H,N
	LD	HL,(NN)
	LD	HL,NN
	LD	I,A
	LD	IX,(NN)
	LD	IX,NN
	LD	IXH,A			; Undocumented
	LD	IXH,B			; Undocumented
	LD	IXH,C			; Undocumented
	LD	IXH,D			; Undocumented
	LD	IXH,E			; Undocumented
	LD	IXH,IXH			; Undocumented
	LD	IXH,IXL			; Undocumented
	LD	IXH,N			; Undocumented
	LD	IXL,A			; Undocumented
	LD	IXL,B			; Undocumented
	LD	IXL,C			; Undocumented
	LD	IXL,D			; Undocumented
	LD	IXL,E			; Undocumented
	LD	IXL,IXH			; Undocumented
	LD	IXL,IXL			; Undocumented
	LD	IXL,N			; Undocumented
	LD	IY,(NN)
	LD	IY,NN
	LD	IYH,A			; Undocumented
	LD	IYH,B			; Undocumented
	LD	IYH,C			; Undocumented
	LD	IYH,D			; Undocumented
	LD	IYH,E			; Undocumented
	LD	IYH,IYH			; Undocumented
	LD	IYH,IYL			; Undocumented
	LD	IYH,N			; Undocumented
	LD	IYL,A			; Undocumented
	LD	IYL,B			; Undocumented
	LD	IYL,C			; Undocumented
	LD	IYL,D			; Undocumented
	LD	IYL,E			; Undocumented
	LD	IYL,IYH			; Undocumented
	LD	IYL,IYL			; Undocumented
	LD	IYL,N			; Undocumented
	LD	L,(HL)
	LD	L,(IX)
	LD	L,(IX+IND)
	LD	L,(IX-IND)
	LD	L,(IY)
	LD	L,(IY+IND)
	LD	L,(IY-IND)
	LD	L,A
	LD	L,B
	LD	L,C
	LD	L,D
	LD	L,E
	LD	L,H
	LD	L,L
	LD	L,N
	LD	SP,(NN)
	LD	SP,HL
	LD	SP,IX
	LD	SP,IY
	LD	SP,NN
	LDD
	LDDR
	LDI
	LDIR
	NEG
	NOP
	OR	(HL)
	OR	(IX)
	OR	(IX+IND)
	OR	(IX-IND)
	OR	(IY)
	OR	(IY+IND)
	OR	(IY-IND)
	OR	A
	OR	B
	OR	C
	OR	D
	OR	E
	OR	H
	OR	IXH			; Undocumented
	OR	IXL			; Undocumented
	OR	IYH			; Undocumented
	OR	IYL			; Undocumented
	OR	L
	OR	N
	OTDR
	OTIR
	OUT	(C),0			; Undocumented
	OUT	(C),A
	OUT	(C),B
	OUT	(C),C
	OUT	(C),D
	OUT	(C),E
	OUT	(C),H
	OUT	(C),L
	OUT	(N),A
	OUTD
	OUTI
	POP	AF
	POP	BC
	POP	DE
	POP	HL
	POP	IX
	POP	IY
	PUSH	AF
	PUSH	BC
	PUSH	DE
	PUSH	HL
	PUSH	IX
	PUSH	IY
	RES	0,(HL)
	RES	0,(IX)
	RES	0,(IX),A		; Undocumented
	RES	0,(IX),B		; Undocumented
	RES	0,(IX),C		; Undocumented
	RES	0,(IX),D		; Undocumented
	RES	0,(IX),E		; Undocumented
	RES	0,(IX),H		; Undocumented
	RES	0,(IX),L		; Undocumented
	RES	0,(IX+IND)
	RES	0,(IX+IND),A		; Undocumented
	RES	0,(IX+IND),B		; Undocumented
	RES	0,(IX+IND),C		; Undocumented
	RES	0,(IX+IND),D		; Undocumented
	RES	0,(IX+IND),E		; Undocumented
	RES	0,(IX+IND),H		; Undocumented
	RES	0,(IX+IND),L		; Undocumented
	RES	0,(IX-IND)
	RES	0,(IX-IND),A		; Undocumented
	RES	0,(IX-IND),B		; Undocumented
	RES	0,(IX-IND),C		; Undocumented
	RES	0,(IX-IND),D		; Undocumented
	RES	0,(IX-IND),E		; Undocumented
	RES	0,(IX-IND),H		; Undocumented
	RES	0,(IX-IND),L		; Undocumented
	RES	0,(IY)
	RES	0,(IY),A		; Undocumented
	RES	0,(IY),B		; Undocumented
	RES	0,(IY),C		; Undocumented
	RES	0,(IY),D		; Undocumented
	RES	0,(IY),E		; Undocumented
	RES	0,(IY),H		; Undocumented
	RES	0,(IY),L		; Undocumented
	RES	0,(IY+IND)
	RES	0,(IY+IND),A		; Undocumented
	RES	0,(IY+IND),B		; Undocumented
	RES	0,(IY+IND),C		; Undocumented
	RES	0,(IY+IND),D		; Undocumented
	RES	0,(IY+IND),E		; Undocumented
	RES	0,(IY+IND),H		; Undocumented
	RES	0,(IY+IND),L		; Undocumented
	RES	0,(IY-IND)
	RES	0,(IY-IND),A		; Undocumented
	RES	0,(IY-IND),B		; Undocumented
	RES	0,(IY-IND),C		; Undocumented
	RES	0,(IY-IND),D		; Undocumented
	RES	0,(IY-IND),E		; Undocumented
	RES	0,(IY-IND),H		; Undocumented
	RES	0,(IY-IND),L		; Undocumented
	RES	0,A
	RES	0,B
	RES	0,C
	RES	0,D
	RES	0,E
	RES	0,H
	RES	0,L
	RES	1,(HL)
	RES	1,(IX)
	RES	1,(IX),A		; Undocumented
	RES	1,(IX),B		; Undocumented
	RES	1,(IX),C		; Undocumented
	RES	1,(IX),D		; Undocumented
	RES	1,(IX),E		; Undocumented
	RES	1,(IX),H		; Undocumented
	RES	1,(IX),L		; Undocumented
	RES	1,(IX+IND)
	RES	1,(IX+IND),A		; Undocumented
	RES	1,(IX+IND),B		; Undocumented
	RES	1,(IX+IND),C		; Undocumented
	RES	1,(IX+IND),D		; Undocumented
	RES	1,(IX+IND),E		; Undocumented
	RES	1,(IX+IND),H		; Undocumented
	RES	1,(IX+IND),L		; Undocumented
	RES	1,(IX-IND)
	RES	1,(IX-IND),A		; Undocumented
	RES	1,(IX-IND),B		; Undocumented
	RES	1,(IX-IND),C		; Undocumented
	RES	1,(IX-IND),D		; Undocumented
	RES	1,(IX-IND),E		; Undocumented
	RES	1,(IX-IND),H		; Undocumented
	RES	1,(IX-IND),L		; Undocumented
	RES	1,(IY)
	RES	1,(IY),A		; Undocumented
	RES	1,(IY),B		; Undocumented
	RES	1,(IY),C		; Undocumented
	RES	1,(IY),D		; Undocumented
	RES	1,(IY),E		; Undocumented
	RES	1,(IY),H		; Undocumented
	RES	1,(IY),L		; Undocumented
	RES	1,(IY+IND)
	RES	1,(IY+IND),A		; Undocumented
	RES	1,(IY+IND),B		; Undocumented
	RES	1,(IY+IND),C		; Undocumented
	RES	1,(IY+IND),D		; Undocumented
	RES	1,(IY+IND),E		; Undocumented
	RES	1,(IY+IND),H		; Undocumented
	RES	1,(IY+IND),L		; Undocumented
	RES	1,(IY-IND)
	RES	1,(IY-IND),A		; Undocumented
	RES	1,(IY-IND),B		; Undocumented
	RES	1,(IY-IND),C		; Undocumented
	RES	1,(IY-IND),D		; Undocumented
	RES	1,(IY-IND),E		; Undocumented
	RES	1,(IY-IND),H		; Undocumented
	RES	1,(IY-IND),L		; Undocumented
	RES	1,A
	RES	1,B
	RES	1,C
	RES	1,D
	RES	1,E
	RES	1,H
	RES	1,L
	RES	2,(HL)
	RES	2,(IX)
	RES	2,(IX),A		; Undocumented
	RES	2,(IX),B		; Undocumented
	RES	2,(IX),C		; Undocumented
	RES	2,(IX),D		; Undocumented
	RES	2,(IX),E		; Undocumented
	RES	2,(IX),H		; Undocumented
	RES	2,(IX),L		; Undocumented
	RES	2,(IX+IND)
	RES	2,(IX+IND),A		; Undocumented
	RES	2,(IX+IND),B		; Undocumented
	RES	2,(IX+IND),C		; Undocumented
	RES	2,(IX+IND),D		; Undocumented
	RES	2,(IX+IND),E		; Undocumented
	RES	2,(IX+IND),H		; Undocumented
	RES	2,(IX+IND),L		; Undocumented
	RES	2,(IX-IND)
	RES	2,(IX-IND),A		; Undocumented
	RES	2,(IX-IND),B		; Undocumented
	RES	2,(IX-IND),C		; Undocumented
	RES	2,(IX-IND),D		; Undocumented
	RES	2,(IX-IND),E		; Undocumented
	RES	2,(IX-IND),H		; Undocumented
	RES	2,(IX-IND),L		; Undocumented
	RES	2,(IY)
	RES	2,(IY),A		; Undocumented
	RES	2,(IY),B		; Undocumented
	RES	2,(IY),C		; Undocumented
	RES	2,(IY),D		; Undocumented
	RES	2,(IY),E		; Undocumented
	RES	2,(IY),H		; Undocumented
	RES	2,(IY),L		; Undocumented
	RES	2,(IY+IND)
	RES	2,(IY+IND),A		; Undocumented
	RES	2,(IY+IND),B		; Undocumented
	RES	2,(IY+IND),C		; Undocumented
	RES	2,(IY+IND),D		; Undocumented
	RES	2,(IY+IND),E		; Undocumented
	RES	2,(IY+IND),H		; Undocumented
	RES	2,(IY+IND),L		; Undocumented
	RES	2,(IY-IND)
	RES	2,(IY-IND),A		; Undocumented
	RES	2,(IY-IND),B		; Undocumented
	RES	2,(IY-IND),C		; Undocumented
	RES	2,(IY-IND),D		; Undocumented
	RES	2,(IY-IND),E		; Undocumented
	RES	2,(IY-IND),H		; Undocumented
	RES	2,(IY-IND),L		; Undocumented
	RES	2,A
	RES	2,B
	RES	2,C
	RES	2,D
	RES	2,E
	RES	2,H
	RES	2,L
	RES	3,(HL)
	RES	3,(IX)
	RES	3,(IX),A		; Undocumented
	RES	3,(IX),B		; Undocumented
	RES	3,(IX),C		; Undocumented
	RES	3,(IX),D		; Undocumented
	RES	3,(IX),E		; Undocumented
	RES	3,(IX),H		; Undocumented
	RES	3,(IX),L		; Undocumented
	RES	3,(IX+IND)
	RES	3,(IX+IND),A		; Undocumented
	RES	3,(IX+IND),B		; Undocumented
	RES	3,(IX+IND),C		; Undocumented
	RES	3,(IX+IND),D		; Undocumented
	RES	3,(IX+IND),E		; Undocumented
	RES	3,(IX+IND),H		; Undocumented
	RES	3,(IX+IND),L		; Undocumented
	RES	3,(IX-IND)
	RES	3,(IX-IND),A		; Undocumented
	RES	3,(IX-IND),B		; Undocumented
	RES	3,(IX-IND),C		; Undocumented
	RES	3,(IX-IND),D		; Undocumented
	RES	3,(IX-IND),E		; Undocumented
	RES	3,(IX-IND),H		; Undocumented
	RES	3,(IX-IND),L		; Undocumented
	RES	3,(IY)
	RES	3,(IY),A		; Undocumented
	RES	3,(IY),B		; Undocumented
	RES	3,(IY),C		; Undocumented
	RES	3,(IY),D		; Undocumented
	RES	3,(IY),E		; Undocumented
	RES	3,(IY),H		; Undocumented
	RES	3,(IY),L		; Undocumented
	RES	3,(IY+IND)
	RES	3,(IY+IND),A		; Undocumented
	RES	3,(IY+IND),B		; Undocumented
	RES	3,(IY+IND),C		; Undocumented
	RES	3,(IY+IND),D		; Undocumented
	RES	3,(IY+IND),E		; Undocumented
	RES	3,(IY+IND),H		; Undocumented
	RES	3,(IY+IND),L		; Undocumented
	RES	3,(IY-IND)
	RES	3,(IY-IND),A		; Undocumented
	RES	3,(IY-IND),B		; Undocumented
	RES	3,(IY-IND),C		; Undocumented
	RES	3,(IY-IND),D		; Undocumented
	RES	3,(IY-IND),E		; Undocumented
	RES	3,(IY-IND),H		; Undocumented
	RES	3,(IY-IND),L		; Undocumented
	RES	3,A
	RES	3,B
	RES	3,C
	RES	3,D
	RES	3,E
	RES	3,H
	RES	3,L
	RES	4,(HL)
	RES	4,(IX)
	RES	4,(IX),A		; Undocumented
	RES	4,(IX),B		; Undocumented
	RES	4,(IX),C		; Undocumented
	RES	4,(IX),D		; Undocumented
	RES	4,(IX),E		; Undocumented
	RES	4,(IX),H		; Undocumented
	RES	4,(IX),L		; Undocumented
	RES	4,(IX+IND)
	RES	4,(IX+IND),A		; Undocumented
	RES	4,(IX+IND),B		; Undocumented
	RES	4,(IX+IND),C		; Undocumented
	RES	4,(IX+IND),D		; Undocumented
	RES	4,(IX+IND),E		; Undocumented
	RES	4,(IX+IND),H		; Undocumented
	RES	4,(IX+IND),L		; Undocumented
	RES	4,(IX-IND)
	RES	4,(IX-IND),A		; Undocumented
	RES	4,(IX-IND),B		; Undocumented
	RES	4,(IX-IND),C		; Undocumented
	RES	4,(IX-IND),D		; Undocumented
	RES	4,(IX-IND),E		; Undocumented
	RES	4,(IX-IND),H		; Undocumented
	RES	4,(IX-IND),L		; Undocumented
	RES	4,(IY)
	RES	4,(IY),A		; Undocumented
	RES	4,(IY),B		; Undocumented
	RES	4,(IY),C		; Undocumented
	RES	4,(IY),D		; Undocumented
	RES	4,(IY),E		; Undocumented
	RES	4,(IY),H		; Undocumented
	RES	4,(IY),L		; Undocumented
	RES	4,(IY+IND)
	RES	4,(IY+IND),A		; Undocumented
	RES	4,(IY+IND),B		; Undocumented
	RES	4,(IY+IND),C		; Undocumented
	RES	4,(IY+IND),D		; Undocumented
	RES	4,(IY+IND),E		; Undocumented
	RES	4,(IY+IND),H		; Undocumented
	RES	4,(IY+IND),L		; Undocumented
	RES	4,(IY-IND)
	RES	4,(IY-IND),A		; Undocumented
	RES	4,(IY-IND),B		; Undocumented
	RES	4,(IY-IND),C		; Undocumented
	RES	4,(IY-IND),D		; Undocumented
	RES	4,(IY-IND),E		; Undocumented
	RES	4,(IY-IND),H		; Undocumented
	RES	4,(IY-IND),L		; Undocumented
	RES	4,A
	RES	4,B
	RES	4,C
	RES	4,D
	RES	4,E
	RES	4,H
	RES	4,L
	RES	5,(HL)
	RES	5,(IX)
	RES	5,(IX),A		; Undocumented
	RES	5,(IX),B		; Undocumented
	RES	5,(IX),C		; Undocumented
	RES	5,(IX),D		; Undocumented
	RES	5,(IX),E		; Undocumented
	RES	5,(IX),H		; Undocumented
	RES	5,(IX),L		; Undocumented
	RES	5,(IX+IND)
	RES	5,(IX+IND),A		; Undocumented
	RES	5,(IX+IND),B		; Undocumented
	RES	5,(IX+IND),C		; Undocumented
	RES	5,(IX+IND),D		; Undocumented
	RES	5,(IX+IND),E		; Undocumented
	RES	5,(IX+IND),H		; Undocumented
	RES	5,(IX+IND),L		; Undocumented
	RES	5,(IX-IND)
	RES	5,(IX-IND),A		; Undocumented
	RES	5,(IX-IND),B		; Undocumented
	RES	5,(IX-IND),C		; Undocumented
	RES	5,(IX-IND),D		; Undocumented
	RES	5,(IX-IND),E		; Undocumented
	RES	5,(IX-IND),H		; Undocumented
	RES	5,(IX-IND),L		; Undocumented
	RES	5,(IY)
	RES	5,(IY),A		; Undocumented
	RES	5,(IY),B		; Undocumented
	RES	5,(IY),C		; Undocumented
	RES	5,(IY),D		; Undocumented
	RES	5,(IY),E		; Undocumented
	RES	5,(IY),H		; Undocumented
	RES	5,(IY),L		; Undocumented
	RES	5,(IY+IND)
	RES	5,(IY+IND),A		; Undocumented
	RES	5,(IY+IND),B		; Undocumented
	RES	5,(IY+IND),C		; Undocumented
	RES	5,(IY+IND),D		; Undocumented
	RES	5,(IY+IND),E		; Undocumented
	RES	5,(IY+IND),H		; Undocumented
	RES	5,(IY+IND),L		; Undocumented
	RES	5,(IY-IND)
	RES	5,(IY-IND),A		; Undocumented
	RES	5,(IY-IND),B		; Undocumented
	RES	5,(IY-IND),C		; Undocumented
	RES	5,(IY-IND),D		; Undocumented
	RES	5,(IY-IND),E		; Undocumented
	RES	5,(IY-IND),H		; Undocumented
	RES	5,(IY-IND),L		; Undocumented
	RES	5,A
	RES	5,B
	RES	5,C
	RES	5,D
	RES	5,E
	RES	5,H
	RES	5,L
	RES	6,(HL)
	RES	6,(IX)
	RES	6,(IX),A		; Undocumented
	RES	6,(IX),B		; Undocumented
	RES	6,(IX),C		; Undocumented
	RES	6,(IX),D		; Undocumented
	RES	6,(IX),E		; Undocumented
	RES	6,(IX),H		; Undocumented
	RES	6,(IX),L		; Undocumented
	RES	6,(IX+IND)
	RES	6,(IX+IND),A		; Undocumented
	RES	6,(IX+IND),B		; Undocumented
	RES	6,(IX+IND),C		; Undocumented
	RES	6,(IX+IND),D		; Undocumented
	RES	6,(IX+IND),E		; Undocumented
	RES	6,(IX+IND),H		; Undocumented
	RES	6,(IX+IND),L		; Undocumented
	RES	6,(IX-IND)
	RES	6,(IX-IND),A		; Undocumented
	RES	6,(IX-IND),B		; Undocumented
	RES	6,(IX-IND),C		; Undocumented
	RES	6,(IX-IND),D		; Undocumented
	RES	6,(IX-IND),E		; Undocumented
	RES	6,(IX-IND),H		; Undocumented
	RES	6,(IX-IND),L		; Undocumented
	RES	6,(IY)
	RES	6,(IY),A		; Undocumented
	RES	6,(IY),B		; Undocumented
	RES	6,(IY),C		; Undocumented
	RES	6,(IY),D		; Undocumented
	RES	6,(IY),E		; Undocumented
	RES	6,(IY),H		; Undocumented
	RES	6,(IY),L		; Undocumented
	RES	6,(IY+IND)
	RES	6,(IY+IND),A		; Undocumented
	RES	6,(IY+IND),B		; Undocumented
	RES	6,(IY+IND),C		; Undocumented
	RES	6,(IY+IND),D		; Undocumented
	RES	6,(IY+IND),E		; Undocumented
	RES	6,(IY+IND),H		; Undocumented
	RES	6,(IY+IND),L		; Undocumented
	RES	6,(IY-IND)
	RES	6,(IY-IND),A		; Undocumented
	RES	6,(IY-IND),B		; Undocumented
	RES	6,(IY-IND),C		; Undocumented
	RES	6,(IY-IND),D		; Undocumented
	RES	6,(IY-IND),E		; Undocumented
	RES	6,(IY-IND),H		; Undocumented
	RES	6,(IY-IND),L		; Undocumented
	RES	6,A
	RES	6,B
	RES	6,C
	RES	6,D
	RES	6,E
	RES	6,H
	RES	6,L
	RES	7,(HL)
	RES	7,(IX)
	RES	7,(IX),A		; Undocumented
	RES	7,(IX),B		; Undocumented
	RES	7,(IX),C		; Undocumented
	RES	7,(IX),D		; Undocumented
	RES	7,(IX),E		; Undocumented
	RES	7,(IX),H		; Undocumented
	RES	7,(IX),L		; Undocumented
	RES	7,(IX+IND)
	RES	7,(IX+IND),A		; Undocumented
	RES	7,(IX+IND),B		; Undocumented
	RES	7,(IX+IND),C		; Undocumented
	RES	7,(IX+IND),D		; Undocumented
	RES	7,(IX+IND),E		; Undocumented
	RES	7,(IX+IND),H		; Undocumented
	RES	7,(IX+IND),L		; Undocumented
	RES	7,(IX-IND)
	RES	7,(IX-IND),A		; Undocumented
	RES	7,(IX-IND),B		; Undocumented
	RES	7,(IX-IND),C		; Undocumented
	RES	7,(IX-IND),D		; Undocumented
	RES	7,(IX-IND),E		; Undocumented
	RES	7,(IX-IND),H		; Undocumented
	RES	7,(IX-IND),L		; Undocumented
	RES	7,(IY)
	RES	7,(IY),A		; Undocumented
	RES	7,(IY),B		; Undocumented
	RES	7,(IY),C		; Undocumented
	RES	7,(IY),D		; Undocumented
	RES	7,(IY),E		; Undocumented
	RES	7,(IY),H		; Undocumented
	RES	7,(IY),L		; Undocumented
	RES	7,(IY+IND)
	RES	7,(IY+IND),A		; Undocumented
	RES	7,(IY+IND),B		; Undocumented
	RES	7,(IY+IND),C		; Undocumented
	RES	7,(IY+IND),D		; Undocumented
	RES	7,(IY+IND),E		; Undocumented
	RES	7,(IY+IND),H		; Undocumented
	RES	7,(IY+IND),L		; Undocumented
	RES	7,(IY-IND)
	RES	7,(IY-IND),A		; Undocumented
	RES	7,(IY-IND),B		; Undocumented
	RES	7,(IY-IND),C		; Undocumented
	RES	7,(IY-IND),D		; Undocumented
	RES	7,(IY-IND),E		; Undocumented
	RES	7,(IY-IND),H		; Undocumented
	RES	7,(IY-IND),L		; Undocumented
	RES	7,A
	RES	7,B
	RES	7,C
	RES	7,D
	RES	7,E
	RES	7,H
	RES	7,L
	RET
	RET	C
	RET	M
	RET	NC
	RET	NZ
	RET	P
	RET	PE
	RET	PO
	RET	Z
	RETI
	RETN
	RL	(HL)
	RL	(IX)
	RL	(IX),A			; Undocumented
	RL	(IX),B			; Undocumented
	RL	(IX),C			; Undocumented
	RL	(IX),D			; Undocumented
	RL	(IX),E			; Undocumented
	RL	(IX),H			; Undocumented
	RL	(IX),L			; Undocumented
	RL	(IX+IND)
	RL	(IX+IND),A		; Undocumented
	RL	(IX+IND),B		; Undocumented
	RL	(IX+IND),C		; Undocumented
	RL	(IX+IND),D		; Undocumented
	RL	(IX+IND),E		; Undocumented
	RL	(IX+IND),H		; Undocumented
	RL	(IX+IND),L		; Undocumented
	RL	(IX-IND)
	RL	(IX-IND),A		; Undocumented
	RL	(IX-IND),B		; Undocumented
	RL	(IX-IND),C		; Undocumented
	RL	(IX-IND),D		; Undocumented
	RL	(IX-IND),E		; Undocumented
	RL	(IX-IND),H		; Undocumented
	RL	(IX-IND),L		; Undocumented
	RL	(IY)
	RL	(IY),A			; Undocumented
	RL	(IY),B			; Undocumented
	RL	(IY),C			; Undocumented
	RL	(IY),D			; Undocumented
	RL	(IY),E			; Undocumented
	RL	(IY),H			; Undocumented
	RL	(IY),L			; Undocumented
	RL	(IY+IND)
	RL	(IY+IND),A		; Undocumented
	RL	(IY+IND),B		; Undocumented
	RL	(IY+IND),C		; Undocumented
	RL	(IY+IND),D		; Undocumented
	RL	(IY+IND),E		; Undocumented
	RL	(IY+IND),H		; Undocumented
	RL	(IY+IND),L		; Undocumented
	RL	(IY-IND)
	RL	(IY-IND),A		; Undocumented
	RL	(IY-IND),B		; Undocumented
	RL	(IY-IND),C		; Undocumented
	RL	(IY-IND),D		; Undocumented
	RL	(IY-IND),E		; Undocumented
	RL	(IY-IND),H		; Undocumented
	RL	(IY-IND),L		; Undocumented
	RL	A
	RL	B
	RL	C
	RL	D
	RL	E
	RL	H
	RL	L
	RLA
	RLC	(HL)
	RLC	(IX)
	RLC	(IX),A			; Undocumented
	RLC	(IX),B			; Undocumented
	RLC	(IX),C			; Undocumented
	RLC	(IX),D			; Undocumented
	RLC	(IX),E			; Undocumented
	RLC	(IX),H			; Undocumented
	RLC	(IX),L			; Undocumented
	RLC	(IX+IND)
	RLC	(IX+IND),A		; Undocumented
	RLC	(IX+IND),B		; Undocumented
	RLC	(IX+IND),C		; Undocumented
	RLC	(IX+IND),D		; Undocumented
	RLC	(IX+IND),E		; Undocumented
	RLC	(IX+IND),H		; Undocumented
	RLC	(IX+IND),L		; Undocumented
	RLC	(IX-IND)
	RLC	(IX-IND),A		; Undocumented
	RLC	(IX-IND),B		; Undocumented
	RLC	(IX-IND),C		; Undocumented
	RLC	(IX-IND),D		; Undocumented
	RLC	(IX-IND),E		; Undocumented
	RLC	(IX-IND),H		; Undocumented
	RLC	(IX-IND),L		; Undocumented
	RLC	(IY)
	RLC	(IY),A			; Undocumented
	RLC	(IY),B			; Undocumented
	RLC	(IY),C			; Undocumented
	RLC	(IY),D			; Undocumented
	RLC	(IY),E			; Undocumented
	RLC	(IY),H			; Undocumented
	RLC	(IY),L			; Undocumented
	RLC	(IY+IND)
	RLC	(IY+IND),A		; Undocumented
	RLC	(IY+IND),B		; Undocumented
	RLC	(IY+IND),C		; Undocumented
	RLC	(IY+IND),D		; Undocumented
	RLC	(IY+IND),E		; Undocumented
	RLC	(IY+IND),H		; Undocumented
	RLC	(IY+IND),L		; Undocumented
	RLC	(IY-IND)
	RLC	(IY-IND),A		; Undocumented
	RLC	(IY-IND),B		; Undocumented
	RLC	(IY-IND),C		; Undocumented
	RLC	(IY-IND),D		; Undocumented
	RLC	(IY-IND),E		; Undocumented
	RLC	(IY-IND),H		; Undocumented
	RLC	(IY-IND),L		; Undocumented
	RLC	A
	RLC	B
	RLC	C
	RLC	D
	RLC	E
	RLC	H
	RLC	L
	RLCA
	RLD
	RR	(HL)
	RR	(IX)
	RR	(IX),A			; Undocumented
	RR	(IX),B			; Undocumented
	RR	(IX),C			; Undocumented
	RR	(IX),D			; Undocumented
	RR	(IX),E			; Undocumented
	RR	(IX),H			; Undocumented
	RR	(IX),L			; Undocumented
	RR	(IX+IND)
	RR	(IX+IND),A		; Undocumented
	RR	(IX+IND),B		; Undocumented
	RR	(IX+IND),C		; Undocumented
	RR	(IX+IND),D		; Undocumented
	RR	(IX+IND),E		; Undocumented
	RR	(IX+IND),H		; Undocumented
	RR	(IX+IND),L		; Undocumented
	RR	(IX-IND)
	RR	(IX-IND),A		; Undocumented
	RR	(IX-IND),B		; Undocumented
	RR	(IX-IND),C		; Undocumented
	RR	(IX-IND),D		; Undocumented
	RR	(IX-IND),E		; Undocumented
	RR	(IX-IND),H		; Undocumented
	RR	(IX-IND),L		; Undocumented
	RR	(IY)
	RR	(IY),A			; Undocumented
	RR	(IY),B			; Undocumented
	RR	(IY),C			; Undocumented
	RR	(IY),D			; Undocumented
	RR	(IY),E			; Undocumented
	RR	(IY),H			; Undocumented
	RR	(IY),L			; Undocumented
	RR	(IY+IND)
	RR	(IY+IND),A		; Undocumented
	RR	(IY+IND),B		; Undocumented
	RR	(IY+IND),C		; Undocumented
	RR	(IY+IND),D		; Undocumented
	RR	(IY+IND),E		; Undocumented
	RR	(IY+IND),H		; Undocumented
	RR	(IY+IND),L		; Undocumented
	RR	(IY-IND)
	RR	(IY-IND),A		; Undocumented
	RR	(IY-IND),B		; Undocumented
	RR	(IY-IND),C		; Undocumented
	RR	(IY-IND),D		; Undocumented
	RR	(IY-IND),E		; Undocumented
	RR	(IY-IND),H		; Undocumented
	RR	(IY-IND),L		; Undocumented
	RR	A
	RR	B
	RR	C
	RR	D
	RR	E
	RR	H
	RR	L
	RRA
	RRC	(HL)
	RRC	(IX)
	RRC	(IX),A			; Undocumented
	RRC	(IX),B			; Undocumented
	RRC	(IX),C			; Undocumented
	RRC	(IX),D			; Undocumented
	RRC	(IX),E			; Undocumented
	RRC	(IX),H			; Undocumented
	RRC	(IX),L			; Undocumented
	RRC	(IX+IND)
	RRC	(IX+IND),A		; Undocumented
	RRC	(IX+IND),B		; Undocumented
	RRC	(IX+IND),C		; Undocumented
	RRC	(IX+IND),D		; Undocumented
	RRC	(IX+IND),E		; Undocumented
	RRC	(IX+IND),H		; Undocumented
	RRC	(IX+IND),L		; Undocumented
	RRC	(IX-IND)
	RRC	(IX-IND),A		; Undocumented
	RRC	(IX-IND),B		; Undocumented
	RRC	(IX-IND),C		; Undocumented
	RRC	(IX-IND),D		; Undocumented
	RRC	(IX-IND),E		; Undocumented
	RRC	(IX-IND),H		; Undocumented
	RRC	(IX-IND),L		; Undocumented
	RRC	(IY)
	RRC	(IY),A			; Undocumented
	RRC	(IY),B			; Undocumented
	RRC	(IY),C			; Undocumented
	RRC	(IY),D			; Undocumented
	RRC	(IY),E			; Undocumented
	RRC	(IY),H			; Undocumented
	RRC	(IY),L			; Undocumented
	RRC	(IY+IND)
	RRC	(IY+IND),A		; Undocumented
	RRC	(IY+IND),B		; Undocumented
	RRC	(IY+IND),C		; Undocumented
	RRC	(IY+IND),D		; Undocumented
	RRC	(IY+IND),E		; Undocumented
	RRC	(IY+IND),H		; Undocumented
	RRC	(IY+IND),L		; Undocumented
	RRC	(IY-IND)
	RRC	(IY-IND),A		; Undocumented
	RRC	(IY-IND),B		; Undocumented
	RRC	(IY-IND),C		; Undocumented
	RRC	(IY-IND),D		; Undocumented
	RRC	(IY-IND),E		; Undocumented
	RRC	(IY-IND),H		; Undocumented
	RRC	(IY-IND),L		; Undocumented
	RRC	A
	RRC	B
	RRC	C
	RRC	D
	RRC	E
	RRC	H
	RRC	L
	RRCA
	RRD
	RST	00H
	RST	08H
	RST	10H
	RST	18H
	RST	20H
	RST	28H
	RST	30H
	RST	38H
	SBC	A,(HL)
	SBC	A,(IX)
	SBC	A,(IX+IND)
	SBC	A,(IX-IND)
	SBC	A,(IY)
	SBC	A,(IY+IND)
	SBC	A,(IY-IND)
	SBC	A,A
	SBC	A,B
	SBC	A,C
	SBC	A,D
	SBC	A,E
	SBC	A,H
	SBC	A,IXH			; Undocumented
	SBC	A,IXL			; Undocumented
	SBC	A,IYH			; Undocumented
	SBC	A,IYL			; Undocumented
	SBC	A,L
	SBC	A,N
	SBC	HL,BC
	SBC	HL,DE
	SBC	HL,HL
	SBC	HL,SP
	SCF
	SET	0,(HL)
	SET	0,(IX)
	SET	0,(IX),A		; Undocumented
	SET	0,(IX),B		; Undocumented
	SET	0,(IX),C		; Undocumented
	SET	0,(IX),D		; Undocumented
	SET	0,(IX),E		; Undocumented
	SET	0,(IX),H		; Undocumented
	SET	0,(IX),L		; Undocumented
	SET	0,(IX+IND)
	SET	0,(IX+IND),A		; Undocumented
	SET	0,(IX+IND),B		; Undocumented
	SET	0,(IX+IND),C		; Undocumented
	SET	0,(IX+IND),D		; Undocumented
	SET	0,(IX+IND),E		; Undocumented
	SET	0,(IX+IND),H		; Undocumented
	SET	0,(IX+IND),L		; Undocumented
	SET	0,(IX-IND)
	SET	0,(IX-IND),A		; Undocumented
	SET	0,(IX-IND),B		; Undocumented
	SET	0,(IX-IND),C		; Undocumented
	SET	0,(IX-IND),D		; Undocumented
	SET	0,(IX-IND),E		; Undocumented
	SET	0,(IX-IND),H		; Undocumented
	SET	0,(IX-IND),L		; Undocumented
	SET	0,(IY)
	SET	0,(IY),A		; Undocumented
	SET	0,(IY),B		; Undocumented
	SET	0,(IY),C		; Undocumented
	SET	0,(IY),D		; Undocumented
	SET	0,(IY),E		; Undocumented
	SET	0,(IY),H		; Undocumented
	SET	0,(IY),L		; Undocumented
	SET	0,(IY+IND)
	SET	0,(IY+IND),A		; Undocumented
	SET	0,(IY+IND),B		; Undocumented
	SET	0,(IY+IND),C		; Undocumented
	SET	0,(IY+IND),D		; Undocumented
	SET	0,(IY+IND),E		; Undocumented
	SET	0,(IY+IND),H		; Undocumented
	SET	0,(IY+IND),L		; Undocumented
	SET	0,(IY-IND)
	SET	0,(IY-IND),A		; Undocumented
	SET	0,(IY-IND),B		; Undocumented
	SET	0,(IY-IND),C		; Undocumented
	SET	0,(IY-IND),D		; Undocumented
	SET	0,(IY-IND),E		; Undocumented
	SET	0,(IY-IND),H		; Undocumented
	SET	0,(IY-IND),L		; Undocumented
	SET	0,A
	SET	0,B
	SET	0,C
	SET	0,D
	SET	0,E
	SET	0,H
	SET	0,L
	SET	1,(HL)
	SET	1,(IX)
	SET	1,(IX),A		; Undocumented
	SET	1,(IX),B		; Undocumented
	SET	1,(IX),C		; Undocumented
	SET	1,(IX),D		; Undocumented
	SET	1,(IX),E		; Undocumented
	SET	1,(IX),H		; Undocumented
	SET	1,(IX),L		; Undocumented
	SET	1,(IX+IND)
	SET	1,(IX+IND),A		; Undocumented
	SET	1,(IX+IND),B		; Undocumented
	SET	1,(IX+IND),C		; Undocumented
	SET	1,(IX+IND),D		; Undocumented
	SET	1,(IX+IND),E		; Undocumented
	SET	1,(IX+IND),H		; Undocumented
	SET	1,(IX+IND),L		; Undocumented
	SET	1,(IX-IND)
	SET	1,(IX-IND),A		; Undocumented
	SET	1,(IX-IND),B		; Undocumented
	SET	1,(IX-IND),C		; Undocumented
	SET	1,(IX-IND),D		; Undocumented
	SET	1,(IX-IND),E		; Undocumented
	SET	1,(IX-IND),H		; Undocumented
	SET	1,(IX-IND),L		; Undocumented
	SET	1,(IY)
	SET	1,(IY),A		; Undocumented
	SET	1,(IY),B		; Undocumented
	SET	1,(IY),C		; Undocumented
	SET	1,(IY),D		; Undocumented
	SET	1,(IY),E		; Undocumented
	SET	1,(IY),H		; Undocumented
	SET	1,(IY),L		; Undocumented
	SET	1,(IY+IND)
	SET	1,(IY+IND),A		; Undocumented
	SET	1,(IY+IND),B		; Undocumented
	SET	1,(IY+IND),C		; Undocumented
	SET	1,(IY+IND),D		; Undocumented
	SET	1,(IY+IND),E		; Undocumented
	SET	1,(IY+IND),H		; Undocumented
	SET	1,(IY+IND),L		; Undocumented
	SET	1,(IY-IND)
	SET	1,(IY-IND),A		; Undocumented
	SET	1,(IY-IND),B		; Undocumented
	SET	1,(IY-IND),C		; Undocumented
	SET	1,(IY-IND),D		; Undocumented
	SET	1,(IY-IND),E		; Undocumented
	SET	1,(IY-IND),H		; Undocumented
	SET	1,(IY-IND),L		; Undocumented
	SET	1,A
	SET	1,B
	SET	1,C
	SET	1,D
	SET	1,E
	SET	1,H
	SET	1,L
	SET	2,(HL)
	SET	2,(IX)
	SET	2,(IX),A		; Undocumented
	SET	2,(IX),B		; Undocumented
	SET	2,(IX),C		; Undocumented
	SET	2,(IX),D		; Undocumented
	SET	2,(IX),E		; Undocumented
	SET	2,(IX),H		; Undocumented
	SET	2,(IX),L		; Undocumented
	SET	2,(IX+IND)
	SET	2,(IX+IND),A		; Undocumented
	SET	2,(IX+IND),B		; Undocumented
	SET	2,(IX+IND),C		; Undocumented
	SET	2,(IX+IND),D		; Undocumented
	SET	2,(IX+IND),E		; Undocumented
	SET	2,(IX+IND),H		; Undocumented
	SET	2,(IX+IND),L		; Undocumented
	SET	2,(IX-IND)
	SET	2,(IX-IND),A		; Undocumented
	SET	2,(IX-IND),B		; Undocumented
	SET	2,(IX-IND),C		; Undocumented
	SET	2,(IX-IND),D		; Undocumented
	SET	2,(IX-IND),E		; Undocumented
	SET	2,(IX-IND),H		; Undocumented
	SET	2,(IX-IND),L		; Undocumented
	SET	2,(IY)
	SET	2,(IY),A		; Undocumented
	SET	2,(IY),B		; Undocumented
	SET	2,(IY),C		; Undocumented
	SET	2,(IY),D		; Undocumented
	SET	2,(IY),E		; Undocumented
	SET	2,(IY),H		; Undocumented
	SET	2,(IY),L		; Undocumented
	SET	2,(IY+IND)
	SET	2,(IY+IND),A		; Undocumented
	SET	2,(IY+IND),B		; Undocumented
	SET	2,(IY+IND),C		; Undocumented
	SET	2,(IY+IND),D		; Undocumented
	SET	2,(IY+IND),E		; Undocumented
	SET	2,(IY+IND),H		; Undocumented
	SET	2,(IY+IND),L		; Undocumented
	SET	2,(IY-IND)
	SET	2,(IY-IND),A		; Undocumented
	SET	2,(IY-IND),B		; Undocumented
	SET	2,(IY-IND),C		; Undocumented
	SET	2,(IY-IND),D		; Undocumented
	SET	2,(IY-IND),E		; Undocumented
	SET	2,(IY-IND),H		; Undocumented
	SET	2,(IY-IND),L		; Undocumented
	SET	2,A
	SET	2,B
	SET	2,C
	SET	2,D
	SET	2,E
	SET	2,H
	SET	2,L
	SET	3,(HL)
	SET	3,(IX)
	SET	3,(IX),A		; Undocumented
	SET	3,(IX),B		; Undocumented
	SET	3,(IX),C		; Undocumented
	SET	3,(IX),D		; Undocumented
	SET	3,(IX),E		; Undocumented
	SET	3,(IX),H		; Undocumented
	SET	3,(IX),L		; Undocumented
	SET	3,(IX+IND)
	SET	3,(IX+IND),A		; Undocumented
	SET	3,(IX+IND),B		; Undocumented
	SET	3,(IX+IND),C		; Undocumented
	SET	3,(IX+IND),D		; Undocumented
	SET	3,(IX+IND),E		; Undocumented
	SET	3,(IX+IND),H		; Undocumented
	SET	3,(IX+IND),L		; Undocumented
	SET	3,(IX-IND)
	SET	3,(IX-IND),A		; Undocumented
	SET	3,(IX-IND),B		; Undocumented
	SET	3,(IX-IND),C		; Undocumented
	SET	3,(IX-IND),D		; Undocumented
	SET	3,(IX-IND),E		; Undocumented
	SET	3,(IX-IND),H		; Undocumented
	SET	3,(IX-IND),L		; Undocumented
	SET	3,(IY)
	SET	3,(IY),A		; Undocumented
	SET	3,(IY),B		; Undocumented
	SET	3,(IY),C		; Undocumented
	SET	3,(IY),D		; Undocumented
	SET	3,(IY),E		; Undocumented
	SET	3,(IY),H		; Undocumented
	SET	3,(IY),L		; Undocumented
	SET	3,(IY+IND)
	SET	3,(IY+IND),A		; Undocumented
	SET	3,(IY+IND),B		; Undocumented
	SET	3,(IY+IND),C		; Undocumented
	SET	3,(IY+IND),D		; Undocumented
	SET	3,(IY+IND),E		; Undocumented
	SET	3,(IY+IND),H		; Undocumented
	SET	3,(IY+IND),L		; Undocumented
	SET	3,(IY-IND)
	SET	3,(IY-IND),A		; Undocumented
	SET	3,(IY-IND),B		; Undocumented
	SET	3,(IY-IND),C		; Undocumented
	SET	3,(IY-IND),D		; Undocumented
	SET	3,(IY-IND),E		; Undocumented
	SET	3,(IY-IND),H		; Undocumented
	SET	3,(IY-IND),L		; Undocumented
	SET	3,A
	SET	3,B
	SET	3,C
	SET	3,D
	SET	3,E
	SET	3,H
	SET	3,L
	SET	4,(HL)
	SET	4,(IX)
	SET	4,(IX),A		; Undocumented
	SET	4,(IX),B		; Undocumented
	SET	4,(IX),C		; Undocumented
	SET	4,(IX),D		; Undocumented
	SET	4,(IX),E		; Undocumented
	SET	4,(IX),H		; Undocumented
	SET	4,(IX),L		; Undocumented
	SET	4,(IX+IND)
	SET	4,(IX+IND),A		; Undocumented
	SET	4,(IX+IND),B		; Undocumented
	SET	4,(IX+IND),C		; Undocumented
	SET	4,(IX+IND),D		; Undocumented
	SET	4,(IX+IND),E		; Undocumented
	SET	4,(IX+IND),H		; Undocumented
	SET	4,(IX+IND),L		; Undocumented
	SET	4,(IX-IND)
	SET	4,(IX-IND),A		; Undocumented
	SET	4,(IX-IND),B		; Undocumented
	SET	4,(IX-IND),C		; Undocumented
	SET	4,(IX-IND),D		; Undocumented
	SET	4,(IX-IND),E		; Undocumented
	SET	4,(IX-IND),H		; Undocumented
	SET	4,(IX-IND),L		; Undocumented
	SET	4,(IY)
	SET	4,(IY),A		; Undocumented
	SET	4,(IY),B		; Undocumented
	SET	4,(IY),C		; Undocumented
	SET	4,(IY),D		; Undocumented
	SET	4,(IY),E		; Undocumented
	SET	4,(IY),H		; Undocumented
	SET	4,(IY),L		; Undocumented
	SET	4,(IY+IND)
	SET	4,(IY+IND),A		; Undocumented
	SET	4,(IY+IND),B		; Undocumented
	SET	4,(IY+IND),C		; Undocumented
	SET	4,(IY+IND),D		; Undocumented
	SET	4,(IY+IND),E		; Undocumented
	SET	4,(IY+IND),H		; Undocumented
	SET	4,(IY+IND),L		; Undocumented
	SET	4,(IY-IND)
	SET	4,(IY-IND),A		; Undocumented
	SET	4,(IY-IND),B		; Undocumented
	SET	4,(IY-IND),C		; Undocumented
	SET	4,(IY-IND),D		; Undocumented
	SET	4,(IY-IND),E		; Undocumented
	SET	4,(IY-IND),H		; Undocumented
	SET	4,(IY-IND),L		; Undocumented
	SET	4,A
	SET	4,B
	SET	4,C
	SET	4,D
	SET	4,E
	SET	4,H
	SET	4,L
	SET	5,(HL)
	SET	5,(IX)
	SET	5,(IX),A		; Undocumented
	SET	5,(IX),B		; Undocumented
	SET	5,(IX),C		; Undocumented
	SET	5,(IX),D		; Undocumented
	SET	5,(IX),E		; Undocumented
	SET	5,(IX),H		; Undocumented
	SET	5,(IX),L		; Undocumented
	SET	5,(IX+IND)
	SET	5,(IX+IND),A		; Undocumented
	SET	5,(IX+IND),B		; Undocumented
	SET	5,(IX+IND),C		; Undocumented
	SET	5,(IX+IND),D		; Undocumented
	SET	5,(IX+IND),E		; Undocumented
	SET	5,(IX+IND),H		; Undocumented
	SET	5,(IX+IND),L		; Undocumented
	SET	5,(IX-IND)
	SET	5,(IX-IND),A		; Undocumented
	SET	5,(IX-IND),B		; Undocumented
	SET	5,(IX-IND),C		; Undocumented
	SET	5,(IX-IND),D		; Undocumented
	SET	5,(IX-IND),E		; Undocumented
	SET	5,(IX-IND),H		; Undocumented
	SET	5,(IX-IND),L		; Undocumented
	SET	5,(IY)
	SET	5,(IY),A		; Undocumented
	SET	5,(IY),B		; Undocumented
	SET	5,(IY),C		; Undocumented
	SET	5,(IY),D		; Undocumented
	SET	5,(IY),E		; Undocumented
	SET	5,(IY),H		; Undocumented
	SET	5,(IY),L		; Undocumented
	SET	5,(IY+IND)
	SET	5,(IY+IND),A		; Undocumented
	SET	5,(IY+IND),B		; Undocumented
	SET	5,(IY+IND),C		; Undocumented
	SET	5,(IY+IND),D		; Undocumented
	SET	5,(IY+IND),E		; Undocumented
	SET	5,(IY+IND),H		; Undocumented
	SET	5,(IY+IND),L		; Undocumented
	SET	5,(IY-IND)
	SET	5,(IY-IND),A		; Undocumented
	SET	5,(IY-IND),B		; Undocumented
	SET	5,(IY-IND),C		; Undocumented
	SET	5,(IY-IND),D		; Undocumented
	SET	5,(IY-IND),E		; Undocumented
	SET	5,(IY-IND),H		; Undocumented
	SET	5,(IY-IND),L		; Undocumented
	SET	5,A
	SET	5,B
	SET	5,C
	SET	5,D
	SET	5,E
	SET	5,H
	SET	5,L
	SET	6,(HL)
	SET	6,(IX)
	SET	6,(IX),A		; Undocumented
	SET	6,(IX),B		; Undocumented
	SET	6,(IX),C		; Undocumented
	SET	6,(IX),D		; Undocumented
	SET	6,(IX),E		; Undocumented
	SET	6,(IX),H		; Undocumented
	SET	6,(IX),L		; Undocumented
	SET	6,(IX+IND)
	SET	6,(IX+IND),A		; Undocumented
	SET	6,(IX+IND),B		; Undocumented
	SET	6,(IX+IND),C		; Undocumented
	SET	6,(IX+IND),D		; Undocumented
	SET	6,(IX+IND),E		; Undocumented
	SET	6,(IX+IND),H		; Undocumented
	SET	6,(IX+IND),L		; Undocumented
	SET	6,(IX-IND)
	SET	6,(IX-IND),A		; Undocumented
	SET	6,(IX-IND),B		; Undocumented
	SET	6,(IX-IND),C		; Undocumented
	SET	6,(IX-IND),D		; Undocumented
	SET	6,(IX-IND),E		; Undocumented
	SET	6,(IX-IND),H		; Undocumented
	SET	6,(IX-IND),L		; Undocumented
	SET	6,(IY)
	SET	6,(IY),A		; Undocumented
	SET	6,(IY),B		; Undocumented
	SET	6,(IY),C		; Undocumented
	SET	6,(IY),D		; Undocumented
	SET	6,(IY),E		; Undocumented
	SET	6,(IY),H		; Undocumented
	SET	6,(IY),L		; Undocumented
	SET	6,(IY+IND)
	SET	6,(IY+IND),A		; Undocumented
	SET	6,(IY+IND),B		; Undocumented
	SET	6,(IY+IND),C		; Undocumented
	SET	6,(IY+IND),D		; Undocumented
	SET	6,(IY+IND),E		; Undocumented
	SET	6,(IY+IND),H		; Undocumented
	SET	6,(IY+IND),L		; Undocumented
	SET	6,(IY-IND)
	SET	6,(IY-IND),A		; Undocumented
	SET	6,(IY-IND),B		; Undocumented
	SET	6,(IY-IND),C		; Undocumented
	SET	6,(IY-IND),D		; Undocumented
	SET	6,(IY-IND),E		; Undocumented
	SET	6,(IY-IND),H		; Undocumented
	SET	6,(IY-IND),L		; Undocumented
	SET	6,A
	SET	6,B
	SET	6,C
	SET	6,D
	SET	6,E
	SET	6,H
	SET	6,L
	SET	7,(HL)
	SET	7,(IX)
	SET	7,(IX),A		; Undocumented
	SET	7,(IX),B		; Undocumented
	SET	7,(IX),C		; Undocumented
	SET	7,(IX),D		; Undocumented
	SET	7,(IX),E		; Undocumented
	SET	7,(IX),H		; Undocumented
	SET	7,(IX),L		; Undocumented
	SET	7,(IX+IND)
	SET	7,(IX+IND),A		; Undocumented
	SET	7,(IX+IND),B		; Undocumented
	SET	7,(IX+IND),C		; Undocumented
	SET	7,(IX+IND),D		; Undocumented
	SET	7,(IX+IND),E		; Undocumented
	SET	7,(IX+IND),H		; Undocumented
	SET	7,(IX+IND),L		; Undocumented
	SET	7,(IX-IND)
	SET	7,(IX-IND),A		; Undocumented
	SET	7,(IX-IND),B		; Undocumented
	SET	7,(IX-IND),C		; Undocumented
	SET	7,(IX-IND),D		; Undocumented
	SET	7,(IX-IND),E		; Undocumented
	SET	7,(IX-IND),H		; Undocumented
	SET	7,(IX-IND),L		; Undocumented
	SET	7,(IY)
	SET	7,(IY),A		; Undocumented
	SET	7,(IY),B		; Undocumented
	SET	7,(IY),C		; Undocumented
	SET	7,(IY),D		; Undocumented
	SET	7,(IY),E		; Undocumented
	SET	7,(IY),H		; Undocumented
	SET	7,(IY),L		; Undocumented
	SET	7,(IY+IND)
	SET	7,(IY+IND),A		; Undocumented
	SET	7,(IY+IND),B		; Undocumented
	SET	7,(IY+IND),C		; Undocumented
	SET	7,(IY+IND),D		; Undocumented
	SET	7,(IY+IND),E		; Undocumented
	SET	7,(IY+IND),H		; Undocumented
	SET	7,(IY+IND),L		; Undocumented
	SET	7,(IY-IND)
	SET	7,(IY-IND),A		; Undocumented
	SET	7,(IY-IND),B		; Undocumented
	SET	7,(IY-IND),C		; Undocumented
	SET	7,(IY-IND),D		; Undocumented
	SET	7,(IY-IND),E		; Undocumented
	SET	7,(IY-IND),H		; Undocumented
	SET	7,(IY-IND),L		; Undocumented
	SET	7,A
	SET	7,B
	SET	7,C
	SET	7,D
	SET	7,E
	SET	7,H
	SET	7,L
	SLA	(HL)
	SLA	(IX)
	SLA	(IX),A			; Undocumented
	SLA	(IX),B			; Undocumented
	SLA	(IX),C			; Undocumented
	SLA	(IX),D			; Undocumented
	SLA	(IX),E			; Undocumented
	SLA	(IX),H			; Undocumented
	SLA	(IX),L			; Undocumented
	SLA	(IX+IND)
	SLA	(IX+IND),A		; Undocumented
	SLA	(IX+IND),B		; Undocumented
	SLA	(IX+IND),C		; Undocumented
	SLA	(IX+IND),D		; Undocumented
	SLA	(IX+IND),E		; Undocumented
	SLA	(IX+IND),H		; Undocumented
	SLA	(IX+IND),L		; Undocumented
	SLA	(IX-IND)
	SLA	(IX-IND),A		; Undocumented
	SLA	(IX-IND),B		; Undocumented
	SLA	(IX-IND),C		; Undocumented
	SLA	(IX-IND),D		; Undocumented
	SLA	(IX-IND),E		; Undocumented
	SLA	(IX-IND),H		; Undocumented
	SLA	(IX-IND),L		; Undocumented
	SLA	(IY)
	SLA	(IY),A			; Undocumented
	SLA	(IY),B			; Undocumented
	SLA	(IY),C			; Undocumented
	SLA	(IY),D			; Undocumented
	SLA	(IY),E			; Undocumented
	SLA	(IY),H			; Undocumented
	SLA	(IY),L			; Undocumented
	SLA	(IY+IND)
	SLA	(IY+IND),A		; Undocumented
	SLA	(IY+IND),B		; Undocumented
	SLA	(IY+IND),C		; Undocumented
	SLA	(IY+IND),D		; Undocumented
	SLA	(IY+IND),E		; Undocumented
	SLA	(IY+IND),H		; Undocumented
	SLA	(IY+IND),L		; Undocumented
	SLA	(IY-IND)
	SLA	(IY-IND),A		; Undocumented
	SLA	(IY-IND),B		; Undocumented
	SLA	(IY-IND),C		; Undocumented
	SLA	(IY-IND),D		; Undocumented
	SLA	(IY-IND),E		; Undocumented
	SLA	(IY-IND),H		; Undocumented
	SLA	(IY-IND),L		; Undocumented
	SLA	A
	SLA	B
	SLA	C
	SLA	D
	SLA	E
	SLA	H
	SLA	L
	SLL	(HL)			; Undocumented
	SLL	(IX)			; Undocumented
	SLL	(IX),A			; Undocumented
	SLL	(IX),B			; Undocumented
	SLL	(IX),C			; Undocumented
	SLL	(IX),D			; Undocumented
	SLL	(IX),E			; Undocumented
	SLL	(IX),H			; Undocumented
	SLL	(IX),L			; Undocumented
	SLL	(IX+IND)		; Undocumented
	SLL	(IX+IND),A		; Undocumented
	SLL	(IX+IND),B		; Undocumented
	SLL	(IX+IND),C		; Undocumented
	SLL	(IX+IND),D		; Undocumented
	SLL	(IX+IND),E		; Undocumented
	SLL	(IX+IND),H		; Undocumented
	SLL	(IX+IND),L		; Undocumented
	SLL	(IX-IND)		; Undocumented
	SLL	(IX-IND),A		; Undocumented
	SLL	(IX-IND),B		; Undocumented
	SLL	(IX-IND),C		; Undocumented
	SLL	(IX-IND),D		; Undocumented
	SLL	(IX-IND),E		; Undocumented
	SLL	(IX-IND),H		; Undocumented
	SLL	(IX-IND),L		; Undocumented
	SLL	(IY)			; Undocumented
	SLL	(IY),A			; Undocumented
	SLL	(IY),B			; Undocumented
	SLL	(IY),C			; Undocumented
	SLL	(IY),D			; Undocumented
	SLL	(IY),E			; Undocumented
	SLL	(IY),H			; Undocumented
	SLL	(IY),L			; Undocumented
	SLL	(IY+IND)		; Undocumented
	SLL	(IY+IND),A		; Undocumented
	SLL	(IY+IND),B		; Undocumented
	SLL	(IY+IND),C		; Undocumented
	SLL	(IY+IND),D		; Undocumented
	SLL	(IY+IND),E		; Undocumented
	SLL	(IY+IND),H		; Undocumented
	SLL	(IY+IND),L		; Undocumented
	SLL	(IY-IND)		; Undocumented
	SLL	(IY-IND),A		; Undocumented
	SLL	(IY-IND),B		; Undocumented
	SLL	(IY-IND),C		; Undocumented
	SLL	(IY-IND),D		; Undocumented
	SLL	(IY-IND),E		; Undocumented
	SLL	(IY-IND),H		; Undocumented
	SLL	(IY-IND),L		; Undocumented
	SLL	A			; Undocumented
	SLL	B			; Undocumented
	SLL	C			; Undocumented
	SLL	D			; Undocumented
	SLL	E			; Undocumented
	SLL	H			; Undocumented
	SLL	L			; Undocumented
	SRA	(HL)
	SRA	(IX)
	SRA	(IX),A			; Undocumented
	SRA	(IX),B			; Undocumented
	SRA	(IX),C			; Undocumented
	SRA	(IX),D			; Undocumented
	SRA	(IX),E			; Undocumented
	SRA	(IX),H			; Undocumented
	SRA	(IX),L			; Undocumented
	SRA	(IX+IND)
	SRA	(IX+IND),A		; Undocumented
	SRA	(IX+IND),B		; Undocumented
	SRA	(IX+IND),C		; Undocumented
	SRA	(IX+IND),D		; Undocumented
	SRA	(IX+IND),E		; Undocumented
	SRA	(IX+IND),H		; Undocumented
	SRA	(IX+IND),L		; Undocumented
	SRA	(IX-IND)
	SRA	(IX-IND),A		; Undocumented
	SRA	(IX-IND),B		; Undocumented
	SRA	(IX-IND),C		; Undocumented
	SRA	(IX-IND),D		; Undocumented
	SRA	(IX-IND),E		; Undocumented
	SRA	(IX-IND),H		; Undocumented
	SRA	(IX-IND),L		; Undocumented
	SRA	(IY)
	SRA	(IY),A			; Undocumented
	SRA	(IY),B			; Undocumented
	SRA	(IY),C			; Undocumented
	SRA	(IY),D			; Undocumented
	SRA	(IY),E			; Undocumented
	SRA	(IY),H			; Undocumented
	SRA	(IY),L			; Undocumented
	SRA	(IY+IND)
	SRA	(IY+IND),A		; Undocumented
	SRA	(IY+IND),B		; Undocumented
	SRA	(IY+IND),C		; Undocumented
	SRA	(IY+IND),D		; Undocumented
	SRA	(IY+IND),E		; Undocumented
	SRA	(IY+IND),H		; Undocumented
	SRA	(IY+IND),L		; Undocumented
	SRA	(IY-IND)
	SRA	(IY-IND),A		; Undocumented
	SRA	(IY-IND),B		; Undocumented
	SRA	(IY-IND),C		; Undocumented
	SRA	(IY-IND),D		; Undocumented
	SRA	(IY-IND),E		; Undocumented
	SRA	(IY-IND),H		; Undocumented
	SRA	(IY-IND),L		; Undocumented
	SRA	A
	SRA	B
	SRA	C
	SRA	D
	SRA	E
	SRA	H
	SRA	L
	SRL	(HL)
	SRL	(IX)
	SRL	(IX),A			; Undocumented
	SRL	(IX),B			; Undocumented
	SRL	(IX),C			; Undocumented
	SRL	(IX),D			; Undocumented
	SRL	(IX),E			; Undocumented
	SRL	(IX),H			; Undocumented
	SRL	(IX),L			; Undocumented
	SRL	(IX+IND)
	SRL	(IX+IND),A		; Undocumented
	SRL	(IX+IND),B		; Undocumented
	SRL	(IX+IND),C		; Undocumented
	SRL	(IX+IND),D		; Undocumented
	SRL	(IX+IND),E		; Undocumented
	SRL	(IX+IND),H		; Undocumented
	SRL	(IX+IND),L		; Undocumented
	SRL	(IX-IND)
	SRL	(IX-IND),A		; Undocumented
	SRL	(IX-IND),B		; Undocumented
	SRL	(IX-IND),C		; Undocumented
	SRL	(IX-IND),D		; Undocumented
	SRL	(IX-IND),E		; Undocumented
	SRL	(IX-IND),H		; Undocumented
	SRL	(IX-IND),L		; Undocumented
	SRL	(IY)
	SRL	(IY),A			; Undocumented
	SRL	(IY),B			; Undocumented
	SRL	(IY),C			; Undocumented
	SRL	(IY),D			; Undocumented
	SRL	(IY),E			; Undocumented
	SRL	(IY),H			; Undocumented
	SRL	(IY),L			; Undocumented
	SRL	(IY+IND)
	SRL	(IY+IND),A		; Undocumented
	SRL	(IY+IND),B		; Undocumented
	SRL	(IY+IND),C		; Undocumented
	SRL	(IY+IND),D		; Undocumented
	SRL	(IY+IND),E		; Undocumented
	SRL	(IY+IND),H		; Undocumented
	SRL	(IY+IND),L		; Undocumented
	SRL	(IY-IND)
	SRL	(IY-IND),A		; Undocumented
	SRL	(IY-IND),B		; Undocumented
	SRL	(IY-IND),C		; Undocumented
	SRL	(IY-IND),D		; Undocumented
	SRL	(IY-IND),E		; Undocumented
	SRL	(IY-IND),H		; Undocumented
	SRL	(IY-IND),L		; Undocumented
	SRL	A
	SRL	B
	SRL	C
	SRL	D
	SRL	E
	SRL	H
	SRL	L
	SUB	(HL)
	SUB	(IX)
	SUB	(IX+IND)
	SUB	(IX-IND)
	SUB	(IY)
	SUB	(IY+IND)
	SUB	(IY-IND)
	SUB	A
	SUB	B
	SUB	C
	SUB	D
	SUB	E
	SUB	H
	SUB	IXH			; Undocumented
	SUB	IXL			; Undocumented
	SUB	IYH			; Undocumented
	SUB	IYL			; Undocumented
	SUB	L
	SUB	N
	XOR	(HL)
	XOR	(IX)
	XOR	(IX+IND)
	XOR	(IX-IND)
	XOR	(IY)
	XOR	(IY+IND)
	XOR	(IY-IND)
	XOR	A
	XOR	B
	XOR	C
	XOR	D
	XOR	E
	XOR	H
	XOR	IXH			; Undocumented
	XOR	IXL			; Undocumented
	XOR	IYH			; Undocumented
	XOR	IYL			; Undocumented
	XOR	L
	XOR	N

; Rest of undocumented op-codes
	DEFB	0EDH,000H		; NOP
	DEFB	0EDH,001H		; NOP
	DEFB	0EDH,002H		; NOP
	DEFB	0EDH,003H		; NOP
	DEFB	0EDH,004H		; NOP
	DEFB	0EDH,005H		; NOP
	DEFB	0EDH,006H		; NOP
	DEFB	0EDH,007H		; NOP
	DEFB	0EDH,008H		; NOP
	DEFB	0EDH,009H		; NOP
	DEFB	0EDH,00AH		; NOP
	DEFB	0EDH,00BH		; NOP
	DEFB	0EDH,00CH		; NOP
	DEFB	0EDH,00DH		; NOP
	DEFB	0EDH,00EH		; NOP
	DEFB	0EDH,00FH		; NOP
	DEFB	0EDH,010H		; NOP
	DEFB	0EDH,011H		; NOP
	DEFB	0EDH,012H		; NOP
	DEFB	0EDH,013H		; NOP
	DEFB	0EDH,014H		; NOP
	DEFB	0EDH,015H		; NOP
	DEFB	0EDH,016H		; NOP
	DEFB	0EDH,017H		; NOP
	DEFB	0EDH,018H		; NOP
	DEFB	0EDH,019H		; NOP
	DEFB	0EDH,01AH		; NOP
	DEFB	0EDH,01BH		; NOP
	DEFB	0EDH,01CH		; NOP
	DEFB	0EDH,01DH		; NOP
	DEFB	0EDH,01EH		; NOP
	DEFB	0EDH,01FH		; NOP
	DEFB	0EDH,020H		; NOP
	DEFB	0EDH,021H		; NOP
	DEFB	0EDH,022H		; NOP
	DEFB	0EDH,023H		; NOP
	DEFB	0EDH,024H		; NOP
	DEFB	0EDH,025H		; NOP
	DEFB	0EDH,026H		; NOP
	DEFB	0EDH,027H		; NOP
	DEFB	0EDH,028H		; NOP
	DEFB	0EDH,029H		; NOP
	DEFB	0EDH,02AH		; NOP
	DEFB	0EDH,02BH		; NOP
	DEFB	0EDH,02CH		; NOP
	DEFB	0EDH,02DH		; NOP
	DEFB	0EDH,02EH		; NOP
	DEFB	0EDH,02FH		; NOP
	DEFB	0EDH,030H		; NOP
	DEFB	0EDH,031H		; NOP
	DEFB	0EDH,032H		; NOP
	DEFB	0EDH,033H		; NOP
	DEFB	0EDH,034H		; NOP
	DEFB	0EDH,035H		; NOP
	DEFB	0EDH,036H		; NOP
	DEFB	0EDH,037H		; NOP
	DEFB	0EDH,038H		; NOP
	DEFB	0EDH,039H		; NOP
	DEFB	0EDH,03AH		; NOP
	DEFB	0EDH,03BH		; NOP
	DEFB	0EDH,03CH		; NOP
	DEFB	0EDH,03DH		; NOP
	DEFB	0EDH,03EH		; NOP
	DEFB	0EDH,03FH		; NOP
	DEFB	0EDH,04CH		; NEG
	DEFB	0EDH,04EH		; IM 0
	DEFB	0EDH,054H		; NEG
	DEFB	0EDH,055H		; RETN
	DEFB	0EDH,05CH		; NEG
	DEFB	0EDH,05DH		; RETI
	DEFB	0EDH,063H		; LD (NN),HL
	DEFW	NN
	DEFB	0EDH,064H		; NEG
	DEFB	0EDH,065H		; RETN
	DEFB	0EDH,066H		; IM 0
	DEFB	0EDH,06BH		; LD HL,(NN)
	DEFW	NN
	DEFB	0EDH,06CH		; NEG
	DEFB	0EDH,06DH		; RETI
	DEFB	0EDH,06EH		; IM 0
	DEFB	0EDH,074H		; NEG
	DEFB	0EDH,075H		; RETN
	DEFB	0EDH,076H		; IM 1
	DEFB	0EDH,077H		; NOP
	DEFB	0EDH,07CH		; NEG
	DEFB	0EDH,07DH		; RETI
	DEFB	0EDH,07EH		; IM 2
	DEFB	0EDH,07FH		; NOP
	DEFB	0EDH,080H		; NOP
	DEFB	0EDH,081H		; NOP
	DEFB	0EDH,082H		; NOP
	DEFB	0EDH,083H		; NOP
	DEFB	0EDH,084H		; NOP
	DEFB	0EDH,085H		; NOP
	DEFB	0EDH,086H		; NOP
	DEFB	0EDH,087H		; NOP
	DEFB	0EDH,088H		; NOP
	DEFB	0EDH,089H		; NOP
	DEFB	0EDH,08AH		; NOP
	DEFB	0EDH,08BH		; NOP
	DEFB	0EDH,08CH		; NOP
	DEFB	0EDH,08DH		; NOP
	DEFB	0EDH,08EH		; NOP
	DEFB	0EDH,08FH		; NOP
	DEFB	0EDH,090H		; NOP
	DEFB	0EDH,091H		; NOP
	DEFB	0EDH,092H		; NOP
	DEFB	0EDH,093H		; NOP
	DEFB	0EDH,094H		; NOP
	DEFB	0EDH,095H		; NOP
	DEFB	0EDH,096H		; NOP
	DEFB	0EDH,097H		; NOP
	DEFB	0EDH,098H		; NOP
	DEFB	0EDH,099H		; NOP
	DEFB	0EDH,09AH		; NOP
	DEFB	0EDH,09BH		; NOP
	DEFB	0EDH,09CH		; NOP
	DEFB	0EDH,09DH		; NOP
	DEFB	0EDH,09EH		; NOP
	DEFB	0EDH,09FH		; NOP
	DEFB	0EDH,0A4H		; NOP
	DEFB	0EDH,0A5H		; NOP
	DEFB	0EDH,0A6H		; NOP
	DEFB	0EDH,0A7H		; NOP
	DEFB	0EDH,0ACH		; NOP
	DEFB	0EDH,0ADH		; NOP
	DEFB	0EDH,0AEH		; NOP
	DEFB	0EDH,0AFH		; NOP
	DEFB	0EDH,0B4H		; NOP
	DEFB	0EDH,0B5H		; NOP
	DEFB	0EDH,0B6H		; NOP
	DEFB	0EDH,0B7H		; NOP
	DEFB	0EDH,0BCH		; NOP
	DEFB	0EDH,0BDH		; NOP
	DEFB	0EDH,0BEH		; NOP
	DEFB	0EDH,0BFH		; NOP
	DEFB	0EDH,0C0H		; NOP
	DEFB	0EDH,0C1H		; NOP
	DEFB	0EDH,0C2H		; NOP
	DEFB	0EDH,0C3H		; NOP
	DEFB	0EDH,0C4H		; NOP
	DEFB	0EDH,0C5H		; NOP
	DEFB	0EDH,0C6H		; NOP
	DEFB	0EDH,0C7H		; NOP
	DEFB	0EDH,0C8H		; NOP
	DEFB	0EDH,0C9H		; NOP
	DEFB	0EDH,0CAH		; NOP
	DEFB	0EDH,0CBH		; NOP
	DEFB	0EDH,0CCH		; NOP
	DEFB	0EDH,0CDH		; NOP
	DEFB	0EDH,0CEH		; NOP
	DEFB	0EDH,0CFH		; NOP
	DEFB	0EDH,0D0H		; NOP
	DEFB	0EDH,0D1H		; NOP
	DEFB	0EDH,0D2H		; NOP
	DEFB	0EDH,0D3H		; NOP
	DEFB	0EDH,0D4H		; NOP
	DEFB	0EDH,0D5H		; NOP
	DEFB	0EDH,0D6H		; NOP
	DEFB	0EDH,0D7H		; NOP
	DEFB	0EDH,0D8H		; NOP
	DEFB	0EDH,0D9H		; NOP
	DEFB	0EDH,0DAH		; NOP
	DEFB	0EDH,0DBH		; NOP
	DEFB	0EDH,0DCH		; NOP
	DEFB	0EDH,0DDH		; NOP
	DEFB	0EDH,0DEH		; NOP
	DEFB	0EDH,0DFH		; NOP
	DEFB	0EDH,0E0H		; NOP
	DEFB	0EDH,0E1H		; NOP
	DEFB	0EDH,0E2H		; NOP
	DEFB	0EDH,0E3H		; NOP
	DEFB	0EDH,0E4H		; NOP
	DEFB	0EDH,0E5H		; NOP
	DEFB	0EDH,0E6H		; NOP
	DEFB	0EDH,0E7H		; NOP
	DEFB	0EDH,0E8H		; NOP
	DEFB	0EDH,0E9H		; NOP
	DEFB	0EDH,0EAH		; NOP
	DEFB	0EDH,0EBH		; NOP
	DEFB	0EDH,0ECH		; NOP
	DEFB	0EDH,0EDH		; NOP
	DEFB	0EDH,0EEH		; NOP
	DEFB	0EDH,0EFH		; NOP
	DEFB	0EDH,0F0H		; NOP
	DEFB	0EDH,0F1H		; NOP
	DEFB	0EDH,0F2H		; NOP
	DEFB	0EDH,0F3H		; NOP
	DEFB	0EDH,0F4H		; NOP
	DEFB	0EDH,0F5H		; NOP
	DEFB	0EDH,0F6H		; NOP
	DEFB	0EDH,0F7H		; NOP
	DEFB	0EDH,0F8H		; NOP
	DEFB	0EDH,0F9H		; NOP
	DEFB	0EDH,0FAH		; NOP
	DEFB	0EDH,0FBH		; NOP
	DEFB	0EDH,0FCH		; NOP
	DEFB	0EDH,0FDH		; NOP
	DEFB	0EDH,0FEH		; NOP
	DEFB	0EDH,0FFH		; NOP
	DEFB	0DDH,0CBH,000H,040H	; BIT 0,(IX)
	DEFB	0DDH,0CBH,IND,040H	; BIT 0,(IX+IND)
	DEFB	0DDH,0CBH,-IND,040H	; BIT 0,(IX-IND)
	DEFB	0DDH,0CBH,000H,041H	; BIT 0,(IX)
	DEFB	0DDH,0CBH,IND,041H	; BIT 0,(IX+IND)
	DEFB	0DDH,0CBH,-IND,041H	; BIT 0,(IX-IND)
	DEFB	0DDH,0CBH,000H,042H	; BIT 0,(IX)
	DEFB	0DDH,0CBH,IND,042H	; BIT 0,(IX+IND)
	DEFB	0DDH,0CBH,-IND,042H	; BIT 0,(IX-IND)
	DEFB	0DDH,0CBH,000H,043H	; BIT 0,(IX)
	DEFB	0DDH,0CBH,IND,043H	; BIT 0,(IX+IND)
	DEFB	0DDH,0CBH,-IND,043H	; BIT 0,(IX-IND)
	DEFB	0DDH,0CBH,000H,044H	; BIT 0,(IX)
	DEFB	0DDH,0CBH,IND,044H	; BIT 0,(IX+IND)
	DEFB	0DDH,0CBH,-IND,044H	; BIT 0,(IX-IND)
	DEFB	0DDH,0CBH,000H,045H	; BIT 0,(IX)
	DEFB	0DDH,0CBH,IND,045H	; BIT 0,(IX+IND)
	DEFB	0DDH,0CBH,-IND,045H	; BIT 0,(IX-IND)
	DEFB	0DDH,0CBH,000H,047H	; BIT 0,(IX)
	DEFB	0DDH,0CBH,IND,047H	; BIT 0,(IX+IND)
	DEFB	0DDH,0CBH,-IND,047H	; BIT 0,(IX-IND)
	DEFB	0DDH,0CBH,000H,048H	; BIT 1,(IX)
	DEFB	0DDH,0CBH,IND,048H	; BIT 1,(IX+IND)
	DEFB	0DDH,0CBH,-IND,048H	; BIT 1,(IX-IND)
	DEFB	0DDH,0CBH,000H,049H	; BIT 1,(IX)
	DEFB	0DDH,0CBH,IND,049H	; BIT 1,(IX+IND)
	DEFB	0DDH,0CBH,-IND,049H	; BIT 1,(IX-IND)
	DEFB	0DDH,0CBH,000H,04AH	; BIT 1,(IX)
	DEFB	0DDH,0CBH,IND,04AH	; BIT 1,(IX+IND)
	DEFB	0DDH,0CBH,-IND,04AH	; BIT 1,(IX-IND)
	DEFB	0DDH,0CBH,000H,04BH	; BIT 1,(IX)
	DEFB	0DDH,0CBH,IND,04BH	; BIT 1,(IX+IND)
	DEFB	0DDH,0CBH,-IND,04BH	; BIT 1,(IX-IND)
	DEFB	0DDH,0CBH,000H,04CH	; BIT 1,(IX)
	DEFB	0DDH,0CBH,IND,04CH	; BIT 1,(IX+IND)
	DEFB	0DDH,0CBH,-IND,04CH	; BIT 1,(IX-IND)
	DEFB	0DDH,0CBH,000H,04DH	; BIT 1,(IX)
	DEFB	0DDH,0CBH,IND,04DH	; BIT 1,(IX+IND)
	DEFB	0DDH,0CBH,-IND,04DH	; BIT 1,(IX-IND)
	DEFB	0DDH,0CBH,000H,04FH	; BIT 1,(IX)
	DEFB	0DDH,0CBH,IND,04FH	; BIT 1,(IX+IND)
	DEFB	0DDH,0CBH,-IND,04FH	; BIT 1,(IX-IND)
	DEFB	0DDH,0CBH,000H,050H	; BIT 2,(IX)
	DEFB	0DDH,0CBH,IND,050H	; BIT 2,(IX+IND)
	DEFB	0DDH,0CBH,-IND,050H	; BIT 2,(IX-IND)
	DEFB	0DDH,0CBH,000H,051H	; BIT 2,(IX)
	DEFB	0DDH,0CBH,IND,051H	; BIT 2,(IX+IND)
	DEFB	0DDH,0CBH,-IND,051H	; BIT 2,(IX-IND)
	DEFB	0DDH,0CBH,000H,052H	; BIT 2,(IX)
	DEFB	0DDH,0CBH,IND,052H	; BIT 2,(IX+IND)
	DEFB	0DDH,0CBH,-IND,052H	; BIT 2,(IX-IND)
	DEFB	0DDH,0CBH,000H,053H	; BIT 2,(IX)
	DEFB	0DDH,0CBH,IND,053H	; BIT 2,(IX+IND)
	DEFB	0DDH,0CBH,-IND,053H	; BIT 2,(IX-IND)
	DEFB	0DDH,0CBH,000H,054H	; BIT 2,(IX)
	DEFB	0DDH,0CBH,IND,054H	; BIT 2,(IX+IND)
	DEFB	0DDH,0CBH,-IND,054H	; BIT 2,(IX-IND)
	DEFB	0DDH,0CBH,000H,055H	; BIT 2,(IX)
	DEFB	0DDH,0CBH,IND,055H	; BIT 2,(IX+IND)
	DEFB	0DDH,0CBH,-IND,055H	; BIT 2,(IX-IND)
	DEFB	0DDH,0CBH,000H,057H	; BIT 2,(IX)
	DEFB	0DDH,0CBH,IND,057H	; BIT 2,(IX+IND)
	DEFB	0DDH,0CBH,-IND,057H	; BIT 2,(IX-IND)
	DEFB	0DDH,0CBH,000H,058H	; BIT 3,(IX)
	DEFB	0DDH,0CBH,IND,058H	; BIT 3,(IX+IND)
	DEFB	0DDH,0CBH,-IND,058H	; BIT 3,(IX-IND)
	DEFB	0DDH,0CBH,000H,059H	; BIT 3,(IX)
	DEFB	0DDH,0CBH,IND,059H	; BIT 3,(IX+IND)
	DEFB	0DDH,0CBH,-IND,059H	; BIT 3,(IX-IND)
	DEFB	0DDH,0CBH,000H,05AH	; BIT 3,(IX)
	DEFB	0DDH,0CBH,IND,05AH	; BIT 3,(IX+IND)
	DEFB	0DDH,0CBH,-IND,05AH	; BIT 3,(IX-IND)
	DEFB	0DDH,0CBH,000H,05BH	; BIT 3,(IX)
	DEFB	0DDH,0CBH,IND,05BH	; BIT 3,(IX+IND)
	DEFB	0DDH,0CBH,-IND,05BH	; BIT 3,(IX-IND)
	DEFB	0DDH,0CBH,000H,05CH	; BIT 3,(IX)
	DEFB	0DDH,0CBH,IND,05CH	; BIT 3,(IX+IND)
	DEFB	0DDH,0CBH,-IND,05CH	; BIT 3,(IX-IND)
	DEFB	0DDH,0CBH,000H,05DH	; BIT 3,(IX)
	DEFB	0DDH,0CBH,IND,05DH	; BIT 3,(IX+IND)
	DEFB	0DDH,0CBH,-IND,05DH	; BIT 3,(IX-IND)
	DEFB	0DDH,0CBH,000H,05FH	; BIT 3,(IX)
	DEFB	0DDH,0CBH,IND,05FH	; BIT 3,(IX+IND)
	DEFB	0DDH,0CBH,-IND,05FH	; BIT 3,(IX-IND)
	DEFB	0DDH,0CBH,000H,060H	; BIT 4,(IX)
	DEFB	0DDH,0CBH,IND,060H	; BIT 4,(IX+IND)
	DEFB	0DDH,0CBH,-IND,060H	; BIT 4,(IX-IND)
	DEFB	0DDH,0CBH,000H,061H	; BIT 4,(IX)
	DEFB	0DDH,0CBH,IND,061H	; BIT 4,(IX+IND)
	DEFB	0DDH,0CBH,-IND,061H	; BIT 4,(IX-IND)
	DEFB	0DDH,0CBH,000H,062H	; BIT 4,(IX)
	DEFB	0DDH,0CBH,IND,062H	; BIT 4,(IX+IND)
	DEFB	0DDH,0CBH,-IND,062H	; BIT 4,(IX-IND)
	DEFB	0DDH,0CBH,000H,063H	; BIT 4,(IX)
	DEFB	0DDH,0CBH,IND,063H	; BIT 4,(IX+IND)
	DEFB	0DDH,0CBH,-IND,063H	; BIT 4,(IX-IND)
	DEFB	0DDH,0CBH,000H,064H	; BIT 4,(IX)
	DEFB	0DDH,0CBH,IND,064H	; BIT 4,(IX+IND)
	DEFB	0DDH,0CBH,-IND,064H	; BIT 4,(IX-IND)
	DEFB	0DDH,0CBH,000H,065H	; BIT 4,(IX)
	DEFB	0DDH,0CBH,IND,065H	; BIT 4,(IX+IND)
	DEFB	0DDH,0CBH,-IND,065H	; BIT 4,(IX-IND)
	DEFB	0DDH,0CBH,000H,067H	; BIT 4,(IX)
	DEFB	0DDH,0CBH,IND,067H	; BIT 4,(IX+IND)
	DEFB	0DDH,0CBH,-IND,067H	; BIT 4,(IX-IND)
	DEFB	0DDH,0CBH,000H,068H	; BIT 5,(IX)
	DEFB	0DDH,0CBH,IND,068H	; BIT 5,(IX+IND)
	DEFB	0DDH,0CBH,-IND,068H	; BIT 5,(IX-IND)
	DEFB	0DDH,0CBH,000H,069H	; BIT 5,(IX)
	DEFB	0DDH,0CBH,IND,069H	; BIT 5,(IX+IND)
	DEFB	0DDH,0CBH,-IND,069H	; BIT 5,(IX-IND)
	DEFB	0DDH,0CBH,000H,06AH	; BIT 5,(IX)
	DEFB	0DDH,0CBH,IND,06AH	; BIT 5,(IX+IND)
	DEFB	0DDH,0CBH,-IND,06AH	; BIT 5,(IX-IND)
	DEFB	0DDH,0CBH,000H,06BH	; BIT 5,(IX)
	DEFB	0DDH,0CBH,IND,06BH	; BIT 5,(IX+IND)
	DEFB	0DDH,0CBH,-IND,06BH	; BIT 5,(IX-IND)
	DEFB	0DDH,0CBH,000H,06CH	; BIT 5,(IX)
	DEFB	0DDH,0CBH,IND,06CH	; BIT 5,(IX+IND)
	DEFB	0DDH,0CBH,-IND,06CH	; BIT 5,(IX-IND)
	DEFB	0DDH,0CBH,000H,06DH	; BIT 5,(IX)
	DEFB	0DDH,0CBH,IND,06DH	; BIT 5,(IX+IND)
	DEFB	0DDH,0CBH,-IND,06DH	; BIT 5,(IX-IND)
	DEFB	0DDH,0CBH,000H,06FH	; BIT 5,(IX)
	DEFB	0DDH,0CBH,IND,06FH	; BIT 5,(IX+IND)
	DEFB	0DDH,0CBH,-IND,06FH	; BIT 5,(IX-IND)
	DEFB	0DDH,0CBH,000H,070H	; BIT 6,(IX)
	DEFB	0DDH,0CBH,IND,070H	; BIT 6,(IX+IND)
	DEFB	0DDH,0CBH,-IND,070H	; BIT 6,(IX-IND)
	DEFB	0DDH,0CBH,000H,071H	; BIT 6,(IX)
	DEFB	0DDH,0CBH,IND,071H	; BIT 6,(IX+IND)
	DEFB	0DDH,0CBH,-IND,071H	; BIT 6,(IX-IND)
	DEFB	0DDH,0CBH,000H,072H	; BIT 6,(IX)
	DEFB	0DDH,0CBH,IND,072H	; BIT 6,(IX+IND)
	DEFB	0DDH,0CBH,-IND,072H	; BIT 6,(IX-IND)
	DEFB	0DDH,0CBH,000H,073H	; BIT 6,(IX)
	DEFB	0DDH,0CBH,IND,073H	; BIT 6,(IX+IND)
	DEFB	0DDH,0CBH,-IND,073H	; BIT 6,(IX-IND)
	DEFB	0DDH,0CBH,000H,074H	; BIT 6,(IX)
	DEFB	0DDH,0CBH,IND,074H	; BIT 6,(IX+IND)
	DEFB	0DDH,0CBH,-IND,074H	; BIT 6,(IX-IND)
	DEFB	0DDH,0CBH,000H,075H	; BIT 6,(IX)
	DEFB	0DDH,0CBH,IND,075H	; BIT 6,(IX+IND)
	DEFB	0DDH,0CBH,-IND,075H	; BIT 6,(IX-IND)
	DEFB	0DDH,0CBH,000H,077H	; BIT 6,(IX)
	DEFB	0DDH,0CBH,IND,077H	; BIT 6,(IX+IND)
	DEFB	0DDH,0CBH,-IND,077H	; BIT 6,(IX-IND)
	DEFB	0DDH,0CBH,000H,078H	; BIT 7,(IX)
	DEFB	0DDH,0CBH,IND,078H	; BIT 7,(IX+IND)
	DEFB	0DDH,0CBH,-IND,078H	; BIT 7,(IX-IND)
	DEFB	0DDH,0CBH,000H,079H	; BIT 7,(IX)
	DEFB	0DDH,0CBH,IND,079H	; BIT 7,(IX+IND)
	DEFB	0DDH,0CBH,-IND,079H	; BIT 7,(IX-IND)
	DEFB	0DDH,0CBH,000H,07AH	; BIT 7,(IX)
	DEFB	0DDH,0CBH,IND,07AH	; BIT 7,(IX+IND)
	DEFB	0DDH,0CBH,-IND,07AH	; BIT 7,(IX-IND)
	DEFB	0DDH,0CBH,000H,07BH	; BIT 7,(IX)
	DEFB	0DDH,0CBH,IND,07BH	; BIT 7,(IX+IND)
	DEFB	0DDH,0CBH,-IND,07BH	; BIT 7,(IX-IND)
	DEFB	0DDH,0CBH,000H,07CH	; BIT 7,(IX)
	DEFB	0DDH,0CBH,IND,07CH	; BIT 7,(IX+IND)
	DEFB	0DDH,0CBH,-IND,07CH	; BIT 7,(IX-IND)
	DEFB	0DDH,0CBH,000H,07DH	; BIT 7,(IX)
	DEFB	0DDH,0CBH,IND,07DH	; BIT 7,(IX+IND)
	DEFB	0DDH,0CBH,-IND,07DH	; BIT 7,(IX-IND)
	DEFB	0DDH,0CBH,000H,07FH	; BIT 7,(IX)
	DEFB	0DDH,0CBH,IND,07FH	; BIT 7,(IX+IND)
	DEFB	0DDH,0CBH,-IND,07FH	; BIT 7,(IX-IND)
	DEFB	0FDH,0CBH,000H,040H	; BIT 0,(IY)
	DEFB	0FDH,0CBH,IND,040H	; BIT 0,(IY+IND)
	DEFB	0FDH,0CBH,-IND,040H	; BIT 0,(IY-IND)
	DEFB	0FDH,0CBH,000H,041H	; BIT 0,(IY)
	DEFB	0FDH,0CBH,IND,041H	; BIT 0,(IY+IND)
	DEFB	0FDH,0CBH,-IND,041H	; BIT 0,(IY-IND)
	DEFB	0FDH,0CBH,000H,042H	; BIT 0,(IY)
	DEFB	0FDH,0CBH,IND,042H	; BIT 0,(IY+IND)
	DEFB	0FDH,0CBH,-IND,042H	; BIT 0,(IY-IND)
	DEFB	0FDH,0CBH,000H,043H	; BIT 0,(IY)
	DEFB	0FDH,0CBH,IND,043H	; BIT 0,(IY+IND)
	DEFB	0FDH,0CBH,-IND,043H	; BIT 0,(IY-IND)
	DEFB	0FDH,0CBH,000H,044H	; BIT 0,(IY)
	DEFB	0FDH,0CBH,IND,044H	; BIT 0,(IY+IND)
	DEFB	0FDH,0CBH,-IND,044H	; BIT 0,(IY-IND)
	DEFB	0FDH,0CBH,000H,045H	; BIT 0,(IY)
	DEFB	0FDH,0CBH,IND,045H	; BIT 0,(IY+IND)
	DEFB	0FDH,0CBH,-IND,045H	; BIT 0,(IY-IND)
	DEFB	0FDH,0CBH,000H,047H	; BIT 0,(IY)
	DEFB	0FDH,0CBH,IND,047H	; BIT 0,(IY+IND)
	DEFB	0FDH,0CBH,-IND,047H	; BIT 0,(IY-IND)
	DEFB	0FDH,0CBH,000H,048H	; BIT 1,(IY)
	DEFB	0FDH,0CBH,IND,048H	; BIT 1,(IY+IND)
	DEFB	0FDH,0CBH,-IND,048H	; BIT 1,(IY-IND)
	DEFB	0FDH,0CBH,000H,049H	; BIT 1,(IY)
	DEFB	0FDH,0CBH,IND,049H	; BIT 1,(IY+IND)
	DEFB	0FDH,0CBH,-IND,049H	; BIT 1,(IY-IND)
	DEFB	0FDH,0CBH,000H,04AH	; BIT 1,(IY)
	DEFB	0FDH,0CBH,IND,04AH	; BIT 1,(IY+IND)
	DEFB	0FDH,0CBH,-IND,04AH	; BIT 1,(IY-IND)
	DEFB	0FDH,0CBH,000H,04BH	; BIT 1,(IY)
	DEFB	0FDH,0CBH,IND,04BH	; BIT 1,(IY+IND)
	DEFB	0FDH,0CBH,-IND,04BH	; BIT 1,(IY-IND)
	DEFB	0FDH,0CBH,000H,04CH	; BIT 1,(IY)
	DEFB	0FDH,0CBH,IND,04CH	; BIT 1,(IY+IND)
	DEFB	0FDH,0CBH,-IND,04CH	; BIT 1,(IY-IND)
	DEFB	0FDH,0CBH,000H,04DH	; BIT 1,(IY)
	DEFB	0FDH,0CBH,IND,04DH	; BIT 1,(IY+IND)
	DEFB	0FDH,0CBH,-IND,04DH	; BIT 1,(IY-IND)
	DEFB	0FDH,0CBH,000H,04FH	; BIT 1,(IY)
	DEFB	0FDH,0CBH,IND,04FH	; BIT 1,(IY+IND)
	DEFB	0FDH,0CBH,-IND,04FH	; BIT 1,(IY-IND)
	DEFB	0FDH,0CBH,000H,050H	; BIT 2,(IY)
	DEFB	0FDH,0CBH,IND,050H	; BIT 2,(IY+IND)
	DEFB	0FDH,0CBH,-IND,050H	; BIT 2,(IY-IND)
	DEFB	0FDH,0CBH,000H,051H	; BIT 2,(IY)
	DEFB	0FDH,0CBH,IND,051H	; BIT 2,(IY+IND)
	DEFB	0FDH,0CBH,-IND,051H	; BIT 2,(IY-IND)
	DEFB	0FDH,0CBH,000H,052H	; BIT 2,(IY)
	DEFB	0FDH,0CBH,IND,052H	; BIT 2,(IY+IND)
	DEFB	0FDH,0CBH,-IND,052H	; BIT 2,(IY-IND)
	DEFB	0FDH,0CBH,000H,053H	; BIT 2,(IY)
	DEFB	0FDH,0CBH,IND,053H	; BIT 2,(IY+IND)
	DEFB	0FDH,0CBH,-IND,053H	; BIT 2,(IY-IND)
	DEFB	0FDH,0CBH,000H,054H	; BIT 2,(IY)
	DEFB	0FDH,0CBH,IND,054H	; BIT 2,(IY+IND)
	DEFB	0FDH,0CBH,-IND,054H	; BIT 2,(IY-IND)
	DEFB	0FDH,0CBH,000H,055H	; BIT 2,(IY)
	DEFB	0FDH,0CBH,IND,055H	; BIT 2,(IY+IND)
	DEFB	0FDH,0CBH,-IND,055H	; BIT 2,(IY-IND)
	DEFB	0FDH,0CBH,000H,057H	; BIT 2,(IY)
	DEFB	0FDH,0CBH,IND,057H	; BIT 2,(IY+IND)
	DEFB	0FDH,0CBH,-IND,057H	; BIT 2,(IY-IND)
	DEFB	0FDH,0CBH,000H,058H	; BIT 3,(IY)
	DEFB	0FDH,0CBH,IND,058H	; BIT 3,(IY+IND)
	DEFB	0FDH,0CBH,-IND,058H	; BIT 3,(IY-IND)
	DEFB	0FDH,0CBH,000H,059H	; BIT 3,(IY)
	DEFB	0FDH,0CBH,IND,059H	; BIT 3,(IY+IND)
	DEFB	0FDH,0CBH,-IND,059H	; BIT 3,(IY-IND)
	DEFB	0FDH,0CBH,000H,05AH	; BIT 3,(IY)
	DEFB	0FDH,0CBH,IND,05AH	; BIT 3,(IY+IND)
	DEFB	0FDH,0CBH,-IND,05AH	; BIT 3,(IY-IND)
	DEFB	0FDH,0CBH,000H,05BH	; BIT 3,(IY)
	DEFB	0FDH,0CBH,IND,05BH	; BIT 3,(IY+IND)
	DEFB	0FDH,0CBH,-IND,05BH	; BIT 3,(IY-IND)
	DEFB	0FDH,0CBH,000H,05CH	; BIT 3,(IY)
	DEFB	0FDH,0CBH,IND,05CH	; BIT 3,(IY+IND)
	DEFB	0FDH,0CBH,-IND,05CH	; BIT 3,(IY-IND)
	DEFB	0FDH,0CBH,000H,05DH	; BIT 3,(IY)
	DEFB	0FDH,0CBH,IND,05DH	; BIT 3,(IY+IND)
	DEFB	0FDH,0CBH,-IND,05DH	; BIT 3,(IY-IND)
	DEFB	0FDH,0CBH,000H,05FH	; BIT 3,(IY)
	DEFB	0FDH,0CBH,IND,05FH	; BIT 3,(IY+IND)
	DEFB	0FDH,0CBH,-IND,05FH	; BIT 3,(IY-IND)
	DEFB	0FDH,0CBH,000H,060H	; BIT 4,(IY)
	DEFB	0FDH,0CBH,IND,060H	; BIT 4,(IY+IND)
	DEFB	0FDH,0CBH,-IND,060H	; BIT 4,(IY-IND)
	DEFB	0FDH,0CBH,000H,061H	; BIT 4,(IY)
	DEFB	0FDH,0CBH,IND,061H	; BIT 4,(IY+IND)
	DEFB	0FDH,0CBH,-IND,061H	; BIT 4,(IY-IND)
	DEFB	0FDH,0CBH,000H,062H	; BIT 4,(IY)
	DEFB	0FDH,0CBH,IND,062H	; BIT 4,(IY+IND)
	DEFB	0FDH,0CBH,-IND,062H	; BIT 4,(IY-IND)
	DEFB	0FDH,0CBH,000H,063H	; BIT 4,(IY)
	DEFB	0FDH,0CBH,IND,063H	; BIT 4,(IY+IND)
	DEFB	0FDH,0CBH,-IND,063H	; BIT 4,(IY-IND)
	DEFB	0FDH,0CBH,000H,064H	; BIT 4,(IY)
	DEFB	0FDH,0CBH,IND,064H	; BIT 4,(IY+IND)
	DEFB	0FDH,0CBH,-IND,064H	; BIT 4,(IY-IND)
	DEFB	0FDH,0CBH,000H,065H	; BIT 4,(IY)
	DEFB	0FDH,0CBH,IND,065H	; BIT 4,(IY+IND)
	DEFB	0FDH,0CBH,-IND,065H	; BIT 4,(IY-IND)
	DEFB	0FDH,0CBH,000H,067H	; BIT 4,(IY)
	DEFB	0FDH,0CBH,IND,067H	; BIT 4,(IY+IND)
	DEFB	0FDH,0CBH,-IND,067H	; BIT 4,(IY-IND)
	DEFB	0FDH,0CBH,000H,068H	; BIT 5,(IY)
	DEFB	0FDH,0CBH,IND,068H	; BIT 5,(IY+IND)
	DEFB	0FDH,0CBH,-IND,068H	; BIT 5,(IY-IND)
	DEFB	0FDH,0CBH,000H,069H	; BIT 5,(IY)
	DEFB	0FDH,0CBH,IND,069H	; BIT 5,(IY+IND)
	DEFB	0FDH,0CBH,-IND,069H	; BIT 5,(IY-IND)
	DEFB	0FDH,0CBH,000H,06AH	; BIT 5,(IY)
	DEFB	0FDH,0CBH,IND,06AH	; BIT 5,(IY+IND)
	DEFB	0FDH,0CBH,-IND,06AH	; BIT 5,(IY-IND)
	DEFB	0FDH,0CBH,000H,06BH	; BIT 5,(IY)
	DEFB	0FDH,0CBH,IND,06BH	; BIT 5,(IY+IND)
	DEFB	0FDH,0CBH,-IND,06BH	; BIT 5,(IY-IND)
	DEFB	0FDH,0CBH,000H,06CH	; BIT 5,(IY)
	DEFB	0FDH,0CBH,IND,06CH	; BIT 5,(IY+IND)
	DEFB	0FDH,0CBH,-IND,06CH	; BIT 5,(IY-IND)
	DEFB	0FDH,0CBH,000H,06DH	; BIT 5,(IY)
	DEFB	0FDH,0CBH,IND,06DH	; BIT 5,(IY+IND)
	DEFB	0FDH,0CBH,-IND,06DH	; BIT 5,(IY-IND)
	DEFB	0FDH,0CBH,000H,06FH	; BIT 5,(IY)
	DEFB	0FDH,0CBH,IND,06FH	; BIT 5,(IY+IND)
	DEFB	0FDH,0CBH,-IND,06FH	; BIT 5,(IY-IND)
	DEFB	0FDH,0CBH,000H,070H	; BIT 6,(IY)
	DEFB	0FDH,0CBH,IND,070H	; BIT 6,(IY+IND)
	DEFB	0FDH,0CBH,-IND,070H	; BIT 6,(IY-IND)
	DEFB	0FDH,0CBH,000H,071H	; BIT 6,(IY)
	DEFB	0FDH,0CBH,IND,071H	; BIT 6,(IY+IND)
	DEFB	0FDH,0CBH,-IND,071H	; BIT 6,(IY-IND)
	DEFB	0FDH,0CBH,000H,072H	; BIT 6,(IY)
	DEFB	0FDH,0CBH,IND,072H	; BIT 6,(IY+IND)
	DEFB	0FDH,0CBH,-IND,072H	; BIT 6,(IY-IND)
	DEFB	0FDH,0CBH,000H,073H	; BIT 6,(IY)
	DEFB	0FDH,0CBH,IND,073H	; BIT 6,(IY+IND)
	DEFB	0FDH,0CBH,-IND,073H	; BIT 6,(IY-IND)
	DEFB	0FDH,0CBH,000H,074H	; BIT 6,(IY)
	DEFB	0FDH,0CBH,IND,074H	; BIT 6,(IY+IND)
	DEFB	0FDH,0CBH,-IND,074H	; BIT 6,(IY-IND)
	DEFB	0FDH,0CBH,000H,075H	; BIT 6,(IY)
	DEFB	0FDH,0CBH,IND,075H	; BIT 6,(IY+IND)
	DEFB	0FDH,0CBH,-IND,075H	; BIT 6,(IY-IND)
	DEFB	0FDH,0CBH,000H,077H	; BIT 6,(IY)
	DEFB	0FDH,0CBH,IND,077H	; BIT 6,(IY+IND)
	DEFB	0FDH,0CBH,-IND,077H	; BIT 6,(IY-IND)
	DEFB	0FDH,0CBH,000H,078H	; BIT 7,(IY)
	DEFB	0FDH,0CBH,IND,078H	; BIT 7,(IY+IND)
	DEFB	0FDH,0CBH,-IND,078H	; BIT 7,(IY-IND)
	DEFB	0FDH,0CBH,000H,079H	; BIT 7,(IY)
	DEFB	0FDH,0CBH,IND,079H	; BIT 7,(IY+IND)
	DEFB	0FDH,0CBH,-IND,079H	; BIT 7,(IY-IND)
	DEFB	0FDH,0CBH,000H,07AH	; BIT 7,(IY)
	DEFB	0FDH,0CBH,IND,07AH	; BIT 7,(IY+IND)
	DEFB	0FDH,0CBH,-IND,07AH	; BIT 7,(IY-IND)
	DEFB	0FDH,0CBH,000H,07BH	; BIT 7,(IY)
	DEFB	0FDH,0CBH,IND,07BH	; BIT 7,(IY+IND)
	DEFB	0FDH,0CBH,-IND,07BH	; BIT 7,(IY-IND)
	DEFB	0FDH,0CBH,000H,07CH	; BIT 7,(IY)
	DEFB	0FDH,0CBH,IND,07CH	; BIT 7,(IY+IND)
	DEFB	0FDH,0CBH,-IND,07CH	; BIT 7,(IY-IND)
	DEFB	0FDH,0CBH,000H,07DH	; BIT 7,(IY)
	DEFB	0FDH,0CBH,IND,07DH	; BIT 7,(IY+IND)
	DEFB	0FDH,0CBH,-IND,07DH	; BIT 7,(IY-IND)
	DEFB	0FDH,0CBH,000H,07FH	; BIT 7,(IY)
	DEFB	0FDH,0CBH,IND,07FH	; BIT 7,(IY+IND)
	DEFB	0FDH,0CBH,-IND,07FH	; BIT 7,(IY-IND)

NN:	DEFS	2

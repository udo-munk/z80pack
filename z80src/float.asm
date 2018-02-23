;
; this is a Z80 floating point package from an ancient german computer magazine
; I'm not going to translate this into english
; assemble this source with z80asm and run it under z80sim, if everything
; is working it should print the numbers below
;
; *******************************
; * Fliesskomma-Arithmetik fuer	*
; * den Z80-Mikroprozessor	*
; * (mc 12/88, Seite 100	*
; *******************************

; ********************************************************
; * Die folgende Testroutine liefert die Ausgabe:
; *	40400000
; *	00700000
; *	7F800000
; *	35BFFFFF
; *	00400000
; *	7F7FFFFF
; *	7F800000
; *	406DB6DB
; *	15400001

START:
	LD	SP,STACK
	LD	BC,3F80H	; Aufruf der Additionsroutine
	LD	DE,0000H	; mit verschiedenen Parametern
	PUSH	BC		; entspricht 1 + 2
	PUSH	DE
	LD	BC,4000H
	LD	DE,0000H
	PUSH	BC
	PUSH	DE
	CALL	F_ADD
	CALL	HEXOUT		; anschliessend Ausgabe

	LD	BC,00F0H	; eine kleine, gerade noch normalisierte
	LD	DE,0000H	; Zahl, dazu die kleinste normalisierte
	PUSH	BC		; Zahl mit negativem Vorzeichen addieren
	PUSH	DE
	LD	BC,8080H
	LD	DE,0000H
	PUSH	BC
	PUSH	DE
	CALL	F_ADD
	CALL	HEXOUT

	LD	BC,7F00H	; die Summe dieser beiden Zahlen
	LD	DE,0000H	; ergibt unendlich. Setzt man
	PUSH	BC		; fuer die zweite Zahl den Wert
	PUSH	DE		; 7EFFFFFE, so ist das Ergebnis
	LD	BC,7EFFH	; gerade MAXFLOAT
	LD	DE,0FFFFH
	PUSH	BC
	PUSH	DE
	CALL	F_ADD
	CALL	HEXOUT

	LD	BC,0000H	; Multiplikation testen
	LD	DE,0003H	; MAXFLOAT * <denormalisierte Zahl>
	PUSH	BC
	PUSH	DE
	LD	BC,7F7FH
	LD	DE,0FFFFH
	PUSH	BC
	PUSH	DE
	CALL	F_MUL
	CALL	HEXOUT

	LD	BC,0080H	; die kleinste normalisierte Zahl
	LD	DE,0000H	; mit 0.5 multiplizieren
	PUSH	BC		; (ergibt eine denormalisierte Zahl)
	PUSH	DE
	LD	BC,3F00H
	LD	DE,0000H
	PUSH	BC
	PUSH	DE
	CALL	F_MUL
	CALL	HEXOUT

	LD	BC,4000H	; eine sehr grosse Zahl mit zwei
	LD	DE,0000H	; multiplizieren. Das Ergebnis
	PUSH	BC		; ist genau MAXFLOAT
	PUSH	DE
	LD	BC,7EFFH
	LD	DE,0FFFFH
	PUSH	BC
	PUSH	DE
	CALL	F_MUL
	CALL	HEXOUT

	LD	BC,0000H	; Test der Divisionsroutine
	LD	DE,0000H	; hier 1 / 0 (ergibt unendlich)
	PUSH	BC
	PUSH	DE
	LD	BC,3F80H
	LD	DE,0000H
	PUSH	BC
	PUSH	DE
	CALL	F_DIV
	CALL	HEXOUT

	LD	BC,40E0H	; jetzt 26 / 7 berechnen
	LD	DE,0000H
	PUSH	BC
	PUSH	DE
	LD	BC,41D0H
	LD	DE,0000H
	PUSH	BC
	PUSH	DE
	CALL	F_DIV
	CALL	HEXOUT

	LD	BC,1FFFH	; jetzt eine sehr kleine
	LD	DE,0FFFFH	; denormalisierte Zahl durch
	PUSH	BC		; eine kleine normalisierte
	PUSH	DE		; Zahl dividieren
	LD	BC,0000H
	LD	DE,0003H
	PUSH	BC
	PUSH	DE
	CALL	F_DIV
	CALL	HEXOUT

	HALT			; Ende des Tests

	DEFS	100
STACK:

; ************************************************
; * Zahl in BC-DE in 8 Hexadezimalziffern drucken.
; * Dazu werden nacheinander die Nibble-Paare in
; * B, C, D und E ausgedruckt.
; *

HEXOUT:
	LD	A,B		; Nacheinander die einzelnen
	CALL	DIG2		; Nibble-Paare in A laden
	LD	A,C		; und ausdrucken
	CALL	DIG2
	LD	A,D
	CALL	DIG2
	LD	A,E
	CALL	DIG2
	LD	A,10
	CALL	OUTCHAR
	LD	A,13
	CALL	OUTCHAR
	RET

DIG2:
	PUSH	AF		; Nibble-Paar ausdrucken
	RRCA			; unterstes Nibble retten
	RRCA			; oberes Nibble rechtsbuendig
	RRCA			; positionieren
	RRCA
	AND	00001111B
	ADD	A,90H		; binaer in ASCII (hex)
	DAA
	ADC	A,40H
	DAA
	CALL	OUTCHAR		; Zeichen ausgeben
	POP	AF		; jetzt unteres Nibble verarbeiten
	AND	00001111B	; Nibble maskieren
	ADD	A,90H		; binaer in ASCII (hex)
	DAA
	ADC	A,40H
	DAA
	CALL	OUTCHAR
	RET

OUTCHAR:			; Zeichen auf Console ausgeben
	OUT	(1),A
	RET

; **********************************
; * Globale Konstanten-Definitionen
; * fuer das Fliesskommapaket
; *

MAXEXPO	EQU	255		; Maximal zulaessiger Exponent
BIAS	EQU	127		; Bias des Exponenten

; *************************************************
; * Fliesskomma-Addition in Single-Precision
; * Parameter: Operand 1 und Operand 2 ueber Stack
; * Ergebnis:  in BC-DE: MSB in B, LSB in E
; *

; * Es folgen Offset-Definitionen fuer Stack-relativen Zugriff

FHL_ALT	EQU	0		; Top of Stack liegt HL
FADR	EQU	2		; dann die Ruecksprungadresse
OP1	EQU	4		; jetzt Offset-Definitionen fuer
OP2	EQU	8		; Parameter-Uebergabe

OPSIZE	EQU	4		; Groesse eines Operanden

F_ADD:
	PUSH	HL		; alten Basepointer retten
	LD	(F_STACK),SP	; aktuellen Stackpointer abspeichern
	LD	HL,(F_STACK)	; und in HL laden (= Basepointer)
	PUSH	AF		; benoetigte Register retten
	PUSH	IX
	PUSH	IY
	LD	BC,OP1		; jeztz die Zeiger auf die
	ADD	HL,BC		; Operanden initialisieren
	PUSH	HL
	POP	IX		; IX zeigt auf Operand 1
	LD	BC,OPSIZE
	ADD	HL,BC
	PUSH	HL
	POP	IY		; IY zeigt auf Operand 2
F_ADSUB:
	ADD	HL,BC		; HL zeigt jetzt hinter die Operanden!
	LD	(F_STACK),HL	; diese Adresse fuer's Ende merken
	LD	A,(IX+3)	; Vorzeichen von Operand 1 laden
	LD	E,A		; Ergebnisvorzeichen in E, Bit 7
	XOR	(IY+3)		; mit Vorzeichen von OP2 verknuepfen
	LD	D,A		; Subtraktionsflag in D, Bit 7
	RES	7,(IX+3)	; Vorzeichen in Mantisse 1 loeschen
	RES	7,(IY+3)	; Vorzeichen in Mantisse 2 loeschen

; Die Operanden sind jetzt in der Form: 0EEE EEEE EFFF ... FFFF

	LD	A,(IX+0)	; Differenz OP1 - OP2 bilden
	SUB	(IY+0)
	LD	A,(IX+1)
	SBC	A,(IY+1)
	LD	A,(IX+2)
	SBC	A,(IY+2)
	LD	A,(IX+3)
	SBC	A,(IY+3)
	JR	NC,FAD_1	; Sprung falls OP1 groesser als OP2
	PUSH	IX		; ansonsten Operanden vertauschen
	EX	(SP),IY		; (eigentlich nur die Pointer), so
	POP	IX		; dass IY den Kleineren adressiert
	LD	A,E		; Ergebnisvorzeichen neu berechnen
	XOR	D
	LD	E,A
FAD_1:
	LD	A,(IX+2)
	LD	C,(IX+3)	; Exponent der groesseren Zahl laden
	SLA	A
	RL	C
	JR	Z,AD_DN1
	SET	7,(IX+2)	; implizite Eins erzeugen
AD_DN1:
	LD	A,(IY+2)
	LD	B,(IY+3)	; Exponent der kleineren Zahl laden
	SLA	A
	RL	B
	JR	Z,AD_DN2
	SET	7,(IY+2)	; implizite Eins erzeugen
AD_DN2:
	PUSH	BC		; Jetzt die Register fuer den
	PUSH	DE		; Blocktransferbefehl retten
	LD	BC,(OPSIZE*2)-1 ; beide Operanden verschieben
	DEC	HL		; HL zeigt auf letztes Byte
	PUSH	HL		; HL nach DE kopieren
	POP	DE
	DEC	HL		; HL zeigt auf vorletztes Byte
	LDDR			; Verschiebung beider Mantissen
	POP	DE		; um 8 Bit nach links
	POP	BC
	XOR	A
	LD	(IX+0),A	; Form: FFFF ... FFFF 0000 0000
	LD	(IY+0),A
	LD	A,C		; Differenz der Exponenten berechnen
	SUB	B
	LD	B,A		; Differenz nach B fuer Loop-Befehl
	JR	Z,AD_NAP	; falls Null, dann keine Anpassung
	CP	25		; mehr als 24? (Abfrage mit Carry
	JP	NC,AD_RND	; erfordert Vergleich mit 25)
AD_ANP:
	SRL	(IY+3)		; Anpassung der zweiten Mantisse
	RR	(IY+2)		; durch Verschiebung nach rechts
	RR	(IY+1)
	RR	(IY+0)
	DJNZ	AD_ANP		; Loop-Befehl bis B = 0
AD_NAP:
	BIT	7,D		; Subtraktion oder Addition?
	JR	NZ,SUBTR	; ggf. zur Subtraktion springen
	LD	A,(IX+0)	; jetzt werden die beiden Mantissen
	ADD	A,(IY+0)	; zueinander addiert
	LD	(IX+0),A
	LD	A,(IX+1)
	ADC	A,(IY+1)
	LD	(IX+1),A
	LD	A,(IX+2)
	ADC	A,(IY+2)
	LD	(IX+2),A
	LD	A,(IX+3)
	ADC	A,(IY+3)
	LD	(IX+3),A
	JR	NC,AD_RND	; kein Ueberlauf --> zum Runden
	RR	(IX+3)		; Ueberlauf einschieben
	RR	(IX+2)		; und Exponent erhoehen
	RR	(IX+1)		; durch die Vorgeschichte ist
	RR	(IX+0)		; gesichert, dass B Null ist; BC
	INC	BC		; enthaelt den 16-Bit-Exponent
	JR	AD_RND		; und zum Runden
SUBTR:
	LD	A,(IX+0)	; Die beiden Mantissen werden
	SUB	(IY+0)		; voneinander subtrahiert
	LD	(IX+0),A
	LD	A,(IX+1)
	SBC	A,(IY+1)
	LD	(IX+1),A
	LD	A,(IX+2)
	SBC	A,(IY+2)
	LD	(IX+2),A
	LD	A,(IX+3)
	SBC	A,(IY+3)
	LD	(IX+3),A
	JP	M,AD_RND	; bei fuehrender Eins zum Runden
	JR	NZ,AD_NRM	; ungleich Null: Normalisieren
	CP	(IX+2)		; Rest der Mantisse auch Null?
	JR	NZ,AD_NRM
	CP	(IX+1)
	JR	NZ,AD_NRM
	CP	(IX+0)
	JR	Z,AD_ZERO	; alles Null --> Ergebnis ist Null
AD_NRM:
	XOR	A		; A = 0
AD_NR1:
	CP	C		; Exponent ist Null?
	JR	NZ,AD_NR2	; nein, Normierung moeglich
	CP	B		; oberes Byte auch Null?
	JR	Z,AD_RND	; dann ist Ergebnis denormalisiert
AD_NR2:
	DEC	BC		; Exponent erniedrigen
	SLA	(IX+0)		; Mantisse normalisieren bis
	RL	(IX+1)		; fuehrende Eins auftaucht
	RL	(IX+2)
	RL	(IX+3)
	JP	P,AD_NR1	; weiter bis fuehrende Eins auftaucht
AD_RND:
	LD	A,(IX+0)	; jetzt Runden auf Bit hinter
	ADD	A,80H		; Mantisse
	JR	NC,AD_NOV	; kein Uebertrag?
	INC	(IX+1)		; doch, naechstes Mantissenbyte
	JR	NZ,AD_NOV	; behandeln, jetzt auf Null pruefen,
	INC	(IX+2)		; da der INC-Befehl kein Carry liefert
	JR	NZ,AD_NOV
	INC	(IX+3)
	JR	NZ,AD_NOV
	SCF			; Eins erzeugen
	RR	(IX+3)		; bei Ueberlauf Mantisse durch
	RR	(IX+2)		; Rechtsschieben wieder normalisieren
	RR	(IX+1)		; (nur noch 24 Bit noetig)
	INC	BC		; und Exponent korrigieren
AD_NOV:
	XOR	A		; A = 0
	CP	(IX+3)		; Mantisse auf Null pruefen
	JR	NZ,AD_NOZ
	CP	(IX+2)
	JR	NZ,AD_NOZ
	CP	(IX+1)		; alle Mantissenbytes Null?
	JR	NZ,AD_NOZ	; dann ist auch das Ergebnis Null
AD_ZERO:			; Null Ergebnis aufbauen
	LD	B,A
	LD	C,A
	LD	D,A
	LD	E,A
	JR	AD_EXIT		; dann Routine verlassen
AD_NOZ:
	CP	B		; A ist 0
	LD	A,MAXEXPO	; Exponent oberstes Byte ungleich Null?
	JR	NZ,AD_OVR	; dann ist Ueberlauf eingetreten
	CP	C		; oder genau maxexpo erreicht?
	JR	NZ,AD_NUE	; nein, --> kein Ueberlauf
AD_OVR:
	LD	C,A		; Exponent auf maxexpo setzen
	XOR	A		; und Mantisse auf Null
	LD	(IX+3),A	; fuer unendlich
	LD	(IX+2),A
	LD	(IX+1),A
	JR	AD_DEN
AD_NUE:
	XOR	A		; A = 0
	CP	C		; Exponent Null (Zahl denormalisiert)?
	JR	Z,AD_DEN	; ja, -->
	SLA	(IX+1)		; fuehrendes Bit wird nicht gespeichert
	RL	(IX+2)		; daher Mantisse um 1 Bit nach links
	RL	(IX+3)
AD_DEN:
	LD	B,C		; Ergebnis aufbauen: Exponent in B
	LD	C,(IX+3)	; Mantisse oberstes Byte
	LD	D,(IX+2)
	SLA	E		; Vorzeichen aus E in Carry schieben
	LD	E,(IX+1)
	RR	B		; Vorzeichen in Ergebnis einschieben
	RR	C
	RR	D
	RR	E
AD_EXIT:
	POP	IY		; Register restaurieren
	POP	IX
	POP	AF
	POP	HL
	LD	(F_HL),HL	; HL zwischenspeichern
	EX	(SP),HL		; alte Ruecksprungadresse in HL
	LD	SP,(F_STACK)	; Stack zuruecksetzen
	PUSH	HL		; Ruecksprungadresse ablegen
	LD	HL,(F_HL)	; HL wieder laden
	RET			; Ende des Unterprogramms

; *************************************************
; * Fliesskomma-Subtraktion in Single-Precision
; * Parameter: Operand 1 und Operand 2 ueber Stack
; * Ergebnis: in BC-DE: MSB in B, LSB in E
; *

F_SUB:
	PUSH	HL		; alten Basepointer retten
	LD	(F_STACK),SP	; aktuellen Stackpointer abspeichern
	LD	HL,(F_STACK)	; und in HL laden (= Basepointer)
	PUSH	AF		; benoetigte Register retten
	PUSH	IX
	PUSH	IY
	LD	BC,OP1
	ADD	HL,BC
	PUSH	HL
	POP	IX		; IX zeigt auf Operand 1
	LD	BC,OPSIZE
	ADD	HL,BC
	PUSH	HL
	POP	IY		; IY zeigt auf Operand 2
	LD	A,80H
	XOR	(IY+3)		; Vorzeichenbit von Operand 2 umdrehen
	LD	(IY+3),A	; wieder abspeichern
	JP	F_ADSUB		; jetzt weiter bei Additionsroutine

; *************************************************
; * Fliesskomma-Multiplikation in Single-Precision
; * Parameter: Operand 1 und Operand 2 ueber Stack
; * Ergebnis: in BC-DE: MSB in B, LSB in E
; *

TEMP	EQU	-10		; Offset lokale Variable (6 Byte)

F_MUL:
	PUSH	HL		; alten Basepointer retten
	LD	(F_STACK),SP	; aktuellen Stackpointer abspeichern
	LD	HL,(F_STACK)	; und in HL laden (= Basepointer)
	PUSH	AF		; benoetigte Register retten
	PUSH	IX
	PUSH	IY
	LD	BC,OP1
	ADD	HL,BC
	PUSH	HL
	EX	(SP),IX		; IX zeigt auf Operand 1
				; 2 Dummy-Byte auf Stack fuer lokale
	LD	BC,OPSIZE	; Variable bleiben stehen
	ADD	HL,BC
	PUSH	HL
	EX	(SP),IY		; IY zeigt auf Operand 2
	PUSH	HL		; insgesamt 6 Byte fuer lokale Variable
	ADD	HL,BC		; HL zeigt jetzt hinter die Operanden!
	LD	(F_STACK),HL
	LD	A,(IX+3)	; Ergebnisvorzeichen bestimmen
	XOR	(IY+3)
	LD	C,A		; Vorzeichen in C Bit 7 merken
	LD	D,0		; Exponent 1 laden
	LD	E,(IX+3)
	LD	A,(IX+2)	; Operand um 8 Bit nach links schieben
	LD	(IX+3),A
	RES	7,(IX+3)	; implizite Null vorbesetzen
	SLA	A		; Exponent unterstes Bit in Carry
	RL	E		; und in E einschieben
	JR	Z,MU_DN1	; falls Null, dann OP1 denormalisieren
	SET	7,(IX+3)	; implizite Eins erzeugen
	DEC	DE		; Bias kompensieren
MU_DN1:
	LD	A,(IX+1)	; jetzt restliche Bytes verschieben
	LD	(IX+2),A
	LD	A,(IX+0)
	LD	(IX+1),A
	XOR	A		; unterste Mantissenbits loeschen
	LD	(IX+0),A	; Form: FFFF ... FFFF 0000 0000
	LD	(IX+TEMP+5),A	; lokale Variable mit Null vorbesetzen
	LD	(IX+TEMP+4),A
	LD	(IX+TEMP+3),A
	LD	(IX+TEMP+2),A
	LD	(IX+TEMP+1),A
	LD	(IX+TEMP+0),A
	LD	H,A		; Exponent 2 in HL aufbauen
	LD	L,(IY+3)
	LD	A,(IY+2)
	RES	7,(IY+2)	; implizite Null vorbesetzen
	SLA	A
	RL	L
	JR	Z,MU_DN2	; gleich Null, dann Op2 denormalisieren
	SET	7,(IY+2)	; implizite Eins erzeugen
	DEC	HL		; Bias kompensieren
MU_DN2:
	ADD	HL,DE		; Exponenten aufaddieren
	LD	DE,3-BIAS	; Bias-3 subtrahieren
	ADD	HL,DE		; bzw. 3-Bias addieren
	JP	P,MU_NOZ
	LD	A,L		; Exponent kleiner als -24?
	CP	-24
	JR	NC,MU_NOZ
	JP	MU_ZERO		; ja, dann ist das Ergebnis Null
MU_NOZ:
	LD	B,24		; Multiplikationsschleifenzaehler
	LD	DE,0		; Hilfsregister fuer Multiplikand
MU_MUL:
	SRL	(IX+3)		; Multiplikand nach rechts schieben
	RR	(IX+2)
	RR	(IX+1)
	RR	(IX+0)
	RR	D		; DE als Verlaengerung von Operand 1
	RR	E
	SLA	(IY+0)		; Multiplikator nach links schieben
	RL	(IY+1)
	RL	(IY+2)		; falls fuehrendes Bit Null ist, dann
	JR	NC,MU_NAD	; muss nicht addiert werden
	LD	A,(IX+TEMP+0)	; sonst Multiplikand aufaddieren
	ADD	A,E
	LD	(IX+TEMP+0),A
	LD	A,(IX+TEMP+1)
	ADC	A,D
	LD	(IX+TEMP+1),A
	LD	A,(IX+TEMP+2)
	ADC	A,(IX+0)
	LD	(IX+TEMP+2),A
	LD	A,(IX+TEMP+3)
	ADC	A,(IX+1)
	LD	(IX+TEMP+3),A
	LD	A,(IX+TEMP+4)
	ADC	A,(IX+2)
	LD	(IX+TEMP+4),A
	LD	A,(IX+TEMP+5)
	ADC	A,(IX+3)
	LD	(IX+TEMP+5),A
MU_NAD:
	DJNZ	MU_MUL		; Schleife durchlaufen
	LD	A,(IX+TEMP+5)
	OR	A		; Flags setzen
	JP	M,MU_RND	; bei fuerender Eins zum Runden
	JR	NZ,MU_NOR	; ungleich Null --> normalisieren
	CP	(IX+TEMP+4)
	JR	NZ,MU_NOR
	CP	(IX+TEMP+3)
	JR	NZ,MU_NOR
	CP	(IX+TEMP+2)
	JR	NZ,MU_NOR
	JP	MU_ZERO		; Mantisse komplett Null --> Null
MU_NOR:
	XOR	A		; A = 0
	OR	H		; Exponent ist negativ?
	JP	M,MU_UNT	; ggf. Unterlauf behandeln
MU_NR1:
	XOR	A		; A = 0
	CP	L		; Exponent = Null?
	JR	NZ,MU_NR2
	CP	H		; bei Null zum Runden
	JR	Z,MU_RND
MU_NR2:
	DEC	HL		; Exponent erniedrigen
	SLA	(IX+TEMP+0)
	RL	(IX+TEMP+1)
	RL	(IX+TEMP+2)	; Mantisse solange nach links
	RL	(IX+TEMP+3)	; verschieben bis fuerende Eins
	RL	(IX+TEMP+4)	; auftaucht
	RL	(IX+TEMP+5)
	JP	P,MU_NR1
MU_RND:
	LD	A,(IX+TEMP+2)	; jetzt Runden auf Bit hinter
	ADD	A,80H		; Mantisse
	JR	NC,MU_NOV	; kein Uebertrag?
	INC	(IX+TEMP+3)	; doch, naechstes Mantissenbyte
	JR	NZ,MU_NOV	; behandeln, jetzt auf Null pruefen
	INC	(IX+TEMP+4)	; da der INC-Befehl kein Carry liefert
	JR	NZ,MU_NOV
	INC	(IX+TEMP+5)
	JR	NZ,MU_NOV
	SCF			; Eins erzeugen
	RR	(IX+TEMP+5)	; bei Ueberlauf Mantisse durch
	RR	(IX+TEMP+4)	; Rechtsschieben wieder normalisieren
	RR	(IX+TEMP+3)
	INC	HL		; und Eponent korrigieren
MU_NOV:
	XOR	A		; A = 0
	CP	H		; Exponent pruefen
	LD	A,MAXEXPO	; A vorbesetzen
	JR	NZ,MU_OVR	; groesser Null: Ueberlauf behandeln
	CP	L		; oder genau maxexpo erreicht?
	JR	NZ,MU_NUE	; nein, kein Ueberlauf
MU_OVR:
	LD	L,MAXEXPO	; Ueberlauf: Exponent = maxexpo
	XOR	A		; 	     Mantisse = Null
	LD	(IX+TEMP+5),A
	LD	(IX+TEMP+4),A
	LD	(IX+TEMP+3),A
	JR	MU_DEN
MU_NUE:
	XOR	A		; A = 0
	CP	L		; Exponent ist Null?
	JR	Z,MU_DEN	; ja, Ergebnis ist denormalisiert
	SLA	(IX+TEMP+3)	; nein, fuehrendes Mantissenbit
	RL	(IX+TEMP+4)	; rausschieben
	RL	(IX+TEMP+5)
MU_DEN:
	SLA	C		; Vorzeichen in Carry schieben
	LD	B,L		; Exponent einsetzen
	LD	C,(IX+TEMP+5)
	LD	D,(IX+TEMP+4)
	LD	E,(IX+TEMP+3)
	RR	B		; und Vorzeichen einschieben
	RR	C
	RR	D		; Form: SEEE EEEE EFFF FFFF ... FFFF
	RR	E
MU_RES:
	POP	HL		; lokale Variable deallozieren
	POP	HL
	POP	HL
	POP	IY		; Register restaurieren
	POP	IX
	POP	AF
	POP	HL
	LD	(F_HL),HL	; Parameter vom Stack deallozieren
	EX	(SP),HL
	LD	SP,(F_STACK)
	PUSH	HL
	LD	HL,(F_HL)
	RET			; und return
MU_ZERO:
	XOR	A		; Ergebnis ist Null
	LD	B,A
	LD	C,A
	LD	D,A
	LD	E,A
	JR	MU_RES
MU_UNT:
	LD	A,L		; Exponent in A
	NEG			; negieren fuer Schleifenzaehler
	CP	24		; totaler Ueberlauf?
	JR	NC,MU_ZERO	; ja, dann ist Ergebnis Null
	LD	B,A		; in B fuer Loop
MU_SHR:
	SRL	(IX+TEMP+5)	; Mantisse denormalisieren
	RR	(IX+TEMP+4)	; bis Exponent Null ist
	RR	(IX+TEMP+3)
	DJNZ	MU_SHR
	LD	L,B		; Exponent in Register L = B = 0
	JP	MU_DEN		; denormalisiertes Ergebnis erzeugen

; *************************************************
; * Fliesskomma-Division in Single-Precision
; * Parameter: Operand 1 und Operand 2 ueber Stack
; * Ergebnis: in BC-DE: MSB in B, LSB in E
; *

F_DIV:
	PUSH	HL		; alten Basepointer retten
	LD	(F_STACK),SP	; aktuellen Stackpointer abspeichern
	LD	HL,(F_STACK)	; und in HL laden (= Basepointer)
	PUSH	AF		; benoetigte Register retten
	PUSH	IX
	PUSH	IY
	LD	BC,OP1
	ADD	HL,BC
	PUSH	HL
	EX	(SP),IX		; IX zeigt auf Operand 1
				; 2 Dummy-Byte auf Stack fuer lokale
	LD	BC,OPSIZE	; Variable bleiben stehen
	ADD	HL,BC
	PUSH	HL
	EX	(SP),IY		; IY zeigt auf Operand 2
	PUSH	HL		; insgesamt 6 Byte fuer lokale Variable
	ADD	HL,BC		; HL zeigt jetzt hinter die Operanden!
	LD	(F_STACK),HL
	LD	A,(IX+3)	; Ergebnisvorzeichen bestimmen
	XOR	(IY+3)
	LD	C,A		; Vorzeichen in C Bit 7 merken
	LD	H,0		; Exponent 1 laden
	LD	L,(IX+3)
	LD	A,(IX+2)
	RES	7,(IX+2)	; implizite Null vorbesetzen
	SLA	A		; Exponent unterstes Bit in Carry
	RL	L		; und in E einschieben
	JR	Z,DV_DN1	; falls Null, dann Op1 denormalisieren
	SET	7,(IX+2)	; implizite Eins erzeugen
	DEC	HL		; Bias kompensieren
DV_DN1:
	LD	D,0		; Exponent 2 in DE aufbauen
	LD	E,(IY+3)
	LD	A,(IY+2)
	LD	(IY+3),A	; Mantisse um 8 Bit verschieben
	RES	7,(IY+3)	; implizite Null vorbesetzen
	SLA	A
	RL	E
	JR	Z,DV_DN2	; gleich Null, dann Op2 denormalisieren
	SET	7,(IY+3)	; implizite Eins erzeugen
	DEC	DE		; Bias kompensieren
DV_DN2:
	LD	A,(IY+1)	; jetzt restliche Bytes verschieben
	LD	(IY+2),A
	LD	A,(IY+0)
	LD	(IY+1),A
	XOR	A		; A = 0
	LD	(IY+0),A	; Form: FFFF ... FFFF 0000 0000
	SRL	(IY+3)
	RR	(IY+2)
	RR	(IY+1)
	RR	(IY+0)		; Form: 0FFF ... FFFF F000 0000
	JR	NZ,DV_NZ1	; Mantisse 2 auf Null pruefen
	CP	(IY+1)
	JR	NZ,DV_NZ1
	CP	(IY+2)
	JR	NZ,DV_NZ1
	CP	(IY+3)
	JR	NZ,DV_NZ1
	JP	MU_OVR		; Bei Division durch Null: unendlich
DV_NZ1:
	XOR	A		; Carry-Flag loeschen
	SBC	HL,DE		; Exponenten subtrahieren
	LD	DE,BIAS		; Bias addieren
	ADD	HL,DE
	BIT	7,H		; Exponent positiv?
	JR	Z,DV_NOZ
	LD	A,L		; Exponent kleiner als -24?
	JR	NC,DV_NOZ
	JP	MU_ZERO		; ja, dann ist das Ergebnis Null
DV_NOZ:
	PUSH	BC		; Vorzeichen retten
	LD	DE,25		; Exponent um 25 erhoehen
	ADD	HL,DE		; jetzt ist er sicher groesser als Null
	XOR	A		; A = 0
	LD	B,(IX+2)	; Divident in Register kopieren
	LD	C,(IX+1)
	LD	D,(IX+0)
	LD	E,A		; die untersten Bits sind Null
	CP	D		; ist Dividend Null?
	JR	NZ,DV_NZ2
	CP	C
	JR	NZ,DV_NZ2
	CP	B
	JR	NZ,DV_NZ2
	POP	BC		; Stack bereinigen (Vorzeichen laden)
	JP	MU_ZERO		; und Null als Ergebnis ausgeben
DV_NZ2:
	LD	(IX+TEMP+5),A	; Ergebnis vorbesetzen
	LD	(IX+TEMP+4),A
	LD	(IX+TEMP+3),A
	LD	(IX+TEMP+2),A
DV_NLP:
	BIT	6,(IY+3)	; ist der Divisor normalisiert
	JR	NZ,DV_NOR	; ja, -->
	INC	HL		; nein, Exponent erhoehen
	SLA	(IY+0)		; Divisor verschieben bis in
	RL	(IY+1)		; Form 01FF ...
	RL	(IY+2)
	RL	(IY+3)
	JR	DV_NLP
DV_NOR:
	SRL	B
	RR	C
	RR	D
	RR	E		; Form: 0FFF ... FFFF F000 0000
DV_LOP:
	LD	(IX+3),B	; Dividend zwischenspeichern
	LD	(IX+2),C	; die Speicherplaetze von Op1
	LD	(IX+1),D	; stehen zur Verfuegung, da wir OP1
	LD	(IX+0),E	; in die Register BC-DE kopiert haben
	LD	A,E		; jetzt Divisor abziehen
	SUB	(IY+0)
	LD	E,A
	LD	A,D
	SBC	A,(IY+1)
	LD	D,A
	LD	A,C
	SBC	A,(IY+2)
	LD	C,A
	LD	A,B
	SBC	A,(IY+3)
	LD	B,A
	JR	NC,DV_ONE	; kein Carry: Divisor passt
	LD	E,(IX+0)	; zurueckkopieren
	LD	D,(IX+1)	; Carry bleibt dabei erhalten
	LD	C,(IX+2)
	LD	B,(IX+3)
DV_ONE:
	CCF			; Carry-Flag umkehren
	RL	(IX+TEMP+2)	; Ergebnis aufbauen
	RL	(IX+TEMP+3)
	RL	(IX+TEMP+4)
	RL	(IX+TEMP+5)
	SLA	E		; Dividend verschieben
	RL	D
	RL	C
	RL	B
	DEC	HL		; Exponent erniedrigen
	XOR	A		; A = 0
	CP	L		; Exponent = Null ?
	JR	NZ,DV_DIV
	CP	H
	JR	Z,DV_DEN	; falls Null, dann denormalisiert
DV_DIV:
	BIT	0,(IX+TEMP+5)	; fuerende Eins in Ergebnis-Mantisse?
	JR	Z,DV_LOP	; nein, weiter rechnen
DV_DEN:
	LD	B,(IX+TEMP+5)	; hoechstes Bit merken
	LD	A,(IX+TEMP+4)
	LD	(IX+TEMP+5),A	; Mantisse in Form
	LD	A,(IX+TEMP+3)	; FFFF ... FFFF 0000 0000
	LD	(IX+TEMP+4),A
	LD	A,(IX+TEMP+2)
	LD	(IX+TEMP+3),A
	RR	B		; hoechstes Bit einschieben
	RR	(IX+TEMP+5)
	RR	(IX+TEMP+4)
	RR	(IX+TEMP+3)	; Form: FFFF ... FFFF F000 0000
	RR	(IX+TEMP+2)
	POP	BC		; Vorzeichen wieder laden
	XOR	A		; A = 0
	CP	(IX+TEMP+5)	; Mantisse ist Null?
	JR	NZ,DV_NZ3
	CP	(IX+TEMP+4)
	JR	NZ,DV_NZ3
	CP	(IX+TEMP+3)
	JR	NZ,DV_NZ3
	CP	(IX+TEMP+2)
	JP	Z,MU_ZERO	; dann ist Ergebnis auch Null
DV_NZ3:
	JP	MU_RND		; sonst weiter wie bei Multiplikation

F_STACK:	
	DEFS	2		; Hilfsspeicher fuer Stackpointer
F_HL:
	DEFS	2		; Hilfsspeicher fuer Basepointer HL

	END

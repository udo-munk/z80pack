	TITLE	'Z80-Disassembler'

;	Hardware-unabhaengiger, ROM-faehiger Z80-Disassembler
;
;	Die Adresse, ab der disassembliert serden soll, ist in der
;	16-Bit Speicherzelle DADR abzulegen. Danach kann eines der
;	Entrys DISSCR (Ausgabe eines Bildschirms) oder DISLIN
;	(Ausgabe einer Zeile) aufgerufen werden. Da die Folgeadressen
;	durch das Programm ebenfalls wieder in der Speicherzelle
;	DADR abgelegt werden, ist mehrfacher Aufruf ohne Laden
;	von neuen Adressen moeglich.
;	Zur Ausgabe muss ein Unterprogramm mit dem Namen PRTSTR
;	erstellt werden, dem in HL die Adresse eines Null-terminierten
;	Strings uebergeben wird.
;
;	27-JUN-89	Udo Munk

LPP	EQU	15		; Anzahl Zeilen/Bildschirm Ausgabe
MNEPOS	EQU	11H		; Offset des Mnemonics in Ausgabe-Workspace

	; Disassembliere einen Bildschirm voll
DISSCR:	LD	B,LPP		; einen Bildschirm mit LPP Zeilen
$DLP1:	PUSH	BC		; disassemblieren
	CALL	DISLIN
	POP	BC
	DJNZ	$DLP1
	RET

	; Disassembliere eine Zeile
DISLIN:	CALL	CLWO		; Workspace fuer eine Zeile Ausgabe loeschen
	LD	HL,WRKS		; Adresse der Ausgabe-Workspace -> HL
	LD	DE,(DADR)	; Disassemblier-Adresse -> DE
	CALL	PRBY		; Adresse in DE ausgeben
	INC	HL		; Blank ausgeben
	LD	(PRTMP),HL	; vordere Printposition retten
	LD	C,0		; Steuerflag loeschen
	DEC	DE		; laufende Adr.-1 -> DE
$DL13:	CALL	PRNB		; laufendes Byte ausgeben
	LD	A,(DE)		; laufendes Byte -> A
	LD	B,A		; und in B retten
	CP	0EDH		; Preafix ED ?
	JR	NZ,$DL14	
	SET	4,C		; ja, ED-Flag setzen
	JR	$DL13		; und naechstes Byte bearbeiten
$DL14:	CP	0FDH		; Preafix FD ?
	JR	NZ,$DL15
	SET	6,C		; ja, FD-Flag setzen
	JR	$DL16		; und Index-Flag setzen
$DL15:	CP	0DDH		; Preafix DD ?
	JR	NZ,$DL17
$DL16:	SET	5,C		; Index-Flag fuer IX/IY-Adressierung setzen
	LD	HL,(PRTMP)	; vordere Printposition -> HL
	JR	$DL13		; naechstes Byte bearbeiten
$DL17:	LD	HL,WRKS+MNEPOS	; HL auf Operator Position setzen

	; nach Praefix CB
CB:	LD	A,B		; Befehlsbyte aus B holen
	CP	0CBH		; Preafix CB ?
	JP	NZ,OHNE
	INC	DE		; ja, Pointer auf naechstes Byte setzen
	BIT	5,C		; IX/IY-Flag ?
	JR	Z,$DL18
	INC	DE		; ja, Pointer auf naechstes Byte setzen
$DL18:	LD	A,(DE)		; naechstes Byte -> A
	LD	B,A		; und in B retten
	PUSH	DE		; Disassemblieradr. retten
	LD	D,MNETAB > 8	; High-Byte Operatorentabelle -> D
	LD	E,0E8H		; DE = Pointer auf "SET"
	CP	0C0H		; SET ?
	JR	NC,$DL19
	LD	E,0E4H		; nein, DE = Pointer auf "RES"
	CP	80H		; RES ?
	JR	NC,$DL19
	LD	E,0E0H		; nein, DE = Pointer auf "BIT"
	CP	40H		; BIT ?
	JR	NC,$DL19
	AND	38H		; loesche Bits 0..2 und 6..7
	RRCA			; Division durch 2
	ADD	A,CBMTAB & 0FFH ; zur Basis der CB-Mnemonics addieren
	LD	E,A
	LD	D,CBMTAB > 8	; DE = Pointer auf CB-Mnemonic
$DL19:	CALL	TRBY		; Mnemonic ausgeben
	POP	DE		; akt. Disassemblieradr. wieder -> DE
	LD	A,B		; Byte wieder -> A
	BIT	5,C		; IX/IY-Flag ?
	JR	Z,$DL20
	DEC	DE		; eins zurueck bei IX/IY-Adressierung
$DL20:	DEC	DE		; Pointer wieder auf CB-Preafix
	CP	40H		; CB-Befehl < 40H ?
	JR	C,$DL21
	AND	38H		; nein, Bits 0..2 und 6..7 loeschen
	RRCA			; Division durch 8 -> 1. Operanden
	RRCA
	RRCA
	CALL	PRO1		; 1. Operanden ausgeben
	LD	A,B		; Byte wieder -> A
	SET	7,C		; Komma-Flag setzen
$DL21:	AND	7		; Bits 3..7 loeschen -> 2. Operanden
	SET	4,A		; Buchstaben-Flag setzen
	CALL	PRO1		; 2. Operanden ausgeben
	CALL	PRNB		; Befehlsbyte vorne ausgeben
	JP	INAD		; fertig, Adresse merken und Workspace ausgeben

	; ohne Preafix CB/ED
OHNE:	PUSH	DE		; Disassemblieradr. retten
	BIT	4,C		; ED-Flag ?
	JP	NZ,ED
	CP	40H		; nein, < 40H ?
	JR	C,$DL25
	CP	80H		; nein, > 80H ?
	JR	NC,$DL23
	LD	E,50H		; nein, DE = Pointer auf "LD"
	CP	76H		; HALT ?
	JR	NZ,$DL22
	LD	E,5CH		; nein, DE = Pointer auf "HALT"
$DL22:	JR	$DL26		; Mnemonic ausgeben
$DL23:	CP	0C0H		; > C0H ?
	JR	NC,$DL24
	AND	38H		; ja, Bits 0..2 und 6..7 loeschen
	RRCA			; Division durch 2 -> Operator
	LD	E,A		; Operator -> E
	JR	$DL26		; Mnemonic ausgeben
$DL24:	SUB	80H		; wenn > C0H, -80 -> Tabellenoperator
$DL25:	LD	E,A		; Operator -> E
	LD	D,CODTAB > 8	; High-Byte Operatortabelle -> D
	LD	A,(DE)		; LSB Mnemonic-Adresse -> A
	LD	E,A		; und nach E
$DL26:	LD	D,MNETAB > 8	; MSB Mnemonic-Adresse -> D
	CALL	TRBY		; Mnemonic ausgeben
	POP	DE		; akt. Disassemblieradr. wieder -> DE
	LD	A,B		; Befehlsbyte wieder -> A
	PUSH	DE		; Disassemblieradr. retten
	CP	40H		; Byte < 40 ?
	JR	C,$DL30
	CP	80H		; nein, > 80 ?
	JR	NC,$DL28
	CP	76H		; nein, HALT ?
	JR	NZ,$DL27
	LD	A,0FFH		; ja, leeren Operanden -> A
	JR	$DL31		; Operanden ausgeben
$DL27:	AND	38H		; loesche Bits 0..2 und 6..7
	RRCA			; Division durch 8 -> 1. Operanden
	RRCA
	RRCA
	SET	4,A		; Buchstabenflag setzen
	JR	$DL31		; Operanden ausgeben
$DL28:	CP	0C0H		; > C0 ?
	JR	NC,$DL29
	CP	90H		; > 90 ?
	JR	C,$DL51
	AND	0F8H		; ja, Register-Bits loeschen
	CP	98H		; "SBC" ?
	JR	Z,$DL51
	LD	A,B		; Byte wieder -> A
	AND	7		; nur Register Bits uebrig lassen
	SET	4,A		; Buchstaben-Flag setzen
	JR	$DL52
$DL51:	LD	A,17H		; ja, 17 = Register A ausgeben
	JR	$DL31		; Operanden ausgeben
$DL29:	SUB	80H		; wenn > C0, -80 -> Operandentabelle
$DL30:	LD	E,A		; LSB Operandentabelle -> E
	LD	D,OPETAB > 8	; MSB Operandentabelle -> D
	LD	A,(DE)		; 1. Operanden -> A
$DL31:	POP	DE		; akt. Disassemblieradr. wieder -> DE
	CALL	PRO1		; 1. Operanden ausgeben
	LD	A,B		; Befehlsbyte wieder -> A
	PUSH	DE		; akt. Disassemblieradr. retten
	CP	40H		; < 40 ?
	JR	C,$DL34
	CP	0C0H		; nein, < C0 ?
	JR	NC,$DL33
	CP	76H		; ja, HALT ?
	JR	NZ,$DL32
	LD	A,0FFH		; ja, wieder leeren Operanden -> A
	JR	$DL35		; Operanden ausgeben
$DL32:	AND	7		; loesche Bits 3..7, -> 2. Operanden
	SET	4,A		; Buchstabenflag setzen
	JR	$DL35		; Operanden ausgeben
$DL33:	SUB	80H		; wenn > C0 : 80 abziehen
$DL34:	ADD	A,80H		; LSB Operandentabelle -> A
	LD	E,A		; und -> E
	LD	D,OPETAB > 8	; MSB Operandentabelle -> D
	LD	A,(DE)		; 2. Operanden -> A
$DL35:	POP	DE		; akt. Disassemblieradr. wieder -> DE
	SET	7,C		; Komma-Flag setzen
	CALL	PRO1		; 2. Operanden ausgeben
	JP	INAD		; fertig, Adresse merken und Workspace ausgeben
	
	; nach Preafix ED
ED:	SUB	40H		; 40 vom 2. Byte subtrahieren
	JP	C,ERRO		; Fehler wenn carry
	CP	60H		; 2. Byte < A0 ?
	JR	NC,$DL36
	CP	40H		; ja, >= 60 ?
	JP	NC,ERRO		; ja, Fehler
	JR	$DL37		; nein, weiter
$DL36:	SUB	20H		; aus 60..7F wird 00..20
$DL37:	ADD	A,80H		; LSB Operatortabelle -> A
	LD	E,A		; und -> E
	LD	D,CODTAB > 8	; MSB Operatortabelle -> D
	LD	A,(DE)		; LSB Mnemonic-Adresse -> A
	CP	0FFH		; leer ?
	JP	Z,ERRO		; ja, Fehler
	LD	E,A		; nein, -> E
	LD	D,MNETAB > 8	; MSB Mnemonic-Adresse -> D
	CALL	TRBY		; Mnemonic ausgeben
	POP	DE		; Disassemblieradr. wieder -> DE
	LD	A,B		; Befehlsbyte wieder -> A
	CP	80H		; < 80 ?
	JP	NC,INAD		; nein, Workspace ausgeben und fertig
	PUSH	DE		; Disassemblieradr. retten
	SUB	40H		; LSB 1. Operanden in A
	LD	E,A		; und -> E
	LD	D,OP2TAB > 8	; MSB 2. Operanden -> D
	LD	A,(DE)		; 1. Operanden -> A
	POP	DE		; akt. Disassemblieradr. wieder -> DE
	CALL	PRO1		; 1. Operanden ausgeben
	LD	A,B		; Befehlsbyte wieder -> A
	CP	80H		; < 80 ?
	JP	NC,INAD		; ja, Workspace ausgeben und fertig
	PUSH	DE		; akt. Disassemblieradr. retten
	LD	E,A		; LSB Operandentabelle -> E
	LD	D,OP2TAB > 8	; MSB Operandentabelle -> D
	LD	A,(DE)		; 2. Operanden -> A
	SET	7,C		; Buchstabenflag setzen
$DL52:	POP	DE		; akt. Disassemblieradr. retten
	CALL	PRO1		; 2. Operanden ausgeben
	JP	INAD		; fertig, Adresse merken und Workspace ausgeben

	; Operand 1 ausgeben
PRO1:	CP	0FFH		; leere Operand ?
	RET	Z		; ja, fertig
	CP	17H		; nein, Register "A" ausgeben ?
	JR	NZ,$DL01
	LD	A,18H		; ja, umkodieren
$DL01:	CP	16H		; Register "(HL)" ausgeben ?
	JR	NZ,$DL02
	LD	A,0B4H		; ja, umkodieren
$DL02:	BIT	7,C		; 2. Operand ?
	JR	Z,$DL03
	LD	(HL),','	; ja, "," ausgeben
	INC	HL		; naechste Printposition -> HL
$DL03:	BIT	7,A		; "(...)" ?
	JR	Z,$DL04
	LD	(HL),'('	; ja, "(" ausgeben
	INC	HL		; naechste Printposition -> HL
$DL04:	BIT	4,A		; Buchstabe ?
	JR	NZ,PRBU		; ja, Buchstaben ausgeben
	BIT	6,A		; nein, Bitnummer/RST-Adresse ?
	JR	NZ,DIST		; ja, Distanz ausgeben
	BIT	5,A		; nein, Bitnummer ?
	JR	NZ,PRO2		; ja, RST ausgeben
	AND	7		; nein, Bits 3..7 loeschen
	CALL	PRCH		; Hexbyte ausgeben
	RET

	; RST ausgeben
PRO2:	PUSH	AF		; A retten
	AND	6		; loesche Bits 0 und 4..7
	RRCA			; Division durch 2
	CALL	PRCH		; oberes Nibble ausgeben
	POP	AF		; A wieder herstellen
	BIT	0,A		; RST x8 ?
	LD	A,'0'		; nein, "0" -> A
	JR	Z,$DL05
	LD	A,'8'		; ja, "8" -> A
$DL05:	LD	(HL),A		; "0" oder "8" ausgeben
	INC	HL		; naechste Printposition -> HL
	RET

	; Distanz ausgeben
DIST:	BIT	0,A		; relative Distanz ?
	JR	Z,PR_N		; nein, N ausgeben
	CALL	PRNB		; Byte vorne ausgeben
	PUSH	DE		; akt. Disassemblieradr. retten
	LD	A,(DE)		; Distanzbyte -> A
	INC	DE		; Disassemblieradr. erhoehen
	RLCA			; Bit 7 Distanzbyte -> carry
	RRCA
	JR	NC,$DL06	; Vorwaertsprung
	SET	0,C		; Flag fuer Rueckwaertssprung setzen
$DL06:	ADD	A,E		; Distanz zu PC addieren
	LD	E,A
	BIT	0,C		; Flag testen
	JR	NC,$DL07	; kein Ueberlauf
	JR	NZ,$DL08	; Rueckwaertssprung
	INC	D		; MSB PC erhoehen
	JR	$DL08		; Zieladresse ausgeben
$DL07:	JR	Z,$DL08		; bei Vorwaertssprung Zieladresse ausgeben
	DEC	D		; sonst MSB PC erniedrigen
$DL08:	CALL	PRBY		; Zieladresse in DE ausgeben
	POP	DE		; akt. Disassemblieradresse wieder -> DE
	RET

	; N ausgeben
PR_N:	PUSH	AF		; A retten
	BIT	1,A		; N ?
	JR	Z,PRNN		; nein, NN ausgeben
	CALL	PRVH		; ja, Byte vorne und hinten ausgeben
	JR	$DL12		; ")" bearbeiten

	; NN ausgeben
PRNN:	CALL	PRNB		; Byte vorne ausgeben
	CALL	PRVH		; Byte vorne und hinten ausgeben
	DEC	DE		; DE -> LSB von NN
	CALL	PRBH		; Byte hinten ausgeben
	INC	DE		; akt. Disassemblieradr. wieder -> DE
	JR	$DL12		; ")" bearbeiten

	; Buchstaben ausgeben
PRBU:	PUSH	AF		; A retten
	PUSH	BC		; Flags in C retten
	PUSH	DE		; akt. Disassemblieradr. retten
	LD	B,1		; Anzahl = 1
	LD	DE,REGTAB	; DE zeigt auf die Register-Namen
	BIT	5,A		; 2 Buchstaben ?
	JR	Z,$DL09
	INC	B		; ja, Anzahl erhoehen
$DL09:	BIT	6,A		; Sprungbedingung ?
	JR	Z,$DL10
	LD	DE,SPRTAB	; ja, DE zeigt auf Condition-Namen
$DL10:	RES	7,A		; Klammer-Bit loeschen
	CP	34H		; "(HL)" ?
	JR	NZ,$DL11
	BIT	5,C		; ja, Indexregister ?
	JR	Z,$DL11
	LD	A,0AH		; ja, A -> IX-Register
	BIT	6,C		; IY-Register ?
	JR	Z,$DL11
	LD	A,0CH		; ja, A -> IY-Register
$DL11:	AND	0FH		; loesche oberes Nibble
	ADD	A,E		; und addiere zur Basis in DE
	LD	E,A
$DL50:	LD	A,(DE)		; Zeichen -> A
	LD	(HL),A		; Zeichen ausgeben
	INC	DE		; Tabellen-Adresse erhoehen
	INC	HL		; naechste Printposition
	DJNZ	$DL50		; naechstes Zeichen
	POP	DE		; Register wieder herstellen
	POP	BC
	POP	AF
	PUSH	AF		; A retten
	CP	0B4H		; "(HL)" ?
	JR	NZ,$DL12
	BIT	5,C		; nein, Indexregister ?
	JR	Z,$DL12
	LD	A,(DE)		; ja, Befehlsbyte nach DD/FD -> A
	CP	0E9H		; "JP (IX/IY)" ?
	JR	Z,$DL12
	LD	(HL),'+'	; nein, "+" ausgeben
	INC	HL		; naechste Printposition
	CALL	PRVH		; Offset ausgeben
$DL12:	POP	AF		; A wieder herstellen
	BIT	7,A		; "()" ?
	RET	Z		; nein, fertig
	LD	(HL),')'	; ja, ")" ausgeben
	INC	HL		; naechste Printposition
	RET

	; Error
ERRO:	LD	DE,CBMTAB+24	; Pointer auf "????" -> DE
	CALL	TRBY		; als Mnemonic ausgeben
	POP	DE		; akt. Disassemblieradr. vom Stack holen
	
	; Disassemblier-Adresse erhoehen und merken
INAD:	INC	DE
	LD	(DADR),DE

	; Workspace ausgeben
PRWO:	PUSH	AF		; Register retten
	PUSH	BC
	PUSH	DE
	PUSH	HL
	LD	HL,WRKS		; Adresse Workspace -> HL
	CALL	PRTSTR		; Workspace aufs Terminal ausgeben
	LD	HL,NL		; Adresse Newline-String -> HL
	CALL	PRTSTR		; Newline ausgeben
	POP	HL		; Register wieder herstellen
	POP	DE
	POP	BC
	POP	AF
	RET
	
	; Workspace loeschen
CLWO:	LD	HL,WRKS		; Workspace mit Space fuellen
	LD	DE,WRKS+1	; und mit Null terminieren
	LD	(HL),32
	LD	BC,32
	LDIR
	XOR	A
	LD	(DE),A
	RET

	; 4 Bytes transferieren
TRBY:	PUSH	BC		; BC retten
	LD	BC,4		; 4 Bytes 
	EX	DE,HL		; DE=Printposition, HL=Mnemonic
	LDIR			; Bytes transferieren
	EX	DE,HL		; HL ist wieder Printposition
	POP	BC		; BC wieder herstellen
	INC	HL		; Leerzeichen
	RET

	; Byte vorne und hinten ausgeben
PRVH:	CALL	PRNB		; Byte vorne ausgeben

	; Byte hinten ausgeben
PRBH:	PUSH	AF		; A retten
	LD	A,(DE)		; Byte -> A
	CALL	PRAK		; A ausgeben
	POP	AF		; A wieder herstellen
	RET

	; Byte vorne ausgeben
PRNB:	PUSH	AF		; A retten
	INC	DE		; DE auf naechstes Byte setzen
	PUSH	HL		; akt. Printposition retten
	LD	HL,(PRTMP)	; vordere Printposition -> HL
	LD	A,(DE)		; Byte -> A
	CALL	PRAK		; A ausgeben
	INC HL			; Leerzeichen
	LD	(PRTMP),HL	; vordere Printposition retten
	POP	HL		; akt. Printposition wieder -> HL
	POP	AF		; A wieder herstellen
	RET

	; DE ausgeben
PRBY:	LD	A,D		; MSB -> A
	CALL	PRAK		; A ausgeben
	LD	A,E		; LSB -> A

	; A ausgeben
PRAK:	PUSH	AF		; A retten
	RRCA			; oberes Nibble ins untere schieben
	RRCA
	RRCA
	RRCA
	CALL	PRCH		; oberes Nibble ausgeben
	POP	AF		; A wieder herstellen
	CALL	PRCH		; unteres Nibble ausgeben
	RET

	; unteres Nibble in ASCII-Hex umwandeln und in Workspace schreiben
PRCH:	AND	0FH
	ADD	A,90H
	DAA
	ADC	A,40H
	DAA
	LD	(HL),A		; ASCII-Ziffer in Workspace schreiben
	INC	HL		; Printposition erhoehen
	RET

	; Die hier folgenden Tabellen muessen am Anfang einer Page
	; beginnen, und die Reihenfolge der Tabellen darf auf keinen
	; Fall geaendert werden, weil das LSB der Tabellenadressen
	; durch arithmetische Operationen mit den Op-Codes berechnet
	; wird !!!

	DEFS	256 - ($ & 0FFH)

MNETAB:		; Tabelle mit den Z80-Mnemonics
	DEFM	'ADD ADC '
	DEFM	'SUB SBC '
	DEFM	'AND XOR '
	DEFM	'OR  CP  '
	DEFM	'JR  NOP '
	DEFM	'DJNZEX  '
	DEFM	'RLCARLA '
	DEFM	'DAA SCF '
	DEFM	'RRCARRA '
	DEFM	'CPL CCF '
	DEFM	'LD  INC '
	DEFM	'DEC HALT'
	DEFM	'RET POP '
	DEFM	'JP  OUT '
	DEFM	'EX  DI  '
	DEFM	'CALLPUSH'
	DEFM	'RST EXX '
	DEFM	'IN  EX  '
	DEFM	'EI  LDI '
	DEFM	'LDIRINI '
	DEFM	'INIROUTI'
	DEFM	'OTIRNEG '
	DEFM	'RETNRRD '
	DEFM	'LDD LDDR'
	DEFM	'CPD CPDR'
	DEFM	'IND INDR'
	DEFM	'OTDROUTD'
	DEFM	'RETIRLD '
	DEFM	'BIT RES '
	DEFM	'SET ????'
	DEFM	'CPI CPIR'
	DEFM	'IM  ----'

CODTAB:		; LSB-Adressen der Mnemonics in MNETAB fuer
		; Befehle 00..3F ohne Preafix ED/CB

	DEFB	024H,050H,050H,054H,054H,058H,050H,030H	; NOP  LD   LD   INC  INC  DEC  LD   RLCA
	DEFB	070H,000H,050H,058H,054H,058H,050H,040H	; EX   ADD  LD   DEC  INC  DEC  LD   RRCA
	DEFB	028H,050H,050H,054H,054H,058H,050H,034H	; DJNZ LD   LD   INC  INC  DEC  LD   RLA
	DEFB	020H,000H,050H,058H,054H,058H,050H,044H	; JR   ADD  LD   DEC  INC  DEC  LD   RRA
	DEFB	020H,050H,050H,054H,054H,058H,050H,038H	; JR   LD   LD   INC  INC  DEC  LD   DAA
	DEFB	020H,000H,050H,058H,054H,058H,050H,048H	; JR   ADD  LD   DEC  INC  DEC  LD   CPL
	DEFB	020H,050H,050H,054H,054H,058H,050H,03CH	; JR   LD   LD   INC  INC  DEC  LD   SCF
	DEFB	020H,000H,050H,058H,054H,058H,050H,04CH	; JR   ADD  LD   DEC  INC  DEC  LD   CCF

		; LSB-Adressen der Mnemonics in MNETAB fuer
		; Befehle C0..FF ohne Preafix ED/CB

	DEFB	060H,064H,068H,068H,078H,07CH,000H,080H	; RET  POP  JP   JP   CALL PUSH ADD  RET
	DEFB	060H,060H,068H,0F1H,078H,078H,004H,080H	; RET  RET  JP   (CB) CALL CALL ADC  RST
	DEFB	060H,064H,068H,06CH,078H,07CH,008H,080H	; RET  POP  JP   OUT  CALL PUSH SUB  RST
	DEFB	060H,084H,068H,088H,078H,0F0H,00CH,080H	; RET  EXX  JP   IN   CALL (DD) SBC  RST
	DEFB	060H,064H,068H,070H,078H,07CH,010H,080H	; RET  POP  JP   EX   CALL PUSH AND  RST
	DEFB	060H,068H,068H,02CH,078H,0F2H,014H,080H	; RET  JP   JP   EX   CALL (ED) XOR  RST
	DEFB	060H,064H,068H,074H,078H,07CH,018H,080H	; RET  POP  JP   DI   CALL PUSH OR   RST
	DEFB	060H,050H,068H,090H,078H,0F8H,01CH,080H	; RET  LD   JP   EI   CALL (FD) CP   RST

		; LSB-Adressen der Mnemonics in MNETAB fuer
		; Befehle 40..7F mit Preafix ED

	DEFB	088H,06CH,00CH,050H,0ACH,0B0H,0F8H,050H	; IN   OUT  SBC  LD   NEG  RETN IM   LD
	DEFB	088H,06CH,004H,050H,0FFH,0D8H,0FFH,050H	; IN   OUT  ADC  LD        RETI      LD
	DEFB	088H,06CH,00CH,050H,0FFH,0FFH,0F8H,050H	; IN   OUT  SBC  LD             IM   LD
	DEFB	088H,06CH,004H,050H,0FFH,0FFH,0F8H,050H	; IN   OUT  ADC  LD             IM   LD
	DEFB	088H,06CH,00CH,0FFH,0FFH,0FFH,0FFH,0B4H	; IN   OUT  SBC                      RRD
	DEFB	088H,06CH,004H,0FFH,0FFH,0FFH,0FFH,0DCH	; IN   OUT  ADC                      RLD
	DEFB	0FFH,0FFH,00CH,050H,0FFH,0FFH,0FFH,0FFH	;           SBC  LD
	DEFB	088H,06CH,004H,050H,0FFH,0FFH,0FFH,0FFH	; IN   OUT  ADC  LD

		; LSB-Adressen der Mnemonics in MNETAB fuer
		; Befehle A0..BF mit Praefix ED

	DEFB	094H,0F0H,09CH,0A4H,0FFH,0FFH,0FFH,0FFH	; LDI  CPI  INI  OUTI
	DEFB	0B8H,0C0H,0C8H,0D4H,0FFH,0FFH,0FFH,0FFH	; LDD  CPD  IND  OUTD
	DEFB	098H,0F4H,0A0H,0A8H,0FFH,0FFH,0FFH,0FFH	; LDIR CPIR INIR OTIR
	DEFB	0BCH,0C4H,0CCH,0D0H,0FFH,0FFH,0FFH,0FFH	; LDDR CPDR INDR OTDR

SPRTAB:		; Tabelle der Sprungbedingungen

	DEFM	'NZNCPOPEPM'
	DEFB	0FFH,0FFH,0FFH,0FFH,0FFH,0FFH

REGTAB:		; Tabelle der Register

	DEFM	'BCDEHLSPAFIXIYIR'

OPETAB:		; Tabelle der Operanden:
		;	Bit 7: Zahl/Buchstabe
		;	Bit 6: einfach/doppelt
		;	Bit 5: Register/Sprungbedingung
		;	Bit 4: ohne/mit Klammer
		;	Bit 0..3: Offset in der Tabelle der Registernamen

		; Befehle 00..3F ohne Preafix ED/CB
		; 1. Operand

	DEFB	0FFH,030H,0B0H,030H,010H,010H,010H,0FFH	; -    BC   (BC) BC   B    B    B    -
	DEFB	038H,034H,017H,030H,011H,011H,011H,0FFH	; AF   HL   A    BC   C    C    C    -
	DEFB	041H,032H,0B2H,032H,012H,012H,012H,0FFH	; DIS  DE   (DE) DE   D    D    D    -
	DEFB	041H,034H,017H,032H,013H,013H,013H,0FFH	; DIS  HL   A    DE   E    E    E    -
	DEFB	070H,034H,0C4H,034H,014H,014H,014H,0FFH	; NZ   HL   (NN) HL   H    H    H    -
	DEFB	051H,034H,034H,034H,015H,015H,015H,0FFH	; Z    HL   HL   HL   L    L    L    -
	DEFB	072H,036H,0C4H,036H,016H,016H,016H,0FFH	; NC   SP   (NN) SP   (HL) (HL) (HL) -
	DEFB	011H,034H,017H,036H,017H,017H,017H,0FFH	; C    HL   A    SP   A    A    A    -

		; Befehle C0..FF ohne Preafix ED/CB
		; 1. Operand

	DEFB	070H,030H,070H,044H,070H,030H,017H,020H	; NZ   BC   NZ   NN   NZ   BC   A    00
	DEFB	051H,0FFH,051H,0F1H,051H,044H,017H,021H	; Z    -    Z    *CB  Z    NN   A    08
	DEFB	072H,032H,072H,0C2H,072H,032H,042H,022H	; NC   DE   NC   (N)  NC   DE   N    10
	DEFB	053H,0FFH,053H,017H,053H,0F2H,017H,023H	; C    -    C    A    C    *DD  A    18
	DEFB	074H,034H,074H,0B6H,074H,034H,042H,024H	; PO   HL   PO   (SP) PO   HL   N    20
	DEFB	076H,016H,076H,032H,076H,0F4H,042H,025H	; PE   (HL) PE   DE   PE   *ED  N    28
	DEFB	058H,038H,058H,0FFH,058H,038H,042H,026H	; P    AF   P    -    P    AF   N    30
	DEFB	059H,036H,059H,0FFH,059H,0F8H,042H,027H	; M    SP   M    -    M    *FD  N    38

		; Befehle 00..3F ohne Preafix ED/CB
		; 2. Operand

	DEFB	0FFH,044H,017H,0FFH,0FFH,0FFH,042H,0FFH	; -    NN   A    -    -    -    N    -
	DEFB	038H,030H,0B0H,0FFH,0FFH,0FFH,042H,0FFH	; AF   BC   (BC) -    -    -    N    -
	DEFB	0FFH,044H,017H,0FFH,0FFH,0FFH,042H,0FFH	; -    NN   A    -    -    -    N    -
	DEFB	0FFH,032H,0B2H,0FFH,0FFH,0FFH,042H,0FFH	; -    DE   (DE) -    -    -    N    -
	DEFB	041H,044H,034H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  NN   HL   -    -    -    N    -
	DEFB	041H,034H,0C4H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  HL   (NN) -    -    -    N    -
	DEFB	041H,044H,017H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  NN   A    -    -    -    N    -
	DEFB	041H,036H,0C4H,0FFH,0FFH,0FFH,042H,0FFH	; DIS  SP   (NN) -    -    -    N    -

		; Befehle C0..FF ohne Praefix ED/CB
		; 2. Operand

	DEFB	0FFH,0FFH,044H,0FFH,044H,0FFH,042H,0FFH	; -    -    NN   -    NN   -    N    -
	DEFB	0FFH,0FFH,044H,0FFH,044H,0FFH,042H,0FFH	; -    -    NN   *CB  NN   -    N    -
	DEFB	0FFH,0FFH,044H,017H,044H,0FFH,0FFH,0FFH	; -    -    NN   A    NN   -    -    -
	DEFB	0FFH,0FFH,044H,0C2H,044H,0FFH,042H,0FFH	; -    -    NN   (N)  NN   *DD  N    -
	DEFB	0FFH,0FFH,044H,034H,044H,0FFH,0FFH,0FFH	; -    -    NN   HL   NN   -    -    -
	DEFB	0FFH,0FFH,044H,034H,044H,0FFH,0FFH,0FFH	; -    -    NN   HL   NN   *ED  -    -
	DEFB	0FFH,0FFH,044H,0FFH,044H,0FFH,0FFH,0FFH	; -    -    NN   -    NN   -    -    -
	DEFB	0FFH,034H,044H,0FFH,044H,0FFH,0FFH,0FFH	; -    HL   NN   -    NN   *FD  -    -

OP2TAB:		; Befehle 40..7F mit Praefix ED
		; 1. Operand

	DEFB	010H,091H,034H,0C4H,0FFH,0FFH,000H,01EH	; B    (C)  HL   (NN) -    -    0    I
	DEFB	011H,091H,034H,030H,0FFH,0FFH,0FFH,01FH	; C    (C)  HL   BC   -    -    -    R
	DEFB	012H,091H,034H,0C4H,0FFH,0FFH,001H,017H	; D    (C)  HL   (NN) -    -    1    A
	DEFB	013H,091H,034H,032H,0FFH,0FFH,002H,017H	; E    (C)  HL   DE   -    -    2    A
	DEFB	014H,091H,034H,0C4H,0FFH,0FFH,076H,0FFH	; H    (C)  HL   -    -    -    -    -
	DEFB	015H,091H,034H,0FFH,0FFH,0FFH,0FFH,0FFH	; L    (C)  HL   -    -    -    -    -
	DEFB	0FFH,0FFH,034H,0C4H,0FFH,0FFH,0FFH,0FFH	; -    -    HL   (NN) -    -    -    -
	DEFB	017H,091H,034H,036H,0FFH,0FFH,0FFH,0FFH	; A    (C)  HL   SP   -    -    -    -

		; Befehle 40..7F mit Preafix ED
		; 2. Operand

	DEFB	091H,010H,030H,030H,0FFH,0FFH,0FFH,017H	; (C)  B    BC   BC   -    -    -    A
	DEFB	091H,011H,030H,0C4H,0FFH,0FFH,0FFH,017H	; (C)  C    BC   (NN) -    -    -    A
	DEFB	091H,012H,032H,032H,0FFH,0FFH,0FFH,01EH	; (C)  D    DE   DE   -    -    -    I
	DEFB	091H,013H,032H,0C4H,0FFH,0FFH,0FFH,01FH	; (C)  E    DE   (NN) -    -    -    R
	DEFB	091H,014H,034H,034H,0FFH,0FFH,0FFH,0FFH	; (C)  H    HL   -    -    -    -    -
	DEFB	091H,015H,034H,0FFH,0FFH,0FFH,0FFH,0FFH	; (C)  L    HL   -    -    -    -    -
	DEFB	0FFH,0FFH,036H,036H,0FFH,0FFH,0FFH,0FFH	; -    -    SP   SP   -    -    -    -
	DEFB	091H,017H,036H,0C4H,0FFH,0FFH,0FFH,0FFH	; (C)  A    SP   (NN) -    -    -    -

CBMTAB:		; Tabelle der Mnemonics mit Praefix CB
	DEFM	'RLC RRC '
	DEFM	'RL  RR  '
	DEFM	'SLA SRA '
	DEFM	'????SRL '

NL:		; Null-terminiertes Newline fuers Terminal
	DEFB	10,13,0

WRKS:	DEFS	34		; Workspace zur Aufbereitung einer Ausgabezeile
PRTMP:	DEFS	2		; temoraerer Speicher fuer Printposition
DADR:	DEFS	2		; Disassemblier-Adresse

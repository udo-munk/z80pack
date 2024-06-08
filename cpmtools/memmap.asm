;***********************************************************************
;      MICROCOSM MEMORY MAP DISPLAY PROGRAM VERSION 1.0  (C) 1979
;***********************************************************************
;			(revised 5/5/80)
;
; COPYRIGHT (C) 1979 ,BY MICROCOSM.ALL RIGHTS RESERVED.NO PART MAY BE
; REPRODUCED,TRANSMITTED,TRANSCRIBED ,STORED IN RETRIEVAL SYSTEM,  OR
; TRANSLATED INTO ANY LANGUAGE OR  COMPUTER LANGUAGE, IN ANY FORM  OR
; BY ANY MEANS,ELECTRONIC,MECHANICAL,MAGNETIC,OPTICAL,CHEMICAL,MANUAL
; OR  OTHERWISE,  WITHOUT THE PRIOR WRITTEN  PERMISSION OF MICROCOSM,
; 3055 WACO ST.,SIMI VALLEY,CALIFORNIA 93063.
;
;                           DISCLAIMER
;                           ==========
;
; MICROCOSM  MAKES NO REPRESENTATION OR WARRANTIES WITH RESPECT TO THE
; CONTENTS HEROF AND SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES  OF
; MERCHANTABILITY  OR  FITNESS FOR ANY PARTICULAR PURPOSE.   FURTHER ,
; MICROCOSM  RESERVES THE  RIGHT TO  REVISE AND TO   MAKE CHANGES FROM
; TIME TO  TIME IN THE  CONTENT HEREOF WITHOUT OBLIGATION OF MICROCOSM
; TO NOTIFY ANY PERSON OF SUCH REVISION OR CHANGES.
;
;************************************************************************
;
;05/05/80 MODIFIED TO WAIT FOR RETURN TO START SO
;	  DISKS CAN BE REMOVED. (THIS IS TO PREVENT
;	  POSSIBILITY OF DISK WRITES IN MEMORY-MAPPED
;	  DISK CONTROLLER SYSTEMS). KEITH PETERSEN, W8SDZ
;
;
	ORG	00100H
;
;
;
BDOS	EQU	5	;BDOS ENTRY
CHIF	EQU	1	;CP/M INPUT CHARACTER FUNCTION
CHRF	EQU	2	;CP/M PRINT CHARACTER FUNCTION
PRNF	EQU	9	;CP/M PRINT CHARACTER STRING FUNCTION
CR	EQU	00DH	;CARRIAGE-RETURN CHARACTER
LF	EQU	00AH	;LINE-FEED CHARACTER
;
;
;
MAP:	LXI	H,0	;SET-UP STACK
	DAD	SP	;ENTRY STACK POINTER FROM CP/M CCP
	SHLD	OLDSP	;SAVE IT
	LXI	SP,STACK	;SET STACK TO LOCAL STACK AREA
GETIT:	MVI	C,PRNF	;OUTPUT "PRESS RETURN TO START"
	LXI	D,STRMSG
	CALL	BDOS
	MVI	C,CHIF	;GET CHARACTER
	CALL	BDOS
	CPI	CR	;CARRIAGE RETURN?
	JNZ	GETIT	;NOPE, TRY AGAIN
	CALL	CRLF	;KEEP IT NEAT
	MVI	C,PRNF	;OUTPUT "BLOCK LOCATION"
	LXI	D,BLKMSG
	CALL	BDOS
	LXI	H,0	;POINT TO BLOCK 0
	MVI	B,1	;"ITEMS TO A LINE" COUNTER
MAP1:	MVI	E,'M'	;INDICATE "M" FOR RAM
	MOV	A,M	;GET FIRST BYTE OF BLOCK
	CMA		;INVERT IT
	MOV	M,A	;TRY TO MODIFY MEMORY
	CMP	M	;WAS MEMORY MODIFIED?
	CMA		;RESTORE A REG. TO INITIALLY READ VALUE
	MOV	M,A	;RESTORE MEMORY
	JNZ	MAP2	;SKIP,IF NOT RAM
	CMP	M	;TEST IF RAM FOR SURE
	JZ	PRINT	;YUP,IT'S RAM
MAP2:	MVI	E,'P'	;INDICATE "P" FOR PROM
MAP3:	MVI	A,0FFH	;ABSENCE OF MEMORY IS ALL ONE'S
	CMP	M	;TEST FOR NO MEMORY
	JNZ	PRINT	;YUP,IT'S PROM
	INR	L	;DO CHECK ON NEXT BYTE
	XRA	A	;CLEAR A REG. FOR 256 BYTE BLOCK COUNTER
	CMP	L	;TEST IF BLOCK CHECK DONE
	JNZ	MAP3	;LOOP 256 TIMES
	MVI	E,'.'	;INDICATE "." FOR NO MEMORY
PRINT:	MVI	L,0	;CLEAR LS BYTE OF MEMORY POINTER
	DCR	B	;COUNT ITEMS TO A LINE
	JNZ	NLINE	;SKIP IF NOT A NEW LINE
	MVI	B,16	;16 ITEMS TO A LINE
	CALL	CRLF	;OUTPUT "CR/LF"
	CALL	HEXO	;OUTPUT BLOCK ADDRESS
NLINE:	MVI	A,' '	;OUTPUT A SPACE CHARACTER
	CALL	PCHAR
	MOV	A,E	;GET MEMORY TYPE CHARACTER
	CALL	PCHAR	;PRINT IT
	INR	H	;BUMP FOR NEXT BLOCK
	JNZ	MAP1	;CONTINUE UNTIL LAST BLOCK
	CALL	CRLF	;KEEP IT NEAT FOR RETURN TO CP/M
	LHLD	OLDSP	;GET OLD STACK POINTER
	SPHL		;RESTORE IT
	RET		;STACK POINTER CONTAINS CCP'S STACK LOCATION
;
;
;
HEXO:	MOV	C,H	;GET MS BYTE
	CALL	HEXO1	;OUTPUT MS BYTE
	MOV	C,L	;GET LS BYTE
HEXO1:	MOV	A,C	;GET BYTE TO BE CONVERTED
	RAR		;ROTATE UPPER NIBBLE
	RAR
	RAR
	RAR
	CALL	HEXO2	;OUTPUT UPPER NIBBLE
	MOV	A,C
HEXO2:	ANI	00FH	;MASK UPPER NIBBLE
	CPI	10	;TEST IF >10
	JC	HEXO3	;SKIP IF <10
	ADI	7	;ADJUST ASCII
HEXO3:	ADI	030H	;ADD ASCII BIAS
PCHAR:	PUSH	H
	PUSH	D
	PUSH	B
	PUSH	PSW
	MVI	C,CHRF	;DO CP/M PRINT CHARACTER FUNCTION
	MOV	E,A	;CHARACTER TO E REG.
	CALL	BDOS	;LET CP/M DO THE WORK
	POP	PSW
	POP	B
	POP	D
	POP	H
	RET
;
;
;
CRLF:	MVI	A,CR	;PRINT "CARRIAGE-RETURN"
	CALL	PCHAR
	MVI	A,LF	;PRINT "LINE-FEED"
	CALL	PCHAR
	RET
;
;
;
BLKMSG	DB	CR,LF,'     0 1 2 3 4 5 6 7 8 9 A B C D E F$'
;
STRMSG	DB	'REMOVE DISKS & PRESS RETURN TO START > $'
;
;
OLDSP	DS	2	;ENTRY STACK POINTER VALUE FROM CP/M CCP
	DS	32	;RESERVE 16 LEVEL STACK
STACK	EQU	$
;
;
;
	END
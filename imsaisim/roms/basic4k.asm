*HEADING IMSAI 8080 4K BASIC
;
; MODIFIED TO WORK WITH SIO-2 TTY IDENTICAL TO 8K VERSION
; OCTOBER 2008, UDO MUNK
;
      ORG   0
;
;
BASIC EQU   $
      LD    HL,RAM+1024      ;POINT FIRST POSSIBLE END OF RAM
;     LD    A,0FAH           ;GET MODE SET
      LD    A,0BAH           ;**UM**
      JP    CONTI            ;GO CONTINUE
;
;
      ORG   8
RST1  EQU   $
;
;SKIP CHARS POINTED TO BY HL UNTIL NON-BLANK,
;LEAVE IN REG A
;
      LD    A,(HL)           ;LOAD THE BYTE AT (HL)
      CP    ' '              ;TEST  IF BLANK
      RET   NZ               ;RETURN IF NOT
      INC   HL               ;POINT NEXT
      JP    RST1             ;LOOP
;
;
      ORG   16
RST2  EQU   $
;
;COMPARE STRING AT (HL) TO STRING AT (DE)
;RETURN IF EQUAL (THRU X'00' IN DE) OR ON FIRST NOT EQUAL
;IGNORE ALL SPACES
;
      RST   8                ;SKIP SPACES
      LD    A,(DE)           ;GET CHAR TO MATCH WITH
      OR    A                ;TEST IT
      JP    NZ,COMP2         ;BRIF NOT EQUAL
      LD    A,(HL)           ;GET CHAR FOLLOWING
      RET                    ;RETURN
;
;
      ORG   24
RST3  EQU   $
;
;PRINT: 'XX ERR @ NNNN'
;
      LD    HL,IOBUF         ;POINT BUFFER
      LD    (HL),B           ;MOVE HI CHAR
      INC   HL               ;POINT NEXT
      JP    ERROR            ;CONTINUE ELSEWHERE
;
;
      ORG   32
RST4  EQU   $
;
;SHIFT THE LOW ORDER 4 BITS OF REG A TO THE HIGH 4 BITS
;
      AND   0FH              ;ISOLATE LOW 4
      RLA                    ;SHIFT ONE BIT
      RLA                    ;AGAIN
      RLA                    ;AGAIN
      RLA                    ;ONE LAST TIME
      RET                    ;RETURN
;
;
      ORG   40
RST5  EQU   $
;
;LOAD THE FLOATING POINT ACCUMULATOR WITH THE 4 BYTES AT (HL)
;
      LD    DE,FACC          ;POINT FLOAT ACC
      LD    B,4              ;BYTE COUNT
      JP    COPYH            ;GO MOVE IT
;
;
      ORG 48
RST6  EQU   $
;
;STORE THE FLOATING POINT ACCUMULATOR AT (HL)
;
      LD    DE,FACC          ;POINT FLOAT ACC
      LD    B,4              ;BYTE COUNT
      JP    COPYD            ;GO MOVE IT
;
;
      ORG   56
RST7  EQU   $
;
;INCREMENT HL BY BYTE AT (SP), RETURN TO (SP)+1
;
      EX    (SP),HL          ;GET RETURN ADDR IN HL
      LD    A,(HL)           ;GET THE INCREMENT
      INC   HL               ;POINT TRUE RETURN
      EX    (SP),HL          ;PUT BACK TO STACK
      PUSH  DE               ;SAVE DE
      LD    E,A              ;PUT IT IN LOW
      OR    A                ;TEST SIGN
      LD    D,0              ;DEFAULT POSITIVE
      JP    P,RST7A          ;BRIF +
      LD    D,0FFH           ;ELSE, NEG
RST7A ADD   HL,DE            ;BUMP HL
      POP   DE               ;RESTORE DE
      RET                    ;RETURN
;
;
;
CONTI EQU   $
;
;INITIALIZATION ROUTINE
;DETERMINE MEMORY SIZE. (START AT 4K AND TRY 1K INCREMENTS)
;SETUP POINTERS FOR STACK, DATA, AND PROGRAM
;INIT SIO BOARD
;
;     OUT   (TTY-1),A        ;WRITE TO SIO
      OUT   (TTY+1),A        ;**UM**
;     LD    A,17H            ;CMND: DTR, ENABLE TRNS, & RCVR,
      LD    A,37H            ;**UM**
;     OUT   (TTY-1),A        ;WRITE TO SIO
      OUT   (TTY+1),A        ;**UM**
      LD    BC,1024          ;1K INCR
FINDL EQU   $
      LD    A,(HL)           ;GET A BYTE FROM MEMORY
      CPL                    ;COMPLEMENT
      LD    (HL),A           ;REPLACE
      CP    (HL)             ;TEST IF RAM/ROM/END
      JP    NZ,MEMEN         ;BRIF OUT OF RAM
      CPL                    ;RE-COMPLEMENT
      LD    (HL),A           ;PUT ORIG BACK
      ADD   HL,BC            ;POINT NEXT 1K BLOCK
      JP    NC,FINDL         ;LOOP TILL 64K
MEMEN LD    SP,HL            ;SET STACK POINTER TO END OF MEMORY
      RST   RST7             ;GO BUMP HL ADDR
      DEFB  -100             ;ALLOW 100 BYTES
      LD    (DATAB),HL       ;SAVE ADDR OF START OF DATA
      XOR   A                ;GET A ZERO IN A
      LD    (HL),A           ;MARK EMPTY DATA
      LD    (OUTSW),A        ;TURN OUTPUT SUPPRESS OFF
      PUSH  AF               ;SET STACK 1 LEVEL DEEP WITHOUT
      LD    HL,0             ;CLEAR HL
      ADD   HL,SP            ;SP TO HL
      LD    (STACK),HL       ;SAVE BEG OF STACK
      LD    HL,BEGPR-1       ;POINT ONE BEFORE START OF PROGRAM
      LD    (HL),A           ;MARK EMPTY
      LD    HL,RNDX          ;POINT INIT RND NUMBER
      RST   RST5             ;GO LOAD TO FACC
      LD    HL,RNDNU         ;POINT RAM AREA
      RST   RST6             ;GO STORE
      LD    HL,RAM           ;POINT 1 BEFORE IOBUFF
      LD    (HL),0FFH        ;SET HIGH VALUE
GENRN CALL  RND              ;GO GENERATE A RANDUM NUMBER
;     IN    A,(TTY-1)        ;GET TTY STATUS
;     AND   40H              ;ISOLATE RXRDY
      IN    A,(TTY+1)        ;**UM**
      AND   2                ;**UM**
      JP    Z,GENRN
*HEADING IMSAI 8080 4K BASIC
READY EQU   $
;
;
;COMMAND INPUT ROUTINE
;
;READ A LINE FROM THE TTY
;IF STARTS WITH NUMERIC CHARACTERS, ASSUME IT'S A BASIC STA
;IF NOT, THEN IT IS EITHER AN IMMEDIATE STATEMENT OR A COM
;
GETCM XOR   A                ;SET NO PROMPT
      LD    HL,(STACK)       ;GET STACK ADDRESS
      LD    SP,HL            ;SET REG SP
      CALL  TERMI            ;GET A LINE
      CALL  PACK             ;GO PACK THE NUMBER INTO BC
      LD    A,B              ;GET HI BYTE OF LINE NUMBER
      OR    C                ;PLUS LOW BYTE
      JP    Z,EXEC           ;BRIF EXEC STATEMENT
      PUSH  BC               ;SAVE LINE NUMBER
      LD    DE,IMMED+1       ;POINT SAVE AREA
      EX    DE,HL            ;FLIP/FLOP
      LD    (HL),B           ;PUT LO LINE
      INC   HL               ;POINT NEXT
      LD    (HL),C           ;PUT LO LINE
      INC   HL               ;POINT NEXT
      LD    B,3              ;INIT COUNT
EDIT1 LD    A,(DE)           ;GET A BYTE
      LD    (HL),A           ;PUT IT DOWN
      INC   B                ;COUNT IT
      INC   HL               ;POINT NEXT
      INC   DE               ;DITTO
      OR    A                ;TEST BYTE JUST MOVED
      JP    NZ,EDIT1         ;LOOP
      LD    A,B              ;GET COUNT
      LD    (IMMED),A        ;STORE THE COUNT
      POP   BC               ;GET LINE NUMBER
      LD    HL,BEGPR         ;POINT BEGINNING OF PROGRAM
EDIT2 LD    A,(HL)           ;GET LEN CODE
      PUSH  HL               ;SAVE ADDR
      OR    A                ;TEST IT
      JP    Z,EDIT5          ;BRIF END
      INC   HL               ;POINT HI LINE
      LD    A,(HL)           ;LOAD IT
      CP    B                ;COMPARE
      JP    C,EDIT4          ;BRIF LOW
      JP    NZ,EDIT5         ;EDIT5 BRIF HIGH
      INC   HL               ;POINT LO LINE
      LD    A,(HL)           ;LOAD IT
      CP    C                ;COMPARE
      JP    C,EDIT4          ;BRIF LOW
      JP    NZ,EDIT5         ;BRIF HIGH
      DEC   HL               ;POINT BACK
      DEC   HL               ;TO BEGIN
      LD    D,H              ;COPY ADDR
      LD    E,L              ;TO DE
      LD    B,0              ;GET A ZERO
      LD    C,(HL)           ;GET LEN
      ADD   HL,BC            ;POINT NEXT STMT
EDIT3 LD    A,(HL)           ;GET LEN NEXT STMT
      OR    A                ;TEST IT
      JP    Z,EDITX          ;BRIF END
      LD    B,A              ;SET LENGTH
      CALL  COPYH            ;ELSE MOVE LINE
      JP    EDIT3            ;LOOP
EDIT4 POP   HL               ;GET ADDR
      LD    D,0              ;ZERO HI LEN
      LD    E,(HL)           ;GET LO LEN
      ADD   HL,DE            ;COMPUTE ADDR NEXT LINE
      JP    EDIT2            ;LOOP
EDITX EX    DE,HL            ;PUT NEW ADDR TO HL
      LD    (HL),A           ;MARK END
      LD    (PROGE),HL       ;AND UPDATE ADDRESS
EDIT5 LD    A,(IMMED)        ;GET LEN OF INSERT
      CP    4                ;TEST IF DELETE
      JP    Z,GETCM          ;BRIF IS
      LD    C,A              ;SET LO LEN
      LD    B,0              ;ZERO HI LEN
      LD    HL,(PROGE)       ;GET END OF PROG
      LD    D,H              ;COPY TO
      LD    E,L              ;DE
      ADD   HL,BC            ;DISP LEN OF INSERT
      LD    (PROGE),HL       ;UPDATE END POINT
      POP   BC               ;GET ADDR
EDIT6 LD    A,(DE)           ;GET A BYTE
      LD    (HL),A           ;COPY IT
      DEC   DE               ;POINT PRIOR
      DEC   HL               ;DITTO
      LD    A,D              ;GET HI ADDR
      CP    B                ;COMPARE
      JP    Z,EDIT7          ;BRIF HI EQUAL
      JP    NC,EDIT6         ;BRIF NOT LESS
EDIT7 LD    A,E              ;GET LO ADDR
      CP    C                ;COMPARE
      JP    NC,EDIT6         ;BRIF NOT LESS
      INC   DE               ;POINT FORWARD
      LD    HL,IMMED         ;POINT INSERT
      LD    B,(HL)           ;GET LENGTH
      CALL  COPYH            ;GO MOVE IT
      JP    GETCM            ;GO COMMAND
*HEADING IMSAI 8080 4K BASIC
EXEC  EQU   $
;
;
;
;DECODE COMMAND IN IOBUFF
;EXECUTE IF POSSIBLE
;THEN GOTO GET NEXT COMMAND
;
;
      LD    DE,NEWLI         ;POINT "NEW"
      LD    HL,IOBUF         ;POINT BUFFER
      RST   RST2             ;GO COMPARE
      JP    NZ,NOTSC         ;BRIF NOT
      LD    HL,BEGPR         ;POINT BEGINNING OF PGM
      LD    (PROGE),HL       ;SAVE END ADDRESS
      XOR   A                ;GET A ZERO
      LD    (HL),A           ;MARK EMPTY
      LD    HL,(DATAB)       ;POINT BEGINNING OF DATA
      LD    (HL),A           ;MARK EMPTY
      JP    READY            ;GO GET NEXT COMMAND
NOTSC LD    DE,LISTL         ;POINT LITERAL
      LD    HL,IOBUF         ;POINT BUFFER
      RST   RST2             ;GO COMPARE
      JP    Z,LIST           ;BRIF 'LIST'
      LD    DE,RUNLI         ;POINT LITERAL
      LD    HL,IOBUF         ;POINT BUFFER
      RST   RST2             ;GO COMPARE
      JP    Z,RUNIT          ;BRIF 'RUN'
      LD    (RUNSW),A        ;SET IMMEDIATE MODE
      LD    HL,IOBUF         ;POINT STMT
      LD    DE,IMMED         ;POINT NEW AREA
IMED  LD    A,(HL)           ;GET A BYTE
      LD    (DE),A           ;PUT TO D
      INC   DE               ;POINT NEXT
      INC   HL               ;DITTO
      OR    A                ;TEST IF END
      JP    NZ,IMED          ;LOOP
      LD    HL,NULLI         ;POINT FFFF
      LD    (LINE),HL        ;SAVE ADDR
      LD    HL,IMMED         ;POINT START OF CMMD
      JP    IMMD             ;GO IMMEDIATE
*HEADING IMSAI 8080 4K BASIC
RUNIT EQU   $
;
;
;RUN PROCESSOR, GET NEXT STATEMENT, AND EXECUTE IT
;IF IN IMMEDIATE MODE, THEN RETURN TO GETCMMD
;
;
      XOR   A                ;CLEAR A REG
      LD    (RUNSW),A        ;RESET SWITCH
      LD    (FORNE),A        ;INIT FOR/NEXT TABLE
      LD    HL,(DATAB)       ;POINT START OF VARIABLES
      LD    (HL),0           ;CLEAR IT
      LD    HL,BEGPR-1       ;GET ADDR OF PROGRAM
      LD    (DATAP),HL       ;'RESTORE'
      INC   HL               ;POINT 1ST BYTE
      LD    (STMT),HL        ;SAVE IT
      JP    NEXTS            ;GO PROCESS IT
;
RUN   LD    HL,(STMT)        ;GET ADDR OF PREVIOUS STMT
      LD    E,(HL)           ;GET LEN CODE
      LD    D,0              ;CLEAR HIGH BYTE OF ADDR
      ADD   HL,DE            ;INCR STMT POINTER
      LD    (STMT),HL        ;SAVE IT
;
NEXTS EQU   $
      LD    A,(RUNSW)        ;GET RUN TYPE
      OR    A                ;TEST IT
      JP    NZ,GETCM         ;BRIF IMMEDIATE MODE
      LD    A,(HL)           ;GET LEN CODE
      OR    A                ;SEE IF NO MORE STATEMENTS
      JP    Z,READY          ;BRIF END
NOTDO EQU   $
      INC   HL               ;POINT LINE NUMBER
      LD    (LINE),HL        ;SAVE ADDR
      INC   HL               ;POINT 2ND BYTE
      INC   HL               ;POINT 1ST PGM BYTE
IMMD  RST   RST1             ;SKIP BLANKS
CONTX LD    (ADDR1),HL       ;SAVE ADDR
      CALL  TSTCH            ;GO SEE IF CONTROL-C
      LD    DE,JMPTB         ;POINT TO TABLE
TABLO LD    A,(DE)           ;GET FIRST BYTE OF LIT
      OR    A                ;TEST IF END OF TABLE
      JP    Z,TABEN          ;BRIF IS
      LD    HL,(ADDR1)       ;GET ADDRESS OF CMMD
      RST   RST2             ;GO COMPARE
      JP    NZ,NOJMP         ;BRIF NOT EQUAL
      PUSH  HL               ;SAVE HL
      INC   DE               ;POINT NEXT BYTE
      LD    A,(DE)           ;LOAD IT
      LD    L,A              ;LOW BYTE TOL
      INC   DE               ;POINT NEXT BYTE
      LD    A,(DE)           ;LOAD IT
      LD    H,A              ;HIGH BYTE TO H
      EX    (SP),HL          ;HL TO STACK, STACK TO HL
      RET                    ;JUMP TO PROPER ROUTINE
NOJMP INC   DE               ;POINT NEXT
      LD    A,(DE)           ;LOAD IT
      OR    A                ;TEST IT
      JP    NZ,NOJMP         ;BRIF NOT
      INC   DE               ;POINT NEXT
      INC   DE               ;DITTO
      INC   DE               ;POINT FIRST BYTE NEXT LIT
      JP    TABLO            ;LOOP
;
TABEN LD    HL,(ADDR1)       ;RESTORE HL POINTER
      JP    LET              ;ASSUME IT'S A LET STATEMENT
*HEADING IMSAI 8080 4K BASIC
LIST  EQU   $
;
;
;LIST PROCESSOR
;DUMP THE SOURCE PROGRAM TO TTY OR PAPER TAPE
;
;
      LD    HL,BEGPR         ;POINT BEGINNING OF PROGRAM
LISTX LD    A,(HL)           ;GET LEN CODE
      OR    A                ;TEST IF END OF PGM
      JP    Z,READY          ;BRIF END OF PGM
      SUB   3                ;SUBTRACT THREE
      LD    B,A              ;SAVE LEN
      INC   HL               ;POINT HI BYTE OF LINE #
      LD    DE,IOBUF         ;POINT BUFFER AREA
      CALL  LINEO            ;CONVERT LINE NUMBER
      CALL  COPYH            ;GO MOVE THE LINE
      CALL  TSTCH            ;GO SEE IF CONTROL-C
      PUSH  HL               ;SAVE HL ADDR
      CALL  TERMO            ;GO TYPE IT
      POP   HL               ;RETREIVE H ADDR
      JP    LISTX            ;CONTINUE
;
*HEADING IMSAI 8080 4K BASIC
GOSUB EQU   $
;
;
; STMT:  GOSUB NNNN
;
      EX    DE,HL            ;FLIP/FLOP DE HL
      LD    HL,(STMT)        ;GET STATEMENT ADDRESS
      PUSH  HL               ;SAVE RETURN ADDRESS IN STACK
      LD    A,0FFH           ;MARK AS GOSUB
      PUSH  AF               ;SAVE STATUS
      EX    DE,HL            ;RESTORE HL
;
;
GOTO  EQU   $
;
;
; STMT:  GOTO NNNN
;
      CALL  PACK             ;GO GET LINE NUMBER IN BC
      LD    HL,BEGPR         ;POINT BEGINNING OF PROGRAM
GOTO1 LD    A,(HL)           ;GET LEN
      OR    A                ;TEST IF END OF PROGRAM
      JP    Z,ULERR          ;BRIF UNDEFIND STATEMENT
      INC   HL               ;POINT NEXT
      LD    A,(HL)           ;GET THE HIGH LINE NUMBER
      CP    B                ;TEST WITH DESIRED
      JP    C,GOTO2          ;BRIF LOW
      INC   HL               ;POINT NEXT BYTE
      LD    A,(HL)           ;GET LOW LINE NUMBER
      DEC   HL               ;POINT BACK
      CP    C                ;TEST WITH WANTED
      JP    C,GOTO2          ;BRIF LOW
      JP    NZ,ULERR         ;BRIF LINE MISSING
      DEC   HL               ;POINT TO START OF STMT
      LD    (STMT),HL        ;SAVE ADDR
      JP    NEXTS            ;GO PROCESS THE STATEMENT
GOTO2 DEC   HL               ;POINT START OF STMT
      LD    E,(HL)           ;GET LENGTH
      LD    D,0              ;ZERO MDB
      ADD   HL,DE            ;POINT NEXT STMT
      JP    GOTO1            ;LOOP
*HEADING IMSAI 8080 4K BASIC
RETUR EQU   $
;
;
; STMT:  RETURN
;
      POP   AF               ;POP THE STACK
      CP    0FFH             ;TEST IF GOSUB IN EFFECT
      JP    NZ,RTERR         ;BRIF ERROR
      POP   HL               ;GET RETURNED STATEMENT ADDRESS
      LD    (STMT),HL        ;RESTORE
      JP    RUN              ;CONTINUE AT STMT FOLLOWING GOSUB
*HEADING IMSAI 8080 4K BASIC
PRINT EQU   $
;
;
; STMT:  PRINT . . . .
;
;
      XOR   A                ;CLEAR REG A
      LD    (PRSW),A         ;SET SWITCH
PR1   LD    DE,IOBUF         ;POINT BUFFER
      RST   RST1             ;SKIP TO NEXT FIELD
      CP    '"'              ;TEST IF QUOTE
      JP    NZ,PR6           ;BRIF NOT LITERAL
PR2   INC   HL               ;POINT NEXT
      LD    A,(HL)           ;GET THE CHAR
      OR    A                ;TEST IF END OF STMT
      JP    Z,SNERR          ;BRIF MISSING END OF QUOTE
PR3   CP    '"'              ;TEST IF END QUOTE
      JP    NZ,PR5           ;BRIF NOT
      INC   HL               ;POINT NEXT
PRNXT LD    A,0FEH           ;SET CODE = NO CR/LF
      LD    (DE),A           ;PUT TO BUFFER
      PUSH  HL               ;SAVE HL
      CALL  TERMO            ;GO PRINT IT
      POP   HL               ;RESTORE HL
      JP    PRINT            ;RECURSIVE TO NEXT FIELD
PR4   LD    A,(PRSW)         ;GET SWITCH
      OR    A                ;TEST IF STMT ENDED WITH , OR ;
      CALL  Z,CRLF           ;CALL IF NOT
      JP    RUN              ;CONTINUE NEXT STMT
PR5   LD    (DE),A           ;PUT CHAR TO BUFFER
      INC   DE               ;POINT NEXT OUT
      JP    PR2              ;LOOP
PR6   OR    A                ;TEST IF END OF STMT
      JP    Z,PR4            ;BRIF IT IS
      CP    ','              ;TEST IF COMMA
      JP    Z,PR7            ;BRIF IT IS
      CP    ';'              ;TEST IF SEMI-COLON
      JP    Z,PR8            ;BRIF IT IS
      PUSH  DE               ;SAVE DE
      CALL  EXPR             ;GO EVALUATE EXPRESSION
      POP   DE               ;RESTORE DE
      PUSH  HL               ;SAVE HL
      EX    DE,HL            ;FLIP/FLOP
      CALL  FOUT             ;GO CONVERT OUTPUT
      INC   HL               ;POINT NEXT
      LD    (HL),' '         ;SPACE FOLLOWS NUMBERS
      INC   HL               ;POINT NEXT
      EX    DE,HL            ;FLIP/FLOP
      POP   HL               ;RESTORE HL
      JP    PRNXT            ;CONTINUE
PR7   LD    A,(COLUM)        ;GET COLUMN POINTER
      CP    56               ;COMPARE TO 56
      JP    NC,TBEND         ;BRIF NO ROOM LEFT
      LD    B,A              ;SAVE IT
      XOR   A                ;INIT POSITION
TBLP  CP    B                ;COMPARE
      JP    Z,TBLP2          ;BRIF ON A TAB STOP
      JP    NC,TBON          ;BRIF SHY OF TAB
TBLP2 ADD   A,14             ;POINT NEXT STOP
      JP    TBLP             ;LOOP
TBON  LD    (COLUM),A        ;UPDATE CTR
      SUB   B                ;COMPUTE NUMBER OF SPACES
      LD    B,A              ;SAVE IT
TBSPA CALL  TESTO            ;WAIT TILL READY
      LD    A,' '            ;SPACE TO REG A
      OUT   (TTY),A          ;OUTPUT IT
      DEC   B                ;SUB 1 FROM CTR
      JP    NZ,TBSPA         ;LOOP IF NOT
PR8   INC   HL               ;POINT NEXT
      LD    (PRSW),A         ;SET THE SWITCH
      JP    PR1              ;GO NEXT FIELD
TBEND CALL  CRLF             ;PUT CR/LF
      JP    PR8              ;GO SET SW
*HEADING IMSAI 8080 4K BASIC
FOR   EQU   $
;
;
; STMT:  FOR VAR = EXPR TO EXPR :STEP EXPR:
;
;
      CALL  VAR              ;NEXT WORD MUST BE VARIABLE
      EX    DE,HL            ;FLIP/FLOP
      LD    (INDX),HL        ;SAVE VARIABLE NAME
      EX    DE,HL            ;FLIP/FLOP AGAIN
      CP    '='              ;TEST FOR EQUAL SIGN
      JP    NZ,SNERR         ;BRIF NO EQUAL
      INC   HL               ;POINT NEXT
      CALL  EXPR             ;GO EVALUATE EXPR IF ANY
      PUSH  HL               ;SAVE HL
      LD    HL,(INDX)        ;GET INDEX NAME
      EX    DE,HL            ;FLIP/FLOP
      CALL  SEARC            ;GO LOCATE NAME
      EX    DE,HL            ;PUT ADDR IN HL
      LD    (ADDR1),HL       ;SAVE ADDR
      RST   RST6             ;GO STORE THE VALUE
      POP   HL               ;RESTORE POINTER TO STMT
      LD    DE,TOLIT         ;GET LIT ADDR
      RST   RST2             ;GO COMPARE
      JP    NZ,SNERR         ;BRIF ERROR
      CALL  EXPR             ;GO EVALUATE TO-EXPR
      PUSH  HL               ;SAVE HL
      LD    HL,TVAR1         ;POINT SAVE AREA
      RST   RST6             ;SAVE 'TO' EXPR
      LD    HL,ONE           ;POINT CONSTANT: 1
      RST   RST5             ;LOAD IT
      POP   HL               ;RESTORE HL
      LD    A,(HL)           ;GET THAT CHAR
      OR    A                ;TEST FOR END OF STATEMENT
      JP    Z,NOSTP          ;BRIF NO STEP
      LD    DE,STEPL         ;TEST FOR LIT STEP
      RST   RST2             ;GO COMPARE
      JP    NZ,SNERR         ;BRIF NOT STEP
FORST CALL  EXPR             ;GO EVAL STEP
NOSTP LD    HL,TVAR2         ;GET ADDR OF TEMP VARIABLE
      RST   RST6             ;SAVE END VALUE
      CALL  FTEST            ;GET SIGN OF FACC
      PUSH  AF               ;SAVE A, STATUS
      LD    HL,TVAR1         ;GET END VALUE
      RST   RST5             ;LOAD IT
      LD    HL,(ADDR1)       ;GET ADDR OF INDEX
      CALL  FSUB             ;COMPAE TO END VALUE
      POP   AF               ;RESTORE STATUS
      JP    P,FORPO          ;BRIF FOR IS POS
FORXE CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    Z,FORTA          ;BRIF START = END
      JP    M,FORTA          ;BRIF START > END
      JP    LNEXT            ;GO LOCATE MATCHING NEXT
FORPO CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    M,LNEXT          ;BRIF START > END
FORTA LD    DE,FORNE         ;POINT TABLE
      LD    HL,(INDX)        ;GET INDEX NAME
      EX    DE,HL            ;FLIP/FLOP
      LD    A,(HL)           ;GET COUNT
      LD    B,A              ;STORE IT
      LD    C,1              ;NEW CTR
      OR    A                ;TEST IF ZERO
      INC   HL               ;POINT
      JP    Z,FOREQ          ;BRIF TABLE EMPTY
FORLP LD    A,(HL)           ;GET 1ST BYTE
      CP    D                ;TEST IF EQUAL
      JP    NZ,FORNO         ;BRIF NOT
      INC   HL               ;POINT NEXT
      LD    A,(HL)           ;GET NEXT BYTE
      DEC   HL               ;POINT BACK
      CP    E                ;TEST IF EQUAL
      JP    NZ,FOREQ         ;BRIF EQUAL
FORNO RST   RST7             ;GO BUMP HL
      DEFB  12               ;BY 12
      INC   C                ;COUNT IT
      DEC   B                ;DECR CTR
      JP    NZ,FORLP         ;LOOP
FOREQ LD    A,C              ;GET UPDATED COUNT
      CP    9                ;TEST IF TBL EXCEEDED
      JP    NC,FOERR         ;ERROR IF MORE THAN 8 OPEN FOR/NEXT
      LD    (FORNE),A        ;PUT IN TABLE
      LD    (HL),D           ;STORE IT
      INC   HL               ;POINT NEXT
      LD    (HL),E           ;STORE IT TOO
      INC   HL               ;POINT NEXT
      PUSH  HL               ;SAVE HL
      LD    HL,TVAR2         ;POINT STEP
      RST   RST5             ;GO LOAD IT
      POP   HL               ;RESTORE HL
      RST   RST6             ;PUT IN TABLE
      PUSH  HL               ;SAVE HL
      LD    HL,TVAR1         ;POINT TO-VAL
      RST   RST5             ;GO LOAD IT
      POP   HL               ;RESTORE HL
      RST   RST6             ;PUT IN TABLE
      LD    A,(STMT+1)       ;GET HIGH STMT ADDR
      LD    (HL),A           ;PUT IT
      INC   HL               ;POINT NEXT
      LD    A,(STMT)         ;GET LOW STMT ADDR
      LD    (HL),A           ;PUT IT
      JP    RUN              ;CONTINUE
LNEXT LD    HL,(STMT)        ;GET ADDR OF STMT
      LD    E,(HL)           ;GET LENGTH CODE
      LD    D,0              ;INIT INCREMENT
      ADD   HL,DE            ;COMPUTE ADDR OF NEXT STATEMENT
      LD    A,(HL)           ;GET NEW LEN CODE
      OR    A                ;SEE IF END OF PGM
      JP    Z,NXERR          ;BRIF IT IS
      LD    (STMT),HL        ;SAVE ADDRESS
      RST   RST7             ;GO BUMP HL
      DEFB  3                ;BY THREE
      RST   RST1             ;SKIP SPACES
      LD    DE,NEXTL         ;POINT 'NEXT'
      RST   RST2             ;SEE IF IT IS A NEXT STMT
      JP    NZ,LNEXT         ;LOOP IF NOT
      RST   RST1             ;SKIP SPACES
      LD    A,(INDX+1)       ;GET FIRST CHAR
      CP    (HL)             ;COMPARE
      JP    NZ,LNEXT         ;BRIF NOT MATCH NEXT
      LD    A,(INDX)         ;GET 2ND CHAR
      INC   HL               ;DITTO
      CP    ' '              ;SEE IF SINGLE CHAR
      JP    Z,FORN1          ;BRIF IT IS
      CP    (HL)             ;COMPARE THE TWO
      JP    NZ,LNEXT         ;BRIF NOT EQUAL
FORN1 RST   RST1             ;SKIP TO END (HOPEFULLY)
      OR    A                ;SEE IF END
      JP    NZ,LNEXT         ;BRIF NOT END
      JP    RUN              ;ELSE, GO NEXT STMT
*HEADING IMSAI 8080 4K BASIC
IF    EQU   $
;
;
; STMT:  IF EXPR RELATION EXPR THEN STMT #
;
;
      CALL  EXPR             ;GO EVALUATE LEFT EXPRESSION
      PUSH  HL               ;SAVE HL
      LD    HL,TVAR1         ;GET ADDR OF TEMP STORAGE
      RST   RST6             ;SAVE IT
      POP   HL               ;RESTORE HL
      XOR   A                ;CLEAR A
      LD    C,A              ;SAVE IN REG C
      LD    B,A              ;INIT REG
IFREL LD    A,(HL)           ;GET OPERATOR
      INC   B                ;COUNT
      CP    '='              ;TEST FOR EQUAL
      JP    NZ,IFEQ          ;BRIF IT IS
      INC   C                ;ADD 1 TO C
      INC   HL               ;POINT NEXT
IFEQ  CP    '>'              ;TEST FOR GREATER THAN
      JP    NZ,IFGT          ;BRIF IT IS
      INC   C                ;ADD TWO
      INC   C                ;TO REL CODE
      INC   HL               ;POINT NEXT
IFGT  CP    '<'              ;TEST FOR LESS THAN
      JP    NZ,IFLT          ;BRIF IT IS
      LD    A,C              ;GET REL CODE
      ADD   A,4              ;PLUS FOUR
      LD    C,A              ;PUT BACK
      INC   HL               ;POINT NEXT
IFLT  LD    A,C              ;GET REL CODE
      OR    A                ;TEST IT
      JP    Z,SNERR          ;BRIF SOME ERROR
      LD    (REL),A          ;SAVE CODE
      LD    A,B              ;GET COUNT
      CP    2                ;TEST FOR TWO
      JP    NZ,IFREL         ;SEE IF MULTIPLE RELATION
      CALL  EXPR             ;GO EVALUATE RIGHT SIDE
      PUSH  HL               ;SAVE STMT LOCATION
      LD    HL,TVAR1         ;POINT LEFT
      CALL  FSUB             ;SUBTRACT LEFT FROM RIGHT
      POP   HL               ;RESTORE STMT ADDR
      LD    A,(REL)          ;GET RELATION
      RRA                    ;TEST BIT D0
      JP    NC,IFNOT         ;BRIF NO EQUAL TEST
      CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    Z,TRUE           ;BRIF LEFT=RIGHT
IFNOT LD    A,(REL)          ;LOAD RELATION
      AND   02H              ;MASK IT
      JP    Z,IFNTX          ;BRIF NO >
      CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    M,TRUE           ;BRIF GT
IFNTX LD    A,(REL)          ;LOAD RELATION
      AND   04H              ;MASK IT
      JP    Z,RUN            ;BRIF NO <
      CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    M,RUN            ;BRIF GT
      JP    Z,RUN            ;BRIF EQUAL
TRUE  LD    DE,THENL         ;GET ADDR 'THEN'
      RST   RST2             ;GO COMPARE
      JP    NZ,SNERR         ;BRIF ERROR
      JP    GOTO             ;BRIF IT IS
*HEADING IMSAI 8080 4K BASIC
LET   EQU   $
;
;
; STMT:  :LET: VAR = EXPR
;
;
      CALL  VAR              ;NEXT MUST BE VARIABLE NAME
      CP    '='              ;TEST FOR EQUAL SIGN
      JP    NZ,SNERR         ;BRIF MISSING EQUAL
      CALL  SEARC            ;GO FIND ADDRESS OF VAR
      PUSH  DE               ;SAVE ADDRESS
      INC   HL               ;POINT NEXT
      CALL  EXPR             ;GO EVALUATE EXPRESSION
      POP   HL               ;RESTORE ADDRESS
      RST   RST6             ;GO STORE VARIABLE
      JP    RUN              ;CONTINUE
*HEADING IMSAI 8080 4K BASIC
NEXT  EQU   $
;
;
; STMT:  NEXT VAR
;
;
      CALL  VAR              ;GET VARIABLE NAME
      EX    DE,HL            ;FLIP/FLOP
      LD    (INDX),HL        ;SAVE VAR NAME
      PUSH  HL               ;SAVE VAR NAME
      LD    HL,FORNE         ;POINT FOR/NEXT TABLE
      LD    B,(HL)           ;GET SIZE
      LD    A,B              ;LOAD IT
      OR    A                ;TEST IT
      JP    Z,NXERR          ;BRIF TABLE EMPTY
      INC   HL               ;POINT NEXT
      POP   DE               ;RESTORE VAR NAME
NXLP  LD    A,(HL)           ;GET 1ST BYTE
      INC   HL               ;POINT NEXT
      CP    D                ;COMPARE
      JP    NZ,NXNE          ;BRIF NOT EQUAL
      LD    A,(HL)           ;GET 2ND BYTE
      CP    E                ;COMPARE
      JP    Z,NXEQ           ;BRIF EQUAL
NXNE  RST   RST7             ;GO BUMP HL
      DEFB  11               ;BY ELEVEN
      DEC   B                ;DECR COUNT
      JP    NZ,NXLP          ;LOOP
      JP    NXERR            ;GO PUT ERROR MSG
NXEQ  LD    A,(FORNE)        ;GET ORIG COUNT
      SUB   B                ;MINUS REMAIN
      INC   A                ;PLUS ONE
      LD    (FORNE),A        ;STORE NEW COUNT
      INC   HL               ;POINT STEP
      PUSH  HL               ;SAVE HL ADDR
      CALL  SEARC            ;GO GET ADDR OF INDEX
      EX    DE,HL            ;PUT TO HL
      LD    (ADDR1),HL       ;SAVR IT
      RST   RST5             ;LOAD INDEX
      POP   HL               ;GET HL (TBL)
      PUSH  HL               ;RESAVE
      CALL  FADD             ;ADD STEP VALUE
      LD    HL,TVAR1         ;POINT NEW INDEX
      RST   RST6             ;STORE IT
      POP   HL               ;GET HL (TBL)
      PUSH  HL               ;RESAVE
      RST   RST7             ;GO BUMP HL
      DEFB  4                ;BY FOUR
      CALL  FSUB             ;SUBTRACT TO VALUE
      CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    Z,NXTZR          ;BRIF ZERO
      POP   HL               ;GET HL (PTR TO STEP)
      PUSH  HL               ;RE-SAVE
      LD    A,(HL)           ;GET SIGN & EXPONENT OF STEP
      OR    A                ;TEST IT
      LD    A,(FACC)         ;GET SIGN & EXPONENT OF DIFFERENCE
      JP    M,NXTNE          ;BRIF NEGATIVE
NXTPO OR    A                ;TEST IT
      JP    M,NXTZR          ;BRIF LESS THAN TO-EXPR
      JP    NEXTZ            ;GO PAST NEXT
NXTNE OR    A                ;TEST IT
      JP    M,NEXTZ          ;BRIF END OF LOOP
NXTZR POP   HL               ;POP THE STACK
      RST   RST7             ;GO BUMP HL
      DEFB  8                ;BY EIGHT
      LD    D,(HL)           ;GET HI BYTE
      INC   HL               ;POINT NEXT
      LD    E,(HL)           ;GET LOW BYTE
      EX    DE,HL            ;PUT TO HL
      LD    (STMT),HL        ;SAVE ADDR OF FOR
      LD    DE,TVAR1         ;POINT UPDATED INDEX VALUE
      LD    HL,(ADDR1)       ;GET ADDR
      LD    B,4              ;LENGTH
      CALL  COPYD            ;GO MOVE TO I
      JP    RUN              ;CONTINUE STMT AFTER FOR
NEXTZ EQU   $
      LD    HL,FORNE         ;GET ADDR TABLE
      DEC   (HL)             ;SUBTRACT ONE FROM COUNT
      JP    RUN              ;GO STMT AFTER NEXT
*HEADING IMSAI 8080 4K BASIC
INPUT EQU   $
;
;
; STMT:  INPUT VAR :, VAR, VAR:
;
;
      LD    DE,IOBUF         ;GET ADDR OF BUFFER
      EX    DE,HL            ;FLIP/FLOP
      LD    (ADDR1),HL       ;SAVE ADDR
      LD    (HL),0           ;MARK BUFFER EMPTY
      EX    DE,HL            ;FLIP/BACK
IN1   CALL  VAR              ;GO GET VAR NAME
      CALL  SEARC            ;GO ;LOOK UP ADDRESS
      PUSH  HL               ;SAVE HL ADDR
      PUSH  DE               ;SAVE VAR ADDRE
      LD    HL,(ADDR1)       ;GET ADDR PREV BUFFER
      LD    A,(HL)           ;LOAD CHAR
      CP    ','              ;TEST IF COMMA
      INC   HL               ;POINT NEXT
      JP    Z,IN2            ;BRIF CONTINUE FROM PREV
      OR    A                ;TEST IF END OF LINE
      JP    NZ,SNERR         ;BRIF ERROR
      LD    A,'?'            ;PROMPT CHAR
      CALL  TERMI            ;GO READ FROM TTY
IN2   CALL  FIN              ;GO CONVERT TO FLOATING
      LD    (ADDR1),HL       ;SAVE ADDRESS
      POP   HL               ;GET VAR ADDRESS
      RST   RST6             ;GO STORE THE NUMBER
      POP   HL               ;RESTORE STMT POINTER
      RST   RST1             ;SKIP SPACES
      CP    ','              ;TEST FOR COMMA
      INC   HL               ;POINT NEXT
      JP    Z,IN1            ;RECURSIVE IF COMMA
      DEC   HL               ;POINT BACK
      JP    RUN              ;GO NEXT STMT
*HEADING IMSAI 8080 4K BASIC
READ  EQU   $
;
; STMT:  READ VAR :,VAR ...:
;
      CALL  VAR              ;GO GET VAR NAME
      CALL  SEARC            ;GO GET ADDRESS
      PUSH  HL               ;SAVE HL
      PUSH  DE               ;SAVE DE
      LD    HL,(DATAP)       ;GET DATA STMT POINTER
      LD    A,(HL)           ;LOAD THE CHAR
      OR    A                ;TEST IF END OF STMT
      JP    NZ,NOTDT         ;BRIF NOT END OF STMT
      INC   HL               ;POINT START NEXT STMT
DATAN LD    A,(HL)           ;LOAD LEN
      LD    (DATAP),HL       ;SAVE ADDR
      OR    A                ;TEST IF END OF PGM
      JP    Z,DAERR          ;BRIF OUT OF DATA
      INC   HL               ;POINT NEXT
      LD    (DASTM),HL       ;SAVE ADDR OF LINE NUMBER
      INC   HL               ;SKIP LINE NUMBER
      INC   HL               ;POINT 1ST DATA BYTE
      RST   RST1             ;SKIP BLANKS
      LD    DE,DATAL         ;POINT 'DATA'
      RST   RST2             ;COMPARE
      JP    Z,NOTDT          ;BRIF IT IS DATA STMT
      LD    HL,(DATAP)       ;GET ADDR START
      LD    E,(HL)           ;GET LEN CODE
      LD    D,0              ;CLEAR D
      ADD   HL,DE            ;POINT NEXT STMT
      JP    DATAN            ;LOOP NEXT STMT
NOTDT CALL  FIN              ;GO CONVERT VALUE
      LD    A,(HL)           ;GET CHAR WHICH STOPPED US
      CP    ','              ;TEST IF COMMA
      JP    NZ,NOTCO         ;BRIF NOT
      INC   HL               ;POINT NEXT
DATOK LD    (DATAP),HL       ;SAVE ADDRESS
      POP   HL               ;RESTORE ADDR OF VAR
      RST   RST6             ;STORE THE VALUE
      POP   HL               ;RESTORE POINTER TO STM
      LD    A,(HL)           ;LOAD THE CHAR
      CP    ','              ;TEST IF COMMA
      INC   HL               ;POINT NEXT
      JP    Z,READ           ;RECURSIVE IF IT IS
      DEC   HL               ;RESET
      JP    RUN              ;CONTINUE
NOTCO OR    A                ;TEST IF END OF STMT
      JP    Z,DATOK          ;BRIF OK
      LD    HL,(DASTM)       ;GET DATA STMT LINE NUMBER
      LD    (LINE),HL        ;SAVE IN LINE NUMBER
      JP    SNERR            ;GO PROCESS ERROR
;
*HEADING IMSAI 8080 4K BASIC
FIN   EQU   $
;
;FLOATING POINT INPUT CONVERSION ROUTINE
;
;THIS SUBROUTINE CONVERTS AN ASCII STRING OF CHARACTERS TO
;POINT ACCUMULATOR.  THE INPUT FIELD MAY CONTAIN ANY VALID
;INCLUDING SCIENTIFIC (NNN.NNNNE+NN)
;THE INPUT STRING IS TERMINATED BY ANY NON-NUMERIC CHARACT
;
;
      EX    DE,HL            ;FLIP/FLOP DE HL
      LD    HL,FACC          ;POINT TO FACC
      LD    B,4              ;LOOP CTR
      CALL  ZEROM            ;GO CLEAR THE FACC
      RST   RST7             ;GO BUMP HL
      DEFB  -4               ;BY NEG FOUR
      LD    C,B              ;INIT DIGIT COUNTER
      LD    A,(DE)           ;GET FIRST BYTE
      CP    '+'              ;TEST FOR PLUS SIGN
      JP    Z,FIN2           ;BRIF IS
      CP    '-'              ;TEST FOR MINUS SIGN
      JP    NZ,FIN3          ;BRIF NOT
      LD    (HL),80H         ;SET MINUS MANTISSA
FIN2  INC   DE               ;POINT NEXT DIGIT
      LD    A,(DE)           ;GET THE BYTE
FIN3  CP    '0'              ;TEST FOR LEADING ZERO
      JP    Z,FIN2           ;BRIF IT IS
FIN4  CP    '9'+1            ;TEST FOR NINE
      JP    NC,FIN14         ;BRIF NOT NUMERIC
      CP    '0'              ;TEST FOR ZERO
      JP    C,FIN5           ;BRIF NOT NUMERIC
      INC   B                ;COUNT EXPONENT
      CALL  FIN9             ;STORE THE DIGIT
      INC   DE               ;POINT NEXT
      LD    A,(DE)           ;GET THE DIGIT
      JP    FIN4             ;LOOP
FIN5  CP    '.'              ;TEST FOR DOT
      JP    NZ,FIN19         ;BRIF NOT
      LD    A,C              ;GET DIGIT COUNT
      OR    A                ;TEST FOR ZERO
      JP    NZ,FIN7          ;BRIF NOT
FIN6  INC   DE               ;POINT NEXT
      LD    A,(DE)           ;GET DIGIT
      CP    '0'              ;TEST FOR ZERO
      JP    NZ,FIN8          ;BRIF NOT
      DEC   B                ;COUNT IT
      JP    FIN6             ;LOOP
FIN7  INC   DE               ;POINT NEXT
      LD    A,(DE)           ;GET THE DIGIT
FIN8  CP    '0'              ;TEST FOR ZERO
      JP    C,FIN19          ;BRIF LOWER
      CP    '9'+1            ;TEST FOR NINE
      JP    NC,FIN14         ;BRIF HIGH
      CALL  FIN9             ;GO STORE DIGIT
      JP    FIN7             ;LOOP
FIN9  LD    A,C              ;GET DIGIT COUNT
      CP    6                ;TEST FOR MAX
      RET   Z                ;RETURN IF EQUAL
      INC   A                ;ADD ONE
      LD    C,A              ;REPLACE PREV COUNT
      INC   A                ;PLUS ONE
      RRA                    ;DIVIDE BY TWO
      AND   0FH              ;MASK OFF UNUSED BITS
      ADD   A,L              ;PLUS LOW BYTE OF H
      LD    L,A              ;REPLACE LOW BYTE OF HL
      LD    A,C              ;RE-LOAD DIGIT COUNT
      RRA                    ;TEST EVEN/ODD
      LD    A,(DE)           ;GET THE DIGIT
      JP    C,FIN12          ;BRIF ODD DIGIT
      AND   0FH              ;LOW 4 BITS ONLY
      OR    (HL)             ;GET HIGH 4 BITS
      JP    FIN13            ;GO RETURN
FIN12 RST   RST4             ;SHIFT LEFT
FIN13 LD    (HL),A           ;REPLACE
      LD    HL,FACC          ;POINT TO FACC
      RET                    ;RETURN
FIN14 CP    'E'              ;TEST FOR EXPLICIT EXPONENT
      JP    NZ,FIN19         ;BRIF NOT EQUAL
      INC   DE               ;POINT NEXT
      LD    A,(DE)           ;GET DIGIT
      LD    C,0              ;CLEAR COUNTER
      CP    '+'              ;TEST FOR PLUS
      JP    Z,FIN17          ;BRIF EQUAL
      CP    '-'              ;TEST FOR MINUS
      JP    NZ,FIN16         ;BRIF NOT EQUAL
      CALL  FIN15            ;GET NUMERIC EXPONENT
      LD    A,C              ;LOAD THE NUMBER
      CPL                    ;COMPLEMENT
      INC   A                ;PLUS ONE (TWOS COMPLEMENT)
      JP    FIN18            ;CONTINUE
FIN15 INC   DE               ;POINT NEXT
      LD    A,(DE)           ;GET DIGIT
      CP    '0'              ;TEST ZERO
      RET   C                ;RETURN IF ERROR
      CP    '9'+1            ;TEST NINE
      RET   NC               ;RETURN IF NOT NUMERIC
      LD    A,C              ;GET PRIOR
      ADD   A,A              ;TIMES TWO
      LD    C,A              ;SAVE
      ADD   A,A              ;TIMES FOUR
      ADD   A,A              ;TIMES EIGHT
      ADD   A,C              ;TIMES TEN
      LD    C,A              ;SAVE
      LD    A,(DE)           ;GET THIS DIGIT
      AND   0FH              ;MASK OFF HIGH FOUR BITS
      ADD   A,C              ;PLUS PREV*10
      LD    C,A              ;SAVE
      JP    FIN15            ;LOOP
FIN16 DEC   DE               ;POINT PRIOR TEMP
FIN17 CALL  FIN15            ;GO GET NUMERIC EXPONENT
      LD    A,C              ;LOAD THE EXPONENT
FIN18 ADD   A,B              ;PLUS COMPUTED EXPONENT
      LD    B,A              ;SAVE IT
      LD    A,(DE)           ;GET LAST CHAR
FIN19 INC   HL               ;POINT 1ST DIGIT
      LD    A,(HL)           ;LOAD
      OR    A                ;TEST IF ZERO
      JP    Z,FIN20          ;BRIF ZERO
      DEC   HL               ;POINT EXPONENT
      DEC   B                ;SUB ONE FROM EXPONENT
      LD    A,B              ;GET EXPONENT
      AND   7FH              ;TURN OFF HIGH BIT
      OR    (HL)             ;OR IN MANTISSA SIGN
      LD    (HL),A           ;STORE IN FACC
      XOR   A                ;TURN CY OFF, CLEAR ACC
FIN20 EX    DE,HL            ;FLIP/FLOP
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
FOUT  EQU   $
;
;FLOATING POINT OUTPUT FORMAT ROUTINE
;
;THIS SUBROUTINE CONVERTS A NUMBER IN THE FLOATING POINT AC
;TO A FORMAT SUITABLE FOR PRINTING. THAT IS, THE NUMBER WIL
;SCIENTIFIC NOTATION (+N.NNNNNE+NN) IF THE EXPONENT IS > 5
;OTHERWISE IT WILL BE ZERO SUPPRESSED BOTH ON THE LEFT OF T
;PORTION AND ON THE RIGHT OF THE FRACTION.
;
      LD    DE,FACC          ;POINT TO FLOATING POINT ACCUMULATOR
      LD    A,(DE)           ;GET EXPONENT BYTE
      LD    C,A              ;SAVE IT
      RLA                    ;SHIFT (TEST MANTISSA SIGN)
      LD    (HL),' '         ;DEFAULT POSITIVE
      JP    NC,FOUT1         ;BRIF POSITIVE
      LD    (HL),'-'         ;MOVE DASH
FOUT1 INC   DE               ;POINT TO FIRST & SECOND DIGITS
      INC   HL               ;AND NEXT OUTPUT POSITION
      LD    A,(DE)           ;PUT TO ACC
      CALL  RIGHT            ;SHIFT RIGHT
      OR    '0'              ;DECIMAL ZONE
      LD    (HL),A           ;PUT OUT
      INC   HL               ;POINT NEXT OUT
      LD    (HL),'.'         ;MOVE DECIMAL POINT
      LD    B,3              ;INIT LOOP COUNTER
      JP    FOUT3            ;JUMP INTO MIDDLE OF LOOP
FOUT2 INC   HL               ;POINT NEXT OUT
      INC   DE               ;NEXT 2 DIGITS
      LD    A,(DE)           ;GET HIGH & LOW
      CALL  RIGHT            ;SHIFT RIGHT
      OR    '0'              ;DECIMAL ZONE
      LD    (HL),A           ;PUT TO OUTPUT
FOUT3 INC   HL               ;POINT NEXT OUTPUT
      LD    A,(DE)           ;GET DIGITS AGAIN
      AND   0FH              ;MASK OFF HIGH
      OR    '0'              ;DECIMAL ZONE
      LD    (HL),A           ;PUT TO OUTPUT
      DEC   B                ;TEST LOOP COUNTER
      JP    NZ,FOUT2         ;BRIF MORE
      INC   HL               ;POINT NEXT OUTPUT
      LD    (HL),'E'         ;MOVE LIT E
      INC   HL               ;POINT NEXT
      LD    A,C              ;GET EXPONENT BYTE
      AND   3FH              ;MASK OFF SIGNS
      LD    B,A              ;SAVE IN B
      LD    A,C              ;GET EXPONENT BYTE
      RLA                    ;IGNORE MANTISSA SIGN
      RLA                    ;TEST EXPONENT SIGN
      LD    (HL),'+'         ;DEFAULT POSITIVE
      JP    NC,FOUT4         ;BRIF POSITIVE
      LD    (HL),'-'         ;ELSE MOVE DASH
      LD    A,C              ;RELOAD EXPONENT BYTE
      OR    0C0H             ;SET ALL ON
      CPL                    ;COMPLEMENT ACC
      INC   A                ;PLUS 1 (TWOS COMPLEMENT)
      LD    B,A              ;SAVE IN B
FOUT4 INC   HL               ;POINT NEXT OUT
      LD    A,B              ;GET EXPONENT
      LD    B,2FH            ;INIT COUNTER
FOUT5 SUB   10               ;SUBTRACT 10
      INC   B                ;COUNT 1
      JP    NC,FOUT5         ;BRIF NOT NEG
      LD    (HL),B           ;POINT TO OUTPUT
      INC   HL               ;POINT NEXT
      ADD   A,58             ;ADJUST
      LD    (HL),A           ;MOVE 2ND DIGIT
      LD    A,C              ;GET EXPONENT
      RLA                    ;SHIFT OFF MANTISSA SIGN
      OR    A                ;TEST
      JP    P,FOUT6          ;BRIF POSITIVE
      SCF                    ;SET CY
      RRA                    ;SHIFT BACK
      CP    -2               ;TEST FOR MIN
      RET   C                ;RETURN IF LESS THAN -2
      JP    FOUT7            ;GO AROUND
FOUT6 RRA                    ;SHIFT BACK
      CP    6                ;TEST IF TOO BIG
      RET   NC               ;RETURN IF 6 OR GREATER
FOUT7 LD    C,A              ;SAVE EXPONENT
      LD    B,4              ;CTR
FOUT8 LD    (HL),' '         ;SPACE OUT EXPONENT
      DEC   HL               ;POINT PRIOR
      DEC   B                ;DECR CTR
      JP    NZ,FOUT8         ;LOOP
      EX    DE,HL            ;FLIP/FLOP
      LD    A,E              ;GET LOW BYTE
      SUB   5                ;POINT TO DOT
      LD    L,A              ;PUT DOWN
      LD    A,D              ;GET HIGH
      SBC   A,0              ;IN CASE OF BORROW
      LD    H,A              ;PUT DOWN
      LD    A,C              ;GET EXPONENT
      OR    A                ;TEST SIGN
      JP    Z,FOX1           ;BRIF ZERO
      JP    M,FOX2           ;BRIF NEGATIVE
FOUT9 LD    B,(HL)           ;GET HIGH BYTE
      INC   HL               ;POINT NEXT
      LD    A,(HL)           ;GET LOW BYTE
      LD    (HL),B           ;SHIFT DOT TO RIGHT
      DEC   HL               ;POINT BACK
      LD    (HL),A           ;MOVE THE DIGIT LEFT
      INC   HL               ;POINT NEXT
      DEC   C                ;DECR CTR
      JP    NZ,FOUT9         ;LOOP
FOX1  EX    DE,HL            ;POINT END
FOX3  LD    A,(HL)           ;GET A DIGIT/DOT
      CP    '0'              ;TEST FOR A TRAILING ZERO
      JP    NZ,FOX4          ;BRIF NOT
      LD    (HL),' '         ;SPACE FILL
      DEC   HL               ;POINT PRIOR
      JP    FOX3             ;LOOP
FOX4  CP    '.'              ;TEST FOR TRAILING DOT
      RET   NZ               ;RETURN IF NOT
      LD    (HL),' '         ;SPACE IT OUT
      DEC   HL               ;POINT PRIOR
      RET                    ;RETURN
FOX2  CP    0FFH             ;TEST IF -1
      JP    NZ,FOX5          ;ELSE -2
      DEC   HL               ;POINT SIGNIFICANT
      LD    A,(HL)           ;GET THE CHAR
      LD    (HL),'.'         ;MOVE THE DOT
      INC   HL               ;POINT NEXT
      LD    (HL),A           ;SHIFT THE DIGIT
      JP    FOX1             ;GO ZERO SUPPRESS
FOX5  DEC   HL               ;POINT ONE TO LEFT
      LD    A,(HL)           ;PICK UP DIGIT
      LD    (HL),'0'         ;REPLACE
      INC   HL               ;POINT RIGHT
      LD    (HL),A           ;PUT THE DIGIT
      LD    H,D              ;GET LOW ADDR
      LD    L,E              ;POINT LAST DIGIT
      LD    B,6              ;CTR
FOX6  DEC   HL               ;POINT PRIOR
      LD    A,(HL)           ;GET A DIGIT
      INC   HL               ;POINT
      LD    (HL),A           ;PUT IT ONE TO RIGHT
      DEC   HL               ;POINT
      DEC   B                ;DECR CTR
      JP    NZ,FOX6          ;LOOP
      LD    (HL),'.'         ;MOVE THE DOT
      JP    FOX1             ;CONTINUE
*HEADING IMSAI 8080 4K BASIC
FADD  EQU   $
;
;
;FLOATING POINT ADD THE NUMBER AT (HL) TO THE FACC
;
;
      INC   HL               ;POINT FIRST DIGIT
      LD    A,(HL)           ;LOAD IT
      OR    A                ;TEST IT
      RET   Z                ;RETURN IF ZERO
      DEC   HL               ;POINT BACK
      CALL  FTEST            ;GO TEST SIGN OF FACC
      JP    Z,RST5           ;JUST LOAD IF FACC = 0
      LD    DE,FACC          ;POINT FACC
      LD    A,(DE)           ;GET EXPONENT OF FACC
      CALL  FEXP             ;GO GET EXPONENT
      LD    B,A              ;SAVE EXPONENT
      LD    A,(HL)           ;GET EXPONENT OF ADDR
      CALL  FEXP             ;GO GET EXPONENT
      LD    C,A              ;SAVE THE EXPONENT
      SUB   B                ;GET DIFFERENCE OF TWO EXPONENTS
      JP    Z,FADD4          ;BRIF THEY'RE EQUAL
      JP    P,FADD3          ;BRIF DIFFERENCE IS POSITIVE
      CPL                    ;COMPLEMENT ACC
      INC   A                ;PLUS ONE (TWO'S COMPLEMENT)
FADD3 CP    6                ;COMPARE DIFFERENCE TO SIX
      JP    C,FADD4          ;BRIF 5 OR LESS
      LD    A,B              ;GET EXPON OF ADDUEND
      SUB   C                ;GET TRUE DIFFERENCE AGAIN
      RET   P                ;RETURN IF FACC > ADDER
      JP    RST5             ;ELSE, ADDER > FACC
FADD4 PUSH  AF               ;SAVE DIFFERENCE
      PUSH  BC               ;SAVE EXPONENTS
      LD    DE,FTEMP         ;GET ADDR OF TEMP ACC
      LD    B,4              ;FOUR BYTES
      CALL  COPYH            ;GO COPY
      POP   BC               ;GET EXPONENTS
      POP   AF               ;GET DIFFERENCE
      JP    Z,FADD9          ;JUST ADD IF ZERO
      LD    HL,FTEMP+1       ;DEFAULT
      PUSH  AF               ;SAVE DIFFERENCE
      LD    A,B              ;GET FACC EXPON
      SUB   C                ;MINUS FTEMP EXPON
      JP    P,FADD6          ;BRIF TEMP MUST BE SHIFTED
      LD    HL,FACC          ;POINT FLOAT ACC
      LD    A,C              ;GET EXPONENT, SIGN
      AND   7FH              ;STRIP EXP SIGN
      LD    C,A              ;PUT BACK
      LD    A,(HL)           ;GET THE EXP
      AND   80H              ;STRIP OFF OLD EXPON
      OR    C                ;MOVE ADDER EXPON TO IT
      LD    (HL),A           ;REPLACE
      INC   HL               ;POINT FIRST DATA BYTE
FADD6 POP   AF               ;GET DIFFER
      LD    C,A              ;SAVE IT
FADD7 LD    B,3              ;LOOP CTR (INNER)
      LD    D,0              ;INIT CARRY OVER TO ZERO
      PUSH  HL               ;SAVE ADDR
      CALL  FSHFT            ;GO SHIFT
      POP   HL               ;GET ADDR
      DEC   C                ;DECR CTR
      JP    NZ,FADD7         ;LOOP
FADD9 EQU   $
      LD    DE,FACC          ;POINT SIGN OF ADDUEND
      LD    HL,FTEMP         ;AND SIGN OF ADDER
      LD    A,(DE)           ;GET SIGN OF ADDUEND
      XOR   (HL)             ;COMPARE THE TWO SIGNS
      JP    M,FADD1          ;BRIF SIGNS DIFFER
      LD    DE,FACC+3        ;POINT LOW END
      LD    HL,FTEMP+3       ;DITTO
      LD    B,3              ;THREE BYTES
      CALL  FADDT            ;GO ADD TWO TOGETHER
      RET   NC               ;RETURN IF NO CARRY
FADX1 LD    HL,FACC          ;GET ADDR OF ACC
      LD    A,(HL)           ;LOAD THE EXPON
      AND   80H              ;ISOLATE SIGN
      LD    B,A              ;SAVE SIGN
      LD    A,(HL)           ;GET EXPON
      CALL  FEXP             ;GO GET EXPONENT
      INC   A                ;ADD ONE
      AND   7FH              ;ISOLATE
      OR    B                ;PUT BACK SIGN
      LD    (HL),A           ;PUT IT DOWN
      INC   HL               ;POINT DATA
      LD    D,10H            ;(THE CARRY)
      LD    B,3              ;CTR
      CALL  FSHFT            ;GO SHIFT IT
      RET                    ;RETURN
FADD1 EQU   $
      LD    HL,FTEMP+4       ;POINT TEMP2 AREA
      LD    B,4              ;PREPARE TO SAVE ACC
      CALL  COPYD            ;GO COPY
FADX2 LD    DE,FACC+3        ;POINT LOW ACC
      LD    HL,FTEMP+3          ;AND LOW TEMP
      LD    B,3              ;CTR
      CALL  FSUBT            ;GO SUBTRACT THE TWO
      JP    NC,FNORM         ;BRIF NO BORROW
      LD    DE,FACC          ;POINT ACC
      LD    HL,FTEMP         ;POINT TEMP
      LD    B,8              ;CTR
      CALL  COPYH            ;GO COPY
      LD    DE,FACC          ;POINT
      LD    HL,FTEMP         ;TEMP
      LD    A,(HL)           ;GET ORIG ACC EXPONENT
      XOR   80H              ;REVERSE SIGN
      LD    (DE),A           ;PUT TO NEW ACC
      JP    FADX2            ;GO SUBTRACT AGAIN
*HEADING IMSAI 8080 4K BASIC
FNORM EQU   $
;
;
;NORMALIZE THE FLOATING ACCUMULATOR
;THAT IS, THE FIRST DIGIT MUST BE SIGNIFICANT
;
;
      LD    HL,FACC+1        ;POINT TO FIRST BYTE
      LD    A,(HL)           ;LOAD IT
      AND   0F0H             ;ISOLATE
      RET   NZ               ;RETURN IF ALREADY NORMALIZED
      LD    A,(HL)           ;GET THE BYTE
      INC   HL               ;POINT NEXT
      OR    (HL)             ;OR THE NEXT BYTE
      INC   HL               ;POINT LAST
      OR    (HL)             ;OR THAT BYTE (ACC HAS LOGICAL S
      JP    NZ,FNOR1         ;BRIF NOT ZERO
      LD    HL,FACC          ;ELSE POINT FLOAT ACC
      LD    (HL),0           ;CLEAR THE EXPONENT
      RET                    ;RETURN
FNOR1 LD    HL,FACC+3        ;POINT LST BYTE
      LD    B,3              ;3 BYTE LOOP
      LD    D,0              ;INIT CARRY OVER
FNOR2 LD    A,(HL)           ;GET A BYTE
      LD    C,A              ;SAVE IT
      RST   RST4             ;SHIFT LEFT 4 BITS
      OR    D                ;PLUS PREV SHIFT OUT
      LD    (HL),A           ;PUT BACK
      LD    A,C              ;GET SAVED BYTE
      CALL  RIGHT            ;SHIFT RIGHT 4 BITS
      LD    D,A              ;SAVE FOR NEXT TIME
      DEC   HL               ;POINT NEXT BYTE
      DEC   B                ;DECR CTR
      JP    NZ,FNOR2         ;LOOP
      LD    A,(HL)           ;GET EXPONENT
      AND   80H              ;ISOLATE SIGN
      LD    B,A              ;SAVE
      LD    A,(HL)           ;GET AGAIN
      CALL  FEXP             ;GO GET EXPONENT
      DEC   A                ;MINUS ONE
      AND   7FH              ;TURN OFF HIGH BIT
      OR    B                ;PLUS SAVED SIGN
      LD    (HL),A           ;PUT BACK
      JP    FNORM            ;GO NORMALIZE
*HEADING IMSAI 8080 4K BASIC
FSUB  EQU   $
;
;
;FLOATING POINT SUBTRACT THE NUMBER AT (HL) FROM THE FACC
;
;
      INC   HL               ;POINT FIRST DATA BYTE OF SUBTRA
      LD    A,(HL)           ;LOAD IT
      OR    A                ;TEST
      RET   Z                ;RETURN IF ZERO
      DEC   HL               ;POINT BACK
      LD    DE,FTEMP         ;GET TEMPORARY STORAGE AREA
      LD    B,4              ;FOUR BYTES
      CALL  COPYH            ;GO COPY
      LD    HL,FTEMP         ;POINT NEW AREA
      LD    A,(HL)           ;GET EXPONENT
      XOR   80H              ;REVERSE SIGN
      LD    (HL),A           ;REPLACE
      JP    FADD             ;GO ADD THE TWO
*HEADING IMSAI 8080 4K BASIC
FMUL  EQU   $
;
;
;FLOATING POINT MULTIPLY THE NUMBER AT (HL) TO THE FACC
;
;
      CALL  FTEST            ;TEST FACC
      RET   Z                ;RETURN IF ZERO
      INC   HL               ;POINT 1ST DIGIT OF MULTIPLIER
      LD    A,(HL)           ;LOAD IT
      DEC   HL               ;RESTORE
      OR    A                ;TEST IF ZERO
      JP    Z,RST5           ;GO LOAD TO FACC IF IT IS
      LD    DE,FACC          ;POINT EXP OF FACC
      LD    A,(DE)           ;LOAD EXPONENT
      OR    A                ;TEST IF 10 TO 0
      JP    NZ,FMUL1         ;BRIF NOT
      INC   DE               ;POINT NEXT
      LD    A,(DE)           ;LOAD IT
      CP    10H              ;TEST IF 1
      JP    NZ,FMUL1         ;BRIF NOT
      INC   DE               ;POINT NEXT
      LD    A,(DE)           ;LOAD IT
      OR    A                ;TEST IF ZERO
      JP    NZ,FMUL1         ;BRIF NOT
      INC   DE               ;POINT NEXT
      LD    A,(DE)           ;LOAD IT
      OR    A                ;TEST IF ZERO
      JP    Z,RST5           ;GO LOAD IF FACC = 1.00000
FMUL1 LD    DE,FACC          ;POINT EXPONENT
      LD    A,(DE)           ;LOAD IT
      CALL  FEXP             ;GO GET EXPONENT
      LD    B,A              ;SAVE IN B
      LD    A,(HL)           ;GET EXPONENT OF MULTIPLIER
      CALL  FEXP             ;GO GET EXPONENT
      SCF                    ;TURN ON CY
      ADC   A,B              ;ADD EXPONENTS TOGETHER
      CALL  FOVUN            ;GO SEE IF OVERFLOW/UNDERFLOW
      AND   7FH              ;TURN OFF SIGN
      LD    B,A              ;SAVE
      LD    A,(DE)           ;GET SIGN OF FACC
      XOR   (HL)             ;PRODUCT SIGN IS NEG IF TWO SIGN
      AND   80H              ;MASK
      OR    B                ;PUT SIGN AND EXPONENT TOGETHER
      LD    (DE),A           ;PUT IN FACC
      PUSH  HL               ;SAVE HL
      LD    HL,FTEMP         ;POINT DIGIT 7 OF RESULT
      LD    B,6              ;LOOP CTR
      CALL  ZEROM            ;GO ZERO 6 BYTES
      LD    DE,FACC+1        ;POINT 1ST DIGIT OF ACC
      LD    B,3              ;LOOP CTR
FMUL5 LD    A,(DE)           ;GET AN ACC DIGIT PAIR
      LD    (HL),A           ;PUT TO TEMP STORAGE
      XOR   A                ;ZERO A
      LD    (DE),A           ;CLEAR ACC
      INC   DE               ;POINT NEXT
      INC   HL               ;DITTO
      DEC   B                ;DECR CTR
      JP    NZ,FMUL5         ;LOOP
      LD    C,6              ;OUTER LOOP CTR
      POP   HL               ;GET ADDR OF MULTIPLIER
      RST   RST7             ;GO BUMP HL
      DEFB  3                ;BY THREE
FMUL6 LD    A,C              ;GET CTR
      RRA                    ;TEST IF EVEN/ODD
      LD    A,(HL)           ;GET MULTIPLIER DIGIT PAIR
      JP    C,FMUL7          ;BRIF LEFT NEEDED
      AND   0FH              ;MASK
      JP    FMUL8            ;GO AROUND
FMUL7 CALL  RIGHT            ;SHIFT RIGHT 4 BITS
FMUL8 LD    B,A              ;SAVE DIGIT
      PUSH  HL               ;SAVE ADDRESS
      PUSH  BC               ;SAVE COUNTERS
      LD    C,B              ;SWAP B/C
      OR    A                ;TEST MULTIPLIER
      JP    Z,FMUX1          ;BRIF ZERO
FMUL9 LD    DE,FTEMP+2       ;POINT PRODUCT
      LD    HL,FTEMP+8       ;POINT MULTIPLICAND
      LD    B,6              ;6 DIGITS PARTICIPATE
      CALL  FADDT            ;GO ADD
      DEC   C                ;DECR OUTER LOOP CTR
      JP    NZ,FMUL9         ;LOOP
FMUX1 LD    D,0              ;INIT SHIFT DIGIT
      LD    B,6              ;LOOP CTR
      LD    HL,FTEMP+8       ;POINT MULTIPLICAND
      CALL  FSHFX            ;GO SHIFT
      POP   BC               ;RESTORE CTRS
      POP   HL               ;ANDADDRESS
      DEC   C                ;DECR CTR
      JP    Z,FMUX2          ;GO AROUND IF ZERO
      LD    A,C              ;LOAD THE CTR
      RRA                    ;TEST EVEN/ODD
      JP    C,FMUL6          ;LOOP IF ODD
      DEC   HL               ;ELSE, POINT NEXT
      JP    FMUL6            ;LOOP
FMUX2 LD    HL,FACC+1        ;POINT MSD OF PRODUCT
      LD    A,(HL)           ;GET MSD PAIR
      AND   0F0H             ;ISOLATE LEFT HALF
      JP    NZ,FMUX3         ;BRIF NORMALIZED
      LD    B,5              ;CTR
      LD    D,H              ;COPY HL
      LD    E,L              ;TO DE
FMUX4 LD    A,(HL)           ;GET A PAIR OF DIGITS
      RST   RST4             ;SHIFT RIGHT TO LEFT
      LD    C,A              ;SAVE DIGIT
      INC   HL               ;POINT NEXT PAIR
      LD    A,(HL)           ;GET NEXT PAIR
      CALL  RIGHT            ;SHIFT LEFT TO RIGHT
      OR    C                ;COMBINE
      LD    (DE),A           ;PUT DOWN
      INC   DE               ;POINT NEXT OUTPUT PAIR
      DEC   B                ;DECR CTR
      JP    NZ,FMUX4         ;LOOP
      LD    A,(HL)           ;GET LAST PAIR
      RST   RST4             ;SHIFT LEFT
      LD    (DE),A           ;PUT DOWN
      LD    A,(FACC)         ;GET EXPON & SIGN
      LD    C,A              ;SAVE
      AND   80H              ;ISOLATE SIGN
      LD    B,A              ;SAVE SIGN
      LD    A,C              ;GET EXPON & SIGN
      CALL  FEXP             ;GO GET EXPON
      DEC   A                ;SUBTRACT ONE
      AND   7FH              ;STRIP 8TH BIT
      OR    B                ;MERGE IN SIGN BIT
      LD    (FACC),A         ;PUT DOWN
      JP    FMUX2            ;CONTINUE
FMUX3 LD    A,(FTEMP)        ;GET 1ST DIGIT PAIR FOLLOWING FA
      ADD   A,50H            ;ADD 5
      DAA                    ;ADJUST
      JP    NC,FNORM         ;BRIF 4 OR LESS
FROUN LD    HL,FACC+3        ;ELSE, POINT LSD OF FACC
      LD    B,3              ;LOOP CTR
      SCF                    ;TURN ON CY INDICATOR
FMUX5 LD    A,(HL)           ;GET A DIGIT PAIR
      ADC   A,0              ;ADD THE CARRY
      DAA                    ;ADJUST
      LD    (HL),A           ;PUT BACK
      DEC   HL               ;POINT NEXT
      DEC   B                ;DECR CTR
      JP    NZ,FMUX5         ;LOOP
      JP    C,FADX1          ;BRIF CARRY INTO 7 DIGITS
      JP    FNORM            ;GO NORMALIZE
*HEADING IMSAI 8080 4K BASIC
FDIV  EQU   $
;
;
;FLOATING POINT DIVIDE THE NUMBER AT (HL) INTO FACC
;
;
      CALL  FTEST            ;TEST IF FACC ZERO
      RET   Z                ;RETURN IF ZERO
      INC   HL               ;POINT 1ST DIGIT OF DIVISOR
      LD    A,(HL)           ;LOAD IT
      DEC   HL               ;POINT BACK
      OR    A                ;TEST IF ZERO
      JP    Z,OVERR          ;DIVISION BY ZERO = ERROR
      LD    A,(HL)           ;LOAD EXPONENT OF DIVISOR
      CALL  FEXP             ;GO GET EXPON
      LD    B,A              ;SAVE IT
      LD    DE,FACC          ;POINT EXPONENT OF DIVIDEND
      LD    A,(DE)           ;LOAD IT
      CALL  FEXP             ;GO GET EXPON
      SUB   B                ;SUBTRACT THE TWO EXPONENTS
      CALL  FOVUN            ;GO SAE IF OVERFLOW/UNDERFLOW
      AND   7FH              ;TRUNCATE TO 7 BITS
      LD    B,A              ;SAVE IT
      LD    A,(DE)           ;GET EXPONENT
      XOR   (HL)             ;IF SIGNS ARE EQUAL, RESULT IS P
      AND   80H              ;MASK OFF UNUSED BITS
      OR    B                ;CREATE SIGN OF QUOTIENT
      LD    (DE),A           ;PUT TO FACC
      PUSH  HL               ;SAVE ADDR
      INC   DE               ;POINT MSD OF DIVIDEND
      LD    HL,FTEMP         ;POINT TEMPORARY STORAGE
      LD    (HL),0           ;CLEAR HIGH ORDER POSITION
      INC   HL               ;POINT NEXT
      LD    B,3              ;LOOP CTR
FDIV3 LD    A,(DE)           ;GET BYTE FROM FACC
      LD    (HL),A           ;PUT TO FTEMP
      XOR   A                ;CLEAR A
      LD    (DE),A           ;ZERO FACC
      INC   HL               ;POINT NEXT
      INC   DE               ;DITTO
      DEC   B                ;DECR CTR
      JP    NZ,FDIV3         ;LOOP
      LD    (DIVSW),A        ;RESET SWITCH
      LD    (HL),A           ;CLEAR HIGH PAIR OF DIVISOR
      POP   DE               ;GET ADDR
      LD    B,3              ;LOOP CTR
      INC   DE               ;POINT MSD OF DIVISOR
      INC   HL               ;AND OF DIVIDEND
      CALL  COPYD            ;GO MOVE IT
      LD    C,6              ;OUTER LOOP CTR
FDIV5 LD    B,-1             ;INIT CTR
FDIV7 LD    DE,FTEMP+3       ;POINT DIVIDEND
      LD    HL,FTEMP+7       ;POINT DIVISOR
      PUSH  BC               ;SAVE BC
      LD    B,4              ;LOOP CTR
      CALL  FSUBT            ;GO SUBTRACT THE TWO
      POP   BC               ;GET COUNTERS
      INC   B                ;COUNT ONE MORE
      JP    NC,FDIV7         ;LOOP IF NOT TOO FAR
      LD    A,(DIVSW)        ;GET SWITCH
      OR    A                ;TEST IT
      JP    NZ,FDIV1         ;BRIF SET
      PUSH  BC               ;SAVE BC
      LD    C,3              ;THREE BYTE LOOP
      LD    HL,FACC+3        ;POINT LSD OF QUOTIENT
FDIX1 LD    A,(HL)           ;GET DIGIT PAIR
      LD    D,A              ;SAVE IT
      RST   RST4             ;SHIFT LEFT
      OR    B                ;MERGE WITH PREV
      LD    (HL),A           ;PUT BACK
      LD    A,D              ;GET SAVED PAIR
      CALL  RIGHT            ;SHIFT RIGHT
      LD    B,A              ;SAVE IT
      DEC   HL               ;POINT NEXT
      DEC   C                ;DECR CTR
      JP    NZ,FDIX1         ;LOOP
      POP   BC               ;GET CTRS
      LD    DE,FTEMP+3       ;POINT PREV
      LD    HL,FTEMP+7       ;POINT DIVISOR
      LD    B,4              ;LOOP CTR
      CALL  FADDT            ;GO ADD
      LD    B,4              ;INNER CTR
      LD    HL,FTEMP+3       ;POINT LSD OF DIVIDEND
      LD    D,0              ;SAVE DIGIT
      CALL  FSHFX            ;GO SHIFT
      DEC   C                ;DECR OUTER CTR
      JP    NZ,FDIV5         ;LOOP IF NOT ZERO
      LD    A,(FACC+1)       ;GET MSD OF QUOTIENT
      AND   0F0H             ;ISOLATE LEFT HALF
      JP    NZ,FDIX2         ;BRIF NORMALIZED
      LD    A,(FACC)         ;GET EXPON & SIGN
      LD    B,A              ;SAVE
      AND   80H              ;ISOLATE SIGN
      LD    C,A              ;SAVE
      LD    A,B              ;GET EXPON & SIGN
      CALL  FEXP             ;GO GET EXPONENT
      DEC   A                ;SUBTRACT ONE
      AND   7FH              ;TRUNCATE 8TH BIT
      OR    C                ;MERGE SIGN BIT
      LD    (FACC),A         ;PUT DOWN
      LD    C,1              ;NEW LOOP CTR
      JP    FDIV5            ;ONE MORE TIME
FDIX2 LD    A,1              ;GET A ONE
      LD    (DIVSW),A        ;SET SWITCH
      JP    FDIV5            ;GO ONE MORE DIGIT
FDIV1 LD    A,B              ;GET THE EXTRA QUOTIENT DIGIT
      CP    5                ;COMPARE TO 5
      JP    C,FNORM          ;BRIF LESS
      JP    FROUN            ;ELSE, GO ROUND IT
FOVUN EQU   $                ;TEST IF EXPONENT OVERFLOW/UNDER
      JP    P,FOVUX          ;BRIF POSITIVE
      CP    0C1H             ;TEST FOR UNDERFLOW
      RET   NC               ;RETIFNOT UNDERFLOW
      JP    OVERR            ;ELSE, ERROR
FOVUX CP    40H              ;TEST IF OVERFLOW
      RET   C                ;RETIF LESS
      JP    OVERR            ;ELSE, OVER/UNDEFLOW
*HEADING IMSAI 8080 4K BASIC
FTEST EQU   $
;
;TEST THE SIGN OF THE NUMBER IN THE FACC
;RETURN WITH S & Z ZET TO SIGN
;
      LD    A,(FACC+1)       ;GET MSD
      OR    A                ;TEST IT
      RET   Z                ;RETURN IF ZERO
      LD    A,(FACC)         ;GET SIGN & EXPON BYTE
      OR    7FH              ;TEST SIGN BIT ONLY
      LD    A,(FACC)         ;RE-LOAD EXPON BYTE
      RET                    ;THEN RETURN
*HEADING IMSAI 8080 4K BASIC
FEXP  EQU   $
;
;EXPAND EXPONENT INTO 8 BINARY BITS
;
      RLA                    ;DROP MANTISSA SIGN
      OR    A                ;TEST SIGN OF EXPON
      JP    P,FEXP1          ;BRIF POSITIVE
      SCF                    ;ELSE, TURN ON CY
FEXP1 RRA                    ;SHIFT BACK
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
FSUBT EQU   $
;
;DECIMAL SUBTRACT THE TWO 6 DIGIT NUMBERS (DE) & (HL)
;
      XOR   A                ;CLEAR STATUS
FSUX1 PUSH  BC               ;SAVE CTR
      LD    A,(DE)           ;GET ACC DIGIT PAIR
      SBC   A,(HL)             ;SUBTRACT PAIR FROM SUBTRAHEND
      PUSH  AF               ;SAVE A, FLAGS
      POP   BC               ;GET A, FLAGS IN BC
      LD    A,C              ;GET FLAGS
      AND   10H              ;TEST AC STATUS
      JP    NZ,FSUX2         ;BRIF SET
      LD    A,B              ;GET DIFFERENCE
      SUB   06H              ;ADJUST RIGHT SIDE
      LD    B,A              ;SAVE
FSUX2 LD    A,C              ;GET FLAGS
      RRA                    ;TEST CY
      JP    NC,FSUX3         ;BRIF NOT SET
      LD    A,B              ;GET DIFF
      SUB   60H              ;ADJUST LEFT SIDE
      LD    B,A              ;SAVE
FSUX3 PUSH  BC               ;RESAVE A, FLAGS
      POP   AF               ;RE-LOAD DIFFERENCE, FLAGS
      LD    (DE),A           ;PUT TO ACC
      POP   BC               ;GET BC
      DEC   DE               ;POINT PRIOR
      DEC   HL               ;DITTO
      DEC   B                ;DECR CTR
      JP    NZ,FSUX1         ;LOOP
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
FADDT EQU   $
;
;ADD TWO DECIMAL NUMBERS (DE) & (HL)
;
      XOR   A                ;CLEAR STATUS
FADXT LD    A,(DE)           ;GET PAIR
      ADC   A,(HL)           ;ADD OTHER PAIR
      DAA                    ;ADJUST
      LD    (DE),A           ;PUT DOWN
      DEC   DE               ;POINT NEXT
      DEC   HL               ;DITTO
      DEC   B                ;DECR LOOP CTR
      JP    NZ,FADXT         ;LOOP
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
FSHFT EQU   $
;
;INCREMENTING SHIFT RIGHT
;
      LD    A,(HL)           ;GET A BYTE
      LD    E,A              ;SAVE IT
      CALL  RIGHT            ;SHIFT RIGHT
      OR    D                ;PLUS PREV
      LD    (HL),A           ;STORE
      LD    A,E              ;GET PREV
      RST   RST4             ;SHIFT LEFT
      LD    D,A              ;SAVE FOR NEXT
      INC   HL               ;POINT NEXT
      DEC   B                ;DECR CTR
      JP    NZ,FSHFT         ;LOOP
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
FSHFX EQU   $
;
;DECREMENTING SHIFT RIGHT
;
      LD    A,(HL)           ;GET A BYTE
      CALL  RIGHT            ;SHIFT RIGHT
      LD    E,A              ;SAVE IT
      LD    A,(HL)           ;RELOAD
      RST   RST4             ;SHIFT LEFT
      OR    D                ;MERGE
      LD    (HL),A           ;REPLACE
      LD    D,E              ;UPDATE SAVED
      DEC   HL               ;POINT NEXT
      DEC   B                ;DECR CTR
      JP    NZ,FSHFX         ;LOOP
      RET                    ;RETURN
;
;
*HEADING IMSAI 8080 4K BASIC
ABS   EQU   $
;
;
;RETURN THE ABSOLUTE VALUE OF THE FLOATING ACCUMULATOR
;
;
      LD    A,(FACC)         ;GET EXPONENT
      AND   7FH              ;STRIP NEGATIVE SIGN
      LD    (FACC),A         ;REPLACE
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
SGN   EQU   $
;
;
;RETURNS THE SIGN OF THE FLOATING ACCUMULATOR
;THAT IS:
; 1 IF FACC > 0
; 0 IF FACC = 0
;-1 IF FACC < 0
;
      CALL FTEST             ;GO TEST FACC
      RET   Z                ;RETURN IF ZERO
      AND   80H              ;ISOLATE IT
      PUSH  AF               ;SAVE IT
      LD    HL,ONE           ;GET ADDRESS OF CONSTANT 1
      RST   RST5             ;GO LOAD IT
      POP   AF               ;RESTORE SIGN
      LD    (FACC),A         ;SET THE SIGN & EXPONENT
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
INT   EQU   $
;
;
;RETURNS THE GREATEST INTEGER NOT LARGER THAN THE ABSOLUTE VALUE
;
;
      LD    HL,FACC          ;POINT FLOAT ACC
      LD    A,(HL)           ;GET EXPONENT
      AND   40H              ;GET SIGN OF CHARACTERISTIC
      JP    Z,INT2           ;BRIF GE ZERO
      LD    B,4              ;FOUR BYTE LOOP
      JP    ZEROM            ;ZERO FACC AND RETURN
INT2  LD    A,(HL)           ;GET EXPONENT
      AND   3FH              ;ISOLATE CHARACTERISTIC
      CP    5                ;TEST FOR FIVE OR LARGER
      RET   P                ;RETURN IF >= 5
      LD    B,A              ;SAVE EXPONENT
      LD    A,5              ;GET CONSTANT
      SUB   B                ;MINUS EXPONENT = LOOP CTR
      LD    B,A              ;SAVE IT
      LD    HL,FACC+3        ;POINT LSD
INT3  LD    A,(HL)           ;LOAD A BYTE
      AND   0F0H             ;DROP RIGHT HALF
      LD    (HL),A           ;PUT BACK
      DEC   B                ;DECR CTR
      RET   Z                ;RETURN IF ZERO
      LD    (HL),0           ;ZERO LEFT HALF
      DEC   HL               ;POINT NEXT
      DEC   B                ;DECR CTR
      JP    NZ,INT3          ;LOOP
      RET                    ;CONTINUE EVALUATION
*HEADING IMSAI 8080 4K BASIC
SQR   EQU   $
;
;
;COMPUTE THE SQUARE ROOT OF THE FACC
;USES NEWTON'S THIRD ORDER ITERATION
;
;
      CALL  FTEST            ;GO GET SIGN OF FACC
      JP    M,OVERR          ;BRIF SQUARE ROOT OF NEGATIVE
      RET   Z                ;RETURN IF SQUARE ROOT OF ZERO
      LD    HL,ORIGS         ;POINT TO TEMP AREA
      RST   RST6             ;SAVE ORIGINAL NUMBER
      LD    HL,ONE           ;POINT CONSTANT
      CALL  FADD             ;ADD ONE
      LD    HL,TWO           ;POINT CONSTANT
      CALL  FDIV             ;DIVIDE BY TWO
;
;FIRST APPROXIMATION = (X+1)/2
;
SQRLP LD    HL,TSTSQ         ;GET ADDR OF TEST
      RST   RST6             ;SAVE IT
      LD    HL,TSTSQ         ;POINT PREV ITERATION
      CALL  FMUL             ;SQUARE IT
      LD    HL,TST2S         ;POINT SAVE AREA
      RST   RST6             ;SAVE IT
      LD    HL,ORIGS         ;GET ORIGINAL NUMBER
      CALL  FSUB             ;SUBTRACT FROM PREV**2
      CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    M,SQRGO          ;BRIF PREV**2 < ORIGINAL
      JP    Z,SQRGO          ;BRIF PREV**2 = ORIGINAL
      LD    HL,TST2S         ;GET PREV**2
      RST   RST5             ;GO LOAD IT
      LD    HL,THREE         ;POINT CONSTANT OF 3
      CALL  FMUL             ;MULTIPLY WITH PREV**2
      LD    HL,ORIGS         ;GET ORIGINAL NUMBER
      CALL  FADD             ;GO ADD
      LD    HL,SQRX          ;POINT TEMP AREA
      RST   RST6             ;SAVE DIVISOR
      LD    HL,THREE         ;POINT CONSTANT OF 3
      RST   RST5             ;GO LOAD IT
      LD    HL,ORIGS         ;GET ORIGINAL NUMBER
      CALL  FMUL             ;MULTIPLY BY THREE
      LD    HL,TST2S         ;GET SQUARE OF PREV ITERATION
      CALL  FADD             ;GO ADD IT
      LD    HL,TSTSQ         ;GET PREV ITERATION
      CALL  FMUL             ;GO MULTIPLY
      LD    HL,SQRX          ;POINT DIVISOR
      CALL  FDIV             ;GO DIVIDE
      LD    HL,SQRX          ;POINT TEMP AREA
      RST   RST6             ;SAVE IT
      LD    HL,TSTSQ         ;GET PREV ESTIMATE
      CALL  FSUB             ;GO COMPARE THEM
      LD    HL,SQRX          ;POINT THIS ANSWER
      CALL  FTEST            ;GET SIGN OF DIFFERENCE
      JP    Z,SQRGX          ;BRIF SAME GUESS
      RST   RST5             ;ELSE, LOAD THIS GUESS
;NEXT ITERATION = PREV*(3*X+PREV**2)/(3*PREV**2+X)
      JP    SQRLP            ;LOOP
SQRGO LD    HL,TSTSQ         ;POINT SQUARE ROOT
SQRGX RST   RST5             ;GO LOAD ACC
      RET                    ;THEN RETURN
*HEADING IMSAI 8080 4K BASIC
NEG   EQU   $
;
;
;REVERSES THE SIGN OF THE FLOATING ACC
;
;
      CALL  FTEST            ;GET SIGN OF FACC
      RET   Z                ;RETURN IF ZERO
      XOR   80H              ;REVERSE SIGN
      LD    (FACC),A         ;RESTORE EXPONENT
      RET                    ;CONTINUE EVALUATION
*HEADING IMSAI 8080 4K BASIC
RND   EQU   $
;
;
;PSEUDO RANDOM NUMBER GENERATOR
;
;
      LD    HL,RNDNU         ;POINT PREV RND
      RST   RST5             ;LOAD TO FACC
      LD    HL,RNDX          ;POINT MULTIPLIER
      CALL  FMUL             ;GO MULTIPLY
      LD    HL,FACC          ;POINT RESULT
      LD    (HL),7FH         ;DEFAULT . XXXXXX
      INC   HL               ;POINT MSD
      LD    B,(HL)           ;LOAD IT
      INC   HL               ;POINT MSD+2
      LD    C,(HL)           ;LOAD IT
      LD    (HL),B           ;SWAP BYTES
      DEC   HL               ;POINT BACK MSD
      LD    (HL),C           ;MOV MSD+2
      CALL  FNORM            ;GO NORMALIZE
      LD    HL,RNDNU         ;POINT NEW RND NUMBER
      RST   RST6             ;GO STORE IT
      RET                    ;RETURN
*HEADING IMSAI 8080 4K BASIC
EXPR  EQU   $
;
;
;EVALUATE EXPRESSION ROUTINE
;LEAVE RESULT IN FACC
;RETURN WHEN EXPRESSION ENDS (TYPICALLY AT END OF LINE)
;
;
      XOR   A                ;CLEAR REG A
      LD    (PARCT),A        ;SET PAREN CTR
      LD    (SPCTR),A        ;SET STACK CTR
      EX    DE,HL            ;SAVE HL IN DE
      LD    HL,(PROGE)       ;POINT END OF PROGRAM AREA
      LD    (EXPRS),HL       ;SAVE IT
      EX    DE,HL            ;RESTORE HL
;
LOOKD EQU   $                ;LOOK FOR CONSTANT, VARIABLE, OR
      CALL  NUMER            ;GO TEST IF NUMERIC
      JP    NZ,LDALP         ;BRIF NOT
LDNUM CALL  FIN              ;GO CONVERT NUMERIC (PUT TO FACC
LDF   LD    B,H              ;COPY HL TO BC
      LD    C,L              ;SAME
      LD    HL,(EXPRS)       ;GET ADDR OF EXPR AREA
      CALL  GTEMP            ;GO STORE THE FACC IN TEMP AREA
      LD    (EXPRS),HL       ;SAVE UPDATED ADDRESS
      LD    H,B              ;RESTORE H
      LD    L,C              ;RESTORE L
      JP    LOOKO            ;GO GET AN OPERATION CODE
LDALP CP    '.'              ;SEE IF LEADING DECIMAL POINT
      JP    Z,LDNUM          ;BRIF IS
      CALL  ALPHA            ;GO SEE IF ALPHA
      JP    NZ,LDDTN         ;BRIF NOT
      LD    B,(HL)           ;SAVE 1ST CHAR
      INC   HL               ;POINT NEXT
      LD    C,' '            ;DEFAULT FOR 1 CHAR VAR
      CALL  NUMER            ;GO SEE IF 2ND IS NUMERIC
      JP    NZ,LDFN          ;BRIF NOT
      INC   HL               ;POINT NEXT
      LD    C,A              ;SAVE THE CHAR
LDVR1 PUSH  HL               ;SAVE HL
      LD    D,B              ;COPY BC
      LD    E,C              ;TO DE
      CALL  SEARC            ;GO GET VAR ADDR IN DE
      LD    HL,(EXPRS)       ;GET EXPR ADDR
      CALL  SADR             ;GO STORE ADDRESS
      LD    (EXPRS),HL       ;SAVE ADDRESS
      POP   HL               ;RESTORE HL
      JP    LOOKO            ;GO LOOK FOR OPCODE
LDFN  CALL  ALPHA            ;GO SEE IF FUNCTION
      JP    NZ,LDVR1         ;BRIF IT'S NOT
LDFN1 DEC   HL               ;POINT BACK TO 1ST
      LD    A,(HL)           ;GET THAT CHAR
      CP    ' '              ;TEST IF SPACE
      JP    Z,LDFN1          ;LOOP IF IS
      PUSH  HL               ;SAVE HL
      LD    DE,RNDLI         ;POINT LITERAL
      RST   RST2             ;GO COMPARE
      JP    Z,LDRND          ;BRIF RND
      POP   HL               ;GET HL
      PUSH  HL               ;RESAVE IT
      LD    DE,SQRLI         ;POINT LITERAL
      RST   RST2             ;GO COMPARE
      LD    BC,SQR           ;GET ADDR OF ROUTINE
      JP    Z,LDFNC          ;BRIF IS
      POP   HL               ;GET HL
      PUSH  HL               ;RESAVE
      LD    DE,INTLI         ;POINT
      RST   RST2             ;GO COMPARE
      LD    BC,INT           ;ROUTINE ADDR
      JP    Z,LDFNC          ;BRIF EQUAL
      POP   HL               ;GET HL
      PUSH  HL               ;SAVE IT
      LD    DE,ABSLI         ;LITERAL
      RST   RST2             ;COMPARE
      LD    BC,ABS           ;ROUTINE
      JP    Z,LDFNC          ;BRIF EQUAL
      POP   HL               ;GET HL
      PUSH  HL               ;SAVE IT
      LD    DE,SGNLI         ;LITERAL
      RST   RST2             ;GO COMPARE
      LD    BC,SGN           ;ROUTINE
      JP    Z,LDFNC          ;BRIF EQUAL
      POP   HL               ;GET HL
      LD    B,(HL)           ;GET 1ST CHAR
      LD    C,' '            ;SPACE 2ND CHAR
      INC   HL               ;POINT NEXT
      JP    LDVR1            ;BRIF VARIABLE
LDRND PUSH  HL               ;SAVE HL
      CALL  RND              ;GO GET RANDOM NUMBER
      POP   HL               ;RESTORE HL
      POP   DE               ;RESTORE STACK POINTER
      JP    LDF              ;ACT AS IF CONSTANT
LDFNC POP   DE               ;POP THE STACK
      EX    DE,HL            ;FLIP/FLOP
      LD    HL,(EXPRS)       ;GET ADDR
      INC   HL               ;POINT NEXT
      LD    (HL),B           ;HIGH ADDR
      INC   HL               ;POINT NEXT
      LD    (HL),C           ;LOW ADDR
      INC   HL               ;POINT NEXT
      LD    (HL),1           ;CODE
      LD    (EXPRS),HL       ;SAVE ADDR
      EX    DE,HL            ;RESTORE HL
      JP    LOOKD            ;NEXT MUST BE DATA TOO
LDDTN CP    '-'              ;TEST IF UNARY MINUS
      JP    NZ,LDDTP         ;BRIF NOT
      LD    BC,NEG           ;SET UP CALL
      INC   HL               ;POINT NEXT
      PUSH  HL               ;SAVE HL
      JP    LDFNC            ;GO AS IF FUNCTION
LDDTP CP    '('              ;TEST IF OPEN PAREN
      JP    NZ,SNERR         ;BRIF NOT CONSTANT, FUNCTION, OR
      LD    A,(PARCT)        ;GET OPEN COUNT
      INC   A                ;ADD ONE
      LD    (PARCT),A        ;STORE IT
      EX    DE,HL            ;SAVE HL
      LD    HL,(EXPRS)       ;GET ADDR
      INC   HL               ;POINT NEXT
      LD    (HL),'('         ;PUT CODE
      LD    (EXPRS),HL       ;SAVE ADDR
      EX    DE,HL            ;RESTORE HL
      INC   HL               ;POINT NEXT
      JP    LOOKD            ;NEXT HAS TO BE DATA TOO
LOOKO RST   RST1             ;SKIP BLANKS
      CP    '+'              ;TEST IF PLUS
      JP    Z,OP1            ;BRIF IS
      CP    '-'              ;TEST IF MINUS
      JP    Z,OP1            ;BRIF IS
      CP    '*'              ;TEST IF MULTIPLY
      JP    Z,OP2            ;BRIF IS
      CP    '/'              ;TEST IF DIVIDE
      JP    Z,OP2            ;BRIF IS
      CP    ')'              ;TEST IF CLOSE PAREN
      JP    Z,OP3            ;BRIF IS
;ELSE MUST BE END OF EXPRESSION
      LD    A,(PARCT)        ;GET OPEN PAREN COUNT
      OR    A                ;TEST IT
      JP    NZ,SNERR         ;BRIF # OF ('S NOT = # OF )'S
      LD    (ADDR3),HL       ;SAVE ADDR OF STMT
      JP    EVAL             ;GO EVALUATE
OP1   PUSH  HL               ;SAVE HL
      LD    C,(HL)           ;SAVE OPERATION
      LD    B,0              ;INIT CTR
      LD    HL,(EXPRS)       ;GET END POINTER
OP1L1 INC   B                ;COUNT ONE MORE
      LD    A,(HL)           ;LOAD TYPE CODE
      CP    '('              ;TEST IF OPEN PAREN
      JP    Z,INSOP          ;BRIF IS
      OR    A                ;TEST IF END BUFF
      JP    Z,INSOP          ;BRIF IS
      OR    A                ;TEST IF DATA
      JP    Z,OP1L2          ;BRIF IS
      CP    1                ;TEST IF FUNCT
      JP    NZ,OP1L3         ;BRIF NOT EQUAL
OP1L2 DEC   HL               ;POINT NEXT
      DEC   HL               ;DITTO
      INC   B                ;COUNT
      INC   B                ;TWO BYTES
OP1L3 DEC   HL               ;POINT NEXT OPCODE
      JP    OP1L1            ;LOOP
INSOP INC   HL               ;POINT FIRST CHAR
      LD    A,(HL)           ;PICK UP OLD VALUE
      LD    (HL),C           ;PUT PREV
      LD    C,A              ;ROTATE
      DEC   B                ;DECR COUNT
      JP    NZ,INSOP         ;LOOP
      LD    (EXPRS),HL       ;SAVE ADDR
      POP   HL               ;GET STMT POINTER
      INC   HL               ;POINT NEXT
      JP    LOOKD            ;NEXT IS DATA
OP2   PUSH  HL               ;SAV HL
      LD    C,(HL)           ;SAVE OPCODE
      LD    B,1              ;INIT CTR
      LD    HL,(EXPRS)       ;GET CURRENT END
OP2A  RST   RST7             ;GO BUMP HL
      DEFB  -3               ;BY NEG THREE
      INC   B                ;ADD
      INC   B                ;THREE
      INC   B                ;TO B
      LD    A,(HL)           ;GET TYPE CODE
      CP    1                ;SEE IF FUNCTION
      JP    Z,OP2A           ;BRIF IS
      JP    INSOP            ;GO INSERT OPCODE
OP3   LD    A,(PARCT)        ;GET OPEN PAREN COUNT
      DEC   A                ;SUBTRACT ONE
      LD    (PARCT),A        ;SAVE IT
      JP    M,SNERR          ;BRIF TOO MANY )'S
      INC   HL               ;POINT NEXT SOURCE
      LD    (ADDR3),HL       ;SAVE ADDR
EVAL  LD    HL,(EXPRS)       ;GET END OF EXPR
EV0   LD    BC,0             ;INIT BC TO ZERO
EV1   INC   B                ;COUNT EACH BYTE
      LD    A,(HL)           ;GET CODE IN REG A
      DEC   HL               ;POINT NEXT
      CP    0E3H             ;TEST IT
      JP    NZ,EV2           ;BRIF NOT DATA
      DEC   HL               ;POINT NEXT
      DEC   HL               ;DITTO
      INC   B                ;BUMP CTR
      INC   B                ;BY TWO
      INC   C                ;COUNT THE TERM
      JP    EV1              ;LOOP
EV2   CP    1                ;TEST IF FUNCTION
      JP    NZ,EV5           ;BRIF NOT
      INC   HL               ;RESET TO TYPE CODE
      INC   HL               ;POINT BACK PREV DATA
      LD    D,(HL)           ;MOVE HIGH TO D
      INC   HL               ;POINT ONE MORE
      LD    E,(HL)           ;MOV LOW
      PUSH  BC               ;SAVE CTRS
      PUSH  HL               ;SAVE LOCATION
      EX    DE,HL            ;FLIP/FLOP
      RST   RST5             ;GO LOAD THE VARIABLE
      POP   HL               ;RESTORE LOCATION
      RST   RST7             ;GO BUMP HL
      DEFB  -3
      LD    E,(HL)           ;LOW BYTE
      DEC   HL               ;POINT BACK
      LD    D,(HL)           ;HIGH BYTE
      PUSH  HL               ;SAVE LOCATION
      LD    HL,EV3           ;GET RETURN ADDRESS
      PUSH  HL               ;SAVE ON STACK
      EX    DE,HL            ;PUT TO HL
      JP    (HL)             ;GO EXECUTE THE FUNCTION
EV3   EQU   $                ;FUNCTIONS RETURN HERE
      POP   DE               ;GET LOCATION
      POP   BC               ;GET COUNTERS
      LD    HL,0             ;LOAD ZERO TO HL
      PUSH  HL               ;GET BLOCK OF
      PUSH  HL               ;4 BYTES
      LD    A,(SPCTR)        ;GET TEMP CTR
      INC   A                ;COUNT ONE
      LD    (SPCTR),A        ;SAVE IT
      ADD   HL,SP            ;GET STACK ADDR
      PUSH  BC               ;SAVE CTRS
      PUSH  DE               ;SAVE LOCATION
      PUSH  HL               ;SAVE ADDR
      RST   RST6             ;GO STORE THE VARIABLE
      POP   DE               ;RESTORE ADDR
      POP   HL               ;RESTORE LOCATION
      POP   BC               ;RESTORE COUNTERS
      LD    (HL),D           ;PUT HIGH ADDR BYTE
      INC   HL               ;POINT NEXT
      LD    (HL),E           ;PUT LOW ADDR BYTE
      INC   HL               ;POINT NEXT
      LD    (HL),0E3H        ;SET CODE = DATA
      LD    D,H              ;COPY
      LD    E,L              ;HL TO DE
      DEC   B                ;SUB 1 FROM BYTE COUNT
      INC   DE               ;POINT
      INC   DE               ;TO
      INC   DE               ;CORRECT
      CALL  SQUIS            ;GO COMPRESS STACK
      LD    HL,(EXPRS)       ;GET ADDR
      RST   RST7             ;GO DECR HL
      DEFB  -3               ;BY THREE
      LD    (EXPRS),HL       ;SAVE UPDATED ADDR
      JP    EVAL             ;START AT BEGINNING
EV5   CP    '('              ;TEST IF OPEN PAREN
      JP    NZ,EV6           ;BRIF NOT
      LD    A,C              ;GET TERM CT
      CP    1                ;TEST IF ONE
      JP    NZ,STERR         ;ERROR IF ONE TERM NOT REMAIN
      LD    D,H              ;COPY HL
      LD    E,L              ;TO DE
      INC   DE               ;POINT SENDING
      DEC   B                ;SUBT ONE FROM COUNT
      CALL  SQUIS            ;GO COMPRESS IT
      LD    HL,(EXPRS)       ;GET POINTER
      DEC   HL               ;LESS ONE
      LD    (EXPRS),HL       ;UPDATE IT
      LD    HL,(ADDR3)       ;RESTORE STMT POINTERS
      JP    LOOKO            ;CONTINUE
EV6   OR    A                ;TEST IF END OF EXPRESSION
      JP    NZ,EV9           ;BRIF NOT
      LD    A,C              ;GET TERM COUNT
      CP    1                ;TEST IF ONE
      JP    NZ,STERR         ;ERROR IF NOT ONE
      INC   HL               ;POINT HIGH ADDR
      INC   HL               ;SAME
      LD    D,(HL)           ;HIGH TO D
      INC   HL               ;POINT LOW
      LD    E,(HL)           ;LOW TO E
      EX    DE,HL            ;PUT DATA ADDRESS IN HL
      RST   RST5             ;GO LOAD IT
      LD    HL,(ADDR3)       ;RESTORE STMT POINTER
      LD    A,(SPCTR)        ;GET STACK WORD (4BYTE) COUNTER
      OR    A                ;TEST IT
      RET   Z                ;RETURN IF ZERO
EV7   POP   DE               ;RETURN 2 BYTES
      POP   DE               ;RETURN 2 MORE
      DEC   A                ;DECR CTR
      JP    NZ,EV7           ;LOOP
      RET                    ;RETURN TO STMT PROCESSOR
EV9   CP    '+'              ;TEST IF PLUS
      LD    DE,FADDJ         ;ADDR
      JP    Z,EV10           ;BRIF IS
      CP    '-'              ;TEST IF MINUS
      LD    DE,FSUBJ         ;ADDR
      JP    Z,EV10           ;BRIF IS
      CP    '*'              ;TEST IF MUL
      LD    DE,FMULJ         ;ADDR
      JP    Z,EV10           ;BRIF IS
      CP    '/'              ;TEST IF DIV
      LD    DE,FDIVJ         ;ADDR
      JP    NZ,STERR         ;ERROR IF NOT
EV10  INC   HL               ;POINT TO
      INC   HL               ;1ST DATA
      PUSH  BC               ;SAVE CTRS
      PUSH  DE               ;SAVE ROUTINE ADDR
      LD    D,(HL)           ;HIGH TO D
      INC   HL               ;POINT NEXT
      LD    E,(HL)           ;LOW TO E
      PUSH  HL               ;SAVE POINTER
      EX    DE,HL            ;ADDR TO HL
      RST   RST5             ;GO LOAD IT
      POP   HL               ;RESTORE HL
      INC   HL               ;POINT 2ND DATA
      INC   HL               ;SAME
      LD    D,(HL)           ;HIGH TO D
      INC   HL               ;POINT NEXT
      LD    E,(HL)           ;LOW TO E
      EX    (SP),HL          ;POP ADDR FROM STACK, PUSH HL ON
      JP    (HL)             ;JUMP TO ROUTINE
FADDJ EX    DE,HL            ;GET HL
      CALL  FADD             ;GO ADD
      JP    EV11             ;CONTINUE
FSUBJ EX    DE,HL            ;GET HL
      CALL  FSUB             ;GO SUBTRACT
      JP    EV11             ;CONTINUE
FMULJ EX    DE,HL            ;GET HL
      CALL  FMUL             ;GO MULTIPLY
      JP    EV11             ;CONTINUE
FDIVJ EX    DE,HL            ;GET HL
      CALL  FDIV             ;GO DIVIDE
EV11  POP   HL               ;GET HL
      POP   BC               ;GET CTRS
      RST   RST7             ;GO DECR HL
      DEFB  -6
      CALL  GTEMP            ;GO SAVE FACC
      LD    D,H              ;COPY HL
      LD    E,L              ;TO DE
      INC   DE               ;POSITION
      INC   DE               ;TO
      INC   DE               ;FOUR
      INC   DE               ;BYTES OFFSET
      LD    A,B              ;GET CTR
      SUB   3                ;MINUS THREE
      LD    B,A              ;SAVE
      CALL  SQUIS            ;GO COMPRESS
      LD    HL,(EXPRS)       ;GET ADDR
      RST   RST7             ;GO DECR HL
      DEFB  -4               ;BY FOUR
      LD    (EXPRS),HL       ;RESTORE
      JP    EVAL             ;CONTINUE
;
;
*HEADING IMSAI 8080 4K BASIC
TERMI EQU   $
;
;READ A LINE FROM THE TTY
;FIRST PROMPT WITH THE CHAR IN THE A REG
;TERMINATE THE LINE WITH A X'00'
;IGNORE EMPTY LINES
;CONTROL C WILL CANCEL THE LINE
;RUBOUT WILL DELETE THE LAST CHAR INPUT
;
;
      LD    (PROMP),A        ;SAVE THE PROMPT CHAR
      LD    A,0FFH           ;GET BEGIN MARKER
      LD    (IOBUF-1),A      ;PUT IT
REIN  LD    HL,IOBUF         ;POINT TO INPUT BUFFER
      LD    A,(PROMP)        ;GET THE PROMPT AGAIN
      OR    A                ;TEST IT
      JP    Z,TREAD          ;BRIF NULL
      CALL  TESTO            ;GO WRITE IT
      LD    A,' '            ;GET A SPACE
      CALL  TESTO            ;WRITE SPACE
TREAD EQU   $
      CALL  TESTI            ;GO WAIT FOR READY
      CALL  GETCH            ;GO GET THE CHARACTER
      LD    (HL),A           ;PUT IN BUFFER
      LD    A,(HL)           ;RELOAD THE CHAR
      CP    0AH              ;TEST IF LINE FEED
      JP    Z,TREAD          ;IGNORE IF IT IS
      CALL  TESTO            ;ECHO THE CHARACTER
      CP    0DH              ;TEST IF CR
      JP    NZ,NOTCR         ;BRIF NOT
      CALL  CRLF             ;GO WRITE CRLF
CR1   LD    (HL),0           ;MARK END WITH ALL HIGH
      DEC   HL               ;POINT PRIOR
      LD    A,(HL)           ;LOAD IT
      CP    ' '              ;TEST IF SPACE
      JP    Z,CR1            ;BRIF SPACE
      CP    0FFH             ;TEST IF AT BEGINNING
      JP    Z,REIN           ;BRIF IS (NULL LINE)
      LD    HL,IOBUF         ;POINT TO START
      RET                    ;ELSE, RETURN
TESTI EQU   $
;     IN    A,(TTY-1)        ;GET TERM STATUS
;     AND   40H              ;MASK FOR RXRDY
      IN    A,(TTY+1)        ;**UM**
      AND   2                ;**UM**
      JP    Z,TESTI          ;LOOP TILL READY
      RET                    ;RETURN
TESTO EQU   $
      PUSH  AF               ;SAVE CHAR TO OUTPUT
      LD    A,(OUTSW)        ;GET OUTPUT SWITCH
      OR    A                ;TEST IF OFF
      JP    NZ,TOUT2         ;BRIF NOT
;TOUT1 IN    A,(TTY-1)        ;GET STATUS
;     RLA                    ;SHIFT LEFT (TEST TXRDY)
TOUT1 IN    A,(TTY+1)        ;**UM**
      RRA                    ;**UM**
      JP    NC,TOUT1         ;LOOP TILL READY
      POP   AF               ;GET CHAR TO OUTPUT
      OUT   (TTY),A          ;WRITE IT
      RET                    ;RETURN
TOUT2 POP   AF               ;RESTORE CHAR
      RET                    ;RETURN
CRLF  XOR   A                ;CLEAR REG A
      LD    (COLUM),A        ;RESET COLUM POINTER
      LD    A,0DH            ;GET CR
      CALL  TESTO            ;WRITE IT
      LD    A,0AH            ;LF
      CALL  TESTO            ;WRITE IT
      PUSH  BC               ;SAVE BC
      LD    B,2              ;DELAY COUNT
DELAY LD    A,0FFH           ;GET RUBOUT
      CALL  TESTO            ;WRITE IT
      DEC   B                ;DECR LOOP CTR
      JP    NZ,DELAY         ;LOOP
      POP   BC               ;RESTORE BC
      RET                    ;RETURN
NOTCR CP    7FH              ;TEST IF RUBOUT
      JP    NZ,NOTBS         ;BRIF NOT
      DEC   HL               ;POINT PRIOR
      LD    A,(HL)           ;LOAD PREV CHAR
      CP    0FFH             ;TEST IF AT BEGIN
      JP    Z,NOTBS          ;BRIF IS
      LD    A,':'            ;BACKSLASH
      CALL  TESTO            ;WRITE IT
      LD    A,(HL)           ;LOAD THE CHAR
      CALL  TESTO            ;WRITE IT
      DEC   HL               ;POINT PRIOR
      LD    A,':'            ;BACKSLASH
      CALL  TESTO            ;WRITE IT
NOTBS INC   HL               ;POINT NEXT BUFFER POSIT
      JP    TREAD            ;LOOP FOR NEXT
;
;
TERMO EQU   $
;
;TTY PRINT ROUTINE
;
;OUTPUT STRING OF CHARS STARTING AT IOBUFF THRU END (00 OR
;FOLLOWING IMBEDDED CHARACTERS ARE INTERPRETED AS CONTROLS:
;X'00' END OF BUFFER, TYPE CR/LF AND RETURN
;X'FE' END OF BUFFER, RETURN (NO CR/LF)
;X'FD' TYPE CR/LF, CONTINUE
;
;
      LD    HL,IOBUF         ;GET ADDR OF BUFFER
OUT1  LD    A,(HL)           ;LOAD A BYTE
      CP    0FEH             ;SEE IF END OF LINE (NO CR/LF)
      RET   Z                ;RETURN IF EQUAL
      CP    0FDH             ;SEE IF EMBEDDED CR/LF
      JP    NZ,OUT2          ;BRIF NOT
      CALL  CRLF             ;LINE FEED
      JP    OUT4             ;CONTINUE
OUT2  OR    A                ;TEST IF END OF OUTPUT
      JP    Z,CRLF           ;BRIF IS
      LD    A,(HL)           ;LOAD THE BYTE
      CALL  TESTO            ;TYPE IT
      LD    A,(COLUM)        ;GET COLUM POINTER
      INC   A                ;ADD ONE
      LD    (COLUM),A        ;RESTORE IT
OUT4  INC   HL               ;POINT NEXT
      JP    OUT1             ;LOOP
;
;
;
LINEO EQU   $
;
;UNPACK LINE NUMBER FROM (HL) TO (DE)
;
;
      CALL  LOUT             ;GO FORMAT 2 BYTES
LOUT  EQU   $
      LD    A,(HL)           ;GET BYTE
      CALL  RIGHT            ;GO SHIFT TO RIGHT
      OR    30H              ;ZONE
      LD    (DE),A           ;PUT TO BUFFER
      INC   DE               ;POINT NEXT
      LD    A,(HL)           ;LOAD BYTE
      AND   0FH              ;MASK
      OR    30H              ;ZONE
      LD    (DE),A           ;PUT TO BUFFER
      INC   DE               ;POINT NEXT
      INC   HL               ;AND NEXT LINE BYTE
      RET                    ;RETURN
;
;
TSTCH EQU   $
;
;TEST IF INPUT CHAR ON KEYBOARD
;IF THERE IS, THEN READ IT
;TERMINATE IF CONTROL-C
;TOGGLE OUTPUT SW IF CONTROL-O
;
;     IN    A,(TTY-1)        ;GET STATUS
;     AND   40H              ;MASK FOR RXRDY
      IN    A,(TTY+1)        ;**UM**
      AND   2                ;**UM**
      RET   Z                ;RETURN IF NOT
GETCH IN    A,(TTY)          ;ELSE, READ THE CHAR
      AND   7FH              ;TURN OFF PARITY
      CP    0FH              ;TEST IF CONTROL-O
      JP    Z,CONTO          ;BRIF IS
      CP    03H              ;TEST IF CONTROL-C
      RET   NZ               ;RETURN IF NOT
      CALL  CRLF             ;PRINT CR/LF
      JP    READY            ;QUIT WHAT YOU WERE DOING
CONTO LD    A,(OUTSW)        ;GET SWITCH
      XOR   01H              ;TOGGLE
      LD    (OUTSW),A        ;RESTORE
      LD    A,0AH            ;GET A LF
      RET                    ;RETURN
;
;
ZEROM EQU   $
;
;MOVE STRING OF ZEROS TO (HL)+...  CNT IN B
;
      LD    (HL),0           ;MOVE ONE ZERO
      INC   HL               ;POINT NEXT
      DEC   B                ;DECR CTR
      JP    NZ,ZEROM         ;LOOP
      RET                    ;RETURN
;
;
COPYH EQU   $
;
;COPY STRING FROM (HL) TO (DE)
;COUNT IN B
;
      LD    A,(HL)           ;GET A CHAR
      LD    (DE),A           ;PUT IT DOWN
      INC   HL               ;POINT NEXT
      INC   DE               ;DITTO
      DEC   B                ;DECR CTR
      JP    NZ,COPYH         ;LOOP
      RET                    ;RETURN
;
;
COPYD EQU   $
;
;COPY STRING FROM (DE) TO (HL)
;COUNT IN B
;
      EX    DE,HL            ;FLIP DE/HL
      CALL  COPYH            ;GO COPY
      EX    DE,HL            ;THEN FLIP BACK
      RET                    ;RETURN
;
;
COMP2 EQU   $
;
;CONTINUE COMP SUBROUTINE (RST RST2)
;
      CP    (HL)             ;COMPARE THE CHAR
      RET   NZ               ;RETURN IF NOT EQUAL
      INC   DE               ;POINT NEXT
      INC   HL               ;DITTO
      JP    RST2             ;LOOP
;
;
ULERR LD    BC,'UL'          ;UNDEFINED LINE NUMBER
      RST   RST3
OVERR LD    BC,'OV'          ;DIV BY ZERO/OVERFLOW/UNDERFLOW
      RST   RST3
STERR LD    BC,'ST'          ;ERROR IN EXPRESSION STACK
      RST   RST3
SNERR LD    BC,'SN'          ;SYNTAX ERROR
      RST   RST3
RTERR LD    BC,'RT'          ;RETURN & NO GOSUB
      RST   RST3
DAERR LD    BC,'DA'          ;OUT OF DATA
      RST   RST3
FOERR LD    BC,'FO'          ;MORE THAN 8 NESTED FOR/NEXT OR
      RST   RST3
NXERR LD    BC,'NX'          ;FOR & NO NEXT / NEXT & NO FOR
      RST   RST3
;
;
;
;
PACK  EQU   $
;
;PACK LINE NUMBER FROM (HL) TO BC
;
;
      RST   RST1             ;SKIP LEADING SPACES
      LD    BC,0             ;CLEAR B AND C
      LD    A,4              ;INIT DIGIT COUNTER
      LD    (PRSW),A         ;SAVE A
PK1   LD    A,(HL)           ;GET CHAR
      CALL  NUMXR            ;TEST FOR NUMERIC
      RET   NZ               ;RETURN IF NOT NUMERIC
      AND   0FH              ;STRIP OFF ZONE
      LD    D,A              ;SAVE IT
      LD    A,(PRSW)         ;GET COUNT
      DEC   A                ;SUBTRACT ONE
      JP    M,SNERR          ;BRIF MORE THAN 4 DIGITS
      LD    (PRSW),A         ;SAVE CTR
      LD    E,4              ;4 BIT SHIFT LOOP
PK3   LD    A,C              ;GET LOW BYTE
      RLA                    ;ROTATE LEFT 1 BIT
      LD    C,A              ;REPLACE
      LD    A,B              ;GET HIGH BYTE
      RLA                    ;ROTATE LEFT 1 BIT
      LD    B,A              ;REPLACE
      DEC   E                ;DECR CTR
      JP    NZ,PK3           ;LOOP
      LD    A,C              ;GET LOW
      OR    D                ;PUT DIGIT IN RIGHT HALF OF BYTE
      LD    C,A              ;REPLACE
      INC   HL               ;POINT NEXT BYTE
      JP    PK1              ;LOOP
;
;
;
SQUIS EQU   $
;
;COMPRESS THE EXPR STACK
;TO ADDR IN HL
;FROM ADDR IN DE
;COUNT IN B
;
SQUI2 INC   DE               ;POINT NEXT SEND
      INC   HL               ;POINT NEXT RECEIVE
      LD    A,(DE)           ;GET A CHAR
      LD    (HL),A           ;PUT IT DOWN
      DEC   B                ;DECR CTR
      JP    NZ,SQUI2         ;LOOP
      RET                    ;RETURN
;
;
GTEMP EQU   $
;
;GETS FOUR BYTE TEMPORARY STORAGE AREA,
;STORES THE FACC THERE,
;PUTS ADDR OF AREA IN EXPR STACK (HL)
;
      EX    DE,HL            ;SAVE HL IN DE
      EX    (SP),HL          ;EXCHANGE 0 AND RET ADDR
      PUSH  HL               ;PUT NEW RET ADDR
      PUSH  HL               ;DO IT AGAIN
      LD    HL,0             ;ZERO HL
      ADD   HL,SP            ;GET SP ADDR IN HL
      INC   HL               ;PLUS ONE
      INC   HL               ;PLUS ONE MORE (POINT TO NEW ARE
      PUSH  BC               ;SAVE CTRS
      PUSH  DE               ;SAVE EXPR ADDR
      PUSH  HL               ;SAVE TEMP ADDR
      LD    A,(SPCTR)        ;GET WORD COUNTER
      INC   A                ;INCR IT
      LD    (SPCTR),A        ;RESTORE IT
      RST   RST6             ;GO STORE FACC
      POP   DE               ;RESTORE TEMP ADDR
      POP   HL               ;RESTORE EXPR ADDR
      POP   BC               ;RESTORE CTRS
SADR  INC   HL               ;POINT NEXT BYTE
      LD    (HL),D           ;HIGH BYTE TO EXPR STACK
      INC   HL               ;POINT NEXT
      LD    (HL),E           ;LOW BYTE TO EXPR STACK
      INC   HL               ;POINT NEXT
      LD    (HL),0E3H        ;CODE = DATA
      RET                    ;RETURN
;
;
ALPHA EQU   $
;
;TESTS THE CHAR AT (HL)
;RETURNS WITH Z SET IF CHAR IS ALPHA (A-Z)
;RETURNS WITH Z OFF IF NOT ALPHA
;CHAR IS LEFT IN REG A
;
      RST   RST1             ;SKIP LEADING SPACES
      CP    'A'              ;TEST IF A OR HIGHER
      RET   C                ;RETURN IF NOT ALPHA (Z IS OFF)
      CP    'Z'+1            ;TEST IF Z OR LESS
      RET   NC               ;RETURN IF NOT < Z (Z OFF)
      CP    A                ;TURN ON Z
      RET                    ;RETURN
;
;
NUMER EQU   $
;
;TESTS THE CHAR AT (HL)
;RETURNS WITH Z SET IF NUMERIC (0-9)
;ELSE, Z IS OFF
;CHAR IS LEFT IN THE A REG
;
      RST   RST1             ;SKIP LEADING SPACES
NUMXR CP    '0'              ;TEST IF ZERO OR GREATER
      RET   C                ;RETURN IF LESS THAN ZERO
      CP    '9'+1            ;TEST IF 9 OR LESS
      RET   NC               ;RETURN IF NOT NUMERIC
      CP    A                ;SET Z
      RET                    ;RETURN
;
;
RIGHT EQU   $
;
;SHIFT THE LEFTMOST 4 BITS OF REG A RIGHT FOUR BITS
;
      AND   0F0H             ;ISOLATE LEFT
      RRA                    ;SHIFT ONCE
      RRA                    ;AGAIN
      RRA                    ;AGAIN
      RRA                    ;ONE LAST TIME
      RET                    ;RETURN
;
;
SEARC EQU   $
;
;SEARCES FOR THE VARIABLE IN DE
;RETURNS WITH ADDR OF DATA AREA FOR VARIABLE
;
      PUSH  HL               ;SAVE HL
      LD    HL,(DATAB)       ;GET ADDR OF DATA POOL
      LD    BC,-6            ;LENGTH OF EACH ENTRY
SCH1  LD    A,(HL)           ;GET THE BYTE
      OR    A                ;TEST IF END
      JP    Z,SCH3           ;BRIF END
      CP    D                ;COMPARE 1ST CHAR
      JP    NZ,SCH2          ;BRIF NOT EQUAL
      DEC   HL               ;POINT NEXT
      LD    A,(HL)           ;LOAD 2ND DIGIT
      INC   HL               ;POINT BACK
      CP    E                ;COMPARE 2ND CHAR
      JP    NZ,SCH2          ;BRIF NOT EQUAL
      ADD   HL,BC            ;POINT NEXT ENTRY
      INC   HL               ;PLUS ONE
      EX    DE,HL            ;FLIP/FLOP
      POP   HL               ;RESTORE HL
      RET                    ;RETURN
SCH2  ADD   HL,BC            ;MINUS SIX
      JP    SCH1             ;LOOP
SCH3  LD    (HL),D           ;PUT 1ST CHAR
      DEC   HL               ;POINT NEXT
      LD    (HL),E           ;PUT 2ND CHAR
      LD    B,4              ;LOOP CTR
SCH4  DEC   HL               ;POINT NEXT
      LD    (HL),0           ;ZERO THE VALUE
      DEC   B                ;DECR CTR
      JP    NZ,SCH4          ;LOOP
      DEC   HL               ;POINT NEXT
      LD    (HL),B           ;MOVE ZERO TO NEW END
      INC   HL               ;POINT ADDR OF VARIABLE
      EX    DE,HL            ;PUT LOCATION TO DE
      POP   HL               ;RESTORE HL
      RET                    ;RETURN
;
;
VAR   EQU   $
;
;
;TEST (HL) FOR A VARIABLE NAME
;PUTS THE NAME IN DE IF FOUND
;
      CALL  ALPHA            ;TEST IF ALPHA
      JP    NZ,SNERR         ;BRIF NOT ALPHA
      LD    D,A              ;FIRST CHAR
      LD    E,' '            ;DEFAULT
      INC   HL               ;POINT NEXT
      CALL  NUMER            ;TEST IF NUMERIC
      RET   NZ               ;RETURN IF NOT NUMERIC
      LD    E,A              ;SAVE 2ND CHAR
      INC   HL               ;POINT NEXT
      RST   RST1             ;SKIP SPACES
      RET                    ;THEN RETURN
;
;
ERROR EQU   $
;
;CONTINUE ERROR ROUTINE (RST RST3)
;
      LD    (HL),C           ;PUT 2ND CHAR
      INC   HL               ;POINT NEXT
      LD    (HL),0FEH        ;MARK END
      CALL  TERMO            ;GO PRINT IT
      LD    HL,ERRXR         ;POINT MESG
      CALL  OUT1             ;GO PRINT IT
      LD    DE,IOBUF         ;POINT BUFFER
      LD    HL,(LINE)        ;GET ADDR OF LINE NUMBER
      CALL  LINEO            ;UNPACK LINE NUMBER
      XOR   A                ;GET END CODE
      LD    (DE),A           ;PUT TO BUFFER
      CALL  TERMO            ;PRINT IT
      JP    GETCM            ;GO GET NEXT COMMAND
*HEADING IMSAI 8080 4K BASIC
LISTL DEFM  'LIS'
      DEFB  0
NEWLI DEFM  'NEW'
      DEFB  0
RUNLI DEFM  'RUN'
      DEFB  0
RNDLI DEFM  'RND'
      DEFB  0
ABSLI DEFM  'ABS'
      DEFB  0
SQRLI DEFM  'SQR'
      DEFB  0
SGNLI DEFM  'SGN'
      DEFB  0
JMPTB EQU   $
IFLIT DEFM  'IF'
      DEFB  0
      DEFW  IF
READL DEFM  'READ'
      DEFB  0
      DEFW  READ
DATAL DEFM  'DATA'
      DEFB  0
      DEFW  RUN
FORLI DEFM  'FOR'
      DEFB  0
      DEFW  FOR
NEXTL DEFM  'NEXT'
      DEFB  0
      DEFW  NEXT
GOSUX DEFM  'GOSUB'
      DEFB  0
      DEFW  GOSUB
RETLI DEFM  'RET'
      DEFB  0
      DEFW  RETUR
INPUX DEFM  'INPUT'
      DEFB  0
      DEFW  INPUT
PRINX DEFM  'PR'
INTLI DEFM  'INT'
      DEFB  0
      DEFW  PRINT
      DEFM  '?'
      DEFB  0
      DEFW  PRINT
GOTOL DEFM  'GO'
TOLIT DEFM  'TO'
      DEFB  0
      DEFW  GOTO
LETLI DEFM  'LET'
      DEFB  0
      DEFW  LET
STOPL DEFM  'STO'
      DEFB  0
      DEFW  READY
ENDLI DEFM  'END'
      DEFB  0
      DEFW  RUN
REMLI DEFM  'REM'
      DEFB  0
      DEFW  RUN
      DEFB  0                ;END OF TABLE
STEPL DEFM  'STEP'
      DEFB  0
THENL DEFM  'THEN'
      DEFB  0
ERRXR DEFM  ' ERR @ '
      DEFB  0FEH
ONE   DEFW  1000H            ;CONSTANT ONE
      DEFW  0
TWO   DEFW  2000H            ;CONSTANT TWO
      DEFW  0
THREE DEFW  3000H            ;CONSTANT THREE
      DEFW  0
RNDX  DEFW  837FH            ;RANDOMIZER
      DEFW  1974H
ROMEN EQU   $                ;END OF READ-ONLY-MEMORY
*HEADING IMSAI 8080 4K BASIC
      ORG   1000H            ;RAM AREA
RAM   EQU   $                ;ALIGN RAM ON 4K BOUNDARY
;TTY   EQU   1                ;DEVICE ADDRESS FOR TERMINAL
TTY   EQU   2                ;**UM**
NULLI DEFS  2
IOBUF DEFS  40               ;INPUT/OUTPUT BUFFER
FACC  DEFS  4
FTEMP DEFS  10
REL   DEFS  1                ;HOLDS THE RELATION IN AN IF STMT
DIVSW DEFS  1                ;0=NORMAL DIVIDE, 1=DIVIDE FOR R
TVAR1 DEFS  4                ;TEMP STORAGE
TVAR2 DEFS  4                ;DITTO
ORIGS DEFS  4                ;HOLDS ORIG NUMBER FOR SQR
TSTSQ DEFS  4                ;HOLDS TRIAL SQUARE ROOT
TST2S DEFS  4                ;HOLDS TRIAL SQUARE ROOT ** 2
SQRX  DEFS  4                ;TEMP STORAGE FOR SQUARE ROOT
EXPRS DEFS  2                ;HOLDS ADDR OF EXPR
PARCT DEFS  1
SPCTR DEFS  1
PRSW  DEFS  1
ADDR1 DEFS  2                ;HOLDS TEMP ADDRESS
ADDR2 DEFS  2                ;HOLDS TEMP ADDRESS
ADDR3 DEFS  2                ;HOLDS STMT ADDRESS DURING EXPR
STMT  DEFS  2                ;HOLDS ADDR OF CURRENT STATEMENT
INDX  DEFS  2                ;HOLDS VARIABLE NAME OF FOR/NEXT
OUTSW DEFS  1                ;OUTPUT SUPPRESS IF ON
RUNSW DEFS  1                ;0=RUN MODE, 1=IMMEDIATE MODE
COLUM DEFS  1                ;CURRENT TTY COLUM
RNDNU DEFS  4
DASTM DEFS  2                ;HOLDS LINE ADDRESS OF CURRENT D
LINE  DEFS  2                ;HOLD ADDR OF PREV LINE NUM
STACK DEFS  2                ;HOLDS ADDR OF START OF RETURN
FORNE DEFS  97
PROMP DEFS  1                ;HOLDS PROMPT CHARACTER
IMMED DEFS  70               ;IMMEDIATE COMMAND STORAGE AREA
DATAP DEFS  2                ;ADDR OF CURRENT DATA STMT
DATAB DEFS  2                ;ADDRESS OF DATA POOL
PROGE DEFS  2                ;ADDR OF PROG POOL END
      DEFS  1                ;THIS HAS LOW VALUE AT RUN TIME
BEGPR EQU   $                ;PROGRAM AREA STARTS HERE
;
;
      END   BASIC

;EDITS:
;    30-JUN-08 KJL
;       - CREATED FROM IMSAI 8K BASIC VERSION 1.4 MANUAL
;
;    07-FEB-14 UM
;	- FIXED TYPOS, MATCHES MANUAL NOW
;
;---------------------------------------------------------
; BASIC30.ASM   1.4     05/19/77        JRB     8K BASIC
; BASICS2.ASM   1.401   05/11/77        DK      8K BASIC
; BASIC19.ASM   1.401   05/11/77        DH
; BASIC18.ASM   1.401   05/10/77        JRB
; BASIC16.ASM   1.401   05/09/77        DH
; BASIC11.ASM   1.401   05/04/77        DH
; BASIC10.ASM   1.401   05/03/77        DH
; BASIC8.ASM    1.401   05/02/77        DH
;
; IMSAI 8K-9K BASIC
;
; COPYRIGHT (C) 1977
; IMSAI MANUFACTURING CORPORATION
; 14860 WICKS BLVD, SAN LEANDRO CALIFORNIA  94577
;
; CORRECTION HISTORY:
;
;   02/25/77 - FIXED BEGPR POINTERS
;            - FIXED LOG(X) FOR 0.5 < X < 1.0
;            - FIXED SQR(X) FOR 0.0 < X < 0.5
;            - FIXED SCI NOTATION INPUT ROUTINE
;            - FIXED EDIT ROUTINE WHEN PROGRAM ENDS ON
;              00 BOUNDARY (SYSTEM USED TO GO AWAY)
;            - ADDED XEQ COMMAND (LIKE RUN BUT KEEPS DATA)
;            - SOFTWARE MEMORY PROTECT OF 1ST 9K IMPLIMENTED
;            - FIXED TAB FOR BACKWARDS MOVEMENT
;            - FIXED OV ERROR FOR SMALL X IN TRIG,LOG & EXP
;            - ADDED PROGRAM CHAINING CAPABILITY.
;            - FIXED EXP(X) ROUTINE FOR LARGE X.
;            - ADDED PEEK(X) COMMAND
;            - ADDED POKE A,X COMMAND
;            - ADDED CALL A COMMAND
;  04/02/77  - ADDED TARBEL CASSETTE SAVE AND LOAD
;            - ADDED FIX LINE EDITOR
;            - RENAMED NATURAL LOG TO LN(X)
;            - ADDED BASE 10 LOG AS LOG(X)
;            - ALLOWED FOR DAZZLER IN OUTPUT ROUTINE
;            - ADDED LINE # SEARCH UTILITY (LOCAT EQU $)
;            - ADDED TABLE SEARCH UTILITY (SEEK EQU $)
;            - ARRAYS CAN NOW HAVE > 256 ELEMENTS PER DIM
; 04/09/77   -ADDED CONDITIONAL ASSY PARAMS FOR 8 AND 9K
;            -FIXED POWER ERROR. (X^B WHEN B=0 GAVE X^2.)
;            -ADDED CONTROL H AS PHYSICAL RUBOUT OF CHAR
; 04/27/77   -CHANGE RST'S TO RUN UNDER CP/M
;            -ADDED EXPRESSION EVALUATER FIX
;            -LOAD UNDER CP/M
; 05/02/77   -ADD DDT, BYE COMMANDS, BIOS I/O
; 05/03/77   -OPTIMIZE FUNCTION ITERATION LOOP (SIN5)
;            -SO UNDERFLOW CAN BE MADE NON-FATAL
; 05/04/77   -OPTIMIZE SIN(X) ROUTINE
;            -ADD NON-FATAL ERRORS
; 05/09/77   -SQUISH TO INCLUDE PEEK,POKE,CALL IN 8K
; 05/11/77   -MAKE RND(X) USE X AS RANGE; X^0->1,0^X->0
;            -TAB(N) GO TO NEXT LINE IF PAST POSITION
; 5/12/77   - BUG IN NESTED FOR'S AND REENTERED FOR'S FIXED
;
; ASSEMBLY PARAMETERS:
        LARGE   EQU     0       ;-1=9K ASSEMBLY, 0=8K
        CPM     EQU     0       ;-1=RUN UNDER CPM
        HUNTER  EQU     0       ;-1= INCLUDE BAUD COMMAND
;
; CPM EQUATES
;
        BOOT    EQU     0       ;WARM BOOT
        BDOS    EQU     5       ;BDOS ENTRY
        TBASE   EQU     0100H   ;PROGRAM LOAD UNDER CPM
        CSTAT   EQU     3       ;OFFSET OF CONSOLE STATUS
                                ;...QUERY IN BIOS TABLE
;
; BASIC EQUATES
;
        FATAL   EQU     0F7H    ;CODE FOR FATAL IS RST 6
;
BASIC:  IF      NOT CPM
        ORG     0
        LXI     H,RAM+1024
        MVI     A,0AEH  ;START OF INIT SEQUENCE
        JMP     INIT1   ;FINISH INIT
        ENDIF
;
        IF      CPM
        ORG     TBASE
        JMP     INITC   ;USE TEMPORARY CODE AT END
        ENDIF
;
;       ORG     8
;
; SKIP CHARS POINTED BY H,L UNTIL NON-BLANK,
; LEAVE IN REG A
;
RST1:   MOV     A,M     ;LOAD THE BYTE AT (H,L)
        CPI     ' '     ;TEST IF BLANK
        RNZ             ;RETURN IF NOT
        INX     H       ;POINT NEXT
        JMP     RST1    ;LOOP
;
;
;       ORG     16
;
; COMPARE STRING AT (H,L) TO STRING AT (D,E)
; RETURN IF EQUAL (THRU X'00' IN D,E) OR ON FIRST NOT EQUAL
; ONLY THE FIRST THREE CHARS NEED BE EQUAL
; IGNORE ALL SPACES
;
RST2:   PUSH    B       ;SAVE B,C
        MVI     B,0     ;INIT COUNT
COMP1:  RST     1       ;SKIP SPACES
        LDAX    D       ;GET CHAR TO MATCH WITH
        JMP     COMP2   ;CONTINUE ELSEWHERE
;
;
;       ORG     24
;
; STORE THE FLOATING POINT ACCUMULATOR AT (H,L)
;
RST3:   LXI     D,FACC  ;POINT FLOAT ACC
        MVI     B,4     ;BYTE COUNT
        JMP     COPYD   ;GO MOVE IT
;
;
;       ORG     32
;
; INCREMENT H,L BY BYTE AT (SP), RETURN TO (SP)+1
;
RST4:   XTHL            ;GET RETURN ADDRESS IN H,L
        MOV     A,M     ;GET THE INCREMENT
        INX     H       ;POINT TRUE RETURN
        XTHL            ;PUT BACK TO STACK
        PUSH    D       ;SAVE D,E
        JMP     RST4A   ;CONTINUE
;
;
;       ORG     40
;
; LOAD THE FLOATING POINT ACCUM WITH THE 4 BYTES AT (H,L)
;
RST5:   LXI     D,FACC  ;POINT FLOAT ACC
        MVI     B,4     ;BYTE COUNT
        JMP     COPYH   ;GO MOVE IT
;
;
;       ORG     48
;
; PRINT:  'XX ERR & NNN'
; **** IF ERROR MESSAGE CHANGES TO A DIFFERENT RST,
; **** ...CHANGE "FATAL" EQUATE
;
RST6:   XTHL            ;SAVE HL, GET ERROR CODE PTR
        PUSH    PSW     ;SAVE REGS
        PUSH    D
        PUSH    B
        JMP     ERROR   ;CONTINUE
;
        IF NOT CPM
        ORG     59      ;LEAVE 3 BYTES FOR DDT
        ENDIF
;
RST4A:  MOV     E,A     ;PUT IN LOW
        ORA     A       ;TEST SIGN
        MVI     D,0     ;DEFAULT POSITIVE
        JP      RST4B   ;BRIF +
        MVI     D,0FFH  ;ELSE, NEG
RST4B:  DAD     D       ;BUMP H,L
        POP     D       ;RESTORE D,E
        RET             ;RETURN
;PAGE
        DB      'COPYRIGHT (C) 1977 '
        DB      'IMSAI MFG CORP '
        DB      'SAN LEANDRO CA 94577 USA'
;
; INITIALIZATION ROUTINE
; DETERMINE MEMORY SIZE.
;    (START AT 9K AND TRY 1K INCREMENTS TILL END)
; SETUP POINTERS FOR STACK, DATA, AND PROGRAM
; INIT SIO BOARD
;
INIT1:  IF      NOT CPM
        OUT     TTY+1   ;INIT TERMINAL
        MVI     A,40H
        OUT     TTY+1
        MVI     A,0BAH
        OUT     TTY+1
        MVI     A,37H
        OUT     TTY+1
        LXI     B,1024  ;1K INCR
INIT2:  MOV     A,M     ;GET A BYTE FROM MEMORY
        CMA             ;COMPLEMENT
        MOV     M,A     ;REPLACE
        CMP     M       ;TEST IF RAM/ROM/END
        JNZ     INIT3   ;BRIF OUT OF RAM
        CMA             ;RE-COMPLEMENT
        MOV     M,A     ;PUT ORIG BACK
        DAD     B       ;POINT NEXT BLOCK
        JNC     INIT2   ;LOOP
        ENDIF
;
INIT3:  SPHL            ;SET STACK POINTER TO END OF MEMORY
        LXI     B,-256 ;ALLOW 256 BYTES FOR STACK
        DAD     B       ;ADD TO ADDRESS
        SHLD    DATAB   ;SAVE ADDR OF START OF DATA
;
; SOFTWARE WRITE PROTECT OF FIRST 9K OF RAM.
;
; BUT NO PROTECT UNDER CPM OR FOR 8K (EPROM) VERSION
        IF      LARGE AND NOT CPM
        MVI     A,2     ;SET PROTECT OF FIRST 1K BLOCK
PROTC:  OUT     0FEH    ;SEND IT
        ADI     4       ;ADDRESS NEXT 1K BLOCK
        CPI     26H     ;STOP AFTER 9 BLOCKS
        JNZ     PROTC   ;CONTINUE TO PROTECT
        ENDIF
        XRA     A       ;GET A ZERO IN A
        PUSH    PSW     ;SET STACK 1 LEVEL DEEP WITHOUT A GOSUB
        LXI     H,0     ;CLEAR H,L
        DAD     SP      ;SP TO H,L
        SHLD    STACK   ;SAVE BEG OF STACK
        CALL    IRAM    ;INIT RAM
        LXI     D,NRNDX ;POINT TO RANDOM # SERIES
        MVI     B,8     ;LOAD COUNT
        CALL    COPYD   ;COPY TO TRND<X> IN RAM TABLE
        MVI     M,2     ;SET RANDOM SWITCH
        IF      CPM
        CALL    NEW0    ;AUTOMATIC "NEW"
        ENDIF
        LXI     H,VERS  ;POINT VERSION MESSAGE
RDYM:   CALL    TERMM   ;WRITE IT
;
RDY     EQU     $
;
; PRINT 'READY'
;
        LXI     H,READY ;POINT READY MSG
        CALL    TERMM   ;GO PRINT IT
;
GETCM   EQU     $
;
;
; COMMAND INPUT ROUTINE
;
; READ A LINE FROM THE TTY
; IF STARTS WITH NUMERIC CH, ASSUME IT'S A BASIC STATEMENT
; IF NOT, IT IS EITHER AN IMMEDIATE STATMENT, OR A COMMAND
;
        MVI     A,':'   ;PROMPT & ON SET FOR SW
        STA     EDSW    ;SET MODE=EDIT
        LHLD    STACK   ;GET STACK ADDRESS
        SPHL            ;SET REG SP
        CALL    TERMI   ;GET A LINE
        CALL    PACK    ;GO PACK THE NUMBER INTO B,C
        MOV     A,B     ;GET HI BYTE OF LINE NUMBER
        ORA     C       ;PLUS LOW BYTE
        JZ      EXEC    ;BRIF EXEC STATEMENT
        PUSH    B       ;SAVE LINE NUMBER
        LXI     D,IMMED+1       ;POINT SAVE AREA
        XCHG            ;FLIP/FLOP
        MOV     M,B     ;PUT LO LINE
        INX     H       ;POINT NEXT
        MOV     M,C     ;PUT LO LINE
        INX     H       ;POINT NEXT
        MVI     B,3     ;INIT COUNT
EDIT1:  LDAX    D       ;GET A BYTE
        MOV     M,A     ;PUT IT DOWN
        INR     B       ;COUNT IT
        INX     H       ;POINT NEXT
        INX     D       ;DITTO
        ORA     A       ;TEST BYTE JUST MOVED
        JNZ     EDIT1   ;LOOP
        MOV     A,B     ;GET COUNT
        STA     IMMED   ;STORE THE COUNT
        POP     B       ;GET LINE NUM
        CALL    LOCAT   ;GO FIND REQUESTED LINE NUMBER
        PUSH    H       ;SAVE H,L
        JC      EDIT5   ;BRIF IF LINE NOT FOUND
EDIT2:  MOV     D,H     ;COPY ADDR
        MOV     E,L     ;TO D,E
        MVI     B,0     ;GET A ZERO
        MOV     C,M     ;GET LEN
        DAD     B       ;POINT NEXT STMT
EDIT3:  MOV     A,M     ;GET LEN NEXT STMT
        ORA     A       ;TEST IT
        JZ      EDIT8   ;BRIF END
        MOV     B,A     ;SET LENGTH
        CALL    COPYH   ;ELSE MOVE LINE
        JMP     EDIT3   ;LOOP
EDIT8:  XCHG            ;PUT NEW ADDR TO H,L
        MOV     M,A     ;MARK END
        SHLD    PROGE   ;AND UPDATE ADDRESS
EDIT5:  LDA     IMMED   ;GET LEN OF INSERT
        CPI     4       ;TEST IF DELETE
        JZ      GETCM   ;BRIF IS
        MOV     C,A     ;SET LO LEN
        MVI     B,0     ;ZERO HI LEN
        LHLD    PROGE   ;GET END OF PROG
        MOV     D,H     ;COPY TO
        MOV     E,L     ;D,E
        DAD     B       ;DISP LEN OF INSERT
        SHLD    PROGE   ;UPDATE END POINT
        POP     B       ;GET ADDR
EDIT6:  LDAX    D       ;GET A BYTE
        MOV     M,A     ;COPY IT
        DCX     D       ;POINT PRIOR
        DCX     H       ;DITTO
        MOV     A,D     ;GET HI ADDR
        CMP     B       ;COMPARE
        JZ      EDIT7   ;BRIF HI EQUAL
        JNC     EDIT6   ;BRIF NOT LESS
EDIT7:  MOV     A,E     ;GET LO ADDR
        CMP     C       ;COMPARE
        JNC     ED7A    ;MUST TEST FOR 00 BOUNDARY
        JMP     ED7B    ;GO AROUND BOUNDARY TEST CODE
ED7A:   CMA             ;COMPLIMENT LOW LINE NUMBER
        CMP     C       ;AND COMPARE TO START
        JNZ     EDIT6   ;BRIF NOT =
        ORA     A       ;NOT TEST FOR 00
        JNZ     EDIT6   ;THIS IS USUAL CASE
ED7B:   INX     D       ;POINT FORWARD
        LXI     H,IMMED ;POINT INSERT
        MOV     B,M     ;GET LENGTH
        CALL    COPYH   ;GO MOVE IT
        JMP     GETCM   ;GO GET ANOTHER COMMAND
;
; IRAM          INITIALIZE RAM
;       ZEROES RAM FROM BZERO TO EZERO
;       INITS RANDOM # CONSTANTS
;       RETURNS H=PTR TO TRND
;
IRAM:   LXI     H,BZERO ;CLEAR BZERO->EZERO
        MVI     B,EZERO-BZERO
        CALL    ZEROM
        LXI     D,NRNDX ;MOVE RANDOM # SERIES TO RNDX
        LXI     H,RNDX
        MVI     B,8     ;COUNT
        JMP     COPYD   ;MOVE IT & RETURN
;PAGE
EXEC    EQU     $
;
;
; DECODE COMMAND IN IOBUFF
; EXECUTE IF POSSIBLE
; THEN GOTO GET NEXT COMMAND
;
;
        STA     MULTI   ;RESET MULTI SW
        STA     FNMOD   ;RESET FN TYPE
        INR     A       ;GET A ONE
        STA     RUNSW   ;SET IMMEDIATE MODE
        LXI     H,IOBUF+1       ;POINT SMT
        LXI     D,IMMED ;POINT NEW AREA
EXEC1:  MOV     A,M     ;GET A BYTE
        STAX    D       ;PUT TO (D,L)
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        ORA     A       ;TEST BYTE
        JNZ     EXEC1   ;CONTINUE
        LXI     H,NULLI ;POINT NO LINE NUM
        SHLD    LINE    ;SAVE ADDR
        LXI     H,IMMED ;POINT START OF CMMD
        JMP     RUN3    ;GO INTO RUN PROCESSOR
;
NEW     EQU     $
;
; NEW COMMAND
; 'NEW'==>CLEAR PROGRAM AND DATA
; 'NEW*'==>CLEAR PROGRAM ONLY
;
        PUSH    H       ;SAE PTR
        LXI     H,GETCM ;MAKE SUBROUTINE
        XTHL            ;RESTORE H
        RST     1       ;GET 1ST NON-BLANK CHAR AFTER 'NEW'
        SBI     '*'     ;TEST
        JZ      NEW1    ;BRIF PROGRAM CLEAR ONLY
NEW0:   XRA     A       ;GET A ZERO
        LHLD    DATAB   ;POINT DATA AREA
        MOV     M,A     ;CLEAR IT
NEW1:   LXI     H,BEGPR ;POINT START
        SHLD    PROGE   ;RESET PROGRAM END
        MOV     M,A     ;CLEAR IT
        RET
;
FREE    EQU     $
;
; FREE COMMAND
; COMPUTE AMOUNT OF AVAILABLE STORAGE (EXCLUDING DATA AREA)
;
        LHLD    DATAB   ;GET DATA BEG ADDRESS
        XCHG            ;PUT IN D,E
        LHLD    PROGE   ;GET PROGRAM END ADDRESS
        MOV     A,E     ;LO ADDR TO REG A
        SUB     L       ;SUBTRACT
        MOV     E,A     ;SAVE IT
        MOV     A,D     ;HI ADDR TO REG A
        SBB     H       ;SUBTRACT
        MOV     D,A     ;SAVE IT
        CALL    BINFL   ;GO FLOAT D,E
        LXI     H,IOBUF ;POINT BUFFER
        CALL    FOUT    ;GO CONVERT TO OUTPUT
        MVI     M,0     ;MARK END
        CALL    TERMO   ;GO WRITE IT
        JMP     GETCM   ;CONTINUE
;
TAPE    EQU     $
;
; TAPE COMMAND. DON'T ECHO INPUT. CONTINUE UNTIL KEY
; COMMAND.
;
        MVI     A,1     ;SET TAPE INPUT SWITCH
        STA     TAPES   ;STORE IT
        MVI     A,11H   ;GET DC1 (=READER ON)
        CALL    TESTO   ;WRITE IT
        JMP     GETCM   ;GO PROCESS INPUT
;
ENDIT   EQU     $
;
; END COMMAND. IF TAPE PUNCH SWITCH IS ON, PUNCH 'KEY' THEN
; CONTINUE
;
        LDA     TAPES   ;GET PAPER TAPE SWITCH
        CPI     2       ;TEST FOR SAVE
        JNZ     RDY     ;BRIF NOT
        LXI     H,KEYL  ;POINT 'KEY'
        CALL    TERMM   ;WRITE IT
        CALL    HDRTL   ;GO PUT TRAILER
;
; KEY COMMAND. RESET TAPE SWITCH. TURN READER OFF
;
KEY:    XRA     A       ;RESET TAPE SWITCH
        STA     TAPES
        LXI     H,PCHOF ;POINT READER/PUNCH OFF
        JMP     RDYM    ;PRINT POFF+READY MESSAGE
;
HDRTL   EQU     $
;
; PUNCH HEADER OR TRAILER ON PAPER TAPE.
;
        MVI     B,25    ;LOAD COUNT
HDR1:   MVI     A,0FFH  ;LOAD RUBOUT
        CALL    TESTO   ;WRITE IT
        DCR     B       ;DECREMENT COUNT
        XRA     A       ;ZERO A
        CMP     B       ;TEST COUNT
        RZ              ;RETURN ON ZERO
        JMP     HDR1    ;CONTINUE
;PAGE
;
; RUN PROCESSOR, GET NEXT STATMENT, AND EXECUTE IT
; IF IN IMMEDIATE MODE, THEN RETURN TO GETCMMD
;
RUNCM:  XRA     A       ;PUT A ZERO TO A
        LHLD    DATAB   ;GET ADDRESS OF DATA POOL
        MOV     M,A     ;INITIALIZE TO 0
XEQ     EQU     $       ;START FOR EXECUTION WITH OLD DATA
        CALL    IRAM    ;INITALIZE START OF RAM
        LXI     H,BEGPR-1       ;POINT 1 PRIOR TO BEGIN
        SHLD    DATAP   ;RESTORE DATA STMT POINTER
        MVI     M,0     ;RESET DATA STMT POINTER
        INX     H       ;POINT TO START
        SHLD    STMT    ;SAVE IT
        JMP     RUN2    ;GO PROCESS IT
;
; STATEMENTS RETURN HERE TO CONTINUE PROCESSING
RUN:    LXI     H,MULTI ;POINT MULTIPLE SWITCH
        MOV     A,M     ;GET SW
        ORA     A       ;TEST IT
        JZ      RUN1    ;BRIF NOT ON
        MVI     M,0     ;ELSE, RESET IT
        LHLD    ENDLI   ;GET ADDRESS
        JMP     RUN3    ;GO PROCESS REMAIN
RUN1:   LHLD    STMT    ;ELSE, GET ADDR OF PREV STMT
        MOV     E,M     ;GET LEN CODE
        MVI     D,0     ;CLEAR HIGH BYTE OF ADDR
        DAD     D       ;INCR STMT POINTER
        SHLD    STMT    ;SAVE IT
RUN2:   LDA     RUNSW   ;GET RUN TYPE
        ORA     A       ;TEST IT
        JNZ     GETCM   ;BRIF IMMEDIATE MODE
        MOV     A,M     ;GET LEN CODE
        ORA     A       ;TEST IF END
        JZ      ENDIT   ;BRIF IS
        INX     H       ;POINT LINE NUMBER
        SHLD    LINE    ;SAVE ADDR
        INX     H       ;POINT 2ND BYTE
        INX     H       ;POINT 1ST PGM BYTE
;
; ENTER HERE TO DO IMMEDIATE COMMAND
RUN3:   RST     1       ;SKIP BLANKS
RUN4:   SHLD    ADDR1   ;SAVE ADDR
        CALL    TSTCC   ;GO SEE IF CONTROL-C OR O
        LXI     D,JMPTB ;POINT TO TABLE
        CALL    SEEK1   ;GO SEARCH COMMAND TABLE
        JZ      RUN7    ;BRIF COMMAND NOT FOUND
        PUSH    H       ;SAVE H,L
        LDAX    D       ;LOAD LOW BYTE
        MOV     L,A     ;LOW BYTE TO L
        INX     D       ;POINT NEXT
        LDAX    D       ;LOAD HIGH BYTE
        MOV     H,A     ;HIGH BYTE TO H
        XTHL            ;COMMAND ADDRESS TO STACK
        RET             ;JUMP TO ROUTINE
RUN7:   LHLD    ADDR1   ;RESTORE H,L POINTER
        JMP     LET     ;ASSUME IT'S LET STMT
;PAGE
;
; SAVE COMMAND. TURN THE PUNCH ON THEN LIST PROGRAM
;
SAVE:   MVI     A,2     ;SET PUNCH MODE
        STA     TAPES
        MVI     A,12H   ;GET DC2 (=PUNCH ON)
        CALL    TESTO   ;WRITE IT
        CALL    HDRTL   ;GP PUT HEADER
;
LIST    EQU     $
;
;
; LIST PROCESSOR
; DUMP THE SOURCE PROGRAM TO TTY OR PAPER TAPE
;
;
        RST     1       ;SKIP TO NON BLANK
        LXI     D,0     ;GET A ZERO IN D
        XCHG            ;FLIP TO H,L
        SHLD    LINEL   ;SAVE IT
        LXI     H,9999H ;GET HIGH NUMBER IN H,L
        SHLD    LINEH   ;SAVE IT
        XCHG            ;FLIP BACK
        ORA     A       ;TEST IF EOL
        JZ      LIST1   ;BRIF IT IS
        CALL    PACK    ;GO PACK THE NUMBER, IF ANY
        MOV     D,B     ;COPY NUMBER TO D,L
        MOV     E,C     ;SAME
        XCHG            ;FLIP TO H,L
        SHLD    LINEL   ;SAVE IT
        SHLD    LINEH   ;SAME
        XCHG            ;RESTORE H,L
        RST     1       ;SKIP TO NON BLANK
        CPI     ','     ;TEST IF COMMA
        JNZ     LIST1   ;BRIF NOT
        INX     H       ;POINT NEXT
        RST     1       ;SKIP TO NON-BLANK
        CALL    PACK    ;ELSE, GO GET THE NUMBER
        MOV     H,B     ;COPY TO
        MOV     L,C     ;D,L
        SHLD    LINEH   ;SAVE IT
LIST1:  LXI     H,BEGPR ;POINT BEGINNING OF PROGRAM
LIST2:  CALL    TSTCC   ;GO SEE IF CONTROL-C OR CONTROL-O
        MOV     A,M     ;GET LEN CODE
        ORA     A       ;TEST IF END OF PROGRAM
        JZ      ENDIT   ;BRIF END OF PGM
        SUI     3       ;SUBTRACT THREE
        MOV     B,A     ;SAVE LEN
        INX     H       ;POINT HIGH BYTE OF LINE#
        XCHG            ;FLIP H,L TO D,E
        LHLD    LINEL   ;GET LOW LINE TO TEST
        XCHG            ;RESTORE H,L
        MOV     A,M     ;GET LOW BYTE OF LINE NUMBER
        CMP     D       ;COMP WITH LINEL
        JC      LIST8   ;BRIF LESS
        JNZ     LIST4   ;BRIF NOT EQUAL
        INX     H       ;POINT NEXT
        MOV     A,M     ;GET NEXT BYTE OF LINE#
        DCX     H       ;POINT BACK
        CMP     E       ;COMP LOW BYTES
        JC      LIST8   ;BRIF LESS
LIST4:  XCHG            ;SAVE H,L IN D,E
        LHLD    LINEH   ;GET HIGH LINE FOR TEST
        XCHG            ;RESTORE H,L
        MOV     A,M     ;GET LINE BYTE
        CMP     D       ;COMPARE HIGH BYTES
        JZ      LIST5   ;BRIF EQUAL
        JNC     ENDIT   ;BRIF HIGHER
        JMP     LIST6   ;GO AROUND
LIST5:  INX     H       ;POINT NEXT
        MOV     A,M     ;GET NEXT BYTE
        DCX     H       ;POINT BACK
        CMP     E       ;COMPARE LOW BYTES
        JZ      LIST6   ;BRIF EQUAL
        JNC     ENDIT   ;BRIF HIGHER
LIST6:  LXI     D,IOBUF ;POINT BUFFER AREA
        CALL    LINEO   ;CONVERT LINE NUMBER
LIST7:  MOV     A,M     ;GET A BYTE
        STAX    D       ;PUT IT TO BUFFER
        INX     D       ;POINT NEXT BUFF
        INX     H       ;POINT NEXT PROG
        DCR     B       ;DECR CTR
        JNZ     LIST7   ;LOOP
        PUSH    H       ;SAVE HL ADDR
        CALL    TERMO   ;GO TYPE IT
        POP     H       ;RETRIEVE H ADDR
        JMP     LIST2   ;CONTINUE
LIST8:  MOV     E,B     ;PUT LEN  IN E
        MVI     D,0     ;CLEAR D
        DAD     D       ;POINT NEXT STMT
        INX     H       ;POINT NEXT
        INX     H       ;POINT LEN CODE
        JMP     LIST2   ;GO LIST IT
;
;
CONTI   EQU     $
;
; CONTINUE EXECUTION AT STATEMENT FOLLOWING STOP OR AT
; STATEMENT THAT WAS INTERRUPTED WHEN CONTROL-C WAS TYPED
;
;
        LXI     H,LINEN ;POINT LINE NUMBER OF LAST STOP/ERROR/
        MOV     A,M     ;GET 1ST CHAR
        ORA     A       ;TEST IF IMMED CMMD
        JZ      LET     ;BRIF IF IMMED CMMD
;PAGE
;
;
; STMT:  GOTO NNNN
;
;
GOTO:   XRA     A       ;CLEAR REG A
        STA     EDSW    ;RESET IMMED MODE (IF IT WAS SET)
        STA     RUNSW   ;AND RUN TYPE
        CALL    NOTEO   ;ERROR IF END-OF-LINE
        CALL    PACK    ;GO GET LINE NUMBER IN B,C
        CALL    EOL     ;ERROR IF NOT END-OF-LINE
GOTO2:  CALL    LOCAT   ;GO SEARCH FOR REQUESTED LINE #
        JC      ULERR   ;BRIF NOT FOUND
        SHLD    STMT    ;SAVE ADDR
        XRA     A       ;GET A ZERO
        STA     MULTI   ;TURN OFF MULTIPLE STMTS
        JMP     RUN2    ;GO PROCESS THE STATEMENT
;
;
; STMT: RESTORE
;
RESTO:  CALL    EOL     ;ERROR IF NOT END-OF-LINE
        LXI     H,BEGPR-1       ;POINT 1 BEFORE START OF PROGRAM
        SHLD    DATAP   ;FORCE NEXT DATA TO BE AT START
        JMP     RUN     ;GO NEXT STMT
;
;
; STMT:  RETURN
;
RETUR:  CALL    EOL     ;ERROR IF NOT END-OF-LINE
        POP     PSW     ;POP THE STACK
        CPI     0FFH    ;TEST IF GOSUB IN EFFECT
        JNZ     RTERR   ;BRIF ERROR
        POP     H       ;GET RETURNED STATMENT ADDRESS
        SHLD    STMT    ;RESTORE
        POP     H       ;GET ENDLINE VALUE
        SHLD    ENDLI   ;RESTORE
        POP     PSW     ;GET MULTI SW VALUE
        STA     MULTI   ;RESTORE
        JMP     RUN     ;CONTINUE (AT STMT FOLLOWING GOSUB)
;
;
; STMT:  GOSUB NNNN
;
GOSUB:  CALL    NOTEO   ;ERROR IF END-OF-LINE
        CALL    PACK    ;GET LINE NUMBER
        CALL    EOL     ;ERROR IF NOT END-OF-LINE
GOSU1:  LDA     MULTI   ;GET SW SETTING
        PUSH    PSW     ;SAVE ON STACK
        LHLD    ENDLI   ;GET ADDR OF END OF STMT
        PUSH    H       ;SAVE ONE STACK
        LHLD    STMT    ;GET STATEMENT ADDRESS
        PUSH    H       ;SAVE RETURN ADDRESS IN STACK
        MVI     A,0FFH  ;MARK AS GOSUB
        PUSH    PSW     ;SAVE STATUS
        JMP     GOTO2   ;GO LOOKUP LINE AND BRANCH
;PAGE
;
PRINT   EQU     $
;
;
; STMT: PRINT ....
;
;
        XRA     A       ;CLEAR REG A
PRIN4:  STA     PRSW    ;SET SW TO SAY CRLF AT END OF LINE
        LXI     D,IOBUF ;POINT BUFFER
        RST     1       ;SKIP TO NEXT FIELD
;
        CALL    TSTEL   ;TEST IF END OF STMT
        JZ      PRINC   ;BRIF IT IS
        CPI     ','     ;TEST IF COMMA
        JZ      PRIN8   ;BRIF IT IS
        CPI     ';'     ;TEST IF SEMI-COLON
        JZ      PRIN9   ;BRIF IT IS
        PUSH    D       ;SAVE D,E
        PUSH    H       ;SAVE H,L
        LXI     D,TABLI ;POINT LITERAL
        RST     2       ;GO SEE IF TAB(XX)
        JZ      PRINA   ;BRIF IS
        POP     H       ;ELSE, RESTORE H,L
        CALL    EXPR    ;GO EVALUATE EXPRESSION
        POP     D       ;RESTORE D,E
        PUSH    H       ;SAVE H,L
        XCHG            ;FLIP/FLOP
        LDA     NS      ;GET TYPE OF RESULT
        CPI     0E7H    ;TEST IF STRING
        JZ      PRIN5   ;BRIF IS
        CALL    FOUT    ;GO CONVERT OUTPUT
        INX     H       ;POINT NEXT
PRIN7:  XCHG            ;FLIP/FLOP: END ADDR TO DE
        POP     H       ;RESTORE H,L
;HERE AFTER SETTING UP VALUE TO PRINT IN BUFFER
PRIN2:  MVI A,0FEH      ;SET END CODE=NO CRLF
        STAX D          ;PUT TO BUFFER
        PUSH H          ;SAVE H,L
        CALL TERMO      ;GO PRINT BUFFER
        POP H           ;RESTORE HL
        JMP PRINT       ;REPEAT FOR NEXT FIELD
;
PRIN5:  LXI     D,STRIN ;POINT STRING
        LDAX    D       ;GET LEN
        ORA     A       ;TEST IT
        JZ      PRIN7   ;BRIF NULL
        MOV     B,A     ;SAVE LEN
PRIN6:  INX     D       ;POINT NEXT
        LDAX    D       ;GET A BYTE
        MOV     M,A     ;STORE IT
        INX     H       ;POINT NEXT
        DCR     B       ;DECR CTR
        JNZ     PRIN6   ;LOOP
        JMP PRIN7       ;DIDDLE DE, HL AND CONTINUE
;
PRIN8:  CALL    TABST   ;GO POSITION NEXT TAB
PRIN9:  INX     H       ;PRINT NEXT
        MVI     A,1     ;GET SETTTING FOR SW
        JMP     PRIN4   ;GO STORE A IN PRSW & DO NEXT FIELD
PRINA:  POP     D       ;GET RID OF STACK ENTRY
        CALL    EXPR    ;GO EVALUATE
        PUSH    H       ;SAVE H,L
        CALL    FBIN    ;CONVERT TO BINARY
        PUSH    PSW     ;SAVE SPECIFIED COLUMN
        LXI     H,COLUM ;POINT CURRENT POSITION
        SUB     M       ;SUBTRACT (LEAVES NUMBER OF FILLS)
        CM      CRLF    ;NEXT LINE IF ALREADY PAST
        POP     PSW     ;RESTORE COL
        SUB     M       ;GET NUMBER FILLS
        POP     H
        POP     D
        MOV     B,A     ;SAVE COUNT
        MVI     A,' '   ;GET FILL
PRINB:  JZ      PRIN2   ;BRIF COUNT ZERO
        STAX    D       ;PUT ONE SPACE
        INX     D       ;POINT NEXT
        DCR     B       ;DECR CTR
        JMP     PRINB   ;LOOP
;
PRINC:  CALL EOL        ;SAVE EOL POSITION
;HERE TO PRINT FINAL CR/LF (OR NOT) AND GO TO NEXT STATEMENT
        LDA     PRSW    ;GET SWITCH
        MOV     B,A     ;SAVE ,; SWITCH
        LDA     OUTSW   ;GET CONTROL-O SWITCH
        ORA     A       ;TEST IF ^O IN EFFECT
        ORA     B       ;AND IF STATEMENT ENDED IN , OR ;
        CZ      CRLF    ;CRLF IF NEITHER
        JMP     RUN     ;CONTINUE NEXT STATEMENT
;PAGE
;
FOR     EQU     $
;
;
;  STMT:  FOR VAR = EXPR TO EXPR [STEP EXPR]
;
;
;  FIRST EVALUATE ARGUMENTS AND STORE POINTERS AND VALUES,
;  BUT DO NOT MAKE TABLE ENTRY YET
        CALL    VAR     ;NEXT WORD MUST BE VARIABLE
        XCHG            ;FLIP/FLOP
        SHLD    INDX    ;SAVE VARIABLE NAME
        XCHG            ;FLIP/FLOP AGAIN
        CPI     '='     ;TEST FOR EQUAL SIGN
        JNZ     SNERR   ;BRIF NO EQUAL
        INX     H       ;POINT NEXT
        CALL    EXPR    ;GO EVALUATE EXPR, IF ANY
        XCHG            ;FLIP/FLOP AGAIN
        LHLD    INDX    ;GET INDEX NAME
        XCHG            ;FLIP/FLOP
        PUSH    H       ;SAVE H,L
        CALL    SEARC   ;GO LOCATE NAME
        XCHG            ;PUT ADDR IN H,L
        SHLD    ADDR1   ;SAVE ADDR
        RST     3       ;GO STORE THE VALUE
        POP     H       ;RESTORE POINTER TO STMT
        LXI     D,TOLIT ;GET LIT ADDR
        RST     2       ;GO COMPARE
        JNZ     SNERR   ;BRIF ERROR
        CALL    EXPR    ;GO EVALUATE TO-EXPR
        PUSH    H       ;SAVE H,L
        LXI     H,TVAR1 ;POINT 'TO' VALUE
        RST     3       ;SAVE IT
        LXI     H,ONE   ;POINT CONSTANT: 1
        RST     5       ;LOAD IT
        POP     H       ;GET H,L
        MOV     A,M     ;GET THE CHAR
        ORA     A       ;TEST FOR END OF STATEMENT
        JZ      FOR2    ;BRIF NO STEP
        PUSH    H       ;RE-SAVE
        LXI     D,STEPL ;TEST FOR LIT 'STEP'
        RST     2       ;GO COMPARE
        JZ      FOR1    ;BRIF STEP
        POP     H       ;RESTORE H,L
        JMP     FOR2    ;GO NO STEP VALUE
FOR1:   POP     D       ;POP OFF THE STACK
        CALL    EXPR    ;GO EVALUATE EXPRESSION
FOR2:   PUSH    H       ;SAVE H,L TO END OF STATEMENT
        LXI     H,TVAR2 ;POINT STEP VALUE
        RST     3       ;SAVE IT
        POP     H       ;RESTORE H,L
        CALL    EOL     ;ERROR IF NOT END-OF-LINE
; DETERMINE WHETHER LOOP IS TO BE EXECUTED AT ALL
; (IF VALUE > "TO" VALUE AND STEP POSITIVE,
;    JUST SKIP TO NEXT, ETC)
        CALL    FTEST   ;GET STATUS OF FACC
        PUSH    PSW     ;SAVE A,STATUS
        LXI     H,TVAR1 ;GET END VALUE
        RST     5       ;LOAD IT
        POP     PSW     ;RESTORE STATUS
        JP      FOR4    ;BRIF FOR IS POSITIVE
        LHLD    ADDR1   ;GET ADDRESS OF INDEX
        CALL    FSUB    ;COMPARE THIS AGAINST END VALUE
        JZ      FOR5    ;BRIF START = END
        JM      FOR5    ;BRIF START > END
        JMP     FOR9    ;GO LOCATE MATCHING NEXT
FOR4:   LHLD    ADDR1   ;GET ADDRESS OF INDEX
        CALL    FSUB    ;COMPARE
        JZ      FOR5    ;BRIF START = END
        JM      FOR9    ;BRIF START > END: SKIP TO "NEXT"
; LOOP IS TO BE EXECUTED AT LEAST ONCE:
; NEED AN ENTRY IN FOR-NEXT TABLE.
; SEE IF THERE IS ALREADY ENTRY FOR THIS VARIABLE
; (IE PROGRAM JUMPED OUT OF LOOP EARLIER)
FOR5:   LXI     D,FORNE ;POINT TABLE
        LHLD    INDX    ;GET INDEX VARIABLE NAME
        XCHG            ;FLIP/FLOP
        MOV     A,M     ;GET COUNT OF ENTRIES NOW IN TABLE
        MOV     B,A     ;STORE IT
        MVI     C,1     ;NEW CTR
        ORA     A       ;TEST IF ZERO
        INX     H       ;POINT
        JZ      FOR8    ;BRIF TABLE EMPTY
FOR6:   MOV     A,M     ;GET 1ST BYTE OF TABLE VARIABLE
        CMP     D       ;TEST IF EQUAL TO THIS FOR'S INDEX
        JNZ     FOR7    ;BRIF NOT
        INX     H       ;POINT NEXT
        MOV     A,M     ;GET NEXT BYTE
        DCX     H       ;POINT BACK
        CMP     E       ;TEST IF EQUAL
        JZ      FOR8    ;BRIF EQUAL
FOR7:   RST     4       ;ADJUST H,L
        DB      14
        INR     C       ;COUNT IT
        DCR     B       ;DECR CTR
        JNZ     FOR6    ;LOOP
; ENTER THIS FOR IN TABLE (WHERE HL POINTS)
FOR8:   MOV     A,C     ;GET UDPATE COUNT
        CPI     9       ;TEST IF TBL EXCEEDED
        JNC     NXERR   ;ERROR IF MORE THAN 8 OPEN FOR/NEXT
        STA     FORNE   ;PUT IN TABLE
        MOV     M,D     ;HI BYTE INDEX VARIABLE NAME
        INX     H       ;POINT NEXT
        MOV     M,E     ;STORE LO BYTE
        INX     H       ;POINT NEXT
        PUSH    H       ;SAVE H,L
        LXI     H,TVAR2 ;POINT STEP VALUE
        RST     5       ;LOAD IT
        POP     H       ;RESTORE H,L
        RST     3       ;STORE IN STACK
        PUSH    H       ;SAVE H,L
        LXI     H,TVAR1 ;POINT 'TO' VALUE
        RST     5       ;LOAD IT
        POP     H       ;RESTORE H,L
        RST     3       ;STORE IN STACK
        XCHG            ;FLIP/FLOP
        LHLD    ENDLI   ;GET END ADDR
        DCX     H       ;POINT ONE PRIOR
        XCHG            ;FLIP BACK
        MOV     M,D     ;STORE IT
        INX     H       ;POINT NEXT
        MOV     M,E     ;STORE IT
        INX     H       ;POINT NEXT
        LDA     STMT+1  ;GET HIGH STMT ADDR
        MOV     M,A     ;PUT IT
        INX     H       ;POINT NEXT
        LDA     STMT    ;GET LOW STMT ADDR
        MOV     M,A     ;PUT IT
        JMP     RUN     ;CONTINUE
;
; IF HERE, THIS LOOP IS TO BE EXECUTED ZERO TIMES:
; SCAN THRU PROGRAM TO FIND MATCHING "NEXT".
; THIS CODE WILL FAIL IF USER'S PROGRAM IS TOO
; COMPLEX SINCE IT WON'T FOLLOW GOTO'S, IF'S, ETC.
FOR9:   LHLD    STMT    ;GET ADDRESS OF STATMENT
        MOV     E,M     ;GET LENGTH CODE
        MVI     D,0     ;INIT INCREMENT
        DAD     D       ;COMPUTE ADDR OF NEXT STATEMENT
        MOV     A,M     ;GET NEW LEN CODE
        ORA     A       ;SEE IF END OF PGM
        JZ      NXERR   ;BRIF IT IS
        SHLD    STMT    ;SAVE ADDRESS
        RST     4       ;ADJUST H,L
        DB      3
        RST     1       ;SKIP SPACES
        LXI     D,NEXTL ;POINT 'NEXT'
        RST     2       ;SEE IF IT IS A NEXT STMT
        JNZ     FOR9    ;LOOP IF NOT
        RST     1       ;SKIP SPACES
        LDA     INDX+1  ;GET FIRST CHAR
        CMP     M       ;COMPARE
        JNZ     FOR9    ;BRIF NOT MATCH NEXT
        LDA     INDX    ;GET 2ND CHAR
        INX     H       ;DITTO
        CPI     ' '     ;SEE IF SINGLE CHAR
        JZ      FORA    ;BRIF IT IS
        CMP     M       ;COMPARE THE TWO
        JNZ     FOR9    ;BRIF NOT EQUAL
FORA:   RST     1       ;SKIP TO END (HOPEFULLY)
        MOV     A,M     ;GET THE NON BLANK
        ORA     A       ;SEE IF END
        JNZ     FOR9    ;BRIF END
        JMP     RUN     ;ELSE, GO NEXT STMT
;PAGE
;
IFSTM   EQU     $
;
;
; STMT: IF EXPR RELATION EXPR THEN STMT#
;
;
        CALL    EXPR    ;GO EVALUATE LEFT EXPR
        PUSH    H       ;SAVE H,L
        LDA     NS      ;GET TYPE CODE
        STA     IFTYP   ;SAVE IT
        CPI     0E7H    ;TEST IF STRING
        JNZ     IF1     ;BRIF NOT
        LXI     H,IOBUF ;POINT BUFFER
        LXI     D,STRIN ;POINT RESULT
        LDAX    D       ;GET LEN
        INR     A       ;PLUS ONE
        MOV     B,A     ;SAVE IT
        CALL    COPYD   ;GO MOVE IT
        JMP     IF2     ;GO AROUND
IF1:    LXI     H,TVAR1 ;GET ADDR OF TEMP STORAGE
        RST     3       ;SAVE IT
IF2:    POP     H       ;RESTORE H,L
        XRA     A       ;CLEAR A
        MOV     C,A     ;SAVE IN REG C
        MOV     B,A     ;INIT REG
IF3:    MOV     A,M     ;GET OPERATOR
        INR     B       ;COUNT
        CPI     '='     ;TEST FOR EQUAL
        JNZ     IF4     ;BRIF IT IS
        INR     C       ;ADD 1 TO C
        INX     H       ;POINT NEXT
IF4:    CPI     '>'     ;TEST FOR GREATER THAN
        JNZ     IF5     ;BRIF IT IS
        INR     C       ;ADD TWO
        INR     C       ;TO REL CODE
        INX     H       ;POINT NEXT
IF5:    CPI     '<'     ;TEST FOR LESS THAN
        JNZ     IF6     ;BRIF IT IS
        MOV     A,C     ;GET REL CODE
        ADI     4       ;PLUS FOUR
        MOV     C,A     ;PUT BACK
        INX     H       ;POINT NEXT
IF6:    MOV     A,C     ;GET REL CODE
        ORA     A       ;TEST IT
        PUSH    B       ;SAVE B,C
        JZ      SNERR   ;BRIF SOME ERROR
        POP     B       ;RESTORE B,C
        STA     REL     ;SAVE CODE
        MOV     A,B     ;GET COUNT
        CPI     2       ;TEST FOR TWO
        JNZ     IF3     ;SEE IF MULTIPLE RELATION
        CALL    EXPR    ;GO EVALUATE RIGHT SIDE
        SHLD    ADDR1   ;SAVE LOCATION OF THEN (IF ANY)
        LDA     NS      ;GET TYPE CODE
        LXI     H,IFTYP ;POINT LEFT TYPE
        CMP     M       ;COMPARE
        JNZ     SNERR   ;BRIF MIXED
        CPI     0E7H    ;TEST IF STRING
        JZ      IFF     ;BRIF IS
        LXI     H,TVAR1 ;POINT LEFT
        CALL    FSUB    ;SUBTRACT LEFT FROM RIGHT
        LDA     REL     ;GET RELATION
        RAR             ;TEST BIT D0
        JNC     IF8     ;BRIF NO EQUAL TEST
        CALL    FTEST   ;GET STATUS OF FACC
        JZ      TRUE    ;BRIF LEFT=RIGHT
IF8:    LDA     REL     ;LOAD RELATION
        ANI     02H     ;MASK IT
        JZ      IF9     ;BRIF NO >
        CALL    FTEST   ;GET STATUS OF FACC
        JM      TRUE    ;BRIF GT
IF9:    LDA     REL     ;LOAD RELATION
        ANI     04H     ;MASK IT
        JZ      FALSE   ;BRIF NO <
        CALL    FTEST   ;GET STATUS OF FACC
        JM      FALSE   ;BRIF GT
        JZ      FALSE   ;BRIF ZERO (NOT EQUAL)
TRUE:   LHLD    ADDR1   ;GET POINTER TO STATEMENT
        LXI     D,GOTOL ;POINT 'GO TO'
        RST     2       ;GO COMPARE
        JZ      GOTO    ;BRIF IF ... GOTO NN
        LHLD    ADDR1   ;GET POINTER TO STATEMENT
        LXI     D,GOSBL ;POINT LITERAL
        RST     2       ;GO COMAPRE
        JZ      GOSUB   ;BRIF IF ... GOSUB NN
        LHLD    ADDR1   ;GET POINTER TO STATEMENT
        LXI     D,THENL ;GET ADDR 'THEN'
        RST     2       ;GO COMPARE
        JNZ     SNERR   ;BRIF ERROR
        CALL    NUMER   ;TEST IF NUMERIC
        JZ      GOTO    ;BRIF IT IS
        JMP     RUN4    ;ELSE, MAY BE ANY STMT
FALSE   EQU     RUN
IFF:    LXI     H,IOBUF ;POINT PRIOR
        MOV     B,M     ;GET LEN
        LXI     D,STRIN ;POINT THIS
        LDAX    D       ;GET LEN
        MOV     C,A     ;SAVE IT
IFG:    INX     D       ;POINT NEXT
        INX     H       ;DITTO
        MOV     A,B     ;GET LEFT LEN
        ORA     A       ;TEST IT
        JNZ     IFH     ;BRIF NOT ZERO
        MVI     M,' '   ;EXTEND WITH SPACE
IFH:    MOV     A,C     ;GET RIGHT LEN
        ORA     A       ;TEST IT
        JNZ     IFI     ;BRIF NOT ZERO
        MVI     A,' '   ;GET SPACE
        STAX    D       ;EXTEND
IFI:    LDAX    D       ;GET RIGHT CHAR
        CMP     M       ;TEST WITH LEFT
        JC      IFM     ;BRIF LEFT>RIGHT
        JNZ     IFN     ;BRIF LEFT<RIGHT
        MOV     A,B     ;GET LEFT COUNT
        DCR     A       ;SUBT ONE
        JM      IFJ     ;BRIF WAS ZERO
        MOV     B,A     ;UPDATE CTR
IFJ:    MOV     A,C     ;GET RIGHT LEN
        DCR     A       ;SUBT ONE
        JM      IFK     ;BRIF WAS ZERO
        MOV     C,A     ;UPDT CTR
IFK:    MOV     A,B     ;GET LEFT LEN
        ORA     C       ;COMPARE TO RIGHT
        JNZ     IFG     ;BRIF BOTH NOT ZERO
        MVI     B,1     ;SET SW= EQUAL
IFL:    LDA     REL     ;GET RELATION
        ANA     B       ;AND WITH RESULT
        JZ      FALSE   ;BRIF FALSE
        JMP     TRUE    ;ELSE, TRUE
IFM:    MVI     B,2     ;SET CODE
        JMP     IFL     ;JUMP
IFN:    MVI     B,4     ;SET CODE
        JMP     IFL     ;JUMP
;PAGE
;
LET     EQU     $
;
;
; STMT: [LET] VAR = EXPR
;
;
        CALL    GETS8   ;GO GET ADDRESS OF VARIABLE
        PUSH    B       ;SAVE NAME
        PUSH    D       ;SAVE ADDRESS
        RST     1       ;GET NEXT CHAR
        CPI     '='     ;TEST FOR EQUAL SIGN
        JZ      LET1    ;BRIF IS
        LDA     EDSW    ;GET MODE SW
        ORA     A       ;TEST IT
        JZ      SNERR   ;BRIF LET ERROR
        LXI     H,WHATL ;POINT LITERAL
        CALL    TERMM   ;GO PRINT IT
        JMP     GETCM   ;GO TO COMMAND
LET1:   INX     H       ;POINT NEXT
        CALL    EXPR    ;GO EVALUATE EXPRESSION
        CALL    EOL     ;ERROR IF NOT END-OF-LINE
        POP     H       ;RESTORE ADDRESSS
        POP     D       ;RESTORE NAME
        MOV     A,E     ;GET TYPE
        ORA     A       ;TEST IT
        LDA     NS      ;GET RESULT TYPE
        JM      LET2    ;BRIF STRING
        CPI     0E3H    ;TEST IF NUMERIC
        JNZ     SNERR   ;BRIF MIXED MODE
        RST     3       ;GO STORE VARIABLE
        JMP     RUN     ;CONTINUE
LET2:   CPI     0E7H    ;TEST IF STRING
        JNZ     SNERR   ;BRIF MIXED MODE
        CALL    LET2A   ;GO STORE IT
        JMP     RUN     ;CONTINUE
;
LET2A:  LXI     D,STRIN ;POINT STRING BUFFER
        LDAX    D       ;GET NEW LEN
        SUB     M       ;MINUS OLD LEN
        JZ      LET8    ;BRIF SAME LENGTH
        MOV     D,H     ;COPY H,L
        MOV     E,L     ;TO D,E
        MOV     A,M     ;GET LEN
        INR     A       ;TRUE LEN
LET3:   INX     D       ;POINT NEXT
        DCR     A       ;DECR CTR
        JNZ     LET3    ;LOOP
        INX     D       ;SKIP
        INX     D       ;AGAIN
        LDAX    D       ;GET LO NAM
        MOV     C,A     ;SAVE
        INX     D       ;GET HI NAME
        LDAX    D       ;LOAD IT
        MOV     B,A     ;SAVE
        PUSH    B       ;SAVE NAME
        DCX     H       ;POINT NEXT ENTRY
LET4:   MOV     A,M     ;GET NEXT
        ORA     A       ;TEST IF END
        JZ      LET6    ;BRIF IS
        PUSH    H       ;SAVE H,L
        DCX     H       ;SKIP NEXT
        DCX     H       ;POINT LEN
        MOV     B,M     ;GET HI LEN
        DCX     H       ;POINT LO
        MOV     C,M     ;GET LO LEN
        POP     H       ;RESTORE H,L
LET5:   MOV     A,M     ;GET A BYTE
        STAX    D       ;COPY
        DCX     H       ;POINT NEXT
        DCX     D       ;DITTO
        INX     B       ;ADD TO CTR
        MOV     A,B     ;GET HI
        ORA     C       ;TEST IF ZERO
        JNZ     LET5    ;LOOP
        JMP     LET4    ;CONTINUE
LET6:   XCHG            ;PUT NEW ADDR TO H,L
        POP     B       ;GET NAME
        MOV     M,B     ;STORE HI BYTE
        DCX     H       ;POINT NEXT
        MOV     M,C     ;STORE LO
        LXI     D,STRIN ;GET NEW LEN
        LDAX    D       ;LOAD IT
        MVI     B,0FFH  ;INIT HI COMPLEMENT
        ADI     5       ;COMPUTE ENTRY LENGTH
        JZ      LET7    ;BRIF 256 BYTES
        JNC     LET7    ;BRIF LESS 256
        MVI     B,0FEH  ;SET BIT OFF
LET7:   CMA             ;1'S COMPLEMENT
        INR     A       ;THEN 2'S
        MOV     C,A     ;SAVE LO LEN
        DCX     H       ;POINT NEXT
        MOV     M,B     ;STORE HI LEN
        DCX     H       ;POINT NEXT
        MOV     M,C     ;STORE LO LEN
        RST     4       ;ADJUST H,L
        DB      3
        DAD     B       ;COMPUTE END OF ENTRY
        MVI     M,0     ;MARK NEW END
        INX     H       ;POINT 1ST BYTE
LET8:   LDAX    D       ;GET LEN
        INR     A       ;TRUE LEN
        MOV     B,A     ;SAVE LEN
LET9:   LDAX    D       ;GET A BYTE
        MOV     M,A     ;COPY IT
        INX     H       ;POINT NEXT
        INX     D       ;DITTO
        DCR     B       ;SUBT CTR
        JNZ     LET9    ;LOOP
        RET             ;RETURN
;PAGE
;
;NEXT   EQQU    $
;
;
; STMT:  NEXT VAR
;
;
NEXT:   CALL    VAR     ;GET VARIABLE NAME
        CALL    EOL     ;ERROR IF NOT END-OF-LNE
        XCHG            ;FLIP/FLOP
        SHLD    INDX    ;SAVE VAR NAME
        PUSH    H       ;SAVE VAR NAME
        LXI     H,FORNE ;POINT FOR/NEXT TABLE
        MOV     B,M     ;GET SIZE
        MOV     A,B     ;LOAD IT
        ORA     A       ;TEST IT
        JZ      NXERR   ;BRIF TABLE EMPTY
        INX     H       ;POINT NEXT
        POP     D       ;RESTORE VAR NAME
NEXT1:  MOV     A,M     ;GET 1ST BYTE
        INX     H       ;POINT NEXT
        CMP     D       ;COMPARE
        JNZ     NEXT2   ;BRIF NOT EQUAL
        MOV     A,M     ;GET 2ND BYTE
        CMP     E       ;COMPARE
        JZ      NEXT3   ;BRIF EQUAL
NEXT2:  RST     4       ;ADJUST H,L
        DB      13
        DCR     B       ;DECR COUNT
        JNZ     NEXT1   ;LOOP
        JMP     NXERR   ;GO PUT ERROR MSG
NEXT3:  LDA     FORNE   ;GET ORIG COUNT
        SUB     B       ;MINUS REMAIN
        INR     A       ;PLUS ONE
        STA     FORNE   ;STORE NEW COUNT
        INX     H       ;POINT ADDR
        PUSH    H       ;SAVE H,L ADDR
        CALL    SEARC   ;GO GET ADDR OF INDEX
        XCHG            ;PUT TO H,L
        SHLD    ADDR1   ;SAVR IT
        RST     5       ;LOAD INDEX
        POP     H       ;GET H,L (TBL)
        PUSH    H       ;RE-SAVE
        CALL    FADD    ;ADD STEP VALUE
        LXI     H,TVAR1 ;POINT TEMP AREA
        RST     3       ;SAVE NEW INDEX
        POP     H       ;GET H,L (TBL)
        PUSH    H       ;RE-SAVE
        RST     4       ;GET LEN TO NEXT
        DB      4
        CALL    FSUB    ;SUBTRACT TO VALUE
        JZ      NEXT6   ;BRIF ZERO
        POP     H       ;GET H,L (PTR TO STEP)
        PUSH    H       ;RE-SAVE
        MOV     A,M     ;GET SIGN&EXPONENT OF STEP
        ORA     A       ;TEST IT
        LDA     FACC    ;GET SIGN & EXPON OF DIFF
        JM      NEXT5   ;BRIF NEGATIVE
        ORA     A       ;TEST SIGN OF DIFF
        JM      NEXT6   ;BRIF LESS THAN TO-EXPR
NEXT7:  LXI     H,FORNE ;GET ADDR TABLE
        DCR     M       ;SUBTRACT ONE FROM COUNT
        POP     D       ;ADJUST STACK
        JMP     RUN     ;GO STMT AFTER NEXT
NEXT5:  ORA     A       ;TEST SIGN OF DIFFERENCE
        JM      NEXT7   ;BRIF END OF LOOP
NEXT6:  POP     H       ;GET PTR TO TBL
        RST     4       ;ADJUST H,L
        DB      8
        MOV     D,M     ;GET HI BYTE
        INX     H       ;POINT NEXT
        MOV     E,M     ;GET LOW BYTE
        INX     H       ;POINT NEXT
        MOV     A,M     ;GET HI BYTE
        STA     STMT+1  ;SAVE
        INX     H       ;POINT NEXT
        MOV     A,M     ;GET LOW BYTE
        STA     STMT    ;SAVE
        XCHG            ;H,L = ADDR OF STMT AFTR FOR
        CALL    EOL     ;SETUP MULTI PTP
        LHLD    STMT    ;GET ADDR OF FOR STMT
        INX     H       ;POINT LINE NUM
        SHLD    LINE    ;SAVE ADDR LINE
        LXI     H,TVAR1 ;POINT UPDTED VALUE
        RST     5       ;GO LOAD IT
        LHLD    ADDR1   ;GET ADDR OF INDEX
        RST     3       ;GO STORE IT
        JMP     RUN     ;CONTINUE WITH STMT AFTER FOR
;PAGE
INPUT   EQU     $
;
;
; STMT:  INPUT VAR [, VAR, VAR]
;
;
        LXI     D,LLINE ;POINT 'LINE'
        PUSH    H       ;SAVE H,L ADDR
        RST     2       ;GO COMPARE
        JZ      INPL    ;BRIF EQUAL
        POP     D       ;ELSE, RESTORE H,L ADDR
        LXI     H,IOBUF ;GET ADDR OF BUFFER
        SHLD    ADDR1   ;SAVE ADDR
        MVI     M,0     ;MARK BUFFER EMPTY
        XCHG            ;FLIP/BACK
INPU1:  RST     1       ;SKIP SPACES
        CPI     27H     ;TEST IF QUOTE
        JZ      INPU2   ;BRIF IS
        CPI     '"'     ;TEST IF INPUT LITERAL
        JNZ     INPU6   ;BRIF NOT
INPU2:  MOV     C,A     ;SAVE DELIM
        LXI     D,IOBUF ;POINT BUFFER
INPU3:  INX     H       ;POINT NEXT
        MOV     A,M     ;LOAD IT
        CMP     C       ;TEST IF END
        JZ      INPU4   ;BRIF IS
        STAX    D       ;PUT TO BUFF
        INX     D       ;POINT NEXT
        JMP     INPU3   ;LOOP
INPU4:  INX     H       ;SKIP TRAILING QUOTE
        XCHG            ;PUT ADDR TO H,L
        MVI     M,0FEH  ;MARK END
        CALL    TERMO   ;GO PRINT PROMPT
        XCHG            ;GET H,L
        RST     1       ;SKIP TO NON BLANK
        CPI     ','     ;TEST IF COMMA
        JZ      INPU5   ;BRIF IS
        CPI     ';'     ;TEST IF COMMA
        JNZ     INPU6   ;BRIF NOT
INPU5:  INX     H       ;SKIP IT
INPU6:  CALL    GETS8   ;GO GET VAR ADDR
        PUSH    H       ;SAVE H ADDR
        PUSH    D       ;SAVE VAR ADDR
        LHLD    ADDR1   ;GET ADDR PREV BUFFER
        MOV     A,M     ;LOAD CHAR
        CPI     ','     ;TEST IF COMMA
        INX     H       ;POINT NEXT
        JZ      INPU7   ;BRIF CONTINUE FROM PREV
        MVI     A,'?'   ;LOAD PROMPT
        CALL    TERMI   ;GO READ FROM TTY
INPU7:  RST     1       ;SKIP SPACES
        MOV     A,C     ;GET LO NAME
        ORA     A       ;TEST IT
        JM      INPUA   ;BRIF STRING
        CALL    FIN     ;GO CONVERT TO FLOATING
        RST     1       ;SKIP SPACES
        CPI     ','     ;TEST IF COMMA
        JZ      INPU8   ;BRIF IS
        ORA     A       ;TEST IF END OF LINE
        JNZ     CVERR   ;BRIF ERROR
INPU8:  SHLD    ADDR1   ;SAVE ADDRESS
        POP     H       ;GET VAR ADDR
        RST     3       ;GO STORE THE NUMBER
INPU9:  POP     H       ;RESTORE STMT POINTER
        MOV     A,M     ;GET CHAR
        CPI     ','     ;TEST FOR COMMA
        INX     H       ;POINT NEXT
        JZ      INPU1   ;RECDURSIVE IF COMMA
        DCX     H       ;POINT BACK
INPUB:  CALL    EOL     ;ERROR IF NOT END OF LINE
        JMP     RUN     ;CONTINUE NEXT STMT
INPUA:  CALL    GETST   ;GO GET THE STRING
        SHLD    ADDR1   ;SAVE ADDRESS
        JMP     INPU9   ;CONTINUE
;
INPL    EQU     $
;
;
; STMT: INPUT LINE A$
;
;
        POP     D       ;DUMMY POP TO ADJUST STACK
        CALL    VAR     ;GET STRING NAME
        MOV     A,E     ;LOAD LO BYTE
        ORA     A       ;TEST IT
        JP      SNERR   ;BRIF NOT STRING VARIABLE
        CALL    SEARC   ;ELSE, GET ADDRESS
        PUSH    D       ;SAVE ON STACK
        CALL    EOL     ;ERROR IF NOT END-OF-LINE
        MVI     A,1     ;GET ON SETTING
        STA     ILSW    ;SET INPUT LINE SWITCH
        MVI     A,'?'   ;LOAD PROMPT
        CALL    TERMI   ;GO READ A LINE
        MVI     B,0     ;INIT COUNT
        LXI     D,STRIN+1       ;POINT STRING BUFFER
        LXI     H,IOBUF+1       ;POINT INPUT BUFFER
INPL1:  MOV     A,M     ;GET NEXT BYTE
        ORA     A       ;TEST IT
        JZ      INPL2   ;BRIF END
        INR     B       ;ADD TO COUNT
        STAX    D       ;PUT TO STRING BUFF
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        JMP     INPL1   ;LOOP
INPL2:  STA     ILSW    ;RESET SWITCH
        MOV     A,B     ;GET COUNT
        STA     STRIN   ;SET STRING LENGTH
        POP     H       ;GET ADDRESS OF VARIABLE
        CALL    LET2A   ;GO STORE THE STRING
        JMP     RUN     ;GO NEXT STMT
;PAGE
;
READ    EQU     $
;
; STMT: READ VAR [,VAR ...]
;
        RST     1       ;SKIP BLANKS
        CALL    GETS8   ;GET VAR ADDR
        PUSH    H       ;SAVE H,L
        PUSH    D       ;SAVE D,E
        LHLD    DATAP   ;GET DATA STMT POINTER
        MOV     A,M     ;LOAD THE CHAR
        ORA     A       ;TEST IF END OF STMT
        JNZ     READ2   ;BRIF NOT END OF STMT
        INX     H       ;POINT START NEXT STMT
READ1:  MOV     A,M     ;LOAD LEN
        SHLD    DATAP   ;SAVE ADDR
        ORA     A       ;TEST IF END OF PGM
        JZ      DAERR   ;BRIF OUT OF DATA
        RST     4       ;ADJUST H,L
        DB      3
        LXI     D,DATAL ;POINT 'DATA'
        RST     2       ;COMPARE
        JZ      READ2   ;BRIF IT IS DATA STMT
        LHLD    DATAP   ;GET ADDR START
        MOV     E,M     ;GET LEN CODE
        MVI     D,0     ;CLEAR D
        DAD     D       ;POINT NEXT STMT
        JMP     READ1   ;LOOP NEXT STMT
READ2:  RST     1       ;SKIP SPACES
        MOV     A,C     ;LOAD LO NAME
        ORA     A       ;TEST IT
        JM      READ6   ;BRIF STRING
        CALL    FIN     ;GO CONVERT VALUE
        MOV     A,M     ;GET CHAR WHICH STOPPED US
        CPI     ','     ;TEST IF COMMA
        JNZ     READ5   ;BRIF NOT
        INX     H       ;POINT NEXT
READ3:  SHLD    DATAP   ;SAVE ADDRESS
        POP     H       ;RESTORE ADDR OF VAR
        RST     3       ;STORE THE VALUE
READ4:  POP     H       ;RESTORE POINTER TO STM
        MOV     A,M     ;GET THE CHAR
        CPI     ','     ;TEST IF COMMA
        INX     H       ;POINT NEXT
        JZ      READ    ;RECURSIVE IF IT IS
        DCX     H       ;RESET
        JMP     INPUB   ;CONTINUE
READ5:  ORA     A       ;TEST IF END OF STMT
        JZ      READ3   ;BRIF OK
        JMP     CVERR   ;GO PROCESS ERROR
READ6:  CALL    GETST   ;GO GET STRING
        MOV     A,M     ;GET CHAR
        CPI     ','     ;TEST IF COMMA
        JZ      READ7   ;BRIF IS
        ORA     A       ;TEST IF END
        JNZ     READ5   ;BRIF NOT
        JMP     READ8   ;GO AROUND
READ7:  INX     H       ;POINT PAST
READ8:  SHLD    DATAP   ;SAVE ADDRESS
        JMP     READ4   ;CONTINUE
;
OUTP    EQU     $
;
; STMT; OUT ADDR,VALUE
;
;
        CALL    EXPR    ;GO EVALUATE ADDRESS
        MOV     A,M     ;GET DELIM
        CPI     ','     ;TEST IF COMMA
        JNZ     SNERR   ;BRIF NOT
        INX     H       ;SKIP OVER COMMA
        CALL    FBIN    ;CONVERT TO BINARY IN A-REG
        LXI     D,OUTA  ;POINT INSTR
        XCHG            ;PUT TO H,L
        MVI     M,0D3H  ;OUT INSTR
        INX     H       ;POINT NEXT
        MOV     M,A     ;PUT ADDR
        INX     H       ;POINT NEXT
        MVI     M,0C9H  ;RET INSTR
        XCHG            ;RESTORE ORIG H,L
        CALL    EXPR    ;GO EVAL DATA BYTE
        CALL    EOL     ;ERROR IF NOT END OF STATEMENT
        CALL    FBIN    ;CONVERT TO BINARY
        CALL    OUTA    ;GO PUT THE BYTE
        JMP     RUN     ;GO NEXT STMT
;PAGE
;
STOP    EQU     $
;
; STMT: STOP
;
;
        CALL    EOL     ;POINT END OF LINE
        LXI     H,STOPM ;POINT MESSAGE: "STOP AT LINE "
        CALL    TERMM   ;GO WRITE IT
        CALL    PRLIN   ;GO PRINT LINE NUMBER
        LDA     RUNSW   ;GET RUN TYPE
        ORA     A       ;TEST IT
        JNZ     RDY     ;BRIF IMMED
        STA     MULTI   ;CLEAR MULTI SW
        LHLD    STMT    ;GET ADDR OF PREV STMT
        MOV     E,M     ;GET LEN
        MVI     D,0     ;CLEAR HI BYTE
        DAD     D       ;POINT NEXT
        INX     H       ;POINT LINE NUMBER
        SHLD    LINE    ;SAVE ADDR
        LXI     D,LINEN ;POINT AREA
        CALL    LINEO   ;GO CONVERT LINE NUMBER
        XCHG            ;FLIP TO H,L
        MVI     M,0     ;MARK END
        JMP     RDY     ;GO TO READY MSG
;
RANDO   EQU     $
;
;
; STMT: RANDOMIZE
;
;
        CALL    EOL     ;ERROR IF NOT END-OF-LINE
        MVI     A,1     ;LOAD A ONE
        STA     RNDSW   ;SET SWITCH = TRUE RANDOM
        LXI     D,TRNDX ;POINT 'TRUE' RANDOM NUMBERS
        LXI     H,RNDX  ;POINT RECEIVE
        MVI     B,8     ;LOOP CTR
        CALL    COPYD   ;GO MOVE IT
        JMP     RUN     ;CONTINUE
;
ON      EQU     $
;
;
; STMT: ON EXPR GOTO NNN NNNN NNNN
;               GOSUB
;
;
        CALL    EXPR    ;GO EVALUATE EXPRESSION
        CALL    FBIN    ;GET BINARY NUMBER IN ACC
        ORA     A       ;TEST RESULT
        JZ      SNERR   ;BRIF ZERO (ERROR)
        MOV     C,A     ;SAVE VALUE
        DCR     C       ;LESS ONE
        XRA     A       ;GET A ZERO
        STA     REL     ;TURN OFF SWITCH
        LXI     D,GOTOL ;POINT LITERAL
        PUSH    H       ;SAVE H,L ADDRESS
        RST     2       ;GO COMPARE
        JZ      ON3     ;BRIF ON...GOTO
        POP     H       ;ELSE, RESTORE H,L
        LXI     D,GOSBL ;POINT LITERAL
        RST     2       ;GO COMPARE
        JNZ     SNERR   ;BRIF ERROR
        MVI     A,1     ;GET ON SETTING
        STA     REL     ;SET SWITCH
        PUSH    H       ;DUMMY PUSH
ON3:    POP     D       ;ADJUST STACK
ON3A:   MOV     A,C     ;GET COUNT
        ORA     A       ;TEST IT
        JZ      ON6     ;BRIF VALUE 1
        RST     1       ;ELSE, SKIP BLANKS
        ORA     A       ;TEST IF END OF LINE
        JZ      SNERR   ;BRIF IS
        CPI     ','     ;TEST IS COMMA
        JNZ     ON4     ;BRIF NOT
        INX     H       ;SKIP COMMA
        JMP     ON3A    ;CONTINUE
ON4:    CALL    NUMER   ;GO TEST IF NUMERIC
        JNZ     ON5     ;BRIF NOT
        INX     H       ;POINT NEXT
        JMP     ON4     ;LOOP
ON5:    DCR     C       ;SUB ONE FROM COUNT
        JNZ     ON3A    ;LOOP TILL JUST BEFORE STMT#
ON6:    CALL    NOTEO   ;ERROR IF NOT END-OF-LINE
        CPI     ','     ;TEST IF COMMA
        JNZ     ON7     ;BRIF NOT
        INX     H       ;POINT NEXT
        JMP     ON6     ;LOOP
ON7:    CALL    NUMER   ;TEST IF NUMERIC
        JNZ     SNERR   ;BRIF NOT
        CALL    PACK    ;GET THE LINE NUMBER
ON8:    MOV     A,M     ;GET NEXT CHAR
        CALL    TSTEL   ;TEST IF END STMT
        JZ      ON9     ;BRIF END
        INX     H       ;POINT NEXT
        JMP     ON8     ;LOOP
ON9:    CALL    EOL     ;SET END OF LINE POINTERS
        LDA     REL     ;GET TYPE (GOTO OR GOSUB)
        ORA     A       ;TEST IT
        JNZ     GOSU1   ;BRIF GOSUB
        JMP     GOTO2   ;BR TO GOTO LOOKUP
;PAGE
;
CHANG   EQU     $
;
; STATEMENT: CHANGE A$ TO X     - OR -
;
;            CHANGE X TO A$
;
        CALL    VAR     ;NEXT WORD MUST BE VAR
        MOV     A,E     ;TEST TYPE
        ORA     A       ;SET FLAGS
        JP      CHA2    ;BRIF NOT-STRING
        CALL    SEARC   ;GET ADDR
        PUSH    D       ;SAVE IT
        LXI     D,TOLIT ;POINT 'TO'
        RST     2       ;COMPARE
        JNZ     SNERR   ;BRIF ERROR
        CALL    VAR     ;GET NEXT VARIABLE
        MOV     A,D     ;GET HI NAME
        ORI     80H     ;SET MASK FOR ARRAY
        MOV     D,A     ;REPLACE
        CALL    SEARC   ;GET ADDRESS
        RST     4       ;POINT START OF ELEMENT 0,0
        DB      -11 AND 0FFH
        POP     D       ;GET PTR TO STMT
        XCHG            ;FLIP
        CALL    EOL     ;NEXT MUST BE E-O-L
        XCHG            ;FLIP AGAIN
        POP     D       ;GET ADDR STRING
        LDAX    D       ;GET COUNT
        MOV     B,A     ;SAVE IT
        INR     B       ;BUMP
CHA1:   PUSH    B       ;SAVE CTR
        PUSH    D       ;SAVE ADDR STRING
        PUSH    H       ;SAVE ADDR NUM
        CALL    FDEC    ;CONVERT TO F.P.
        POP     H       ;GET ADDR
        RST     3       ;STORE IT
        RST     4       ;POINT TO NEXT
        DB      -8 AND 0FFH
        POP     D       ;RESTORE STRING
        POP     B       ;AND CTR
        INX     D       ;POINT NEXT CHAR
        LDAX    D       ;LOAD IT
        DCR     B       ;DECR CTR
        JNZ     CHA1    ;LOOP
        JMP     RUN
;
;
CHA2:   MOV     A,D     ;GET HI NAME
        ORI     80H     ;MAKE ARRAY NAME
        MOV     D,A     ;SAVE
        CALL    SEARC   ;GET ADDR
        RST     4       ;POINT ELEMENT 0,0
        DB      -11 AND 0FFH
        XTHL            ;SAVE ON STACK
        LXI     D,TOLIT ;POINT 'TO'
        RST     2       ;COMPARE
        JNZ     SNERR   ;BRIF ERROR
        CALL    VAR     ;GET NAME
        MOV     A,E     ;GET TYPE
        ORA     A       ;SET FLAGS
        JP      SNERR   ;BRIF NOT STRING
        CALL    EOL     ;BRIF NOT E-O-L
        CALL    SEARC   ;GET ADDR
        POP     H       ;GET ADDR VAR
        PUSH    D       ;SAVE D,E
        LXI     D,STRIN ;POINT STRING BUFFER
        PUSH    D       ;SAVE IT
        RST     5       ;LOAD IT
        RST     4       ;POINT NEXT
        DB      -8 AND 0FFH
        PUSH    H       ;SAVE H,L
        CALL    FBIN    ;CONVERT
        POP     H       ;RESTORE
        POP     D       ;DITTO
        MOV     B,A     ;SAVE COUNT
        INR     B       ;BUMP IT
CHA3:   STAX    D       ;PUT TO STRING
        INX     D       ;POINT NEXT STR LOC.
        PUSH    B       ;SAVE CTRS
        PUSH    D       ;AND AD^DR
        RST     5       ;LOAD NEXT
        RST     4       ;POINT NEXT
        DB      -8 AND 0FFH
        PUSH    H       ;AND H ADDR
        CALL    FBIN    ;CONVERT
        POP     H       ;RESTORE H,L
        POP     D       ;AND D,E
        POP     B       ;AND CTRS
        DCR     B       ;DECR CTR
        JNZ     CHA3    ;LOOP
        POP     H       ;GET ADDR OF VAR (STRING)
        CALL    LET2A   ;GO STORE IT
        JMP     RUN     ;CONTINUE
;PAGE
;
DIM     EQU     $
;
; STMT: DIM VAR(A,B),...
;
;
        CALL    VAR     ;GO GET VAR NAME
        JP      SNERR   ;BRIF NO (
        CALL    SEARC   ;GO LOCATE THE VAR
        XTHL            ;PUT ADDR IN STACK, GET PTR TO (
        PUSH    PSW     ;SAVE STATUS
        MVI     A,0FFH  ;TURN ON SW
        STA     DIMSW   ;SET IT
        CALL    EXPR    ;GO EVALUATE
        POP     PSW     ;GET STATUS
        XTHL            ;SWAP PTRS
        PUSH    D       ;SAVE ROW NUMBER
        PUSH    B       ;SAVE COL NUMBER
        INX     B       ;INCREMENT COLUMNS
        INX     D       ;AND ROWS
        PUSH    H       ;SAVE H,L
        PUSH    PSW     ;RESAVE STATUS
        LXI     H,0     ;GET A ZERO
DIM1:   DAD     D       ;TIMES ONE
        DCX     B       ;DCR COLS
        MOV     A,B     ;GET HI
        ORA     C       ;PLUS LO
        JNZ     DIM1    ;LOOP
        POP     PSW     ;GET STATUS
        POP     D       ;GET ADDRESS
        DAD     H       ;TIMES TWO
        DAD     H       ;TIMES FOUR
        LXI     B,8     ;PLUS 2 (NAME AND DISP)
        JM      REDIM   ;GO RE-DIMENSION
        PUSH    H       ;SAVE PRODUCT
        DAD     B       ;ADD IT
        XCHG            ;FLIP/FLOP
        DCX     H       ;POINT LO NAME
        DCX     H       ;POINT HI DISP
        MOV     A,E     ;GET LO
        CMA             ;COMPLEMENT
        ADI     1       ;PLUS ONE
        MOV     E,A     ;RESTORE
        MOV     A,D     ;GET HI
        CMA             ;COMPLEMENT
        ACI     0       ;PLUS CARRY
        MOV     M,A     ;STORE IT
        DCX     H       ;POINT NEXT
        MOV     M,E     ;STORE LO
        XCHG            ;SAVE IN D,E
        POP     H       ;GET PRODUCT
        MOV     B,H     ;COPY H,L
        MOV     C,L     ;TO B,C
        XCHG            ;GET LOCAT
        POP     D       ;GET COLUMNS
        DCX     H       ;POINT NEXT
        MOV     M,D     ;MOVE LO COL
        DCX     H       ;POINT NEXT
        MOV     M,E     ;MOVE HI COL
        POP     D       ;GET ROWS
        DCX     H       ;POINT NEXT
        MOV     M,D     ;MOVE HI ROW
        DCX     H       ;POINT NEXT
        MOV     M,E     ;MOVE LO ROW
        DCX     H       ;POINT NEXT
DIM2:   MVI     M,0     ;CLEAR ONE BYTE
        DCX     H       ;POINT NEXT
        DCX     B       ;DECR CTR
        MOV     A,B     ;GET HI
        ORA     C       ;PLUS LO
        JNZ     DIM2    ;LOOP
        MVI     M,0     ;MARK END
DIM3:   POP     H       ;GET PTR TO STMT
        MOV     A,M     ;LOAD CHAR
        CPI     ','     ;TEST IF COMMA
        JNZ     DIM4    ;BRIF NOT
        INX     H       ;SKIP IT
        JMP     DIM     ;CONTINUE
DIM4:   CALL    EOL     ;TEST END OF LINE
        JMP     RUN     ;CONTINUE WITH PROGRAM
REDIM:  DAD     B       ;COMPUTE LEN TO NEXT
        DCX     D       ;POINT LO NAME
        DCX     D       ;POINT HI DISP
        LDAX    D       ;GET IT
        MOV     B,A     ;SAVE
        DCX     D       ;POINT LO DISP
        LDAX    D       ;GET IT
        MOV     C,A     ;SAVE
        DAD     B       ;COMPUTE DIFF OR PRIOR DIM AND THIS
        MOV     A,H     ;GET HI DIFF
        ORA     A       ;TEST IT
        JM      REDM1   ;BRIF PREV > NEW
        JNZ     SNERR   ;BRIF PREV < NEW
        MOV     A,L     ;GET LO DIFF
        ORA     A       ;TEST IT
        JNZ     SNERR   ;BRIF PREV < NEW
REDM1:  XCHG            ;PUT ADDR IN H,L
        DCX     H       ;POINT HI COL
        POP     D       ;GET COL
        MOV     M,D     ;MOVE HI
        DCX     H       ;POINT LO COL
        MOV     M,E     ;MOVE LO
        POP     D       ;GET ROW
        DCX     H       ;POINT HI ROW
        MOV     M,D     ;MOVE HI
        DCX     H       ;POINT LO ROW
        MOV     M,E     ;MOVE LO
        JMP     DIM3    ;CONTINUE
;PAGE
;
SIN     EQU     $
;
; COMPUTE SINE OF X, (X IN RADIANS)
;
; USES 4TH DEGREE POLYNOMIAL APPROXIMATION
;
;
; FIRST, REDUCE ANGLE TO RANGE: (-PI/2,PI/2)
;
        CALL    FTEST   ;GET STATUS OF ANGLE
        RZ              ;SIN(0)=0
        PUSH    PSW     ;SAVE SIGN OF ANGLE
        CALL    ABS
SIN1:   POP     PSW     ;COMPLEMENT SIGN FOR EACH PI SUB'D
        CMA             ;..
        PUSH    PSW     ;..
        LXI     H,PI    ;REDUCE TO -PI<X<0
        CALL    FSUB
        JP      SIN1
        LXI     H,HALFP ;NOW ADD PI FOR -PI<X<-PI/2
        PUSH    H
        CALL    FADD
        CP      NEG     ;AND JUST NEGATE FOR -PI/2<X<0
        POP     H
        CALL    FADD
        POP     PSW     ;RESTORE SIGN
        ORA     A
        CP      NEG
;
; INIT REGISTERS
;
        LXI     H,TEMP1 ;POINT IT
        RST     3       ;SAVE IT
        LDA     FACC    ;GET SIGN&EXPONENT
        CALL    FEXP    ;EXPAND EXPON.
        JP      SIN3A   ;BRIF POSITIVE
        CPI     0FDH    ;TEST EXPONENT
        RC              ;RETURN IF VERY SMALL RADIAN
;
; ABOVE ROUTINE WILL APPROX SIN(X) == X FOR X: (-.06,.06)
;
SIN3A:  LXI     H,HALFP ;POINT PI/2
        CALL    FDIV    ;COMPUTE X/PI/2
        LXI     H,TEMP2 ;POINT T2
        RST     3       ;STORE IT
        LXI     H,TEMP2 ;POINT BACK
        CALL    FMUL    ;COMPUTE SQUARE
        LXI     H,SINCO ;POINT CONSTANTS
;
; EVALUATE POWER SERIES
;
; EVALUATE STARTING FROM HIGH ORDER COEFFICIENT:
;  F(X)=(...(CN*FACC+C(N-1))*FACC+...+C1)*FACC*TEMP2+TEMP1
;
;ON ENTRY:
;       TEMP1=CONSTANT TERM
;       TEMP2=X OR 1
;       FACC=X^2 OR X
;       (HL)=COEFFICIENT OF LAST TERM
;
EVPS:   PUSH    H       ;SAVE POINTER TO COEFFICIENTS
        LXI     H,TEMP3 ;SAVE FACC
        RST     3
        POP     H       ;RESTORE H
        PUSH    H
        JMP     EVPS2
EVPS1:  PUSH    H       ;SAVE PTR TO NEXT COEFFICIENT
        CALL    FADD    ;FACC+CN->FACC
        LXI     H,TEMP3 ;POINTER TO X^N
EVPS2:  CALL    FMUL    ;FACC*X^N->FACC
        POP     H       ;COEFFICENT PTR
        RST     4       ;MOVE TO NEXT COEFFICIENT
        DB      -4 AND 0FFH
        MOV     A,M     ;GET EXPONENT
        DCR     A       ;TEST FOR 1
        JNZ     EVPS1   ;BRIF NOT 1
        LXI     H,TEMP2 ;MUL BY TEMP2
        CALL    FMUL
        LXI     H,TEMP1 ;POINT TO CONSTANT TERM
        JMP     FADD    ;ADD IT AND RETURN TO CALLER
;
COS     EQU     $
;
;
; COMPUTE COSINE OF ANGLE, X EXPRESSED IN RADIANS
; USES THE TRANSFORMATION: Y = PI/2 +- X
;     AND THEN COMPUTES SIN(Y).
;
;
        LXI     H,HALFP ;COMPUTE PI/2 + X
        CALL    FADD    ;GO ADD
        JMP     SIN     ;GO COMPUTE SINE
;
TAN     EQU     $
;
; COMPUTE TANGENT OF X, IN RADIANS
; USES THE RELATION:
;
;          SIN(X)
; TAN(X) = ------
;          COS(X)
;
        LXI     H,TEMP4 ;POINT SAVE AREA
        RST     3       ;SAVE ANGLE
        CALL    COS     ;COMPUTE COS(X)
        LXI     H,TEMP7 ;SAVE COS(X)->TEMP7
        RST     3
        LXI     H,TEMP4 ;MOVE X->FACC
        RST     5
        CALL    SIN     ;COMPUTE SINE
        LXI     H,TEMP7 ;POINT COS
        JMP     FDIV    ;DIVIDE AND RETURN TO CALLER
;
ATN     EQU     $
;
; COMPUTES THE ARCTANGENT OF X
; USES A SEVENTH DEGREE POLYNOMIAL APPROXIMATION
;
        CALL    FTEST   ;CHECK SIGN OF ARGUMENT
        JP      ATN1    ;BRIF POSITIVE
        CALL    NEG     ;REVERSE SIGN
        CALL    ATN1    ;GET POSITIVE ATN
        JMP     NEG     ;MAKE NEG & RETURN
;
ATN1:   LXI     H,ONE   ;POINT: 1
        CALL    FADD    ;GO ADD
        LXI     H,TEMP1 ;POINT SAVE
        RST     3       ;STORE
        LXI     H,TWO   ;POINT: 2
        CALL    FSUB    ;GO SUBTRACT
        LXI     H,TEMP1 ;POINT SAVED
        CALL    FDIV    ;DIVIDE
        LXI     H,TEMP2 ;POINT SAVE
        RST     3       ;SAVE X'=(X-1)/(X+1)
        LXI     H,QTRPI ;X'+PI/4 -> TEMP1
        CALL    FADD
        LXI     H,TEMP1
        RST     3
        PUSH    H       ;SAVE PTR TO TEMP2
        RST     5       ;LOAD IT
        POP     H
        CALL    FMUL    ;FACC=X'*X'
        LXI     H,ATNCO ;POINT LIST COEFFICIENTS
        JMP     EVPS    ;GO COMPUTE & RETURN
;
LN      EQU     $
;
;
; COMPUTES THE NATRUAL LOGRITHM, LN(X)
; USES A 7TH DEGREE POLYNOMIAL APPROXIMATION
;
        CALL    FTEST   ;TEST THE ARGUMENT
        JM      ZMERR   ;LN(-X)=NO NO
        JZ      ZMERR   ;LN(0)=NO NO ALSO
        LXI     H,TEMP2 ;POINT SAVE AREA
        RST     3       ;STORE IT
        LDA     FACC    ;GET EXPON
        CALL    FEXP    ;EXPAND TO 8 BITS
        JZ      LN0     ;BRIF 0.5 < X < 1.0
        JP      LN1     ;BRIF POSITIVE EXPONENT
LN0:    CMA             ;ELSE COMPLIMENT
        ADI     2       ;PLUS TWO
        CALL    FDEC    ;CONVERT TO FLOAT POINT
        CALL    NEG     ;THEN NEGATE
        JMP     LN2     ;GO AROUND
LN1:    SBI     1       ;MINUS ONE
        CALL    FDEC    ;CONVERT TO FLOATING POINT
LN2:    LXI     H,LN2C  ;POINT LN(2)
        CALL    FMUL    ;MULTIPLY
        LXI     H,TEMP1 ;POINT SAVE AREA
        RST     3       ;STORE IT
        RST     5       ;GET ORIG X
        MVI     A,1     ;GET EXPONENT: 1
        STA     FACC    ;ADJUST TO RANGE (1,2)
        LXI     H,ONE   ;POINT 1
        PUSH    H       ;SAVE PTR TO ONE
        CALL    FSUB    ;SUBTRACT ONE
        POP     D       ;SET TEMP2=1
        LXI     H,TEMP2
        CALL    CPY4D
        LXI     H,LNCO  ;POINT COEFFICIENTS
        JMP     EVPS    ;APPROXIMATE & RETURN
;
; X=LOG(X) --- THIS IS LOG BASE 10.
;
LOG     EQU     $
        CALL    LN      ;COMPUTE NATURAL LOG
        LXI     H,LNC   ;POINT LOG(E)
        JMP     FMUL    ;MULTIPLY AND RETURN
;
EXP     EQU     $
;
;  COMPUTES EXP(X) USING ALGORITHM EXP(X)=(2^I)*(2^FP) WHERE
;  2^I=INT(X*LN BASE 2 OF E) AND,
;  2^FP=5TH DEGREE POLY. APPROXIMATION
;  FP=FRACTIONAL PART OF INT(X*LN2E)
;
        CALL    FTEST   ;CHECK SIGN
        JP      EXP1    ;BRIF POSITIVE
        CALL    NEG     ;ELSE, REVERSE SIGN
        CALL    EXP1    ;COMPUTE POSITIVE EXP
        LXI     H,TEMP1 ;POINT SAVE AREA
        RST     3       ;STORE IT
        LXI     H,ONE   ;POINT 1
        RST     5       ;LOAD IT
        LXI     H,TEMP1 ;POINT PREV
        JMP     FDIV    ;RECIPRICAL AND RETURN
;
EXP1:   LXI     H,LN2E  ;POINT LN BASE 2 OF E
        CALL    FMUL    ;FACC=X*(LN2E)
        LXI     H,TEMP3 ;POINT SAVE AREA
        RST     3       ;TEMP3=X*LN2E
        CALL    INT     ;FACC=INT(X*LN2E)
        LXI     H,TEMP4 ;POINT SAVE AREA
        RST     3       ;TEMP4=INT(X*LN2E)
        RST     3       ;DITTO FOR TEMP5
        LDA     FACC    ;GET THE EXPONENT COUNT
        MOV     B,A     ;SAVE COUNT IN B
        LDA     FACC+1  ;GET MANTISSA
ELOOP:  RLC             ;ROTATE LEFT
        DCR     B       ;REDUCE COUNT
        JNZ     ELOOP   ;CONTINUE SHIFTING
        INR     A       ;ADJUST EXPONENT
        STA     TEMP4   ;STORE EXPONENT
        MVI     A,80H   ;LOAD CONSTANT
        STA     TEMP4+1 ;STORE AS MANTISSA
        LXI     H,ONE   ;1 -> TEMP1, TEMP2
        RST     5
        LXI     H,TEMP1
        RST     3
        RST     3
        RST     5       ;LOAD TEMP3=INT(X*LN2E)
        LXI     H,TEMP5 ;GET FACC=FP(X*LN2E)
        CALL    FSUB
        LXI     H,EXPCO ;POINT CONSTANTS
        CALL    EVPS    ;COMPUTE POLYNOMIAL
        LXI     H,TEMP4 ;POINT 2^(INT(X*LN2E))
        JMP     FMUL    ;MULTIPLY,NORMALIZE AND RETURN
;
;
ABS     EQU     $
;
;
; RETURN THE ABSOLUTE VALUE OF THE FLOATING ACCUMULATOR
;
;
        LDA     FACC    ;GET EXPONENT
        ANI     7FH     ;STRIP NEGATIVE SIGN
        STA     FACC    ;REPLACE
        RET             ;RETURN
;
SGN     EQU     $
;
;
; RETURNS THE SIGN OF THE FLOATING ACCUMULATOR
; THAT IS:
;  1 IF FACC > 0
;  0 IF FACC = 0
; -1 IF FACC < 0
;
        CALL    FTEST   ;GET STATUS OF FACC
        RZ              ;RETURN IF ZERO
        ANI     80H     ;ISOLATE SIGN
SGN1:   ORI     1       ;CREATE EXPONENT
        PUSH    PSW     ;SAVE IT
        LXI     H,ONE   ;GET ADDRESS OF CONSTANT 1
        RST     5       ;GO LOAD IT
        POP     PSW     ;RESTORE SIGN
        STA     FACC    ;SET THE SIGN
        RET             ;RETURN
;
INT     EQU     $
;
;
; RETURNS THE GREATEST INTEGER NOT LARGER THAN VALUE IN FACC
; E.G.:
;    INT(3.14159) =  3
;    INT(0)       =  0
;    INT(-3.1415) = -4
;
;
        LXI     H,FACC  ;POINT FLOAT ACC
        MOV     A,M     ;GET EXPONENT
        ANI     40H     ;GET SIGN OF CHARACTERISTIC
        JZ      INT2    ;BRIF GE ZERO
        MVI     B,4     ;LOOP CTR
        JMP     ZEROM   ;GO ZERO THE FACC
INT2:   MOV     A,M     ;GET EXPONENT AGAIN
        ORA     A       ;TEST SIGN
        JP      INT3    ;BRIF POSITIVE OR ZERO
        LXI     H,NEGON ;POINT CONSTANT: -.9999999
        CALL    FADD    ;ADD TO FACC
        LXI     H,FACC  ;POINT EXPONTENT AGAIN
        MOV     A,M     ;LOAD IT
INT3:   ANI     3FH     ;ISOLATE CHARACTERISTIC
        CPI     24      ;TEST IF ANY FRACTION
        RP              ;RETURN IF NOT
        MOV     B,A     ;SAVE EXPONENT
        MVI     A,24    ;GET CONSTANT
        SUB     B       ;MINUS EXPONENT = LOOP CTR
        MOV     C,A     ;SAVE IT
INT4:   LXI     H,FACC+1        ;POINT MSB
        XRA     A       ;CLEAR CY FLAG
        MVI     B,3     ;BYTE COUNT
INT5:   MOV     A,M     ;LOAD A BYTE
        RAR             ;SHIFT RIGHT
        MOV     M,A     ;REPLACE
        INX     H       ;POINT NEXT
        DCR     B       ;DECR BYTE CTR
        JNZ     INT5    ;LOOP
        DCR     C       ;DECR BIT CTR
        JNZ     INT4    ;LOOP
        LXI     H,FACC  ;POINT SIGN & EXP
        MOV     A,M     ;LOAD IT
        ANI     80H     ;ISOLATE SIGN
        ADI     24      ;PLUS INTEGER
        MOV     M,A     ;REPLACE IT
        JMP     FNORM   ;GO NORMALIZE & RETURN
;
SQR     EQU     $
;
; COMPUTE SQAURE ROOT OF ARG IN FACC, PUT RESULT IN FACC
;
; USE HERON'S ITERATIVE PROCESS
;
        CALL    FTEST   ;TEST THE ARGUMENT
        RZ              ;RETURN IF ZERO
        JM      ZMERR   ;ERROR IF NEGATIVE
        STA     DEXP    ;SAVE ORIG EXPONENT
        XRA     A       ;GET A ZERO
        STA     FACC    ;PUT ARG IN RANGE [.5, 1]
        LXI     H,TEMP2 ;POINT SAVE AREA
        RST     3       ;STORE IT
;
; INITIAL APPROXIMATION 0.41730759 + 0.59016206 * MANTISSA
;
        LXI     H,SQC1  ;POINT .59016
        CALL    FMUL    ;GO MULTIPLY
        LXI     H,SQC2  ;PINT .4173
        CALL    FADD    ;GO ADD
        LXI     H,TEMP1 ;POINT SAVE AREA
        RST     3       ;GO STORE IT
;
; NEWTON'S METHOD OF ITERATION TO THE APPROXIMATE
; VALUE OF THE SQR OF MANTISSA
;
        CALL    SQR1    ;FIRST ITERATION
        LXI     H,TEMP1 ;POINT SAVE AREA
        RST     3       ;STORE IT
        CALL    SQR1    ;SECOND ITERATION
;
; RESTORE RANGE TO OBTAIN THE FINAL RESULT
;
        LDA     DEXP    ;GET SAVE EXPONENT
        CALL    FEXP    ;EXPAND IT
        RAR             ;DIVIDE BY 2
        STA     FACC    ;STORE IT
        RNC             ;RETURN IF EXPON EVEN
        LXI     H,SQC3  ;ELSE, POINT SQR(2)
        JMP     FMUL    ;GO MULTIPLY AND RETURN
;
; THIS ROUTINE PERFORMS ONE NEWTON ITERATION
; TO THE SQUARE ROOT FUNCTION
;
SQR1:   LXI     H,TEMP2 ;POINT MANTISSA
        RST     5       ;LOAD IT
        LXI     H,TEMP1 ;POINT PREV GUESS
        CALL    FDIV    ;FORM MANT/TEMP1
        LXI     H,TEMP1 ;POINT PREV
        CALL    FADD    ;FORM TEMP1 + MANT/TEMP1
        SUI     1       ;DIVIDE BY 2
        STA     FACC    ;FORM (TEMP1 + MANT/TEMP1)/2
        RET             ;RETURN
;
NEG     EQU     $
;
;
; REVERSES THE SIGN OF THE FLOATING ACC
;
;
        CALL    FTEST   ;GET STATUS OF FACC
        RZ              ;RETURN IF ZERO
        XRI     80H     ;REVERSE SIGN
        STA     FACC    ;RESTORE EXPONENT
        RET             ;CONTINUE EVALUATION
;
RND     EQU     $
;
;
; PSEUDO RANDOM NUMBER GENERATOR
;
;
        LXI     H,TEMP7 ;SAVE ARG
        RST     3
        MVI     B,4     ;LOOP CTR
        LXI     H,FACC  ;POINT FLOAT ACCUM
        CALL    ZEROM   ;GO ZERO THE FACC
        MVI     C,3     ;OUTTER LOP CTR
        LXI     H,FACC+1        ;POINT MSB
        PUSH    H       ;SAVE H,L
RND1:   LXI     H,RNDZ+1        ;POINT X,Y,Z
        MVI     B,6     ;LOOP CTR
        ORA     A       ;TURN OFF CY
RND2:   MOV     A,M     ;GET A BYTE
        RAL             ;SHIFT LEFT (MULT BY 2)
        MOV     M,A     ;REPLACE THE BYTE
        DCX     H       ;POINT NEXT
        DCR     B       ;DECR CTR
        JNZ     RND2    ;LOOP
        INX     H       ;POINT MSD X,Y,Z
        LXI     D,RNDP  ;POINT TO MODULO
        MVI     B,3     ;LOOP CTR
FND3:   LDAX    D       ;GET BYTE OF P,Q,R
        CMP     M       ;COMPARE WITH X,Y,Z
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        JC      RND4    ;BRIF P<X
        JNZ     RND5    ;BRIF P>X
        LDAX    D       ;GET LOW BYTE
        CMP     M       ;CMPARE
        JNC     RND5    ;BRIF P>=X
RND4:   XCHG            ;FLIP D,E TO H,L
        LDAX    D       ;GET LOW X BYTE
        SUB     M       ;SUBTRACT LOW P BYTE
        STAX    D       ;STORE IT
        DCX     D       ;POINT HIGH
        DCX     H       ;DITTO
        LDAX    D       ;GET HIGH X BYTE
        SBB     M       ;SUB HIGH P BYTE
        STAX    D       ;STORE IT
        INX     D       ;POINT LOW
        INX     H       ;DITTO
        XCHG            ;RESTORE ADDRS
RND5:   INX     D       ;POINT NEXT
        INX     H       ;DITTO
        DCR     B       ;DECR CTR
        JNZ     FND3    ;LOOP
        MVI     B,3     ;LOOP CTR
RND6:   LXI     D,RNDS+1        ;POINT LOW S
        LDAX    D       ;GET LOW S
        ADD     M       ;ADD LOW X,Y,Z
        STAX    D       ;PUT S
        DCX     D       ;POINT HIGH
        DCX     H       ;DITTO
        LDAX    D       ;GET HIGH S
        ADC     M       ;ADD HIGH X,Y,Z
        ANI     3FH     ;TURN OFF HIGH BITS
        STAX    D       ;STORE IT
        DCX     H       ;POINT NEXT X,Y,Z
        DCR     B       ;DECR CTR
        JNZ     RND6    ;LOOP
        MVI     A,8     ;CONSTANT
        SUB     C       ;LESS CTR
        RAR             ;DIVIDE BY TWO
        POP     H       ;GET H,L ADDR
        LDA     RNDS+1  ;GET LSB OF S
        MOV     M,A     ;STORE IT
        INX     H       ;POINT NEXT
        PUSH    H       ;SAVE H,L
        DCR     C       ;DECR CTR
        JNZ     RND1    ;LOOP
        POP     H       ;RESTORE SP PTR
        LDA     RNDSW   ;GET SWITCH
        ORA     A       ;TEST IT
        JZ      RND7    ;BRIF NO RANDOMIZE
        LXI     D,TRNDX ;POINT SAVED VALUES
        LXI     H,RNDX  ;POINT NEXT VALUES
        MVI     B,8     ;LOOP CTR
        CALL    COPYH   ;GO COPY
RND7:   CALL    FNORM
        LXI     H,TEMP7 ;MULTIPLY BY RANGE
        JMP     FMUL
;
INP     EQU     $
;
;
; INPUT A BYTE FROM THE DEVICE IN FACC
;
; PUT THE RESULT IN THE FACC
;
        CALL    FBIN    ;CONVERT FACC TO BINARY
        LXI     H,OUTA  ;POINT INSTR BUFFER
        MVI     M,0DBH  ;IN INSTR
        INX     H       ;POINT NEXT
        MOV     M,A     ;MOVE ADDR
        INX     H       ;POINT NEXT
        MVI     M,0C9H  ;RET INSTR
        CALL    OUTA    ;GO INPUT A BYTE
FDEC:   MOV     E,A     ;MOVE BYTE TO LO D,E
        MVI     D,0     ;ZERO HI D,E
        JMP     BINFL   ;GO CONVERT TO DEC & RET
;
POS     EQU     $
;
;
; RETURNS THE CURRENT POSITION OF THE TTY CURSOR
;
;
        LDA     COLUM   ;GET POSITION
        JMP     FDEC    ;CONVERT TO FLOAT AND RETURN
;
CONCA   EQU     $
;
;
; CONCATONATE TWO STRING TOGETHER
; COMBINE LENGTH <= 255
;
        POP     D       ;ADJUST STACK
        LXI     D,STRIN ;POINT STRING BUFFER
        LDAX    D       ;GET CURRENT LENGTH
        MOV     C,A     ;STORE IT
        MVI     B,0     ;CLEAR HI
        XCHG            ;FLIP FLOP
        DAD     B       ;COMPUTE NEXT
        XCHG            ;FLIP BACK
        ADD     M       ;COMPUTE COMBINE LENGTH
        MOV     B,M     ;SAVE LEN2
        JNC     CONC2   ;BRIF NO OVFLW
        MVI     A,255   ;MAX LEN
        SUB     C       ;MINUS 1ST PART
        MOV     B,A     ;SAVE LEN
        MVI     A,255   ;UPDATED LENGTH
CONC2:  STA     STRIN   ;STORE IT
        MOV     A,B     ;GET LEN TO MOVE
        ORA     A       ;TEST IT
        JZ      CONC4   ;BRIF NULL
CONC3:  INX     H       ;POINT NEXT
        INX     D       ;DITTO
        MOV     A,M     ;GET NEXT CHAR
        STAX    D       ;PUT IT
        DCR     B       ;DECR COUNT
        JNZ     CONC3   ;LOOP
CONC4:  POP     H       ;GET H,L
        DCX     H       ;POINT BACK
        LDA     STRIN   ;GET LEN
        RAR             ;DIVIDE BY TWO
        INR     A       ;PLUS ONE
        XCHG            ;SAVE H,L
        LHLD    SPCTR   ;GET CTR
        MOV     C,A     ;SAVE CTR
        MVI     B,0     ;ZERO HI BYTE
        DAD     B       ;ADD LEN THIS STRING
        SHLD    SPCTR   ;SAVE CTR
        POP     B
        LXI     H,0     ;GET ADDR ZERO
CONC5:  PUSH    H       ;2 BYTE WORD
        DCR     A       ;DECR CTR
        JNZ     CONC5   ;CONTINUE
        DAD     SP      ;GET ADDRESS IN H,L
        XCHG            ;PUT STACK PTR IN D,E
        MOV     M,D     ;MOVE HI ADDR
        INX     H       ;POINT NEXT
        MOV     M,E     ;MOVE LO ADDR
        INX     H       ;POINT NEXT
        MVI     M,0E7H  ;TYPE=STRING
        PUSH    H       ;SAVE H,L
        LXI     H,STRIN ;GET TEMP STR
        MOV     A,M     ;GET LENGTH
        INR     A       ;PLUS ONE
        MOV     C,A     ;SAVE IT
CONC6:  MOV     A,M     ;GET A BYTE
        STAX    D       ;PUT IT DOWN
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        DCR     C       ;SUBT CTR
        JNZ     CONC6   ;LOOP
        POP     H       ;RESTORE H,L
        RST     4       ;ADJUST H,L
        DB      -7 AND 0FFH
        MVI     A,4     ;DELETE 4 BYTES
        CALL    SQUIS   ;GO COMPRESS
        JMP     EVAL    ;CONTINUE EVALUATION
;
LENFN   EQU     $
;
; X=LEN(A$)
;
; RETURN THE LENGTH OF THE STRING
;
        LDA     STRIN   ;GET LEN IN ACC
        JMP     FDEC    ;GO CONVERT TO DECIMAL & RETURN
;
CHRFN   EQU     $
;
; A$=CHR$(X)
;
; RETURNS A ONE CHAR STRING HAVING THE ASCII VALUE - X
;
        CALL    FBIN    ;CONVERT FACC TO BINARY
        LXI     H,STRIN ;POINT OUT AREA
        MVI     M,1     ;LEN=1
        INX     H       ;POINT NEXT
        MOV     M,A     ;STORE THE CHAR
        RET             ;RETURN
;
ASCII   EQU     $
;
; X=ASCII(A$)
;
; RETURNS THE ASCII VALUE OF THE FIRST CHAR IN STRING
;
        LXI     H,STRIN ;POINT STRING
        MOV     A,M     ;GET LENGTH
        ORA     A       ;TEST IF > ZERO
        JZ      FDEC    ;BRIF ZERO & RETURN A ZERO
        INX     H       ;POINT 1ST CHAR
        MOV     A,M     ;LOAD IT
        JMP     FDEC    ;GO CONVERT TO DECIMAL & RETURN
;
NUMFN   EQU     $
;
; A$=NUM$(X)
;
; RETURNS A STRING REPRESENTING X AS IT WOULD HAVE
; BEEN PRINTED (INCLUDING TRAILING SPACE)
;
        LXI     H,STRIN ;POINT STRING AREA
        MVI     M,0     ;INIT COUNT
        INX     H       ;SKIP TO 1ST POSITION
        CALL    FOUT    ;GO CONVERT TO EXTRN DEC
        XRA     A       ;GET A ZERO
        MOV     B,A     ;INIT CTR
NUM1:   DCX     H       ;POINT PRIOR
        INR     B       ;COUNT IT
        CMP     M       ;TEST IF ZERO
        JNZ     NUM1    ;LOOP TILL AT START
        MOV     M,B     ;SET LEN CODE
        RET             ;THEN RETURN
;
VAL     EQU     $
;
; X = VAL(A$)
;
; RETURNS THE VALUE OF THE STRING OF NUMERIC CHARACTERS
;
        LXI     H,STRIN ;POINT STRING AREA
        MOV     A,M     ;GET LEN
        ORA     A       ;TEST FOR NULL STRING
        MOV     B,A     ;SAVE LEN
        JZ      FDEC    ;BRIF IS (RETURNS A 0.00)
        LXI     D,STRIN ;POINT BUFFER
VAL1:   INX     H       ;POINT NEXT
        MOV     A,M     ;GET A CHAR
        CPI     ' '     ;TEST IF SPACE
        JZ      VAL2    ;BRIF IS
        STAX    D       ;PUT THE CHAR
        INX     D       ;INCR ADDR
VAL2:   DCR     B       ;DECR CTR
        JNZ     VAL1    ;LOOP
        XRA     A       ;GET A ZERO
        STAX    D       ;PUT IN BUFF
        LXI     H,STRIN ;POINT START OF BUFFER
        CALL    FIN     ;GO CONVERT
        MOV     A,M     ;GET NON-NUMERIC
        ORA     A       ;TEST IT
        JNZ     CVERR   ;BRIF ERROR
        RET             ;ELSE, RETURN
;
SPACE   EQU     $
;
; A$=SPACE$(X)
;
; CREATES A STRING FO SPACES LENGTH = X
;
        CALL    FBIN    ;GET BINARY LENGTH
        LXI     H,STRIN ;POINT TEMP STRING
        MOV     M,A     ;PUT LEN
        ORA     A       ;TEST IT
SPAC1:  RZ              ;RETURN IF ZERO
        INX     H       ;ELSE, POINT NEXT
        MVI     M,' '   ;MOVE 1 SPACE
        DCR     A       ;DECR CTR
        JMP     SPAC1   ;LOOP
;
STRFN   EQU     $
;
; A$=STRING$(X,Y)
;
; CREATES STRING OF LNGTH X CONTAINING REPETITION OF CHR$(Y)
;
        CALL    FBIN    ;GET BINARY LENGTH
        STA     STRIN   ;PUT TO STRING
        CALL    ARGNU   ;GET NEXT ARGUMENT
        LXI     H,STRIN ;POINT STRING
        MOV     B,M     ;GET COUNT
STR11:  INX     H       ;POINT NEXT
        MOV     M,A     ;STORE THE CHAR
        DCR     B       ;DECR CTR
        JNZ     STR11   ;LOOP
        RET             ;RETURN
;
LEFT    EQU     $
;
; B$=LEFT$(A$,X)
;
; SUBSTRING FROM THE LEFTMOST X CHARACTERS OF A$
;
        CALL    ARGNU   ;GET 2ND ARGUMENT
        MOV     C,A     ;SAVE LEN
        MVI     B,1     ;INIT START
        JMP     MID0    ;CONTINUE
;
RIGHT   EQU     $
;
; B$=RIGHT$(A$,X)
;
; SUBSTRING STARTING AT POSITION X TO END OF STRING
;
        CALL    ARGNU   ;GET 2ND ARGUMENT
        MOV     B,A     ;SAVE START
        MVI     C,255   ;MAX LEN
        JMP     MID0    ;CONTINUE
;
MIDFN   EQU     $
;
; B$=MID$(A$,X,Y)
;
; SUBSTRING OF THE STRING A$ STARTING WITH CHARACTER @ X
; AND Y CHARACTERS LONG
;
        CALL    ARGNU   ;LOAD X
        MOV     B,A     ;SAVE START
        PUSH    B       ;PUT ON STACK
        CALL    ARGNU   ;GET 3RD ARG
        POP     B       ;RETREIVE
        MOV     C,A     ;SAVE LEN
MID0:   MOV     A,B     ;LOAD START
        LXI     H,STRIN ;POINT STRING
        CMP     M       ;TEST IF X>L
        JC      MID1    ;BRIF X>L
        JZ      MID1    ;OR EQUAL
        MVI     M,0     ;ELSE, RESULT IS NULL
        RET             ;RETURN
MID1:   ADD     C       ;COMPUTE END POSITION
        JC      MID2    ;BRIF OVERFLOW
        SBI     1       ;COMPUTE X+Y-1
        JC      MID2    ;BRIF OVERFLOW
        CMP     M       ;COMPARE TO EXISTING LEN
        JC      MID3    ;BRIF X+Y-1<LEN(A$)
MID2:   MOV     A,M     ;ELSE GET ORIG LEN
        SUB     B       ;MINUS X
        INR     A       ;PLUS ONE
        MOV     C,A     ;SAVE (REPLACE Y)
MID3:   MOV     M,C     ;PUT NEW LEN
        MOV     E,B     ;PUT START IN LO
        MVI     D,0     ;ZERO IN HI
        DAD     D       ;COMPUTE START
        LXI     D,STRIN ;GET BEGIN
MID4:   MOV     A,M     ;GET A CHAR
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        STAX    D       ;PUT DOWN
        DCR     C       ;DECR CTR
        JNZ     MID4    ;LOOP
        RET             ;THEN RETURN
;
INSTR   EQU     $
;
; X = INSTR(Y,A$,B$)
;
; SEARCH FOR SUBSTRING B$ IN STRING A$ STARTING AT POS Y.
; RETURN 0 IF B$ IS NOT IN A$
; RETURN 1 IF B$ IS NULL
; ELSE RETURN THE CHARACTER POSITION
;
        CALL    ARGNU   ;GET A$
        LXI     H,STRIN ;POINT A$
        ORA     A       ;TEST Y
        JNZ     INST2   ;BRIF Y NOT ZERO
INST1:  MVI     M,0     ;ELSE A$ IS NULL
        JMP     INST3   ;GO AROUND
INST2:  CMP     M       ;TEST Y TO LEN(A$)
        JZ      INST3   ;BRIF EQUAL
        JNC     INST1   ;BRIF Y > LEN(A$)
INST3:  MOV     C,A     ;SAVE Y
        MVI     B,0     ;ZERO HI INCR
        MOV     A,M     ;GET LEN(A$)
        SUB     C       ;MINUS Y
        INR     A       ;PLUS ONE
        DAD     B       ;COMPUTE START ADDR
        MOV     B,A     ;# CHARS REMAIN IN A$
        PUSH    H       ;SAVE ADDR
        LHLD    ADDR1   ;GET ADDR OF ARG
        INX     H       ;POINT NEXT
        MOV     D,M     ;GET HI ADDR
        INX     H       ;POINT NEXT
        MOV     E,M     ;GET LO ADDR
        INX     H       ;POINT NEXT
        SHLD    ADDR1   ;UPDATED PTR
        POP     H       ;RESTORE ADDR
        LDAX    D       ;GET LEN(B$)
        ORA     A       ;TEST IF NULL
        JNZ     INST6   ;BRIF NOT
        MVI     C,1     ;SET POSIT = 1
INST5:  MOV     A,C     ;GET POSIT
        JMP     FDEC    ;CONVERT TO DECIMAL & RETURN
INST6:  XCHG            ;FLIP/FLOP
        MOV     A,B     ;GET LEN OF A$
        CMP     M       ;COMPARE TO LEN B$
        JC      INSTA   ;BRIF LEN(B$)< LEN(REM A$)
        PUSH    B       ;SAVE CTR, POSIT
        PUSH    D       ;SAVE ADDR A$
        PUSH    H       ;SAVE ADDR B$
        MOV     C,M     ;GET LEN B$
        XCHG            ;FLIP/FLOP
INST8:  INX     D       ;POINT NEXT B$
        LDAX    D       ;GET B$ CHAR
        CMP     M       ;COMPARE A$ CHAR
        JNZ     INST9   ;BRIF NOT EQUAL
        INX     H       ;POINT NEXT A$
        DCR     C       ;DECR CTR (LEN(B$))
        JNZ     INST8   ;LOOP
        POP     H       ;DUMMY POP
        POP     H       ;GET DUMMY STACK
        POP     B       ;GET POSITION
        JMP     INST5   ;WE FOUND A MATCH
INST9:  POP     D       ;GET PTR B$
        POP     H       ;GET PTR A$
        POP     B       ;GET CTRS, POSIT
        INR     C       ;UP PTR NUM
        INX     H       ;POINT NEXT A$
        DCR     B       ;DECR B
        JNZ     INST6   ;LOOP
INSTA:  MVI     C,0     ;ELSE B$ NOT IN A$
        JMP     INST5   ;RETURN
;
FN      EQU     $
;
; STMT: DEF FNX(A)=EXPR
;
; NOTE: ENTRY FROM EXPR ANALYZER (RECURSIVE)
;
        PUSH    B       ;SAVE B,C
        PUSH    D       ;SAVE D,E
        PUSH    H       ;SAVE H,L
        XCHG            ;PUT H,L TO D,E
        LHLD    ADDR3   ;GET ADDR
        PUSH    H       ;SAVE IT
        XCHG            ;PUT D,E BACK TO H,L
        SHLD    ADDR3   ;UPDATE PTR
        LHLD    SPCTR   ;GET SP COUNT
        PUSH    H       ;SAVE IT
        LDA     PARCT   ;GET PAREN COUNT
        MOV     B,A     ;PUT TO B
        LDA     FNMOD   ;GET FN MODE
        MOV     C,A     ;PUT TO C
        PUSH    B       ;SAVE B,C
        LDA     DIMSW   ;GET DIM SW
        PUSH    PSW     ;SAVE IT
        XRA     A       ;CLEAR A
        STA     DIMSW   ;RESET DIM SW
        LHLD    FNARG   ;GET OLD ARG NAME
        PUSH    H       ;SAVE
        LHLD    FNARG+2 ;GET OLD ARG ADDRESS
        PUSH    H       ;SAVE
        LHLD    PROGE   ;GET END OF PROGRAM
        PUSH    H       ;SAVE IT
        LHLD    EXPRS   ;GET END OF EXPR
        PUSH    H       ;SAVE IT
        SHLD    PROGE   ;SAVE NEW 'END' OF PROGRAM
        MVI     A,1     ;GET ON SETTING
        STA     FNMOD   ;SET IN FUNCTION
        LHLD    ADDR3   ;POINT TO EXPR
        MOV     C,M     ;GET FN CHAR
        DCX     H       ;POINT BACK
        MOV     B,M     ;GET HI NAME
        LXI     H,BEGPR ;POINT START OF PROGRAM
FN2:    MOV     A,M     ;LOAD LEN TO NEXT STMT
        ORA     A       ;TEST IF AT END
        JZ      SNERR   ;BRIF FN NOT FOUND
        PUSH    H       ;SAVE PTR
        RST     4       ;ADJUST H,L
        DB      3
        LXI     D,DEFLI ;LITERAL
        RST     2       ;GO COMPARE
        JNZ     FN3     ;BRIF NOT EQUAL
        PUSH    B       ;SAVE TEST NAME
        CALL    VAR     ;GO GET NAME
        POP     B       ;RESTORE NAME
        MOV     A,D     ;GET HI NAME
        CMP     B       ;COMPARE
        JNZ     FN3     ;BRIF NOT EQUAL
        MOV     A,E     ;GET LO
        CMP     C       ;COMPARE
        JZ      FN4     ;BRIF EQUAL
FN3:    POP     H       ;GET OLD PTR
        MOV     E,M     ;GET LO LEN
        MVI     D,0     ;ZERO HI LEN
        DAD     D       ;POINT NEXT STMT
        JMP     FN2     ;LOOP
FN4:    POP     D       ;ADJUST STACK
        RST     1       ;SKIP BLANKS
        CPI     '('     ;TEST IF OPEN PAREN
        JNZ     SNERR   ;BRIF NOT
        INX     H       ;SKIP IT
        CALL    VAR     ;GO GET VAR NAME
        PUSH    H       ;SAVE HL ADDR
        LXI     H,FNARG ;POINT DUMMY ARG TBL
        MOV     M,D     ;STORE LETTER
        INX     H       ;POINT NEXT
        MOV     M,E     ;STORE DIGIT
        INX     H       ;POINT NEXT
        XCHG            ;PUT H,L TO D,E
        LHLD    ADDR3   ;POINT TO EXPR STACK
        INX     H       ;POINT CODE
        INX     H       ;POINT HI ADR
        MOV     A,M     ;GET HI
        STAX    D       ;PUT TO TABLE
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        MOV     A,M     ;GET LO ADDR
        STAX    D       ;PUT TO TABLE
        POP     H       ;RESTORE PTR TO STMT
        RST     1       ;SKIP BLANKS
        CPI     ')'     ;TEST IF CLOSE PAREN
        JNZ     SNERR   ;BRIF NOT
        INX     H       ;SKIP IT
        RST     1       ;SKIP BLANKS
        CPI     '='     ;TEST IF EQUAL SIGN
        JNZ     SNERR   ;BRIF NOT
        INX     H       ;SKIP IT
        CALL    EXPR    ;GO EVAL FUNCTION
        CALL    EOL     ;MUST BE END OF LINE
        POP     H       ;GET H,L
        SHLD    EXPRS   ;RESTORE START OF EXPR
        POP     H       ;GET H,L
        SHLD    PROGE   ;RESTORE 'END' OF PROGRAM
        POP     H       ;GET H,L
        SHLD    FNARG+2 ;STORE ADDR
        POP     H       ;GET H,L
        SHLD    FNARG   ;STORE DUMMY ARG
        POP     PSW     ;GET A,STATUS
        STA     DIMSW   ;RESTORE DIM SW
        POP     B       ;GET B,C
        MOV     A,C     ;LOAD C
        STA     FNMOD   ;RESTORE MOE
        MOV     A,B     ;LOAD B
        STA     PARCT   ;RESTORE PAREN COUNT
        POP     H       ;GET H,L
        SHLD    SPCTR   ;RESTORE SP COUNTER
        POP     H       ;GET H,L
        SHLD    ADDR3   ;RESTORE ADDR OF EVAL
        POP     H       ;GET H,L
        POP     D       ;GET D,E
        DCX     H       ;POINT 2ND BYTE FOLLOWING OP
        SHLD    ADDR2   ;SAVE IT
        RST     4       ;POINT TO ARG TYPE
        DB      5
        SHLD    ADDR1   ;SAVE ADDR
        JMP     EV3     ;GO WRAPUP
;PAGE
;
EXPR    EQU     $
;
;
; EVALUATE EXPRESSION ROUTINE
; LEAVE RESULT IN FACC
; RETURN WHEN EXPRESSION ENDS (TYPICALLY AT END OF LINE)
;
;
        XRA     A       ;CLEAR REG A
        STA     PARCT   ;SET PAREN CTR
        XCHG            ;SAVE H,L
        LXI     H,0     ;GET A ZERO
        SHLD    SPCTR   ;INIT CTR
        LHLD    PROGE   ;POINT END OF PROGRAM AREA
        INX     H       ;POINT ONE MORE
        MVI     M,0     ;INIT START OF STACK
        SHLD    EXPRS   ;SAVE IT
        XCHG            ;RESTORE H,L
;
LOOKD   EQU     $       ;LOOK FOR CON, VAR, OR FUNCTION
        RST     1       ;SKIP TO NON-BLANK
        CALL    NUMER   ;GO TEST IF NUMERIC
        JNZ     LDALP   ;BRIF NOT
LDNUM:  CALL    FIN     ;GO CONVERT NUMERIC (PUT TO FACC)
LDF:    MOV     B,H     ;COPY H,L TO B,C
        MOV     C,L     ;SAME
        LHLD    EXPRS   ;GET ADDR OF EXPR AREA
        CALL    GTEMP   ;GO STORE THE FACC IN TEMP AREA
        SHLD    EXPRS   ;SAVE UPDATED ADDRESS
        MOV     H,B     ;RESTORE H
        MOV     L,C     ;RESTORE L
        JMP     LOOKO   ;GO GET AN OPERATION CODE
LDALP:  CPI     '.'     ;SEE IF LEADING DECIMAL POINT
        JZ      LDNUM   ;BRIF IS
        CALL    ALPHA   ;GO SEE IF ALPHA
        JNZ     LDDTN   ;BRIF NOT
        MOV     B,M     ;SAVE 1ST CHAR
        INX     H       ;POINT NEXT
        MVI     C,' '   ;DEFAULT FOR 1 CHAR VAR
        CALL    NUMER   ;GO SEE IF 2ND IS NUMERIC
        JNZ     LDFN    ;BRIF NOT
        INX     H       ;POINT NEXT
        MOV     C,A     ;SAVE THE CHAR
LDV1:   RST     1       ;GET NEXT CHAR
        CPI     '$'     ;TEST IF STRING
        PUSH    PSW     ;SAVE STATUS
        JNZ     LDV2    ;BRIF NOT
        MOV     A,C     ;GET LOW CHAR
        ORI     80H     ;SET STRING
        MOV     C,A     ;SAVE IT
        INX     H       ;SKIP $
        RST     1       ;SKIP SPACES
LDV2:   CPI     '('     ;TEST IF PAREN
        JZ      LDV2A   ;BRIF IS
        PUSH    H       ;SAVE H,L
        MOV     D,B     ;COPY B,C
        MOV     E,C     ;TO D,E
        CALL    SEARC   ;GO GET VAR ADDR IN D,E
LDV:    LHLD    EXPRS   ;GET EXPR ADDR
        CALL    SADR    ;GO STORE ADDRESS
        SHLD    EXPRS   ;SAVE ADDRESS
        XCHG            ;H,L TO D,E
        POP     H       ;GET OLD H,L
        POP     PSW     ;GET STATUS
        JNZ     LOOKO   ;BRIF NOT STRING
        XCHG            ;GET OLD H,L
        MVI     M,0E7H  ;MARK AS STRING ADDRESS
        XCHG            ;RESTORE H,L
        JMP     LOOKO   ;GO LOOK FOR OPCODE
LDFN:   CALL    ALPHA   ;GO SEE IF FUNCTION
        JNZ     LDV1    ;BRIF IT'S NOT
LDFN1:  DCX     H       ;POINT BACK TO 1ST
        MOV     A,M     ;GET THAT CHAR
        CPI     ' '     ;TEST IF SPACE
        JZ      LDFN1   ;LOOP IF TRUE
        PUSH    H       ;SAVE H,L
        LXI     D,RNDLI ;POINT LITERAL
        RST     2       ;GO COMPARE
        JZ      LDRND   ;BRIF FND
        POP     H       ;GET H,L
        PUSH    H       ;RESAVE
        LXI     D,FNLIT ;POINT LITERAL
        RST     2       ;GO SEE IF FN X
        JZ      FNL     ;BRIF IS
        POP     H       ;GET H,L
        PUSH    H       ;RESAVE
        LXI     D,PILIT ;POINT LIT
        RST     2       ;GO COMPARE
        JZ      LDPI    ;BRIF PI
FUNC0:  POP     H       ;GET H,L
        LXI     D,FUNCT ;POINT FUNCTION TABLE
        PUSH    H       ;SAVE POINTER
        CALL    SEEK1   ;GO SEARCH FUNCTION TABLE
        JZ      FUNC4   ;BRIF FUNCTION NOT FOUND
        LDAX    D       ;GET A BYTE LOW
        MOV     C,A     ;SAVE IT
        INX     D       ;POINT NEXT
        LDAX    D       ;GET HI BYTE
        MOV     B,A     ;SAVE IT (B,C = ADDR OF FUNC)
        RST     1       ;SKIP BLANKS
        CPI     '('     ;TEST FOR OPEN PAREN
        JNZ     SNERR   ;BRIF MISSING PAREN
        INX     D       ;POINT TYPE CODE
        LDAX    D       ;LOAD IT
        JMP     LDFNC   ;CONTINUE
FUNC4:  POP     H       ;GET H,L
        MOV     B,M     ;GET 1ST CHAR
        MVI     C,' '   ;SPACE 2ND CHAR
        INX     H       ;POINT TO NEXT
        JMP     LDV1    ;BRIF VARIABLE
FNL:    POP     D       ;DUMMY RESET STACK POINTER
        CALL    VAR     ;GO GET FN NAME
        MOV     B,D     ;COPY TO B,C
        MOV     C,E     ;SAME
        XCHG            ;SAVE H,L
        LHLD    EXPRS   ;POINT EXPR STACK
        INX     H       ;POINT NEXT
        MOV     M,B     ;MOVE THE LETTER
        INX     H       ;POINT NEXT
        MOV     M,C     ;MOVE DIGIT ($??)
        INX     H       ;POINT NEXT
        MVI     M,0AFH  ;MOVE CODE
        MOV     A,C     ;GET LO NAME
        ORA     A       ;TEST IT
        JP      FNL3    ;BRIF NOT STRING
        MVI     M,0CFH  ;MOVE CODE
FNL3:   SHLD    EXPRS   ;SAVE POINTER
        XCHG            ;GET H,L
        RST     1       ;GET NEXT CHAR
        CPI     '('     ;TEST IF OPEN PAREN
        JNZ     SNERR   ;BRIF NOT
        JMP     LOOKD   ;CONTINUE
LDRND:  CPI     '('     ;TEST IF RND(X)
        JZ      FUNC0   ;BRIF IS
        PUSH    H       ;ELSE, SAVE H,L
        LXI     H,ONE   ;USE RANGE (0,1)
        RST     5       ;LOAD FACC
        CALL    RND     ;GO GET RANDOM NUMBER
        POP     H       ;RESTORE H,L
        POP     D       ;RESTORE STACK POINTER
        JMP     LDF     ;ACT AS IF CONSTANT
LDPI:   INR     A       ;SET NON ZERO
        POP     D       ;DUMMY STACK POP
        PUSH    PSW     ;SAVE STATUS
        PUSH    H       ;SAVE H,L
        LXI     D,PI    ;GET ADDRESS OF 3.1415
        JMP     LDV     ;GO ACT LIKE VARIABLE
LDFNC:  POP     D       ;POP THE STACK
        XCHG            ;FLIP/FLOP
        LHLD    EXPRS   ;GET ADDR
        INX     H       ;POINT NEXT
        MOV     M,B     ;HIGH ADDR
        INX     H       ;POINT NEXT
        MOV     M,C     ;LOW ADDR
        INX     H       ;POINT NEXT
        MOV     M,A     ;CODE
        SHLD    EXPRS   ;SAVE ADDR
        XCHG            ;RESTORE H,L
        JMP     LOOKD   ;NEXT MUST BE DATA TOO
LDDTN:  CPI     '-'     ;TEST IF UNARY MINUS
        JNZ     LDDTP   ;BRIF NOT
        XCHG            ;SAVE H,L
        LHLD    EXPRS   ;GET EXPR END
        INX     H       ;POINT ONE MORE
        MVI     M,61H   ;CODE FOR NEG
        SHLD    EXPRS   ;RESTORE PTR
        XCHG            ;RESTORE H,L
SKPP:   INX     H       ;POINT PAST THIS BYTE
        JMP     LOOKD   ;NEXT MUST BE DATA
LDDTP:  CPI     '+'     ;TEST IF UNARY PLUS
        JZ      SKPP    ;IGNORE IF IS
        CPI     '('     ;ELSE, TEST IF OPEN PAREN
        JZ      CERCE   ;BRIF IS
        CPI     27H     ;TEST IF LITERAL (SINGLE QUOTE)
        JZ      LITST   ;BRIF IS
        CPI     '"'     ;TEST IF LITERAL
        JNZ     SNERR   ;BRIF NOT CON, FUNCTION, OR VAR
LITST:  MOV     C,A     ;SAVE DELIMITER
        LXI     D,STRIN ;POINT BUFFER
        MVI     B,0FFH  ;INIT CTR
LIT1:   INX     H       ;POINT NEXT
        MOV     A,M     ;LOAD NEXT
        INX     D       ;POINT NEXT
        STAX    D       ;STORE IT
        ORA     A       ;TEST IF END
        JZ      SNERR   ;BRIF ERROR
        INR     B       ;COUNT IT
        CMP     C       ;TEST IF END OF STRING
        JNZ     LIT1    ;BRIF NOT
        INX     H       ;POINT NEXT
        LXI     D,STRIN ;POINT BEGIN
        MOV     A,B     ;GET COUNT
        STAX    D       ;PUT COUNT
        RAR             ;DIVIDE BY TWO
        INR     A       ;PLUS ONE
        MOV     C,A     ;SAVE IT
        MVI     B,0     ;ZERO HIGH
        PUSH    H       ;SAVE PTR
        LHLD    SPCTR   ;GET CTR
        DAD     B       ;PLUS OLD
        SHLD    SPCTR   ;UPDATE IT
        POP     D       ;GET OLD H,L
        LXI     H,0     ;GET A ZERO
LIT2:   PUSH    H       ;GET 2 WORK BYTES
        DCR     C       ;SUB 1 FROM COUNT
        JNZ     LIT2    ;CONTINUE
        DAD     SP      ;GET ADDR OF STACK
        PUSH    D       ;SAVE PTR TO STMT
        XCHG            ;SAVE H,L IN D,E
        LHLD    EXPRS   ;GET START OF EXPR
        INX     H       ;PLUS ONE
        MOV     M,D     ;HI BYTE
        INX     H       ;POINT NEXT
        MOV     M,E     ;LO BYTE
        INX     H       ;POINT NEXT
        MVI     M,0E7H  ;TYPE CODE
        SHLD    EXPRS   ;SAVE ADDR
        XCHG            ;D,E BACK TO H,L
        LXI     D,STRIN ;POINT STRING AREA
        LDAX    D       ;GET COUNT
        INR     A       ;ADD ONE TO COUNT
        MOV     B,A     ;SAVE CTR
LIT3:   LDAX    D       ;GET A BYTE
        MOV     M,A     ;STORE IT
        INX     H       ;POINT NEXT
        INX     D       ;DITTO
        DCR     B       ;DECR CTR
        JNZ     LIT3    ;LOOP
        POP     H       ;RESTORE H,L
        JMP     LOOKO   ;NEXT IS OP
CERCE:  XCHG            ;SAVE H,L
        LXI     H,PARCT ;POINT PAREN COUNT
        INR     M       ;ADD 1
        LHLD    EXPRS   ;GET ADDR
        INX     H       ;POINT NEXT
        MVI     M,5     ;PUT CODE
        SHLD    EXPRS   ;SAVE ADDR
        XCHG            ;RESTORE H,L
        JMP     SKPP    ;GO SKIP CHAR
LOOKO:  RST     1       ;SKIP BLANKS
        CPI     '+'     ;TEST IF PLUS
        MVI     B,21H   ;CODE
        JZ      OP1     ;BRIF IS
        CPI     '-'     ;TEST IF MINUS
        MVI     B,25H
        JZ      OP1     ;BRIF IS
        CPI     '/'     ;TEST IF DIVIDE
        MVI     B,45H   ;CODE
        JZ      OP1     ;BRIF IS
        CPI     '^'     ;TEST IF EXPON
        MVI     B,81H   ;CODE
        JZ      OP1     ;BRIF IS
        CPI     ')'     ;TEST IF CLOSE PAREN
        JZ      OP3     ;BRIF IS
        CPI     ','     ;TEST IF COMMA
        JZ      OP2     ;BRIF IS
        CPI     '*'     ;TEST IF MULTIPLY
        MVI     B,41H   ;CODE
        JZ      OP1     ;BRIF IS
; ELSE MUST BE END OF EXPRESSION
ENDXP:  LDA     PARCT   ;GET OPEN PAREN COUNT
        ORA     A       ;TEST IT
        JNZ     SNERR   ;BRIF # OF ('S NOT = # OF )'S
        SHLD    ADDR3   ;SAVE ADDR OF STMT
        JMP     EVAL    ;GO EVALUATE
OP1:    PUSH    H       ;SAVE PLACE IN ASCII EXPRESSION
        LXI     D,0105H ;D=BYTE COUNT, E=CODE FOR "("
        LHLD    EXPRS   ;POINT TO LAST BYTE
        MOV     A,B     ;B&E3 -> C
        ANI     0E3H
        MOV     C,A
; INSERT ( AND EVALUATE IF PRECEDENCE REDUCTION,
;   ELSE INNSERT OP CODE
OPLP1:  MOV     A,M     ;GET TYPE CODE FROM EXPRESSION
        PUSH    PSW     ;SAVE
        ANI     3       ;GET LENGTH
OPLP2:  INR     D       ;BUMP BYTE COUNT
        DCX     H       ;EXPRESSION POINTER
        DCR     A       ;LOOP MOVES TO NEXT ELEMENT
        JNZ     OPLP2
        POP     PSW     ;RESTORE TYPE CODE
        ANI     0E3H    ;MASK FOR VARIABLE
        CPI     0E3H    ;WE SKIP OVER VARIABLES
        JZ      OPLP1   ;BR IF TYPE = E3 OR E7
        CMP     C       ;PRECEDENCE REDUCTION?
        JNC     INS     ;IF NC, YES, INSERT 05
        LHLD    EXPRS   ;NO, INSERT OPCODE BEFORE VAR AT END
        RST     4       ;SKIP OVER VARIABLE
        DB      -3 AND 0FFH
        MVI     D,4     ;BYTE COUNT
        MOV     E,B     ;INSERT THIS OP CODE
INS:    MOV     B,E     ;SAVE FOR BRANCH AFTER INSERTION
INS1:   INX     H       ;BUMP POINTER
        MOV     C,M     ;PICK UP BYTE
        MOV     M,B     ;PUT DOWN REPLACEMENT
        MOV     B,C     ;SAVE FOR NEXT LOOP
        DCR     D       ;DONE?
        JNZ     INS1    ;IF NZ, NO
        SHLD    EXPRS   ;STORE POINTER
        POP     H       ;RESTORE ASCII EXPRESSION POINTER
        MOV     A,E     ;GET FLAG SAVED IN E
        CPI     5       ;STORED A "("?
        JNZ     SKPP    ;IF NZ, NO, PROCESS NEXT ELEMENT
        JMP     OP4     ;YES, GO EVALUATE
OP2:    LDA     PARCT   ;GET OPEN PAREN COUNT
        ORA     A       ;TEST IT
        JZ      ENDXP   ;BRIF END OF EXPR
        XCHG            ;ELSE SAVE H,L
        LHLD    EXPRS   ;GET EXPR BEGIN
        INX     H       ;POINT NEXT
        MVI     M,1     ;MOVE A COMMA
        SHLD    EXPRS   ;UPDATE POINTER
        XCHG            ;FLIP BACK
        JMP     SKPP
OP3:    LDA     PARCT   ;GET OPEN PAREN COUNT
        DCR     A       ;SUBTRACT ONE
        STA     PARCT   ;SAVE IT
        JM      SNERR   ;BRIF TOO MANY )'S
        INX     H       ;POINT NEXT SOURCE
OP4:    SHLD    ADDR3   ;SAVE ADDR
EVAL:   LHLD    EXPRS   ;GET END OF EXPR
        LXI     B,0     ;INIT B,C TO ZERO
EV1:    INR     B       ;COUNT EACH BYTE
        MOV     A,M     ;GET CODE IN REG A
        DCX     H       ;POINT NEXT
        CPI     0E3H    ;TEST IF DATA
        JNZ     EV2     ;BRIF NOT DATA
EV1A:   DCX     H       ;POINT NEXT
        DCX     H       ;DITTO
        INR     B       ;BUMP CTR
        INR     B       ;BY TWO
        INR     C       ;COUNT THE TERM
        JMP     EV1     ;LOOP
EV2:    CPI     0AFH    ;TEST IF NUMERIC USER FN
        JZ      FN      ;BRIF IS
        CPI     0CFH    ;TEST IF STRING USER FN
        JZ      FN      ;BRIF IS
        PUSH    PSW     ;ELSE, SAVE STATUS
        ANI     0E3H    ;MASK IT
        CPI     0A3H    ;TEST IF NUMERIC FUNCTION
        JZ      EV2A    ;BRIF IS
        CPI     0C3H    ;TEST IF STRING FUNCTION
        JZ      EV2A    ;BRIF IS
        POP     PSW     ;RESTORE CODE
        CPI     0E7H    ;TEST IF STRING ADDR
        JZ      EV1A    ;BRIF IS
        JMP     EV5     ;BR AROUND
EV2A:   INX     H       ;RESET TO TYPE CODE
        SHLD    ADDR1   ;SAVE ADDR
        POP     D       ;DUMMY POP
        PUSH    B       ;SAVE CTRS
        DCX     H       ;POINT TO LOW JMP ADDR
        MOV     E,M     ;LOW BYTE
        DCX     H       ;POINT BACK
        MOV     D,M     ;HIGH BACK
        SHLD    ADDR2   ;SAVE LOCATION
        LXI     H,EV3   ;GET RETURN ADDRESS
        PUSH    H       ;SAVE ON STACK
        PUSH    D       ;SAVE ADDRESS
        CALL    ARG     ;GO GET 1ST ARG
        POP     H       ;GET H,L ADDRESS
        PCHL            ;GO EXECUTE THE FUNCTION
EV3     EQU     $       ;FUNCTIONS RETURN HERE
        LHLD    ADDR2   ;GET ADDR FUNC
        INX     H       ;POINT LO
        INX     H       ;POINT TYPE
        MOV     A,M     ;LOAD IT
        ANI     0E0H    ;MASK IT
        CPI     0C0H    ;TEST IF STRING
        JZ      EV4     ;BRIF IS
        POP     B       ;GET CTRS
        LHLD    SPCTR   ;GET COUNTER
        INX     H       ;PLUS
        INX     H       ;TWO WORDS
        SHLD    SPCTR   ;STORE IT
        LXI     H,0     ;LOAD ZERO TO H,L
        PUSH    H       ;GET BLOCK OF
        PUSH    H       ;BYTES
        DAD     SP      ;GET STACK ADDR
        PUSH    B       ;SAVE CTRS
        PUSH    H       ;SAVE ADDR
        RST     3       ;GO STORE THE VARIABLE
        MVI     A,0E3H  ;TYPE=NUM
EV3A:   POP     D       ;GET ADDR IN STACK
        LHLD    ADDR1   ;GET ADDR LST ARG
        MOV     M,A     ;STORE TYPE CODE
        DCX     H       ;POINT ONE BACK
        MOV     M,E     ;STORE LO ADDR
        DCX     H       ;POINT BACK
        MOV     M,D     ;STORE HI ADDR
        LHLD    ADDR2   ;GET LOCATION FUNCTION
        INX     H       ;POINT LO
        INX     H       ;POINT TYPE
        MOV     A,M     ;LOAD TYPE
        MOV     B,M     ;GET TYPE
        RST     4       ;ADJUST H,L
        DB      -3 AND 0FFH
        MOV     A,B     ;LOAD TYPE
        POP     B       ;RESTORE CTRS
        ANI     18H     ;ISOLATE #ARGS
        RAR             ;SHIFT RIGHT
        RAR             ;AGAIN
        RAR             ;ONCE MORE
        MOV     D,A     ;SAVE IT
        ADD     D       ;TIMES 2
        ADD     D       ;TIMES 3
        INR     B       ;POINT
        INR     B       ;LST POSIT IN LOC
        CALL    SQUIS   ;GO COMPRESS STACK
        JMP     EVAL    ;START AT BEGINNING
EV4:    LXI     D,STRIN ;POINT STRING BUFFER
        LDAX    D       ;LOAD IT
        RAR             ;DIVIDE BY TWO
        INR     A       ;ADD 1
        LHLD    SPCTR   ;GET SP COUNT
        MOV     C,A     ;SAVE LO
        MVI     B,0     ;SET HI
        DAD     B       ;ADD NUMBER WORDS
        SHLD    SPCTR   ;SAVE SP COUNT
        LXI     H,0     ;GET SOME ZEROS
        POP     B       ;GET CTRS
EV4A:   PUSH    H       ;GET 1 WORD
        DCR     A       ;DECR CTR
        JNZ     EV4A    ;LOOP
        DAD     SP      ;GET ADDRESS IN H,L
        PUSH    B       ;RE-SAVE CTRS
        PUSH    H       ;SAVE ADDR
        LDAX    D       ;GET COUNT
        INR     A       ;PLUS ONE
        MOV     B,A     ;SAVE IT
EV4B:   LDAX    D       ;GET A BYTE
        MOV     M,A     ;STORE IT
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        DCR     B       ;DECR CTR
        JNZ     EV4B    ;LOOP
        MVI     A,0E7H  ;TYPE CODE
        JMP     EV3A    ;CONTINUE
EV5:    CPI     5       ;TEST IF OPEN PAREN
        JNZ     EV6     ;BRIF NOT
        MVI     A,1     ;DELETE 1 BYTE
        CALL    SQUIS   ;GO COMPRESS IT
        LHLD    ADDR3   ;RESTORE STMT POINTER
        LDA     DIMSW   ;GET SUBSR SWITCH
        ORA     A       ;TEST IT
        JZ      LOOKO   ;BRIF NOT IN SUBS^CRIPT
        LDA     PARCT   ;GET OPEN PAREN COUNT
        ORA     A       ;TEST
        JNZ     LOOKO   ;BRIF NOT ZERO
        JMP     EVAL    ;ELSE EVALUATE COMPLETE SUBSCR
EV6:    ORA     A       ;TEST IF END OF EXPRESSION
        JNZ     EV9     ;BRIF NOT
        LDA     DIMSW   ;GET DIM SW
        ORA     A       ;TEST IT
        CNZ     EDM1    ;BRIF NOT OFF
        MOV     A,C     ;GET TERM COUNT
        CPI     1       ;TEST IF ONE
        JNZ     STERR   ;ERROR IF NOT ONE
        INX     H       ;POINT HIGH ADDR
        INX     H       ;SAME
        MOV     D,M     ;HIGH TO D
        INX     H       ;POINT LOW
        MOV     E,M     ;LOW TO E
        CALL    EVLD    ;GO LOAD VALUE
        LHLD    SPCTR   ;GET STACK CTR
EV7:    MOV     A,L     ;GET LO BYTE
        ORA     H       ;PLUS HI
        JZ      DV8     ;BRIF ZERO
        POP     D       ;RETURN 2 BYTES
        DCX     H       ;DECR CTR
        JMP     EV7     ;LOOP
DV8:    LDA     DIMSW   ;GET DIM SW
        ORA     A       ;TEST IT
        CNZ     EDM4    ;BRIF ON
        LHLD    ADDR3   ;RESTORE STMT PTR
        RET             ;RETURN TO STMT PROCESSOR
EV9:    CPI     21H     ;TEST IF PLUS
        LXI     D,FADDJ ;ADDR
        JZ      EV10    ;BRIF IS
        CPI     25H     ;TEST IF MINUS
        LXI     D,FSUB  ;ADDR
        JZ      EV10    ;BRIF IS
        CPI     41H     ;TEST IF MUL
        LXI     D,FMUL  ;ADDR
        JZ      EV10    ;BRIF IS
        CPI     45H     ;TEST IF DIV
        LXI     D,FDIV  ;ADDR
        JZ      EV10    ;BRIF IS
        CPI     1       ;TEST IF COMMA
        JZ      EVCOM   ;BRIF IS
        CPI     61H     ;TEST IF UNARY MINUS
        JZ      EVNEG   ;BRIF IS
        CPI     81H     ;TEST IF EXPONENTIAL
        LXI     D,POWER ;ADDR
        JNZ     STERR   ;ERROR IF NOT
EV10:   INX     H       ;POINT TO
        INX     H       ;1ST DATA
        PUSH    B       ;SAVE CTRS
        PUSH    D       ;SAVE ROUTINE ADDR
        MOV     D,M     ;HIGH TO D
        INX     H       ;POINT NEXT
        MOV     E,M     ;LOW TO E
        PUSH    H       ;SAVE POINTER
        CALL    EVLD    ;GO LOAD VALUE
        POP     H       ;RESTORE H,L
        INX     H       ;POINT 2ND DATA
        INX     H       ;SAME
        MOV     D,M     ;HIGH TO D
        INX     H       ;POINT NEXT
        MOV     E,M     ;LOW TO E
        INX     H       ;POINT NEXT
        LDA     NS      ;GET PREV TYPE
        CMP     M       ;TEST THIS TYPE
        JNZ     SNERR   ;BRIF MIXED MODE
        DCX     H       ;POINT BACK
        XTHL            ;POP ADDR FROM STACK, PUSH H ONTO
        LXI     B,EV11  ;RETURN ADDRESS
        PUSH    B       ;SAVE ON STACK
        PUSH    H       ;SAVE JUMP ADDR
        XCHG            ;PUT VAR ADDR TO H,L
        RET             ;FAKE CALL TO ROUTINE
FADDJ:  CPI     0E7H    ;TEST IF STRINGS
        JZ      CONCA   ;BRIF IS
        JMP     FADD    ;ELSE, GO ADD
POWER:  PUSH    H       ;SAVE ADDR OF VAR
        LXI     H,TEMP1 ;POINT SAVE AREA
        RST     3       ;SAVE X
        POP     H       ;RESTORE H,L
        RST     5       ;LOAD IT
        CALL    FTEST   ;TEST FOR ZERO
        JZ      SGN1    ;GIVE RESULT = 1 IF POWER = 0
        LXI     H,TEMP7 ;POINT SAVE AREA
        RST     3       ;SAVE B
        LXI     H,TEMP1 ;POINT X
        RST     5       ;GO LOAD IT
        CALL    FTEST   ;TEST FOR ZERO
        RZ              ;0^X = 0
        CALL    LN      ;GET NATURAL LNRITHM
        LXI     H,TEMP7 ;POINT B
        CALL    FMUL    ;GO MULTIPLY
        JMP     EXP     ;GET EXP FUNC
; X^B = EXP(B*LN(X))
XSQR:   LXI     H,TEMP1 ;POINT X
        RST     5       ;LOAD X
        LXI     H,TEMP1 ;POINT X
        JMP     FMUL    ;TIMES X
EV11:   POP     H       ;GET H,L
        POP     B       ;GET CTRS
        DCX     H       ;POINT BACK
        DCX     H       ;AND AGAIN
        CALL    GTEMP   ;GO SAVE FACC
        RST     4       ;ADJUST H,L
        DB      -7 AND 0FFH
        MVI     A,4     ;DELETE 4 BYTES
        CALL    SQUIS   ;GO COMPRESS
        JMP     EVAL    ;CONTINUE
EVNEG:  INX     H       ;POINT BACK TO OP
        PUSH    B       ;SAVE CTRS
        PUSH    H       ;SAVE H,L
        INX     H       ;DITTO
        MOV     D,M     ;GET HI BYTE
        INX     H       ;POINT NEXT
        MOV     E,M     ;GET LO BYTE
        CALL    EVLD    ;GO LOAD VAR
        CALL    NEG     ;GO NEGATE IT
        POP     H       ;GET LOCATINO
        POP     B       ;GET CTRS
        CALL    GTEMP   ;GO STORE FACC IN STACK
        RST     4       ;ADJUST H,L
        DB      -4 AND 0FFH
EVCOM:  MVI     A,1     ;DELETE 1 BYTE
        CALL    SQUIS   ;COMPRESS
        LXI     H,CMACT ;GET COUNT
        INR     M       ;INCR
        JMP     EVAL    ;CONTINUE
EVLD:   INX     H       ;POINT TYPE
        MOV     A,M     ;LOAD IT
        STA     NS      ;SAVE IT
        XCHG            ;SAVE H,L IN D,E
        CPI     0E7H    ;TEST IF STRING
        JNZ     RST5    ;LOAD FLOATING POINT
        LXI     D,STRIN ;POINT BUFFER
        MOV     A,M     ;GET COUNT
        INR     A       ;ADD ONE
        MOV     B,A     ;SAVE COUNT
EVLD1:  MOV     A,M     ;GET NEXT
        STAX    D       ;STORE IT
        INX     H       ;POINT NEXT
        INX     D       ;DITTO
        DCR     B       ;DECR COUNT
        JNZ     EVLD1   ;LOOP
        RET             ;RETURN
;
EDM1:   MOV     A,C     ;GET ITEM COUNT
        PUSH    H       ;SAVE H,L
        CPI     1       ;TEST IF 1
        JNZ     EDM3    ;BRIF NOT
        MVI     B,4     ;GET COUNT
        LXI     H,TEMP1 ;POINT AREA
        CALL    ZEROM   ;GO ZERO IT
EDM2A:  POP     H       ;RESTORE H,L
        MVI     C,1     ;SET COUNT
        RET             ;RETURN
EDM3:   CPI     2       ;TEST IF 2
        JNZ     SNERR   ;ELSE, ERROR
        RST     4       ;POINT 2ND ARG
        DB      5
        MOV     D,M     ;GET HI ADDR
        INX     H       ;POINT NEXT
        MOV     E,M     ;GET LO ADDR
        CALL    EVLD    ;LOAD THE ARG
        LXI     H,TEMP1 ;POINT AREA
        RST     3       ;SAVE THE ARG
        JMP     EDM2A   ;CONTINUE
EDM4:   CALL    FACDE   ;CONVERT FACC TO D,E
        PUSH    D       ;PUT D,E TO B,C
        POP     B
        PUSH    B       ;SAVE COL
        LXI     H,TEMP1 ;POINT 2ND ARGUMENT
        RST     5       ;LOAD IT IN FACC
        CALL    FACDE   ;CONVERT TO D,E
        POP     B       ;GET COL
        XRA     A       ;GET A ZERO
        STA     DIMSW   ;RESET SW
        RET             ;RETURN
LDV2A:  MOV     A,B     ;GET HI NAME
        ORI     80H     ;SET BIT
        MOV     B,A     ;RESTORE
        PUSH    B       ;SAVE NAME
        XCHG            ;SAVE H,L IN D,E
        LDA     PARCT   ;GET PAREN COUNT
        PUSH    PSW     ;SAVE
        XRA     A       ;CLEAR REG A
        STA     PARCT   ;RESET COUNT
        LHLD    SPCTR   ;GET STACK COUNTER
        PUSH    H       ;SAVE IT
        LXI     H,0     ;GET A ZERO
        SHLD    SPCTR   ;RESET CTR
        LHLD    EXPRS   ;GET EXPRST
        PUSH    H       ;SAVE IT
        INX     H       ;POINT NEXT
        MVI     M,0     ;SET NEW START
        SHLD    EXPRS   ;SAVE IT
        LDA     DIMSW   ;GET PREV SE
        PUSH    PSW     ;SAVE IT
        XCHG            ;RESTORE H,L
        MVI     A,0FFH  ;GET ON VALUE
        STA     DIMSW   ;SET SW
        CALL    LOOKD   ;RECURSIVE CALL
        POP     PSW     ;GET DIM SW
        STA     DIMSW   ;REPLACE IT
        SHLD    ADDR3   ;SAVE H,L
        POP     H       ;GET EXPRST
        SHLD    EXPRS   ;SAVE IT
        POP     H       ;GET STACK COUNTER
        SHLD    SPCTR   ;RESTORE IT
        POP     PSW     ;GET PAREN COUNT
        STA     PARCT   ;RESTORE IT
        POP     H       ;GET NAME
        PUSH    D       ;SAVE ROW
        PUSH    B       ;SAVE COL
        XCHG            ;PUT NAME IN D,E
        CALL    SEARC   ;GO FIND ADDRESS (PUT IN D,E)
        POP     D       ;GET ADDR
        POP     B       ;RESTORE COL
        POP     D       ;RESTORE ROW
        CALL    SUBSC   ;GET SUBSCRIPT (RETURNS ADDR IN H,L)
        XCHG            ;SAVE IN D,E
        LHLD    ADDR3   ;GET H,L
        PUSH    H       ;SAVE ON STACK
        JMP     LDV     ;CONTINUE
;       PAGE
;
FIN     EQU     $
;
; FLOATING POINT INPUT CONVERSION ROUTINE
;
; THIS SUBROUTINE CONVERTS AN ASCII STRING OF CHARACTERS
; TO THE FLOATING POINT ACCUMULATOR.  THE INPUT FIELD
; MAY CONTAIN ANY VALID NUMBER, INCLUDING SCIENTIFIC
; NOTATION (NNN.NNNNE+NN).
; THE INPUT STRING IS TERMINATED BY ANY NON-NUMERIC CHAR
;
;
        XCHG            ;PUT ADDR TO D,E
        MVI     C,0     ;INITIAL VALUE EXCESS DIGIT COUNT
        CALL    FIN8    ;GET INTEGER PORTION
        MVI     B,0     ;CLEAR DIGIT COUNT
        CPI     '.'     ;TEST IF DEC-POINT
        JNZ     FIN2    ;BRIF NOT
        CALL    FIN9    ;GET FRACTION
FIN2:   POP     PSW     ;GET SIGN
        ORI     24      ;SET UP FOR FLOAT
        STA     FACC
        MOV     A,B     ;GET # FRACTION DIGITS
        ADD     C       ;+ EXCESS DIGITS
        PUSH    PSW     ;SAVE POWER OF TEN
        PUSH    D       ;SAVE PTR
        CALL    FNORM   ;NORMALIZE NUMBER
        LDAX    D       ;GET NEXT CHARACTER
        CPI     'E'     ;TEST IF EXPONENT
        JNZ     FIN4    ;BRIF NOT
        LXI     H,FTEMP ;POINT SAVE AREA
        RST     3       ;SAVE ACC
        POP     D       ;RESTORE PTR
        INX     D       ;SKIP 'E'
        CALL    FIN8    ;GET NUMERIC EXP
        LDA     FACC+3  ;GET EXPONENT
        POP     B       ;EXPONENT SIGN
        INR     B       ;TEST
        JP      FIN3    ;BRIF NOT NEG
        CMA             ;NEGATE EXPONENT
        INR     A
FIN3:   POP     B       ;POWER OF TEN
        ADD     B       ;ADD EXPONENT
        PUSH    PSW     ;SAVE COUNT
        LXI     H,FTEMP ;RESTORE NUMBER
        PUSH    D       ;SAVE PTR
        RST     5       ;LOAD IT
FIN4:   POP     H       ;RESTORE PTR
        POP     PSW     ;RESTORE COUNT
FIN5:   RZ              ;RETURN IF ZERO
        PUSH    H       ;SAVE H,L
        LXI     H,TEN   ;POINT CONSTANT: 10
        JM      FIN7    ;BRIF DIVIDE NEEDED
        DCR     A       ;DECR COUNT
        PUSH    PSW     ;SAVE COUNT
        CALL    FMUL    ;GO MULTIPLY BY 10
FIN6:   POP     PSW     ;RESTORE COUNT
        POP     H       ;RESTORE H,L
        JMP     FIN5    ;CONTINUE
FIN7:   INR     A       ;INCR COUNT
        PUSH    PSW     ;SAVE COUNT
        CALL    FDIV    ;GO DIVIDE BY 10
        JMP     FIN6    ;LOOP
;
; FIN8  CONVERT NUMBER STRING TO FACC
; ON ENTRY, C=INIT VALUE EXCESS DIGIT COUNT
;             DE=INPUT STRING
; ON EXIT, SIGN IS ON STACK
;       B=DIGIT COUNT
;       C=EXCESS DIGIT COUNT
;
FIN8:   LXI     H,FACC  ;CLEAR FACC
        MVI     B,4
        CALL    ZEROM
        LXI     H,8000H ;ASSUME MINUS
        LDAX    D       ;GET CHAR
        CPI     '-'
        JZ      FIN8A
        MOV     H,L     ;NOPE, MUST BE PLUS
                        ;(B IS CLEARED BY ZEROM)
        CPI     '+'
        JZ      FIN8A
        DCX     D       ;NEITHER, BACK UP POINTER
FIN8A:  XTHL            ;GET RETURN, PUSH SIGN
        PUSH    H       ;RESTORE RETURN
FIN9:   INX     D       ;POINT NEXT
        LDAX    D       ;GET CHAR
        CPI     '0'     ;TEST IF LESS ZERO
        RC              ;RETURN IF IS
        CPI     '9'+1   ;TEST IF GT NINE
        RNC             ;RETURN IF IS
        DCR     B       ;DIGIT COUNT
        PUSH    D       ;SAVE PTR
        PUSH    B       ;SAVE COUNTERS
        CALL    FMTEN   ;MULTIPLY FACC*TEN
        ORA     A       ;TEST FOR OVERFLOW
        JZ      FINB    ;BRIF NO OVERFLOW
        LXI     H,FTEMP+4
        RST     5       ;RESTORE OLD FACC
        POP     B       ;RESTORE COUNTERS
        INR     C       ;EXCESS DIGIT
        POP     D
        JMP     FIN9
FINB:   POP     B       ;RSTORE COUNTERS
        POP     D       ;& PTR
        LDAX    D       ;GET THE DIGIT
        ANI     0FH     ;MASK OFF ZONE
        LXI     H,FACC+3        ;POINT ACC
        ADD     M       ;ADD
        MOV     M,A     ;STORE
        DCX     H       ;POINT NEXT
        MOV     A,M     ;LOAD
        ACI     0       ;PLUS CARRY
        MOV     M,A     ;STORE
        DCX     H       ;POINT NEXT
        MOV     A,M     ;LOAD
        ACI     0       ;PLUS CARRY
        MOV     M,A     ;STORE
        JMP     FIN9    ;LOOP
;
; MULTIPLY FACC BY TEN
;
FMTEN:  LXI     H,FTEMP+4
        RST     3       ;SAVE FACC
        CALL    FIND    ;*2
        CALL    FIND    ;*4
        LXI     H,FTEMP+7
        CALL    FIND0   ;*5
FIND:   LXI     H,FACC+3        ;DOUBLE FACC
FIND0:  LXI     D,FACC+3
        MVI     B,4     ;BYTE COUNT
        JMP     FADDT   ;ADD & RETURN
;PAGE
;
FOUT    EQU     $
;
; FLOATING POINT OUTPUT FORMAT ROUTINE
;
; THIS SUBROUTINE CONVERTS A NUMBER IN FACC TO A
; FORMAT SUITABLE FOR PRINTING.  THAT IS, THE
; NUMBER WILL BE IN SCIENTIFIC NOTATION IF EXPONENT
; IS > 5 OR < -2, OTHERWISE IT WILL BE ZERO SUPRESSED
; ON BOTH SIDES.
;
        LXI     D,FACC+3        ;POINT LSB
        LDAX    D       ;LOAD IT
        ORI     7       ;MASK FOR OUTPUT
        STAX    D       ;REPLACE
        CALL    FTEST   ;GET SIGN OF NUMBER
        MVI     M,' '   ;DEFAULT SPACE
        JP      FOUT0   ;BRIF NOT MINUS
        MVI     M,'-'   ;MOVE DASH
FOUT0:  INX     H       ;POINT NEXT
        JNZ     FOUT2   ;BRIF NOT ZERO
        MVI     M,'0'   ;MOVE THE ZERO
        INX     H       ;POINT NEXT
        MVI     M,' '   ;MOVE SPACE FOLLOWING
        RET             ;RETURN
FOUT2:  LDA     FACC    ;GET SIGN & EXP
        CALL    FEXP    ;EXPAND EXPONENT
        JNZ     FOUTV   ;BRIF NOT ZERO
        MVI     A,80H   ;SET NEG
FOUTV:  ANI     80H     ;ISOLATE
        STA     DEXP    ;SAVE SIGN
        PUSH    H       ;SAVE H,L
FOUT3:  LDA     FACC    ;GET SIGN & EXP
        CALL    FEXP    ;EXPAND EXP
        CPI     1       ;TEST RANGE
        JP      FOUT6   ;BRIF IN RANGE
FOUT4:  LXI     H,DEXP  ;POINT DEC.EXP
        INR     M       ;INCR IT
        LXI     H,TEN   ;POINT CONST: 10
        JP      FOUT5   ;BRIF POS.
        CALL    FMUL    ;MULTIPLY
        JMP     FOUT3   ;LOOP
FOUT5:  CALL    FDIV    ;DIVIDE
        JMP     FOUT3   ;LOOP
FOUT6:  CPI     5       ;TEST HIGH RANGE
        JP      FOUT4   ;BRIF 5 OR GREATER
        LXI     H,FTEMP ;POINT SAVE AREA
        RST     3       ;STORE IT
        LDA     FACC    ;GET EXPONENT
        CALL    FEXP    ;EXPAND
        MVI     C,6     ;DIGIT COUNT
        CALL    FOUTB   ;SHIFT LEFT
        CPI     10      ;TEST IF DECIMAL POINT
        JM      FOUTU   ;BRIF LT
        LXI     H,FTEMP ;POINT SAVE AREA
        RST     5       ;LOAD IT
        JMP     FOUT4   ;ONCE MORE
FOUTU:  CALL    FOUT9   ;PUT DIGIT
FOUT7:  XRA     A       ;CLEAR STATUS
        STA     FACC    ;AND OVERFLOW
        CALL    FMTEN   ;MULTIPLY BY TEN
        CALL    FOUT9   ;PUT DIGIT
        JNZ     FOUT7   ;LOOP
        JMP     FOUTH   ;GO AROUND
FOUT9:  ORI     30H     ;DEC. ZONE
        POP     H       ;GET RETURN ADDR
        XTHL            ;EXCH WITH TOP (PTR)
        MOV     M,A     ;PUT DIGIT
        INX     H       ;POINT NEXT
        MOV     A,C     ;GET COUNT
        CPI     6       ;TEST IF 1ST
        JNZ     FOUTA   ;BRIF NOT
        MVI     M,'.'   ;MOVE DEC. PT.
        INX     H       ;POINT NEXT
FOUTA:  XTHL            ;EXCH WITH RTN
        DCR     C       ;DECR COUNT
        PCHL            ;RETURN
FOUTB:  MOV     E,A     ;SAVE BIT COUNT
        XRA     A       ;CLEAR ACC FLAGS
        STA     FACC    ;AND OVERFLOW
FOUTC:  LXI     H,FACC+3        ;POINT LSB
        MVI     B,4     ;BYTE COUNT
FOUTD:  MOV     A,M     ;GET A BYTE
        RAL             ;SHIFT LEFT
        MOV     M,A     ;STORE
        DCX     H       ;POINT NEXT
        DCR     B       ;DECR CTR
        JNZ     FOUTD   ;LOOP
        DCR     E       ;DECR BIT CTR
        JNZ     FOUTC   ;LOOP
        RET             ;RETURN
FOUTH:  POP     H       ;GET PTR
        MVI     M,'E'   ;EXPONENT
        INX     H       ;POINT NEXT
        LDA     DEXP    ;GET EXPONENT
        MVI     M,'+'   ;DEFAULT
        MOV     D,A     ;SAVE NUMBER
        ORA     A       ;TEST IT
        JP      FOUTI   ;BRIF POS
        MVI     M,'-'   ;ELSE, DASH
        ANI     7FH     ;STRIP DUMB SIGN
        CMA             ;COMPLEMENT
        INR     A       ;PLUS ONE (TWOS COMP)
        MOV     D,A     ;SAVE IT
        CMA             ;RE-COMPLEMENT
        INR     A       ;PLUS ONE
FOUTI:  INX     H       ;POINT NEXT
        PUSH    H       ;SAVE PTR
        MVI     E,-1 AND 0FFH   ;INIT CTR (TENS)
FOUTJ:  INR     E       ;ADD ONE
        SUI     10      ;LESS 10
        JP      FOUTJ   ;LOOP
        ADI     10      ;CORRECT UNITS
        MOV     B,A     ;SAVE UNITS
        MOV     A,E     ;GET TENS
        CALL    FOUT9   ;OUTPUT
        MOV     A,B     ;GET UNITS
        CALL    FOUT9   ;OUTPUT
        POP     H       ;GET PTR
        MVI     M,' '   ;SPACE AFTER
        MOV     A,D     ;GET DEC EXPON
        ORA     A       ;SET FLAGS
        JP      FOUTK   ;BRIF POS.
        CPI     -2 AND 0FFH     ;TEST FOR MIN
        RC              ;RETURN IF LESS THAN -2
        JMP     FOUTL   ;GO AROUND
FOUTK:  CPI     6       ;TEST IF TOO BIG
        RNC             ;RETURN IF 6 OR GREATER
FOUTL:  MOV     C,A     ;SAVE EXPONENT
        MVI     B,5     ;CTR
FOUTM:  MVI     M,' '   ;SPACE OUT EXPONENT
        DCX     H       ;POINT PRIOR
        DCR     B       ;DECR CTR
        JNZ     FOUTM   ;LOOP
        XCHG            ;FLIP/FLOP
        MOV     A,E     ;GET LOW BYTE
        SUI     5       ;POINT TO DOT
        MOV     L,A     ;PUT DOWN
        MOV     A,D     ;GET HIGH
        SBI     0       ;IN CASE OF BORROW
        MOV     H,A     ;PUT DOWN
        MOV     A,C     ;GET EXPONENT
        ORA     A       ;TEST SIGN
        JZ      FOUTO   ;BRIF ZERO
        JM      FOUTR   ;BRIF NEGATIVE
FOUTN:  MOV     B,M     ;GET HIGH BYTE
        INX     H       ;POINT NEXT
        MOV     A,M     ;GET LOW BYTE
        MOV     M,B     ;SHIFT DOT TO RIGHT
        DCX     H       ;POINT BACK
        MOV     M,A     ;MOVE THE DIGIT LEFT
        INX     H       ;POINT NEXT
        DCR     C       ;DECR CTR
        JNZ     FOUTN   ;LOOP
FOUTO:  XCHG            ;POINT END
FOUTP:  MOV     A,M     ;GET A DIGIT/DOT
        CPI     '0'     ;TEST FOR TRAILING ZERO
        JNZ     FOUTQ   ;BRIF NOT
        MVI     M,' '   ;SPACE FILL
        DCX     H       ;POINT PRIOR
        JMP     FOUTP   ;LOOP
FOUTQ:  CPI     '.'     ;TEST FOR TRAILING DOT
        INX     H       ;JUST IN CASE NOT
        RNZ             ;RETURN IF NOT
        DCX     H       ;RESET PTR
        MVI     M,' '   ;SPACE IT OUT
        RET             ;RETURN
FOUTR:  CPI     0FFH    ;TEST IF -1
        JNZ     FOUTS   ;ELSE -2
        DCX     H       ;POINT SIGNIFICANT
        MOV     A,M     ;GET THE CHAR
        MVI     M,'.'   ;MOVE THE DOT
        INX     H       ;POINT NEXT
        MOV     M,A     ;SHIFT THE DIGIT
        JMP     FOUTO   ;GO ZERO SUPPRESS
FOUTS:  DCX     H       ;POINT ONE TO LEFT
        MOV     A,M     ;PICK UP DIGIT
        MVI     M,'0'   ;REPLACE
        INX     H       ;POINT RIGHT
        MOV     M,A     ;PUT THE DIGIT
        MOV     H,D     ;GET LOW ADDR
        MOV     L,E     ;POINT LAST DIGIT
        MVI     B,6     ;CTR
FOUTT:  DCX     H       ;POINT PRITO
        MOV     A,M     ;GET A DIGIT
        INX     H       ;POINT
        MOV     M,A     ;PUT IT ONE TO RIGHT
        DCX     H       ;POINT
        DCR     B       ;DECR CTR
        JNZ     FOUTT   ;LOOP
        MVI     M,'.'   ;MOVE THE DOT
        JMP     FOUTO   ;CONTINUE
;
FADD    EQU     $
;
;
; FLOATING POINT ADD THE NUMBER AT (H,L) TO THE FACC
;
;
        INX     H       ;POINT FIRST DIGIT
        MOV     A,M     ;LOAD IT
        ORA     A       ;TEST IT
        JZ      FTEST   ;BRIF ZERO
        DCX     H       ;POINT BACK
        CALL    FTEST   ;GO TEST SIGN OF FACC
        JZ      RST5    ;JUST LOAD IF FACC = 0
        CALL    FEXP    ;GO GET EXPONENT
        MOV     B,A     ;SAVE EXPONENT
        MOV     A,M     ;GET EXPONENT OF ADDR
        CALL    FEXP    ;GO GET EXPONENT
        MOV     C,A     ;SAVE THE EXPONENT
        SUB     B       ;GET DIFFERENCE OF TWO EXPONENTS
        JZ      FADD4   ;BRIF THEY'RE EQ
        JP      FADD3   ;BRIF DIFFERENCE IS POSITIVE
        CMA             ;COMPLEMENT ACC
        INR     A       ;PLUS ONE (TWO'S COMPLEMENT)
FADD3:  CPI     24      ;COMPARE DIFFERENCE TO MAX
        JC      FADD4   ;BRIF LESS
        MOV     A,B     ;GET EXPON OF ADDUEND
        SUB     C       ;GET TRUE DIFFERENCE AGAIN
        JP      FTEST   ;BRIF FACC > ADDER
        JMP     RST5    ;ELSE, ADDER > FACC
FADD4:  PUSH    PSW     ;SAVE DIFFERENCE
        PUSH    B       ;SAVE EXPONENTS
        LXI     D,FTEMP ;GET ADDR OF TEMP ACC
        CALL    CPY4H
        POP     B       ;GET EXPONENTS
        POP     PSW     ;GET DIFFERENCE
        JZ      FADD9   ;JUST ADD IF ZERO
        LXI     H,FTEMP+1       ;DEFAULT
        PUSH    PSW     ;SAVE DIFFERENCE
        MOV     A,B     ;GET FACC EXPON
        SUB     C       ;MINUS FTEMP EXPON
        JP      FADD6   ;BRIF TEMP MUST BE SHIFTED
        LXI     H,FACC  ;POINT FLOAT ACC
        MOV     A,C     ;GET EXPONENT, SIGN
        ANI     7FH     ;STRIP EXP SIGN
        MOV     C,A     ;PUT BACK
        MOV     A,M     ;GET THE EXP
        ANI     80H     ;STRIP OFF OLD EXPON
        ORA     C       ;MOVE ADDR EXPON TO IT
        MOV     M,A     ;REPLACE
        INX     H       ;POINT FIRST DATA BYTE
FADD6:  POP     PSW     ;GET DIFFER
        MOV     C,A     ;SAVE IT
FADD7:  MVI     B,3     ;LOOP CTR (INNER)
        XRA     A       ;INIT CARRY TO Z
        PUSH    H       ;SAVE ADDR
        CALL    FSHFT   ;GO SHIFT
        POP     H       ;GET ADDR
        DCR     C       ;DECR CTR
        JNZ     FADD7   ;LOOP
FADD9   EQU     $
        LXI     H,FTEMP
        LDA     FACC    ;GET EXPONENT
        XRA     M       ;SEE IF SIGNS THE SAME
        LXI     D,FACC+3        ;POINT LEAST SIGN BYTE
        LXI     H,FTEMP+3
        JM      FADDA   ;BRIF SIGNS DIFFERENT
        CALL    FADT3   ;ADD 3 BYTES
        JNC     FTEST   ;BRIF NO OVERFLOW
        XCHG            ;POINT HL TO FACC
        CALL    SVSGN   ;SAVE SIGN, RETURN EXPONENT
        INR     A       ;INCREMENT EXPONENT
        CALL    RSSGN   ;RESTORE SIGN TO EXPONENT
        INX     H       ;POINT DATA
        STC             ;SET CY
        MVI     B,3     ;CTR
        CALL    FSHFT   ;GO SHIFT IT
        JMP     FTEST   ;RETURN
FADDA   EQU     $
        MVI     B,3
        CALL    FSUBT   ;SUBTRACT
        JNC     FNORM   ;BRIF NO BORROW
        LXI     H,FACC+3        ;MUST NEGATE
        MVI     B,3
        STC
FNEG1:  MOV     A,M     ;GET BYTE
        CMA
        JNC     FNEG2
        ADI     1       ;INCREMENT + COMPLEMENT=NEGATE
FNEG2:  MOV     M,A
        DCX     H
        DCR     B
        JNZ     FNEG1
        CALL    FNORM
        JMP     NEG     ;REVERSE SIGN
;PAGE
;
FNORM   EQU     $
;
;
; NORMALIZE THE FLOATING ACCUMULATOR
; THAT IS, THE FIRST BIT MUST BE SIGNIFICANT
;
;
        LXI     H,FACC+3        ;POINT LSB
        MOV     A,M     ;LOAD IT
        DCX     H       ;POINT PRIOR
        ORA     M       ;MERGE
        DCX     H       ;POINT PRIOR
        ORA     M       ;MERGE
        DCX     H
        MOV     B,M     ;SAVE EXPONENT
        MOV     M,A     ;CLEAR
        RZ              ;RETURN ON NOTHING TO NORMALIZE
        MOV     M,B     ;RESTORE EXP
        PUSH    B       ;SAVE C FOR CALLER
        CALL    SVSGN   ;SAVE SIGN
        MOV     M,A     ;STORE EXPANDED EXPONENT
FNRM1:  INX     H       ;POINT TO MOST SIGN BYTE
        MOV     A,M     ;GET MSB
        ORA     A       ;TEST IT
        JM      FNRM3   ;BRIF NORMALIZED
        INX     H       ;POINT LSB
        INX     H
        MVI     B,3     ;SHIFT COUNT
FNRM2:  MOV     A,M     ;SHIFT LEFT
        RAL
        MOV     M,A
        DCX     H
        DCR     B
        JNZ     FNRM2
        DCR     M       ;ADJUST EXPONENT
        JMP     FNRM1   ;LOOP
FNRM3:  DCX     H       ;POINT BACK TO EXPONENT
        MOV     A,M
        CALL    RSSGN   ;RESTORE SIGN
        POP     B       ;RESTORE C
        RET
;
FSUB    EQU     $
;
;
; FLOATING POINT SUBTRACT THE NUMBER AT (H,L) FROM THE FACC
;
;
        CALL    NEG     ;NEGATE FACC
        CALL    FADD    ;ADD
        CALL    NEG     ;NEGATE RESULT
        JMP     FTEST
;PAGE
;
FMUL    EQU     $
;
;
; FLOATING POINT MULTIPLY THE NUMBER AT (H,L) TO THE FACC
;
;
        CALL    FTEST   ;TEST FACC
        RZ              ;RETURN IF ZERO
        INX     H       ;POINT 1ST DIGIT OF MULTIPLIER
        MOV     A,M     ;LOAD IT
        DCX     H       ;RESTORE
        ORA     A       ;TEST IF ZERO
        JZ      RST5    ;GO LOAD TO FACC IF IT IS
        PUSH    H       ;SAVE MULTIPLIER ADDRESS
        CALL    MDSGN   ;GET SIGN PRODUCT, & BOTH EXPONENTS
        ADD     B       ;ADD EXPONENTS
        CALL    RSSGN   ;RESTORE SIGN
        POP     H       ;RESTORE
        LXI     D,FTEMP+9       ;POINT TEMP STORAGE
        MVI     B,3     ;BYTE COUNT
        INX     H       ;POINT MSD
        CALL    COPYH   ;MOVE MULTIPLIER
        LXI     H,FTEMP ;POINT DIGIT 7 OF RESULT
        MVI     B,6     ;LOOP CTR
        CALL    ZEROM   ;GO ZERO EIGHT BYTES
        LXI     D,FACC+1        ;POINT 1ST DIGIT OF ACC
        MVI     B,3     ;LOOP CTR
FMUL5:  LDAX    D       ;GET AN ACC DIGIT PAIR
        MOV     M,A     ;PUT TO TEMP STORAGE
        XRA     A       ;ZERO A
        STAX    D       ;CLEAR ACC
        INX     D       ;POINT NEXT
        INX     H       ;DITTO
        DCR     B       ;DECR CTR
        JNZ     FMUL5   ;LOOP
        MVI     C,24    ;OUTTER LOOP CTR
FMUL6:  MVI     B,3     ;CTR
        LXI     H,FTEMP+9       ;POINT MULTIPLIER
        XRA     A       ;CLEAR CY
FMUL7:  MOV     A,M     ;GET BYTE
        RAR             ;SHIFT RIGHT
        MOV     M,A     ;PUT DOWN
        INX     H       ;POINT NEXT
        DCR     B       ;DECR CTR
        JNZ     FMUL7   ;LOOP
        JNC     FMUL8   ;BRIF ZERO BIT
        LXI     D,FTEMP+2       ;POINT RESULT
        LXI     H,FTEMP+8       ;POINT MULTIPLICAND
        MVI     B,6     ;SIX BYTE ADD
        CALL    FADDT   ;GO ADD
FMUL8:  MVI     B,6     ;SIZ BYTE SHIFT
        LXI     H,FTEMP+8       ;POINT MULTIPLICAND
        XRA     A       ;CLEAR CY
FMUL9:  MOV     A,M     ;GET BYTE
        RAL             ;SHIFT LEFT
        MOV     M,A     ;PUT BACT
        DCX     H       ;POINT NEXT BYTE
        DCR     B       ;DECR CTR
        JNZ     FMUL9   ;LOOP
        DCR     C       ;DEC BIT COUNT
        JNZ     FMUL6   ;CONTINUE
        JMP     FNORM   ;GO NORMALIZE
;
; MDSGN   GET SIGN PRODUCT AND EXPONENTS FOR MULT & DIV
; ON ENTRY:
;       (HL) = ONE NUMBER
;       (FACC)=THE OTHER
; ON RETURN:
;       A = EXPONENT OF FACC(EXPANDED)
;       B = OTHER EXPONENT
;       C = SIGN PRODUCT
;       HL DESTROYED
;
MDSGN:  CALL    SVSGN   ;GET SIGN IN C, EXP IN A
        MOV     B,A     ;SAVE EXPONENT
        LXI     H,FACC
        MOV     A,C     ;GET SIGN
        ADD     M       ;MULTIPLY SIGNS
        MOV     M,A     ;PUT DOWN
;
; SVSGN         GET SIGN AND EXP
; ON ENTRY:
;       (HL) = EXPONENT
; ON RETURN:
;       A = EXPANDED EXPONENT
;       C = SIGN IN HI ORDER BIT
;
SVSGN:  MOV     A,M     ;GET EXPONENT
        ANI     80H     ;ISOLATE SIGN
        MOV     C,A
        MOV     A,M
        JMP     FEXP    ;EXPAND EXP AND RETURN
;
; RSSGN         RESTORE SIGN TO EXPONENT
; ON ENTRY:
;       (HL)=EXPONENT
;       A = EXPANDED EXPONENT
;       C = SIGN
; ON RETURN:
;       A = EXPONENT
;       (HL) = EXPONENT WITH SIGN
;       Z,M BITS SET FOR EXPONENT
;
RSSGN:  CALL    FOVUN   ;CHECK FOR OVER/UNDERFLOW
        ANI     7FH     ;REMOVE EXPONENT SIGN
        ORA     C       ;ADD SIGN
        MOV     M,A     ;SET DOWN
        JMP     FTEST   ;SET Z,M BITS
;PAGE
;
FDIV    EQU     $
;
;
; FLOATING POINT DIVIDE THE NUMBER AT (H,L) INTO THE FACC
;
;
        CALL    FTEST   ;TEST IF FACC ZERO
        RZ              ;RETURN IF IT IS
        INX     H       ;POINT 1ST DIGIT OF DIVISOR
        MOV     A,M     ;LOAD IT
        DCX     H       ;POINT BACK
        ORA     A       ;TEST IF ZERO
        JZ      ZMERR   ;DIVISION BY ZERO = ERROR
        PUSH    H       ;SAVE DIVISOR PTR
        CALL    MDSGN   ;GET SIGN ON STACK, EXPS INTO A,B
        SUB     B       ;SUBTRACT EXPONENTS
        INR     A       ;PLUS ONE
        CALL    RSSGN   ;SET SIGN/EXPONENT IN FACC
        LXI     D,FACC+1
        LXI     H,FTEMP ;POINT TEMPORARY STORAGE
        MVI     M,0     ;CLEAR MSB
        INX     H       ;POINT NEXT
        MVI     B,3     ;LOOP CTR
FDIV3:  LDAX    D       ;GET BYTE FROM FACC
        MOV     M,A     ;PUT TO FTEMP
        XRA     A       ;CLEAR A
        STAX    D       ;ZERO FACC
        INX     H       ;POINT NEXT
        INX     D       ;DITTO
        DCR     B       ;DECR CTR
        JNZ     FDIV3   ;LOOP
        POP     D       ;GET ADDR
        MVI     B,3     ;LOOP CTR
        INX     D       ;POINT MSD OF DIVISOR
        MVI     M,0     ;CLEAR MSB
        INX     H       ;POINT NEXT
        CALL    COPYD   ;GO MOVE IT
        MVI     C,24    ;OUTER LOOP CTR
FDIV5:  LXI     D,FTEMP+3       ;POINT DIVIDEND
        LXI     H,FTEMP+7       ;AND DIVISOR
        MVI     B,4     ;CTR
        CALL    FSUBT   ;GO SUBTRACT
        JNC     FDIV6   ;BRIF NO GO
        LXI     D,FTEMP+3       ;POINT DIVIDEND
        LXI     H,FTEMP+7       ;AND DIVISOR
        MVI     B,4     ;CTR
        CALL    FADDT   ;GO RE-ADD
        STC             ;TURN ON CY
FDIV6:  CMC             ;REVERSE CY
        MVI     B,3     ;CTR
        LXI     H,FACC+3        ;POINT LSB
FDIV7:  MOV     A,M     ;LOAD BYTE
        RAL             ;SHIFT LEFT
        MOV     M,A     ;REPLACE
        DCX     H       ;POINT NEXT
        DCR     B       ;DECR CTR
        JNZ     FDIV7   ;LOOP
        XRA     A       ;CLEAR FLAGS
        MVI     B,4     ;CTR
        LXI     H,FTEMP+3       ;POINT-DIVIDEND
FDIV8:  MOV     A,M     ;LOAD BYTE
        RAL             ;SHIFT LEFT
        MOV     M,A     ;REPLACE
        DCX     H       ;POINT ENXT
        DCR     B       ;DECR CTR
        JNZ     FDIV8   ;LOOP
        DCR     C       ;DECR OTR CTR
        JNZ     FDIV5   ;LOOP
        JMP     FNORM   ;WRAPUP
;
; UTILITY ROUTINE TO GET A VARIABLE'S ADDRESS TO H,L
;
GETST:  LXI     D,STRIN ;POINT BUFFER
        MVI     B,0     ;INIT CTR
        MOV     A,M     ;GET THE CHAR
        CPI     '"'     ;TEST IF LIT TYPE
        JZ      GETS2   ;BRIF IS
        CPI     27H     ;TEST IF QUOTED LITERAL
        JZ      GETS2   ;BRIF IS
GETS1:  CPI     ','     ;TEST IF COMMA
        JZ      GETS5   ;BRIF IS
        ORA     A       ;TEST IF END
        JZ      GETS5   ;BRIF IS
        INR     B       ;COUNT IT
        INX     D       ;POINT NEXT
        STAX    D       ;PUT CHAR
        INX     H       ;POINT NEXT
        RST     1       ;SKIP SPACES
        JMP     GETS1   ;LOOP
GETS2:  MOV     C,A     ;SAVE DELIM
GETS3:  INX     H       ;SKIP THE QUOTE
        MOV     A,M     ;GET NEXT CHAR
        CMP     C       ;TEST IF END OF LITERAL
        JZ      GETS4   ;BRIF IS
        ORA     A       ;TEST IF END OF LINE
        JZ      CVERR   ;BRIF IS
        INR     B       ;COUNT IT
        INX     D       ;POINT NEXT
        STAX    D       ;PUT CHAR
        JMP     GETS3   ;LOOP
GETS4:  INX     H       ;SKIP END QUOTE
        RST     1       ;SKIP TRAILING SPACES
GETS5:  LXI     D,STRIN ;POINT BEGIN BUFFER
        MOV     A,B     ;GET COUNT
        STAX    D       ;PUT COUNT
        POP     D       ;GET RETURN ADDR
        XCHG            ;FLIP/FLOP
        XTHL            ;PUT RET ON STACK, HL OF VAR IN HL
        PUSH    D       ;SAVE H,L OF LOC
        CALL    LET2A   ;GO STORE STRING
        POP     H       ;RESTORE LOCATION
        RET             ;RETURN
GETS8:  CALL    VAR     ;GET VAR NAME
        PUSH    D       ;SAVE ON STACK
        MOV     A,D     ;GET HI BYTE
        ORA     A       ;TEST IF ARRAY
        JP      GETS9   ;BRIF NOT
        CALL    SEARC   ;GO GET ARRAY PARAMS
        MVI     A,0FFH  ;TURN ON SW
        STA     DIMSW   ;SET IT
        XTHL            ;SWAP ADDR ON STACK
        CALL    EXPR    ;GO GET ROW, COL PTRS
        XTHL            ;SWAP ADDR ON STACK
        CALL    SUBSC   ;GO POINT TO ENTRY
        XCHG            ;EXCHANGE
        POP     H       ;GET ADDRESS OF STMT
        POP     B       ;GET NAME
        RET             ;RETURN
GETS9:  CALL    SEARC   ;FIND ADDR
        POP     B       ;RESTORE NAME
        RET             ;RETURN
;
FOVUN   EQU     $
;
; TEST EXPONENT FOR OVERFLO OR UNDERFLOW
;
        ORA     A       ;TEST IT
        JP      FOV1    ;BRIF POS.
        CPI     0C1H    ;TEST FOR MAX NEG
        RNC             ;RETURN IF NO UNDER.
        MVI     A,0C1H  ;SET EXPONENT AT MINIMUM
        JMP     UNERR
FOV1:   CPI     40H     ;TEST MAX POS
        RC              ;RETURN IF NO OVER.
        MVI     A,3FH   ;SET EXPONENT AT MAXIMUM
        JMP     OVERR
;
SUBSC   EQU     $
;
;
; COMPUTES SUBSCR ADDR
; INPUT: B HAS ROW NUMBER (1ST SUB)
;        D HAS COL NUMBER (2ND SUB)
;        H HAS ADDR NAME
;
        PUSH    D       ;SAVE COL
        RST     4       ;ADJUST H,L
        DB      -4 AND 0FFH     ;BY FOUR
        MOV     D,M     ;GET HI
        DCX     H       ;POINT LO
        MOV     E,M     ;GET LO
        MOV     A,D     ;GET HI
        CMP     B       ;COMPARE
        JC      SNERR   ;BRIF EXCESS
        JNZ     SUB1    ;BRIF NOT EQUAL
        MOV     A,E     ;GET LO
        CMP     C       ;COMPARE
        JC      SNERR   ;BRIF EXCESS
SUB1:   DCX     H       ;POINT HI COLS
        MOV     D,M     ;LOAD IT
        DCX     H       ;POINT LO COLS
        MOV     E,M     ;LOAD IT
        XTHL            ;SAVE ADDRESS
        PUSH    H       ;SAVE SUB COL
        PUSH    D       ;SAVE DIM COLS
        INX     D       ;MAKE COLS=MAX+1 (ACCOUNT FOR 0 B??KE
        LXI     H,0     ;GET A ZERO
SUB2:   MOV     A,B     ;GET HI
        ORA     C       ;PLUS LO
        JZ      SUB3    ;BRIF ZERO
        DAD     D       ;ADD ONCE
        DCX     B       ;SUB ONCE
        JMP     SUB2    ;LOOP
SUB3:   POP     D       ;GET DIM COL
        POP     B       ;GET SUB COL
        MOV     A,D     ;GET HI
        CMP     B       ;COMPARE
        JC      SNERR   ;BRIF GT
        JNZ     SUB4    ;BRIF NOT ZERO
        MOV     A,E     ;GET LO
        CMP     C       ;COMPARE
        JC      SNERR   ;BRIF GT
SUB4:   DAD     B       ;ADD TO PROD
        DAD     H       ;TIMES TWO
        DAD     H       ;TIMES FOUR
        MOV     A,L     ;GET LOW
        CMA             ;COMPLEMENT
        ADI     1       ;PLUS ONE
        MOV     E,A     ;SAVE IT
        MOV     A,H     ;GET HI
        CMA             ;COMPLEMENT
        ACI     0       ;PLUS CARRY
        MOV     D,A     ;SAVE
        POP     H       ;GET ADDR (0,0)
        DAD     D       ;COMPUTE (I,J) RIGHT SIDE
        RST     4       ;ADJUST H,L
        DB      -4 AND 0FFH
        RET             ;RETURN
FTEST   EQU     $
;
; TEST THE SIGN OF THE NUMBER IN THE FACC
; RETURN WITH S & Z SET TO SIGN
;
        LDA     FACC+1  ;GET MSD
        ORA     A       ;TEST IT
        RZ              ;RETURN IF ZERO
        LDA     FACC    ;GET SIGN&EXPON BYTE
        ORI     7FH     ;TEST SIGN BIT ONLY
        LDA     FACC    ;RE-LOAD EXPON BYTE
        RET             ;THEN RETURN
FEXP    EQU     $
;
; EXPAND EXPONENT INTO 8 BINARY BITS
;
        ANI     7FH     ;MASK MANTISA SIGN
        ADI     40H     ;PROPAGATE CHAR SIGN TO LEFTMOST BIT
        XRI     40H     ;RESTORE ORIGINAL SIGN BIT
        RET             ;RETURN
;
FSUBT   EQU     $
;
; SUBTRACT THE TWO MULTIPRECISION NUMBERS (D,E) & (H,L)
;
        XRA     A       ;TURN OF CY
FSB1:   LDAX    D       ;GET A BYTE
        SBB     M       ;SUB OTHER BYTE
        STAX    D       ;PUT DOWN
        DCX     D       ;POINT NEXT
        DCX     H       ;DITTO
        DCR     B       ;DECR CTR
        JNZ     FSB1    ;LOOP
        RET             ;RETURN
;
; ADD TWO MULTI-PRECISION NUMBERS (D,E) & (H,L)
;
FADT3:  MVI     B,3
FADDT:  XRA     A       ;CLEAR STATUS
FAD1:   LDAX    D       ;GET BYTE
        ADC     M       ;ADD OTHER BYTE
        STAX    D       ;PUT DOWN
        DCX     D       ;POINT NEXT
        DCX     H       ;DITTO
        DCR     B       ;DECR LOOP CTR
        JNZ     FAD1    ;LOOP
        RET             ;RETURN
;
FSHFT   EQU     $
;
; INCREMENTING SHIFT RIGHT
;
        MOV     A,M     ;GET A BYTE
        RAR             ;SHIFT RIGHT
        MOV     M,A     ;PUT DOWN
        INX     H       ;POINT NEXT
        DCR     B       ;DECR CTR
        JNZ     FSHFT   ;LOOP
        RET             ;RETURN
;PAGE
;
TERMI   EQU     $
;
; READ A LINE FROM THE TTY
; FIRST PROMPT WITH THE CHAR IN THE A REG
; TERMINATE THE LINE WITH A X'00'
; IGNORE EMPTY LINES
; CONTROL C WILL CANCLE THE LINE
; CONTROL O WILL TOGGLE THE OUTPUT SWITCH
; RUBOUT WILL DELETE THE LAST CHAR INPUT
;
;
        STA     PROMP   ;SAVE THE PROMPT CHAR
REIN:   LXI     H,IOBUF ;POINT TO INPUT BUFFER
        MVI     M,0     ;MARK BEGIN
        INX     H       ;POINT START
        LDA     PROMP   ;GET THE PROMPT AGAIN
        CALL    TESTO   ;WRITE TO TERMINAL
        CPI     '?'     ;TEST IF Q.M.
        JNZ     TREAD   ;BRIF NOT
        MVI     A,' '   ;GET SPACE
        CALL    TESTO   ;WRITE TO TERMINAL
TREAD   EQU     $
        IF      NOT CPM
        IN      TTY+1   ;GET TTY STATUS
        ANI     2       ;TEST IF RXRDY
        JZ      TREAD   ;LOOP TIL CHAR
        ENDIF
        CALL    GETCH   ;GO READ THE CHAR
        MOV     M,A     ;PUT IN BUFFER
        CPI     0AH     ;TEST IF LINE FEED
        JZ      TREAD   ;IGNORE IF IT IS
        CPI     0DH     ;TEST IF CR
        JNZ     NOTCR   ;BRIF NOT
        LDA     TAPES   ;GET PAPER TAPE SWITCH
        RAR             ;TEST IF LOAD
        CNC     CRLF    ;CR/LF IF NOT
CR1:    MVI     M,0     ;MARK END
        LDA     ILSW    ;GET INPUT LINE SW
        ORA     A       ;TEST IT
        RNZ             ;RETURN IF ON
        DCX     H       ;POINT PRIOR
        MOV     A,M     ;LOAD IT
        CPI     20H     ;TEST IF SPACE
        JZ      CR1     ;BRIF SPACE
        ORA     A       ;TEST IF AT BEGINNING
        JZ      REIN    ;BRIF IS (NULL LINE)
        LXI     H,IOBUF+1       ;POINT BEGIN
        RET             ;ELSE, RETURN
TESTO   EQU     $
        IF      NOT CPM
        PUSH    PSW     ;SAVE CHAR
TEST1:  IN      TTY+1   ;GET STATUS
        RAR             ;TEST IF TXRDY
        JNC     TEST1   ;LOOP TILL READY
        POP     PSW     ;GET CHAR
        OUT     TTY     ;WRITE IT
        ENDIF
        IF      CPM
        PUSH    B       ;BIOS CALLS DESTROYS C,DE
        PUSH    D
        PUSH H
        MOV     C,A     ;OUTPUT BYTE
        CALL    BTOUT   ;CALL BIOS
        POP H
        POP     D       ;RESTORE
        POP     B
        ENDIF
        IF      LARGE   ;SAVE ROOM ONLY IN 8+K VERSIONS
        DB      0,0,0   ;SAVE ROOM FOR CALL TO USER ROUTINE
        ENDIF
        RET             ;RETURN
CRLF:   MVI     A,0DH   ;LOAD A CR
        CALL    TESTO   ;WRITE IT
        MVI     A,0AH   ;LF
        CALL    TESTO   ;WRITE IT
        MVI     A,255   ;GET RUBOUT CHAR
        MVI     B,0FAH  ;LOAD 255-RUBOUT COUNT
PAUZ:   CALL    TESTO   ;SEND RUBOUT
        INR     B       ;INCREMENT COUNT
        CMP     B       ;COMPARE TO 255
        JNZ     PAUZ    ;SET ANOTHER RUBOUT
        XRA     A       ;GET A ZERO
        STA     COLUM   ;RESET COLUMN POINTER
        RET             ;RETURN
NOTCR:  CPI     15H     ;TEST IF CONTROL-U
        JNZ     NOTCO   ;BRIF NOT
        CALL    PRCNT   ;GO PRINT ^U
        CALL    CRLF    ;GET CR/LF
        JMP     REIN    ;GO RE-ENTER
NOTCO:  CPI     7FH     ;TEST IF RUBOUT
        JNZ     NOTBS   ;BRIF NOT
        LDA     TAPES   ;GET PAPER TAPE SW
        RAR             ;TEST IF LOAD
        JC      TREAD   ;IGNORE IF LOAD
        DCX     H       ;POINT PRIOR
        MOV     A,M     ;LOAD PREV CHAR
        ORA     A       ;TEST IF BEGIN
        JZ      ECHO    ;BRIF IS
        MVI     A,'\'   ;BACK SLASH
        CALL    TESTO   ;WRITE IT
        MOV     A,M     ;FETCH CHARACTER TO BE DISCARDED
        CALL    TESTO   ;WRITE IT
        MVI     A,'\'   ;BACK SLASH
        CALL    TESTO   ;WRITE IT
        JMP     TREAD   ;GET REPLACEMENT CHARACTER
NOTBS   EQU     $
        IF      LARGE   ;CONTROL H WORKS ONLY ON 9K VERSION
        CPI     8       ;TEST FOR ASCII BACKSPACE
        JNZ     NOTCH   ;BRIF NOT CONTROL H
        DCX     H       ;POINT PRIOR
        MOV     A,M     ;FETCH CHARACTER
        ORA     A       ;TEST FOR BEGINNING
        JZ      ECHO    ;BRIF IT IS
        PUSH    H       ;SAVE POSITION
        LXI     H,RBOUT ;POINT RUBOUT SEQUENCE
        CALL    TERMM   ;WRITE IT
        POP     H       ;RESTORE H,L
        JMP     TREAD   ;GET REPLACEMENT CHARACTER
        ENDIF
NOTCH:  LDA     TAPES   ;GET PAPER TAPE SWITCH
        RAR             ;FLAG TO CARRY
        JC      ECHO    ;NO ECHO IF TAPE
        MOV     A,M     ;ELSE, LOAD THE CHAR
        CALL    TESTO   ;ECHO THE CHARCTER
ECHO:   INX     H       ;POINT NEXT POSIT
        JMP     TREAD   ;LOOP FOR NEXT
;
TERMO   EQU     $
;
; TTY PRINT ROUTINE
;
; OUTPUT STRING OF CHARS
; STARTING AT IOBUF +0 THRU END (FF OR FE OR 00)
; FOLLOWING IMBEDDED CHARACTERS ARE INTERPRETED AS CONTROLS:
; X'00' END OF BUFFER, TYPE CR/LF AND RETURN
; X'FE' END OF BUFFER, RETURN (NO CR/LF)
; X'FD' TYPE CR/LF, CONTINUE
;
; RETURN WITHOUT OUTPUT IF OUTPUT SW IS OFF
;
        LDA     OUTSW   ;GET OUTPUT SW
        ORA     A       ;TEST IT
        RNZ             ;RETURN IF NO PRINT
        LXI     H,IOBUF ;POINT I/O BUFFER
OT1:    MOV     A,M     ;LOAD A BYTE
        CPI     0FEH    ;SEE IF END OF LINE (NO CR/LF)
        RZ              ;RETURN IF EQUAL
        CPI     0FDH    ;SEE IF IMBEDDED CR/LF
        JNZ     OT2     ;BRIF NOT
        CALL    CRLF    ;LINE FEED
        JMP     OT4     ;CONTINUE
OT2:    ORA     A       ;TEST IF END OF OUTPUT
        JZ      CRLF    ;BRIF IS
        MOV     A,M     ;LOAD THE BYTE
        CALL    TESTO   ;TYPE IT
        LDA     COLUM   ;GET COLUMN POINTER
        INR     A       ;ADD ONE
        STA     COLUM   ;RESTORE IT
OT4:    INX     H       ;POINT NEXT
        JMP     OT1     ;LOOP
TERMM   EQU     OT1
;
TABST   EQU     $
;
;
; POSITION TTY AT NEXT TAB STOP
;
;
        LDA     OUTSW   ;GET OUTPUT SWITCH
        ORA     A       ;TEST IT
        RNZ             ;RETURN IF SUPPRESSED
        LDA     COLUM   ;GET COLUMN POINTER
        CPI     56      ;COMPARE TO 56
        JNC     CRLF    ;BRIF NO ROOM LEFT
        MOV     B,A     ;SAVE IT
        XRA     A       ;INIT POSITION
TBLP:   CMP     B       ;COMPARE
        JZ      TBLP2
        JNC     TBON    ;BRIF SHY OF TAB
TBLP2:  ADI     14      ;POINT NEXT STOP
        JMP     TBLP    ;LOOP
TBON:   STA     COLUM   ;UPDATE CTR
        SUB     B       ;COMPUTE NUMBER OF SPACES
        MOV     B,A     ;SAVE IT
TBSPA:  MVI     A,' '   ;SPACE TO REG A
        CALL    TESTO   ;OUTPUT IT
        DCR     B       ;SUB 1 FROM CTR
        RZ              ;RETURN IF ZERO
        JMP     TBSPA   ;ELSE, LOOP
;
LINEO   EQU     $
;
; UNPACK LINE NUMBER FROM (H,L) TO (D,E)
; ZERO SUPPRESS LEADING ZEROS
;
;
        PUSH    B       ;PUSH B,C
        MVI     B,1     ;SET SWITCH
        CALL    LOUT    ;GO FORMAT 2 BYTES
        CALL    LOUT    ;THEN THE NEXT 2
        POP     B       ;RESTORE B,C
        RET             ;RETURN
;
LOUT    EQU     $
        MOV     A,M     ;GET BYTE
        ANI     0F0H    ;ISOLATE LEFT HALF
        RAR             ;SHIFT RIGHT 1 BIT
        RAR             ;AGAIN
        RAR             ;AGAIN
        RAR             ;LAST TIME
        JNZ     NOTZ1   ;BRIF NOT ZERO
        ORA     B       ;MERGE IN B
        JNZ     Z1      ;BRIF ZERO
NOTZ1:  MVI     B,0     ;RESET SWITCH
        ORI     30H     ;ZONE
        STAX    D       ;PUT TO BUFFER
        INX     D       ;POINT NEXT
Z1:     MOV     A,M     ;LOAD BYTE
        ANI     0FH     ;MASK
        JNZ     NOTZ2   ;BRIF NOT ZERO
        ORA     B       ;MERGE SWITCH
        JNZ     Z2      ;BRIF ZERO
NOTZ2:  MVI     B,0     ;SET SWITCH OFF
        ORI     30H     ;ZONE
        STAX    D       ;PUT TO BUFFER
        INX     D       ;POINT TO NEXT
Z2:     INX     H       ;AND NEXT LINE BYTE
        RET             ;RETURN
;
TSTCC   EQU     $
;
; TEST IF KEY WAS PRESSED DURING EXECUTION
; CANCEL IF CONTROL-C
; TOGGLE OUTPUT SUPPRESS SW IF CONTROL-O
;
        IF      NOT CPM
        IN      TTY+1   ;GET TTY STATUS
        ANI     2       ;MASK FOR RXRDY
        RZ              ;RETURN IF NO CHAR
GETCH:  IN      TTY     ;READ THE CHAR
        ANI     7FH     ;TURN OFF PARITY
        ENDIF
        IF      CPM
        ;NOTE: FOLLOWING CLOBBERS REGISTERS,
        ; PUSH AND POP IF FOUND TO CREATE BUGS.
        CALL    BTSTAT  ;CALL BIOS
        RZ              ;RETURN ON NO CHAR
GETCH:  PUSH    B       ;SAVE REGS - CPM CAN CLOBBER
        PUSH    D
        PUSH    H
        CALL    BTIN    ;CALL BIOS TO INPUT
        POP     H
        POP     D
        POP     B
        ENDIF
        CPI     3       ;TEST IF CONTROL C
        JNZ     TSTC1   ;BRIF NOT
        CALL    PRCNT   ;GO PRINT ^C
        LDA     EDSW    ;GET MODE SW
        ORA     A       ;TEST IT
        JNZ     KEY     ;**;BRIF COMMAND MODE
        LXI     H,STOPM ;POINT MSG
        CALL    TERMM   ;GO PRINT IT
        CALL    PRLIN   ;GO PRINT LINE
        JMP     KEY     ;GOTO READY
TSTC1:  CPI     0FH     ;TEST IF CONTROL O
        RNZ             ;RETURN IF NOT
        CALL    PRCNT   ;GO PRINT ^O
        LDA     OUTSW   ;GET OUTPUT SWTICH
        XRI     1       ;TOGGLE
        STA     OUTSW   ;PUT SW
        RET             ;RETURN
;
PRCNT   EQU     $
;
;
; PRINTS ^ AND CHAR
;
        PUSH    PSW     ;SAVE CHAR
        MVI     A,'^'   ;GET UP ARROW
        CALL    TESTO   ;WRITE IT
        POP     PSW     ;GET CHAR
        ADI     64      ;TRNSLATE
        JMP     TESTO   ;WRITE IT
;PAGE
;
COMP2   EQU     $
;
; CONTINUATION OF COMPARE (RST 2) ROUTINE
;
        ORA     A       ;TEST IT
        JNZ     COMP5   ;BRIF NOT END
COMP3:  XRA     A       ;SET EQUAL STATUS
COMP4:  MOV     A,M     ;GET LAST CHAR
        POP     B       ;RESTORE B,C
        RET             ;RETURN
COMP5:  CMP     M       ;COMPARE THE TWO CHARS
        JZ      COMP6   ;BRIF EQUAL
        MOV     A,B     ;GET COUNT
        CPI     3       ;GET IF >= 3
        JNC     COMP3   ;BRIF NOT LESS THAN 3
        JMP     COMP4   ;BRIF LESS THAN 3 AND NOT EQUAL
COMP6:  INR     B       ;COUNT IT
        INX     D       ;POINT NEXT LIT
        INX     H       ;POINT NEXT VAR
        JMP     COMP1   ;CONTINUE
;
EOL     EQU     $
;
; TESTS IF (H,L) IS END OF LINE
; ERROR-DL IF NOT
;
        RST     1       ;SKIP TO NON-BLANK
        CALL    TSTEL   ;TEST IF END LINE
        JNZ     SNERR   ;ERROR IF NOT
        CPI     ':'     ;TEST FOR MULTIPLE STATEMENT
        JNZ     EOL1    ;BRIF NOT
        STA     MULTI   ;SET SWITCH
EOL1:   INX     H       ;POINT NEXT
        SHLD    ENDLI   ;SAVE POINTER
        RET             ;RETURN
;
TSTEL   EQU     $
;
; TEST (H,L) FOR END OF STATEMENT (00H OR ':')
; RETURN WITH Z SET IF IT IS
;
        ORA     A       ;TEST FOR ZERO
        RZ              ;RETURN IF IS
        CPI     ':'     ;TEST FOR MULTIPLE STATEMENT
        RET             ;RETURN
;
NOTEO   EQU     $
;
;
; TEST IF (H,L) IS END OF LINE
; RETURN IF NOT, ERROR-DL IF IS
;
        RST     1       ;SKIP TO NON-BLANK
        CALL    TSTEL   ;TEST IF END OF LINE
        JZ      SNERR   ;ERROR IF IS
        RET             ;ELSE, RETURN
;
PACK    EQU     $
;
; PACK LINE NUMBER FROM (H,L) TO B,C
;
;
        LXI     B,0     ;CLEAR B AND C
        MVI     A,4     ;INIT DIGIT COUNTER
        STA     PRSW    ;SAVE A
PK1:    MOV     A,M     ;GET CHAR
        CALL    NUMER   ;TEST FOR NUMERIC
        RNZ             ;RETURN IF NOT NUMERIC
        ANI     0FH     ;STRIP OFF ZONE
        MOV     D,A     ;SAVE IT
        LDA     PRSW    ;GET COUNT
        DCR     A       ;SUBTRACT ONE
        JM      SNERR   ;BRIF ERROR
        STA     PRSW    ;SAVE CTR
        MVI     E,4     ;4 BIT SHIFT LOOP
PK3:    MOV     A,C     ;GET LOW BYTE
        RAL             ;ROTATE LEFT 1 BIT
        MOV     C,A     ;REPLACE
        MOV     A,B     ;GET HIGH BYTE
        RAL             ;ROTATE LEFT 1 BIT
        MOV     B,A     ;REPLACE
        DCR     E       ;DECR CTR
        JNZ     PK3     ;LOOP
        MOV     A,C     ;GET LOW
        ORA     D       ;PUT DIGIT IN RIGHT HALF OF BYTE
        MOV     C,A     ;REPLACE
        INX     H       ;POINT NEXT BYTE
        JMP     PK1     ;LOOP
;
SQUIS   EQU     $
;
; COMPRESS THE EXPR STACK
; REG A CONTAINS # OF BYTES TO REMOVE STARTING AT (H,L+1)
; CONTAINS TOTAL NUMBER OF CHARACTERS IN STACK THUS FAR
;
        PUSH    H       ;SAVE H,L
        MOV     E,A     ;COUNT TO E
        MVI     D,0     ;ZERO HI BYTE
        DAD     D       ;COMPUTE START
        XCHG            ;PUT TO D,E
        POP     H       ;GET H,L
        CMA             ;COMPLEMENT COUNT
        INR     A       ;THEN 2'S COMPLEMENT
        ADD     B       ;COMPUTE B-A
        MOV     B,A     ;PUT TO B
SQUI2:  INX     D       ;POINT NEXT SEND
        INX     H       ;POINT NEXT RECEIVE
        LDAX    D       ;GET A CHAR
        MOV     M,A     ;PUT IT DOWN
        DCR     B       ;DECR CTR
        JNZ     SQUI2   ;LOOP
        SHLD    EXPRS   ;UPDATE NEW START OF EXPR
        RET             ;RETURN
;
SKP2Z   EQU     $
;
; FIND END OF LITERAL IN (D,E)
;
        LDAX    D       ;GET BYTE OF LIT
        ORA     A       ;TEST IT
        RZ              ;RETURN IF ZERO (END)
        INX     D       ;ELSE, POINT NEXT
        JMP     SKP2Z   ;LOOP
;
GTEMP   EQU     $
;
; GETS FOUR BYTE TEMPORARY STORAGE AREA,
; STORES THE FACC THERE,
; PUTS ADDR OF AREA IN EXPR STACK (H,L)
;
        XCHG            ;SAVE H,L IN D,E
        XTHL            ;EXCHANGE 0 AND RET ADDR
        PUSH    H       ;PUT NEW RET ADDR
        PUSH    H       ;DOIT IT AGAIN
        LXI     H,0     ;ZERO H,L
        DAD     SP      ;GET SP ADDR IN H,L
        INX     H       ;PLUS ONE
        INX     H       ;PLUS ONE MORE (POINT TO NEW AREA)
        PUSH    B       ;SAVE CTRS
        PUSH    D       ;SAVE EXPR ADDR
        PUSH    H       ;SAVE TEMP ADDR
        RST     3       ;GO STORE FACC
        POP     D       ;RESTORE TEMP ADDR
        LHLD    SPCTR   ;GET COUNT
        INX     H       ;PLUS ONE
        INX     H       ;ONE MORE
        SHLD    SPCTR   ;PUT BACK
        POP     H       ;RESTORE EXPR ADDR
        POP     B       ;RESTORE CTRS
SADR:   INX     H       ;POINT NEXT BYTE
        MOV     M,D     ;HIGH BYTE TO EXPRSTK
        INX     H       ;POINT NEXT
        MOV     M,E     ;LOW BYTE TO EXPR STK
        INX     H       ;POINT NEXT
        MVI     M,0E3H  ;CODE = NUMERIC DATA
        RET             ;RETURN
;
ALPHA   EQU     $
;
; TESTS THE CHAR AT (H,L)
; RETURNS WITH Z SET IF CHAR IS ALPHA (A-Z)
; RETURNS WITH Z OFF IF NOT ALPHA
; CHAR IS LEFT IN REG A
;
        MOV     A,M     ;PUT CHAR TO REG A
        CPI     'A'     ;TEST IF A OR HIGHER
        RC              ;RETURN IF NOT ALPHA (Z IS OFF)
        CPI     'Z'     ;TEST IF Z OR LESS
        JMP     NUMEN   ;GO WRAPUP
;
NUMER   EQU     $
;
; TESTS THE CHAR AT (H,L)
; RETURNS WITH Z SET IF NUMERIC (0-9)
; ELSE Z IS OFF
; CHAR IS LEFT IN THE A REG
;
        MOV     A,M     ;GET CHAR TO REG A
        CPI     '0'     ;TEST IF ZERO OR GREATER
        RC              ;RETURN IF LESS THAN ZERO
        CPI     '9'     ;TEST IF 9 OR LESS
NUMEN:  RZ              ;RETURN IF 9
        RNC             ;RETURN IF NOT NUMERIC
        CMP     A       ;SET Z
        RET             ;RETURN
;
SEARC   EQU     $
;
; SEARCHES FOR THE VARIABLE IN D,E
; RETURNS WITH ADDR OF DATA AREA FOR VARIABLE
;
        PUSH    H       ;SAVE H,L
        LDA     FNMOD   ;GET FUNCTION MODE
        ORA     A       ;TEST IT
        JNZ     SCH6    ;BRIF IN A FUNCTION
SCH0:   LHLD    DATAB   ;GET ADDR OF DATA POOL
SCH1:   MOV     A,M     ;GET THE BYTE
        ORA     A       ;TEST IF END
        JZ      SCH3    ;BRIF END
        DCX     H       ;POINT NEXT
        DCX     H       ;DITTO
        MOV     B,M     ;GET HI LEN
        DCX     H       ;POINT NEXT
        MOV     C,M     ;GET LO LEN
        RST     4       ;ADJUST H,L
        DB      3
        MOV     A,M     ;LOAD 1ST CHAR
        CMP     D       ;COMPARE 1ST CHAR
        JNZ     SCH2    ;BRIF NOT EQUAL
        DCX     H       ;POINT NEXT
        MOV     A,M     ;LOAD 2ND DIGIT
        INX     H       ;POINT BACK
        CMP     E       ;COMPARE 2ND CHAR
        JNZ     SCH2    ;BRIF NOT EQUAL
        MOV     A,D     ;GET HI NAME
        ORA     A       ;TEST IT
        JM      SCH9    ;RETURN IF MATRIX
        DAD     B       ;POINT NEXT ENTRY
        INX     H       ;PLUS ONE
        XCHG            ;FLIP/FLOP
        POP     H       ;RESTORE H
        RET             ;RETURN
SCH2:   DAD     B       ;MINUS LEN
        JMP     SCH1    ;LOOP
SCH3:   MOV     M,D     ;PUT 1ST CHAR
        DCX     H       ;POINT NEXT
        MOV     M,E     ;PUT 2ND CHAR
        DCX     H       ;POINT NEXT
        MOV     A,D     ;GET HI NAME
        ORA     A       ;TEST IT
        JM      SCH7    ;BRIF ARRAY
        MVI     M,0FFH  ;HI LEN
        DCX     H       ;POINT NEXT
        MOV     A,E     ;GET LO NAME
        ORA     A       ;TEST TYPE
        JM      SCH4    ;BRIF CHAR
        MVI     M,0F8H  ;LO LEN
        MVI     B,4     ;LOOP CTR
        JMP     SCH5    ;BRARND
SCH4:   MVI     M,0FBH  ;LO LEN
        MVI     B,1     ;LOOP CTR
SCH5:   DCX     H       ;POINT NEXT
        MVI     M,0     ;ZERO THE VALUE
        DCR     B       ;DECR CTR
        JNZ     SCH5    ;LOOP
        DCX     H       ;POINT NEXT
        MVI     M,0     ;MARK NEW END
        INX     H       ;POINT ADDR OF VARIABLE
        XCHG            ;PUT LOCATION TO D,E
        POP     H       ;RESTORE H,L
        RET             ;RETURN
SCH6:   LXI     H,FNARG ;POINT DUMMY ARG
        MOV     A,M     ;LOAD 1ST CHAR
        CMP     D       ;COMPARE
        JNZ     SCH0    ;BRIF NOT EQUAL
        INX     H       ;POINT NEXT
        MOV     A,M     ;LOAD 2ND CHAR
        CMP     E       ;COMPARE
        JNZ     SCH0    ;BRIF NOT EQUAL
        INX     H       ;POINT NEXT
        MOV     D,M     ;GET HI ADDR
        INX     H       ;POINT NEXT
        MOV     E,M     ;GET LO ADDR
        POP     H       ;RESTORE H,L
        RET             ;RETURN
SCH7:   PUSH    H       ;SAVE ADDRESS
        MVI     M,0FEH  ;MOVE HI DISP
        DCX     H       ;POINT NEXT
        MVI     M,14H   ;MOVE LO DISP
        DCX     H
        MVI     M,0     ;MOVE A ZERO
        DCX     H       ;POINT NEXT
        MVI     M,10    ;MOVE 10
        DCX     H       ;POINT NEXT
        MVI     M,0     ;MOVE A ZERO
        DCX     H       ;POINT NEXT
        MVI     M,10    ;MOVE A 10 (DEFAULT IS 10 X 10)
        LXI     B,485   ;TOTAL # OF BYTES TAKEN BY ARRAY
SCH8:   DCX     H       ;POINT NEXT
        MVI     M,0     ;CLEAR ONE BYTE
        DCX     B       ;DCR CTR
        MOV     A,B     ;GET HI
        ORA     C       ;PLUS LO
        JNZ     SCH8    ;LOOP
        POP     H       ;RESTORE PTR TO START
        INX     H       ;POINT LO NAME
        INX     H       ;POINT HI NAME
SCH9:   POP     B       ;NEED TO XCHANGE LAST 2 STACK ENTRIES
        POP     D       ;SO DOIT
        PUSH    B
        PUSH    D
        RET             ;RETURN
;
VAR     EQU     $
;
;
; TEST (H,L) FOR A VARIABLE NAME
; PUTS THE NAME IN D,E IF FOUND
; ERROR SN IF NONE FOUND
;
        RST     1       ;SKIP TO NON-BLANK
        CALL    ALPHA   ;TEST IF ALPHA
        JNZ     SNERR   ;BRIF NOT ALPHA
        MOV     D,A     ;FIRST CHAR
        MVI     E,' '   ;DEFAULT
        INX     H       ;POINT NEXT
        RST     1       ;GET 2ND CHAR
        CALL    NUMER   ;TEST IF NUMERIC
        JNZ     VAR2    ;BRIF NOT NUMERIC
        MOV     E,A     ;SAVE 2ND CHAR
        INX     H       ;POINT NEXT
        RST     1       ;GET NON-BLANK FOLLOWING
VAR2:   CPI     '$'     ;TEST IF STRING
        JNZ     VAR3    ;BRIF NOT
        MOV     A,E     ;GET 2ND CHAR
        ORI     80H     ;SET TYPE
        MOV     E,A     ;SAVE IT
        INX     H       ;SKIP $
        RET             ;THEN RETURN
VAR3:   CPI     '('     ;TEST IF ARRAY
        RNZ             ;RETURN IF NOT
        MOV     A,D     ;GET HI NAME
        ORI     80H     ;TURN ON D7
        MOV     D,A     ;RESTORE
        RET             ;RETURN
;
PRLIN   EQU     $
;
; PRINTS LINE NUMBER FOLLOWED BY CR/LF
;
        LXI     D,LINEN ;POINT AREA
        LHLD    LINE    ;GET ADDR OF LINE NUMBER
        CALL    LINEO   ;GO UNPACK
        XCHG            ;PUT TO H,L
        MVI     M,0     ;END OF MSG
        LXI     H,LINEN ;POINT AREA
        JMP     TERMM   ;GO PRINT IT
;PAGE
;
; ERROR MESSAGE ROUTINES
; FATAL ERROR MUST BE FIRST
;
EM      EQU     0FEH
;
ULERR:  RST     6
        DB      'UL',EM,FATAL   ;NOTE FATAL = CODE FOR RST 6
ZMERR   EQU     $-1             ;LOG(X<=0),SQR(-X),0 DIVIDE
        DB      'OF',EM,FATAL
STERR   EQU     $-1             ;ERROR IN EXPRESSION STACK
        DB      'ST',EM,FATAL
SNERR   EQU     $-1             ;DELIMITER ERROR
        DB      'SN',EM,FATAL
RTERR   EQU     $-1             ;RETURN & NO GOSUB
        DB      'RT',EM,FATAL
DAERR   EQU     $-1             ;OUT OF DATA
        DB      'DA',EM,FATAL
NXERR   EQU     $-1             ;NEXT & NO FOR / >8 FOR'S
        DB      'NX',EM,FATAL
CVERR   EQU     $-1             ;CONVERSION ERROR
        DB      'CV',EM,FATAL
CKERR   EQU     $-1             ;CHECKSUM ERROR
        DB      'CK',EM,FATAL
;
; NON-FATAL ERRORS
;
OVERR   EQU     $-1             ;OVERFLOW ERROR
        DB      'OV',EM
        RET                     ;RETURN TO ROUTINE
UNERR:  RST     6               ;CALL   ERROR ROUTINE
        DB      'UN',EM
        RET
;
; CONTINUATION OF ERROR MESSAGE ROUTINE (RST 6)
;
ERROR:  CALL    TERMM   ;PRINT 'XX'
        PUSH    H       ;SAVE RETURN
        LXI     H,ERRMS ;PRINT 'ERROR IN LINE'
        CALL    TERMM
        CALL    PRLIN   ;PRINT LINE #
        POP     H
        INX     H       ;RETURN ADDRESS
        MOV     A,M     ;GET INSTRUCTION
        CPI     FATAL   ;IS IT AN RST 6?
        JZ      KEY     ;IF ZERO, YES, ABORT
        POP     B       ;RESTORE REGISTERS
        POP     D
        POP     PSW
        XTHL
        RET
        ;PAGE
;
;
; MOVE THE STRING FROM (D,E) TO (H,L) COUNT IN B
;
;
CPY4D:  MVI     B,4
COPYD:  LDAX    D       ;GET A BYTE
        MOV     M,A     ;MOVE IT
        INX     H       ;POINT NEXT
        INX     D       ;DITTO
        DCR     B       ;DECR CTR
        JNZ     COPYD   ;LOOP
        RET             ;THEN RETURN
;
;
; MOVE THE STRING FROM (H,L) TO (D,E) COUNT IN B
;
;
CPY4H:  MVI     B,4
COPYH:  XCHG            ;FLIP/FLOP
        CALL    COPYD   ;GO COPY
        XCHG            ;FLIP/FLOP BACK
        RET             ;RETURN
;
ZEROM   EQU     $
;
; MOVES A STRING OF BINARY ZEROS, COUNT IN B
;
        MVI     M,0     ;MOVE A ZERO
        INX     H       ;POINT NEXT
        DCR     B       ;DECR CTR
        JNZ     ZEROM   ;LOOP
        RET             ;RETURN
;
FBIN    EQU     $
;
;
; CONVERT FLOAT ACC TO UNSIGNED BINARY NUMBER IN A REG
; RETURNS 0 IN A REG IF FACC<0 OR FACC>255
;
;
        PUSH    H       ;SAVE H,L
        PUSH    D       ;SAVE D,E
        CALL    FACDE   ;CONVERT FACC TO D,E
        XRA     A       ;ZERO A
        ORA     D       ;TEST HIGH VALUE
        JNZ     FBIN1   ;BRIF NOT ZERO
        MOV     A,E     ;VALUE TO A
FBIN1:  POP     D       ;RESTORE D,E
        POP     H       ;RESTORE H,L
        RET             ;RETURN
;
ARG     EQU     $
;
; GET NEXT ARGUMENT FROM POLISH STACK
;
        LHLD    ADDR1   ;GET ADDRESS
        INX     H       ;POINT NEXT
        MOV     D,M     ;GET HI ADDRESS
        INX     H       ;POINT NEXT
        MOV     E,M     ;GET LO ADDRESS
        INX     H       ;POINT TYPE
        SHLD    ADDR1   ;GET ADDRESS
        DCX     H       ;POINT BACK
        JMP     EVLD    ;CALL EVLOAD AND RETURN
;
;
ARGNU   EQU     $
;
        CALL    ARG     ;GET ARGUMENT
        JMP     FBIN    ;THEN CONVERT FACC TO BIN
;
BINFL   EQU     $
;
; CONVERT D,E TO FLOATING POINT NUMBER IN FAC
;
;
        LXI     H,FACC  ;POINT ACC
        MVI     M,24    ;MAX BITS
        INX     H       ;POINT NEXT
        MVI     M,0     ;CLEAR MSB
        INX     H       ;POINT NEXT
        MOV     M,D     ;MOVE MID
        INX     H       ;POINT NEXT
        MOV     M,E     ;MOVE LSB
        JMP     FNORM   ;GO NORMALIZE & RETURN
;PAGE
;
; FUNCTION TABLE. FORMAT IS:
;     DB <LITERAL>,0
;     DW <ADDRESS>
;     DB <FUNCTION TYPE>
;
; TABLE IS TERMINATED WITH A '00'
;
FUNCT   EQU     $
        DB      'ABS',0
        DW      ABS
        DB      0ABH
        DB      'SQR',0
        DW      SQR
        DB      0ABH
        DB      'INT',0
        DW      INT
        DB      0ABH
        DB      'SGN',0
        DW      SGN
        DB      0ABH
RNDLI:  DB      'RND',0
        DW      RND
        DB      0ABH
        DB      'SIN',0
        DW      SIN
        DB      0ABH
        DB      'COS',0
        DW      COS
        DB      0ABH
        DB      'TAN',0
        DW      TAN
        DB      0ABH
        DB      'ATN',0
        DW      ATN
        DB      0ABH
        DB      'INP',0
        DW      INP
        DB      0ABH
        DB      'LN',0
        DW      LN
        DB      0ABH
        DB      'LOG',0
        DW      LOG
        DB      0ABH
        DB      'EXP',0
        DW      EXP
        DB      0ABH
        DB      'POS',0
        DW      POS
        DB      0ABH
        DB      'LEN',0
        DW      LENFN
        DB      0ABH
        DB      'CHR$',0
        DW      CHRFN
        DB      0CBH
        DB      'ASCII',0
        DW      ASCII
        DB      0ABH
        DB      'NUM$',0
        DW      NUMFN
        DB      0CBH
        DB      'VAL',0
        DW      VAL
        DB      0ABH
        DB      'SPACE$',0
        DW      SPACE
        DB      0CBH
        DB      'STRING$',0
        DW      STRFN
        DB      0D3H
        DB      'LEFT$',0
        DW      LEFT
        DB      0D3H
        DB      'RIGHT$',0
        DW      RIGHT
        DB      0D3H
        DB      'MID$',0
        DW      MIDFN
        DB      0DBH
        DB      'INSTR',0
        DW      INSTR
        DB      0BBH
        DB      'PEEK',0
        DW      PEEK
        DB      0ABH
        IF      LARGE
        DB      0,0,0,0 ;ROOM FOR ONE MORE FUNCTION
        DB      0,0,0,0
        ENDIF
        DB      0       ;END OF FUNCTION TABLE
;PAGE
;
; PROGRAM CONSTANTS
;
PCHOF:  DB      19,20,0
RNDP:   DB      3FH,0FDH        ;16381
        DB      3FH,0EBH        ;16363
        DB      3FH,0DDH        ;16349
NRNDX:  DB      1BH,0ECH
        DB      33H,0D3H
        DB      1AH,85H
        DB      2BH,1EH
WHATL:  DB      'WHAT',0
VERS    EQU     $       ;VERSION MESSAGE
        IF      LARGE
        DB      '9K VERS 1.4',0
RBOUT:  DB      08H,20H,08H,0FEH ;RUBOUT SEQUENCE (9K ONLY)
        ENDIF
        IF      NOT LARGE
        DB      '8K VERS 1.4',0
        ENDIF
LLINE:  DB      'LINE',0
TABLI:  DB      'TAB',0
STEPL:  DB      'STEP',0
THENL:  DB      'THEN',0
PILIT:  DB      'PI',0
TWO:    DB      02H,80H,00H,00H    ;CONSTANT:  2
TEN:    DB      04H,0A0H,00H,00H   ;CONSTANT:  10
PI:     DB      02H,0C9H,0FH,0D7H  ;CONSTANT:  3.141593
QTRPI:  DB      00H,0C9H,0FH,0D7H  ;CONSTANT:  0.7853892
NEGON:  DB      80H,0FFH,0FFH,0FFH ;CONSTANT: -0.9999999
LN2C:   DB      00H,0B1H,72H,16H   ;CONSTANT:  0.6931472
SQC1:   DB      00H,97H,14H,0EBH   ;CONSTANT:  0.59016206
SQC2:   DB      7FH,0D5H,0A9H,56H  ;CONSTANT:  0.41730759
;PAGE
;
; THE FOLLOWING CONSTANTS MUST BE IN THIS ORDER ***********
;
;       CONSTANT WITH EXPONENT OF 1
;       COEFFICIENT OF FIRST TERM
;       ...
;       COEEFICIENT OF NTH TERM
;
; SINCE ALL COEFFICIENTS ARE LESS THAN 1,
; THE ITERATION LOOP USES THE
; CONSTANT WITH EXPONENT 1 TO TERMINATE THE EVALUATION.
;
SQC3:   DB      01H,0B5H,04H,0F3H    ;CONSTANT:  1.41421356
        DB      0FFH,0AAH,95H,0BCH   ;CONSTANT: -0.3331738
        DB      7EH,0CAH,0D5H,20H    ;CONSTANT:  0.1980787
        DB      0FEH,87H,82H,0D6H    ;CONSTANT: -0.1323351
        DB      7DH,0A3H,13H,1CH     ;CONSTANT:  0.07962632
        DB      0FCH,89H,0A6H,0B8H   ;CONSTANT: -0.03360627
ATNCO:  DB      79H,0DFH,3AH,9EH     ;CONSTANT:  0.006812411
;
HALFP:  DB      01H,0C9H,0FH,0D7H    ;CONSTANT:  1.570796
        DB      80H,0A5H,5DH,0DEH    ;CONSTANT: -0.64596371
        DB      7DH,0A3H,34H,55H     ;CONSTANT:  0.076589679
        DB      0F9H,99H,38H,60H     ;CONSTANT: -0.0046737656
SINCO:  DB      74H,9EH,0D7H,0B6H    ;CONSTANT:  0.00015148419
;
ONE:    DB      001H,080H
NULLI:  DB      00H,00H              ;CONSTANT:  1.0
        DB      00H,0FFH,0FEH,0C1H   ;CONSTANT:  0.99998103
        DB      0FFH,0FFH,0BAH,0B0H  ;CONSTANT: -0.4994712
        DB      7FH,0A8H,0EH,2BH     ;CONSTANT:  0.3282331
        DB      0FEH,0E7H,4BH,55H    ;CONSTANT: -0.2258733
        DB      7EH,89H,0DEH,0E3H    ;CONSTANT:  0.134693
        DB      0FCH,0E1H,0C5H,078H  ;CONSTANT: -0.05511996
LNCO:   DB      7AH,0B0H,3FH,0AEH    ;CONSTANT:  0.01075737
;
LN2E:   DB      001H,0B8H,0AAH,03BH  ;CONSTANT:  1.44269504
        DB      000H,0B1H,06FH,0E6H  ;C=.69311397
        DB      07EH,0F6H,02FH,070H  ;C=.24041548
        DB      07CH,0E1H,0C2H,0AEH  ;C=.05511732
        DB      07AH,0A0H,0BBH,07EH  ;C=.00981033
EXPCO:  DB      077H,0CAH,009H,0CBH  ;C=.00154143
;
LNC:    DB      07FH,0DEH,05BH,0D0H     ;C=LOG BASE 10 OF E
READY   EQU     $
        DB      0FDH
        DB      'READY',0
STOPM   EQU     $
        DB      0FDH
        DB      'STOP AT LINE ',254
ERRMS:  DB      ' ERROR IN LINE ',0FEH
TTY     EQU     2
;PAGE
;
; VERB (STATEMENT/COMMAND) TABLE
; FORMAT IS: DB 'VERB',0
;            DW ADDR
;            DB 'NEXT VERB',0
;            ETC
;  END OF TABLE IS MARKED BY DB 0
;
JMPTB   EQU     $
        DB      'LIST',0
        DW      LIST
        DB      'RUN',0
        DW      RUNCM
        DB      'XEQ',0
        DW      XEQ
        DB      'NEW',0
        DW      NEW
        DB      'CON',0
        DW      CONTI
        DB      'TAPE',0
        DW      TAPE
        DB      'SAVE',0
        DW      SAVE
KEYL:   DB      'KEY',0
        DW      KEY
        DB      'FRE',0
        DW      FREE
        DB      'IF',0
        DW      IFSTM
        DB      'READ',0
        DW      READ
        DB      'RESTORE',0
        DW      RESTO
DATAL:  DB      'DATA',0
        DW      RUN
        DB      'FOR',0
        DW      FOR
NEXTL:  DB      'NEXT',0
        DW      NEXT
GOSBL:  DB      'GOSUB',0
        DW      GOSUB
        DB      'RETURN',0
        DW      RETUR
        DB      'INPUT',0
        DW      INPUT
        DB      'PRINT',0
        DW      PRINT
GOTOL:  DB      'GO'
TOLIT:  DB      'TO',0
        DW      GOTO
        DB      'LET',0
        DW      LET
        DB      'STOP',0
        DW      STOP
        DB      'END',0
        DW      ENDIT
        DB      'REM',0
        DW      RUN
        DB      '!',0
        DW      RUN
        DB      '?',0
        DW      PRINT
        DB      'RANDOMIZE',0
        DW      RANDO
        DB      'ON',0
        DW      ON
        DB      'OUT',0
        DW      OUTP
        DB      'DIM',0
        DW      DIM
        DB      'CHANGE',0
        DW      CHANG
DEFLI:  DB      'DEF'
FNLIT:  DB      'FN',0
        DW      RUN
        IF      CPM
        DB      'DDT',0
        DW      DDT
        DB      'BYE',0
        DW      BOOT
        ENDIF
        DB      'POKE',0
        DW      POKE
        DB      'CALL',0
        DW      JUMP
        IF      LARGE   ;INCLUDE ONLY IN 8K+ VERSION
        DB      'EDIT',0
        DW      FIX
        DB      'CLOAD',0
        DW      CLOAD
        DB      'CSAVE',0
        DW      CSAVE
        ENDIF
        IF      HUNTER
        DB      'BAUD',0
        DW      BAUD
        ENDIF
        DB      0       ;END OF TABLE
;
; DDT COMMAND, CPM ONLY
;
        IF      CPM
DDT:    RST     7
        JMP     RDY
        ENDIF
;PAGE
;
FACDE   EQU     $
;
; THIS ROUTINE CONVERTS THE FACC TO AN ADDRESS IN D,E
;
        CALL    INT     ;INTEGERIZE THE FACC
        LDA     FACC    ;GET THE EXPONENT
        ORA     A       ;TEST IT
        JM      OVERR   ;BRIF NEGATIVE ADDRESS
        SUI     16      ;SUBTRACT MAX EXPONENT
        JZ      FDE2    ;BRIF EQUAL MAX
        JP      OVERR   ;BRIF GREATER THAN 64K
        CMA             ;2'S COMPLIMENT OF A YIELDS..
        INR     A       ;16-A
        MOV     C,A     ;SAVE SHIFT COUNT
FDE1:   XRA     A       ;CLEAR CARRY
        LXI     H,FACC+1        ;POINT MANTISSA
        MVI     B,2     ;WORDS TO SHIFT
        CALL    FSHFT   ;GO SHIFT FACC+1 AND FACC+2
        DCR     C       ;REDUCE COUNT
        JNZ     FDE1    ;LOOP TILL COMPLETE
FDE2:   LXI     H,FACC+1        ;POINT HIGH BYTE
        MOV     D,M     ;LOAD D
        INX     H       ;POINT LOW BYTE
        MOV     E,M     ;LOADE E
        RET             ;RETURN
;
;
LOCAT   EQU     $
;
; THIS ROUTINE SEARCHES FOR A LINE IN THE PROGRAM FILE.
; Z SET, C RESET==>LINE FOUND. ADDRESS IS IN H,L
; C SET, Z RESET==>NOT FOUND. H,L POINT TO NEXT LINE
; C SET, Z SET==>NOT FOUND. H,L POINT AT END OF PROGRAM
;
        LXI     H,BEGPR ;POINT START
FIND1:  MOV     A,M     ;FETCH LENGTH OF LINE
        PUSH    H       ;SAVE POINTER
        ORA     A       ;TEST
        JZ      FIND3   ;BRIF END
        INX     H       ;POINT LINE #
        MOV     A,M     ;FETCH HI #
        CMP     B       ;COMPARE TO REQUESTED
        JC      FIND2   ;BRIF LOW
        JNZ     FIND3   ;BRIF PAST AND NOT FOUND
        INX     H       ;POINT LO #
        MOV     A,M     ;FETCH IT
        CMP     C       ;COMPARE TO REQUESTED
        JC      FIND2   ;BRIF LOW
        JNZ     FIND3   ;BRIF PAST AND NOT FOUND
        POP     H       ;POINT BEGIN IF MATCH
        RET             ;RETURN
;
; BUMP H,L TO NEXT LINE
;
FIND2:  POP     H       ;POINT START OF LINE
        MOV     E,M     ;LENGHT TO E
        MVI     D,0     ;CLEAR D
        DAD     D       ;BUMP H,L
        JMP     FIND1   ;CONTINUE
;
; LINE NOT FOUND
;
FIND3:  STC             ;SET CARRY
        POP     H       ;POINT LINE JUST PAST REQUESTED
        RET             ;RETURN
;
;
SEEK    EQU     $
;
;  THIS CODE FINDS AN ENTRY IN THE TABLE POINTED TO BY D,E.
;  THE SOUGHT ENTRY IS POINTED TO BY H,L.
;
SEEK1:  PUSH    H       ;SAVE ADDRESS OF STRING
        LDAX    D       ;GET BYTE FROM TABLE
        ORA     A       ;TEST IT
        JZ      SEEK3   ;BRIF END OF TABLE
        RST     2       ;COMPARE
        JNZ     SEEK2   ;BRIF NOT FOUND
        XTHL            ;PUT CURRENT H,L ON STACK
        CALL    SKP2Z   ;FIND END TO LITERAL IN TABLE
        INX     D       ;POINT LOW BYTE
        POP     H       ;RESTORE LINE POINTER
        INR     A       ;PUT 1 IN A
        ORA     A       ;RESET Z BIT
        RET             ;RETURN
SEEK2:  CALL    SKP2Z   ;FIND END OF TABLE LITERAL
        INX     D       ;
        INX     D       ;POINT NEXT LIT IN TABLE
        INX     D       ;
        POP     H       ;GET ORIGINAL STRING
        LDAX    D       ;GET BYTE
        RAL             ;HIGH BIT TO CARRY
        JNC     SEEK1   ;NOT A FUNCTION SEARCH
        INX     D       ;POINT NEXT BYTE IN FUNCTION TABLE
        JMP     SEEK1   ;CONTINUE SEARCH
SEEK3:  POP     H       ;RESTORE ORIGINAL STRING
        RET             ;RETURN
        IF      LARGE   ;ASSEMBLE THE REMAINDAR ONLY FOR 8+K
;
;
; EDIT COMMAND
; EDIT <LINE #><DELIMITER><OLD TEXT><DELIMITER><NEW TEXT>
;
FIX:    EQU     $
        RST     1       ;SKIP BLANKS
        CALL    PACK    ;GET LINE # IN B,C
        RST     1       ;SKIP BLANKS
        SHLD    ADDR2   ;SAVE COMMAND POINTER
        CALL    LOCAT   ;SEARCH FOR LINE # IN PROGRAM
        JC      ULERR   ;BRIF NOT FOUND
        PUSH    H       ;SAVE ADDR OF EXISTING LINE <SOURCE>
        PUSH    B       ;SAVE LINE #
        MOV     B,M     ;GET LENGTH OF <SOURCE>
        XCHG            ;D,E POINT <SOURCE>
        LXI     H,STRIN ;POINT STRING BUFFER
        CALL    COPYD   ;<SOURCE> TO STRING BUFFER
        LDA     STRIN   ;LENGTH OF <SOURCE> TO A
        SUI     2       ;ADJUST
        STA     STRIN   ;STORE
        LXI     D,IOBUF+1       ;POINT BUFFER
        LHLD    ADDR2   ;FETCH COMMAND POINTER
        MOV     B,M     ;FETCH <DELIMITER>
;
; FIND LENGTH OF <OLD TEXT>. STORE IT IN IOBUF.
;
        MVI     C,0     ;INITIAL LENGTH
FIX1:   INX     H       ;POINT NEXT CHARACTER
        MOV     A,M     ;FETCH
        ORA     A       ;TEST
        JZ      SNERR   ;MISSING 2ND <DELIMITER>.
        CMP     B       ;TEST
        JZ      FIX2    ;BRIF 2ND <DELIMITER> FOUND
        INR     C       ;ELSE, BUMP C
        STAX    D       ;STORE CHARACTER IN IOBUF
        INX     D       ;BUMP IOBUF POINTER
        JMP     FIX1    ;CONTINUE
;
; GET READY TO SEARCH <SOURCE> FOR <OLD TEXT>
;
FIX2:   MOV     A,C     ;LENGTH OF <OT> TO A
        STA     IOBUF   ;STORE
        SHLD    ADDR2   ;SAVE COMMAND POINTER
        MVI     A,3     ;SEARCH WILL START IN POS 3.
        LHLD    PROGE   ;POINT END OF PROGRAM
        INX     H       ;BUMP TWICE
        INX     H
        SHLD    ADDR1   ;SAVE EXPR. STACK POINTER
        INX     H       ;POINT NEXT
        LXI     D,IOBUF ;POINT BUFFER AREA
        MOV     M,D     ;STORE ADDRESS
        INX     H
        MOV     M,E
        LXI     H,STRIN ; POINT <SOURCE>
;
; USE THE INSTR ROUTINE TO SEARCH
;
        CALL    INST2   ;GO SEARCH
        MOV     A,E     ;RESULT TO A
        ORA     A       ;TEST
        JZ      DAERR   ;BR IF NOT FOUND
        MOV     C,A     ;SAVE POSITION IN C
        DCR     A       ;ADJUST
        MOV     B,A     ;COPY TO B
        LXI     H,STRIN+1       ;POINT <OLD SOURCE>
        LXI     D,IOBUF+1       ;PIONT <NEW LINE AREA>
        CALL    COPYH   ;COPY <OLD SOURCE> UP TO <OLD TEXT>
        PUSH    D       ;SAVE DEST POINTER
;
; SKIP OVER <OLD TEXT> IN <SOURCE>
;
        MVI     D,0     ;CLEAR D
        LDA     IOBUF   ;GET LENGTH OF <OT>
        MOV     E,A     ;LENGTH TO E
        DAD     D       ;BUMP H,L PAST <OT>
        POP     D       ;RESTORE <DEST> POINTER
        PUSH    H       ;SAVE <REMAINING SOURCE> POINTER
;
; APPEND <NEW TEXT> TO <DEST>
;
        LHLD    ADDR2   ;FETCH COMMAND POINTER
FIX3:   INX     H       ;POINT NEXT
        MOV     A,M     ;FETCH CHARACTER
        ORA     A       ;TEST IT
        JZ      FIX4    ;BRIF NO MORE <NEW TEXT>
        INR     C       ;BUMP LENGTH COUNT
        STAX    D       ;STORE CHARACTER
        INX     D       ;BUMP <DEST> POINTER
        JMP     FIX3    ;CONTINUE
;
; APPEND <REMAINING SOURCE> TO <DEST>
;
FIX4:   POP     H       ;GET REMAINING SOURCE POINTER
FIX4A:  MOV     A,M     ;FETCH CHARACTER
        ORA     A       ;TEST
        JZ      FIX5    ;BRIF DONE
        STAX    D       ;STORE CHARACTER
        INR     C       ;BUMP CHAR COUNT
        INX     D       ;BUMP DEST POINTER
        INX     H       ;BUMP <SOURCE> POINTER
        JMP     FIX4A   ;CONTINUE
;
; PREPARE <DEST> FOR SUBMISSION AS NEW LINE
;
FIX5:   STAX    D       ;BUFFER TERMINATOR
        INR     C       ;BUMP LENGTH COUNT
        MOV     A,C     ;FETCH COUNT
        STA     IOBUF   ;STORE IT
        MOV     B,A     ;COPY COUNT TO B
        LXI     H,IMMED ;POINT NEW LINE AREA
        LXI     D,IOBUF ;POINT WHERE IT IS NOW
        CALL    COPYD   ;COPY IT
        POP     B       ;RESTORE LINE #
        POP     H       ;RESTORE PROGRAM POINTER
        PUSH    H       ;SAVE IT
        JMP     EDIT2   ;PROCESS AS NEW LINE
;PAGE
;
; TAPE CASSETTE COMMANDS
;
;
;       TAPE CASSETTE EQUATES
;
SWCH    EQU     0FFH    ;SWITCH PORT
CASC    EQU     3       ;STATUS PORT FOR TARBELL
CASD    EQU     0       ;DATA PORT
CFLAG   EQU     4       ;DATA FLAG FOR TARBELL ON MIO
;
; CASSETTE FILE FORMAT
;
;    EACH RECORD:
;       TYPE BYTE: 4 FOR BASIC PROGRAM,
;                  PLUS BIT 7 ON IF DATA NOT HEADER RECORD
;       LENGTH BYTE: # DATA BYTES (1-128)
;       2 BYTES OF CHECKSUM
;
;    EACH FILE BEGINS WITH A HEADER RECORD
;       TYPE 4
;       LENGTH: 7
;           5 CHARS FILENAME, BLANK-FILLED
;           2 BYTES TOTAL LENGTH OF DATA IN FILE
;       2 BYTES OF CHECKSUM
;
;    AND HAS N DATA RECORDS
;       TYPE: 84
;       LENGTH: 128 EXCEPT LAST RECORD MAY BE LESS
;       DATA: NEXT (LENGTH) BYTES OF IMAGE OF PROGRAM AREA
;       CHECKSUM: 2 BYTES, 2'S COMPLEMENT OF SUM OF BYTES
;
;    FILES OF TYPE OTHER THAN 4 ARE IGNORED BY BASIC
;
; HARDWARE USED:
;       IMSAI MIO BOARD, CASSETTE DATA ON PORT 0,
;       STATUS ON PORT 3,
;       CASSETTE READY JUMPERED TO BIT 2 OF PORT 3.
;
;
;       TAPE UTILITY ROUTINE
;
; WATCH         WAIT FOR TARBELL READY OR CONTROL-C
;
WATCH:  PUSH B          ;SAVE REGS - CPM STATUS CALL CAN CLOBBER
        PUSH D
        PUSH H
        CALL    TSTCC   ;TEST FOR CNTRL-C
        POP H           ;RESTORE REGS IN CPM DEBUGGING MODE
        POP D
        POP B
        IN      CASC    ;READ STATUS PORT
        ANI     CFLAG   ;TEST
        JZ      WATCH   ;LOOP TILL RE^AADY
        RET
;
;
; CASI          CASSETTE INPUT TO A-REGISTER
;
CASI:   CALL    WATCH   ;WAIT TIL READY
        IN      CASD    ;READ FROM DATA PORT
        RET
;
;
; RECO          WRITE A RECORD TO THE TARBELL.
;               D,E==>TYPE, LENGTH BYTES
;               H,L==>START OF SOURCE
;               RETURNS UPDATED SOURCE POINTER IN DE
;
RECO:   MOV     A,D     ;TYPE BYTE
        CALL    CASO    ;WRITE IT
        MOV     A,E     ;COUNT
        CALL    CASO    ;WRITE IT
        MOV     B,E     ;COUNT
        XCHG            ;SOURCE NOW IN DE
        LXI     H,0     ;INITIAL CHECKSUM
NCHAR:  LDAX    D       ;FETCH NEXT CHAR
        CALL    CASO    ;WRITE IT
        INX     D       ;PNT NEXT CHAR
        CALL    CKSUM   ;ADD TO CKSUM, PUT ADD IN LIGHTS
        DCR     B       ;REDUCE COUNT
        JNZ     NCHAR   ;LOOP ON COUNT
        DCX     H       ;ADJUST HL FOR COMPLIMENT
        MOV     A,H     ;WRITE CHECKSUM
        CMA
        CALL    CASO
        MOV     A,L
        CMA
        ;WRITE LAST BYTE & RETURN
;
;
; CASO          CASSETTE OUTPUT BYTE FROM A-REGISTER
;
CASO:   PUSH PSW
        CALL WATCH      ;WAIT TILL READY
        POP PSW
        OUT CASD        ;WRITE TO DATA PORT
        RET
;
;
; CKSUM         CALCULATE THE CHECKSUM:
;               ADD A TO HL
;       ALSO OUTPUS HI ADDR TO SENSE LIGHTS
;
CKSUM:  ADD     L       ;ADD PREVIOUS LO
        MOV     L,A     ;SAVE NEW LO
        RNC
        INR     H       ;PROPAGATE CARRY
;
;
; SENSE         OUTPUT HI ADDR FROM D TO LIGHTS
;
SENSE:  MOV     A,D
        CMA
        OUT     SWCH
        RET
;
;
; RECI          INPUT A RECORD FROM THE TARBELL
;       TAKES A BUFFER POINTER IN HL
;       RETURNS UPDATED POINTER IN DE,
;               RECORD TYPE IN A, RECORD LENGTH IN C
;               CLOBBERS B,H,L
;
RECI:   CALL    CASI    ;GET TYPE
        PUSH    PSW     ;SAVE TYPE TO RETURN TO CALLER
        CALL    CASI    ;GET LENGTH
        MOV     C,A     ;STORE LEN
        MOV     B,A     ;IN B ALSO
        XCHG            ;PUT DESTINATION PTR IN DE
        LXI     H,0     ;INITIAL CHECKSUM
RECI1:  CALL    CASI    ;INPUT BYTE
        STAX    D       ;STORE IT
	INX	D
        CALL    CKSUM   ;UPDATE CKSUM, PUT ADDR IN LIGHTS
        DCR     B       ;LOOP ON COUNT
        JNZ     RECI1
        PUSH    D       ;SAVE DESTINATION PTR
        CALL    CASI    ;INPUT CHECKSUM
        MOV     D,A
        CALL    CASI
        MOV     E,A
        DAD     D       ;COMPARE
        MOV     A,H
        ORA     L
        JNZ     CKERR   ;BRIF CHECKSUM ERROR
        POP     D       ;RESTORE DEST PTR
        POP     PSW     ;RESTORE RECORD TYPE BYTE
        RET
;
;
; CSAVE COMMAND
;
CSAVE:  RST     1       ;SKIP ANY SPACES
        MVI     A,10H   ;ENABLE WRITE
        OUT     CASC
        PUSH    H       ;SAVE PTR
        MVI     B,255   ;WRITE INITIAL 255 NULLS
        XRA     A
NULS:   CALL    CASO
        DCR     B
        JNZ     NULS
        MVI     A,3CH   ;START BYTE
        CALL    CASO
        MVI     B,32    ;32 SYNC BYTES
        MVI     A,0E6H  ;SYNC BYTE VALUE
SYNCS:  CALL    CASO
        DCR     B
        JNZ     SYNCS
        LXI     H,IOBUF ;POINT BUFFER
        MVI     B,5     ;FILE NAME LENGTH
        POP     D       ;RESTORE CMD PTR
FNAME:  MVI     M,20H   ;DEFAULT BLANK
        LDAX    D       ;FETCH FILE NAME
        ORA     A       ;TEST
        JZ      BLANK
        MOV     M,A     ;STORE CHAR
        INX     D       ;NAME PTR
BLANK:  INX     H       ;BUFFER PTR
        DCR     B       ;COUNT
        JNZ     FNAME
;
; CALCULATE LGTH OF PROGRAM FILE&WRITE IT ON THE HEADER
;
        LXI     D,BEGPR ;BEGINNING OF PROGRAM
        LHLD    PROGE   ;END
        MOV     A,L
        SUB     E
        MOV     L,A
        MOV     A,H
        SBB     D
        MOV     H,A
        INX     H       ;PLUS 1 TO GET # OF BYTES INCLUSIVE
        PUSH    H       ;SAVE FOR LATER
        SHLD    IOBUF+5 ;STUFF LENGTH
        LXI     D,407H  ;TYPE AND LEN OF HEADER RECORD
                        ;TYPE 4: BASIC PROG FILE, HEADER RCD
        LXI     H,IOBUF
        CALL    RECO    ;WRITE RECORD
;
; WRITE PROGRAM FILE
;
        LXI     H,BEGPR ;POINT START OF PROGRAM
NXTRC:  XTHL            ;GET REMAINING LENGTH
        MOV     A,H     ;GET HI REMAINING
        ORA     L       ;TEST FOR DONE
        JZ      ERITE   ;BRIF DONE
        LXI     D,0FF80H;-128
        DAD     D       ;SUBTRACT RECORD LENGTH
        JC      RITE    ;IF CARRY, NOT AT END
        MOV     A,L     ;GET LOW
        ANI     7FH     ;NUMBER BYTES LEFT
        MOV     E,A     ;COUNT
        LXI     H,0     ;REMAINING BYTES
RITE:   XTHL            ;RESTORE H
        MVI     D,084H  ;TYPE BYTE: 80=DATA RECORD (NOT
                        ;FILE HDR), 4=BASIC PROGRAM FILE.
        CALL    RECO    ;WRITE
        XCHG            ;SAVE SOURCE PTR
        JMP     NXTRC
ERITE:  POP     H       ;CLEAN STACK
;
;
; BELL          RING USER'S CHIMES
;
BELL:   MVI     A,7     ;CODE FOR BELL
        CALL    TESTO
        JMP     RDY
        ;PAGE
; CLOAD         LOAD A PROGRAM FROM THE TARBELL
;
CLOAD:
NULL1:  MVI     A,60H   ;MIO CONTROL TO READ BY BITS
        OUT     CASC    ;WRITE TO STATUS PORT
NULLS:  CALL    CASI    ;READ LEADING NULLS
        OUT     SWCH    ;PUT IN LIGHTS
        CPI     0E6H    ;WAIT FOR FIRST SYNC BYTE
        JNZ     NULLS
        MVI     A,20H   ;MIO CONTROL TO READ BY BYTES
        OUT     CASC    ;WRITE TO STATUS PORT
        MVI     B,31    ;NUMBER REMAINING SYNC BYTES
SYNC:   CALL    CASI    ;READ PAST SYNC
        OUT     SWCH
        CPI     0E6H
        JNZ     NULL1   ;TRY FOR MORE NULLS
        DCR     B
        JNZ     SYNC
        LXI     H,IOBUF ;POINT BUFFER
        CALL    RECI    ;READ A RECORD
        CPI     4       ;TEST TYPE BYTE: IS IT BASIC PROGRAM
                        ;..FILE HEADER RECORD?
        JNZ     NULL1   ;NO, START OVER, KEEP LOOKING
        LHLD    IOBUF+5 ;LOAD LENGTH OF PROGRAM FILE
        PUSH    H       ;SAVE
        LXI     H,BEGPR
NXTR:   CALL    RECI    ;READ RECORD
        CPI     84H     ;IS IT BASIC PROGRAM FILE DATA RECORD
        JNZ     CKERR   ;NO, SOMETHING'S WRONG.
        POP     H       ;LENGTH
        ;SUBTRACT 0,C  FROM HL
        MOV     A,L
        SUB     C
        MOV     L,A
        MOV     A,H
        MVI     C,0
        SBB     C
        MOV     H,A
        ORA     L       ;TEST RESULT FOR 0
        XCHG            ;BUFFER ADDR TO HL
        PUSH    D       ;SAVE REMAINING LENGTH
        JNZ     NXTR    ;JIF NOT DONE READING DATA
        POP     D       ;CLEAR STACK
;LOADING DONE. SET POINTER TO END OF PROGRAM.
        XRA     A
        MOV     M,A     ;EXTRA 0 FOR PARANOISA
        DCX     H       ;POINT LAST RECORD BYTE (SHOULD BE 0)
        SHLD    PROGE   ;SAVE END OF PROG FOR EDIT, LIST, &C
        STA     IOBUF+5 ;MARK END OF FILE NAME FOR TYPEOUT
;TYPE FILE NAME
        LDA     IOBUF
        CPI     20H     ;TEST FOR NO NAME
        CNZ     TERMO   ;PRINT NAME IF THERE
        JMP     BELL
        ENDIF
;
PEEK    EQU     $
;
; STMT: A=PEEK(X). RETURNS DECIMAL VALUE OF MEMORY ADDRESS X.
;
        CALL    FACDE   ;GET ADDRESS IN D,E
        XCHG            ;ADDRESS TO H,L
        LXI     D,0     ;CLEAR D,E
        MOV     E,M     ;PUT MEMORY BYTE IN E
        JMP     BINFL   ;CONVERT D,E TO BINARY AND RETURN
;
POKE    EQU     $
;
; STMT: POKE <ADDRESS>,<VALUE>.  PUTS IN MEMORY ADDRESS.
;
        CALL    EXPR    ;EVALUATE ADDRESS EXPRESSION
        MOV     A,M     ;LOAD NEXT CHARACTER
        CPI     ','     ;TEST
        JNZ     SNERR   ;BRIF ERROR
        INX     H       ;POINT NEXT
        PUSH    H       ;SAVE H,L
        CALL    FACDE   ;PUT ADDRESS IN D,E
        POP     H       ;RESTORE H,L
        PUSH    D       ;SAVE ADDRESS
        CALL    EXPR    ;EVALUATE VALUE EXPRESSION
        CALL    EOL     ;TEST FOR END OF LINE
        CALL    FBIN    ;CONVERT FACC TO A REGISTER VALUE
        POP     H       ;GET D,E ADDRESS IN H,L
        MOV     M,A     ;MOVE BYTE
        JMP     RUN     ;CONTINUE
;
;
JUMP    EQU     $
;
; STMT: CALL <ADDRESS>. EXECUTES CODE AT MEMORY ADDRESS.
;
        CALL    EXPR    ;EVALUATE ADDRESS EXPRESSION
        CALL    EOL     ;TEST FOR END OF LINE
        CALL    FACDE   ;CONVERT FACC TO ADDRESS IN D,E
        LXI     H,RUN   ;MAKE INTO SUBROUTINE
        PUSH    H
        XCHG            ;MOVE ADDRESS TO HL
        PCHL            ;EXECUTE USER'S ROUTINE
;PAGE
        IF      HUNTER
;
;
BAUD    EQU     $
;
; SOFTWARE BAUD SELECTION ON SIO BOARDS MODIFIED BY
; W. HARTER, COYOTE COMPUTERS, DAVIS, CALIF.
;
; COMMAND 'BAUD <RATE>' WHERE <RATE>=110,300,1200,2400,9600
;
        RST     1       ;SKIP BLANKS
        LXI     D,BAUDS+6       ;POINT BAUD TABLE
        CALL    SEEK    ;GO SEARCH BAUD TABLE
        JZ      CVERR   ;BRIF RATE NOT FOUND
        DCX     H       ;ADJUST POINTER
BAUD1:  INX     H       ;LOOK AT CHARACTER
        CALL    NUMER   ;TEST FOR DIGIT
        JZ      BAUD1   ;LOOP PAST RATE
        CALL    EOL     ;TEST FOR END OF LINE
        XCHG            ;POINT ADDRESS OF CONTROL BYTES
        MOV     E,M     ;LOW BYTE TO E
        INX     H       ;POINT NEXT
        MOV     D,M     ;HIGH BYTE TO D
        LDA     EDSW    ;GET MODE SWITCH
        ORA     A       ;TEST IT
        JNZ     SETIT   ;BRIF IMMEDIATE MODE
        LXI     H,BAUDS ;POINT 'BAUD'
        CALL    TERMM   ;WRITE IT
        PUSH    D       ;SAVE ADDRESS OF CONTROL BYTES
        LXI     H,IOBUF ;POINT BUFFER
        MVI     B,4     ;LOAD COUNT
        CALL    COPYD   ;COPY RATE TO IOBUF
        MVI     M,0     ;TERMINATE MESSAGE
        CALL    TERMO   ;WRITE IT
        POP     D       ;RESTORE CONTROL BYTES
SETIT:  LXI     H,4     ;LOAD OFFSET
        DAD     D       ;PIONT 1ST CONTROL BYTE
        MVI     A,40H   ;LOAD RESET
        OUT     TTY+1   ;WRITE IT
        MVI     A,M     ;MODE BYTE
        OUT     TTY+1   ;WRITE IT
        MVI     A,17H   ;ENABLE BYTE
        OUT     TTY+1   ;WRITE IT
        INX     H       ;POINT SPEED BYTE
        MOV     A,M     ;LOAD IT
        OUT     8       ;WRITE IT
BAUD2:  IN      TTY+1   ;READ STATUS
        ANI     2       ;TEST
        JZ      BAUD2   ;WAIT FOR ACKNOWLEDGMENT
        IN      TTY     ;READ AND DISCARD
        LDA     EDSW    ;GET MODE SWITCH
        ORA     A       ;TEST IT
        JZ      RUN     ;BRIF RUN MODE
        JMP     GETCM   ;BRIF IMMEDIATE MODE
BAUDS:  DB      'BAUD',0FEH     ;BAUD MESSAGE
;
; BAUD TABLE.
;
B110:   DB      '110 ',0FAH,2,0
        DW      B110
B300:   DB      '300 ',0FBH,0
        DW      B300
B1200:  DB      '1200',0FAH,0
        DW      B1200
B2400:  DB      '2400',0FAH,32,0
        DW      B2400
B9600:  DB      '9600',0FAH,34,0
        DW      B9600
        DB      0       ;END OF BAUD TABLE
;
        ENDIF
;
        IF      CPM     ;CPM INITIALIZATION STORES
                        ;...BIOS JUMP TABLE HERE
BTSTAT: DS      3       ;JMP TO BIOS CONSOLE STATUS
BTIN:   DS      3       ;JMP TO BIOS CONSOLE INPUT
BTOUT:  DS      3       ;JMP TO BIOS CONSOLE OUTPUT
        ENDIF
;PAGE
ROMEN   EQU     $-1
;
        ORG     8192    ;RAM STARTS OF 8K BOUNDARY
        IF      LARGE OR CPM    ;ADJUST START OF RAM IF 8+K
        ORG     2400H   ;RAM STARTS ON 9K BOUNDARY
        ENDIF
;
; ALL CODE ABOVE THIS POINT IS READ ONLY AND CAN BE PROM'ED
;
;
RAM     EQU     $
;
BZERO   EQU     $
FORNE:  DS      1       ;# ENTRYS IN TABLE (MUST BE HERE)
        DS      112     ;ROOM FOR 8 NESTS (MUST BE HERE)
TAPES:  DS      1       ;TAPE SWITCH (MUST BE HERE)
DIMSW:  DS      1       ;DIM SWITCH (MUST BE HERE)
OUTSW:  DS      1       ;OUTPUT SWITCH (MUST BE HERE)
ILSW:   DS      1       ;INPUT LINE SWITCH (MUST BE HERE)
RUNSW:  DS      1       ;RUN SWITCH(MUST BE HERE)
EDSW:   DS      1       ;MODE SWITCH(MUST BE HERE)
EZERO   EQU     $
;
LINEN:  DS      5
IMMED:  DS      82      ;IMMEDIATE COMMAND STORAGE AREA
IOBUF:  DS      82      ;INPUT/OUTPUT BUFFER
STRIN:  DS      256     ;STRING BUFFER AREA
OUTA:   DS      3       ;*** FILLED IN AT RUN TIME
INDX:   DS      2       ;HOLDS VARIABLE NAME OF FOR/NEXT
REL:    DS      1       ;HOLDS THE RELATION IN AN IF STMT
IFTYP:  DS      1       ;HOLDS TYPE CODE OF LEFT SIDE
TVAR1:  DS      4       ;TEMP STORAGE
TVAR2:  DS      4       ;DITTO
TEMP1:  DS      4       ;TEMP STORAGE FOR FUNCTIONS
TEMP2:  DS      4
TEMP3:  DS      4
TEMP4:  DS      4
TEMP5:  DS      4
TEMP6:  DS      4
TEMP7:  DS      4
LINEL:  DS      2       ;HOLDS MIN LINE NUMBER IN LIST
LINEH:  DS      2       ;HOLDS MAX LINE NUMBER IN LIST
PROMP:  DS      1       ;HOLDS PROMPT CHAR
EXPRS:  DS      2       ;HOLDS ADDR OF EXPRESSION
ADDR1:  DS      2       ;HOLDS TEMP ADDRESS
ADDR2:  DS      2       ;HOLDS TEMP ADDRESS
ADDR3:  DS      2       ;HOLDS STMT ADD DURING EXPR EVAL
FACC:   DS      4
FTEMP:  DS      12
PARCT:  DS      1
SPCTR:  DS      2
CMACT:  DS      1       ;COUNT OF COMMAS
FNARG:  DS      4       ;SYMBOLIC ARG & ADDRESS
STMT:   DS      2       ;HOLDS ADDR OF CURRENT STATEMENT
ENDLI:  DS      2       ;HOLDS ADDR OF MULTI STMT PTR
MULTI:  DS      1       ;SWITCH 0=NO, 1=MULTI STMT LINE
DEXP:   DS      1
COLUM:  DS      1       ;CURRENT TTY COLUMN
RNDX:   DS      2       ;RANDOM VARIABLE STORAGE
RNDY:   DS      2       ;THE RND<X>,TRND<X>,AND RNDSW
RNDZ:   DS      2       ;MUST BE KEPT IN ORDER
RNDS:   DS      2
TRNDX:  DS      2
TRNDY:  DS      2
TRNDZ:  DS      2
TRNDS:  DS      2
RNDSW:  DS      1
FNMOD:  DS      1       ;SWITCH, 0=NOT, <>0 = IN DEF FN
LINE:   DS      2       ;HOLD ADD OF PREV LINE NUM
STACK:  DS      2       ;HOLDS ADDR OF START OF RETURN STACK
PRSW:   DS      1       ;ON=PRINT ENDED WITH , OR ;
NS:     DS      1       ;HOLDS LAST TYPE (NUMERIC/STRING)
DATAP:  DS      2       ;ADDRESS OF CURRENT DATA STMT
DATAB:  DS      2       ;ADDRESS OF DATA POOL
PROGE:  DS      2       ;ADDRESS OF PROGRAM END
;
        IF      CPM
;TEMPORARY CODE FOR INITIALIZATION HERE
;
INITC:  LHLD    BOOT+1  ;PTR TO BIOS TABLE
        LXI     D,CSTAT ;OFFSET OF CONSOLE QUERY ENTRY
        DAD     D       ;POINT INTO BIO JUMP TABLE
        LXI     D,BTSTAT;POINT INTO BASIC JMP TABLE
        MVI     B,9     ;COUNT
        CALL    COPYH   ;MOE BIOS TABLE INTO BASIC
        MVI     A,0C3H  ;JMP OP CODE
        LXI     H,RST1! STA 8H! SHLD 9H
        LXI     H,RST2! STA 10H! SHLD 11H
        LXI     H,RST3! STA 18H! SHLD 19H
        LXI     H,RST4! STA 20H! SHLD 21H
        LXI     H,RST5! STA 28H! SHLD 29H
        LXI     H,RST6! STA 30H! SHLD 31H
        LHLD    BDOS+1  ;LOCATE TOP OF RAM
        JMP     INIT1   ;CONTINUE AS IN NON-CPM VERSION
        ENDIF
;
;
        DS      1       ;DATA STATEMENT FLAG (MUST BE HERE)
BEGPR:
;
        END

        TITLE '<APPLE MONITOR, *ECT ROM* V1.0  JAN 07, 1979>'
;
; "APPLE MONITOR" COPYRIGHT 1975,1976,1977
; BY ROGER AMIDON
;
; THIS MONITOR IS 8080 CODE ONLY
;
; MAY 2018 BY UDO MUNK:
;       TYPED IN FROM MANUAL USING INTEL SYNTAX TO ASSEMBLE
;       WITH INTEL MACRO 80 OR DRI 8080 ASSEMBLER
;
BASE    EQU  0F000H             ;ROM STARTING ADDRESS
USER    EQU  BASE+800H
;
;       THIS VERSION WRITTEN FOR ELECTRONIC CONTROL TECHNOLOGY
;                 ALL RIGHTS RESERVED
;
IO      EQU  0                  ;I/O PORT BASE
;
RST7    EQU  38H                ;RST 7 (LOCATION FOR TRAP)
;
;       <I/O DEVICES>
;
;-C.R.T. SYSTEM
;
CRTI    EQU  IO+1H              ;DATA PORT (IN)
CRTS    EQU  IO+0H              ;STATUS PORT (IN)
CRTO    EQU  IO+1H              ;DATA PORT (OUT)
CRTDA   EQU  1                  ;DATA AVAILABLE MASK
CRTBE   EQU  80H                ;XMTR BUFFER EMPTY MASK
;
;-PRINTER
;
TTI     EQU  IO+3H              ;DATA IN PORT
TTO     EQU  IO+3H              ;DATA OUT PORT
TTS     EQU  IO+2H              ;STATUS PORT (IN)
TTYDA   EQU  1                  ;DATA AVAILABLE MASK BIT
TTYBE   EQU  80H                ;XMTR BUFFER EMPTY MASK
;
;-DATA TRANSFER SYSTEM
;
RCSD    EQU  IO+5H              ;DATA IN PORT
RCSS    EQU  IO+4H              ;STATUS PORT (IN)
RCSDA   EQU  1                  ;DATA AVAILABLE MASK
PCASO   EQU  IO+5H              ;DATA PORT (OUT)
PCSBE   EQU  80H                ;XMTR BUFFER EMPTY MASK
;
; PARALLEL PORT
;
PPDATA  EQU  IO+7               ;PARALLEL DATA PORT
PPSTAT  EQU  IO+6               ;PARALLEL STATUS PORT
PPDA    EQU  1                  ;DATA AVAILABLE
PPBE    EQU  80H                ;CLEAR TO TRANSMIT DATA
;
;       <CONSTANTS>
;
FALSE   EQU  0                  ;ISN'T SO
TRUE    EQU  NOT FALSE          ;IT IS SO
CR      EQU  0DH                ;ASCII CARRIAGE RETURN
LF      EQU  0AH                ;ASCII LINE FEED
BELL    EQU  7                  ;DING
RUB     EQU  0FFH               ;RUB OUT
FIL     EQU  0                  ;FILL CHARACTER AFTER CRLF
MAX     EQU  7                  ;NUMBER OF QUES IN EOF
;
;       <I/O CONFIGURATION MASKS>
;
CMSK    EQU  11111100B          ;CONSOLE DEVICE
RMSK    EQU  11110011B          ;STORAGE DEVICE (IN)
PMSK    EQU  11001111B          ;STORAGE DEVICE (OUT)
LMSK    EQU  00111111B          ;LIST DEVICE
;
;-CONSOLE CONFIGURATION
CCRT    EQU  0                  ;C.R.T.
CTTY    EQU  1                  ;PRINTER
BATCH   EQU  2                  ;READER FOR INPUT, LIST FOR OUTPUT
CUSE    EQU  3                  ;USER DEFINED
;
;-STORAGE INPUT CONFIGURATION
RPTR    EQU  0                  ;DATA TRANSFER DEVICE
RTTY    EQU  4                  ;PRINTER DEVICE
RCAS    EQU  8                  ;PARALLEL PORT
RUSER   EQU  0CH                ;USER DEFINED
;
;-STORAGE OUTPUT CONFIGURATION
PPTP    EQU  0                  ;DATA TRANSFER DEVICE
PTTY    EQU  10H                ;PRINTER PUNCH
PCAS    EQU  20H                ;PARALLEL PORT
PUSER   EQU  30H                ;USER DEFINED
;
;-LIST DEVICE CONFIGURATION
LTTY    EQU  0                  ;CONSOLE DEVICE
LCRT    EQU  40H                ;PRINTER
LINE    EQU  80H                ;DATA TRANSFER DEVICE
LUSER   EQU  0C0H               ;USER DEFINED
;
;
;       VECTORS FOR USER DEFINED ROUTINES
;
CILOC   EQU  USER               ;CONSOLE INPUT
COLOC   EQU  CILOC+3            ;CONSOLE OUTPUT
CSLOC   EQU  COLOC+3            ;CONSOLE INPUT STATUS ROUTINE
RULOC   EQU  CSLOC+3            ;USER DEFINED STORAGE (INPUT)
PULOC   EQU  RULOC+3            ;USER DEFINED STORAGE (OUTPUT)
LULOC   EQU  PULOC+3            ;USER DEFINED PRINTER (LIST)
J       EQU  LULOC+3
;
;       PROGRAM CODE BEGINS HERE
;
        ORG  BASE
;
APPLE:  JMP  BEGIN              ;GO AROUND VECTORS
;
;       <VECTORS FOR CALLING PROGRAMS>
;
; THESE VECTORS MAY BE USED BY USER WRITTEN
; PROGRAMS TO SIMPLIFY THE HANDLING OF I/O
; FROM SYSTEM TO SYSTEM.  WHATEVER THE CURRENT
; ASSIGNED DEVICE, THESE VECTORS WILL PERFORM
; THE REQUIRED I/O OPERATIION, AND RETURN TO
; THE CALLING PROGRAM. (RET)
;
; THE REGISTER CONVENTION USED FOLLOWS-
;
; ANY INPUT OR OUTPUT DEVICE-
;       CHARACTER TO BE OUTPUT IN 'C' REGISTER.
;       CHARACTER WILL BE IN 'A' REGISTER UPON
;       RETURNING FROM AN INPUT OR OUTPUT.
; 'CSTS'-
;       RETURNS TRUE (0FFH IN 'A' REG.) IF THERE IS
;       SOMETHING WAITING, AND ZERO (00) IF NOT.
; 'IOCHK'-
;       RETURNS WITH THE CURRENT I/O CONFIGURATION
;       BYTE IN 'A' REGISTER.
; 'IOSET'-
;       ALLOWS A PROGRAM TO DYNAMICALLY ALTER THE
;       CURRENT I/O CONFIGURATION, AND REQUIRES
;       THE NEW BYTE IN 'C' REGISTER.
; 'MEMCK'-
;       RETURNS WITH THE HIGHEST ALLOWED USER
;       MEMORY LOCATION. 'B'=HIGH BYTE, 'A'=LOW.
; 'TRAP'-
;       THIS IS THE 'BREAKPOINT' ENTRY POINT,
;       BUT MAY BE 'CALLED'. IT WILL SAVE
;       THE MACHINE STATE. RETURN CAN BE MADE WITH
;       A SIMPLE 'G[CR]' ON THE CONSOLE.
;
        JMP  CI                 ;CONSOLE INPUT
        JMP  RI                 ;READER INPUT
        JMP  CO                 ;CONSOLE OUTPUT
        JMP  PO                 ;PUNCH OUTPUT
        JMP  LO                 ;LIST OUTPUT
        JMP  CSTS               ;CONSOLE STATUS
        JMP  IOCHK              ;I/O ASSIGNMENT CHECK
        JMP  IOSET              ;I/O SET
        JMP  MEMCK              ;MEMORY LIMIT CHECK
;
TRAP:   PUSH H                  ;ASSUME A VALID STACK
        PUSH D
        PUSH B
        PUSH PSW                ;SAVE MACHINE STATE
        LXI  D,65535-(ENDX-EXIT)
TR0:    LXI  H,10               ;GO UP 10 BYTES IN STACK
        DAD  SP
        MVI  B,4
        XCHG
TR1:    DCX  H
        MOV  M,D
        DCX  H
        MOV  M,E
        POP  D
        DCR  B
        JNZ  TR1
        POP  B                  ;OLD PC
        DCX  B                  ;-1
        SPHL                    ;SET MONITOR'S STACK
        LXI  H,TLOC
        DAD  SP
        CALL TR5                ;TEST IF A TRAP SET
        INX  H
        INX  H
        CNZ  TR5                ;TEST FOR 2ND TRAP
        JZ   TR2                ; NO
        INX  B
TR2:    LXI  H,LLOC
        DAD  SP
        MOV  M,E
        INX  H
        MOV  M,D
        INX  H
        INX  H
        MOV  M,C
        INX  H
        MOV  M,B
        PUSH B
        MVI  C,'@'
        CALL CO
        POP  H
        CALL LADR
        LXI  H,TLOC
        DAD  SP
        LXI  D,2
TR3:    MOV  C,M
        MOV  M,D
        INX  H
        MOV  B,M
        MOV  M,D
        INX  H
        MOV  A,C
        ORA  B
        JZ   TR4
        MOV  A,M
        STAX B
TR4:    INX  H
        DCR  E
        JNZ  TR3
        JMP  START
;
TR5:    MOV  A,M
        SUB  C
        INX  H
        RNZ
        MOV  A,M
        SUB  B
        RET
;
MEMSIZ: LXI  H,-1               ;START AT THE BOTTOM
ME0:    INR  H                  ;FIRST FIND R/W MEMORY
        MOV  A,M
        CMA
        MOV  M,A
        CMP  M
        CMA
        MOV  M,A
        JNZ  ME0
ME1:    INR  H                  ;NOW FIND NON-R/W
        MOV  A,M
        CMA
        MOV  M,A
        CMP  M
        CMA
        MOV  M,A
        JZ   ME1
        DCR  H
        RET
;
MEMCK:  PUSH H
        CALL MEMSIZ
        MOV  B,H                ;USER'S HIGH BYTE
        POP  H
        MVI  A,0C0H             ;USER'S LOW BYTE
        RET
;
TOM:    LXI  H,MSG
TOM1:   MOV  C,M
        INX  H
        CALL CO
        DCR  B
        JNZ  TOM1
        CALL CSTS
        ORA  A
        RZ
;
CCHK:   CALL KI
        CPI  3
        RNZ
;
ERROR:  LXI  SP,65535-((ENDX-EXIT)+8)
        MVI  C,'*'
        CALL CO
        JMP  START
;
;
;
;
;       ANNOUNCEMENT OF MONITOR NAME & VERSION
;
MSG:    DB   CR,LF,FIL,FIL,FIL
        DB   'APPLE V'
        DB   '1.0 ECT'
MSGL    EQU  $-MSG
;
;       LET US BEGIN
;
BEGIN:  LXI  H,65535-(ENDX-EXIT)
        SPHL                    ;SET UP A STACK
        MVI  B,ENDX-EXIT
        LXI  D,EXIT
BG1:    LDAX D
        MOV  M,A
        INX  H
        INX  D
        DCR  B
        JNZ  BG1
        CALL MEMSIZ             ;GET USER'S STACK
        PUSH H
        MOV  H,B                ;ZERO OUT HL
        MOV  L,B
        PUSH H
        PUSH H
        PUSH H
;       MVI  A,CONFIG ???
        MVI  A,0
        STA  -1
        MVI  B,MSGL
        CALL TOM                ;PRINT SIGN-ON
START:  LXI  D,START
        PUSH D
        CALL CRLF
        MVI  C,'>'
        CALL CO
        LXI  H,TBL
STAR0:  CALL TI
        JZ   STAR0
        CPI  ' '                ;CONTROL?
        JC   STAR0              ;IGNORE
        SUI  'A'
        RC                      ;<A
        CPI  'Z'-'A'+1
        RNC                     ;>Z
        ADD  A                  ;A*2
        ADD  L                  ;+TBL
        MOV  L,A
        MOV  A,M
        INX  H
        MOV  H,M
        MOV  L,A
        ANA  H
        INR  A
        JZ   ERROR              ;DON'T GO TO 0FFFFH
        PCHL
;
;
TBL:    DW   ASSIGN             ;A - ASSIGN I/O
        DW   BRANCH             ;B - BRANCH TO USER ROUTINE A-Z
        DW   OFF                ;C UNDEFINED
        DW   DISP               ;D - DISPLAY MEMORY ON CONSOLE IN HEX
        DW   EOF                ;E - END OF FILE TAG FOR HEX DUMPS
        DW   FILL               ;F - FILL MEMORY WITH CONSTANT
        DW   GOTO               ;G - GOTO <ADDRESS>, W/BKPNTS (2)
        DW   HEXN               ;H - HEX MATH <SUM> <DIFFERENCE>
        DW   J                  ;I *** USER DEFINED
        DW   TEST               ;J - NON-DESTRUCTIVE MEMORY TEST
        DW   J+3                ;K *** USER DEFINED
        DW   LOAD               ;L - LOAD A BINARY FORMAT FILE
        DW   MOVE               ;M - MOVE MASS MEMORY
        DW   NULL               ;N - PUNCH LEADER/TRAILER
        DW   J+6                ;O *** USER DEFINED
        DW   PUTA               ;P - 'PUT' ASCII INTO MEMORY
        DW   QUERY              ;Q - QI(N)=READ I/O; QO(N,V)=SEND I/O
        DW   READ               ;R - READ A HEX FILE (W/CHECKSUM)
        DW   SUBS               ;S - EXAMINE/SUBSTITUTE MEMORY
        DW   TYPE               ;T - DISPLAY MEMORY IN ASCII
        DW   UNLD               ;U - DUMP MEMORY IN BINARY FORMAT
        DW   VERIFY             ;V - COMPARE MEMORY TO MEMORY
        DW   WRITE              ;W - DUMP MEMORY IN HEX FILE FORMAT
        DW   XAM                ;X - EXAMINE/MODIFY CPU REGISTERS
        DW   WHERE              ;Y - FIND 'N' BYTES IN MEMORY
        DW   SIZE               ;Z - ADDR OF LAST R/W MEMORY LOCATION
;
OFF     EQU  -1
;
UTAB    EQU  USER+80H
;
;
ASSIGN: CALL TI                 ;GET A DEVICE
        LXI  H,LTBL-1           ;POINT TO TABLE
        LXI  B,4                ;TO SKIP THRU TABLE
        CALL AS3                ;GET DEVICE COUNT
        PUSH D
AS1:    CALL TI
        SUI  '='
        JNZ  AS1
        MOV  C,A                ;C=0
        CALL TI                 ;GET ASSIGNMENT
        CALL AS3
        POP  PSW                ;A=DEVICE
        MOV  L,D                ;L=ASSIGNMENT
        MVI  H,3                ;SETUP A MASK
AS2:    DCR  A                  ;ZERO=DONE
        JM   AS5
        DAD  H
        DAD  H                  ;DOUBLE SHIFT LEFT
        JMP  AS2
AS3:    LXI  D,4                ;GO THRU THIS 4 TIMES
AS4:    INX  H                  ;BUMP POINTER 1
        CMP  M                  ;MATCH?
        RZ                      ;YES
        DAD  B                  ;BUMP HL
        INR  D
        DCR  E                  ;COUNT DOWN
        JNZ  AS4
        JMP  ERROR              ;CAN'T FIND IT
;
AS5:    XRA  A                  ;COMPLIMENT H
        MOV  H,A
        CALL IOCHK              ;GET CURRENT CONFIGURATION
        ANA  H                  ;KILL ASSIGNMENT BITS
        ORA  L                  ;MODIFY TO NEW DEVICE
        MOV  C,A                ;PUT NEW IOBYT IN C
;
SZA     EQU  $-ASSIGN
;
IOSET:  MOV  A,C
        STA  -1
        RET
;
IOCHK:  LDA  -1
        RET
;
BRANCH: CALL TI                 ;GET A '.'
        CPI  '.'
        JNZ  ERROR
        LXI  H,UTAB             ;POINT TO USER'S TBL
        JMP  STAR0              ;GOOD LUCK
;
SZB     EQU  $-BRANCH
;
SZC     EQU  $-$
;
DISP:   MVI  C,16               ;SET A DEFAULT
        CALL EXPC
        PUSH PSW
DI0:    CALL LFADR
        POP  PSW
        PUSH PSW                ;GET SIZE
        MOV  B,A                ;IN B
DI1:    CALL BLK
        MOV  A,M
        CALL LBYTE
        CALL HILO
        JC   PRET
        DCR  B
        JNZ  DI1
        JMP  DI0
;
SZD     EQU  $-DISP
;
EOF:    CALL EXPR
        CALL PEOL
        MVI  C,':'
        CALL PO
        XRA  A
        CALL PBYTE
        POP  H
        CALL PADR
        LXI  H,0
        CALL PADR
        JMP  NULL
;
SZE     EQU  $-EOF
;
FILL:   CALL EXPC
FI1:    MOV  M,C
        CALL HILO
        JNC  FI1
        POP  D
        JMP  START
;
SZF     EQU  $-FILL
;
GOTO:   CALL PCHK
        JZ   GO0                ;DELIMITER ENTERED
        CALL EXF                ; ELSE GET A 'GO' ADDR
        POP  D
        LXI  H,PLOC
        DAD  SP
        MOV  M,D                ;PLACE IN EXIT TEMPLATE
        DCX  H
        MOV  M,E
GO0:    CPI  CR                 ;TEST DELIMITER
        JZ   GO4                ;NO BREAKPOINTS, JUST GO
        MVI  D,2                ;2 POSSIBLE BREAKPOINTS
        LXI  H,TLOCX
        DAD  SP
GO1:    PUSH H
        CALL EXPR               ;GET AN ADDRESS
        POP  B                  ;IN BC
        POP  H
        PUSH PSW                ;SAVE DELIMITER
        MOV  A,B                ;CAN'T ALLOW ANY
        ORA  C                  ; BREAKPOINTS AT ZERO
        JZ   GO2
        MOV  M,C
        INX  H
        MOV  M,B                ;ELSE SAVE BKPT ADDRESS
        INX  H
        LDAX B                  ;AND OPCODE THERE
        MOV  M,A
        INX  H
        MVI  A,0FFH             ;RST 7
        STAX B                  ;REPLACE OPCODE
GO2:    POP  PSW                ;LOOK AT DELIMITER
        JC   GO3
        DCR  D
        JNZ  GO1
GO3:    MVI  A,0C3H             ;SET A 'JMP' AT RST 7
        STA  RST7
        LXI  H,TRAP
        SHLD RST7+1
GO4:    CALL CRLF
        POP  D                  ;THROW AWAY RETURN
        LXI  H,8
        DAD  SP
        PCHL
;
SZG     EQU  $-GOTO
;
TEST:   CALL EXPC
TE1:    MOV  A,M
        MOV  B,A                ;SAVE CHAR IN 'B'
        CMA
        MOV  M,A
        XRA  M
        MOV  M,B                ;REPLACE BYTE
        JZ   TE2
        PUSH D                  ;SAVE END POINTER
        MOV  E,A                ;SAVE ERROR MASK
        CALL HLSP               ;DISPLAY BAD ADDRESS
        CALL BITS+1             ;DISPLAY BAD BIT(S)
        POP  D                  ;RESTORE DE
TE2:    CALL HILOX
        JMP  TE1
;
SZJ     EQU  $-TEST
;
LOAD:   CALL EXPR
        CALL CRLF
        POP  H
        MVI  D,RUB
LO0:    LXI  B,407H             ;B=4 MATCHES, C=BELL
LO1:    CALL RIFF
        JNZ  LO0
        DCR  B
        JNZ  LO1
LO2:    CALL RIFF
        JZ   LO2
        MOV  M,A
        CALL CO                 ;TELL CONSOLE
LO3:    INX  H
        CALL RIFF
        JZ   LO5
LO4:    MOV  M,A
        JMP  LO3
LO5:    MVI  E,1                ;INITIALIZE
LO6:    CALL RIFF
        JNZ  LO7
        INR  E
        MVI  A,MAX
        CMP  E
        JNZ  LO6
        JMP  LFADR
LO7:    MOV  M,D
        INX  H
        DCR  E
        JNZ  LO7
        JMP  LO4
;
SZL     EQU  $-LOAD
;
MOVE:   CALL EXPC
MO:     MOV  A,M
        STAX B
        INX  B
        CALL HILOX
        JMP  MO
;
SZM     EQU  $-MOVE
;
PUTA:   CALL EXPR
        CALL CRLF
        POP  H
PU0:    CALL KI
        CPI  4                  ;EOT?
        JZ   LFADR              ;PRINT ADDRESS & QUIT
        CPI  7FH                ;RUB-OUT?
        JZ   PU2                ; YES
        MOV  M,A                ;PUT CHARACTER INTO MEMORY
        MOV  C,A
        INX  H
PU1:    CALL CO                 ;ECHO CHARACTER
        JMP  PU0                ;& CONTINUE
PU2:    DCX  H                  ;BACK-UP POINTER
        MOV  C,M                ;ECHO CANCELED CHARACTER
        JMP  PU1
;
SZP     EQU  $-PUTA
;
WHERE:  LXI  H,0                ;GET STRING POINTER (SP)
        MOV  C,L                ;ZERO C REG
        DAD  SP
        DCX  H                  ;SP-1
        XCHG                    ;SAVE IN DE
WH1:    CALL EXPR
        POP  H                  ;CONSERVE STACK USAGE
        MOV  H,L                ;L=SEARCH BYTE
        PUSH H                  ;H=L
        INX  SP                 ;ADJUST STACK
        INR  C                  ;COUNT SEARCH BYTES
        JNC  WH1
        XCHG
        MOV  D,C
        PUSH H                  ;HL=SEARCH STRING POINTER
        LXI  B,0
        PUSH B                  ;BC=START SEARCH (0)
WH2:    CALL CRLF
WH3:    POP  B
        POP  H
        MOV  E,D
        MOV  A,B
        ANA  C
        INR  A
        JNZ  WH5
WH4:    INX  H
        SPHL                    ;RESET STACK
        RET
WH5:    LDAX B
        INX  B
        CMP  M
        PUSH H
        PUSH B
WH6:    JNZ  WH3
        DCR  E
        JZ   WH7
        LDAX B
        INX  B
        DCX  H
        CMP  M
        JMP  WH6
WH7:    POP  H
        PUSH H
        DCX  H
        CALL LADR
        JMP  WH2
;
SZY     EQU  $-WHERE
;
READ:   CALL EXPR               ;GET 16 BIT VALUE
        POP  D                  ;DE=BIAS
        LXI  H,0                ;SET-UP DEFAULT BASE[1]
        PUSH H                  ;AND DEFAULT BASE[2]
        JC   RD0                ;CR
        CALL EXPR               ;GET ACTUAL BASE[1]
        POP  H                  ;HL=BASE[1]
        JC   RD0                ;CR
        XTHL                    ;GET DEFAULT BASE[2]
        CALL EXPR               ;GET ACTUAL BASE[2]
        POP  H
        XTHL                    ;(SP)=BASE[2]
RD0:    PUSH H                  ;HL=BASE[1]
        PUSH D                  ;DE=BIAS
        CALL CRLF               ;BEGIN READING FILE
RD1:    CALL RIX                ;GET READER CHARACTER
        SUI  ':'                ;GET FILE TYPE CUE
        MOV  B,A                ;SAVE CUE CLUE
        ANI  0FEH               ;KILL BIT 0
        JNZ  RD1                ;NOT ':' OR ';'
        MOV  D,A                ;ZERO CHECKSUM STORAGE
        CALL BYTE               ;GET FILE LRNGTH
        MOV  E,A                ;SAVE IN E
        CALL BYTE               ;GET LOAD MSB
        PUSH PSW                ;SAVE IN STACK
        CALL BYTE               ;GET LOAD LSB
        POP  H                  ;H=MSB
        MOV  L,A                ;HL=LOAD ADDR
        CALL BYTE               ;GET FILE TYPE
        ORA  A                  ;TEST FILE TYPE
        MOV  A,B                ;GET CUE
        POP  B                  ;BC=BIAS
        JZ   RD2                ;ABSOLUTE LOAD
        XCHG                    ;RELOCATE LOAD ADDR.
        XTHL
        XCHG
        DAD  D                  ;DO IT
        XCHG
        XTHL
        XCHG                    ;HL=LOAD+BASE[1]
RD2:    INR  E                  ;TEST LENGTH
        DCR  E                  ;ZERO?
        JZ   DONE
        DAD  B                  ;ADD BIAS TO LOAD
        PUSH B                  ;SAVE BIAS
        MOV  B,A                ;SET-UP B
        DCR  A                  ;TEST CUE CLUE
        JZ   RD6                ;Z=REL. FILE, NZ=ABS.
RD3:    CALL BYTE               ;GET NEXT DATA BYTE
        MOV  M,A                ;WRITE TO MEMORY
        INX  H                  ;BUMP UP LOAD POINT
        DCR  E                  ;BUMP DOWN BYTE COUNT
        JNZ  RD3                ;CONTINUE
RD4:    CALL BYTE               ;TEST CHECKSUN
        JZ   RD1                ;OK; CONTINUE W/NEXT
RD5:    CALL LADR               ; ELSE PRINT LOAD ADDR
        JMP  ERROR              ; & ABORT
RD6:    CALL RD10               ;GET NEXT DATA BYTE
        MOV  M,A                ;STORE IT
        JNC  RD9                ;NORMAL BYTE
        PUSH H                  ;CARRY=RELOCATE NEXT WORD
        LXI  H,5                ;POINT TO BASE[1]
        DAD  SP                 ;IN STACK
RD7:    CALL RD10               ;GET HIGH BYTE
        JNC  RD8                ;USE BASE[N]
        DCR  E                  ;COUNT EXTRA BYTE
        XTHL                    ;GET LOAD ADDR
        DCR  M                  ;TEST FOR BASE[1]
        MOV  M,A                ;NEW LOW BYTE
        XTHL                    ;SAVE LOAD AGAIN
        JZ   RD7                ;BASE[1]
        INX  H
        INX  H                  ;POINT TO BASE[2]
        JMP  RD7                ;AND TRY AGAIN
;
RD8:    ADD  M                  ;ADD IN MSB
        XTHL
        INX  H                  ;STICK AT LOAD+1
        MOV  M,A
        DCX  H                  ;GET LOAD BYTE
        MOV  A,M                ;IN A
        XTHL
        DCX  H
        ADD  M                  ;RELOCATE LSB
        POP  H                  ;GET LOAD ADDR
        MOV  M,A                ;STORE IT
        INX  H                  ;GET MSB
        MOV  A,M                ;IN A
        ACI  0                  ;ADJUST FOR CARRY
        MOV  M,A                ;STORE IT
        DCR  E                  ;COUNT IT
RD9:    INX  H                  ;BUMP THE COUNT
        DCR  E                  ;MORE?
        JNZ  RD6                ; & CONTINUE
        JMP  RD4                ;TEST CHECKSUM
;
RD10:   DCR  B                  ;COUNT BITS/BYTES
        JNZ  RD11               ;NEXT IS DATA BYTE
        CALL BYTE               ;GET RELOC. MAP
        DCR  E                  ;BUMP DOWN BYTE COUNT
        MOV  C,A                ;MAP IN C
        MVI  B,8                ;RESET FOR NEXT 8
RD11:   CALL BYTE               ;NEXT DATA BYTE
        PUSH D                  ;SAVE DE
        MOV  D,A                ;SAVE DATA BYTE
        MOV  A,C                ;TEST FOR RELOC.
        RAL                     ;IN CARRY FLAG
        MOV  C,A                ;UPDATE C
        MOV  A,D                ;RESTORE DATA BYTE
        POP  D                  ;RESTORE DE
        RET                     ;CONTINUE
;
BYTE:   PUSH B                  ;SAVE BC
        CALL RIBBLE             ;GET A CONVERTED CHAR.
        RLC
        RLC
        RLC
        RLC                     ;MOVE IT TO HIGH NIBBLE
        MOV  C,A                ;SAVE IT
        CALL RIBBLE             ;GET OTHER HALF
        ORA  C                  ;MAKE WHOLE
        MOV  C,A                ;SAVE IN C
        ADD  D                  ;UPDATE CHECKSUM
        MOV  D,A                ;NEW CHECKSUM
        MOV  A,C                ;RESTORE DATA BYTE
        POP  B                  ;RESTORE BC
        RET                     ;CONTINUE
;
DONE:   POP  B                  ;BASE[1]
        POP  B                  ;BASE[2]
        MOV  A,H                ;TEST EOF
        ORA  L                  ;FOR ZERO
        RZ
        XCHG                    ;ELSE STORE IT IN 'P'
        LXI  H,PLOC
        DAD  SP
        MOV  M,D                ;IN 'EXIT' TEMPLATE
        DCX  H
        MOV  M,E
        RET                     ;REALLY DONE.
;
SZR     EQU  $-READ
;
SUBS:   CALL EXPR
        POP  H
        RC                      ;QUIT
SU0:    MOV  A,M
        CALL LBYTE
        CALL COPCK
        RC
        JZ   SU1
;       CPI  '_'                ;BACK-UP?
        CPI  5FH                ;*UM*
        JZ   SU3
        PUSH H
        CALL EXF
        POP  D
        POP  H
        MOV  M,E
        RC
SU1:    INX  H
SU2:    MOV  A,L
        ANI  7
        CZ   LFADR
        JMP  SU0
SU3:    DCX  H                  ;BACK-UP
        JMP  SU2
;
SZS     EQU  $-SUBS
;
TYPE:   MVI  C,64               ;SET UP A DEFAULT
        CALL EXPC
        PUSH PSW
TY0:    CALL LFADR
        POP  PSW
        PUSH PSW
        MOV  B,A                ;RESET LENGTH
TY1:    MOV  A,M
        ANI  7FH
        CPI  ' '                ;TEST LOWER END
        JNC  TY3
TY2:    MVI  A,'.'              ;PRINT PERIODS INSTEAD
TY3:    CPI  7DH                ;TEST UPPER END
        JNC  TY2
        MOV  C,A                ;PUT WHATEVER INTO C
        CALL CO
        CALL HILO
        JC   PRET
        DCR  B
        JNZ  TY1
        JMP  TY0
;
SZT     EQU  $-TYPE
;
VERIFY: CALL EXPC
VE0:    LDAX B
        PUSH D                  ;SAVE END POINTER
        MOV  E,M                ;GET MEMORY DATA
        CMP  E                  ;TEST FOR MATCH
        JZ   VE1                ;MATCHES
        PUSH B
        MOV  B,A
        CALL HLSP
        MOV  A,E                ;GET MISMATCH
        CALL LBYTE              ;PRINT IT
        CALL BLK                ;SPACE OVER
        MOV  A,B                ;GET OTHER MISMATCH
        CALL LBYTE              ;PRINT THAT TOO
        CALL CRLF               ;PREPARE FOR ANOTHER
        POP  B
VE1:    POP  D                  ;RESTORE END POINTER
        INX  B
        CALL HILOX
        JMP  VE0
;
SZV     EQU  $-VERIFY
;
WRITE:  CALL EXPC
        CALL WAIT
WR0:    CALL PEOL
        LXI  B,':'
        CALL PO
        PUSH D
        PUSH H
WR1:    INR  B
        CALL HILO
        JC   WR2
        MVI  A,24
        SUB  B
        JNZ  WR1
        POP  H
        CALL WR3
        POP  D
        JMP  WR0
WR2:    POP  H
        POP  D
WR3:    MOV  A,B
        CALL PBYTE              ;PUNCH FILE SIZE
        CALL PADR               ;AND ADDR.
        MOV  A,B                ;SET-UP CHECKSUM
        ADD  H
        ADD  L
        MOV  D,A                ;CHECKSUM IN D
        XRA  A                  ;ZERO FILE TYPE
        CALL PBYTE
WR4:    MOV  A,M
        ADD  D                  ;UPDATE CHECKSUM
        MOV  D,A
        MOV  A,M
        CALL PBYTE
        INX  H
        DCR  B
        JNZ  WR4
        XRA  A
        SUB  D
        JMP  PBYTE
;
SZW     EQU  $-WRITE
;
XAM:    CALL PCHK
        LXI  H,ACTBL            ;POINT TO REG. TABLE
        MVI  B,ACTSZ            ;SET UP B
        JC   XA6
XA0:    CMP  M                  ;VALID REG. NAME?
        JZ   XA1                ; YES
        INX  H                  ;ELSE TEST NEXT ONE
        INX  H                  ;SKIP OFFSET
        DCR  B                  ;END OF TABLE?
        JZ   ERROR              ; YES
        JMP  XA0                ;ELSE KEEP LOOKING
XA1:    CALL BLK
XA2:    CALL XA8                ;GET & PRINT REG(S)
XA3:    CALL COPCK              ;MODIFY?
        JZ   XA5                ; NO, DELIMITER ENTERED
        PUSH H                  ;SAVE TABLE POINTER
        PUSH B                  ;SAVE FLAG TEST (B)
        CALL EXF                ;GET NEW VALUE
        POP  H                  ;IN HL
        POP  B                  ;B=FLAG BYTE
        PUSH PSW                ;A=DELIMITER
        MOV  A,L                ;L=LOW BYTE
        STAX D                  ;STORE IT
        MOV  A,B                ;GET FLAG
        RAL                     ;TEST BIT 7
        JNC  XA4                ;SINGLE BYTE
        INX  D                  ;ELSE
        MOV  A,H                ; SAVE
        STAX D                  ;  HIGH BYTE
XA4:    POP  PSW                ;GET DELIMITER
        POP  H                  ;RESTORE TABLE POINTER
XA5:    RC                      ;CR=DONE
        MOV  A,M                ;END OF TABLE?
        ORA  A                  ;TEST BIT 7
        RM                      ;YES, DONE
        JMP  XA2                ;ELSE CONTINUE
;
XA6:    CALL CRLF               ;FULL REGISTER DISPLAY
XA7:    CALL BLK                ;SPACE OVER
        MOV  A,M                ;GET REGISTER NAME
        ORA  A                  ;END OF TABLE?
        RM                      ;YES, RETURN
        MOV  C,A                ;ELSE PRINT IDENTIFIER
        CALL CO                 ; ON CONSOLE
        MVI  C,'='              ;FOR READABILITY
        CALL CO
        CALL XA8                ;GET & PRINT REG(S)
        JMP  XA7
XA8:    INX  H                  ;POINT TO DISPLACEMENT
        MOV  A,M                ;GET IT
        INX  H                  ;POINT TO NEXT IN TABLE
        XCHG                    ;SAVE IN DE
        MOV  B,A                ;SAVE FOR FLAGS
        ANI  3FH                ;KILL FLAGS
        MOV  L,A                ;CALCULATE DISPLACEMENT
        MVI  H,0
        DAD  SP                 ;UP IN STACK
        INX  H                  ;ADJUST FOR RET IN STACK
        INX  H
        MOV  A,B                ;TEST FOR 'M'
        ANI  40H                ;BIT 6
        JZ   XA9                ;NO, NOT 'M'
        MOV  A,M                ;ELSE GET 'M' POINTER
        DCX  H                  ; INSTEAD
        MOV  L,M                ;  IN HL
        MOV  H,A                ;   (WHERE ELSE)
XA9:    MOV  A,M                ;GET THE VALUE
        CALL LBYTE              ;AND PRINT IT
        XCHG                    ;SWITCH POINTERS
        MOV  A,B                ;TEST FLAG
        RAL                     ;SINGLE OR DOUBLE?
        RNC                     ;SINGLE
        DCX  D                  ;DOUBLE
        LDAX D                  ;GET IT
        JMP  LBYTE              ;PRINT IT & RETURN
;
SZX     EQU  $-XAM
;
QUERY:  CALL TI                 ;SEE IF IN OR OUT
        LXI  H,QLOC             ;PRESET
        DAD  SP                 ;TO ROUTINE IN EXIT AREA
        PUSH H                  ;FOR BOTH ROUTINES
        CPI  'O'                ;OUT?
        JNZ  QI                 ; NO, MUST BE IN
        CALL EXPC               ;GET PORT & VALUE
        MOV  A,E                ;L=PORT E=VALUE
        MOV  C,L
        POP  H
        MOV  M,C
        DCX  H
        MVI  M,0D3H             ;SET FOR OUTPUT
        PCHL                    ;DO IT & RETURN
;
QI:     CPI  'I'
        JNZ  ERROR
        CALL EXPR
        POP  B
        LXI  H,BITS             ;SET-UP A RETURN
        XTHL
        MOV  M,C                ;SET PORT NUMBER
        DCX  H
        MVI  M,0DBH             ;SET FOR INPUT
        PCHL                    ;DO IT
;
SZQ     EQU  $-QUERY
;
SIZE:   CALL MEMSIZ
;
LFADR:  CALL CRLF
;
HLSP:   CALL LADR
;
BLK:    MVI  C,' '
;
CO:     LDA  -1
        ANI  NOT CMSK
        JZ   CRTOUT
        DCR  A
        JNZ  COU
;
TTYOUT: IN   TTS
        ANI  TTYBE
        JNZ  TTYOUT
        MOV  A,C
        OUT  TTO
        RET
;
CRTOUT: IN   CRTS
        ANI  CRTBE
        JNZ  CRTOUT
        MOV  A,C
        OUT  CRTO
        RET
;
COU:    DCR  A                  ;BATCH
        JNZ  COLOC              ;NO
;
LO:     LDA  -1
        ANI  NOT LMSK
        JZ   CRTOUT             ;USE MAIN CONSOLE
        CPI  LCRT
        JZ   TTYOUT             ;USE PRINTER
        CPI  LINE
        JNZ  LULOC              ;MUST BE USER DEFINED
                                ;ELSE USE DATA TRANSFER
LNLOC:  IN   RCSS
        ANI  PCSBE
        JNZ  LNLOC
        MOV  A,C
        OUT  PCASO
        RET
;
CONV:   ANI  0FH
        ADI  90H
        DAA
        ACI  40H
        DAA
        MOV  C,A
        RET
;
BITS:   MOV  E,A
        MVI  D,8
        CALL BLK
BI:     MOV  A,E
        RAL
        MOV  E,A
        MVI  A,0
        ACI  '0'
        MOV  C,A
        CALL CO
        DCR  D
        JNZ  BI
;
CRLF:   PUSH H
        PUSH B                  ;SAVE BC
        MVI  B,5
        CALL TOM
        POP  B
        POP  H
        RET
;
CSTS:   LDA  -1
        ANI  NOT CMSK
        JZ   CS1                ;CRT
        DCR  A
        JZ   CS0                ;TTY
        DCR  A
        RZ                      ;BATCH MODE
        JMP  CSLOC              ;USER
;
CS0:    IN   TTS
        ANI  TTYDA
        JMP  CS2
;
CS1:    IN   CRTS
        ANI  CRTDA
CS2:    MVI  A,TRUE
        RZ
        CMA
        RET
;
; THIS ROUTINE WILL GET TWO PARAMETERS
; FROM THE KEYBOARD, AND RETURN WITH THE
; 'C' REEGISTER IN A, & CARRY SET IF THE
; TERMINATOR WAS A CARRIAGE RETURN. OTHERWISE,
; IT WILL GET THE THIRD PARAMETER. IF THE
; THIRD PARAMETER IS NON-ZERO, IT WILL RETURN
; WITH THE THIRD PARAMETER IN 'A'. IF IT IS
; ZERO, IT WILL RETURN WITH THE DEFAULT PARAM.
; - IN EITHER CASE, IF THREE PARAMETERS WERE
; ENTERED, IT WILL RETURN WITH THE CARRY CLEAR.
;
EXPC:   PUSH B                  ;SAVE DEFAULT PARAMETER
        CALL EXPR               ;GET 1ST.
        JC   ERROR              ;CR ENTERED TOO SOON
        CALL EXPR               ;GET 2ND. PARAMETER
        POP  D                  ;2ND. IN DE
        POP  H                  ;1ST. IN HL
        POP  B                  ;REMOVE DEFAULT
        PUSH H                  ;SAVE 1ST. PARAMETER
        MOV  A,C                ;USE DEFAULT
        JC   EX1                ;NO THIRD PARAMETER
        PUSH B                  ;SAVE DEFAULT AGAIN
        CALL EXPR               ;GET 3RD. PARAMETER
        POP  B                  ;BC=TRUE 3RD. PARAMETER
        MOV  A,C                ;TEST IT
        POP  H                  ;HL=DEFAULT
        ORA  A                  ;TEST LOW BYTE
        JNZ  EX1                ;OK, TAKE IT
        MOV  A,L                ;ELSE USE DEFAULT
EX1:    POP  H                  ;GET 1ST. PARAM
        PUSH PSW                ;SAVE ACC & FLAGS
        CALL CRLF
        POP  PSW
        RET
;
; THIS ROUTINE RETURNS ONLY IF THREE PARAMETERS
; WERE ENTERED. LESS THAN THREE RESULTS IN AN
; ERROR CONDITION.
;
EXP3:   CALL EXPC               ;GET THREE PARAMETERS
        JC   ERROR              ;I SAID 3
        RET
;
EXPR:   CALL TI                 ;GET KEYBOARD
EXF:    LXI  H,0                ;INITIALIZE HL
XF1:    MOV  B,A                ;SAVE KEYBOARD
        CALL NIBBLE             ;CONVERT ASCII TO HEX
        JC   XF2                ;BOT LEGAL
        DAD  H                  ;HL*16
        DAD  H
        DAD  H
        DAD  H
        ORA  L                  ;ADD IN NIBBLE
        MOV  L,A
        CALL TI                 ;GET NEXT KEYBORAD
        JMP  XF1                ;AND CONTINUE
XF2:    XTHL                    ;STICK PARAMETER IN STACK
        PUSH H                  ;REPLACE RETURN
        MOV  A,B                ;TEST CHARACTER
        CALL QCHK               ;FOR DELIMITERS
        JNZ  ERROR              ;ILLEGAL
        RET
;
HILOX:  CALL HILO
        RNC                     ;RETURN IF OK
PRET:   POP  D                  ;ELSE RETURN
        RET                     ; ONE LEVEL BACK
;
HILO:   INX  H
        MOV  A,H
        ORA  L
        STC
        RZ
        MOV  A,E
        SUB  L
        MOV  A,D
        SBB  H
        RET
;
HEXN:   CALL EXPC
        PUSH H
        DAD  D
        CALL HLSP
        POP  H
        MOV  A,L
        SUB  E
        MOV  L,A
        MOV  A,H
        SBB  D
        MOV  H,A
;
SZH     EQU  $-HEXN
;
LADR:   MOV  A,H
        CALL LBYTE
        MOV  A,L
;
LBYTE:  PUSH PSW
        RRC
        RRC
        RRC
        RRC
        CALL LB
        POP  PSW
LB:     CALL CONV
        JMP  CO
;
MARK:   LXI  B,08FFH            ;PRESET FOR RUB-OUTS
        JMP  LEED
;
LEAD:   LXI  B,4800H            ;PRESET FOR NULLS
LEED:   CALL PO
        DCR  B
        JNZ  LEED
        RET
;
RIBBLE: CALL RIX
NIBBLE: SUI  '0'
        RC
        CPI  'G'-'0'
        CMC
        RC
        CPI  10
        CMC
        RNC
        SUI  'A'-'9'-1
        CPI  10
        RET
;
PADR:   MOV  A,H
        CALL PBYTE
        MOV  A,L
;
PBYTE:  PUSH PSW
        RRC
        RRC
        RRC
        RRC
        CALL PBL
        POP  PSW
PBL:    CALL CONV
        JMP  PO
;
COPCK:  MVI  C,'-'
        CALL CO
;
PCHK:   CALL TI
;
QCHK:   CPI  ' '
        RZ
        CPI  ','
        RZ
        CPI  CR
        STC
        RZ
        CMC
        RET
;
PEOL:   MVI  C,CR
        CALL PO
        MVI  C,LF
;
PO:     LDA  -1
        ANI  NOT PMSK
        JZ   LNLOC              ;DATA XFER DEVICE
        CPI  PTTY
        JZ   TTYOUT             ;PRINTER DEVICE
        CPI  PCAS
        JNZ  PULOC              ;USER DEFINED
;
PTPL:   IN   PPSTAT             ;PARALLEL PORT
        ANI  PPBE
        JNZ  PTPL
        MOV  A,C
        OUT  PPDATA
        RET
;
UNLD:   CALL EXPC
        CALL WAIT
        CALL LEAD
        CALL MARK
UL1:    MOV  C,M
        CALL PO
        CALL HILO
        JNC  UL1
        CALL MARK
;
SZU     EQU  $-UNLD
;
NULL:   CALL LEAD
;
SZN     EQU  $-NULL
;
WAIT:   LDA  -1
        ANI  NOT CMSK
        RZ
;
CI:     LDA  -1
        ANI  NOT CMSK
        JZ   CRTIN
        DCR  A
        JNZ  CIU
;
TTYIN:  IN   TTS
        ANI  TTYDA
        JNZ  TTYIN
        IN   TTI
        RET
;
CRTIN:  IN   CRTS
        ANI  CRTDA
        JNZ  CRTIN
        IN   CRTI
        RET
;
CIU:    DCR  A                  ;BATCH?
        JNZ  CILOC              ; NO, MUST BE USER
;
RI:     LDA  -1
        ANI  NOT RMSK
        OUT  TTS                ;PULSE A PORT TO SHOW REQUEST
        JNZ  RI3                ;NEXT
;DATA XFER
RI4:    CALL RI2                ;ABORT?
        IN   RCSS
        ANI  RCSDA
        JNZ  RI4
        IN   RCSD
        RET
;
RI3:    CPI  RTTY               ;IS IT PRINTER
        JNZ  RI5                ;NEXT
;PRINTER
RI1:    CALL RI2                ;SEE IF ABORT
        IN   TTS
        ANI  TTYDA
        JNZ  RI1
        IN   TTI
        RET
;
RI5:    CPI  RCAS
        JNZ  RULOC              ;USER DEFINED
;PARALLEL PORT
RI6:    CALL RI2
        IN   PPSTAT
        ANI  PPDA
        JNZ  RI6
        IN   PPDATA
        RET
;
RI2:    LDA  -1                 ;MAKE SURE CONSOLE=0
        ANI  NOT CMSK
        RNZ
        CALL CSTS               ;ANYTHING WAITING THERE?
        ORA  A
        RZ                      ;NO, CONTINUE
        CALL KI                 ;ELSE GET IT
        CPI  3                  ;CONTROL-C?
        RNZ
        POP  PSW                ;ELSE RETURN
        XRA  A                  ;WITH CARRY SET
        STC
        RET
;
RIX:    CALL RIFF
        ANI  7FH
        RET
;
RIFF:   CALL RI
        JC   ERROR
        CMP  D
        RET
;
KI:     CALL CI                 ;GET CONSOLE CHARACTER
        ANI  7FH                ;KILL PARITY BIT
        RET
;
TI:     CALL KI
        RZ
        CPI  7FH
        RZ                      ;TEST FOR RUB-OUT
        CPI  CR                 ;IGNORE CR'S
        RZ
        PUSH B
        MOV  C,A
        CALL CO
        MOV  A,C
        POP  B
        CPI  'A'-1              ;CONVERT TO UPPER CASE
        RC
;       CPI  'z'+1
        CPI  7BH                ;*UM*
        RNC
        ANI  05FH
        RET
;
;
; <SYSTEM I/O LOOK-UP TABLE>
;
; THE FIRST CHARACTER IS THE DEVICE NAME
; (ONE LETTER) AND THE NEXT FOUR ARE THE
; NAMES OF THE FOUR POSSIBLE DRIVERS TO BE
; ASSIGNED.
;
LTBL:
;
        DB   'C'                ;CONSOLE ASSIGNMENTS
;
        DB   'C'                ;CRT
        DB   'P'                ;PRINTER
        DB   'B'                ;BATCH= COMMANDS FROM READER
        DB   'U'                ;CUSE   USER
;
;
        DB   'R'                ;READER ASSIGNMENTS
;
        DB   'D'                ;DATA TRANSFER DEVICE
        DB   'P'                ;PRINTER
        DB   'A'                ;ALTERNATE (PARALLEL)
        DB   'U'                ;RUSER  USER
;
;
        DB   'P'                ;PUNCH ASSIGNMENTS
;
        DB   'D'                ;DATA TRANSFER DEVICE
        DB   'P'                ;PRINTER
        DB   'A'                ;ALTERNATE (PARALLEL)
        DB   'U'                ;PUSER  USER
;
;
        DB   'L'                ;LIST ASSIGNMENTS
;
        DB   'C'                ;CRT
        DB   'P'                ;PRINTER
        DB   'D'                ;DATA TRANSFER DEVICE
        DB   'U'                ;LUSER  USER
;
EXIT:
        POP  D
        POP  B
        POP  PSW
        POP  H
        SPHL
        NOP                     ;COULD BE EI
        LXI  H,0
HLX     EQU  $-2
        JMP  0
PCX     EQU  $-2
T1A:    DW   0
        DB   0
        DW   0
        DB   0
QIO:
        IN   0
        RET
;
ENDX:
;
ALOC    EQU  7
BLOC    EQU  5
CLOC    EQU  4
DLOC    EQU  3
ELOC    EQU  2
FLOC    EQU  6
HLOC    EQU  HLX-EXIT+11
LLOC    EQU  HLX-EXIT+8
PLOC    EQU  PCX-EXIT+11
SLOC    EQU  9
TLOC    EQU  T1A-EXIT+8
TLOCX   EQU  TLOC+2
QLOC    EQU  QIO-EXIT+11
;
ACTBL:
        DB   'A', ALOC
        DB   'B', BLOC
        DB   'C', CLOC
        DB   'D', DLOC
        DB   'E', ELOC
        DB   'F', FLOC
        DB   'H', HLOC
        DB   'L', LLOC+2
        DB   'M', HLOC OR 040H
        DB   'P', PLOC OR 080H
        DB   'S', SLOC OR 080H
;
ACTSZ   EQU  ($-ACTBL)/2
;
        DB   -1                 ;TABLE DELIMITER
;
        DB   'RWA'              ;AUTHOR
;       DB   '(C) 1979 ECT'
;
Z:                              ;END OF PROGRAM
;
;
        END

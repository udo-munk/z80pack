;
;       CROMEMCO Z-1 MONITOR SOURCE
;
;       RETYPED FROM MANUAL AND MODIFIED TO ASSEMBLE WITH INTEL MACRO-80
;       DECEMBER 2014, UDO MUNK
;
;PPAGE   EQU     0E1H            ;MUST BE THE HIGHER OF A PAIR OF NON-RAM PAGES.
PPAGE   EQU     0FFH            ;PROM CAN'T BE DETECTED YET!
;PSW    EQU     6               ;DEFINED BY INTEL ASSEMBLER ALREADY
;SP     EQU     6
PF      EQU     80H             ;PRIME-ABLE REG FLAG
R2F     EQU     40H             ;2-BYTE REG FLAG
BELL    EQU     07
ESC     EQU     1BH
CR      EQU     0DH
LF      EQU     0AH
STAT    EQU     0
DAV     EQU     40H
TBE     EQU     80H
DATA    EQU     1
TEMPS   EQU     16H             ;ROOM FOR TEMP STORAGE
RSTLC   EQU     30H             ;RST LOCATION
CASE    EQU     20H             ;DIFF BETW LOWER & UPPER CA
;
; Z80 OP-CODES
JR      EQU     18H
JRC     EQU     38H
JRNC    EQU     30H
JRZ     EQU     28H
JRNZ    EQU     20H
DJNZ    EQU     10H
EXAF    EQU     08              ;EX AF,AF'
EXX     EQU     0D9H
RLD     EQU     0EDH
RLD1    EQU     6FH
CPI0    EQU     0EDH
CPI1    EQU     0A1H
CPIR    EQU     0EDH
CPIR1   EQU     0B1H
LDI     EQU     0EDH
LDI1    EQU     0A0H
LDIR    EQU     0EDH
LDIR1   EQU     0B0H
LDD     EQU     0EDH
LDD1    EQU     0A8H
LDDR    EQU     0EDH
LDDR1   EQU     0B8H
SET5A   EQU     0CBH
ST5A1   EQU     0EFH
;
IX      EQU     0DDH
IY      EQU     0FDH
;
; DISPLACEMENTS FROM IX OF HI BYTE OF REG PAIRS
DUPC    EQU     0
DUSP    EQU     -2
DUAF    EQU     -4
DUBC    EQU     -6
DUDE    EQU     -8
DUHL    EQU     -10
DUIT    EQU     -12             ;USER I & INTERRUPT ENABLE
DUIX    EQU     -14
DUIY    EQU     -16
DUAF2   EQU     -18
DUBC2   EQU     -20
DUDE2   EQU     -22
DUHL2   EQU     -24

        ORG     0E000H
START:
;
; ENTER MONITOR FROM RESET
;
        MVI     A,1
        OUT     40H             ;SELECT BANK 0
;
; PLACE SYS STACK AT HIGHEST PAGE OF AVAILABLE RAM.
; ALLOW ROOM FOR TEMP STORAGE.
;
        LXI     H,00FFH-TEMPS+2
INIT:   DCR     H
        MOV     A,M
        INR     M
        CMP     M               ;DID IT CHANGE?
        DB      JRZ
        DB      INIT-$-1
        DCR     M               ;YES. RESTORE IT.
;
; HL NOW POINTS TO BP STACK END
;
        MVI     M,0             ;BP STACK END MARK
        MOV     A,L             ;SAVE
        DCX     H               ;STORAGE FOR BPSP,LO
        MOV     M,A             ;STORE BPSP,LO
        LXI     D,DUHL2-2
        DAD     D               ;TO END OF REG STORAGE
        SPHL                    ;SYS SP
;
        DB      0EDH            ;SBC HL,DE: BACK TO UPC;HI
        DB      52H             ;(CY WAS SET BY 'DAD D')
        PUSH    H
        DB      IX
        POP     H               ;POP IX: STORAGE PNTR
;
        MVI     D,PPAGE         ;FORCE USER SP TO
        DCX     H
        DCX     H
        MOV     M,D             ;POINT TO PROM
;
; SET BAUD RATE
;
INIT1:  MVI     A,0D8H          ;300 BAUD
        CALL    BAUD
        MVI     A,0F4H          ;110 BAUD
        CNZ     BAUD
        DB      JRNZ
        DB      INIT1-$-1
;
        LXI     H,HEAD          ;HEADING
        CALL    PMSG
;
        DB      JR
        DB      CMND-$-1
;
BAUD:   OUT     STAT            ;SET BAUD RATE
        CALL    GBYTE
        CALL    GBYTE           ;CAN WE
        ANI     7FH             ;READ
        CPI     CR              ;A CR?
        RET
;
; ENTER MONITOR FROM BRKPT
;
; SAVE MACHINE STATE. SAVES ALL REGS INCLUDING
; SP, FINDS THE TOP OF RAM INSTALLED IN MACHINE
; & SWITCHES THE STACK THERE.
;
SVMS:   XTHL                    ;ADJUST BRKPT RET ADDR
        DCX     H
        XTHL
;
        PUSH    H               ;SAVE
        LXI     H,4
        DAD     SP              ;USP (USER-SP)
        XTHL                    ;TO STACK
;
        PUSH    PSW             ;UAF
        PUSH    B               ;UBC
        PUSH    D               ;UDE
        PUSH    H               ;UHL
;
; FIND SYS STACK AGAIN
;
        LXI     H,00FFH-TEMPS
SVMS1:  DCR     H               ;DECRM MEM PAGE
        MOV     A,M
        INR     M
        CMP     M               ;DID IT CHANGE?
        DB      JRZ
        DB      SVMS1-$-1
        DCR     M               ;YES. RESTORE IT.
;
        XCHG
        LXI     H,11
        DAD     SP              ;PNTS TO BPRA, HI BYTE
        LXI     B,12
        DB      LDDR            ;TRANSFER TO SYS STACK
        DB      LDDR1
        INX     D               ;DE HAS CURRENT VALUE OF SYS SP AND POINTS TO UR
        INX     H               ;HL HAS CURRENT VALUE OF USER SP AND ALSO POINTS
                                ;TO UR
        XCHG
        SPHL                    ;SYS SP
;
        DB      0EDH            ;LD A,I
        DB      57H
        MVI     C,0
       	JPO     SVMS3           ;IFF?
        INR     C               ;C NOW HOLDS USER-IFF
SVMS3:  MOV     B,A
        PUSH    B               ;UIF (USER-I & USER-IFF)
;
        DB      IX
        PUSH    H               ;PUSH IX: UIX
        DB      IY
        PUSH    H               ;PUSH IY: UIY
        LXI     B,DUPC-DUHL+1
        DAD     B               ;PNTS TO UPC, HI BYTE
        PUSH    H
        DB      IX
        POP     H               ;TO IX (POINTS TO UPC)
;
        DB      EXAF
        PUSH    PSW
        DB      EXX
        PUSH    B               ;UBC2
        PUSH    D               ;UDE2
        PUSH    H               ;UHL2
;
        DB      IX
        PUSH    H               ;PUSH IX
        POP     H
        INX     H               ;POINTS TO BPSP,LO
        MOV     L,M             ;BPSP NOW IN HL
;
; CLEAR ALL BRKPTS
;
CLBP1:  MOV     A,M             ;BP STK EMPTY?
        ORA     A
        DB      JRZ
        DB      CLBP2-$-1
;
        DCX     H
        MOV     D,M
        DCX     H
        MOV     E,M
        DCX     H
        MOV     A,M
        STAX    D               ;RESTORE CONTENTS TO MEM
        DCX     H
        DB      JR
        DB      CLBP1-$-1
;
CLBP2:  MOV     A,L
        DCX     H
        MOV     M,A             ;ADJUST BPSP
;
        CALL    DSPR            ;DISPLAY USER REGISTERS
;
; GET 1-BYTE COMMAND.
; RETURNS VALUE IN HL & JUMPS TO THAT ADDR.
;
CMND:   CALL    CRLF
CMND1:  LXI     H,PRMPT
        CALL    PMSG
; HL NOW PNTS TO TBL ADDR
        CALL    GCMND           ;DE GETS LETTER - 'A'
        XCHG
        DAD     H               ;TIMES 2
        DAD     D               ; + TBL ADDR
        MOV     E,M
        INX     H
        MOV     D,M
        XCHG
        LXI     D,CMND1         ;SET UP RETURN
        PUSH    D               ;TO CMND
        MOV     A,C             ;A & C HAVE CMND DELIMITER
        PCHL
;
; REJECTS ALL BUT ALPHABETIC CHARACTERS.
; RETURNS THE CHAR LESS THE ASCII VALUE OF 'A'.
;
ABCYZ:  SUI     'A'+CASE        ;'A' OR ABOVE?
        DB      JRC
        DB      ERROR-$-1
        CPI     25D             ;'Y' OR BELOW?
        RC                      ;IF NOT, CONTINUE BELOW
;
; ERROR & ESCAPE. RETURNS TO CMND WITH SP
; POINTING TO SAVED-REG AREA (UHL2).
;
ERROR:  CALL    PSQS            ;PRINT '? <BELL>'
ESCPE:  DB      IX
        PUSH    H               ;PUSH IX
        POP     H
        LXI     D,DUHL2-1-DUPC
        DAD     D
        SPHL
        DB      JR
        DB      CMND-$-1        ;GET NEW CMND
;
; PROGRAM PROMS. ABORTS IF DESTINATION
; IS NOT ON A 1K (400H) BOUNDARY, SWATH
; WIDTH IS NOT A MULTIPLE OF 1K.
;
PROG:   MVI     B,181           ;360 ITERATION
PROG1:  PUSH    B               ;SAVE # OF ITERATIONS
        CALL    LD2N            ;SOURCE TO DE,INCRM TO BC,
        PUSH    PSW             ;SAVE LATEST DELIMITER
        MOV     A,B             ;IS INCRM A MULT OF 1024?
        ANI     3
        ORA     C
        DB      JRNZ
        DB      ERROR-$-1
        POP     PSW             ;LAST DELIMITER
        CALL    LINCR           ;SOURCE TO HL, DEST TO DE
        MOV     A,D             ;IS DEST A MULT OF 1024?
        ANI     3
        ORA     E
        DB      JRNZ
        DB      ERROR-$-1
;
PROG3:  POP     PSW             ;ITERATION
        PUSH    PSW
        PUSH    B               ;INCREMENT
        LXI     B,1024
        PUSH    B               ;SAVE
        CALL    MVE             ;MOVE IT
        POP     B               ;RETRIEVE
        XTHL                    ;INCRM TO HL
        ORA     A               ;RESET CY
        DB      0EDH            ;SBC HL,BC
        DB      42H
        XTHL                    ;SOURCE BACK TO HL
        POP     B               ;NEW INCRM
        DB      JRNZ
        DB      PROG3-$-1       ;LOOP IF INCRM NOT 0
        POP     PSW             ;CLEAN UP
        RET                     ;BACK TO CMND
;
; COMMAND
;
; DISPLAY THE USER REGISTERS.
;
DSPR:   CALL    CRLF
        DB      IX
        PUSH    H               ;PUSH IX
        POP     H               ;POINTS TO UPC
        MVI     B,2             ;UPC & USP
        CALL    PREGS
        MVI     B,7             ;UAF THRU UIY
        CALL    PREGS
        MVI     B,4             ;UAF2 THRU UHL2
PREGS:  CALL    P2BMS           ;PRINT 2 BYTES PNTED TO B
        DB      DJNZ
        DB      PREGS-$-1
; (CONTINUE BELOW)
;
; PRINT CR & LF. PRESERVES ALL REGS BUT A.
;
CRLF:   MVI     A,CR
; (CONTINUE BELOW)
;
; PRINT THE CHARACTER IN THE A-REGISTER. (CHECKS
; INPUT FOR ESCAPE.) PRESERVES ALL REGS.
PCHR:   PUSH    PSW             ;SAVE THE CHAR
        IN      STAT
        ANI     DAV
        DB      JRZ
        DB      PCHR2-$-1
        IN      DATA
        ANI     7FH
;
PCHR1:  CPI     ESC
        DB      JRZ
        DB      ESCPE-$-1
;
PCHR2:  IN      STAT
        ANI     TBE
        DB      JRZ
        DB      PCHR2-$-1
        POP     PSW
        OUT     DATA
        PUSH    PSW
        PUSH    H
        LXI     H,LFNN
        CPI     CR
        CZ      PMSG
        POP     H
        POP     PSW
        RET
;
; GET CHARATER. RETURNS IT IN A. CONVERTS
; ALPHA CHARS TO LOWER-CASE. ALTERS F.
;
GCHR:   CALL    GBYTE
        ANI     7FH
        CPI     'A'
        DB      JRC
        DB      GCHR1-$-1
        ORI     20H             ;CONVERT TO LOWER-CASE
GCHR1:  PUSH    PSW             ;SAVE THE CHAR
        DB      JR
        DB      PCHR1-$-1       ;PRINT IT
;
GBYTE:  IN      STAT
        ANI     DAV
        DB      JRZ
        DB      GBYTE-$-1
        IN      DATA
        RET
;
; PRINT 2 BYTES IN (HL) & (HL - 1).
; DECREMENTS HL BY 2. ALTERS A. PRESERVES OTHERS
;
P2BMS:  CALL    PNM
        DCX     H
        CALL    PNM
        DCX     H
;
; PRINTS SPACE. PRESERVES ALL REGS BUT A.
;
SPACE:  MVI     A,20H
        DB      JR
        DB      PCHR-$-1
;
; IF HL IS A MULTIPLE OF 16, DO PADDR.
;
CK16B:  MVI     A,15
;
; ENTER WITH A CONTAINING N. IF HL IS A MULTIPLE
; OF N+1, DO PADDR.
;
CKBND:  ANA     L
        RNZ
;
; PRINT THE NUMBER IN HL, FOLLOWED BY A COLON.
; PRESERVES ALL REGS EXCEPT A.
;
PADDR:  CALL    CRLF
PADR1:  CALL    PNHL
        MVI     A,':'
        DB      JR
        DB      PCHR-$-1
;
; LOAD TWO NUMBERS. FOLLOW WITH A CRLF.
;
L2NCR:  CALL    LD2N
;
; SKIP INITIAL SPACES.
; IF DELIMITER NOT A CR, ERROR
;
SKSGC:  CALL    SKSG            ;LOOK FOR A NON-SPACE
        CPI     CR              ;CR?
        JNZ     ERROR
        RET
;
; LOAD TWO NUMBERS. LOADS DE WITH THE BEGINNING
; ADDR, N1. LOADS BC & HL WITH THE INCREMENT
; N2-N1+1 (OR WITH N2 IF THE OPR IS 'S').
; RETURN WITH LAST DELIMITER IN A.
;
LD2N:   CALL    GNHL            ;N1 TO HL, DELIMITER TO A
        XCHG                    ;SAVE N1 IN DE
        CALL    SKSG            ;GET NEXT NON-SPACE CHAR
        CPI     'S'+CASE        ;SWATH?
        DB      JRNZ
        DB      LD2N1-$-1
;
        XRA     A               ;YES
        CALL    GNHL            ;INCREMENT TO HL
        DB      JR
        DB      LD2N2-$-1
;
LD2N1:  CALL    GNHL            ;INCREMENT
        ORA     A               ;CLEAR CY
        DB      0EDH            ;SBC HL,DE
        DB      52H             ;N2-N1
        INX     H               ;INCLUDE END POINT
LD2N2:  MOV     B,H
        MOV     C,L             ;BC GETS THE INCRM
        RET
;
; LOAD 3 OPERANDS. HL GETS SOURCE, DE THE
; 3RD OPERAND, BC THE INCREMENT & A THE
; LOW BYTE OF THE 3RD OPERAND.
;
LD3N:   CALL    LD2N
; (CONTINUE BELOW)
;
; TRANSFER DE TO HL. ENTER WITH SPACE OR
; 1ST DIGIT OF NUMBER IN A. GET NUMBER
; INTO DE WITH LOW BYTE ALSO TO A.
; FINISHES WITH A CRLF.
;
LINCR:  CALL    GNHL            ;SKIP SPACES, LOAD HL
        CALL    SKSGC           ;WAIT FOR A CR
        MOV     A,L
        XCHG
        RET
;
; CLEARS HL. IF ENTERED WITH HEX CHAR IN A,
; SHIFTS IT INTO HL. O/W, IGNORES LEADING
; SPACES. FIRST CAHR MUST BE HEX. CONTINUES
; SHIFT UNTIL A NON-HEX CHAR RECEIVED & THEN
; RETURNS WITH THE LETTER IN A.
; PRESERVES B,C,D,E.
;
GNHL:   PUSH    B               ;SAVE
GNHL1:  LXI     H,0             ;CLEAR BUFFER
; STRIP LEADING SPACES & GET CHAR
        CALL    SKSG
; FIRST CHAR MUST BE HEX
        CALL    HEXSH           ;IF HEX, SHIFT INTO HL
        JC      ERROR           ;O/W,RETRY
GNHL3:  CALL    GCHR
GNHL5:  CALL    HEXSH           ;IF HEX SHIFT INTO HL
        MOV     A,B             ;RESTORE CHAR
        DB      JRNC
        DB      GNHL3-$-1       ;IF HEX, CONTINUE
        POP     B               ;IF NON-HEX, DONE
        RET
;
; IF A CONTAINS HEX CHAR, SHIFTS BINARY EQUIVALE
; INTO HL. IF NOT HEX, RET WITH CY SET. SAVES
; ORIGINAL CHAR IN B
;
HEXSH:  MOV     B,A
        SUI     '0'             ;< '0'?
        RC
        ADI     '0'-'G'-CASE
        RC
        SUI     'A'-'G'
        DB      JRNC            ;OK IF >= 'A'
        DB      HXSH0-$-1
        ADI     'A'-'9'-1+CASE
        RC
HXSH0:  DW      0AC6H           ;ADI '9'+1-'0'
; THE A-REG NOW CONTAINS THE HEX DIGIT IN BINARY
; (THE HIGH-ORDER NIBBLE OF A IS 0.)
HXSH4:  CALL    HXSH1           ;SHIFT 4 BITS INTO HL
        CALL    HXSH1
        CALL    HXSH1
;
HXSH1:  RLC                     ;SHIFT INTO BIT 4
        DAD     H               ;SHIFT LEFT
; CLEAR CY IN CASE OF RET FROM HEXSH
        ORA     A
        DB      0CBH            ;BIT 4,A
        DB      67H             ;IS IT 0?
        RZ
        INX     H
        RET
;
; RETURNS WITH A NON-SPACE IN THE A-REG.
; IF ENTERED WITH A-REG CONTAINING A NULL
; OR A SPACE, GETS NEW CHARS UNTIL FIRST
; NON-SPACE OCCURS. ALTERS AF.
;
SKSG0:  XRA     A               ;START WITH A NULL
;
SKSG:   ORA     A               ;DOES A CONTAIN NULL?
SKSG1:  CZ      GCHR
        CPI     20H             ;SPACE?
        DB      JRZ
        DB      SKSG1-$-1
        RET
;
; PRINT THE NUMBER IN HL. PRESERVES ALL REGS.
;
PNHL:   PUSH    PSW
        PUSH    H               ;TO STACK
        CALL    P4HEX
        POP     H
        POP     PSW
        RET
;
; PRINT SPACE FOLLOWED BY THE NUMBER POINTED
; TO BY HL. ALTERS A ONLY.
;
PSNM:   CALL    SPACE
; (CONTINUE BELOW)
;
; PRINTS THE NUMBER POINTED TO BY HL.
; PRESERVES ALL REGISTERS.
;
PNM:    PUSH    PSW
        CALL    P2HEX
        POP     PSW
        RET
;
; PRINTS 4 HEX CHARS FROM TOP OF STACK.
; ALTERS F,H,L.
;
P4HEX:  LXI     H,3
        DAD     SP              ;HL = SP
        CALL    P2HEX           ;HIGH BYTE
        DCX     H               ;LOW BYTE
;
; PRINT THE NUMBER POINTED TO BY HL.
; PRESERVES ALL REGS EXCEPT AF.
P2HEX:  MOV     A,M             ;GET THE NUMBER
        RRC
        RRC
        RRC
        RRC
        CALL    P1HEX           ;LEFT NIBBLE
        MOV     A,M             ;NOW DO THE RIGHT NIBBLE
P1HEX:  ANI     0FH             ;MASK
        CPI     10              ;<= 9?
        DB      JRC
        DB      P1HX1-$-1
        ADI     7               ;A THRU F
P1HX1:  ADI     30H             ;ASCII BIAS
        JMP     PCHR            ;PRINT IT
;
; PRINT MESSAGE. ENTER WITH ADDR OF MSG
; IN HL. MSG IS TERMINATED BY 00 THRU 07.
; PRESERVES FLAGS, CLEARS A, INCRM HL.
;
; PRINT '? <BELL>'
;
PSQS:   LXI     H,SQS
;
PMSG:   MVI     A,0             ;CLEAR A (FOR GNHL)
        PUSH    PSW             ;SAVE FLAGS
PMSG1:  MOV     A,M
        INX     H
        CALL    PCHR
        ANI     0F8H            ;<NULL> THRU <BELL>?
        DB      JRNZ
        DB      PMSG1-$-1
        POP     PSW
        RET
;
; DE GETS THE FIRST ALPHA CHAR - 'A'.
; C GETS THE FIRST DELIMITER.
; B IS INITIALIZED TO '0' & RETURNS
; THE LAST CMND CHARACTER.
;
GCMND:  CALL    SKSG0           ;GET NON-SPACE
        CALL    ABCYZ           ;ALPHA CHECK
        MOV     E,A
        MVI     D,0             ;DE HAS TBL DISPLACEMENT
        MVI     B,'O'+CASE      ;INITIALIZE FOR GO CMND
GCMN1:  CALL    GCHR            ;GET CHAR
        CPI     30H             ;DELIMITER?
        MOV     C,A             ;DELIM STORE
        RC                      ;IF SO, DONE
        MOV     B,A             ;LAST CHAR STORE
        DB      JR
        DB      GCMN1-$-1
;
; COMMAND
;
VERIF:  CALL    LD3N            ;GET 3 OPERANDS
;
; COMPARES TWO AREAS OF MEMORY. ENTER WITH
; SOURCE IN HL. DESTINATION IN DE & COUNT
; IN BC. ALTERS ALL REGISTERS.
;
VRFY:   LDAX    D               ;DESTINATION
        DB      CPI0            ;COMPARE TO SOURCE
        DB      CPI1
        CNZ     CRLF            ;IF NOT SAME, CRLF
        DCX     H               ;(CPI INCRMS HL)
        CNZ     PNHL            ; & PRINT SOURCE ADDR
        CNZ     PSNM            ; & SOURCE CONTENTS
        XCHG
        CNZ     PSNM            ; & DEST CONTENTS
        XCHG
        INX     H               ;RESTORE HL FOR CPI
        INX     D               ;NEXT DEST
        JPO     CRLF            ;IF BC = 0, DONE
        DB      JR
        DB      VRFY-$-1
;
; COMMAND
;
MOVE:   CALL    LD3N            ;OPERANDS
        MVI     A,1             ;# OF ITERATION
;
; MOVE FROM ONE LOCATION TO ANOTHER. ENTER
; WITH SOURCE ADDR IN HL, DEST IN DE, BYTE
; COUNT IN BC. THE MOVE IS ITERATED N TIMES,
; WHERE N = TWICE THE CONTENTS OF A, LESS ONE.
; INCREMENTS HL & DE BY BC. CHECKS RESULT
; & PRINTS THE ERRORS FOUND.
MVE:    STC                     ;CY IS USED IN ITERATION COUNT
MVE1:   PUSH    H               ;SOURCE
        PUSH    D               ;DEST
        PUSH    B               ;BYTE COUNT
        DI                      ;FOR PROM PROGRAMMING
        DB      LDIR            ;ONE ITERATION
        DB      LDIR1
        EI
        POP     B
        POP     D
        POP     H
; ITERATION CALCULATIONS
        CMC
        DB      JRC
        DB      MVE1-$-1
        DCR     A
        DB      JRNZ
        DB      MVE1-$-1
; CHECK RESULT
        DB      JR
        DB      VRFY-$-1
;
; COMMAND
;
; GO <CR>     EXECUTION BEGINS AT USER PC.
;
; COMMAND
;
; GO <ADDR1>/<ADDR2> ... >ADDRN>
; EXECUTION BEGINS AT ADDR1 WITH BREAKPOINTS SET
; AT ADDR2,...,ADDRN.
;
GO:     MOV     A,B             ;CHECK THAT THE LAST
        CPI     'O'+CASE        ;CMND CHAR IS 'O'
        JNZ     ERROR
        MOV     A,C             ;CMND DELIMITER
        MVI     C,0             ;BP FLAG
GO1:    CALL    SKSG            ;WAIT FOR NON-SPACE
        CPI     CR
        DB      JRZ
        DB      RETN-$-1        ;RETN IF CR
        CPI     '/'             ;BP?
        DB      JRNZ
        DB      GO3-$-1
        MVI     C,1             ;SET BRKPT FLAG
        LXI     H,RSTLC         ;TRANSFER
        MVI     M,0C3H          ;'JMP SVMS' TO
        LXI     H,SVMS
        SHLD    RSTLC+1         ;RST LOC
        XRA     A
GO3:    CALL    GNHL            ;GET ADDR
        DW      41CBH           ;BIT 0,C: FLAG SET?
        XCHG
        DB      JRZ
        DB      GO5-$-1         ;JMP IF NO BP
        DB      IX
        PUSH    H               ;PUSH IX
        POP     H
        INX     H
        MOV     L,M             ;HL = BPSP
;
        INX     H               ;BUMP BPSP
        XCHG                    ;DE=BPSP, HL= BP ADDR
        MOV     B,M             ;CONTENTS
        MVI     M,0C7H+RSTLC    ;RST INSTRUCTION
        XCHG                    ;HL=BPSP
        MOV     M,B             ;TO BP STACK
        INX     H               ;BUMP BPSP
        MOV     M,E             ;BP ADDR TO STACK
        INX     H
        MOV     M,D
        INX     H
        MVI     M,01            ;PUNCTUATION (BP SET)
        DB      IX
        MOV     M,L             ;LD (IX+1),L
        DB      1
        DB      JR
        DB      GO1-$-1
; CHANGE USER PC
GO5:    DB      IX
        MOV     M,D             ;LD (IX+DUPC),D
        DB      DUPC
        DB      IX
        MOV     M,E             ;LD (IX+DUPC-1),E
        DB      DUPC-1
        DB      JR
        DB      GO1-$-1         ;BACK FOR MORE
;
RETN:   POP     H               ;STRIP CMND ADDR FROM STK
        POP     H               ;UHL2
        POP     D               ;UDE2
        POP     B               ;UBC2
        POP     PSW             ;UAF2
        DB      EXX
        DB      EXAF
        DB      IY
        POP     H               ;POP IY: UIY
        DB      IX
        POP     H               ;POP IX: UIX
;
        POP     PSW             ;UIF
        DB      0EDH
        DB      47H             ;LD I,A: UI
        DI
        DB      JRNC
        DB      RETN1-$-1
        EI
; IFF NOW RESTORED
RETN1:  POP     H               ;UHL
        POP     D               ;UDE
        POP     B               ;UBC
        POP     PSW             ;UAF
        XTHL                    ;USP TO HL, UHL TO (SP)
        PUSH    PSW
        PUSH    B
        PUSH    D
        LXI     B,10
        XCHG                    ;USP TO DE
        DCX     D
        LXI     H,9
        DAD     SP
        DB      LDDR            ;TRANSFER UPC THRU UHL, L
        DB      LDDR1           ;TO USER STACK
        XCHG                    ;IS (USER SP - 1) RAM?
        MOV     A,M
        INR     M
        CMP     M               ;DID IT CHANGE?
        DB      JRZ
        DB      RETN2-$-1
;
        DCR     M               ;YES, RESTORE IT.
        SPHL                    ;CHABGE TO USER STACK
        INX     SP              ;CORRECT FOR LDDR EXTRA DCR
;
RETN2:  POP     D               ;OTHERWISE, CONTINUE SYS
        POP     B
        POP     PSW
        POP     H
        RET
;
; ENTER WITH HL POINTING TO MEMORY & B CONTAINING
; THE 2-BYTE REG FLAG.
; PRINTS SPACE, CONTENTS OF (HL) & ALSO (HL-1) FOR
; 2-BYTE REGS, GETS SUBSTITUTION VALUE INTO DE,
; WRITES E INTO (HL) OR (HL-1) FOR 2-BYTE REGS.
; RETURNS WITH Z-FLAG RESET IFF A CHANGE IS INDICATED
; (BY A LACK OF '.') FOR A 2-BYTE REG.
; PRESERVES BC,HL.
;
GSUBV:  CALL    PSNM            ;PRINT (HL)
        DB      0CBH            ;BIT 6,B
        DB      70H             ;2-BYTE REG?
        DB      JRZ
        DB      GSBV1-$-1
        DCX     H               ;YES, PRINT
        CALL    PNM             ;  LO BYTE
GSBV1:  MVI     A,'.'
        CALL    PCHR
        CALL    GCHR
        CPI     '.'             ;SUSTITUTION?
        CZ      PCHR            ;IF NOT, PRINT ANOTHER
        DB      JRZ
        DB      GSBV2-$-1
        XCHG
        CALL    GNHL            ;NEW VALUE
        XCHG                    ;TO DE
        MOV     M,E             ;LOAD MEM
; THE FOLLOWING TEST IS FOR SBSR
        DB      0CBH            ;BIT 6,B
        DB      70H             ;2-BYTE REG?
GSBV2:  INX     H
        RET
;
; COMMAND
;
; SM <ADDR>    SUBSTITUTE MEMORY LOCATION.
;
; COMMAND
;
; SR <REGISTER NAME>    SUBSTITUTE USER REGISTER
;
; REGISTER NAMES: P (PC), S (SP),
;                 A, F, B, C, D, E, H, L,
;                 I, T (IFF), X (IX), Y (IY),
;                 A',F',B',C',D',E',H',L'.
;
SUBST:  MOV     A,B             ;LAST CMND CHAR
        CPI     'R'+CASE        ;SR?
        MOV     A,C             ;DELIMITER
        DB      JRZ
        DB      SBSR-$-1
;
SBSM:   CALL    GNHL            ;HL GETS ADDR
SBSM1:  MVI     B,0             ;REG FLAGS
; PRINT CURRENT VALUE, REQUEST NEW VALUE &
; PRINT IT IF GIVEN
        CALL    GSUBV
        MVI     A,7             ;8 ENTRIES PER LINE
        CALL    CKBND
        DB      JR
        DB      SBSM1-$-1
;
SBSR:   CALL    GCMND           ;DE GETS LETTER - 'A'
        LXI     H,RGTBL
        DAD     D               ;PNTS TO REG DISPLACEMENT
        MOV     B,D             ;D = 0
        DB      0CBH            ;BIT 7,(HL)
        DB      7EH             ;A THRU L?
        DB      JRZ
        DB      SBSR1-$-1
        MOV     A,C             ;LAST CMND DELIMITER
        CPI     20H             ;SPACE?
        DB      JRZ
        DB      SBSR1-$-1
        CPI     ''''            ;PRIMED?
        JNZ     ERROR
        MVI     B,DUAF-DUAF2    ;YES
;
SBSR1:  MOV     A,M             ;DISPLACEMENT & FLAGS
        ORA     A               ;IF 0, ILLEGAL CMND
        JZ      ERROR
        ANI     1FH             ;STRIP FLAGS OFF
        ADD     B               ;ADJUST FOR PRIMES
        MOV     E,A             ;DE GETS DISPL (D=0)
        MOV     B,M             ;SAVE ORIG ENTRY
        DB      IX
        PUSH    H               ;PUSH IX
        POP     H               ;STACK FRAME
        DB      0EDH            ;SBC HL,DE
        DB      52H             ;PNTS TO USER REG
; PRINT CURRENT VALUE, DE GETS SUBSTITUTION
; VALUE, IF ANY, & (HL) OR (HL-1) GETS E.
; Z-FLAG RESET IFF CHANGE FOR A 2-BYTE REG.
        CALL    GSUBV
        DB      JRZ
        DB      SBSR3-$-1
        MOV     M,D             ;NO. HI BYTE
SBSR3:  CALL    SPACE
        DB      JR
        DB      SBSR-$-1
;
DISPL:  MOV     A,B             ;LAST CMND CHAR
        CPI     'R'+CASE        ;DR?
        MOV     A,C             ;CMND DELIMITER
        JZ      DSPR
;
; COMMAND
;
; DISPLAY MEMORY.
;
DSPM:   CALL    L2NCR           ;INTO DE, INCRM TO BC,
                                ;DELIMITER TO A
        XCHG                    ;N1 TO HL
DSPM1:  CALL    PADR1           ;PRINT ADDR, ':'
DSPM2:  CALL    PSNM            ;PRINT CONTENTS OF MEM
        INX     H
        DCX     B
        MOV     A,B
        ORA     C               ;DONE?
        JZ      CRLF
        CALL    CK16B           ;CHECK FOR 16 COUNT
        DB      JR
        DB DSPM2-$-1
;
; COMMAND
; READ BINARY INPUT FROM DATA PORT
;
READB:  CALL    L2NCR           ;GET MEM ADDRS
RDB1:   CALL    GBYTE           ;GET INPUT
        STAX    D               ;TO MEM
        INX     D
        DCX     B               ;COUNT
        MOV     A,B
        ORA     C               ;BC = 0?
        DB      JRNZ
        DB      RDB1-$-1
        RET
;
; COMMAND
; WRITE BINARY OUTPUT TO DATA PORT
;
WRITB:  CALL    L2NCR           ;GET MEM ADDRS
WRTB1:  IN      STAT
        ANI     TBE
        DB      JRZ
        DB      WRTB1-$-1
        LDAX    D
        OUT     DATA
        INX     D
        DCX     B
        MOV     A,B
        ORA     C
        DB      JRNZ
        DB      WRTB1-$-1
        RET
;
; COMMAND
; OUT <DATA-BYTE> <PORT NUMBER>
;
OUTP:   CALL    GNHL
        XCHG                    ;E GETS DATA
        CALL    GNHL            ;GET PORT NUMBER
;
        MOV     C,L             ;TO C
        DW      59EDH           ;OUT (C),E
        RET
;
HEAD:   DB      CR
        DB      CR
        DB      'CROMEMCO MON1.0 C.1976'
        DB      0
;
SQS:    DB      ' ?'
        DB      BELL
;
LFNN:   DB      LF
        DB      7FH             ;NULL
        DB      0
;
PRMPT:  DB      ':'
        DB      0
; THE COMMAND TBL MUST IMMEDIATELY FOLLOW
; THE PROMT MESSAGE
        DW      ERROR           ;A
        DW      ERROR           ;BANK
        DW      ERROR           ;C
        DW      DISPL           ;DISPLAY
        DW      ERROR           ;ENTER
        DW      ERROR           ;FILE
        DW      GO
        DW      ERROR           ;H
        DW      ERROR           ;INPUT
        DW      ERROR           ;J
        DW      ERROR           ;K
        DW      ERROR           ;LIST
        DW      MOVE
        DW      ERROR           ;NUMBER
        DW      OUTP            ;OUTPUT
        DW      PROG            ;PROGRAM
        DW      ERROR           ;Q
        DW      READB           ;READ BINARY OR ASCII
        DW      SUBST           ;SUBSTITUTE
        DW      ERROR           ;TRAP
        DW      ERROR           ;UNEQUAL
        DW      VERIF           ;VERIFY
        DW      WRITB           ;WRITE BINARY OR ASCII
        DW      ERROR           ;X
        DW      ERROR           ;Y
;
RGTBL:  DB      -DUAF+PF        ;A
        DB      -DUBC+PF        ;B
        DB      -DUBC+1+PF      ;C
        DB      -DUDE+PF        ;D
        DB      -DUDE+1+PF      ;E
        DB      -DUAF+1+PF      ;F
        DB      0
        DB      -DUHL+PF        ;H
        DB      -DUIT           ;I
        DB      0
        DB      0
        DB      -DUHL+1+PF      ;L
        DB      0
        DB      0
        DB      0
; INTEL MACRO-80 FLAGS A VALUE ERROR, BUT CORRECT VALUE 40 IS COMPUTED
;        DB      -DUPC+R2F       ;PC
	DB	40H
        DB      0
        DB      0
        DB      -DUSP+R2F       ;SP
        DB      -DUIT+1         ;T (INTERRUPT ENABLE)
        DB      0
        DB      0
        DB      0
        DB      -DUIX+R2F       ;X (IX)
        DB      -DUIY+R2F       ;Y (IY)

        END

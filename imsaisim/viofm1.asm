;*************************************************************
; VIO TERMINAL FIRMWARE  REQUIRES REFRESH MEMORY TO BE AT
; F000 AND FIRMWARE ITSELF AT F800
; COPYRIGHT IMSAI MANUFACTURING COMPANY
;           SAN LEANDRO, CALIFORNIA
;           6/1/77
;
; MODIFIED TO ASSEMBLE WITH INTEL 8080 CROSS ASSEMBLER
; AND PRETTIFIED SOURCE
;           UDO MUNK
;           1/3/2017
;
; CHANGED DEFINITIONS OF SYSRAM VARIABLES FROM DB/DW TO DS
; SO THAT NO CODE IS EMITTED
;           UDO MUNK
;           02/17/2017
;**************************************************************
REFRESH EQU  0F000H     ;REFRESH MEMORY ON VIO
SYSRAM  EQU  0F780H     ;SYSTEM RAM
VIOFM   EQU  0F800H     ;FIRMWARE BEGINNING
        ORG  VIOFM+7FDH
        DB   'VI0'      ;SYSTEM IDENTIFIER
CTRPORT EQU  REFRESH+7FFH ;HARDWARE CONTROL WORD
        ORG  SYSRAM
CURLIN: DS   1          ;CURRENT LINE # 0-23
CURCOL: DS   1          ;CURRENT COL # 0-79
INVIDIO:DS   1          ;INVERSE VIDIO MODE(BY CHAR)
VDIMDE: DS   1          ;MODE 0=GRAPHICS, NOT 0= TEXT
INSRT:  DS   1          ;INSERTING CHARS MODE
ESCCNT: DS   1          ;ESCAPE CHAR COUNT
ESCCDE: DS   1          ;ESCAPE CODE LAST USED
USRCTR: DS   2          ;USER CTR TBLE PTR, NON ZERO
USRESC: DS   2          ;USER ESCAPE TBLE PTR,NON ZERO
USERCMD:DS   2          ;USER MONITOR COMMAND TABLE
RAMPTR: DS   2          ;RAM SPACE PTR WITH DIRECT I/O
CURPTR: DS   2          ;CURSOR ADDRESS
PRTMD:  DS   1          ;PROTECTED MODE 0=NO
CCUR:   DS   1          ;CHAR UNDER CURSOR(FOR GRAPHICS MODE)
CCHAR:  DS   1          ;CURRENT CHARACTER TO DISPLAY
CTRLC:  DS   1          ;CONTROL WORD AS FOLLOWS
;                               7 SCROLL MODE 0=SCROLL,1=WRAP
;                               6 UNUSED
;                               5 UP/LOW        0=UP,1=UP+LOW
;                               4 1=INVERSE VIDIO SCREEN
;                               3 00=BLANK,01=LOW128+INV,10=HIGH128+INV
;                               2 11=256 CHAR GRAPHICS
;                               1 #LINES 0=24,1=12
;                               0 #CHARS 0=80,1=40
TAB     EQU  $
        ORG  $+10       ;80 BITS FOR TAB CONTROL
CLINE:  DS   2          ;CHARS/LINE
LPAGE:  DS   1          ;LINES/PAGE-1
NCHARS: DS   2          ;#CHARS ON DISPLAY
PRUPRF: DS   1          ;TRANSITION PROTECT FLAG
USERF:  DS   1          ;ENTRY POINT FLAG 0=INIT48,1=CHAR48,2=USER
LASTC:  DS   2          ;LAST CHAR ON SCREEN PTR+1
;***********************************
; USER ENTRY POINTS
;***********************************
        ORG  VIOFM
        JMP  INIT               ;INITIALIZATION POINT
        JMP  CHAROUT            ;CHARACTER OUTPUT
        JMP  MONT               ;MONITOR ENTRY POINT
VIOTEST:LXI  SP,SYSRAM+7EH
        CALL INIT
        CALL CHIN
        JMP  VIOTEST+6          ;DEMO TESTER
;**********************************************
INIT:   PUSH H
        PUSH D
        PUSH B
        PUSH PSW
        LXI  H,CURLIN           ;START OF ZEROED AREA
        MVI  B,CLINE-CURLIN AND 0FFH
        XRA  A
INIT1:  MOV  M,A                ;ZERO AREA
        INX  H
        DCR  B
        JNZ  INIT1
        LXI  H,REFRESH  ;BEGIN CURSOR POS
        SHLD CURPTR
        LXI  H,1920     ;DEFAULT CHARS/SCREEN
        SHLD NCHARS
        CALL BLNKS      ;CLEAR SCREEN AND HOME
        MVI  A,8H       ;DEFAULT 80X24 SCREEN TEXT MODE
        LXI  D,BMP1     ;SET UP RETURN ADDR
        PUSH D
;SUBROUTINE TO SET HARDWARE CONTROL PORT
SETCMD: STA  CTRLC
        STA  CTRPORT    ;HARDWARE CONTROL PORT
        CMA
        ANI  3
        RRC             ;FINDING NCHARS ON SCREEN
        LXI  H,40       ;COLS/LINE
        JNC  $+4        ;ENOUGH
        DAD  H          ;COLS/LINE=80
        SHLD CLINE
        LXI  H,LPAGE    ;PT AT LINES/PAGE
        MVI  M,11
        RAR
        JNC  $+5
        MVI  M,23
        LXI  H,480      ;COUNT FOR 12 X 40 SCREEN
        JNC  $+4
        DAD  H
        ORA  A          ;SET FLAGS
        JZ   $+4
        DAD  H
        SHLD NCHARS
        LXI  D,REFRESH
        DAD  D
        SHLD LASTC      ;LAST CHAR ON SCREEN PTR+1
        LDA  CTRLC      ;CONTROL CODE
        ANI  0CH        ;MODE BITS ONLY
        XRI  0CH
        STA  VDIMDE     ;0=GRAPHICS
;CHECK CURSOR WITHIN POSSIBLE NEW BOUNDS
ESCRET: XRA  A
        STA  ESCCNT     ;COUNT=0
        RET
;****************************************************************
; THIS IS THE NORMAL ENTRY POINT FOR COMMUNICATING WITH THE
; VIDIO MONITOR AS YOU WOULD A CRT.
;****************************************************************
CHAROUT:PUSH H
        PUSH D
        PUSH B
        PUSH PSW
        STA  CCHAR
        LHLD CURPTR     ;CURSOR POSITION
        LDA  CCUR       ;CHAR UNDER CURSOR
        MOV  M,A        ;REMOVE CURSOR
        LXI  H,CCHAR    ;PT AT CURRENT CHAR
        MOV  A,M        ;GET CHAR
        CPI  1BH        ;ESCAPE CHAR?
        JZ   ESCAPE
        LDA  ESCCNT     ;ARE WE IN ESCAPE SEQ ALREADY?
        ORA  A
        JNZ  ESCAPE     ;YES
        MOV  A,M        ;CURRENT CHAR
        CPI  7FH        ;DELETE CHAR (RUBOUT)?
        JZ   NOUSER+3   ;YES
        LDA  VDIMDE     ;GRAPHICS MODE?
        ORA  A
        JZ   CHAR1      ;YES
        MOV  A,M
        CPI  0FFH       ;DUMMY PAD FROM USER?
        JZ   BMP1       ;YES
        ANI  7FH        ;STRIP PARITY BIT
        MOV  M,A
        SBI  20H        ;CONTROL CODE?
        JM   CONTROL    ;YES
        LDA  CTRLC      ;CONTROL WORD
        MOV  B,A        ;TMP SAVE
        ANI  0CH        ;MODE ONLY
        CPI  08H        ;LOW HALF OF CHAR GEN ROM?
        JNZ  CHAR1      ;NO,UPPER HALF
        MOV  A,B        ;CONTROL WORD
        ANI  20H        ;UP/LOW CASE
        JNZ  CHAR1      ;LOWER OK AS IS
        MOV  A,M        ;CURRENT CHAR
        SBI  61H        ;LOWER CASE A
        JM   CHAR1      ;NOT ALPHA
        SBI  7BH-61H
        JP   CHAR1      ;NOT ALPHA
        ADI  7BH-20H    ;RESTORE AND CONVERT TO UPPER CASE
        MOV  M,A
CHAR1:  CALL INSCHR     ;INSERT CCHAR AT CURSOR POS
        CALL BMPCUR
BMP10:  CALL CALPOS     ;CURSOR POS
BMP1:   CALL INSCURS    ;INSERT CURSOR
        POP  PSW
        POP  B
        POP  D
        POP  H
        RET
BMPCUR  EQU  $
        CALL BMPC       ;BUMP CURSOR CHAR POSITION
        CZ   BMPC1      ;DO LINE FEED
        LDA  PRTMD      ;PROTECT MODE?
        XCHG            ;H,L=CURRENT CURSOR PTR
        ANA  M          ;IS IT PROTECTED?
        JM   BMPCUR     ;YES,SKIP PROTECTED FIELD
        RET             ;GO INSERT CURSOR
LFEED:  LXI  H,CURLIN
BMPC1:  INR  M
        LDA  LPAGE      ;MAX LINES/PAGE
        CMP  M          ;EXCEED MAX?
        RP
        DCR  M          ;LEAVE AT LAST LINE
;********************************************
;SCROLL UP OR WRAP AROUND AS SET BY CTRLC
;********************************************
SCROLL: LDA  CTRLC      ;KIND OF SCROLL?
        ANI  8CH        ;LEAVE SCROLL AND MODE BITS
        JM   SCR3       ;WRAP AROUND
        CPI  0CH        ;GRAPHICS MODE
        JNZ  SCR1       ;NO, ALLOW SCROLL
SCR3:   XRA  A
        MOV  M,A        ;HOME CURSOR FOR WRAP AROUND
        INX  H
        MOV  M,A
        RET
SCR1:   LHLD CLINE      ;COLS/LINE
        PUSH H          ;SAVE COLS/LINE
        XCHG
        LHLD NCHARS     ;# CAHRS PER PAGE
        MOV  A,L
        SUB  E
        MOV  C,A
        MOV  B,H
        XCHG
        LXI  D,REFRESH
        DAD  D          ;HL=SOURCE,DE=DEST.
        CALL MVCUP
SCR2:   JMP  DLN1       ;ERASE CURRENT LINE AND RETURN
;******************************
;PROCESS CONTROL CODES
;******************************
CONTROL:XCHG            ;D,E=CCHR PTR
        LHLD USRCTR     ;USER TABLE IF ANY
        MOV  A,H
        ORA  A
        LDAX D          ;CCHAR IN A
        JZ   NOUSER     ;NO TABLE USER DEFINED
        CALL LOOKUP
        JNZ  FNDCTRL    ;FOUND TABLE ENTRY
NOUSER: LDA  CCHAR
        LXI  H,CTRTBL
        CALL LOOKUP
        JZ   BMP1       ;NOT FOUND
FNDCTRL:LXI  D,BMP10    ;RETURN ADDRESS
        PUSH D          ;ON STACK
        LXI  D,CURCOL
        PCHL
;****************************************
;PROCESS ESCAPE SEQUENCES
;****************************************
ESCAPE: LXI  D,BMP10    ;RETURN ADDR
        PUSH D
        XCHG
        LXI  H,ESCCNT
        MOV  A,M        ;ESCCAPE COUNT
        INR  M          ;ESCCNT=ESCCNT+1
        ORA  A
        RZ
        DCR  A
        INX  H
        LDAX D          ;GET CCHAR
        JNZ  ESC1       ;ESCCNT>1
        MOV  M,A        ;SAVE ESCAPE CODE
ESC1:   LHLD USRESC     ;USER ESCAPE TABLE PTR
        MOV  A,H
        ORA  A
        LDAX D          ;ESCCODE
        JZ   NUESC      ;NO USER DEFINED TABLE
        CALL LOOKUP     ;LOOKUP IN USERS TABLE
        JNZ  FNDESC     ;FOUND ESCAPE SEQ IN USER
NUESC:  LDA  ESCCDE     ;TRY AGAIN IN VIO TABLE
        ANI  0DFH       ;REMOVE LOWER CASE BIT
        LXI  H,ESCTBL
        CALL LOOKUP
        JZ   ESCRET     ;NOT FOUND
FNDESC: LDA  CTRLC
        LXI  D,SETCMD
        PCHL
;************************
;CURSOR CONTROL
;************************
UPLINE: DCX  D          ;D,E=CURLIN PTR
BCKLNE: LDAX D          ;D,E=CURLIN OR CURCOL
        ORA  A          ;SET FLAGS
        RZ
        DCR  A          ;BACK UP 1
BCKL1:  STAX D
        RET
CRET:   XRA  A
        STA  INSRT      ;REMOVE INSERT MODE
        JMP  BCKL1
;**********************************
;TOGGLE PROTECTED MODE FLAG
;**********************************
PRTECT: LXI  H,PRTMD    ;PT AT FLAG
        JMP  INSMDE+3   ;GO TOGGLE IT
;*************************************
;TOGGLE INSERT MODE FLAG
;*************************************
INSMDE: LXI  H,INSRT
        MOV  A,M
        CMA
        MOV  M,A
        RET
;***********************
;BLANK SCREEN AND HOME
;***********************
BLNKS:  LHLD NCHARS     ;#CHARS ON SCREEN
        XCHG
        LXI  H,REFRESH
BLNK1:  LDA  PRTMD      ;IN PROTECTED MODE?
        ANI  80H
        ANA  M          ;PROTECTED?
        JM   BLNK2      ;IS PROTECTED, DO NOT BLANK
        MVI  M,' '
BLNK2:  INX  H
        DCX  D
        MOV  A,D
        ORA  E          ;DONE YET?
        JNZ  BLNK1      ;NO
HOME:   LXI  H,0
        SHLD CURLIN
        RET
;***********************************************
;BLANK FROM CURSOR TO END OF UNPROTECTED FIELD
;***********************************************
BLANKL: CALL CHARLN     ;CALC # CHARS TO END OF FIELD
BLAN3:  LDA  PRTMD      ;PROTECTED MODE?
        ORA  A
        JZ   BLAN1      ;NOT PROTECTED,SKIP CHECK
        MOV  A,M        ;GET CHAR
        ORA  A
        JM   BLAN1+3    ;IS PROTECTED,DO NOT BLANK
BLAN1:  MVI  A,' '
        MOV  M,A        ;INSERT BLANK
        INX  H          ;NEXT CHAR
        DCR  C          ;COUNT
        JNZ  BLAN3
        RET
;**********************************************
;TURN ON PROTECTED FIELD/TURN OFF PROT FIELD
;**********************************************
PROTC:  LXI  H,INVIDIO  ;PT AT INVERTED VIDIO FLAG
        JMP  INSMDE+3
;****************************************************
;DELETE CHAR AND SHIFT PROTECTED FIELD LEFT ONE PLACE
;****************************************************
DELETE: CALL CHARLN
        LHLD CURPTR     ;CURSOR POSITION
        MOV  D,H
        MOV  E,L
        INX  H
        CALL MVCUP      ;SHIFT LINE LEFT ONE PLACE
        MVI  A,' '
        DCX  D          ;BACK UP ONE
        STAX D          ;INSERT FINAL BLANK
        RET
;**********************************************************
;CALC # CHARS FROM CURSOR TO END OF UNPROT FIELD INCLUSIVE
; RETURN H,L=CURSOR PTR
;**********************************************************
CHARLN: LDA  PRTMD      ;PROTECT MODE FLAG
        ANI  80H
        MOV  D,A        ;SAVE PROTECT MODE BIT
        LHLD CURPTR     ;CURSOR POSITION
        PUSH H
        LDA  CURCOL
        MOV  E,A        ;E=CURRENT COLUMN
        LXI  B,0        ;# CHARS TO END
CHRL1:  LDA  CLINE      ;COLS/LINE
        SUB  E
        INR  E
        INX  H
        INR  C          ;COUNT INCREASED
        DCR  A          ;DNE YET WITH LINE
        JZ   CHRL2      ;END OF LINE RETURN
        MOV  A,M        ;H,L=END +1
        ANA  D          ;PROTECTED?
        JP   CHRL1      ;NO, KEEP GOING
CHRL2:  POP  H          ;CURSOR POSITION
        RET
;*******************************************************************
;TABLE LOOK UP ROUTINE. SEARCHES FIRST BYTE OF THREE BYTE TABLE OF
;RECORDS FOR A MATCH OR ZERO. ZERO INDICATES END OF TABLE WITH NO
;MATCH, RETURNED IN A REG.H,L LOADED WITH SECOND TWO BYTES OF TABLE
;IF MATCH FOUND.
;*******************************************************************
LOOKUP: MOV  B,A        ;SAVE
        MOV  A,M        ;GET FIRST BYTE OF RECORD
        LXI  D,CURLIN
        ORA  A
        RZ              ;DONE,NO MATCH
        CMP  B          ;SAME AS REQUESTED?
        JNZ  TBLUP1     ;NO
        INX  H
        MOV  E,M
        INX  H
        MOV  D,M
        XCHG
        ORA  A          ;SET FLAGS
        RET
TBLUP1: INX  H
        INX  H
        INX  H          ;BUMP TO NEXT RECORD
        JMP  LOOKUP+1   ;
;*************************************************
;DELETE CURRENT LINE AND RETURN CURSOR
;*************************************************
DLINE:  CALL NMCHM      ;SET UP FOR MOVE
        PUSH H          ;SAVE COLS/LINE
        DAD  D          ;H,L=SOURCE BEGIN
        CALL MVCUP
DLN1:   POP  B          ;COLS/LINE
        XCHG
        JMP  EN1        ;ERASE LINE
;***********************************************
;ENTER NEW LINE AT CURSOR LINE,PUSH BOTTOM DOWN
;***********************************************
ENLINE: CALL NMCHM      ;SET UP FOR MOVE
        PUSH H          ;SAVE COLS/LINE
        DAD  D          ;H,L=SOURCE BEGIN
        DAD  B          ;H,L=END OF DEST+1
        XCHG
        DAD  B          ;H,L=END OF SOURCE+1
        DCX  H
        DCX  D
        CALL MVCDN      ;MOVE DOWN 1 LINE
        POP  B
        INX  H
EN1:    MVI  M,' '
        INX  H
        DCR  C
        JNZ  EN1
        RET
NMCHM:  XRA  A
        STAX D          ;COL=0
        CALL CHARSN     ;#CHARS TO END OF SCREEN
        XCHG            ;D,E=DEST.
        LHLD CLINE      ;COLS/LINE
        MOV  A,L        ;COLS/LINE
        DCX  B
        DCR  A
        JNZ  $-2        ;DECREASE COUNT BY ONE LINES WORTH
        RET
;********************************************
;CALC # CHARS TO END OF SCREEN FROM CURSOR
;********************************************
CHARSN: CALL CALPOS
        PUSH H          ;SAVE
        XCHG            ;D,E=CURSOR POS
        LHLD LASTC      ;LAST CHAR POSITION+1
        MOV  A,D
        CMA
        MOV  D,A
        MOV  A,E
        CMA
        MOV  E,A
        INX  D          ;COMPLIMENT D,E
        DAD  D          ;H,L=# CHARS TO END-1
        PUSH H
        POP  B          ;B,C=#CHARS TO END
        POP  H          ;CURRENT POSITION CURSOR
        RET
;************************
;ESCAPE CODE PROCESSING
;************************
HIGH128:ANI  0F3H
        ORI  4H
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;GRAPHIC MODE 256 CHAR ROM,NO INVERSE VIDIO
GRAPHIC:ORI  0CH
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;LOWER HALF OF ROM+REVERSE VIDIO
LOW128: ANI  0F3H
        ORI  8H
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;SCROLL TOGGLE
SCRL:   XRI  80H
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;UPPER LOWER CASE TOGGLE
UPLOW:  XRI  20H
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;INVERSE VIDIO TOGGLE
VIDIO:  XRI  10H
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;# LINES PER PAGE SWITCH
LINES:  XRI  02H
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;#COLS/LINE TIGGLE
COLS:   XRI  01H
        XCHG            ;H,L=SETCMD ADDR
        PCHL
;**************************************
;INSERT CURSOR CHAR AT PROPER POSITION
;**************************************
INSCURS:LHLD CURPTR
        MOV  A,M
        STA  CCUR       ;SAVE CHAR UNDER CURSOR FOR GRAPHICS MODE
        ORI  80H        ;BIT 7 FOR INVERSE VIDIO
        MOV  M,A        ;STORE BACK
        LDA  VDIMDE
        ORA  A
        RNZ             ;NO GRAPHICS
        MVI  M,7FH      ;BLOCK FOR GRAPHICS MODE
        RET
;*****************************************
;CLEAR TABS
;*****************************************
CLRTBS: LXI  H,TAB      ;TABS BITS
        MVI  B,10       ;#BYTES FOR TABS
        XRA  A
CLRT1:  MOV  M,A
        INX  H
        DCR  B
        JNZ  CLRT1
        JMP  ESCRET     ;PUT IN CURSOR
;***********************************
;SET OR CLEAR TAB TOGGLE BIT
;***********************************
SETTAB: CALL FNDTB
        XRI  80H        ;INVERT TAB BIT
SETD2:  RRC
        DCR  B
        JNZ  SETD2
        STAX D          ;STORE TAB BYTE
        JMP  ESCRET     ;DO CURSOR
;FIND TAB BIT, LEAVE IN A REG BIT 7
FNDTB:  LDA  CURCOL     ;COL #
        MOV  H,A
        INR  H
        LXI  D,TAB      ;WORD PTR
FNDT1:  MVI  C,8        ;BIT COUNTER
        DCR  H
        JZ   FNDTDN     ;FOUND IT
        DCR  C          ;BIT COUNTER
        JNZ  FNDT1+2
        INX  D          ;PT AT NEXT BYTE
        JMP  FNDT1
FNDTDN: LDAX D          ;GET TAB BYTE BITS
        MOV  B,C        ;SAVE COUNT OF BITS
FNDT2:  RLC
        DCR  C
        JNZ  FNDT2      ;ROTATE UNTIL FOUND
        RET
;*****************************************************************
;TAB TO BEGINNING OF NEXT UNPROTECTED FIELD OR TAB OR HOME IF NONE
;*****************************************************************
TABB:   XRA  A
        STA  PRUPRF     ;PROTECT/UNPROTECT TRANSITION FLAG
TAB3:   CALL BMPC       ;BUMP CURSOR POSITION
        JNZ  TAB1       ;NO LINE FEED NECESSARY
        INR  M          ;BUMP LINE #
        CMP  M          ;EXCEED LPAGE?
        JM   SCR3       ;YES,HOME AND RETURN
TAB1:   LDA  PRTMD      ;PROTECT MODE FLAG
        XCHG            ;H,L PTS AT CHAR
        ANA  M          ;PROTECTED?
        MOV  A,M        ;GET CHAR
        LXI  D,PRUPRF   ;TRANSITION FLAG
        JP   TAB2       ;NO PROTECTED FIELD
        STAX D          ;SET TRANSITION FLAG
        JMP  TAB3
TAB2:   LDAX D          ;GET TRANSITION FLAG
        ORA  A
        RM              ;UNPROT FIELD WITH TRANSITION
        CALL FNDTB      ;FIND TAB POSITION BIT
        ORA  A          ;SET FLAGS
        RM              ;TAB IS SET
        JMP  TAB3
;**************************************************
;CALCULATE CURSOR POSITION FROM CURLIN AND CURCOL
;**************************************************
CALPOS: LHLD CLINE      ;CHARS/LINE-1
        XCHG
        LHLD CURLIN     ;L=CURLIN,H=CURCOL
        MOV  C,H
        MOV  B,L
        LXI  H,REFRESH  ;BOTTOM OF REFRESH MEMORY
        INR  B
CALP1:  DCR  B          ;DONE YET
        JZ   CALP2      ;YES
        DAD  D          ;ADD ANOTHER LINE OF CHARS
        JMP  CALP1
CALP2:  DAD  B          ;ADD CURRENT COL
        SHLD CURPTR     ;SAVE
        RET
;**********************************************************
;BMPC BUMP CURSOR 1 PLACE. ON RETURN
; D,E=CURSOR POSITION
; H,L=CURCOL PTR OR CURLIN PTR DEPENDING ON Z FLAG
; Z FLAG=0 IF NO LINE FEED NEEDED,1 IF LINE FEED NEEDED
; CURLIN AND CURCOL AND CURPTR ARE UPDATED AS IF LINE FEED
; A REG =LPAGE IF LINE FEED NEEDED
;**********************************************************
BMPC:   LHLD CURPTR
        INX  H
        SHLD CURPTR     ;UPDATE ABS CURSOR ADDRESS
        XCHG            ;D,E=PTR
        LXI  H,CURCOL
        INR  M          ;BUMP COLUMN
        LDA  CLINE      ;MAX COLS/LINE
        SUB  M          ;ZERO IF EXCEED LINE
        RNZ             ;OK AS IS
        MOV  M,A        ;COL=0
        DCX  H
        LDA  LPAGE      ;MAX LINES/PAGE
        RET
;************************************************
;ADDRESSABLE CURSOR FUNCTION
;************************************************
ADDCURS:LXI  H,ESCCNT   ;PT AT ESCAPE COUNT
        LXI  D,CURLIN   ;PT AT CURRENT LINE COUNT
        LDA  CCHAR
        SUI  20H        ;REMOVE OFFSET FOR COUNT
        MOV  B,A
        MOV  A,M        ;GET COUNT
        SUI  3
        RM              ;NO VALID NUMBS YET
        JNZ  XADD       ;X AXIS VALUE
;Y-AXIS VALUE
        LDA  LPAGE
XADD3:  STAX D          ;MAX LINE #
        CMP  B
        RM
        MOV  A,B
        STAX D
        RET
XADD:   MVI  M,0        ;ESCCNT=0
        LDA  CLINE      ;MAX COL/LINE
        INX  D
        DCR  A
        JMP  XADD3
;************************************************************
;INSERT CHAR AT CURSOR POSITION.EITHER WRITES OVER PREVIOS
;CHAR OR PUSHES ENTIRE FIELD OVER ONE CHAR BEFORE INSERTING.
;************************************************************
INSCHR: LHLD CURPTR     ;CURSOR ADDRESS
        PUSH H          ;SAVE
        LDA  INSRT      ;INSERT FLAG
        ORA  A
        JZ   INSC3      ;OVERWRITE
        CALL CHARLN
        DCX  B          ;3CHARS-1 TO END
        DAD  B          ;H,L PTS AT LAST CHAR ON LINE
        MOV  D,H
        MOV  E,L
        DCX  D          ;D,E PTS AT SOURCE
        XCHG            ;H,L=SOURCE,D,E=DEST
        CALL MVCDN      ;MOVE CHARS RIGHT
INSC3:  POP  H          ;CURSOR POSITION
        LDA  INVIDIO
        ANI  80H
        MOV  B,A        ;INVERT BIT
        LDA  CCHAR
        ORA  B          ;MERGE WITH INVERT BIT
INSC4:  MOV  M,A
        RET
;********************************************************
;SHIFT CHARS RIGHT FROM D,E TO H,L,  B,C CHARS FROM RIGHT
;********************************************************
MVCDN:  MOV  A,C
        ORA  B
        RZ              ;DONE
        MOV  A,M
        STAX D
        DCX  H
        DCX  D
        DCX  B
        JMP  MVCDN
;CONTROL FUNCTION JUMP TABLE
CTRTBL  EQU  $
        DB   0DH        ;CARRIAGE RETURN
        DW   CRET
        DB   0AH        ;LINE FEED
        DW   LFEED
        DB   0BH        ;UP CURSOR (CTRL K)
        DW   UPLINE
        DB   0CH        ;FORWARD CURSOR (CTRL L)
        DW   BMPCUR
        DB   08H        ;BACK CURSOR (CTRL H)
        DW   BCKLNE
        DB   1EH        ;HOME CURSOR (CTRL ^)
        DW   HOME
        DB   1AH        ;SCREEN ERASE (CTRL Z)
        DW   BLNKS
        DB   15H        ;CLEAR TO EOL (CTRL U)
        DW   BLANKL
        DB   16H        ;PROTECTED FIELDS (CTRL V)
        DW   PROTC
        DB   09H        ;TAB (CTRL I)
        DW   TABB
        DB   7FH        ;DELETE CHAR (RUBOUT)
        DW   DELETE
        DB   14H        ;INSERT MODE (CTRL T)
        DW   INSMDE
        DB   04H        ;DELETE LINE CTRL D
        DW   DLINE
        DB   05H        ;INSERT LINE (CTRL E)
        DW   ENLINE
        DB   10H        ;PROTECTED MODE TOGGLE (CTRL P)
        DW   PRTECT
        DB   0          ;TERMINATOR
;ESCAPE FUNCTION JUMP TABLE
ESCTBL  EQU  $
        DB   1DH        ;CURSOR CONTROL ('=' LESS BIT 5 LOWER CASE)
        DW   ADDCURS
        DB   49H        ;SET TAB
        DW   SETTAB
        DB   09H        ;CLEAR TABS
        DW   CLRTBS
        DB   'T'        ;LOWER 128 BYTES OF ROM
        DW   LOW128
        DB   'E'        ;EXTENDED MODE UPPER 128
        DW   HIGH128
        DB   'G'        ;GRAPHIC SET
        DW   GRAPHIC
        DB   'S'        ;SCROLL TOGGLE
        DW   SCRL
        DB   'U'        ;UPPER/LOWER CASE
        DW   UPLOW
        DB   'V'        ;INVERSE VIDIO TOGGLE
        DW   VIDIO
        DB   'L'        ;LINES/PAGE
        DW   LINES
        DB   'C'        ;COLS/LINE
        DW   COLS
        DB   0          ;TERMINATOR
;*******************************************************
;8085 MONITOR PROGRAM USING THE VIO FIRMWARE
; COPYRIGHT IMSAI MANUFACTURING COMPANY, INC.
; SAN LEANDRO,CALIFORNIA
; 6/7/77
;*******************************************************
MONT:   LXI  SP,REFRESH+7FFH ;TOP OF MEMORY
        MVI  A,0AAH
        OUT  3
        CMA
        OUT  3
        CMA
        OUT  3
        MVI  A,27H
        OUT  3          ;SET UP USART
        CALL INIT       ;INIT VIO
        LXI  H,SIGNON
        CALL MSGNC      ;SIGNON MSG
PRMPT:  LXI  SP,REFRESH+7FFH
        CALL CRLF
        MVI  A,'?'
        CALL CHAROUT
        CALL CHIN       ;GET COMMAND
        MOV  B,A        ;SAVE IT
        LXI  D,PRMPT
        PUSH D          ;RETURN ADDRESS
        LHLD USERCMD    ;USER COMMAND TABLE
        MOV  A,H
        ORA  A          ;SET FLAGS
        MOV  A,B        ;RETRIEVE CODE
        JZ   NUCMD      ;NO USER COMMAND TABLE
        CALL LOOKUP     ;LOOKUP IN USER TABLE
        JNZ  FNDCMD     ;FOUND COMMAND
NUCMD:  MOV  A,B        ;GET COMMAND AGAIN
        LXI  H,CMDTBL   ;COMAND TABLE PTR
        CALL LOOKUP
        RZ              ;NO  ENTRY,PROMPT AGIN
        MVI  B,1        ;FOR PROT/UNPROT
FNDCMD: PCHL            ;GO TO ROUTINE
SIGNON: DB   'IMSAI SMP/80.0',0
        DB   'COPYRIGHT 6/77'
;*************************************************
;JUMP TO MEMORY "JAAAA"
;CALL MEMORY WITH RETURN TO MONITOR
;*************************************************
JUMP:   POP  D          ;REMOVE RETURN ADDRESS
CALL1:  CALL IHEX       ;GET JUMP ADDRESS
        PCHL            ;DO IT
;******************************************************
;ENTER BYTE INTO MEMORY AND MODIFY IF DESIRED
;******************************************************
ENTR:   CALL IHEX       ;START ADDR
        CALL CRLF
        CALL OHEXHL     ;DISPLAY ADDRESS
        MOV  A,M        ;GET BYTE IN MEMORY
        MOV  E,A        ;PRESET FOR IHEX
        CALL OHEXB      ;DISPLAY BYTE
        XCHG            ;D,E=ADDRESS,L=DEFAULT CHAR
        CALL IHEX+3     ;GET MODIFIER OR DEFAULT
        XCHG            ;H,L=ADDR,E=BYTE
        MOV  M,E
        DCX  H
        CPI  0AH        ;DONE?
        RZ              ;YES
        CPI  '-'        ;BACKWARD
        JZ   ENTR+3     ;YES
        INX  H
        INX  H          ;DEFAULT FORWARD
        JMP  ENTR+3
;*****************************************************
;DISPLAY MEMORY "D,START,END CR"
;*****************************************************
DISP:   CALL SIZE       ;H,L=START,B,C=SIZE
        LDA  CTRLC      ;#LINES/COLS
        RRC             ;#LINES BIT IN CARRY
        RRC
        MVI  D,12
        JC   $+5
        MVI  D,24
DISP2:  CALL CRLF
        IN   3
        ANI  2          ;ANY INPUT
        RNZ             ;YES,INTERRRUPT
        MVI  E,8
        LDA  CTRLC
        RRC
        JC   $+5
        MVI  E,16
        CALL OHEXHL     ;OUTPUT ASCII H,L REG
DISP1:  MOV  A,M        ;GET DATA BYTE
        CALL OHEXB      ;OUTPUT WITH TRAIL BLANK
        INX  H
        DCX  B
        MOV  A,B
        ORA  C
        RZ              ;DONE
        DCR  E
        JNZ  DISP1      ;KEEP WITH CURRENT LINE
        DCR  D          ;FILLED PAGE YET?
        JNZ  DISP2
        CALL CHIN       ;WAIT FOR PAGE PROMT
        JMP  DISP+3
;*********************************
;ALLOW ESCAPE SEQUENCES TO CONTROL
;*********************************
ESCCO:  CALL CHIN       ;READ ESCAPE SEQUENCE CODE
        RET
;********************************
;OUTPUT HEX WITH TRAILING BYTE
;********************************
OHEXB:  CALL OHEX
        MVI  A,' '
        CALL CHAROUT
        RET
;*********************************
;OUTPUT 16 BIT ASCII HEX FROM H,L
;*********************************
OHEXHL: MOV  A,H
        CALL OHEX
        MOV  A,L
        CALL OHEXB
        RET
;*******************************************
;PROTECT /UNPROTECT RAM4A-4 MEMORY
;*******************************************
PROT:   INR  B          ;PROTECT/UNPROTECT FLAG
UNPRT:  CALL PARM2      ;GET START,END ADDRESSES IN H,L D,E
MEMP:   MOV  A,D
        ANI  0FCH       ;GET 1K OFFSET ONLY
        ORA  B
        MOV  D,A
        MOV  A,H
        ANI  0FCH
        ORA  B
PROT1:  OUT  0FEH       ;DO IT
        CMP  D          ;DONE?
        RZ              ;YES
        ADI  4          ;SET FOR NEXT 1K BLOCK
        JMP  PROT1
;**************************************************
;INTEL LOADER LOADS INTEL FORMAT TAPES FROM
;TELETYPE (PORT 2,3) 
;**************************************************
INTEL:  CALL CHIN       ;READ WITHOUT ECHO
        SBI  ':'        ;RCORD MARKER?
        JNZ  INTEL      ;NO
        MOV  D,A        ;ZERO CHECKSUM
        CALL IBYTE      ;INPUT 2 HEX CHARS
        ORA  A          ;SET FLAGS
        RZ              ;COUNT =0 MEANS END
        MOV  D,A        ;BYTE COUNT
        CALL IBYTE
        MOV  H,A
        CALL IBYTE
        MOV  L,A
        CALL IBYTE      ;DUMMY RECORD TYPE IGNORED
DATA:   CALL IBYTE
        MOV  M,A
        INX  H
        DCR  D
        JNZ  DATA
        CALL IBYTE      ;READ AND ADD CHECKSUM
        JZ   INTEL      ;OK AS IS
        MVI  A,'C'
        CALL CHAROUT    ;ERROR MESSAGE
        RET
;********************************************
;READ 2 ASCII HEX BYTES AND CONVERT TO BINARY
;********************************************
IBYTE:  CALL CHIN       ;READ CHAR
        CALL ASBI       ;CONVERT TO BINARY
        JC   ERR2
        ADD  A
        ADD  A
        ADD  A
        ADD  A
        MOV  E,A        ;SAVE
        CALL CHIN
        CALL ASBI
        JC   ERR2       ;INVALID ASCII HEX CHAR
        ADD  E
        MOV  E,A        ;SAVE CHAR
        ADD  D          ;ADD TO CHECKSUM
        MOV  D,A
        RET
;*************************************************
;SIZE INPUTS START,END ADDR AND CONVERTS TO START
;  AND SIZE IN H,L AND B,C
;*************************************************
SIZE:   CALL PARM2      ;H,L=START D,E=END
        PUSH PSW
        MOV  A,E
        SUB  L          ;LOW BYTE SIZE
        MOV  C,A
        MOV  A,D
        SBB  H          ;HIGH BYTE SIZE
        MOV  B,A
        INX  B          ;ADD 1
        POP  PSW
        RET
;***********************************************
;MEMORY MOVE "M SOURCE BEG,SOURCE END,DEST BEG"
;***********************************************
MOVE:   CALL PARM4      ;START,END,DEST
MOVE1:  CALL MVCUP      ;DO MOVE
        RET
;******************************
;FILL MEMORY WITH CHAR
;******************************
FILL:   CALL PARM4      ;START,END,FILL CHAR IN L
        MOV  A,E        ;FILL CHAR
        MOV  M,A        ;STORE IN FIRST LOCATION
        DCX  B
        MOV  D,H
        MOV  E,L        ;DEST ADDR
        INX  D          ;=START ADDR+1
        JMP  MOVE1
;*******************************
;MEMORY TEST ROUTINE
;*******************************
MEMTEST:CALL SIZE       ;H,L=START,B,C=SIZE
        DCX  B          ;B,C=SIZE-1 OR 0
MEM2:   XRA  A
        MOV  D,M        ;SAVE CELL
MEM1:   MOV  M,A
        CMP  M
        JNZ  MEMERR     ;NOT GOOD
        DCR  A          ;NEXT PATTERN
        JNZ  MEM1
        MOV  M,D        ;RESTORE MEMORY
        IN   3
        ANI  2          ;BAIL OUT?
        RNZ             ;YES
        INX  H
        DCX  B
        MOV  A,B
        ORA  C
        JNZ  MEM2
        RET
MEMERR: INX  H          ;ADJUST FOR PRNMEM
        MOV  E,A        ;SAVE
        CALL PNTMEM     ;PRINT ADDR,CONTENTS
        MOV  A,E        ;RESTORE
        JMP  SRCP1      ;PRINT SOULD BE
;******************************************
;DO DIRECT INPUT/OUTPUT FROM SPECIFIED PORT
;******************************************
INPORT: DCR  B          ;B=0=INPUT,B=1=OUTPUT
OUTPORT:CALL PARM2      ;INPUT PORT,VALUE IN H,L AND D,E
        MOV  A,B        ;FLAG
        RLC
        RLC
        RLC
        XRI  08H        ;INVERT BIT 3
        ORI  0D3H       ;FORM I/O INST
        MOV  D,L
        LHLD RAMPTR     ;GET AVAIL RAM PTR
        MOV  M,A
        CMP  M
        RNZ             ;INVALID RAM
        PUSH H
        INX  H
        MOV  M,D        ;PORT #
        INX  H
        MVI  M,0C9H     ;RETURN
        LXI  H,IORET
        XTHL            ;PUT RETURN ADDRESS,GET START ADDR
        MOV  A,B
        ORA  A          ;SET FLAG FOR IN OR OUT
        MOV  A,E        ;OUTPUT BYTE
        PCHL
IORET:  RNZ             ;DONE IF OUTPUT INST
        CALL OHEXB      ;PRINT VALUE IF INPUT
        RET
;************************************
;SET FREE RAM PTR FOR DIRECT IO INSTS
;************************************
RAMFND: CALL IHEX       ;GET RAM ADDR
        SHLD RAMPTR     ;SAVE IN VIO RAM
        RET
;************************************************
;COMPARE MEMORY BLOCKS AND PRINT DIFFERENCES
;************************************************
CMPBLK: CALL PARM4      ;START,SIZE,DEST IN HL BC,DE
CMPB1:  LDAX D          ;DEST BYTE
        CMP  M          ;SAME AS SOURCE BYTE?
        INX  H
        INX  D
        JZ   CMPB2      ;YES, NO PRINT
        CALL PNTMEM     ;PRINT ADDR,SOURCE DEST
        XCHG
        CALL PNTMEM+3   ;NO CRLF
        XCHG
CMPB2:  DCX  B
        MOV  A,B
        ORA  C
        RZ              ;YES,RETURN
        IN   3
        ANI  2
        RNZ             ;BAIL OUT
        JMP  CMPB1
;*****************************************
;SEARCH MEMORY FOR MASKED 16 BIT VALUE
;S,FROM,TO,16BIT VALUE,16 BIT MASK
;*****************************************
SEARCH: CALL PARM4      ;START,SIZE,VALUE IN H,L B,C D,E
        PUSH H          ;SAVE
        LXI  H,-1       ;DEFAULT MASK ALL
        CPI  0AH        ;USER SPECIFIED MASK?
        CNZ  IHEX       ;YES,READ IT INTO H,L
        XTHL            ;MASK ON STACK,START IN H,L
SEAR1:  MOV  A,M        ;LOW BYTE
        XTHL            ;H,L=MASK VALUE
        ANA  H          ;MASK HIGH BYTE
        CMP  D          ;IS IT CORRECT VALUE?
        XTHL            ;RESTORE START PTR
        INX  H          ;BUMP PTR
        JNZ  CMP16      ;NO MATCH
        MOV  A,M        ;LOW BYTE
        XTHL            ;GET MASK IN H,L
        ANA  L
        CMP  E
        XTHL            ;H,L=START,STACK=MASK
        CZ   SRCPNT     ;PRINT MATCH IF FOUND
CMP16:  DCX  B
        MOV  A,B
        ORA  C
        JNZ  SEAR1
        POP  B          ;REMOVE MASK VALUE
        RET
;**************************************************
;PARM4 INPUTS START,END,VALUE AND
;CONVERTS TO START,SIZE,VALUE IN H,L B,C AND D,E
;RESPECTIVELY
;**************************************************
PARM4:  CALL SIZE
        JMP  PARM3
;************************************************
;MVCUP MOVE B,C CHARS FROM H,L TO D,E FROM BOTTOM
;************************************************
MVCUP:  MOV  A,B
        ORA  C
        RZ
        MOV  A,M
        STAX D          ;MOVE IT
        DCX  B
        INX  H
        INX  D
        JMP  MVCUP      ;KEEP GOING
;**************************************************
;LOAD OR EXECUTE CASETTE FILE USING HEADER OR
;USER SPECIFIED START,END,EXEC ADDRESSES
;**************************************************
EXEC:   DCR  B          ;EXECUTE FLAG
LOAD:   PUSH B          ;SAVE EXEC/LOAD FLAG
        CALL PARM2      ;ANY PARMS SPECIFIED?
        MOV  A,D
        ORA  E
        JZ   HEADER     ;NO PARMS,OR NOT ENOUGH PARMS
;SKIP HEADER IF PRESENT
        PUSH H          ;START
        PUSH D          ;END
        CALL IHEX       ;GET EXEC IF ANY
        PUSH H          ;EXEC ADDR
        CALL RDHEAD     ;READ HEADER IF THERE
        JNZ  RDRCRDS    ;NOT THERE DO OBJECT
        POP  PSW
        POP  PSW
        POP  PSW        ;REMOVE HEADER PARMS
        JMP  RDRCRDS    ;DO OBJECT
HEADER: CALL RDHEAD     ;READ HEADER
ERR2:   MVI  A,'T'      ;TYPE CODE ERROR
        JZ   RDRCRDS    ;NO ERROR
ERR1:   CALL CHAROUT
        JMP  PRMPT      ;BAIL OUT
RDRCRDS:POP  B          ;EXEC
        POP  H          ;END
        POP  D          ;START
        PUSH B          ;RETURN EXEC ADDR
        PUSH H          ;END
        PUSH D          ;START
        MOV  A,L
        SUB  E
        MOV  L,A
        MOV  A,H
        SBB  D
        MOV  H,A
        DAD  H
        MOV  C,H
        INR  C          ;RECORD COUNT TO READ
RDRCO:  CALL CASIN      ;TYPE CODE
RDRC1:  CPI  81H        ;ABS BINARY?
        JNZ  ERR2       ;TYPE CODE ERROR
        CALL CASIN      ;BYTE COUNT
        MOV  B,A        ;SAVE RECORD BYTE COUNT
        LXI  H,0        ;0 CHECKSUM
RDATA:  CALL CAINCK     ;READ DATA BYTE
        STAX D          ;STORE IT
        INX  D
        DCR  B
        JNZ  RDATA      ;CONTINUE IF NOT DONE
        PUSH D          ;SAVE MEMORY PTR
        XCHG            ;DE=CHECKSUM
        CALL CASWD      ;READ TAPE CHECKSUM
        MOV  H,L
        MOV  L,A        ;REVERSE BYTES
        DAD  D          ;ADD TO COMPUTED CHECKSUM
        MOV  A,H
        ORA  L
        MVI  A,'C'      ;CHECKSUM ERROR
        CNZ  CHAROUT    ;TYPE 'C'
        JNZ  $+8
        MVI  A,'*'
        CALL CHAROUT    ;TYPE * FOR GOOD RECORD
        POP  D          ;RETRIEVE MEMORY PTR
        DCR  C          ;ALL RECORDS READ YET
        JNZ  RDRCO      ;NO READ ANOTHER
        CALL CRLF
        MVI  C,3
LP2:    POP  H
        CALL OHEXHL
        DCR  C
        JNZ  LP2
        POP  PSW        ;EXEC/LOAD FLAG
        RAR
        RC              ;DONE
        PCHL
;******************************
;READ HEADER RECORD
;******************************
RDHEAD: CALL CAINIT     ;INIT CASETTE
        CALL CASIN      ;READ TYPE CODE
        CPI  1          ;HEADER RECORD?
        RNZ             ;NO
        CALL CASIN      ;RECORD LENGTH
        MVI  C,5        ;NAME SIZE
NM1:    CALL CASIN      ;NAME BYTE
        CALL CHAROUT    ;DISPLAY IT
        DCR  C
        JNZ  NM1        ;DO IT TILL DONE
        MVI  C,3
ADDRS:  CALL CASWD      ;INPUT START,END,EXEC
        XTHL            ;EXCH RETURN ADDR WITH PARM
        PUSH H          ;PUSH RETURN ADDR AGAIN
        DCR  C
        JNZ  ADDRS
        CALL CASWD      ;DUMMY CHECKSUM
        XRA  A          ;ZER FLAGS FOR NORMAL RETURN
        RET
;**********************************************
;CAINIT READ CASETTE UNTIL 32 SYNC BYTES READ
;**********************************************
CAINIT: CALL BYTESET    ;READ FIRST SYNC
        MVI  B,31
CAIN2:  CALL CASIN
        CPI  0E6H
        MVI  A,'I'
        JNZ  ERR1
        DCR  B
        JNZ  CAIN2
        RET
;*******************************
;CASETTE OUTPUT BYTE
;*******************************
CASOUT: PUSH PSW        ;SAVE IT
        IN   3
        ANI  4
        JZ   CASOUT+1
        POP  PSW
        OUT  0          ;WRITE BYTE
        RET
;*****************************
;GENERATE SYNC STREAM
;*****************************
GEN:    CALL IHEX       ;WAIT FOR RETURN
        MVI  A,10H
        OUT  3          ;WRITE ENABLE MIO
GEN1:   MVI  A,0E6H
        CALL CASOUT
        IN   3
        ANI  2
        RNZ
        JMP  GEN1
;************************************************
;READ BYTE FROM CASETTE WITHOUT CHECKSUM
;************************************************
CASIN:  IN   3
        RRC
        RRC             ;C=SERIAL READY
        JC   PRMPT      ;BAIL OUT
        RRC             ;C=CASETTE READY
        JNC  CASIN      ;KEEP TRYING
        IN   0          ;DATA PORT
        RET
;*****************************************
;CAINCK- READ BYTE WITH CHECKSUM
;*****************************************
CAINCK: CALL CASIN
CHKSUM: PUSH B
        MOV  C,A        ;NEW CHAR IN LOW BYTE
        MVI  B,0
        DAD  B          ;ADD TO CHECKSUM
        POP  B          ;RESTORE
        RET
;**********************************************
;ALLIGN CASETTE BY READING AND DISPLAYING BYTES
;**********************************************
ALIGN:  CALL CHIN       ;WAIT FOR CR
        CALL BYTESET
ALL4:   LXI  H,REFRESH
        LXI  D,481      ;FILL SMALLEST SCREEN
ALL3:   DCX  D
        MOV  A,D
        ORA  E
        JZ   ALL4       ;START AGAIN EVERY 256 CHARS
        MVI  M,7FH
        CALL CASIN      ;READ NEXT CHAR
        MOV  M,A        ;PUT IN SCREEN
        INX  H
        JMP  ALL3
;*******************************************
;GET CASETTE IN BYTE MODE IE READ FIRST 0E6H
;*******************************************
BYTESET:MVI  A,60H      ;BIT MODE
        OUT  3
BYTE1:  CALL CASIN      ;READ BYTE EVERY BIT TIME
        CPI  0E6H       ;SYNC YET
        JNZ  BYTE1      ;NO
        MVI  A,20H      ;BYTE MODE
        OUT  3
        RET
;*******************************************
;CASWD-INPUT WORD TO H,L ADD FIRST BYTE ONLY
;TO CHECKSUM
;*******************************************
CASWD:  CALL CASIN      ;READ LOW BYTE
        MOV  L,A
        CALL CASIN      ;READ HIGH BYTE
        MOV  H,A
        RET
;********************************
;CHARACTER INPUT ROUTINES
;********************************
CHIN:   CALL CHIN1
        CPI  03         ;CRTL C?
        JZ   MONT       ;YES,RESET AND PROMPT
        CALL CHAROUT
        CPI  0DH
        CZ   CRLF       ;ADD LINE FEED
        RET
CHIN1:  IN   3
        ANI  2
        JZ   CHIN1
        IN   2          ;READ PORT 2
        ANI  7FH        ;MASK PARITY
        RET
        DB   0,0,0,0,0,0,0,0,0,0,0,0,0
;********************************************************
;PARM2 READ 2 PARAMATERS 16 BITS EACH INTO H,L AND D,E
;********************************************************
PARM2:  CALL IHEX
        MOV  D,H
        MOV  E,L
        CPI  0AH        ;TERMINATED?
        RZ              ;YES,USE SAME VALUE
        CPI  ','
        JZ   PARM3
        CPI  ' '
        RNZ             ;INVALID
PARM3:  XCHG
        CALL IHEX       ;GET SECOND PARM
        XCHG
        RET
;*********************************
;CRLF DO CARRIAGE RETURN/LINE FEED
;*********************************
CRLF:   MVI  A,0DH
        CALL CHAROUT
        MVI  A,0AH
        CALL CHAROUT
        RET
;:************************************************
;INPUT CHARS ASSUMED HEX AND CONVERT TO BINARY
;TERMINATES ON FIRST NO HEX CHAR WHICH IS LEFT
;IN A REG. H,L RETURNS WITH VALUE
;*************************************************
IHEX:   LXI  H,0
        CALL CHIN       ;READ CHAR
        PUSH PSW
        CALL ASBI       ;CONVERT TO BIBARY
        JNC  IHEX1
        POP  PSW
        RET
IHEX1:  DAD  H
        DAD  H
        DAD  H
        DAD  H          ;ADD NEW DIGIT
        ADD  L
        MOV  L,A
        POP  PSW
        JMP  IHEX+3
;********************************************************
;CONVERT ASCII HEX CHAR IN A-REG TO BINARY IN A REG
;RETURN WITH CARRY SET IF INVALID CHAR,RESET OTHERWISE
;********************************************************
ASBI:   SUI  30H        ;REMOVE ASCII BIAS
        RC              ;INVALID <0
        CPI  10
        JC   ASBI1      ;VALID 0-9
        SUI  17
        RC              ;INVALID
        ADI  10
        CPI  16         ;SET CARRY IF <0FH
ASBI1:  CMC
        RET
;*****************************************
;PRINT H,L AND 16 BIT MEMORY AT H,L
;*****************************************
SRCPNT: CALL PNTMEM
        MOV  A,M        ;BYTE 2
SRCP1:  CALL OHEX
        RET
PNTMEM: CALL CRLF
        DCX  H          ;BACK UP 1
        CALL OHEXHL
        MOV  A,M
        CALL OHEXB
        INX  H
        RET
;**********************************************
;OUTPUT HEX CHARS TO VIDIO FROM A REG
;**********************************************
OHEX:   PUSH PSW        ;SAVE CHAR
        RRC 
        RRC
        RRC
        RRC
        CALL BIAS       ;BINARY TO ASCII AND OUT
        POP  PSW
        CALL BIAS
        RET
;****************************
;CONVERT BINARY TO ASCII
;****************************
BIAS:   ANI  0FH        ;MASK NIBBLE
        ADI  90H
        DAA
        ACI  40H
        DAA
        CALL CHAROUT
        RET
;*********************************************
;OUTPUT MESSAGE PTED TO BY H,L AND TERMINATED
;BY ONE BYTE OF BINARY ZEROS
;*********************************************
MSG:    CALL CRLF
MSGNC:  MOV  A,M
        ORA  A
        RZ
        CALL CHAROUT
        INX  H
        JMP  MSGNC
CMDTBL  EQU  $
        DB   'H'
        DW   INTEL      ;INTEL HEX LOADS
        DB   'R'        ;FREE RAM LOCATION
        DW   RAMFND
        DB   'G'
        DW   GEN        ;GENERATE SYNC STREAM
        DB   'A'
        DW   ALIGN      ;ALLIGN CASETTE ON MIO
        DB   'V'
        DW   CMPBLK     ;COMPARE MEMORY BLOCKS
        DB   'I'
        DW   INPORT     ;INPUT FROM SPECIFIED PORT
        DB   'O'
        DW   OUTPORT    ;OUPUT TO SPECIFIED PORT
        DB   'T'
        DW   MEMTEST
        DB   'J'
        DW   JUMP       ;JUMP TO ADDRESS
        DB   'C'
        DW   CALL1      ;CALL MEMORY WITH RETURN
        DB   'D'        ;DISPLAY MEMORY
        DW   DISP
        DB   'E'
        DW   ENTR       ;ENTER INTO MEMORY
        DB   'M'
        DW   MOVE       ;MOVE MEMORY BLOCK
        DB   'F'        ;FILL MEMORY
        DW   FILL
        DB   'U'
        DW   UNPRT      ;UNPROTECT MEMORY
        DB   'P'
        DW   PROT       ;PROTECT MEMORY
        DB   'L'
        DW   LOAD       ;LOAD CASETTE
        DB   'S'
        DW   SEARCH     ;16 BIT MASKED SEARCH
        DB   'X'
        DW   EXEC       ;EXECUTE FROM CASETTE
        DB   1BH        ;ESCAPE CODE
        DW   ESCCO
        DB   0
        END

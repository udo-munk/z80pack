;       PAGE    62
        TITLE           'IMSAI SCS-1 Rev. 2 06 Oct. 1976'
;
; MODIFIED TO ASSEMBLE WITH INTEL 8080 CROSS ASSEMBLER
; OCTOBER 2008, UDO MUNK
;
TTS     EQU     03H
TTI     EQU     02H
TTO     EQU     02H
TTYDA   EQU     02H
TTYTR   EQU     01H
;
        ORG     0H
        JMP     INITA           ;DEAD START
        JMP     EOR             ;RESTART MONITOR
;
        ORG     08H
        JMP     BRKP            ;BREAKPOINT RESTART
;
        ORG     40H;
;
; THIS ROUTINE SETS UP THE SIO BOARD
;
INITA:  MVI     A,0AAH          ;GET DUMMY MODE WORD
        OUT     TTS             ;OUTPUT IT
        MVI     A,40H           ;GET RESET BIT
        OUT     TTS             ;RESET SIO BOARD
        MVI     A,0CEH          ;GET REAL MODE WORD
        OUT     TTS             ;SET THE MODE FOR REAL
        MVI     A,37H           ;GET THE COMMAND
        OUT     TTS             ;OUTPUT IT
;
; THIS ROUTINE INITIALIZES THE FILE AREAD FOR SUBSEQUENT
; PROCESSING
;
        LXI     H,FILE0
        MVI     C,MAXFIL*FELEN
        XRA     A
INIT2:  MOV     M,A
        INX     H
        DCR     C
        JNZ     INIT2
;
; CLEAR THE BREAKPOINT TABLE
;
        MVI     B,NBR*3
        LXI     H,BRT
INIT3:  MOV     M,A
        INX     H
        DCR     B
        JNZ     INIT3
;
; THIS IS THE STARTING POINT OF THE SELF CONTAINED
; SYSTEM ONCE THE SYSTEM HAS BEEN INITIALIZED.  COMMANDS
; ARE READ FROM THE USER, EXECUTED, AND CONTROL RETURNS
; BACK TO THIS POINT TO READ ANOTHER COMMAND.
;
EOR:    LXI     SP,AREA+18
        CALL    CRLF            ;PRINT C/R, LINE FEED
        CALL    READ            ;READ INPUT LINE
        INX     H
        MOV     A,M             ;FETCH FIRST CHARACTER
        CPI     '9'+1           ;COMMAND OR LINE NUMBER?
        JC      LINE            ;JUMP IF LINE FOR FILE
        CALL    VALC
        CALL    COMM
        JMP     EOR
;
; THIS ROUTINE READS IN A LINE FROM THE TTY AND PLACES
; IT IN AN INPUT BUFFER.
; THE FOLLOWING ARE SPECIAL CHARACTERS
;   CR          TERMINATES READ ROUTINE
;   LF          NOT RECOGNIZED BY ROUTINE
;   CTRL X      DELETES CURRENT LINE
;   DEL         DELETES CHARACER
; ALL DISPLAYABLE CHARACTERS BETWEEN BLANK & Z AND THE
; ABOVE ARE RECOGNIZED BY THE READ ROUTINE, ALL OTHERS
; ARE SKIPPED OVER.  THE ROUTINE WILL NOT ACCEPT MORE
; CHARACTERS THAN THE INPUT BUFFER WILL HOLD.
;
READ:   LXI     H,IBUF          ;GET INPUT BUFFER ADDRESS
        SHLD    ADDS            ;SAVE ADDRESS
        MVI     E,2             ;INITIALIZE CHARACTER COUNT
NEXT:   CALL    IN8             ;READ A LINE
        MOV     A,B
        CPI     24              ;CHECK FOR CTRL X
        JNZ     CR
        CALL    CRLF            ;OUTPUT A CRLF
        JMP     READ
CR:     CPI     ASCR            ;GET AN ASCII CR
        JNZ     DEL
        MOV     A,L
        CPI     IBUF AND 00FFH  ;CHECK FOR FIRST CHAR
        JZ      READ
        MVI     M,ASCR          ;PLACE CR AT END OF LINE
        INX     H
        MVI     M,1             ;PLACE EOF INDICATOR IN LINE
        INX     H
        MVI     A,IBUF+83 AND 00FFH
        CALL    CLER            ;CLEAR REMAINING BUFFER
        LXI     H,IBUF-1
        MOV     M,E             ;SAVE CHARACTER COUNT
        RET
DEL:    CPI     127             ;CHECK FOR DELETE CHARACTER
        JNZ     CHAR
        MVI     A,IBUF AND 00FFH
        CMP     L               ;IS IT 1ST CHARACTER
        JZ      NEXT
        DCX     H               ;DECREMENT POINTER
        DCR     E               ;DECREMENT COUNT
BSPA:   MVI     B,5FH
        CALL    OUT8
        JMP     NEXT
CHAR:   CPI     ' '             ;CHECK FOR LEGAL CHARACTER
        JC      NEXT
        CPI     'Z'+1
        JNC     NEXT
        MOV     B,A
        CALL    OUT8            ;ECHO CHARACTER
        MOV     M,B
        MVI     A,IBUF+81 AND 00FFH
        CMP     L               ;CHECK FOR END OF LINE
        JZ      BSPA
        INX     H
        INR     E               ;INCREMENT CHARACTER COUNT
        JMP     NEXT
;
; THIS ROUTINE IS USED TO BLANK OUT A PORTION OF MEMORY
;
CLER:   CMP     L
        RZ      
        MVI     M,' '           ;PLACE BLANK IN MEMORY
        INX     H
        JMP     CLER
;
; SEE IF TTY INPUT READY AND CHECK FOR CTRL X.
;
INK:    IN      TTS             ;GET TTY STATUS
        CMA                     ;INVERT STATUS
        ANI     TTYDA           ;IS DATA AVAILABLE?
        RNZ                     ;RETURN IF NOT
        IN      TTI             ;GET THE CHAR
        ANI     07FH            ;STRIP OFF PARITY
        CPI     'X'-40H         ;IS IT A CTRL X?
        RET
;
; THIS ROUTINE READS A BYTE OF DATA FROM THE USART
;
IN8:    IN      TTS             ;READ USART STATUS
        ANI     TTYDA
        JZ      IN8
        IN      TTI             ;READ DATA
        ANI     127             ;STRIP OFF PARITY
        MOV     B,A
        RET

;
; THIS ROUTINE OUTPUTS A BYTE OF DATA TO THE USART
;
OUT8:   IN      TTS             ;READ STATUS
        ANI     TTYTR
        JZ      OUT8
OK:     MOV     A,B
        OUT     TTO             ;TRANSMIT DATA
        RET
;
; THIS ROUTINE WILL OUTPUT A CARRIAGE RETURN AND
; LINE FEED FOLLOWED BY TWO DELETE CHARACTERS WHICH
; PROVIDE TIME FOR PRINT HEAD TO RETURN.
;
CRLF:   MVI     B,13            ;CR
        CALL    OUT8
LF:     MVI     B,10            ;LF
        CALL    OUT8
        MVI     B,127
        CALL    OUT8
        CALL    OUT8
        RET
;
; THIS ROUTINE JUMPS TO A LOCATION IN MEMORY GIVEN BY
; THE INPUT COMMAND AND BEGINS EXECUTION OF PROGRAM
; THERE.
;
EXEC:   CALL    VCHK            ;CHECK FOR PARAMETERS
        CALL    CRLF
        LHLD    BBUF            ;FETCH ADDRESS
        PCHL                    ;JUMP TO PROGRAM
;
; THIS ROUTINE CHECKS THE INPUT COMMAND AGAINS ALL
; LEGAL COMMANDS STORED IN A TABLE.  IF A LEGAL COMMAND
; IS FOUND, A JUMP IS MADE TO THAT ROUTINE.  OTHERWISE
; AN ERROR MESSAGE IS OUTPUT TO THE USER.
;
COMM:   LXI     D,CTAB          ;COMMAND TABLE ADDRESS
        MVI     B,NCOM          ;NUMBER OF COMMANDS
        MVI     A,4             ;LENGTH OF COMMAND
        STA     NCHR            ;SAVE
        CALL    COMS            ;SEARCH TABLE
        JNZ     WHAT            ;JUMP IF ILLEGAL COMMAND
        PCHL                    ;BE HERE NOW
;
; THIS ROUTINE CHECKS TO SEE IF A BASE CHARACTER STRING
; IS EQUAL TO ANY OF THE STRINGS CONTAINED IN A TABLE
; POINTED TO BY D,E.  tHE TABLE CONSISTS OF ANY NUMBER
; OF CHARS, WITH 2 BYTES CONTAINING VALUES ASSOCIATED
; WITH IT.  REG B CONTAINS THE # OF STRINGS TO COMPARE.
; THIS ROUTINE CAN BE USED TO SEARCH THROUGH A COMMAND
; OR SYMBOL TABLE.  ON RETURN, IF THE ZERO FLAG IS SET,
; A MATCH WAS FOUND; IF NOT, NO MATCH WAS FOUND.  IF
; A MATCH WAS FOUND, D,E POINT TO THE LAST BYTE
; ASSOCIATED WITH THE CHARACTER STRING.  iF NOT, D,E
; POINT TO THE NEXT LOCATION AFTER THE END FO THE TABLE.
;
COMS:   LHLD    ADDS            ;FETCH COMPARE ADDRESS
        LDA     NCHR            ;GET LENGTH OF STRING
        MOV     C,A
        CALL    SEAR            ;COMPARE STRINGS
        LDAX    D               ;FETCH VALUE
        MOV     L,A
        INX     D
        LDAX    D               ;FETCH VALUE
        MOV     H,A
        RZ      
        INX     D               ;SET TO NEXT STRING
        DCR     B               ;DECREMENT COUNT
        JNZ     COMS
        INR     B               ;CLEAR ZERO FLAG
        RET
;
; THIS ROUTINE CHECKS TO SEE IF TWO CHARACTER STRINGS IN
; MEMORY ARE EQUAL.  tHE STRINGS ARE POINTED TO BY D,E
; AND H,L.  ON RETURN, THE ZERO FLAG SET INDICATES A
; MATCH.  REG C INDICATES THE LENGTH OF THE STRINGS.  ON
; RETURN, THE POINTERS POINT TO THE NEXT ADDRESS AFTER
; THE CHARACTER STRINGS.
;
SEAR:   LDAX    D               ;FETCH CHARACTER
        CMP     M               ;COMPARE CHARACTERS
        JNZ     INCA
        INX     H
        INX     D
        DCR     C               ;DECREMENT CHARACTER COUNT
        JNZ     SEAR
        RET
INCA:   INX     D
        DCR     C
        JNZ     INCA
        INR     C               ;CLEAR ZERO FLAG
        RET
;
; THIS ROUTINE ZEROES OUT A BUFFER IN MEMORY WHICH IS
; THEN USED BY OTHER SCANNING ROUTINES
;
ZBUF:   XRA     A               ;GET A ZERO
        LXI     D,ABUF+12       ;BUFFER ADDRESS
        MVI     B,12            ;BUFFER LENGTH
ZBU1:   DCX     D               ;DECREMENT ADDRESS
        STAX    D               ;ZERO BUFFER
        DCR     B
        JNZ     ZBU1
        RET
;
; THIS ROUTINE CALLS ETRA TO OBTAIN THE INPUT PARAMETER
; VALUES AND CALLS AN ERROR ROUTINE IF AN ERROR OCCURED
; IN THAT ROUTINE
;
VALC:   CALL    ETRA            ;GET INPUT PARAMETERS
        JC      WHAT            ;JUMP IF ERROR
        RET
;
; THIS ROUTINE EXTRACTS THE VALUES ASSOCIATED WITH A
; COMMAND FROM THE INPUT STREAM AND PLACES THEM IN THE
; ASCII BUFFER (ABUF).  IT ALSO CALLS A ROUTINE TO
; CONVERT THE ASCII HEXADECIMALS TO BINARY AND STORES
; THEM IN THE BINARY BUFFER (BBUF).  ON RETURN, CARRY
; SET INDICATES AN ERROR IN INPUT PARAMETERS.
;
ETRA:   LXI     H,0             ;GET A ZERO
        SHLD    BBUF+2          ;ZERO VALUE
        SHLD    FBUF            ;SET NO FILE NAME
        CALL    ZBUF            ;ZERO BUFFER
        LXI     H,IBUF-1
VAL1:   INX     H
        MOV     A,M             ;FETCH INPUT CHARACTER
        CPI     ' '             ;LOOK FOR FIRST CHARACTER
        CMC
        RNC                     ;RETURN IF NO CARRY
        JNZ     VAL1            ;JUMP IF NO BLACK
        SHLD    PNTR            ;SAVE POINTER
        CALL    SBLK            ;SCAN TO FIRST PARAMETER
        CMC
        RNC                     ;RETURN IF CR
        CPI     '/'
        JNZ     VAL5            ;NO FILE NAME
        LXI     D,FBUF          ;NAME FOLLOWS PUT IN FBUF
        MVI     C,NMLEN
VAL2:   INX     H
        MOV     A,M
        CPI     '/'
        JZ      VAL3
        DCR     C
        JM      WHAT
        STAX    D               ;STORE FILE NAME
        INX     D
        JMP     VAL2
VAL3:   MVI     A,' '           ;GET AN ASCII SPACE
VAL4:   DCR     C
        JM      DONE
        STAX    D               ;FILL IN WITH SPACES
        INX     D
        JMP     VAL4
DONE:   CALL    SBL2
        CMC
        RNC
VAL5:   LXI     D,ABUF
        CALL    ALPS            ;PLACE PARAMETER IN BUFFER
        MOV     A,B             ;GET DIGIT COUNT
        CPI     5               ;CHECK NUMBER OF DIGITS
        CMC
        RC                      ;RETURN IF TOO MANY DIGITS
        LXI     B,ABUF
        CALL    AHEX            ;CONVERT VALUE
        RC                      ;ILLEGAL CHARACTER
        SHLD    BBUF            ;SAVE IN BINARY BUFFER
        LXI     H,ABUF
        CALL    NORM            ;NORMALIZE ASCII VALUE
        CALL    SBLK            ;SCAN TO NEXT PARAMETER
        CMC
        RNC                     ;RETURN IF cr
        LXI     D,ABUF+4
        CALL    ALPS            ;PLACE PARAMETERS IN BUFFER
        MOV     A,B             ;GET DIGIT COUNT
        CPI     5               ;CHECK NUMBER OF DIGITS
        CMC
        RC                      ;RETURN IF TOO MANY DIGITS
        LXI     B,ABUF+4
        CALL    AHEX            ;CONVERT VALUE
        RC                      ;ILLEGAL VALUE
        SHLD    BBUF+2          ;SAVE IN BINARY BUFFER
        LXI     H,ABUF+4
        CALL    NORM            ;NORMALIZE ASCII VALUE
        ORA     A               ;CLEAR CARRY
        RET
;
; THIS ROUTINE FETCHES DIGITS FROM THE BUFFER ADDRESSED
; BY B,C AND CONVERTS THE ASCII DECIMAL DIGITS INTO
; BINARY.  UP TO A 16-BIT VALUE CAN BE CONVERTED.  THE
; SCAN STOPS WHEN A BINARY ZERO IS FOUND IN THE BUFFER.
;
ADEC:   LXI     H,0             ;GET A 16 BIT ZERO
ADE1:   LDAX    B               ;FETCH ASCII DIGIT
        ORA     A               ;SET ZERO FLAG
        RZ                      ;RETURN IFF FINISHED
        MOV     D,H             ;SAVE CURRENT VALUE
        MOV     E,L             ;SAVE CURRENT VALUE
        DAD     H               ;TIMES TWO
        DAD     H               ;TIMES TWO
        DAD     D               ;ADD IN ORIGINAL VALUE
        DAD     H               ;TIMES TWO
        SUI     48              ;ASCII BIAS
        CPI     10              ;CHECK FOR LEGAL VALUE
        CMC
        RC                      ;RETURN IF ERROR
        MOV     E,A
        MVI     D,0
        DAD     D               ;ADD IN NEXT DIGIT
        INX     B               ;INCREMENT POINTER
        JMP     ADE1
;
; THIS ROUTINE FETCHES DIGITS FROM THE BUFFER ADDRESSED
; BY B,C AND CONVERTS THE ASCII HEXADECIMAL DIGITS INTO
; BINARY.  UP TO A 16-BIT VALUE CAN BE CONVERTED.  THE
; SCAN STOPS WHEN A BINARY ZERO IS FOUNDIN THE BUFFER.
;
AHEX:   LXI     H,0             ;GET A 16 BIT ZERO
AHE1:   LDAX    B               ;FETCH ASCII DIGIT
        ORA     A               ;SET ZERO FLAG
        RZ                      ;RETURN IF DONE
        DAD     H               ;LEFT SHIFT
        DAD     H               ;LEFT SHIFT
        DAD     H               ;LEFT SHIFT
        DAD     H               ;LEFT SHIFT
        CALL    AHS1            ;CONVERT TO BINARY
        CPI     10H             ;CHECK FOR LEGAL VALUE
        CMC
        RC                      ;RETURN IF ERROR
        ADD     L
        MOV     L,A
        INX     B               ;INCREMENT POINTER
        JMP     AHE1
;
; THIS ROUTINE CONVERTS ASCII HEX DIGITS INTO BINARY
;
AHS1:   SUI     48              ;ASCII BIAS
        CPI     10              ;DIGIT 0-10
        RC
        SUI     7               ;ALPHA BIAS
        RET
;
; THIS ROUTINE CONVERTS A BINARY VALUE TO ASCII
; HEXADECIMAL AND OUTPUTS THE CHARACTERS TO THE TTY.
;
HOUT:   CALL    BINH
        LXI     H,HCON
CHOT:   MOV     B,M
        CALL    OUT8
        INX     H
        MOV     B,M
        CALL    OUT8
        RET
;
; THIS ROUTINE DOES THE SAME AS ABOVE BUT OUTPUTS A
; BLANK AFTER THE LAST CHARACTER
;
HOTB:   CALL    HOUT            ;CONVERT AND OUTPUT
        CALL    BLK1            ;OUTPUT A BLANK
        RET
;
; THIS ROUTINE CONVERTS A BINARY VALUE TO ASCII
; DECIMAL DIGITS AND OPTPUTS THE CHARACTERS TO THE TTY
;

DOUT:   CALL    BIND            ;CONVERT VALUE
        CALL    HOUT+3          ;OUTPUT VALUE (2 DIGITS)
        INX     H
        MOV     B,M             ;GET LAST DIGIT
        CALL    OUT8            ;OUTPUT
        RET
;
; THIS ROUTINE OUTPUTS A BLANK
;
BLK1:   MVI     B,' '           ;GET A BLANK
        CALL    OUT8
        RET
;
; THIS ROUTINE IS USED BY OTHER ROUTINES TO INCREMENT
; THE STARTING ADDRESS IN A COMMAND AND COMPARE IT WITH
; THE FINAL ADDRESS IN THE COMMAND.  ON RETURN, THE
; CARRY FLAG SET INDICATES THAT THE FINAL ADDRESS HAS
; BEEN REACHED.
;
ACHK:   LHLD    BBUF            ;FETCH START ADDRESS
        LDA     BBUF+3          ;STOP ADDRESS (HIGH)
        CMP     H               ;COMPARE ADDRESSES
        JNZ     ACH1
        LDA     BBUF+2          ;STOP ADDRESS (LOW)
        CMP     L               ;COMPARE ADDRESSES
        JNZ     ACH1
        STC                     ;SET CARRY IF EQUAL
ACH1:   INX     H               ;INCREMENT START ADDRESSES
        SHLD    BBUF            ;STORE START ADDRESS
        RET
;
; THIS ROUTINE OUTPUTS CHARACTER OF A STRING
; UNTIL A CARRIAGE RETURN IS FOUND
;
SCRN:   MOV     B,M             ;FETCH CHARACTER
        MVI     A,13            ;CARRIAGE RETURN
        CMP     B               ;CHARACTER = CR?
        RZ      
        CALL    OUT8
        INX     H
        JMP     SCRN
;
; THIS ROUTINE CONVERTS THE BINARY VALUE IN REG A INTO
; ASCII HEXADECIMAL DIGITS AND STORES THEM IN MEMORY
;
BINH:   LXI     H,HCON          ;CONVERSION
        MOV     B,A             ;SAVE VALUE
        RAR
        RAR
        RAR
        RAR
        CALL    BIN1
        MOV     M,A
        INX     H
        MOV     A,B
        CALL    BIN1            ;CONVERT TO ASCII
        MOV     M,A
        RET
;
; THIS ROUTINE CONVERTS A VALUE TO HEXADECIMAL
;
BIN1:   ANI     0FH             ;LOW 4 BITS
        ADI     48              ;CONVERT TO ASCII
        CPI     58              ;DIGIT 0-9
        RC
        ADI     7               ;MODIFY FOR A-F
        RET
;
; THIS ROUTINE CONVERTS THE BINARY VALUE IN REG A INTO
; ASCII DECIMAL DIGITS AND STORES THEM IN MEMORY
;
BIND:   LXI     H,HCON          ;CONVERSION ADDRESS
        MVI     B,100
        CALL    BID1            ;CONVERT HUNDREDS DIGIT
        MVI     B,10
        CALL    BID1            ;CONVERT TENS DIGIT
        ADI     '0'             ;GET UNITS DIGIT
        MOV     M,A             ;STORE IN MEMORY
        RET
;
; THIS ROUTINE CONVERTS A VALUE TO DECIMAL
;
BID1:   MVI     M,'0'-1         ;INITIALIZE DIGIT COUNT
        INR     M
        SUB     B               ;CHECK DIGIT
        JNC     BID1+2
        ADD     B               ;RESTORE VALUE
        INX     H
        RET
;
; LEGAL COMMAND TABLE
;
CTAB:   DB      'DUMP'
        DW      DUMP
        DB      'EXEC'
        DW      EXEC
        DB      'ENTR'
        DW      ENTR
        DB      'FILE'
        DW      FILE
        DB      'LIST'
        DW      LIST
        DB      'DELT'
        DW      DELL
        DB      'ASSM'
        DW      ASSM
        DB      'PAGE'
        DW      PAGEMOV
        DB      'CUST'
        DW      2000H
        DB      'BREK'
        DW      BREAK
        DB      'PROC'
        DW      PROC
;
; THIS ROUTINE CHECKS IF ANY PARAMETERS WERE ENTERED
; WITH THE COMMAND, IF NOT AN ERROR MESSAGE IS ISSUED
;
VCHK:   LDA     ABUF            ;FETCH PARAMETER BYTE
        ORA     A               ;SET FLAGS
        JZ      WHAT            ;NO PARAMETER
        RET
;
; THIS ROUTINE DUMPS OUT THE FONTENTS OF MEMORY FROM
; THE START TO FINAL ADDRESSES GIVEN IN THE COMMAND.
;
DUMP:   CALL    VCHK            ;CHECK FOR PARAMETERS
DUMS:   CALL    CRLF            ;START NEW LINE
DUM1:   LHLD    BBUF            ;FETCH MEMORY ADDRESS
        MOV     A,M
        CALL    HOTB            ;OUTPUT VALUE
        CALL    ACHK            ;CHECK ADDRESS
        RC                      ;RETURN IF FINISHED
        MOV     A,L             ;IS NEXT ADDRESS
        ANI     0FH             ; DIVISIBLE BY 16?
        JNZ     DUM1
        JMP     DUMS
;
; THIS ROUTINE WILL MOVE 256 BYTES FROM 1ST ADDRESS
; GIVEN IN COMMAND TO 2ND ADDRESS IN COMMAND.
;
PAGEMOV:CALL    VCHK            ;CHECK FOR PARAMETER
        LDA     ABUF+4          ;FETCH 2ND PARAMETER
        ORA     A               ;DOES 2ND PARAMETER EXIST?
        JZ      WHAT
        LHLD    BBUF            ;FETCH MOVE TO ADDRESS
        XCHG
        LHLD    BBUF+2          ;FETCH MOVE TO ADDRESS
        MVI     B,0             ;SET COUNTER
PAG1:   LDAX    D
        MOV     M,A
        INX     H
        INX     D
        DCR     B               ;DECREMENT COUNTER
        JNZ     PAG1
        RET
;
; THIS COMMAND INITIALIZES THE BEGINNING OF FILE ADDRESS
; AND END OF FILE ADDRESS AS WELL AS THE FILE AREA
; WHEN THE FILE COMMAND IS USED
;
FILE:   CALL    CRLF
; CHECK FOR FILE PARAMETERS
        LDA     FBUF
        ORA     A
        JZ      FOUT            ;NO ? GO LIST
        CALL    FSEA            ;LOOK UP FILE
        XCHG                    ;PNTR IN DE
        JNZ     TEST
; NO ENTRY
        LDA     ABUF            ;CHECK FOR PARAM
        ORA     A
        JZ      WHA1            ;NO?? - ERROR
; CHECK FOR ROOM IN DIRECTORY
        LDA     FEF
        ORA     A
        JNZ     ROOM
        LXI     H,EMES1
        JMP     MESS
; ENTRY FOUND ARE THESE PARAMETERS
TEST:   LDA     ABUF
        ORA     A
        JZ      SWAPS
        LHLD    BBUF
        MOV     A,H
        ORA     L
        JZ      SWAPS
        LXI     H,EMES2         ;NO-NO CAN?T DO
        JMP     MESS            ;IT - DELETE FIRST
; MOVE FILE NAME TO BLOCK POINTED TO BY FREAD
ROOM:   LHLD    FREAD
        XCHG
        LXI     H,FBUF          ;FILE NAME POINTER IN H,L
        PUSH    D
        MVI     C,NMLEN         ;NAME LENGTH COUNT
MOV23:  MOV     A,M
        STAX    D
        INX     D
        DCR     C               ;TEST COUNT
        INX     H
        JNZ     MOV23
        POP     D               ;RESTORE ENTRY POINTER
; MAKE FILE POINTED TO BY D,E CURRENT
SWAPS:  LXI     H,FILE0
        MVI     C,FELEN         ;ENTRY LENGTH
SWAP:   LDAX    D
        MOV     B,M
        MOV     M,A             ;EXCHANGE
        MOV     A,B
        STAX    D
        INX     D
        INX     H               ;BUMP POINTER
        DCR     C               ;TEST COUNT
        JNZ     SWAP

; CHECK FOR 2ND PARAMETER
            LDA ABUF
        ORA     A
        JZ      FOOT            ;NO SECOND PARAMETER
; PROCESS 2ND PARAMETER
        LHLD    BBUF            ;GET ADDRESS
        SHLD    BOFP            ;SET BEGIN
        SHLD    EOFP            ;SET END
        MOV     A,L             ;IS ADDRESS ZERO
        ORA     H
        JZ      FIL35           ;YES
FIL30:  MVI     M,1             ;NON-ZERO ? SET EOF
FIL35:  XRA     A               ;AND MAX LINE #
        STA     MAXL
        JMP     FOOT            ;OUTPUT PARAMETERS
FOUT:   LDA     IBUF+4
        CPI     'S'             ;IS COMMAND FILES?
        MVI     C,MAXFIL
        JZ      FOUL
FOOT:   MVI     C,1
; OUTPUT THE # OF ENTRIES IN C
FOUL:   LXI     H,FILE0
        MOV     A,C
FINE:   STA     FOCNT           ;SAVE COUNT
        PUSH    H
        LXI     D,NMLEN
        DAD     D
        MOV     A,M
        ORA     A
        JNZ     FOOD
        INX     H
        ADD     M
        INX     H
        JNZ     FOOD            ;NON ZERO, OK TO OUTPUT
        INX     SP
        INX     SP
        INX     H
        INX     H
        JMP     FEET
; HAVE AN ENTRY TO OUTPUT
FOOD:   POP     H               ;PTR
        MVI     C,NMLEN
FAST:   MOV     B,M             ;LOAD CHARACTER TO B
        CALL    OUT8
        DCR     C
        INX     H
        JNZ     FAST            ;DO THE REST
; NOW OUTPUT BEGIN-END PTRS
        CALL    FOOL            ;OUTPUT BEGIN
        CALL    FOOL            ;OUTPUT END
        CALL    CRLF            ;AND C/R
; TEST COUNT, H,L POINTS PAST EOFP
FEET:   LXI     D,FELEN-NMLEN-4
        DAD     D               ;MOVE TO NEXT ENTRY
        LDA     FOCNT
        DCR     A               ;TEST COUNT
        JNZ     FINE            ;MORE TO DO
        RET                     ;DONE!
; OUTPUT NUMBER POINTED TO BY H,L
; ON RET, H,L POINT 2 WORDS LATER
FOOL:   CALL    BLK1            ;SPACE
        INX     H
        MOV     A,M
        DCX     H
        PUSH    H
        CALL    HOUT            ;OUTPUT
        POP     H
        MOV     A,M
        INX     H
        INX     H
        PUSH    H
        CALL    HOTB            ;OUTPUT
        POP     H               ;RESTORE H,L
        RET
;
; SEARCH THE FILE DIRECTORY FOR THE FILE
; WHOSE NAME IS IN FBUF.
; RETURN IF FOUND, ZERO IF OFF, H,L POINT TO
; ENTRY WHILE SEARCHING, ON ENTRY FOUND WITH ADDR
; ZERO, SET FEF TO >0 AND FREAD TO THE ADDR OF ENTRY
;
FSEA:   XRA     A
        STA     FEF             ;CLAIM NO FREE ENTRIES
        MVI     B,MAXFIL        ;COUNT OF ENTRIES
        LXI     D,FILE0         ;TABLE ADDRESS
FSE10:  LXI     H,FBUF
        MVI     C,NMLEN
        CALL    SEAR            ;TEST STRINGS
        PUSH    PSW             ;SAVE FLAG
        PUSH    D
        LDAX    D               ;GET BOFP
        ORA     A               ;EMPTY ENTRY?
        JNZ     FSE20
        INX     D               ;STORE OTHER WORD
        LDAX    D
        ORA     A
        JNZ     FSE20           ;NOPE-GO TEST FOR MATCH
        XCHG
        LXI     D,-NMLEN-1
        DAD     D               ;MOVE TO BEGINNING
        SHLD    FREAD           ;SAVE ADDR
        MOV     A,D
        STA     FEF             ;SET FREE ENTRY FOUND
        POP     H               ;RESTORE INTERIM PTR
        POP     PSW             ;UNJUNK STACK
; MOVE TO NEXT ENTRY
FSE15:  LXI     D,FELEN-NMLEN
        DAD     D
        XCHG                    ;NEXT ENTRY IN DE
        DCR     B               ;TEST COUNT
        RZ                      ;DONE--NOPE
        JMP     FSE10           ;TRY NEXT
; ENTRY WASN?T FREE, TEST FOR MATCH
FSE20:  POP     H
        POP     PSW
        JNZ     FSE15           ;IF ZERO CLEAR, NO MATCH
; ENTRY FOUND
        LXI     D,-NMLEN        ;BACKUP
        DAD     D               ;H,L POINTS TO ENTRY
        MOV     A,D
        ORA     A               ;CLEAR ZERO
        RET                     ;THAT?S ALL
;
; OUTPUT ERROR MESSAGE FOR ILLEGAL COMMAND
;
WHAT:   CALL    CRLF            ;OUT CRLF
WHA1:   LXI     H,EMES          ;MESSAGE ADDRESS
MESS:   CALL    SCRN
        JMP     EOR
;
EMES:   DB      'WHAT'
        DB      13
EMES1:  DB      'FULL',13
EMES2:  DB      'NO NO',13
;
; CALL ROUTINE TO ENTER DATA INTO MEMORY
; AND CHECK FOR ERROR ON RETURN
;
; THIS ROUTINE IS USED TO ENTER DATA VALUES INTO MEMORY.
; EACH VALUE IS ONE BYTE AND IS WRITTEN IN HEXADECIMAL
; VALUES GREATER THAN 255 WILL CAUSE CARRY TO BE SET
; AND RETURN TO BE MADE TO CALLING PROGRAM
;
ENTR:   CALL    VCHK            ;CHECK FOR PARAMETERS
        CALL    ENTS
        JC      WHAT
        CALL    CRLF
        RET
;
EEND    EQU     '/'             ;TERMINATION CHARACTER
ENTS:   CALL    CRLF
        CALL    READ            ;READ INPUT DATA
        LXI     H,IBUF          ;SET LINE POINTER
        SHLD    PNTR            ;SAVE POINTER
ENT1:   CALL    ZBUF            ;CLEAR BUFFER
        CALL    SBLK            ;SCAN TO FIRST VALUE
        JC      ENTS            ;JUMP IF CR FOUND
        CPI     EEND
        RZ                      ;RETURN CARRY IS ZERO
        CALL    ALPS            ;PLACE VALUE IN BUFFER
        MOV     A,B             ;GET DIGIT COUNT
        CPI     3               ;CHECK NMBR OF DIGITS
        CMC
        RC                      ;RETURN IF MORE THAN 2 DIGITS
        LXI     B,ABUF          ;CONVERSION ADDRESS
        CALL    AHEX            ;CONVERT VALUE
        RC                      ;ERROR IN HEX CHARACTER
        MOV     A,L
        LHLD    BBUF            ;FETCH MEMORY ADDRESS
        MOV     M,A             ;PUT IN MEMORY
        CALL    ACH1            ;INCREMENT MEMORY LOCATION
        JMP     ENT1
;
; THIS ROUTINE IS USED TO ENTER LINES INTO THE FILE
; AREA.  THE LINE NUMBER IS FIRST CHECKED TO SEE IF IT IS
; A VALID NUMBER (0000-9999).  NEXT IT IS CHECKED TO SEE
; IF IT IS GREATER THAN THE MAXIMUM CURRENT LINE NUMBER.
; IF IT IS, THE NEXT LINE IS INSERTED AT THE END OF THE
; CURRENT FILE AND THE MAXIMUM LINE NUMBER IS UPDATED AS
; WELL AS THE END OF FILE POSITION.  LINE NUMBERS THAT
; ALREADY EXIST ARE INSERTED INTO THE FILE AREA AT THE
; APPROPRIATE PLACE AND ANY EXTRA CHARACTERS IN THE OLD
; LINE ARE DELETED.
;
LINE:   LDA     FILE0           ;IS A FILE DEFINED?
        ORA     A
        JZ      WHAT            ;ABORT IF NOT
        MVI     C,4             ;NO OF DIGITS TO CHECK
        LXI     H,IBUF-1                ;INITIALIZE ADDRESS
LICK:   INX     H
        MOV     A,M             ;FETCH LINE DIGIT
        CPI     '0'             ;CHECK FOR VALID NUMBER
        JC      WHAT
        CPI     '9'+1
        JNC     WHAT
        DCR     C
        JNZ     LICK
        SHLD    ADDS            ;FIND ADDRESS
        LXI     D,MAXL+3                ;GET ADDRESS
        CALL    COM0
        JNC     INSR
; GET HERE IF NEW LINE IS GREATER THAN MAXIMUM LINE #
        INX     H
        CALL    LODM            ;GET NEW LINE NUMBER
        LXI     H,MAXL+3
        CALL    STOM            ;MAKE IT MAXIMUM LINE NUMBER
        LXI     D,IBUF-1
        LHLD    EOFP            ;END OF FILE POSITION
        MVI     C,1
        CALL    LMOV            ;PLACE LINE IN FILE
SEOF:   MVI     M,1             ;END OF FILE INDICATOR
        SHLD    EOFP            ;END OF FILE ADDRESS
        JMP     EOR
; GET HERE IF NEW LINE MUST BE INSERTED INTO ALREADY
; EISTING FILE AREA
INSR:   CALL    FIN1            ;FIND LINE IN FILE
        MVI     C,2
        JZ      EQUL
        DCR     C               ;NEW LN NOT EQUAL TO SOME OLD LN
EQUL:   MOV     B,M
        DCX     H
        MVI     M,2             ;MOVE LINE INDICATOR
        SHLD    INSP            ;INSERT LINE POSITION
        LDA     IBUF-1          ;NEW LINE COUNT
        DCR     C
        JZ      LESS            ;NEW LN NOT = OLD LN
        SUB     B               ;COUNT DIFFERENCE
        JZ      ZERO            ;LINE LENGTHS EQUAL
        JC      MORE
; GET HERE IF # OF CHARS IN OLD LINE > # OF CHARS IN
; NEW LINE OR NEW LINE # WAS NOT EQUAL TO SOLD OLD
; LINE #
LESS:   LHLD    EOFP            ;END OF FILE ADDRESS
        MOV     D,H
        MOV     E,L
        CALL    ADR             ;MOVE TO ADDRESS
        SHLD    EOFP            ;NEW END OF FILE ADDRESS
        MVI     C,2
        CALL    RMOV            ;OPEN UP FILE AREA
        JMP     ZERO
; GET HERE IF # OF CHARS IN OLD LINE < # OF CHARS IN
; NEW LINE
MORE:   CMA
        INR     A               ;COUNT DIFFERENCE
        MOV     D,H
        MOV     E,L
        CALL    ADR
        XCHG
        CALL    LMOV            ;DELETE EXCESS CHAR IN FILE
        MVI     M,1             ;E-O-F INDICATOR
        SHLD    EOFP            ;E-O-F ADDRESS
; GET HERE TO INSERT LINE INTO FILE AREA
ZERO:   LHLD    INSP            ;INSERT ADDRESS
        MVI     M,ASCR
        INX     H
        LXI     D,IBUF-1                ;NEW LINE ADDRESS
        MVI     C,1             ;CHECK VALUE
        CALL    LMOV            ;PLACE LINE IN FILE
        JMP     EOR
;
; THIS ROUTINE IS USED TO FIND A LN IN THE FILE AREA
; WHICH IS GREATER THAN OR EQUAL TO THE CURRENT LINE #
;
FIND:   LXI     H,ABUF+3                ;BUFFER ADDRESS
        SHLD    ADDS            ;SAVE ADDRESS
FIN1:   LHLD    BOFP            ;BEGIN FILE ADDRESS
        MOV     A,H             ;RETURN TO MONITOR IF
        ORA     L               ;  FILE IS EMPTY...
        JZ      EOR
FI1:    CALL    EO1             ;CHECK FOR END OF FILE
        XCHG
        LHLD    ADDS            ;FETCH FIND ADDRESS
        XCHG
        MVI     A,4
        CALL    ADR             ;BUMP LINE ADDRESS
        CALL    COM0            ;COMPARE LINE NUMBERS
        RC
        RZ      
FI2:    MOV     A,M
        CALL    ADR             ;NEXT LINE ADDRESS
        JMP     FI1
;
; WHEN SEARCHING THROUGH THE FILE AREA, THIS ROUTINE
; CHECKS TO SEE IF THE CURRENT ADDRESS IS THE END OF
; FILE
;
EOF:    INX     H
EO1:    MVI     A,1             ;E-O-F INDICATOR
        CMP     M
        RNZ
        JMP     EOR
;
; THIS ROUTINE IS USED TO ADD A VALUE TO AN ADDRESS
; CONTAINED IN REGISTER H,L
;
ADR:    ADD     L
        MOV     L,A
        RNC
        INR     H
        RET
;
; THIS ROUTINE WILL MOVE CHARACTER STRINGS FROM ONE
; LOCATION OF MEMORY TO ANOTHER
; CHARACTERS ARE MOVED FROM LOCATION ADDRESSED BY D,E
; TO LOCATION ADDRESSED BY H,L.  ADDITIONAL CHARACTERS
; ARE MOVED BY BUMPING POINTERS UNTIL THE CHARACTER IN
; REG C IS FETCHED.
;
LMOV:   LDAX    D               ;FETCH CHARACTER
        INX     D               ;INCREMENT FETCH ADDRESS
        CMP     C               ;TERMINATION CHARACTER
        RZ      
        MOV     M,A             ;STORE CHARACTER
        INX     H               ;INCREMENT STORE ADDRESS
        JMP     LMOV
;
; THIS ROUTINE IS SIMILAR TO ABOVE EXCEPT THAT THE
; CHARACTER ADDRESS IS DECREMENTED AFTER EACH FETCH
; AND STORE
;
RMOV:   LDAX    D               ;FETCH CHARACTER
        DCX     D               ;DECREMENT FETCH CHARACTER
        CMP     C               ;TERMINATION CHARACTER
        RZ      
        MOV     M,A             ;STORE CHARACTER
        DCX     H               ;DECREMENT STORE ADDRESS
        JMP     RMOV
;
; THIS ROUTINE IS USED TO LOAD FOUR CHARACTERS FROM
; MEMORY INTO REGISTERS
;

LODM:   MOV     B,M             ;FETCH CHARACTER
        INX     H
        MOV     C,M             ;FETCH CHARACTER 
        INX     H
        MOV     D,M             ;FETCH CHARACTER 
        INX     H
        MOV     E,M             ;FETCH CHARACTER 
        RET
;
; THIS ROUTINE STORES FOUR CHARACTERS FROM THE REGISTERS
; INTO MEMORY
;
STOM:   MOV     M,E             ;STORE CHARACTER
        DCX     H
        MOV     M,D             ;STORE CHARACTER 
        DCX     H
        MOV     M,C             ;STORE CHARACTER 
        DCX     H
        MOV     M,B             ;STORE CHARACTER 
        RET
;
; THIS ROUTINE IS USED TO COMPARE TWO CHARACTER STRINGS
; OF LENGTH 4, ON RETURN ZERO FLAG SET MEANS BOTH
; STRINGS ARE EQUAL.  CARRY FLAG =0 MEANS STRING ADDRESS
; BY D,E WAS GREATER THAN OR EQUAL TO CHARACTER STRING
; ADDRESSED BY H,L
;
COM0:   MVI     B,1             ;EQUAL COUNTER
        MVI     C,4             ;STRING LENGTH
        ORA     A               ;CLEAR CARRY
CO1:    LDAX    D               ;FETCH CHARACTER
        SBB     M               ;COMPARE CHARACTERS
        JZ      CO2
        INR     B               ;INCREMENT EQUAL COUNTER
CO2:    DCX     D
        DCX     H
        DCR     C
        JNZ     CO1
        DCR     B
        RET
;
; THIS ROUTINE IS SIMILAR TO THE ABOVE ROUTINE EXCEPT ON
; RETURN CARRY FLAG = 0 MEANS THAT CHARACTER STRING
; ADDRESSED BY D,E IS ONLY > STRING ADDRESSED BY H,L.
;
COM1:   MVI     C,4             ;STRING LENGTH
        LDAX    D               ;TCH CHARACTER
        SUI     1
        JMP     CO1+1
;
; THIS ROUTINE WILL TAKE ASCII CHARACTERS AND ADD ANY
; NECESSARY ASCII ZEROES SO THE RESULT IS A 4 CHARACTER
; ASCII VALUE
;
NORM:   CALL    LODM            ;LOAD CHARACTERS
        XRA     A               ;FETCH A ZERO
        CMP     B
        RZ      
NOR1:   CMP     E
        CNZ     STOM            ;STORE VALUES
        RNZ
        MOV     E,D             ;NORMALIZE VALUE
        MOV     D,C
        MOV     C,B
        MVI     B,'0'
        JMP     NOR1
;
; THIS ROUTINE IS USED TO LIST THE CONTENTS OF THE FILE
; AREA STARTING AT THE LINE NUMBER GIVEN IN THE COMMAND
;
LIST:   CALL    CRLF
        CALL    FIND            ;FIND STARTING LINE NUMBER
LIST0:  INX     H               ;OUTPUT LINE...
        CALL    SCRN
        CALL    CRLF
        CALL    EOF             ;CHECK FOR END OF FILE
        CALL    INK             ;CHECK FOR ?X
        JNZ     LIST0           ;LOOP IF NO ?X
        RET
;
; THIS ROUTINE IS USED TO DELETE LINES FROM THE
; FILE AREA.  THE REMAINING FILE AREA IS THEN MOVED IN
; MEMORY SO THAT THERE IS NO EXCESS SPACE.
;
DELL:   CALL    VCHK            ;CHECK FOR PARAMETER
        CALL    FIND            ;FIND LINE IN FILE AREA
        SHLD    DELP            ;SAVE DELETE POSITION
        LXI     H,ABUF+7
        MOV     A,M             ;CHECK FOR 2ND PARAMETER
        ORA     A               ;SET FLAGS
        JNZ     DEL1
        LXI     H,ABUF+3                ;USE FIRST PARAMETER
DEL1:   SHLD    ADDS            ;SAVE FIND ADDRESS
        XCHG
        LXI     H,MAXL+3
        CALL    COM0            ;COMPARE LINE NUMBERS
        LHLD    DELP            ;LOAD DELETE POSITION
        JC      NOVR
; GET HERE IF DELETION INVOLVES END OF FILE
        SHLD    EOFP            ;CHANGE E-O-F POSITION
        MVI     M,1             ;SET E-O-F INDICATOR
        XCHG
        LHLD    BOFP
        XCHG
        MVI     B,13            ;SET SCAN SWITCH
        DCX     H               ;CHECK FOR BOFP
DEL2:   MOV     A,L
        SUB     E
        MOV     A,H
        SBB     D
        MVI     A,ASCR          ;LOOK FOR CR
        JC      DEL4            ;DECREMENTED PAST BOF
        DCR     B
        DCX     H
        CMP     M               ;FIND NEW MAX LN
        JNZ     DEL2
        DCX     H
        MOV     A,L
        SUB     E
        MOV     A,H
        SBB     D
        JC      DEL5
        CMP     M               ;END OF PREVIOUS LINE
        INX     H
        INX     H
        JZ      DEL3
        INX     H
DEL3:   CALL    LODM            ;LOAD NEW MAX LN
        LXI     H,MAXL+3                ;SET ADDRESS
        CALL    STOM            ;STORE NEW MAX LN
        RET
DEL4:   CMP     B               ;CHECK SWITCH
DEL5:   XCHG
        JNZ     DEL3-1
        STA     MAXL            ;MAKE MAX LN A SMALL NUMBER
        RET
; GET HERE IF DELETION IS IN THE MIDDLE OF FILE AREA
NOVR:   CALL    FI1             ;FIND END OF DELETE AREA
        CZ      FI2             ;NEXT LINE IF THIS LN EQUAL
NOV1:   XCHG
        LHLD    DELP            ;CHAR MOVE TO POSITION
        MVI     C,1             ;MOVE TERMINATOR
        CALL    LMOV            ;COMPACT FILE AREA
        SHLD    EOFP            ;SET EOF POSITION
        MVI     M,1             ;SET EOF INDICATOR
        RET
;
; STARTING HERE IS THE SELF ASSEMBLER PROGRAM
; THIS PROGRAM ASSEMBLES PROGRAMS WHICH ARE
; IN THE FILE AREA
;
ASSM:   CALL    VCHK            ;CHECK FOR PARAMETERS
        LDA     ABUF+4          ;GET 2ND PARAMETER
        ORA     A               ;CHECK FOR PRARMETERS
        JNZ     ASM4
        LHLD    BBUF            ;FETCH 1ST PARAMETER
        SHLD    BBUF+2          ;STORE INTO 2ND PARAMETER
ASM4:   LDA     IBUF+4          ;FETCH INPUT CHARACTER
        SUI     'E'             ;RESET a IF ERRORS ONLY
        STA     AERR            ;SAVE ERROR FLAG
        XRA     A               ;GET A ZERO
        STA     NOLA            ;INITIALIZE LABEL COUNT
ASM3:   STA     PASI            ;SET PASS INDICATOR
        CALL    CRLF            ;INDICATE START OF PASS
        LHLD    BBUF            ;FETCH ORIGIN
        SHLD    ASPC            ;INITIALIZE PC
        LHLD    BOFP            ;GET START OF FILE
        SHLD    APNT
ASM1:   LHLD    APNT            ;FETCH LINE POINTER
        LXI     SP,AREA+18
        MOV     A,M             ;FETCH CHARACTER
        CPI     1               ;END OF FILE?
        JZ      EASS            ;JUMP IF END OF FILE
        XCHG
        INX     D               ;INCREMENT ADDRESS
        LXI     H,OBUF          ;BLANK START ADDRESS
        MVI     A,IBUF-5 AND 0FFH       ;BLANK END ADDRESS
        CALL    CLER            ;BLANK OUT BUFFER
        MVI     C,ASCR          ;STOP CHARACTER
        CALL    LMOV            ;MOVE LINE INTO BUFFER
        MOV     M,C             ;PLACE CR IN BUFFER
        XCHG
        SHLD    APNT            ;SAVE ADDRESS
        LDA     PASI            ;FETCH PASS INDICATOR
        ORA     A               ;SET FLAGW
        JNZ     ASM2            ;JUMP IF PASS 2
        CALL    PAS1
        JMP     ASM1
;
ASM2:   CALL    PAS2
        LXI     H,OBUF          ;OUTPUT BUFFER ADDRESS
        CALL    AOUT            ;OUTPUT LINE
        JMP     ASM1
;
; THIS ROUTINE IS USED TO OUTPUT THE LISTING FOR
; AN ASSEMBLY.  IT CHECKS THE ERROR SWITCH TO SEE IF
; ALL LINES ARE TO BE PRINTED OR JUST THOSE WITH
; ERRORS.
;
AOUT:   LDA     AERR            ;FETCH ERROR SWITCH
        ORA     A               ;SET FLAGS
        JNZ     AOU1            ;OUTPUT ALL LINES
AOU2:   LDA     OBUF            ;FETCH ERROR INDICATOR
        CPI     ' '             ;CHECK FOR AN ERROR
        RZ                      ;RETURN IF NO ERROR
AOU1:   LXI     H,OBUF          ;OUTPUT BUFFER ADDRESS
        CALL    SCRN            ;OUTPUT LINE...
        CALL    CRLF
        RET
;
; PASS 1 OF ASSEMBLER, USED TO FORM SYMBOL TABLE
;
PAS1:   CALL    ZBUF            ;CLEAR BUFFER
        STA     PASI            ;SET FOR PASS1
        LXI     H,IBUF          ;INITIALIZE LINE POINTER
        SHLD    PNTR
        MOV     A,M             ;FETCH CHARACTER
        CPI     ' '             ;CHECK FOR A BLANK
        JZ      OPC             ;JUMP IF NO LABLE
        CPI     '*'             ;CHECK FOR COMMENT
        RZ                      ;RETURN IF COMMENT
;
; PROCESS LABEL
;
        CALL    SLAB            ;GET AND CHECK LABEL
        JC      OP5             ;ERROR IN LABEL
        JZ      ERRD            ;DUPLICATE LABEL
        CALL    LCHK            ;CHECK CHARACTER AFTER LABEL
        JNZ     OP5             ;ERROR IF NO BLANK
        MVI     C,LLAB          ;LENGTH OF LABELS
        LXI     H,ABUF          ;SET BUFFER ADDRESS
MLAB:   MOV     A,M             ;FETCH NEXT CHARACTER
        STAX    D               ;STORE IN SYMBOL TABLE
        INX     D
        INX     H
        DCR     C
        JNZ     MLAB
        XCHG
        SHLD    TABA            ;SAVE TABLE ADDRESS FOR EQU
        LDA     ASPC+1          ;FETCH PC (HIGH)
        MOV     M,A
        INX     H
        LDA     ASPC            ;FETCH PC (LOW)
        MOV     M,A             ;STORE IN TABLE
        LXI     H,NOLA
        INR     M               ;INCREMENT NUMBER OF LABELS
;
; PROCESS OPCODE
;
OPC:    CALL    ZBUF            ;ZERO WORKING BUFFER
        CALL    SBLK            ;SCAN TO OPCODE
        JC      OERR            ;FOUND CARRIAGE RETURN
        CALL    ALPS            ;PLACE OPCODE IN BUFFER
        CPI     ' '             ;CHECK FOR BLANK AFTER OPCODE
        JC      OPCD            ;CR AFTER OPCODE
        JNZ     OERR            ;ERROR IF NO BLANK
        JMP     OPCD            ;CHECK OPCODE
;
; THIS ROUTINE CHECKS THE CHARACTER AFTER A LABEL
; FOR A BLANK OR COLON
;
LCHK:   LHLD    PNTR
        MOV     A,M             ;GET CHARACTER AFTER LABEL
        CPI     ' '             ;CHECK FOR BLANK
        RZ                      ;RETURN IF A BLANK
        CPI     ':'             ;CHECK FOR COLON
        RNZ
        INX     H
        SHLD    PNTR            ;SAVE POINTER
        RET
;
; PROCESS ANY PSEUDO OPS THAT NEED TO BE IN PASS 1
;
PSU1:   CALL    SBLK            ;SCAN TO OPERAND
        LDAX    D               ;FETCH VALUE
        ORA     A               ;SET FLAGS
        JZ      ORG1            ;ORG OPCODE
        JM      DAT1            ;DATA STATEMENT
        JPO     EQU1            ;EQU OPCODE
        CPI     5
        JC      RES1            ;RES OPCODE
        JNZ     EASS            ;JUMP IF END
; DO DW PSEUDO/OP
ACO1:   MVI     C,2             ;2 BYTE INSTRUCTION
        XRA     A               ;GET A ZERO
        JMP     OCN1            ;ADD VALUE TO PROGRAM COUNTER
; DO ORG PSUEDO OP
ORG1:   CALL    ASCN            ;GET OPERAND
        LDA     OBUF            ;FETCH ERROR INDICATOR
        CPI     ' '             ;CHECK FOR AN ERROR
        RNZ
        SHLD    ASPC            ;STORE NEW ORIGIN
        LDA     IBUF            ;GET FIRST CHARACTER
        CPI     ' '             ;CHECK FOR AN ERROR
        RZ                      ;NO LABEL
        JMP     EQUS            ;CHANGE LABEL VALUE
; DO EQU PSUEDO-OP
EQU1:   CALL    ASCN            ;GET OPERAND
        LDA     IBUF            ;FETCH 1ST CHARACTER
        CPI     ' '             ;CHECK FOR LABEL
        JZ      ERRM            ;MISSING LABEL
EQUS:   XCHG
        LHLD    TABA            ;SYMBOL TABLE ADDRESS
        MOV     M,D             ;STORE LABEL VALUE
        INX     H
        MOV     M,E
        RET
; DO DS PSEUDO-OP
RES1:   CALL    ASCN            ;GET OPERAND
        MOV     B,H
        MOV     C,L
        JMP     RES21           ;ADD VALUE TO PROGRAM COUNTER
;
; DO DB PSEUDO-OP
;
DAT1:   JMP     DAT2A
;
; PERFORM PASS 2 OF THE ASSEMBLER
;
PAS2:   LXI     H,OBUF+2                ;SET OUTPUT BUFFER ADDRESS
        LDA     ASPC+1          ;FETCH PC (HIGH)
        CALL    BINH+3          ;CONVERT FOR OUTPUT
        INX     H
        LDA     ASPC            ;FETCH PC(LOW)
        CALL    BINH+3          ;CONVERT FOR OUTPUT
        INX     H
        SHLD    OIND            ;SAVE OUTPUT ADDRESS
        CALL    ZBUF            ;CLEAR BUFFER
        LXI     H,IBUF          ;INITIALIZE LINE POINTER
PABL:   SHLD    PNTR            ;SAVE POINTER
        MOV     A,M             ;FETCH FIRST CHARACTER
        CPI     ' '             ;CHECK FOR LABEL
        JZ      OPC             ;GET OPCODE
        CPI     '*'             ;CHECK FOR COMMENT
        RZ                      ;RETURN IF COMMENT
        CALL    SLAB            ;SCAN OFF LABEL
        JC      ERRL            ;ERROR IN LABEL
        CALL    LCHK            ;CHECK FOR A BLANK OR COLON
        JNZ     ERRL            ;ERROR IF NOT A BLANK
        JMP     OPC
;
; PROCESS PSEUDO OPS FOR PASS2
;
PSU2:   LDAX    D
        ORA     A               ;SET FLAGS
        JZ      ORG2            ;ORG OPCODE
        JM      DAT2            ;DATA OPCODE
        JPO     EQU2            ;EQUATE PSEUDO-OP
        CPI     5
        JC      RES2            ;RES OPCODE
        JNZ     EASS            ;END OPCODE
; DO DW OPCODE
ACO2:   CALL    TYS6            ;GET VALUE
        JMP     ACO1
; DO DS PSEUDO-OP
RES2:   CALL    ASBL            ;GET OPERAND
        MOV     B,H
        MOV     C,L
        LHLD    BBUF+2          ;FETCH STORAGE COUNTER
        DAD     B               ;ADD VALUE
        SHLD    BBUF+2
RES21:  XRA     A               ;GET A ZERO
        JMP     OCN2
; DO DB PSEUDO-OP
DAT2:   CALL    TY55            ;GET OPERAND
DAT2A:  XRA     A               ;MAKE ZERO
        MVI     C,1             ;BYTE COUNT
        JMP     OCN1
;
; HANDLE EQUATES ON 2ND PASS
;
EQU2:   CALL    ASBL            ;GET OPERAND INTO HL AND
                                ;  FALL INTO NEXT ROUTINE
;
; STORE CONTENTS OF HL AS HEX ASCII AT OBUF+2
;   ON RETURN, DE HOLDS VALUE WHICH WAS IN HL.
;
BINAD:  XCHG                    ;PUT VALUE INTO DE
        LXI     H,OBUF+2                ;POINTER TO ADDR IN OBUF
        MOV     A,D             ;STORE HI BYTE
        CALL    BINH+3
        INX     H
        MOV     A,E             ;STORE LOW BYTE...
        CALL    BINH+3
        INX     H
        RET
; DO ORG PSEUDO-OP
ORG2:   CALL    ASBL            ;GET NEW ORIGIN
        LDA     OBUF            ;GET ERROR INDICATOR
        CPI     ' '             ;CHECK FOR AN ERROR
        RNZ                     ;DON?T MODIFY PC IF ERROR
        CALL    BINAD           ;STORE NEW ADDR IN OBUF
        LHLD    ASPC            ;FETCH PC
        XCHG
        SHLD    ASPC            ;STORE NEW PC
        MOV     A,L
        SUB     E               ;FORM DIFFERENCE OF ORIGINS
        MOV     E,A
        MOV     A,H
        SBB     D
        MOV     D,A
        LHLD    BBUF+2          ;FETCH STORAGE POINTER
        DAD     D               ;MODIFY
        SHLD    BBUF+2          ;SAVE
        RET
;
; PROCESS 1 BYTE INSTRUCTIONS WITHOUT OPERANDS
;
TYP1:   CALL    ASTO            ;STORE VALUE IN MEMORY
        RET
;
; PROCESS STAX AND LDAX INSTRUCTIONS
;
TYP2:   CALL    ASBL            ;FETCH OPERAND
        CNZ     ERRR            ;ILLEGAL REGISTER
        MOV     A,L             ;GET LOW ORDER OPERAND
        ORA     A               ;SET FLAGS
        JZ      TY31            ;OPERAND = 0
        CPI     2               ;OPERAND = 2
        CNZ     ERRR            ;ILLEGAL REGISTER
        JMP     TY31
;
; PROCESS PUSH, POP, INX, DCX, DAD INSTRUCTIONS
;
TYP3:   CALL    ASBL            ;FETCH OPERAND
        CNZ     ERRR            ;ILLEGAL REGISTER
        MOV     A,L             ;GET LOW ORDER OPERAND
        RRC                     ;CHECK LOW ORDER BIT
        CC      ERRR            ;ILLEGAL REGISTER
        RAL                     ;RESTORE
        CPI     8
        CNC     ERRR            ;ILLEGAL REGISTER
TY31:   RLC                     ;MULTIPLY BY 8
        RAL
        RAL
TY32:   MOV     B,A
        LDAX    D               ;FETCH OPCODE BASE
        ADD     B               ;FORM OPCODE
        CPI     118             ;CHECK FOR MOV M,M
        CZ      ERRR            ;ILLEGAL REGISTER
        JMP     TYP1
;
; PROCESS ACCUMULATOR, INR,DCR,MOV,RST INSTRUCTIONS
;
TYP4:   CALL    ASBL            ;FETCH OPERAND
        CNZ     ERRR            ;ILLEGAL REGISTER
        MOV     A,L             ;GET LOW ORDER OPERAND
        CPI     8
        CNC     ERRR            ;ILLEGAL REGISTER
        LDAX    D               ;FETCH OPCODE BASE
        CPI     64              ;CHECK FOR MOV INSTRUCTION
        JZ      TY41
        CPI     199
        MOV     A,L
        JZ      TY31            ;RST INSTRUCTION
        JM      TY32            ;ACCUMULATOR INSTRUCTION
        JMP     TY31            ;INR, DCR
; PROCESS MOV INSTRUCTION
TY41:   DAD     H               ;MULTIPLY OPERAND BY 8
        DAD     H
        DAD     H
        ADD     L               ;FORM OPCODE
        STAX    D               ;SAVE OPCODE
        CALL    MPNT
        CALL    ASCN
        CNZ     ERRR            ;INCREMENT POINTER
        MOV     A,L
        CPI     8
        CNC     ERRR            ;ILLEGAL REGISTER
        JMP     TY32
;
; PROCESS IMMEDIATE INSTRUCTIONS
; IMMEDIATE BYTE CAN BETWEEN -256 AND +255
; MVI INSTRUCTION IS A SPECIAL CASE AND CONTAINS
; 2 ARGUMENTS IN OPERAND
;
TYP5:   CPI     6               ;CHECK FOR MVI
        CZ      TY56
        CALL    ASTO            ;STORE OBJECT BYTE
TY55:   CALL    ASBL            ;GET IMMEDIATE ARGUMENT
        INR     A
        CPI     2               ;CHECK OPERAND FOR RANGE
        CNC     ERRV
        MOV     A,L
        JMP     TYP1
;
; FETCH 1ST ARG FOR MVI AND LXI INSTRUCTIONS
;
TY56:   CALL    ASBL            ;FETCH ARG
        CNZ     ERRR            ;ILLEGAL REGISTER
        MOV     A,L             ;GET LOW ORDER ARGUMENT
        CPI     8
        CNC     ERRR            ;ILLEGAL REGISTER
        DAD     H
        DAD     H
        DAD     H
        LDAX    D               ;FETCH OPCODE BASE
        ADD     L               ;FOR OPCODE
        MOV     E,A             ;SAVE OBJECT BYTE
MPNT:   LHLD    PNTR            ;FETCH POINTER
        MOV     A,M             ;FETCH CHARACTER
        CPI     ','             ;CHECK FOR COMMA
        INX     H               ;INCREMENT POINTER
        SHLD    PNTR
        JNZ     ERRS            ;SYNTAX ERROR IF NO COMMA
        MOV     A,E
        RET
;
; PROCESS 3 BYTE INSTRUCTIONS
; LXI INSTRUCTION IS A SPECIAL CASE
;
TYP6:   CPI     1               ;CHECK FOR LXI INSTRUCTION
        JNZ     TY6             ;JUMP IF NOT LXI
        CALL    TY56            ;GET REGISTER
        ANI     08H             ;CHECK FOR ILLEGAL REGISTER
        CNZ     ERRR            ;REGISTER ERROR
        MOV     A,E             ;GET OPCODE
        ANI     0F7H            ;CLEAR BIT IN ERROR
TY6:    CALL    ASTO            ;STORE OBJECT BYTE
TYS6:   CALL    ASBL            ;FETCH OPERAND
        MOV     A,L
        MOV     D,H
        CALL    ASTO            ;STORE 2ND BYTE
        MOV     A,D
        JMP     TYP1
        RET
;
; THIS ROUTINE IS USED TO STORE OBJECT CODE PRODUCED
; BY THE ASSEMBLER DURING PASS 2 INTO MEMORY
;
ASTO:   LHLD    BBUF+2          ;FETCH STORAGE ADDRESS
        MOV     M,A             ;STORE OBJECT BYTE
        INX     H               ;INCREMENT LOCATION
        SHLD    BBUF+2
        LHLD    OIND            ;FETCH OUTPUT ADDRESS
        INX     H
        CALL    BINH+3          ;CONVERT OBJECT BYTE
        SHLD    OIND
        RET
;
; GET HERE WHEN END PSEUDO-OP IS FOUND OR WHEN
; END-OF-FILE OCCURS IN SOURCE FILE.  CONTROL IS SET
; FOR EITHER PASS 2 OR ASSEMBLY TERMINATOR IF FINISHED
;
EASS:   LDA     PASI            ;FETCH PASS INDICATOR
        ORA     A               ;SET FLAGS
        JNZ     EOR             ;JUMP IF FINISHED
        MVI     A,1             ;PASS INDICATOR FOR 2ND PASS
        JMP     ASM3            ;DO 2ND PASS
;
; THIS ROUTINE SCANS THROUGH A CHARACTER STRING UNTIL
; THE FIRST NON-BLANK CHARACTER IS FOUND
;
; ON RETURN, CARRY SET INDICATES A CARRIAGE RETURN
; AS  FIRST NON-BLANK CHARACTER.
;
SBLK:   LHLD    PNTR            ;FETCH ADDRESS
SBL1:   MOV     A,M             ;FETCH CHARACTER
        CPI     ' '             ;CHECK FOR BLANK
        RNZ                     ;RETURN IF NON-BLANK
SBL2:   INX     H               ;INCREMENT
        SHLD    PNTR            ;SAVE POINTER
        JMP     SBL1
;
; THIS ROUTINE IS USED TO CHECK THE CONDITION
; CODE NMEUMONICS FOR CONDITIONAL JUMPS, CALLS,
; AND RETURNS.
;
COND:   LXI     H,ABUF+1
        SHLD    ADDS
        MVI     B,2             ;2 CHARACTERS
        CALL    COPC
        RET
;
; THE FOLLOWING IS THE OPCODE TABLE
;
OTAB:   DB      'ORG'
        DB      0
        DB      0
        DB      'EQU'
        DB      0
        DB      1
        DB      'DB'
        DB      0
        DB      0
        DB      -1 AND 0FFH
        DB      'DS'
        DB      0
        DB      0
        DB      3
        DB      'DW'
        DB      0
        DB      0
        DB      5
        DB      'END'
        DB      0
        DB      6
        DB      0
        DB      'HLT'
        DB      118
        DB      'RLC'
        DB      7
        DB      'RRC'
        DB      15
        DB      'RAL'
        DB      23
        DB      'RAR'
        DB      31
        DB      'RET'
        DB      201
        DB      'CMA'
        DB      47
        DB      'STC'
        DB      55
        DB      'DAA'
        DB      39
        DB      'CMC'
        DB      63
        DB      'EI'
        DB      0
        DB      251
        DB      'DI'
        DB      0
        DB      243
        DB      'NOP'
        DB      0
        DB      0
        DB      'XCHG'
        DB      235
        DB      'XTHL'
        DB      227
        DB      'SPHL'
        DB      249
        DB      'PCHL'
        DB      233
        DB      0
        DB      'STAX'
        DB      2
        DB      'LDAX'
        DB      10
        DB      0
        DB      'PUSH'
        DB      197
        DB      'POP'
        DB      0
        DB      193
        DB      'INX'
        DB      0
        DB      3
        DB      'DCX'
        DB      0
        DB      11
        DB      'DAD'
        DB      0
        DB      9
        DB      0
        DB      'INR'
        DB      4
        DB      'DCR'
        DB      5
        DB      'MOV'
        DB      64
        DB      'ADD'
        DB      128
        DB      'ADC'
        DB      136
        DB      'SUB'
        DB      144
        DB      'SBB'
        DB      152
        DB      'ANA'
        DB      160
        DB      'XRA'
        DB      168
        DB      'ORA'
        DB      176
        DB      'CMP'
        DB      184
        DB      'RST'
        DB      199
        DB      0
        DB      'ADI'
        DB      198
        DB      'ACI'
        DB      206
        DB      'SUI'
        DB      214
        DB      'SBI'
        DB      222
        DB      'ANI'
        DB      230
        DB      'XRI'
        DB      238
        DB      'ORI'
        DB      246
        DB      'CPI'
        DB      254
        DB      'IN'
        DB      0
        DB      219
        DB      'OUT'
        DB      211
        DB      'MVI'
        DB      6
        DB      0
        DB      'JMP'
        DB      0
        DB      195
        DB      'CALL'
        DB      205
        DB      'LXI'
        DB      0
        DB      1
        DB      'LDA'
        DB      0
        DB      58
        DB      'STA'
        DB      0
        DB      50
        DB      'SHLD'
        DB      34
        DB      'LHLD'
        DB      42
        DB      0
;       CONDITION       CODE    TABLE
        DB      'NZ'
        DB      0
        DB      'Z'
        DB      0
        DB      8
        DB      'NC'
        DB      16
        DB      'C'
        DB      0
        DB      24
        DB      'PO'
        DB      32
        DB      'PE'
        DB      40
        DB      'P'
        DB      0
        DB      48
        DB      'M'
        DB      0
        DB      56
        DB      0
;
; THIS ROUTINE IS USED TO CHECK A GIVEN OPCODE
; AGAINST THE LEGAL OPCODES IN THE OPCODE TABLE
;
COPC:   LHLD    ADDS
        LDAX    D               ;FETCH CHARACTER
        ORA     A               ;SET FLAGS
        JZ      COP1            ;JUMP IF TERMINATION CHARACTER
        MOV     C,B
        CALL    SEAR
        LDAX    D
        RZ                      ;RETURN IF A MATCH
        INX     D               ; NEXT STRING
        JMP     COPC            ;CONTINUE SEARCH
COP1:   INR     A               ;CLEAR ZERO FLAG
        INX     D               ;INCREMENT ADDRESS
        RET
;
; THIS ROUTINE CHECKS THE LEGAL OPCODES IN BOTH PASS 1
; AND PASS 2.  IN PASS 1 THE PROGRAM COUNTER IS INCRE-
; MENTED BY THE CORRECT NUMBER OF BYTES.  AN ADDRESS IS
; ALSO SET SO THAT AN INDEXED JUMP CAN BE MADE TO
; PROCESS THE OPCODE FOR PASS 2.
;
OPCD:   LXI     H,ABUF          ;GET ADDRESS
        SHLD    ADDS
        LXI     D,OTAB          ;OPCODE TABLE ADDRESS
        MVI     B,4             ;CHARACTER COUNT
        CALL    COPC            ;CHECK OPCODE
        JZ      PSEU            ;JUMP IF PSEUDO-OP
        DCR     B               ;3-CHARACTER OPCODES
        CALL    COPC
        JZ      OP1
        INR     B               ;4 CHARACTER OPCODES
        CALL    COPC
OP1:    LXI     H,TYP1          ;TYPE 1 INSTRUCTIONS
OP2:    MVI     C,1             ;1 BYTE INSTRUCTIONS
        JZ      OCNT
;
OPC2:   CALL    COPC            ;CHECK FOR STAX, LDAX
        LXI     H,TYP2
        JZ      OP2
        CALL    COPC            ;CHECK FOR PUSH,POP,INX
                                ; DCX AND DAD
        LXI     H,TYP3
        JZ      OP2
        DCR     B               ;3 CHAR OPCODES
        CALL    COPC            ;ACCUMULATOR INSTRUCTIONS,
                                ; INR, DCR, MOV, RST
        LXI     H,TYP4
        JZ      OP2
;
OPC3:   CALL    COPC            ;IMMEDIATE INSTRUCTIONS
        LXI     H,TYP5
        MVI     C,2             ;2 BYTE INSTRUCTIONS
        JZ      OCNT
        INR     B               ;4 CHARACTER OPCODES
        CALL    COPC            ;JMP, CALL, LIX, LDA, STA,
                                ; LHLD, SHLD OPCODES
        JZ      OP4
        CALL    COND            ;CONDITIONAL INSTRUCTIONS
        JNZ     OERR            ;ILLEGAL OPCODE
        ADI     192             ;ADD BASE VALUE TO RETURN
        MOV     D,A
        MVI     B,3             ;3 CHARACTER OPCODES
        LDA     ABUF            ;FETCH FIRST CHARACTER
        MOV     C,A             ;SAVE CHARACTER
        CPI     'R'             ;CONDITIONAL RETURN
        MOV     A,D
        JZ      OP1
        MOV     A,C
        INR     D               ;FORM CONDITIONAL JUMP
        INR     D
        CPI     'J'             ;CONDITIONAL JUMP
        JZ      OPAD
        CPI     'C'             ;CONDITIONAL CALL
        JNZ     OERR            ;ILLEGAL OPCODE
        INR     D               ;FORM CONDITIONAL CALL
        INR     D
OPAD:   MOV     A,D             ;GET OPCODE
OP4:    LXI     H,TYP6
OP5:    MVI     C,3             ;3 BYTE INSTRUCTION
OCNT:   STA     TEMP            ;SAVE OPCODE
;
; CHECK FOR OPCODE ONLY CONTAINING THE CORRECT NUMBER OF
; CHARACTERS.  THUS ADDQ, SAY, WOULD GIVE AN ERROR
;
        MVI     A,ABUF AND 0FFH ;LOAD BUFFER ADDRESS
        ADD     B               ;ADD LENGTH OF BUFFER
        MOV     E,A
        MVI     A,ABUF/256
        ACI     0               ;GET HIGH ORDER ADDRESS
        MOV     D,A
        LDAX    D               ;FETCH CHARACTER AFTER OPCODE
        ORA     A               ;IT SHOULD BE ZERO
        JNZ     OERR            ;OPCODE ERROR
        LDA     PASI            ;FETCH PASS INDICATOR
OCN1:   MVI     B,0
        XCHG
OCN2:   LHLD    ASPC            ;FETCH PROGRAM COUNTER
        DAD     B               ;ADD IN BYTE COUNT
        SHLD    ASPC            ;STORE PC
        ORA     A               ;WHICH PASS?
        RZ                      ;RETURN IF PASS 1
        LDA     TEMP            ;FETCH OPCODE
        XCHG
        PCHL
;
OERR:   LXI     H,ERRO          ;GET ERROR ADDRESS
        MVI     C,3             ;LEAVE 3 BYTES FOR PATCH
        JMP     OCN1-3
;
PSEU:   LXI     H,ABUF+4                ;SET BUFFER ADDRESS
        MOV     A,M             ;FETCH CHARACTER AFTER OPCODE
        ORA     A               ;SHOULD BE A ZERO
        JNZ     OERR
        LDA     PASI            ;FETCH PASS INDICATOR
        ORA     A
        JZ      PSU1
        JMP     PSU2
;
; THIS ROUTINE IS USED TO PROCESS LABELS.
; IT CHECKS TO SEE IF A LABEL IS IN THE SYMBOL TABLE
; OR NOT.  ON RETURN, Z=1 INDICATES A MATCH WAS FOUND
; AND H,L CONTAIN THE VALUE ASSOCIATED WITH THE LABEL.
; THE REGISTER NAMES A, B, C, D, E, H, L, P, AND S ARE
; PRE-DEFINED AND NEED NOT BE ENTERED BY THE USER.
; ON RETURN, C=1 INDICATES A LABEL ERROR.
;
SLAB:   CPI     'A'             ;CHECK FOR LEGAL CHARACTER
        RC
        CPI     'Z'+1           ;CHECK FOR ILLEGAL CHARACTER
        CMC
        RC                      ;RETURN IF ILLEGAL CHARACTER
        CALL    ALPS            ;PLACE SYMBOL IN BUFFER
        LXI     H,ABUF          ;SET BUFFER ADDRESS
        SHLD    ADDS            ;SAVE ADDRESS
        DCR     B               ;CHECK IF ONE CHARACTER
        JNZ     SLA1
; CHECK IF PREFEFINED REGISTER NAME
        INR     B               ;SET B=1
        LXI     D,RTAB          ;REGISTER NAME TABLE
        CALL    COPC            ;CHECK NAME OF REGISTER
        JNZ     SLA1            ;NOT A PREFEFINED REGIGTER
        MOV     L,A             ;SET VALUE (HIGH)
        MVI     H,0
        JMP     SLA2
SLA1:   LDA     NOLA            ;FETCH SYMBOL COUNT
        MOV     B,A
        LXI     D,SYMT          ;SET SYMBOL TABLE ADDRESS
        ORA     A               ;ARE THERE ANY LABELS?
        JZ      SLA3            ;JUMP IF NO LABELS
        MVI     A,LLAB          ;FETCH LENGTH OF LABEL
        STA     NCHR
        CALL    COMS            ;CHECK TABLE
        MOV     C,H             ;SWAP H AND L
        MOV     H,L
        MOV     L,C
SLA2:   STC                     ;SET CARRY
        CMC                     ;CLEAR CARRY
        RET                     ;RETURN
SLA3:   INR     A               ;CLEAR ZERO FLAG
        ORA     A               ;CLEAR CARRY
        RET
;
; PREDEFINE REGISTER VALUES IN THIS TABLE
;
RTAB:   DB      'A'
        DB      7
        DB      'B'
        DB      0
        DB      'C'
        DB      1
        DB      'D'
        DB      2
        DB      'E'
        DB      3
        DB      'H'
        DB      4
        DB      'L'
        DB      5
        DB      'M'
        DB      6
        DB      'P'
        DB      6
        DB      'S'
        DB      6
        DB      0               ;END OF TABLE INDICATOR.
;
; THIS ROUTINE SCANS THE INPUT LINE AND PLACES TH
; OPCODES AND LABELS IN THE BUFFER.  THE SCAN TERMINATES
; WHEN A CHARACTER OTHER THAN 0-9 OR A-Z IS FOUND.
;
ALPS:   MVI     B,0             ;SET COUNT
ALP1:   STAX    D               ;STORE CHARACTER IN BUFFER
        INR     B               ;INCREMENT COUNT
        MOV     A,B             ;FETCH COUNT
        CPI     11              ;MAXIMUM BUFFER SIZE
        RNC                     ;RETURN IF BUFFER FILLED
        INX     D               ;INCREMENT BUFFER
        INX     H               ;INCREMENT INPUT POINTER
        SHLD    PNTR            ;SAVE LINE POINTER
        MOV     A,M             ;FETCH CHARACTER
        CPI     '0'             ;CHECK FOR ILLEGAL CHARACTERS
        RC
        CPI     '9'+1
        JC      ALP1
        CPI     'A'
        RC
        CPI     'Z'+1
        JC      ALP1
        RET
;
; THIS ROUTINE IS USED TO SCAN THROUGH THE INPUT LINE
; TO FETCH THE VALUE OF THE OPERAND FIELD.  ON RETURN,
; THE VALUE OF THE OPERAND IS CONTAINED IN REG?S H,L
;
ASBL:   CALL    SBLK            ;GET 1ST ARGUMENT
ASCN:   LXI     H,0             ;GET A ZERO
        SHLD    OPRD            ;INITIALIZE OPERAND
        INR     H
        SHLD    OPRI-1          ;INITIALIZE OPERAND INDICATOR
NXT1:   LHLD    PNTR            ;FETCH SCAN POINTER
        DCX     H
        CALL    ZBUF            ;CLEAR BUFFER
        STA     SIGN            ;ZERO SIGN INDICATOR
NXT2:   INX     H               ;INCREMENT POINTER
        MOV     A,M             ;FETCH NEXT CHARACTER
        CPI     ' '+1
        JC      SEND            ;JUMP IF CR OR BLANK
        CPI     ','             ;FIELD SEPARATOR
        JZ      SEND
; CHECK FOR OPERATOR
        CPI     '+'             ;CHECK FOR PLUS
        JZ      ASC1
        CPI     '-'             ;CHECK FOR MINUS
        JNZ     ASC2
        STA     SIGN
ASC1:   LDA     OPRI            ;FETCH OPERAND INDICATOR
        CPI     2               ;CHECK FOR 2 OPERATORS
        JZ      ERRS            ;SYNTAX ERROR
        MVI     A,2
        STA     OPRI            ;SET INDICATOR
        JMP     NXT2
; CHECK FOR OPERANDS
ASC2:   MOV     C,A             ;SAVE CHARACTER
        LDA     OPRI            ;GET INDICATOR
        ORA     A               ;CHECK FOR 2 OPERANDS
        JZ      ERRS            ;SYNTAX ERROR
        MOV     A,C
        CPI     '$'             ;LC EXPRESSION
        JNZ     ASC3
        INX     H               ;INCREMENT POINTER
        SHLD    PNTR            ;SAVE POINTER
        LHLD    ASPC            ;FETCH LOCATION COUNTER
        JMP     AVAL
;CHECK FOR ASCII CHARACTERS
ASC3:   CPI     27H             ;CHECK FOR SINGLE QUOTE
        JNZ     ASC5            ;JUMP IF NOT QUOTE
        LXI     D,0             ;GET A ZERO
        MVI     C,3             ;CHARACTER COUNT
ASC4:   INX     H               ;BUMP POINTER
        SHLD    PNTR            ;SAVE
        MOV     A,M             ;FETCH NEXT CHARACTER
        CPI     ASCR            ;IS IT A CARRIAGE RETURN?
        JZ      ERAR            ;ARGUMENT ERROR
        CPI     27H             ;IS IT A QUOTE?
        JNZ     SSTR
        INX     H               ;INCREMENT POINTER
        SHLD    PNTR            ;SAVE
        MOV     A,M             ;FETCH NEXT CHAR
        CPI     27H             ;CHECK FOR 2 QUOTES IN A ROW
        JNZ     AVAL+1          ;TERMINAL QUOTE
SSTR:   DCR     C               ;CHECK COUNT
        JZ      ERAR            ;TOO MANY CHARACTERS
        MOV     D,E
        MOV     E,A             ;SET CHARACTER IN BUFFER
        JMP     ASC4
ASC5:   CPI     '0'             ;CHECK FOR NUMERIC
        JC      ERAR            ;ILLEGAL CHARACTER
        CPI     '9'+1
        JNC     ALAB
        CALL    NUMS            ;GET NUMERIC VALUE
        JC      ERAR            ;ARGUMENT ERROR
AVAL:   XCHG
        LHLD    OPRD            ;FETCH OPERAND
        XRA     A               ;GET A ZERO
        STA     OPRI            ;STOR IN OPERAND INDICATOR
        LDA     SIGN            ;GET SIGN INDICATOR
        ORA     A               ;SET FLAGS
        JNZ     ASUB
        DAD     D               ;FORM RESULT
ASC7:   SHLD    OPRD            ;SAVE RESULT
        JMP     NXT1
ASUB:   MOV     A,L
        SUB     E
        MOV     L,A
        MOV     A,H
        SBB     D
        MOV     H,A
        JMP     ASC7
ALAB:   CALL    SLAB
        JZ      AVAL
        JC      ERAR            ;ILLEGAL SYMBOL
        JMP     ERRU            ;UNDEFINED SYMBOL
;
; GET HERE WHEN TERMINATING CHARACTER IS FOUND.
; CHECK FOR LEADING FIELD SEPARATOR
;
SEND:   LDA     OPRI            ;FETCH OPERAND INDICATOR
        ORA     A               ;SET FLAGS
        JNZ     ERRS            ;SYNTAX ERROR
        LHLD    OPRD
SEN1:   MOV     A,H             ;GET HIGH ORDER BYTE
        LXI     D,TEMP          ;GET ADDRESS
        ORA     A               ;SET FLAGS
        RET
;
; GET A NUMERIC VALUE WHICH IS EITHER HEXADECIMAL OR
; DECIMAL.  ON RETURN, CARRY SET INDICATES AN ERROR.
;
NUMS:   CALL    ALPS            ;GET NUMERIC
        DCX     D
        LDAX    D               ;GET LAST CHARACTER
        LXI     B,ABUF          ;SET BUFFER ADDRESS
        CPI     'H'             ;IS IT HEXADECIMAL?
        JZ      NUM2
        CPI     'D'             ;IS IT DECIMAL
        JNZ     NUM1
        XRA     A               ;GET A ZERO
        STAX    D               ;CLEAR D FROM BUFFER
NUM1:   CALL    ADEC            ;CONVERT DECIMAL VALUE
        RET
NUM2:   XRA     A               ;GET A ZERO
        STAX    D               ;CLEAR H FROM BUFFER
        CALL    AHEX
        RET
;
; PROCESS REGISTER ERROR
;
ERRR:   MVI     A,'R'           ;GET INDICATOR
        LXI     H,0             ;GET A ZERO
        STA     OBUF            ;SET IN OUTPUT BUFFER
        RET
;
; PROCESS SYNTAX ERROR
;
ERRS:   MVI     A,'S'           ;GET INDICATOR
        STA     OBUF            ;STORE IN OUTPUT BUFFER
        LXI     H,0
        JMP     SEN1
;
; PROCESS UNDEFINED SYMBOL ERROR
;
ERRU:   MVI     A,'U'           ;GET INDICATOR
        JMP     ERRS+2
;
; PROCESS VALUE ERROR
;
ERRV:   MVI     A,'V'           ;GET INDICATOR
        JMP     ERRR+2
;
; PROCESS MISSING LABEL ERROR
;
ERRM:   MVI     A,'M'           ;GET INDICATOR
        STA     OBUF            ;STORE IN OUTPUT BUFFER
        CALL    AOU1            ;DISPLAY ERROR
        RET
;
;PROCESS ARGUMENT ERROR
;
ERAR:   MVI     A,'A'           ;GET INDICATOR
        JMP     ERRS+2
;
; PROCESS OPCODE ERROR
; STORE 3 BYTES OF ZERO IN OBJECT CODE TO PROVIDE
; FOR A PATCH
;
ERRO:   MVI     A,'O'           ;GET INDICATOR
        STA     OBUF            ;STORE IN OUTPUT BUFFER
        LDA     PASI            ;FETCH PASS INDICATOR
        ORA     A               ;WHICH PASS
        RZ                      ;RETURN IF PASS 1
        MVI     C,3             ;NEED 3 BYTES
ERO1:   XRA     A               ;GET A ZERO
        CALL    ASTO            ;PUT IN LISTING AND MEMORY
        DCR     C
        JNZ     ERO1
        RET
;
; PROCESS LABEL ERROR
;
ERRL:   MVI     A,'L'           ;GET INDICATOR
        JMP     ERRO+2
;
; PROCESS DUPLICATE LABEL ERROR
;
ERRD:   MVI     A,'D'           ;GET INDICATOR
        STA     OBUF
        CALL    AOUT
        JMP     OPC
;
; THIS ROUTINE SETS OR CLEARS BREAKPOINTS
;
BREAK:  LDA     ABUF            ;CHECK FOR AN ARG
        ORA     A
        JZ      CLRB            ;IF NO ARGUMENT, GO CLEAR BREAKPOINT
        MVI     D, NBR          ;ELSE GET NUMBER OF BREAKPOINTS
        LXI     H,BRT           ;AND ADDR OF TABLE
B1:     MOV     A,M             ;GET HI BYTE OF ENTRY
        INX     H
        MOV     B,M             ;GET LOW BYTE OF ENTRY
        ORA     B               ;CHECK FOR EMPTY ENTRY
        JZ      B2              ;BRANCH IF EMPTY
        INX     H               ;ELSE GO ON TO NEXT ENTRY
        INX     H
        DCR     D               ;BUMP COUNT
        JNZ     B1              ;AND TRY AGAIN
        JMP     WHAT            ;OOPS NO ROOM
B2:     DCX     H
        XCHG
        LHLD    BBUF            ;GET ADDRESS
        XCHG                    ;IN D,E
        MOV     A,D             ;CHECK FOR ADDR > 11D
        ORA     A
        JNZ     B3
        MOV     A,E
        CPI     11
            JC  WHAT            ;OOPS, TOO LOW
B3:     MOV     M,D             ;SAVE ADDRESS
        INX     H
        MOV     M,E
        INX     H
        LDAX    D               ;PICK UP INSTRUCTION
        MOV     M,A             ;SAVE IT
        MVI     A,0CFH          ;RST 1 INSTRUCTION
        STAX    D
        MVI     A,0C3H          ;SET UP LO MEMORY
        STA     8               ;WITH A JUMP TO BREAKPOINT
        LXI     H,BRKP
        SHLD    9
        RET                     ;THEN RETURN
;
; THIS ROUTINE CLEARS ALL BREAKPOINTS
;
CLRB:   LXI     H,BRT           ;GET TABLE ADDRESS
        MVI     B,NBR           ;GET NUMBER OF BREAKPOINTS
CLBL:   XRA     A               ;GET A ZERO
        MOV     D,M             ;GET HI-BYTE OF ENTRY
        MOV     M,A
        INX     H
        MOV     E,M             ;GET LO-BYTE OF ENTRY
        MOV     M,A
        INX     H
        MOV     B,M             ;GET INST BYTE
        INX     H
        MOV     A,D             ;WAS THIS A NULL ENTRY
        ORA     E
        JZ      CL2             ;BRANCH IF IT WAS
        MOV     A,B
        STAX    D               ;ELSE PLUG INST BACK IN
CL2:    DCR     B               ;BUMP COUNT
        JNZ     CLBL            ;GO DO NEXT ONE
        RET
;
; COME HERE WHEN WE HIT A BREAKPOINT
;
BRKP:   SHLD    HOLD+8          ;SAVE H,L
        POP     H               ;GET PC
        DCX     H               ;ADJUST IT
        SHLD    HOLD+10         ;SAVE IT
        PUSH    PSW             ;SAVE FLAGS
        POP     H               ;GET THEM INTO H,L
        SHLD    HOLD            ;NOW STORE THEM FOR USER
        LXI     H,0
        DAD     SP              ;GET STACK POINTER
        LXI     SP,HOLD+8       ;SET STACK POINTER AGAIN
        PUSH    H               ;SAVE OLD SP
        PUSH    D               ;SAVE D,E
        PUSH    B               ;SAVE B,C
        CMA                     ;COMPLEMENT ACCUMULATOR
        OUT     0FFH            ;DISPLAY IT IN LIGHTS
        LXI     SP,AREA+18      ;SET SP AGAIN
        LHLD    HOLD+10         ;GET PC
        XCHG                    ;INTO D,E
        LXI     H,BRT           ;GET ADDR OF TABLE
        MVI     B,NBR           ;AND NUMBER OF ENTRIES
BL1:    MOV     A,M             ;GET AN ENTRY FROM THE TABLE
        INX     H
        CMP     D               ;DOES IT MATCH?
        JNZ     BL2             ;BRANCH IF NOT
        MOV     A,M             ;ELSE GET NEXT BYTE
        CMP     E               ;CHECK IT
        JZ      BL3             ;IT MATCHES!
BL2:    INX     H               ;BUMP AROUND THIS ENTRY
        INX     H
        DCR     B               ;BUMP COUNT
        JZ      WHAT            ;NOT IN OUR TABLE
        JMP     BL1
;
BL3:    INX     H
        MOV     A,M             ;GET INSTR BYTE
        STAX    D               ;PUT IT BACK
        XRA     A               ;CLEAR ENTRY IN TABLE
        DCX     H
        MOV     M,A
        DCX     H
        MOV     M,A
        CALL    CRLF            ;RESTORE THE CARRIAGE
        LDA     HOLD+11         ;GET HI-BYTE OF PC
        CALL    HOUT            ;TYPE IT
        LDA     HOLD+10         ;GET LO-BYTE OF PC
        CALL    HOUT            ;TYPE IT
        LXI     H,BMES          ;TELL USER WHAT IT IS
        CALL    SCRN
        JMP     EOR             ;GO BACK TO COMMAND LEVEL
;
BMES:   DB      ' BREAK',13
;
; THIS ROUTINE PROCEEDS FROM A BREAKPOINT
;
PROC:   LDA     ABUF            ;CHECK FOR ARG
        ORA     A
        JZ      P1              ;JUMP IF NO ARG
        LHLD    BBUF            ;ELSE GET ARG
        SHLD    HOLD+10         ;PLUG IT INTO PC SLOT
P1:     LXI     SP,HOLD         ;SET SP TO POINT AT REG?S
        POP     PSW             ;RESTORE PSW
        POP     B               ;RESTORE B,C
        POP     D               ;RESTORE D,E
        POP     H               ;GET OLD SP
        SPHL                    ;RESTORE IT
        LHLD    HOLD+10         ;GET PC
        PUSH    H               ;PUT IT ON STACK
        LHLD    HOLD+8          ;RESTORE H,L
        RET                     ;AND PROCEED
;
; SYSTEM RAM
;
        ORG     1000H
;
; DEFINE BREAKPOINT REGION
;
NBR     EQU     8               ;NUMBER OF BREAKPOINTS
HOLD:   DS      12              ;REGISTER HOLD AREA
BRT:    DS      3*NBR           ;BREAKPOINT TABLE
;
; FILE AREA PARAMETERS
;
MAXFIL  EQU     6
NMLEN   EQU     5
FELEN   EQU     NMLEN+8
FILE0:  DS      NMLEN
BOFP:   DS      2
EOFP:   DS      2
MAXL:   DS      4
FILTB:  DS      (MAXFIL-1)*FELEN
INSP:   DS      2
DELP    EQU     INSP
ASCR    EQU     13
HCON:   DS      2
ADDS    EQU     HCON
FBUF:   DS      NMLEN
FREAD:  DS      2
FEF:    DS      1
FOCNT   EQU     FEF
ABUF:   DS      12
BBUF:   DS      4
SCNT:   DS      1
DCNT:   DS      1
NCOM    EQU     11
TABA:   DS      2
ASPC:   DS      2
PASI:   DS      1
NCHR:   DS      1
PNTR:   DS      2
NOLA:   DS      1
SIGN:   DS      1
OPRD:   DS      2
OPRI:   DS      1
TEMP:   DS      1
APNT    EQU     INSP
AERR    EQU     SCNT
OIND:   DS      2
LLAB    EQU     5
AREA:   DS      18
OBUF:   DS      16
        DS      5
IBUF:   DS      83
SWCH    EQU     0FFH
SYMT    EQU     $
        END

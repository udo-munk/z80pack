;
; DISASSEMBLY OF ALS-8 SYSTEM
; BY BARRY A. WATZMAN
;
; <><><><><><><><><><><><><><><><><><><><><><><><><><><><><
; MODIFIED TO PROPER INTEL SYNTAX AND FIXED TYPOS.
; MODIFIED TERMINAL IO FOR MITS SIO REV. 1, SEE *UM*.
;
; ENTRY POINTS:
;     E024     USE WHEN POWER IS FIRST APPLIED OR AFTER
;              A MAJOR PROGRAM CRASH.
;
;     E000     USE AFTER A MINOR CRASH TO REINITIALIZE
;              THE RAM I/O DRIVERS. FOLLOWING THIS THE
;              FILE, IODR, CUST, AND SYMBOL TABLE CAN
;              BE EXAMINED FOR POSSIBLE AFFECTS.
;
;     E060     USE TO RETURN TO THE ALS-8 AFTER AN
;              OPERATIONAL PROGRAM HAS GONE INTO AN
;              ENDLESS LOOP OR HALTED.
;
; UDO MUNK, AUGUST 2018
; <><><><><><><><><><><><><><><><><><><><><><><><><><><><><
;
DATA    EQU     0D000H          ;LOCATION OF DATA BLOCK
CODE    EQU     0E000H          ;LOCATION OF ACTUAL CODE
VDM1    EQU     0CC00H          ;VDM-1 SCREEN MEMORY LOCATION
;
        ORG     CODE
ALS8:   NOP
        JMP     LE03B
;
;PART OF THE CODE TO DELETE CUSTOM COMMANDS
;
LE004:  CALL    LE50C           ;SET UP ABUF, H,L FOR DELETE
        JMP     LE4B7           ;USE SYMLD CODE TO DELETE COMD
;
;THIS ROUTINE FETCHES DIGITS FROM THE BUFFER ADDRESSED
;BY B,C AND CONVERTS THE ASCII DECIMAL DIGITS INTO
;BINARY.  UP TO A 16-BIT VALUE CAN BE CONVERTED.  THE
;SCAN STOPS WHEN A BINARY ZERO IS FOUND IN THE BUFFER.
;
ADEC:   LXI     H,0             ;GET A 16 BIT ZERO
ADE1:   LDAX    B               ;FETCH ASCII DIGIT
        ORA     A               ;SET ZERO FLAG
        RZ                      ;RETURN IFF FINISHED
        MOV     D,H             ;DUPLICATE CURRENT VALUE
        MOV     E,L             ;FROM H,L INTO D,E
        DAD     H               ;CURRENT VALUE TIMES TWO
        DAD     H               ;AGAIN - NOW 4X ORIG. VALUE
        DAD     D               ;ADD IN ORIG VALUE - NOW 5X
        DAD     H               ;TIMES TWO - NOW 10X
        SUI     30H             ;CONVERT FETCHED DGT TO BINARY
        CPI     10              ;CHECK FOR LEGAL VALUE
        CMC                     ;SET UP CARRY FLAG
        RC                      ;RETURN W/CARRY SET IF ERROR
        MOV     E,A             ;ELSE MOVE BINARY VALUE TO E
        MVI     D,0             ;AND ZERO TO D
        DAD     D               ;AND ADD TO 10X ORIG. NUMBER
        INX     B               ;NOW POINT TO NEXT DIGIT
        JMP     ADE1            ;AND PROCESS IT
;
;LOWEST LEVEL ENTRY POINT -- INITIALIZES EVERYTHING
;
INITA:  LXI     H,FILE0         ;POINT TO START OF DATA AREA
        XRA     A               ;GET A ZERO
        MOV     C,A             ;MOVE IT TO C
INIT2:  MOV     M,A             ;STASH THE ZERO
        DCR     C               ;DECREMENT C
        INX     H               ;POINT TO NEXT LOCATION
        JNZ     INIT2           ;IF HAVEN'T DONE 256 LOCATIONS
        STA     SYSYM           ;ANOTHER LOCATION TO ZERO
        STA     SMODE           ;AND ANOTHER
        STA     CUCOM           ;AND STILL ANOTHER
        JMP     ALS8            ;NOW START UP THE SYSTEM
;
;WARM RESTART - RESTORES I/O DRIVERS AND DOES PARTIAL
;INITIALIZATION
;
LE03B:  MVI     C,(CRLF-IONME) AND 0FFH
        LXI     SP,SMODE        ;SET STACK POINTER
        LXI     D,IONME         ;POINT TO IO CODE SOURCE
        LXI     H,SYSIO         ;POINT TO DEST IN DATA AREA
LE046:  LDAX    D               ;GET A BYTE
        MOV     M,A             ;PUT IT IN DATA AREA
        INX     D               ;SOURCE POINTER
        INX     H               ;DEST POINTER
        DCR     C               ;NO OF BYTES LEFT TO BE MOVED
        JNZ     LE046           ;IF NOT DONE
        LXI     H,ALS8          ;POINT TO START CODE
        SHLD    SYMADD          ;INITIAL SYMBOL TABLE ADDR
        MVI     A,(IBUF+81) AND 0FFH
        STA     TERMW           ;INITIAL TERMINAL WIDTH
        XRA     A               ;GET A ZERO
        STA     SMODE           ;SAVE AS SMODE
        STA     CHRR            ;AND CHRR
;
;WARM RESTART WITH NO INITIALIZATION
;PRINTS "READY" AND ENTERS MAIN PROCESSING LOOP
;
EORMS:  LXI     H,SYSIN         ;POINT TO CUR DRIVER ADDRS
        CALL    LE15A           ;SET I/O JUMPS TO CUR DRIVERS
        CALL    CRLF            ;PRINT A CR/LF ON TERMINAL
        LXI     H,RDYMG         ;POINT TO READY MESSAGE
        CALL    SCRN            ;PRINT "READY"
        JMP     LE0CB           ;ANOTHER CRLF & ENTER MAIN LOOP
;
;THIS IS THE MAIN ALS-8 COMMAND LOOP
;
LE072:  LXI     H,0             ;GET A ZERO TO H,L
        SHLD    SWCH1           ;TO SWITCH 1
LE078:  LXI     SP,SMODE        ;SET STACK POINTER
        CALL    READ            ;GET A COMND LINE IN IBUF
        INX     H               ;POINT TO 1ST CHAR
        MOV     A,M             ;GET IT
        CPI     '9'+1           ;COMPARE TO 9
        JC      LINE            ;NEW LINE IF 1ST CHAR NUMERIC
        CALL    VALC            ;EXTRACT ASCII & BINARY ARGS
        MVI     B,4             ;LENGTH OF ALS8 COMMANDS
        LXI     D,CTAB          ;POINT AT COMMAND TABLE
        CALL    COMS            ;SEE IF COMMAND IS IN TABLE
        PUSH    H               ;SAVE H,L PNTR INTO LINE
        JZ      LE0AC           ;MATCH WAS FOUND
        POP     H               ;RESTORE POINTER TO LINE
        INX     D               ;POINT TO 2ND PART OF TABLE
        CALL    COMS            ;SEARCH 2ND PART
        JZ      LE0A5           ;IF MATCH FOUND
        LXI     D,CUCOM         ;NO MATCH, TRY CUSTOM TABLE
        CALL    LE240           ;SEARCH CUSTOM TABLE
        JNZ     WHAT            ;ERROR MSG IF NO MATCH FOUND
LE0A5:  PUSH    H               ;SAVE H,L
        CALL    LE31E           ;CONVERT ABUF CNTS TO BINARY
        JC      WHAT            ;IF ABUF NOT LEGAL NO.
LE0AC:  LDA     IOSWC           ;1 COMND DRVR SWTCH DELAY FLAG
        ORA     A               ;SET FLAGS
        CNZ     LE157           ;TIME TO SWITCH DRIVERS
        POP     H               ;RESTORE POINTER TO COMD LINE
        CALL    LE0DD           ;EXECUTE THE COMMAND
EOR:    LDA     SWCH1           ;GET I/O RESTORE SWITCH
        ORA     A               ;SET FLAGS
        JNZ     EORMS           ;RESTORE JMPS & PRT "READY"
        LDA     SWCH2           ;GET DRIVER HOLD REQ.
        ORA     A               ;SET FLAGS
        JNZ     EORNS           ;HOLD CURRENT DRVR, DO CRLF
        LXI     H,SYSIN         ;POINT TO SYSTEM DRIVERS
        CALL    LE15A           ;PUT IN JUMPS & DO CRLF
LE0CB:  CALL    CRLF            ;PRINT CRLF
        JMP     LE072           ;DO ANOTHER ALS8 COMMAND
;
;END OF MAIN PROCESSING LOOP
;
;FOLLOWING RETURN POINT DOES A CR/LF AND LEAVES
;ALL DRIVERS AND SWITCHES INTACT
;
EORNS:  CALL    CRLF            ;PRINT A CRLF
        JMP     LE078           ;RET LEAVING DRIVERS INTACT
;
;SIGN-ON AND COMMAND MODE MESSAGE
;
RDYMG:  DB      'READY',0DH
;
;HERE IS WHERE WE DISPATCH TO COMMANDS
;NOTE THAT SINCE THIS IS A CALLED ROUTINE, SO
;IN EFFECT IS THE ROUTINE TRANSFERED TO
;
LE0DD:  PCHL                    ;GO TO IT
;
;THE IODR COMMAND AND IT'S ZILLION FORMS
;
IODR:   MVI     C,MAXFIL        ;MAX NUMBER OF DRIVER NAMES
        LXI     H,IOFLE         ;POINT TO DRIVER FILE
        LDA     FBUF            ;NAME IN SLASHES AFTR COMMAND ?
        ORA     A               ;SET FLAGS
        JZ      LE151           ;NO - PRT LIST OF DRIVERS
        XCHG                    ;YES - TABLE PNTR TO D,E
        CALL    LE68D           ;SEARCH DIRECTORY FOR NAME
        JNZ     LE0FE           ;NAME FOUND
        CALL    VCHK            ;NOT FOUND - CHECK FOR PARMS
        LDA     FEF             ;GET FREE ENTRY FOUND FLAG
        ORA     A               ;SET FLAGS
        JZ      LE5CC           ;NO ENTRIES LEFT - PRT "FULL"
        LHLD    FREAD           ;GET ADDR OF FREE ENTRY
LE0FE:  SHLD    FREAD           ;FREAD HAS ADDR OF ENTRY
        CALL    ROOM            ;PUT FILE NAME INTO DIRECTORY
        LXI     H,NMLEN         ;NAME LENGTH
        DAD     D               ;H,L POINT JUST AFTER NAME
        LDA     ABUF            ;SEE IF ADDR SPECIFIED
        ORA     A               ;SET FLAGS
        JNZ     LE119           ;IF ADDR WAS SPECIFIED
        SHLD    OPRD            ;NO ADDR, SET UP FOR SWTCH
        INR     A               ;MAKE ACCUM. NON-ZERO
        STA     IOSWC           ;SET I/O SWITCH FLAG
        JMP     LE14C           ;PRINT THE SELECTED ENTRY
LE119:  XCHG                    ;SAVE I/O ADDRS IN D,E  
        LHLD    BBUF            ;GET 1ST ADDR SPECIFIED
        MOV     A,L             ;L TO A
        ORA     H               ;SEE IF ADDRS IS 0
        LDA     ABUF+7          ;POINT TO 2ND ASCII PARM.
        JNZ     LE130           ;IF 1ST ADDR NOT ZERO
        ORA     A               ;SEE IF 2ND ADDR SPECIFIED
        JNZ     LE12D           ;IF 2ND ARGUMENT NOT OMITTED
        STAX    D               ;STORE 0 FOR INPUT, DELT ENTRY
        INX     D               ;FOR 2ND BYTE
        STAX    D               ;ZERO IT TOO
BITBKT: RET                     ;DONE
;
;1ST ARG. WAS 0, 2ND WAS PRESENT
;
LE12D:  LHLD    SYSIN           ;GET STD SYSIN DRIVER
LE130:  XCHG                    ;TO D,E
        MOV     M,E             ;STORING IT AS INPUT DRIVER
        INX     H               ;2ND BYTE
        MOV     M,D             ;STORE IT
        INX     H               ;POINT TO OUTPUT DRIVER
        XCHG                    ;OUTPUT DRIVER ADDR TO D,E
        ORA     A               ;SET FLAGS FOR 2ND ARG OMITTED
        LXI     H,BITBKT        ;POINT TO THE BIT BUCKET
        JZ      LE148           ;IF 2ND ARG OMITTED
        LHLD    BBUF+2          ;GET 2ND ARG
        MOV     A,L             ;MOVE L TO A
        ORA     H               ;SEE IF ARG = 0
        JNZ     LE148           ;JMP IF NOT ZERO
        LHLD    SYSOT           ;ELSE SET TO USE SYSOT
LE148:  XCHG                    ;OUTPUT ADDR TO D,E
        MOV     M,E             ;MOVING IT TO OUTPUT TABLE
        INX     H               ;2ND HALF
        MOV     M,D             ;DONE
LE14C:  MVI     C,1             ;SET UP TO PRINT ENTRY
        LHLD    FREAD           ;GET IT'S ADDRESS
LE151:  CALL    LE62A           ;PRINT IT
        JMP     EORNS           ;DONE WITH IODR
;
;SET UP I/O JUMPS TO I/O DRIVER SET WHOSE ADDRESSES
;ARE POINTED TO BY OPRD (SET ABOVE)
;
LE157:  LHLD    OPRD            ;GET ADDRESS OF I/O ADDRESSAS
;
;STORE I/O JUMPS TO ADDRESSES POINTED TO BY CONTENTS OF
;H,L -- SYSIO, OR IF ENTERD IN LINE, ONE OF THE OTHER
;I/O DRIVERS
;
LE15A:  CALL    LODM            ;GET THE 4 BYTES OF 2 ADDRESSES
        LXI     H,IN8           ;POINT TO THE I/O JUMPS
        MVI     M,0C3H          ;STORE JUMP INST.
        INX     H               ;POINT TO NEXT BYTE
        MOV     M,B             ;STORE IT
        INX     H               ;2ND BYTE
        MOV     M,C             ;STORE IT
        INX     H               ;NOW 1ST BYTE FOR OUTPUT
        MVI     M,0C3H          ;STORE JUMP INST
        INX     H               ;POINT TO ADDR 1ST BYTE
        MOV     M,D             ;STORE IT
        INX     H               ;POINT 2ND BYTE
        MOV     M,E             ;STORE IT
        XRA     A               ;A ZERO
        STA     IOSWC           ;RESET SWITCH TO NO-SWITCH MODE
        RET                     ;DONE
;
;THE FOLLOWING ROUTINE READS A COMMAND LINE FROM THE
;TERMINAL AND PUTS IT IN THE OUTPUT BUFFER
;
READ:   LXI     H,IBUF          ;GET INPUT BUFFER ADDR
        SHLD    ADDS            ;SAVE ADDRESS
        MVI     E,2             ;INIT. CHAR COUNT
NEXT:   CALL    IN8             ;READ A LINE
        CPI     'X'-40H         ;CHECK FOR CNTL X
        JNZ     CR              ;CHECK FOR C/R IF NOT CNTL X
        CALL    CRLF            ;ELSE DO CRLF
        JMP     READ            ;AND START OVER
CR:     CPI     0DH             ;IS IT A C/R ?
        JNZ     DEL             ;IF NOT, CHECK FOR CHAR DELETE
        MOV     A,L             ;GET LOW ORDER BYTE OF ADDR
        CPI     IBUF AND 0FFH   ;SEE IF CR IS ONLY CHAR ON LINE
        JZ      READ            ;GET ANOTHER LINE IF SO
        MOV     M,B             ;PUT CHAR IN THE LINE
        INX     H               ;POINT TO NEXT POSITION
        MVI     M,1             ;PUT END OF LINE INDICATOR
        INX     H               ;POINT 1 AFTER THE LINE
        CALL    LF3DF           ;GET TERMW+1 IN A
        INR     A               ;INCREMENT IT FOR EOF MARK
        CALL    CLER            ;CLEAR REST OF THE BUFFER
        LXI     H,IBUF-1        ;POINT TO CHAR COUNT
        MOV     A,E             ;PUT IT BEFORE THE LINE
        STA     CCNT            ;AND SAVE AS CHAR COUNT
        RET                     ;DONE
DEL:    CPI     7FH             ;IS THE CHAR A DELETE ?
        JNZ     CHAR            ;IF NOT, PUT IT IN LINE
        MVI     A,IBUF AND 0FFH ;ELSE GET LOW-ORDER ADDR
        CMP     L               ;MAKE SURE NOT AT IBUF
        JZ      NEXT            ;IF SO, IGNORE DELETE
        DCX     H               ;ELSE DCR BUFFER POINTER
        DCR     E               ;AND CHAR COUNT
BSPA:   MVI     B,5FH           ;GET BACKSPACE CHAR
        CALL    OUT8            ;MAKE TERMINAL BKSPACE
        JMP     NEXT            ;THEN GET NEXT CHAR
CHAR:   CALL    OUT8            ;ECHO THE CHAR
        CPI     ' '             ;MAKE SURE IT'S PRINTABLE
        JC      NEXT            ;SKIP IT IF IT'S CNTL CHAR
        MOV     M,A             ;ELSE PUT IT IN THE BUFFER
        LDA     TERMW           ;GET TERMINAL WIDTH
        CMP     L               ;SEE IF WE'RE THERE
        JZ      BSPA            ;BACKSPACE IF BUFFER OVERFLOW
        INX     H               ;ELSE INCREMENT THE BUFFER PNTR
        INR     E               ;AND THE CHAR COUNT
        JMP     NEXT            ;THEN DO NEXT CHAR
;
;THIS ROUTINE IS USED TO BLANK OUT A PORTION OF MEMORY
;SINCE ONLY L IS TESTED, AREA MUST BE < 256 BYTES
;
CLER:   CMP     L               ;SEE IF AT END OF AREA TO BLANK
        RZ                      ;DONE IF SO
        MVI     M,' '           ;ELSE GET A SPACE
        INX     H               ;INCREMENT THE MEMORY POINTER
        JMP     CLER            ;AND TEST NEW ADDRESS
;
;HERE ARE THE BUILT IN I/O DRIVERS AND THEIR NAME
;THIS CODE IS MOVED INTO THE DATA PORTION DURING
;PROGRAM INITIALIZATION.
;PART OF THE ENTER COMMAND IS ALSO MOVED, FOR NO REASON.
;
IONME:  DB      'SYSIO'         ;STANDARD DRIVER NAME
        DW      INDR            ;INPUT ROUTINE ADDR
        DW      OUTDR           ;OUTPUT ROUTINE ADDR
INP8:   CALL    STAT            ;BECOMES INDR: AFTER THE MOVE
;       JZ      INDR            ;*UM*
        JNZ     INDR            ;*UM*
        IN      UDATA
        ANI     7FH
        MOV     B,A
        RET
        IN      USTA            ;THIS BECOMES STAT:
        ANI     DAV
        RET
OUTP8:  CALL    STAT
        JZ      NOCHR
        IN      UDATA           ;NOTE THAT BEFORE EA. OUTPUT
        ANI     7FH
        CPI     ESC             ;ALS8 TESTS FOR AN ESC. CHAR
        JZ      EORMS           ;AND RETNS TO CMD MODE IF FOUND
        IN      USTA            ;BECOMES NOCHR:
        ANI     TBE
;       JZ      NOCHR
        JNZ     NOCHR           ;*UM*
        MOV     A,B
        OUT     UDATA
        RET
;
;THE ENTER COMMAND.
;
ENTR:   CALL    VCHK            ;MAKE SURE PARAMETERS GIVEN
        CALL    ENTS            ;TO DO THE ENTER FUNCTION
        JC      WHAT            ;IF ERROR FOUND
;
;CRLF UTILITY
;
CRLF:   MVI     B,0DH           ;CAR. RETURN
        CALL    OUT8
LF:     MVI     B,0AH           ;LINE FEED
        CALL    OUT8
        MVI     B,7FH           ;NULLS FOR SLOW TERMINALS
        CALL    OUT8
        JMP     OUT8            ;LET OUT8 DO THE RETURN
;
;TABLE SEARCH ROUTINE USED FOR BOTH COMMAND SEARCHES AND
;SYMBOL LOOK-UPS.  ZERO FLAG SET IF MATCH FOUND, IN WHICH
;CASE H,L HAS VALUE OF STRING (16 BIT).
;LENGTH OF STRING IS IN B, TABLE ADDRESS IS IN
;D,E, ADDS POINTS TO ITEM TO BE LOOKED UP.
;IF NO MATCH, D,E POINT TO BYTE AFTER TABLE END.
;
COMS:   LHLD    ADDS            ;GET ADDR OF ITEM TO LOOK UP
        MOV     C,B             ;LENGTH OF ITEM TO LOOK UP
        LDAX    D               ;1ST BYTE OF NEXT TABLE ENTRY
        ORA     A               ;IF ZERO, ==> END OF TABLE
        JZ      LE23E           ;TO SET RET FLAG FOR NO MATCH
        CALL    SEAR            ;SEE IF ENTRY MATCHES STRING
        LDAX    D               ;GET 1ST BYTE OF VALUE
        MOV     H,A             ;MOVING TO H,L
        INX     D               ;INCREMENT POINTER
        LDAX    D               ;GET 2ND BYTE OF VALUE
        MOV     L,A             ;MOVE TO H,L COMPLETE
        RZ                      ;RET IF SEAR INDICATED MATCH
        INX     D               ;ELSE POINT TO NEXT TABLE ITEM
        JMP     COMS            ;AND TEST IT
LE23E:  INR     A               ;MAKE NON ZERO
        RET                     ;INDICATING NO MATCH
;
;THE FOLLOWING ROUTINE IS THE SAME AS COMS, ABOVE, EXCEPT
;THAT IT IS USED TO SEARCH THE CUSTOM COMMAND TABLE, WHICH
;CONSISTS OF 5 CHAR NAMES WITH ONLY 1ST FOUR TESTED.
;BECAUSE OF THIS IT HAS AN EXTRA INX D INSTR.
;
LE240:  LHLD    ADDS            ;GET ADDR OF ITEM TO LOOK UP
        MOV     C,B             ;LENGTH OF ITEM TO LOOK UP
        LDAX    D               ;1ST BYTE OF TABLE ENTRY
        ORA     A               ;IF ZERO ==> END OF TABLE
        JZ      LE23E           ;TO SET RET FLAG FOR NO MATCH
        CALL    SEAR            ;SEE IF ENTRY MATCHES STRING
        INX     D               ;SKIP 5TH CHAR OF ENTRY NAME
        LDAX    D               ;GET 1ST BYTE OF VALUE
        MOV     H,A             ;MOVING TO H,L
        INX     D               ;POINT TO 2ND BYTE OF VALUE
        LDAX    D               ;GET 2ND BYTE OF VALUE
        MOV     L,A             ;MOVE TO H,L COMPLETE
        RZ                      ;RET IF SEAR INDICATED MATCH
        INX     D               ;POINT TO NEXT ENTRY
        JMP     LE240           ;TEST IT
;
;ROUTINE TO SEE IF TWO STRINGS ARE EQUAL.  STRINGS POINTED
;TO BY D,E AND H,L.  ZERO FLAG SET INDICATES MATCH.  REG C
;HAS LENGTH OF MASTER STRING.  D POINT AFTER ITS STRING
;EVEN IF NO MATCH; H,L POINTS AFTER IT'S STRING IF MATCH,
;ELSE TO 1ST NON-MATCHING CHAR.
;
SEAR:   LDAX    D               ;GET A CHAR FROM 1 STRING
        CMP     M               ;COMPARE TO OTHER STRING
        JNZ     INCA            ;IF IT DOESN'T MATCH
        INX     H               ;INCREMENT POINTER
        INX     D               ;INCREMENT POINTER
        DCR     C               ;COUNT OF CHARS LEFT TO CHECK
        JNZ     SEAR            ;IF CHARS LEFT, TEST THEM
        RET                     ;ELSE DONE
INCA:   INX     D               ;INCREMENT POINTER
        DCR     C               ;DECREMENT COUNT
        JNZ     INCA            ;KEEP INR. POINTER IF NOT DONE
        INR     C               ;ELSE CLEAR ZERO FLAG
        RET                     ;AND RETURN
;
;ROUTINE TO ZERO OUT A BUFFER IN MEMORY
;
ZBUF:   XRA     A               ;GET A ZERO
        LXI     D,ABUF+16       ;POINT TO THE BUFFER
        MVI     B,16            ;BUFFER LENGTH
ZBU1:   DCX     D               ;DECREMENT POINTER
        STAX    D               ;ZERO OUT A LOCATION
        DCR     B               ;DECREMENT COUNT
        JNZ     ZBU1            ;IF MORE LEFT TO ZERO
        RET                     ;DONE
;
;THE EXECUTE COMMAND - DOES A CALL TO A SPECIFIED ADDR.
;
EXEC:   CALL    VCHK            ;CHECK FOR PROPER ARGUMENT
        CALL    CRLF            ;DO CRLF
        LHLD    BBUF            ;GET EXEC ADDR
        PCHL                    ;GO TO IT
;
;ROUTINE TO EXTRACT VALUES FROM A COMMAND LINE
;AND PLACE THEM IN BUFFERS
;
ETRA:   LXI     H,0             ;GET A ZERO
        SHLD    BBUF+2          ;ASSUME NO 2ND OPERAND
        SHLD    FBUF            ;ASSUME NO FILE NAME IN /'S
        CALL    ZBUF            ;ZERO OUT ABUF
        LXI     H,IBUF-1        ;SET TO SCAN COMMAND LINE
VAL1:   INX     H               ;POINT TO NEXT CHAR
        MOV     A,M             ;GET CHAR
        CPI     ' '             ;SPACE ?
        CMC                     ;SET CARRY TO INDICATE CNTL CHR
        RNC                     ;DONE-CR ONLY CNTL CHR ALLOWED
        JNZ     VAL1            ;IF CHAR WAS NOT A SPACE
        SHLD    PNTR            ;ELSE SPACE, SAVE POINTER
        CALL    SBLK            ;AND SCAN TO 1ST CHAR
        CMC                     ;CARRY SET ==> C/R 1ST CHAR
        RNC                     ;DONE THEN
        CPI     '/'             ;FILE NAME DELIMITER ?
        JNZ     VAL5            ;NO, LOOK FOR OTHER PARAMETERS
        LXI     D,FBUF          ;YES, POINT TO FILE NAME BUFFER
        MVI     C,NMLEN         ;FILE NAME LENGTH
VAL2:   INX     H               ;MOVING NAME TO FILE NAME BUFR
        MOV     A,M             ;GET A CHAR
        CPI     '/'             ;TERMINATING SLASH ?
        JZ      VAL3            ;DONE MOVING NAME THEN
        DCR     C               ;ELSE DCR COUNT OF CHARS LEFT
        JM      WHAT            ;IF NAME > 5 CHARS
        STAX    D               ;ELSE PUT LETTER IN BUFFER
        INX     D               ;INCREMENT DEST. POINTER
        JMP     VAL2            ;AND DO NEXT CHARACTER
VAL3:   MVI     A,' '           ;GET A SPACE
VAL4:   DCR     C               ;DECREMENT COUNT OF CHARS LEFT
        JM      DONE            ;IF NEGATIVE, DONE
        STAX    D               ;PAD REST OF BUFR WITH SPACES
        INX     D               ;INCREMENT BUFFER POINTER
        JMP     VAL4            ;CONTINUE TILL BUFFER FILLED
DONE:   CALL    SBL2            ;SCAN TO NEXT NON-SPACE CHAR
        CMC                     ;CARRY MEANS CHAR. RETRN FOUND
        RNC                     ;AND CAR. RETRN = DONE
VAL5:   LXI     D,ABUF          ;POINT TO THE ASCII BUFFER
        CALL    LE2D7           ;PUT IN ABUF & CNVRT BY MODE
        RC                      ;IF C/R WAS TERMINATING CHAR
        CALL    SBLK            ;ELSE SCAN TO 2ND ASCII PARM.
        CMC                     ;TEST FOR C/R FOUND
        RNC                     ;RET., ELSE FALL THRU TO DO 2ND
LE2D7:  CALL    ALPS            ;PUT PARM. IN ABUF
        MOV     A,E             ;GET ENDING ADDR LOW BYTE
        SUB     B               ;SUBTRACTING PARM LENGTH
        MOV     L,A             ;TO GET BACK TO START
        MOV     A,D             ;NOW HIGH ORDER BYTE
        SBI     0               ;IN CASE OF BORROW
        MOV     H,A             ;H,L HAS START OF PARM IN ABUF
        LDA     SMODE           ;GET NUMERIC MODE
        ADD     A               ;MULT. BY 2
        ADI     4               ;AND ADD 4
        CMP     B               ;TEST FOR NO. OF DIGITS ALLOWED
        RC                      ;# DIGITS > 4 AND HEX MODE
        CALL    NORM            ;NORMALIZE ASCII NUMBER
        XCHG                    ;START OF SUB-BUFFER TO D,E
        LXI     H,7             ;LENGTH OF SUB-BUFFERS IN ABUF
        DAD     D               ;H,L POINT TO NEXT SUB-BUFFER
        XCHG                    ;NEXT SUB-BUFFER IN D,E
        RET                     ;DONE
;
;SOME NUMERICAL CONVERSION ROUTINES FOLLOW.
;B,C POINT TO 1ST ASCII DIGIT, NUMERIC STRING ENDS WITH 
;BINARY ZERO.  ANSWER IN H,L AND CARRY SET = ERROR.
;
;EMODE CONVERTS BASED ON CURRENT MODE SETTING, SMODE
;EQUAL TO 0 FOR HEX, 1 FOR OCTAL.
;
EMODE:  LDA     SMODE           ;GET MODE SETTING
        ORA     A               ;SET FLAGS
        JNZ     AOCT            ;OCTAL IS 1
AHEX:   LXI     H,0             ;START WITH A ZERO
AHE1:   LDAX    B               ;GET A DIGIT
        ORA     A               ;SEE IF ZERO
        RZ                      ;YES - DONE
        DAD     H               ;MULT H,L BY TWO
        DAD     H               ;AGAIN - NOW 4X ORIG VALUE
        DAD     H               ;AGAIN - NOW 8X ORIG VALUE
        DAD     H               ;AGAIN - NOW 10H X ORIG VALU
        CALL    AHS1            ;CONVERT DIGIT TO BINARY
        CPI     10H             ;SEE IF LEGAL
        CMC                     ;SET CARRY FLAG
        RC                      ;FOR PROPER ILLEAGLE RETURN
        ADD     L               ;OTHERWISE ADD IT INTO THE NO.
        MOV     L,A             ;AND RESTORE TO H,L
        INX     B               ;NOW POINT TO NEXT ASCII DIGIT
        JMP     AHE1            ;AND PROCESS IT
;
;ROUTINE TO CONVERT ASCII HEX DIGIT INTO BINARY
;
AHS1:   SUI     30H             ;CONVERT 0-9 TO BINARY
        CPI     0AH             ;SEE IF > ASCII 9
        RC                      ;RETURN IF NOT, DONE
        SUI     7               ;TO TAKE CARE OF A-F
        CPI     0AH             ;COMPARE TO DECIMAL 10
        RNC                     ;IF > 10 OR =, RETURN DONE
        MVI     A,0FFH          ;ELSE BAD CHAR, SET A > 10H
        RET                     ;AND RETURN
;
;HERE IS A ROUTINE TO CONVERT BOTH OF TWO POSSIBLE
;VALUES IN ABUF INTO BINARY BASED ON THE CURRENT MODE
;SETTING, AND PLACE THEM INTO BBUF AND BBUF+2
;
LE31E:  LXI     B,ABUF          ;POINT TO THE 1ST OPERAND
        CALL    EMODE           ;CONVERT IT
        RC                      ;RETURN W/CARRY SET ON ERROR
        SHLD    BBUF            ;PUT VALUE IN BBUF
        LXI     B,ABUF+7        ;POINT TO 2ND VALUE
        CALL    EMODE           ;CONVERT IT
        RC                      ;RETURN W/CARRY SET ON ERROR
        SHLD    BBUF+2          ;PUT IT IN THE BINARY BUFFER
        RET                     ;DONE
;
;OCTAL TO BINARY CONVERSION
;
AOCT:   LXI     H,0             ;START W/ZERO
LE336:  LDAX    B               ;GET A CHAR
        ORA     A               ;ZERO ?
        RZ                      ;DONE THEN, RETURN
        DAD     H               ;MULT H,L BY TWO
        DAD     H               ;AGAIN - NOW 4X ORIG VALU
        DAD     H               ;AGAIN - NOW 8X ORIG VALU
        SUI     30H             ;CONVERT ASCII DIGIT TO BINARY
        CPI     8               ;LEGAL VALUE ?
        CMC                     ;SET CARRY FOR PROPER RETURN
        RC                      ;RETURN IF BAD
        ADD     L               ;ELSE ADD IN VALUE
        MOV     L,A             ;AND PUT IT BACK
        INX     B               ;POINT TO NEXT ASCII DIGIT
        JMP     LE336           ;AND PROCESS IT
;
;DECIMAL OUTPUT ROUTINE FOR BINARY NO. IN REG A.
;
DOUT:   CALL    BIND            ;CONVRT A TO DECIMAL IN ADDS
LE34B:  CALL    LE57A           ;PRINT 1ST TWO CHARS IN ADDS
        INX     H               ;POINT TO 3RD CHAR IN ADDS
        MOV     B,M             ;GET IT
        JMP     OUT8            ;PRINT IT AND RETURN
;
;OCTAL OUTPUT ROUTINE
;
OOUT:   CALL    BINO            ;CONVERT TO OCTAL IN ADDS
        JMP     LE34B           ;PUT OUT 3 OCTAL DIGITS AND RET
;
;OCTAL FOLLOWED BY SPACE
;
OOTB:   CALL    OOUT            ;PUT OUT OCTAL NO.
LE35C:  MVI     B,' '           ;GET A SPACE
        JMP     OUT8            ;PUT IT OUT
;
;OUTPUT NUMBER OF SPACES IN REG C
;
BLKO:   CALL    LE35C           ;PUT OUT A SPACE
        DCR     C               ;DECREMENT COUNT
        JNZ     BLKO            ;PUT OUT MORE IF NOT DONE
        RET                     ;ELSE RETURN
;
;COMPARE THE ADDRESSES AT BBUF AND BBUF+2
;INCREMENT THE ADDR AT BBUF AND RET CARRY SET IF SAME
;USED BY DUMP COMMAND TO TEST FOR END
;
ACHK:   LHLD    BBUF            ;GET 1ST ADDRESS
        LDA     BBUF+3          ;GET HIGH BYTE OF 2ND ADDR
        CMP     H               ;COMPARE HIGH ORDER BYTES
        JNZ     ACH1            ;BBUF LOWER, INR IT & GO HOME
        LDA     BBUF+2          ;GET 2ND ADDR LOW BYTE
        CMP     L               ;COMPARE TWO ADDR.
        JNZ     ACH1            ;BBUF LOWER, INR IT & GO HOME
        STC                     ;ADDR. ARE SAME, SET CARRY
ACH1:   INX     H               ;INCREMENT BBUF ADDR
        SHLD    BBUF            ;PUT IT BACK
        RET                     ;DONE
;
;PRINT CHARACTERS UNTIL A CAR. RET. FOUND
;
SCRN:   XRA     A               ;BINARY ZERO
        STA     XOUT            ;LAST CHAR OUTPUT
        MOV     B,M             ;GET A CHARACTER
        MVI     A,0DH           ;CAR. RETRN TO A.
        CMP     B               ;CHAR IN B = C/R ?
        RZ                      ;YES - DONE
        CALL    OUT8            ;NO - OUTPUT IT & MOV TO A
        INX     H               ;INCREMENT POINTER
        JMP     SCRN+1          ;UPDATE XOUT AND DO NEXT CHAR
;
;CONVERT BINARY TO HEX ASCII & STORE
;
BINH:   LXI     H,ADDS          ;DESTINATION FOR ASCII
        MOV     B,A             ;SAVE CHAR IN B
        RAR                     ;MOVING HIGH ORDER NIBBLE TO
        RAR                     ;LOW ORDER NIBBLE
        RAR     
        RAR     
        CALL    BIN1            ;CONVERT TO LETTER OR NO.
        MOV     M,A             ;STORE IT
        INX     H               ;INCREMENT DEST. POINTER
        MOV     A,B             ;GET BACK ORIG NO.
        CALL    BIN1            ;CONVERT LOW ORDER NIBBLE TO NO
        MOV     M,A             ;PUT IT IN MEMORY
        RET                     ;DONE
;
;CONVERT LOW ORDER NIBBLE OF A TO HEX ASCII
;
BIN1:   ANI     0FH             ;MASK LOW NIBBLE
        ADI     30H             ;CONVERT TO NO.
        CPI     '9'+1           ;SEE IF > 9
        RC                      ;IT WASN'T -- DONE
        ADI     7               ;ADD 7 TO GET LETTER
        RET                     ;DONE
;
;BINARY TO DECIMAL
;
BIND:   LXI     H,ADDS          ;POINT TO DESTINATION
        MVI     B,100           ;DOING SUCCESSIVE SUBTRACTIONS
        CALL    BID1            ;SUBTRACT & KEEP COUNT
        MVI     B,10            ;NOW WORK WITH TENS
        CALL    BID1            ;MORE SUBTRACTIONS
        ADI     30H             ;CONVERT UNITS TO DECIMAL ASCII
        MOV     M,A             ;STORE UNITS DIGIT IN MEMORY
        RET                     ;DONE
BID1:   MVI     M,'0'-1
        INR     M               ;INCR COUNT OF SUBTRACTIONS
        SUB     B               ;DO A SUBTRACTION
        JNC     BID1+2          ;DO IT AGAIN IF ANS > 0
        ADD     B               ;IF ANS < 0, UN-SUBTRACT
        INX     H               ;INCREMENT DEST. POINTER
        RET                     ;DIGIT STORED
;
;BINARY TO OCTAL
;
;NOTE THAT THE CARRY IS USED, SO THE MAX SINGLE BYTE OCTAL
;VALUE THAT CAN BE HANDLED IS 777 NOT 377.  THIS FEATURE IS
;USED IN THE DUMP ROUTINE.
;
BINO:   LXI     H,ADDS          ;GET DEST. POINTER
        MOV     B,A             ;SAVE ORIG NO. IN B
        RAL                     ;HIGH ORDER BITS & CARRY
        RAL                     ;MOVING TO LOW ORDER 3 BITS
        RAL                     ;
        ANI     7               ;MASK TO 3 BITS
        ADI     30H             ;CONVERT TO ASCII
        MOV     M,A             ;STORE IN MEMORY
        MOV     A,B             ;GET ORIG. NO.
        RAR                     ;MOVE MIDDLE 3 BITS
        RAR                     ;TO LOW ORDER 3 BITS
        RAR     
        ANI     7               ;AND MASK THEM OUT
        ADI     30H             ;CONVERT TO ASCII
        INX     H               ;INCREMENT MEMORY POINTER
        MOV     M,A             ;AND PUT DIGIT IN MEMORY
        MOV     A,B             ;GET ORIGINAL DIGIT AGAIN
        ANI     7               ;MASK TO LOW ORDER 3 BITS
        ADI     30H             ;CONVERT TO ASCII
        INX     H               ;INCREMENT MEMORY POINTER
        MOV     M,A             ;AND STORE IN MEMORY
        RET                     ;DONE
;
;HERE IS THE ALS-8 COMMAND TABLE
;
CTAB:   DB      'LIST'
        DB      (LIST SHR 8) AND 0FFH, LIST AND 0FFH
        DB      'TEXT'
        DB      (TEXT SHR 8) AND 0FFH, TEXT AND 0FFH
        DB      'DELT'
        DB      (DELT SHR 8) AND 0FFH, DELT AND 0FFH
        DB      'MODE'
        DB      (MODE SHR 8) AND 0FFH, MODE AND 0FFH
        DB      'FCHK'
        DB      (FCHK SHR 8) AND 0FFH, FCHK AND 0FFH
        DB      'FORM'
        DB      (FORM SHR 8) AND 0FFH, FORM AND 0FFH
        DB      'NFOR'
        DB      (NFOR SHR 8) AND 0FFH, NFOR AND 0FFH
        DB      'SWCH'
        DB      (EORNS SHR 8) AND 0FFH, EORNS AND 0FFH
        DB      'SIMU'
        DB      (SIMU SHR 8) AND 0FFH, SIMU AND 0FFH
        DB      0
        DB      'DUMP'
        DB      (DUMP SHR 8) AND 0FFH, DUMP AND 0FFH
        DB      'EXEC'
        DB      (EXEC SHR 8) AND 0FFH, EXEC AND 0FFH
        DB      'ENTR'
        DB      (ENTR SHR 8) AND 0FFH, ENTR AND 0FFH
        DB      'FILE'
        DB      (FILE SHR 8) AND 0FFH, FILE AND 0FFH
        DB      'ASSM'
        DB      (ASSM SHR 8) AND 0FFH, ASSM AND 0FFH
        DB      'ASSI'
        DB      (ASSM SHR 8) AND 0FFH, ASSM AND 0FFH
        DB      'IODR'
        DB      (IODR SHR 8) AND 0FFH, IODR AND 0FFH
        DB      'STAB'
        DB      (STAB SHR 8) AND 0FFH, STAB AND 0FFH
        DB      'SYML'
        DB      (SYML SHR 8) AND 0FFH, SYML AND 0FFH
        DB      'CUST'
        DB      (CUST SHR 8) AND 0FFH, CUST AND 0FFH
        DB      'RNUM'
        DB      (RNUM SHR 8) AND 0FFH, RNUM AND 0FFH
        DB      'FMOV'
        DB      (FMOV SHR 8) AND 0FFH, FMOV AND 0FFH
        DB      'EDIT'
        DB      (TXT2 SHR 8) AND 0FFH, TXT2 AND 0FFH
        DB      'AUTO'
        DB      (AUTO SHR 8) AND 0FFH, AUTO AND 0FFH
        DB      'FIND'
        DB      (FIND2 SHR 8) AND 0FFH, FIND2 AND 0FFH
        DB      'TERM'
        DB      (TERM SHR 8) AND 0FFH, TERM AND 0FFH
        DB      0
;
;SYSTEM SYMBOL TABLE ROUTINES
;
SYML:   LXI     H,IBUF+4        ;POINT TO FIFTH CHAR
        MOV     A,M             ;GET IT
        CPI     'E'             ;SYMLE - ENTER INTO TABLE ?
        JZ      LE4AB           ;YES - MAKE AN ENTRY
        CPI     'D'             ;SYMLD - DELETE FROM TABLE ?
        JZ      LE4B4           ;YES - DELETE IT THEN
;
;PRINT CONTENTS OF SYSTEM SYMBOL TABLE
;
        LXI     H,SYSYM         ;POINT TO SYSTEM SYMBOL TABLE
LE48F:  MVI     A,4             ;PRINT 4 ACROSS
        STA     CCNT            ;SAVE AS COUNT
        CALL    CRLF            ;START ON NEW LINE
LE497:  MOV     A,M             ;GET 1ST CHAR
        ORA     A               ;END OF TABLE ?
        JZ      LE4EE           ;THEN PRINT END ADDRESS
        CALL    LEEA8           ;PRINT 1 SYMBOL & VALUE
        INX     D               ;POINT TO NEXT SYMBOL
        LXI     H,CCNT          ;POINT AT COUNT
        DCR     M               ;DCR NO LEFT THIS LINE
        XCHG                    ;TABLE POINTER TO H,L
        JNZ     LE497           ;PRINT MORE THIS LINE
        JMP     LE48F           ;OR START NEW LINE
;
;ENTER NEW SYMBOL INTO TABLE
;
LE4AB:  CALL    VCHK            ;CHECK FOR VALID VALUE & NAME
LE4AE:  CALL    LE4CE           ;VALIDATE NAME & MOVE TO ABUF
        JMP     LECE2           ;PUT SYMBOL INTO TABLE
;
;DELETE SYMBOL
;
LE4B4:  CALL    LE4AE           ;H,L POINTS TO VALUE ON RET
LE4B7:  INX     H               ;POINT TO 2ND CHAR OF VALUE
        INX     H               ;POINT TO NEXT SYMBOL
        XCHG                    ;D,E POINT AT SYM. AFTR DELETED
        LXI     H,-(NMLEN+2)    ;SYMBOL TABLE ENTRY LENGTH
        DAD     D               ;H,L POINTS AT BEG. OF ENTRY
        LDAX    D               ;GET CHAR FROM TABLE ENTRY
LE4BF:  ORA     A               ;SET FLAGS
        MVI     B,(NMLEN+2)     ;ENTRY LENGTH
LE4C2:  MOV     M,A             ;MOVE CHAR UP IN TABLE
        RZ                      ;IF END OF TABLE MOVED
        INX     H               ;INR DEST. POINTER
        INX     D               ;INR SOURCE POINTER
        LDAX    D               ;GET NEXT CHAR FROM SYMBOL
        DCR     B               ;COUNT OF CHRS THIS SYMBOL
        JNZ     LE4C2           ;IF NOT DONE THIS SYMBOL
        JMP     LE4BF           ;TO MOVE NEXT SYMBOL UP
;
;ROUTINE TO VALIDATE A SYMBOL NAME & MOVE IT TO ABUF
;
LE4CE:  CALL    ZBUF            ;BLANK OUT ABUF
        XCHG                    ;H,L POINT TO START OF ABUF
        SHLD    ADDS            ;SAVE ADDRESS
        LXI     D,FBUF          ;POINT AT FILE-NAME BUFFER
        XCHG                    ;H,L=FBUF & D,E=ABUF
        XRA     A               ;GET A ZERO
        STA     FREAD           ;SAVE AS 1ST FREE ADDRESS
        MOV     A,M             ;GET 1ST CHAR OF NAME
        ORA     A               ;ZERO ?
        JZ      WHAT            ;ERROR IF NO NAME GIVEN
        CALL    LF154           ;MOVE THE NAME IF IT'S VALID
        CPI     21H             ;MAKE SURE SPACE FOLLOWS NAME
        JNC     WHAT            ;ERROR IF SPACE DOESN'T FOLLOW
        LHLD    BBUF            ;GET VALUE
        RET                     ;RETURN W/VALUE IN H,L
;
;DO A CR/LF AND PRINT ASCII CONTENTS OF ADDR POINTED TO BY H,L
;
LE4EE:  XCHG                    ;POINTER FROM D,E TO H,L
        CALL    CRLF            ;START ON NEW LINE
        JMP     ADOUT           ;PRINT ADDR POINTED TO BY H,L
;
;CUSTOM COMMAND TABLE MANAGEMENT
;
CUST:   LXI     H,IBUF+4        ;POINT AT 5TH CHAR
        MOV     A,M             ;GET IT
        CPI     'E'             ;ENTER NEW CUST. COMMAND ?
        JZ      LE509           ;YES - DO IT
        CPI     'D'             ;DELETE CUSTOM COMMAND ?
        JZ      LE004           ;YES - DO IT
        LXI     H,CUCOM         ;POINT AT CUSTOM COMMAND TABLE
        JMP     LE48F           ;PRINT USING SYML ROUTINES
;
;ENTER AND DELETE MOSTLY USE SYMLE & SYMLD CODE
;
LE509:  CALL    VCHK            ;MAKE SURE VALUE GIVEN
LE50C:  CALL    LE4CE           ;VALIDATE NAME & MOVE TO ABUF
        LXI     D,CUCOM         ;D,E POINT AT CUST. TABLE
        JMP     LECE5           ;PUT VALUE INTO TABLE
;
;ROUTINE TO EXTRACT VALUES FROM COMMAND LINE & REPORT AN ERROR
;
VALC:   CALL    ETRA            ;ETRA DOES THE REAL WORK
        JC      WHAT            ;IF AN ERROR OCCURED
        RET                     ;DONE, NO ERROR
;
;ROUTINE TO VERIFY THAT REQUIRED PARAMETERS WERE ENTERED
;
VCHK:   LDA     ABUF            ;GET 1ST CHAR OF ABUF
        ORA     A               ;ZERO ?
        JZ      WHAT            ;ERROR - REQD. PARM. OMITTED
        RET                     ;DONE
;
;THE DUMP COMMAND
;
DUMP:   CALL    VCHK            ;MAKE SURE ADDRESS GIVEN
        MVI     A,16            ;16 VALUES PER LINE
        STA     CCNT            ;SAVE AS COUNT
LE52C:  CALL    CRLF            ;START NEW LINE
        LHLD    BBUF            ;GET ADDRESS
        XCHG                    ;DO D,E
        CALL    ADOUT           ;PRINT ADDRESS ON LEFT OF LINE
        MVI     B,':'           ;FOLLOW ADDRESS WITH COLON
        CALL    OUT8            ;PRINT IT
        CALL    LE35C           ;PUT OUT A SPACE
        LDA     CCNT            ;GET COUNT OF ADDRS/LINE
LE541:  STA     DCNT            ;SAVE AS DUMP COUNT
        LHLD    BBUF            ;GET ADDR TO DUMP
        MOV     A,M             ;GET DATA AT THAT ADDR
        CALL    DUMO            ;PRINT ITS VALUE
        CALL    LE35C           ;PUT A SPACE OUT
        CALL    ACHK            ;COMPARE BBUF & BBUF+2
        RC                      ;REACHED END ADDRS. - DONE
        LDA     DCNT            ;ELSE GET DUMP COUNT
        DCR     A               ;DCR COUNT BYTES LEFT THIS LINE
        JNZ     LE541           ;DO MORE BYTES ON THIS LINE
        JMP     LE52C           ;DO ANOTHER LINE
;
;ROUTINE TO PUT OUT AN ADDRESS
;
;NOTE THAT THIS ROUTINE TAKES ADVANTAGE (?) OF THE FACT THAT
;BINO USES THE CARRY BIT TO OUTPUT A SINGLE BYTE IN A AS
;XXX WHERE XXX MAY BE 777 (INCLUDING CARRY) AND IS NOT
;LIMITED TO 377.  ADDRESSES THUS COME OUT AS A SINGLE 16 BIT
;NUMBER RATHER THAN IN THE MORE COMMON "SPLIT OCTAL" CONSISTING
;OF TWO SEPERATE 8 BIT QUANTITIES WITH A MAX VALUE OF 377 EACH.
;
ADOUT:  LDA     SMODE           ;GET CURRENT MODE
        ORA     A               ;CLEAR CARRY & SET ZERO FLAG
        MOV     A,D             ;HIGH ORDER BYTE TO A
        JZ      LE565           ;SKIP NEXT PART IF BASE HEX
        RAR                     ;LOW ORDER BIT TO CARRY FOR OCT
LE565:  MOV     D,A             ;RESTORE HIGH ORDER BYTE
        RAL                     ;SHIFT LEFT 1 BIT THRU ACCUM.
        MOV     C,A             ;LOW BIT/CARRY IN C FOR OCTAL
        MOV     A,D             ;GET HIGH ORDER BYTE
        CALL    DUMO            ;OUTPUT USING CURRENT MODE
        MOV     A,C             ;FOR OCTAL, GET ORIG HIGH BYTE
        RAR                     ;SET CARRY BIT
        MOV     A,E             ;GET LOW ORDER BYTE
DUMO:   LXI     H,SMODE         ;POINT TO MODE
        MOV     B,M             ;GET IT
        DCR     B               ;DECR (OCTAL MODE = 1)
        JZ      OOUT            ;FOR OCTAL
HOUT:   CALL    BINH            ;CONVERT BIN TO HEX IN ADDS
LE57A:  LXI     H,ADDS          ;BUFFER WHERE CHARS STORED
        MOV     B,M             ;GET CHAR IN B
        CALL    OUT8            ;PRINT A HEX ASCII CHAR
        INX     H               ;INX POINTER
        MOV     B,M             ;GET NEXT CHAR
        JMP     OUT8            ;PRINT IT & GO HOME
HOTB:   CALL    HOUT            ;PUT OUT HEX
        JMP     LE35C           ;PUT OUT SPACE & GO HOME
;
;MODE - USED TO SET THE OUTPUT MODE
;
MODE:   CALL    VCHK            ;VERIFY PARAMETER GIVEN
        LXI     B,ABUF          ;POINT TO PARAMETER
        CALL    AHEX            ;CONVERT TO BINARY
        JC      WHAT            ;IF PARAMETER INVALID
        MOV     A,H             ;GET HIGH ORDER BYTE
        ORA     A               ;SET FLAGS
        JNZ     WHAT            ;PARM. MUST BE < 256
        MVI     B,0             ;ASSUME HEX SPECIFIED
        MOV     A,L             ;GET LOW ORDER PARMATER
        CPI     16H             ;IS PARAMETER 16 ?
        JZ      LE5AB           ;YES - STORE 0 AS MODE
        INR     B               ;MAKE B = MODE = 1
        CPI     8               ;IS PARAMETER 8 ?
        JNZ     WHAT            ;MUST BE EITHER 8 OR 16
LE5AB:  MOV     A,B             ;GET MODE (REG B)
        STA     SMODE           ;MAKE IT THE MODE
        RET                     ;WE ARE DONE
;
;PROCESS THE FILE COMMANDS
;
FILE:   LDA     FBUF            ;GET 1ST CHAR OF FILE NAME
        ORA     A               ;SET FLAGS
        JZ      FOUT            ;NO NAME GIVEN - DO A LISTING
        CALL    FSEA            ;LOOK UP FILE
        XCHG                    ;D,E HAS ADDR OF DIR. ENTRY
        JNZ     TEST            ;IF DIR. ENTRY FOUND
;
;ENTRY MATCHING FBUF NOT FOUND
;
        LDA     ABUF            ;CHECK FOR PARAMETER GIVEN
        ORA     A               ;SET FLAGS
        JZ      WHAT            ;ADDR. REQUIRED FOR NEW FILE
;
;CHECK FOR ROOM TO ADD A NEW FILE
;
        LDA     FEF             ;GET FREE ENTRY FOUND FLAG
        ORA     A               ;SET FLAGS
        JNZ     ROOM1           ;TO ADD THE NEW FILE
LE5CC:  LXI     H,EMES1         ;POINT TO ERROR MESSAGE
        JMP     MESS            ;PRINT IT
;
;ENTRY MATCHING FBUF FOUND
;
TEST:   LDA     ABUF            ;SEE IF PARAMETER GIVEN
        ORA     A               ;SET FLAGS
        JZ      SWAPS           ;NO PARMS - MAKE IT CUR FILE
        LHLD    BBUF            ;GET BINARY VALU OF PARM
        MOV     A,H             ;HIGH ORDER BYTE
        ORA     L               ;SEE IF PARM IS ZERO
        JZ      SWAPS           ;TO DELETE THE FILE
LE5E1:  LXI     H,EMES2         ;CAN'T RESPECIFY EXISTING FILE
        JMP     MESS            ;TELL IT TO THE OPERATOR
;
;CREATE NEW FILE - NAME IS IN FBUF
;FREAD POINTS TO A FREE DIRECTORY ENTRY
;
ROOM1:  CALL    ROOM            ;MOVE NAME IN FBUF TO DIRECTORY
;
;MAKE FILE POINTED TO BY D,E CURRENT
;
SWAPS:  LXI     H,FILE0         ;POINT H,L TO CUR FILE
        MVI     C,FELEN         ;BYTE COUNT
SWAP:   LDAX    D               ;GET A BYTE FROM NEW CUR FILE
        MOV     B,M             ;GET BYTE FROM CUR CUR FILE
        MOV     M,A             ;NEW CUR FILE BYTE TO FILE 0
        MOV     A,B             ;EXCHANGE
        STAX    D               ;CUR CUR FILE BYTE TO FREAD
        INX     D               ;INR NON-CURRENT FILE POINTER
        INX     H               ;INR FILE 0 POINTER
        DCR     C               ;COUNT OF BYTES TO MOVE
        JNZ     SWAP            ;IF NOT DONE SWAPPING ENTRIES
;
;CHECK FOR 2ND PARAMETER (ADDRESS)
;
        LDA     ABUF            ;GET 2ND PARAMETER 1ST CHAR
        ORA     A               ;SET FLAGS
        JZ      FOUT            ;NONE GIVEN
;
;PROCESS 2ND PARAMETER
;
        LHLD    BBUF            ;GET 2ND PARAMETER ADDRESS
        SHLD    BOFP            ;MAKE IT BEG OF FILE POINTER
        SHLD    EOFP            ;AND END OF FILE POINTER
        MOV     A,L             ;TEST FOR ADDR = 0
        ORA     H               ;
        JZ      FIL35           ;YES - DELETE IT
        MVI     M,1             ;NO - PUT EOF MARK AT BOFP
FIL35:  XRA     A               ;BINARY ZERO
        STA     MAXL            ;AT MAX. LINE NO.
        JMP     FOOT            ;OUTPUT PARAMETERS
;
;LIST 1 OR MORE FILES & THEIR PARAMETERS
;
FOUT:   CALL    CRLF            ;DO A CR/LF
        LDA     IBUF+4          ;GET OPT. 5TH CHAR
        CPI     'S'             ;WAS COMMAND FILES ?
        MVI     C,MAXFIL        ;MAX NO OF FILES
        JZ      FOUL            ;YES - LIST ALL FILES
FOOT:   MVI     C,1             ;ELSE LIST ONLY 1 FILE
;
;LIST PARMS. FOR NO OF FILES IN REG C.
;
FOUL:   LXI     H,FILE0         ;POINT TO DIRECTORY
LE62A:  MOV     A,C             ;GET NO FILES LEFT TO LIST
FINE:   STA     FOCNT           ;SAVE AS FILE OUTPUT COUNT
        PUSH    H               ;SAVE DIRECTORY POINTER
        LXI     D,NMLEN         ;NAME LENGTH
        DAD     D               ;POINT AT FILES BOFP
        MOV     A,M             ;GET 1ST BYTE OF BOFP
        ORA     A               ;ZERO ?
        JNZ     FOOD            ;NO - LIST IT
        INX     H               ;POINT AT 2ND BYTE OF BOFP
        ADD     M               ;ADD TO 1ST BYTE
        INX     H               ;POINT TO EOFP 1ST BYTE
        JNZ     FOOD            ;BOFP NON ZERO - LIST IT
        INX     SP              ;TO "UN-PUSH" H,L
        INX     SP              ;
        INX     H               ;POINT TO 2ND BYTE OF EOFP
        INX     H               ;POINT TO MAXL
        JMP     FEET            ;TO GO TO NEXT ENTRY
;
;HAVE A VALID ENTRY TO OUTPUT
;
FOOD:   POP     H               ;GET POINTER TO FILES NAME
        CALL    CRLF            ;START IT ON A FRESH LINE
        MVI     C,NMLEN         ;LENGTH
FAST:   MOV     B,M             ;GET A CHAR OF NAME
        CALL    OUT8            ;PRINT IT
        DCR     C               ;DECREMENT COUNT
        INX     H               ;POINT TO NEXT CHAR
        JNZ     FAST            ;PRINT REMAINING CHARS IF ANY
        CALL    LE666           ;PRINT SPACE, BOFP, SPACE
        CALL    LE666           ;PRINT SPACE, EOFP, SPACE
FEET:   LXI     D,FELEN-NMLEN-4 ;LENGTH OF MAXL
        DAD     D               ;POINT AT NEXT FILE ENTRY
        LDA     FOCNT           ;PRINTING 1 OR ALL ?
        DCR     A               ;DECR. COUNT
        JNZ     FINE            ;TO PRINT MORE
        RET                     ;DONE
;
;ROUTINE TO PRINT AN ADDRESS
;
LE666:  CALL    LE35C           ;PRINT SPACE
        MOV     E,M             ;GET BYTE OF BOFP OR EOFP
        INX     H               ;POINT TO 2ND BYTE
        MOV     D,M             ;GET IT
        PUSH    H               ;SAVE POINTER INTO FILE DIR.
        CALL    ADOUT           ;PRINT ADDRESS
        CALL    LE35C           ;AND TRAILING SPACE
        POP     H               ;RESTORE POINTER
        INX     H               ;POINT TO MAXL
        RET                     ;DONE
;
;MOVE FILE NAME TO BLOCK POINTED TO BY FREAD
;
ROOM:   LHLD    FREAD           ;GET ADDR OF FREE ENTRY
        XCHG                    ;TO D,E
        LXI     H,FBUF          ;ADDR OF NAME IN COMMAND
        PUSH    D               ;SAVE DIRECTORY ADDR
        MVI     C,NMLEN         ;NO CHARS TO MOVE
MOV23:  MOV     A,M             ;GET CHAR FROM COMMAND
        STAX    D               ;PUT INTO DIRECTORY
        INX     D               ;INCR. DIR. POINTER
        DCR     C               ;DCR CHAR COUNT
        INX     H               ;INCR. FBUF POINTER
        JNZ     MOV23           ;MOVE REMAINING CHARS
        POP     D               ;RESTORE DIR. POINTER
        RET                     ;DONE
;
;FILE DIRECTORY SEARCH ROUTINE
;LOOK FOR ENTRY W/NAME IN FBUF - ZERO SET ==> NOT FOUND
;ELSE H,L POINT TO THE ENTRY
;ALSO, IF ENTRY FOUND W/BOFP ADDR = 0, SET FEF FLAG > 0
;AND FREAD TO THE ADDR OF THAT ENTRY
;
FSEA:   LXI     D,FILE0         ;POINT TO DIRECTORY
LE68D:  XRA     A               ;GET ZERO
        STA     FEF             ;ASSUME NO FREE ENTRIES
        MVI     B,MAXFIL        ;NO OF ENTRIES IN DIR.
FSE10:  LXI     H,FBUF          ;ADDR OF NAME TO LOOK UP
        MVI     C,NMLEN         ;NO CHARS PER NAME
        CALL    SEAR            ;COMPARE CURRENT ENTRY TO FBUF
        PUSH    PSW             ;SAVE RESULTS FLAG
        PUSH    D               ;SAVE DIR POINTER
        LDAX    D               ;GET BYTE OF BOFP ADDR
        ORA     A               ;EMPTY ?
        JNZ     FSE20           ;NO
        INX     D               ;2ND BYTE
        LDAX    D               ;GET IT
        ORA     A               ;SET FLAGS
        JNZ     FSE20           ;NOT ZERO
        XCHG                    ;DIR POINTER TO H,L
        LXI     D,-NMLEN-1      ;
        DAD     D               ;POINT BACK TO EMPTY ENTRY
        SHLD    FREAD           ;AND SAVE IT'S ADDRESS
        MOV     A,D             ;D HAPPENS TO BE NON-ZERO
        STA     FEF             ;AND SO NOW IS FEF
        POP     H               ;RESTORE DIR POINTER TO H,L
        POP     PSW             ;RESTORE SEAR RESULTS FLAG
;
;MOVE TO NEXT ENTRY
;
FSE15:  LXI     D,FELEN-NMLEN
        DAD     D               ;H,L POINTS TO NEXT ENTRY
        XCHG                    ;ENTRY ADDR TO D,E
        DCR     B               ;COUNT OF FILES IN DIR
        RZ                      ;NO FILES LEFT TO SEARCH
        JMP     FSE10           ;SEARCH REMAINING FILES
;
;ENTRY WASN'T FREE, LOOK AT SEAR RESULTS
;
FSE20:  POP     H               ;RESTORE DIRECTORY POINTER
        POP     PSW             ;SEAR RESULTS IN FLAG
        JNZ     FSE15           ;IT DIDN'T MATCH ANYWAY
;
;MATCHING ENTRY FOUND WHICH WASN'T FREE
;
        LXI     D,-NMLEN
        DAD     D               ;H,L POINT AT THE DIR. ENTRY
        MOV     A,D             ;D IS NON-ZERO
        ORA     A               ;SET FLAGS TO CLEAR ZERO FLAG
        RET                     ;DONE
;
;FORM COMMAND TO SET OUTPUT MODE
;
FORM:   CMA                     ;ALL COMNDS ARE EXEC WITH A=0
NFOR:   STA     LFMT            ;
        JMP     EORMS           ;IT'S THAT SIMPLE, FOLKS
;
;FCHK
;
FCHK:   LHLD    BOFP            ;GET BEG. OF FILE PTR
        MOV     A,H             ;SEE IF BOFP IS ZERO
        ORA     L               ;WOULD IMPLY NO FILES
        JZ      LE5E1           ;PRINT ERROR MESSAGE
        LXI     B,4             ;LENGTH OF LINE NO.
        DAD     B               ;H,L POINTS JUST PAST LINE NO
        INR     C               ;C = 5 = LINE LEN TO THIS PT.
LE6E0:  INX     H               ;POINT TO NEXT CHAR
        INR     C               ;LINE LENGTH TO THIS POINT
        CALL    LE72B           ;GET TERM WIDTH IN A
        CMP     C               ;LINE > TERMINAL WIDTH ?
        JC      LE718           ;YES - ERROR
        MOV     A,M             ;GET LINE CHAR
        CPI     0DH             ;END OF LINE ?
        JNZ     LE6E0           ;KEEP TESTING THIS LINE
        LHLD    BOFP            ;GET BEG. OF FILE PTR
        MOV     M,C             ;MAKE LENGTH BYTE OK 1ST LINE
LE6F3:  MOV     A,M             ;GET LENGTH BYTE
        CPI     1               ;END OF FILE ?
        JZ      LE725           ;TO RESET MAXL
        CPI     6               ;MIN LINE LEN (NO. + LEN + CR)
        JC      LE718           ;REPORT ERROR IF LINE TOO SHORT
        CALL    LE72B           ;GET TERMINAL WIDTH IN A
        CMP     M               ;COMPARE WITH LENGTH BYTE
        MOV     C,M             ;LENGTH BYTE TO C
        DCR     C               ;GOING TO COUNT CHARS IN LINE
LE704:  DCR     C               ;COUNT OF ACUTAL CHARS LEFT
        INX     H               ;POINT TO NEXT CHAR
        MOV     A,M             ;GET IT
        JZ      LE712           ;COUNT TO 0 - MUST BE C/R
        CPI     20H             ;IS CHAR CNTL CHAR ?
        JC      LE718           ;CNTL CHARS NOT ALLOWED
        JMP     LE704           ;TEST NEXT CHAR IN LINE
LE712:  CPI     0DH             ;IS CHAR C/R ?
        INX     H               ;POINT TO NEXT CHAR
        JZ      LE6F3           ;YES - GO ON, CHECK FOR EOF
;
;ERROR
;
LE718:  XCHG                    ;BAD ADDRESS TO D,E
        CALL    LE35C           ;PRINT A SPACE
        CALL    ADOUT           ;FOLLOWED BY THE BAD ADDRESS
        LXI     H,FERR          ;POINT TO FILE ERROR MESSAGE
        JMP     MESS            ;PRINT THE ERROR MESSAGE
LE725:  CALL    LEA50           ;UPDATE MAXL
        JMP     FOUT            ;PRINT FILE PARMS & RETURN
LE72B:  LDA     TERMW           ;GET TERM WIDTH
        ADI     -(IBUF+1) AND 0FFH
        RET                     ;BINARY TERM WIDTH IN A
;
;FILE MOVE COMMANDS
;
FMOV:   CALL    VCHK            ;CHECK FOR REQD PARMS
        LHLD    BOFP            ;GET BEG OF FILE PTR
        XCHG                    ;BOFP TO D,E
        LHLD    BBUF            ;GET DEST ADDR
        SHLD    BOFP            ;NEW BEG OF FILE
        MOV     A,L             ;LOW ORDER DEST TO A
        SUB     E               ;SUBRACTING CUR BOF FROM NEW
        MOV     A,H             ;TO SEE WHICH WAY TO MOVE
        SBB     D               ;ANSWER NOW IN CARRY FLAG
        JC      LE760           ;IF MOVING TO LOWER ADDR
;
;MOVING TO HIGHER ADDR
;
        LHLD    EOFP            ;MOVING TO HIGHER ADDR
        MOV     A,L             ;GOING TO SUBTRACT CUR (OLD)
        SUB     E               ;BOFP FROM CUR EOFP & PUT
        MOV     C,A             ;RESULT (FILE LENGTH) IN
        MOV     A,H             ;REG B,C
        SBB     D               ;
        MOV     B,A             ;
        LHLD    BOFP            ;GET DEST BEG OF FILE
        DAD     B               ;ADD LENGTH = NEW END
        XCHG                    ;NEW END IN D,E
        MVI     M,2             ;PUT 02H AT CUR BEG OF FILE
        MOV     C,M             ;AND IN REG C
        LHLD    EOFP            ;GET CUR END OF FILE TO H,L
        XCHG                    ;CUR EOF=D,E & NEW END H,L
        CALL    RMOV            ;DO THE MOVE
        JMP     LE765           ;PUT 02H AT BOF AND FCHK
;
;MOVING LOWER IN MEMORY
;
LE760:  MVI     C,1             ;LMOV WILL USE 01H AS END MARK
        CALL    LMOV            ;DO THE MOVE
LE765:  MOV     M,C             ;PUT TERMINATOR MARK IN FILE
        JMP     FCHK            ;FCHK TO UPDATE 1ST L.B. & EOF
;
;RENUMBER COMMAND
;
RNUM:   CALL    VCHK            ;CHECK FOR REQD. PARAMETERS
        LDA     ABUF+7          ;SEE IF INCREMENT SPECIFIED
        MVI     B,5             ;DEFAULT INCREMENT
        ORA     A               ;SET FLAGS
        JZ      LE785           ;NO INCREMENT GIVEN, USE 5
        LXI     B,ABUF+7        ;POINT AT GIVEN INCREMENT
        CALL    ADEC            ;CONVERT TO DECIMAL
        JC      WHAT            ;IF ERROR
        MOV     A,L             ;GET INCREMENT
        CPI     26              ;MAXIMUM INCREMENT + 1
        JNC     WHAT            ;IF INCREMENT > 25
        MOV     B,A             ;INCREMENT IN B
LE785:  LHLD    BOFP            ;GET BOF POINTER
        SHLD    APNT            ;SAVE
LE78B:  MOV     A,M             ;GET LENGTH BYTE
        CPI     1               ;END OF FILE ?
        JZ      FCHK            ;YES, DONE, DO FCHK
        INX     H               ;POINT 1ST DIGIT
        LXI     D,ABUF          ;STARTING LINE NO. SOURCE
        MVI     C,4             ;LENGTH OF LINE NO.
LE797:  LDAX    D               ;GET A DIGIT
        MOV     M,A             ;PUT IT IN THE LINE
        INX     H               ;POINT TO NEXT DIGIT
        INX     D               ;IN SOURCE, TOO
        DCR     C               ;COUNT OF DIGITS LEFT
        JNZ     LE797           ;DO NEXT DIGITS
        LHLD    APNT            ;GET POINTER TO LINE
        MOV     A,M             ;GET LENGTH BYTE
        CALL    ADR             ;POINT TO NEXT LINE
        SHLD    APNT            ;UPDATE LINE POINTER
        DCX     H               ;POINT TO LAST CHR OF PREV LINE
        MOV     A,M             ;GET IT
        CPI     0DH             ;IS IT A CARRIAGE RET ?
        JNZ     LE718           ;DOING A LITTLE FCHKING HERE
        LXI     H,ABUF+3        ;POINT TO LSB OF LINE NO.
        MOV     A,M             ;GET IT
        ADD     B               ;ADD INCREMENT
LE7B5:  CPI     '9'+1           ;CARRY ?
        JNC     LE7CC           ;NO - CONTINUE
        MOV     M,A             ;PUT UPDATED DIGIT BACK
        LDA     ABUF            ;GET MSB
        CPI     '9'             ;EQUAL 9 ?
        CNC     LE7C9           ;IF SO, CHANGE INCREMENT TO 1
        LHLD    APNT            ;GET POINTER TO NEXT LINE
        JMP     LE78B           ;AND DO IT
;
;CHANGE INCREMENT TO 1 IF NOS. GET TO 9000
;
LE7C9:  MVI     B,1             ;NEW INCREMENT
        RET
;
;GET HERE IF ADDITION GAVE CARRY
;
LE7CC:  MVI     C,0             ;C WILL BECOME DECIMAL CARRY
LE7CE:  INR     C               ;INR FOR EA SUBTR. OF 10
        SUI     10              ;SUB 10 FROM ADDITION RESULT
        CPI     '9'+1           ;RESULT STILL > 9 ?
        JNC     LE7CE           ;SUB AGAIN & KEEP CNT OF SUBTR
        MOV     M,A             ;LESS THAN 10 - PUT DIGIT BACK
        DCX     H               ;POINT TO NEXT MSB
        MOV     A,M             ;GET IT
        ADD     C               ;ADD CARRY FROM PREV. DIGIT
        JMP     LE7B5           ;AND REPEAT THE PROCESS
;
;GENERAL ERROR MESSAGE ROUTINES
;
WHAT:   LXI     H,EMES
MESS:   CALL    CRLF            ;DO CR/LF
        PUSH    H               ;SAVE H,L
        LXI     H,SYSIN         ;RESTORE STD I/O DRIVERS
        CALL    LE15A           ;DO IT
        POP     H               ;RESTORE H,L
        CALL    SCRN            ;PRINT THE MSG POINTED TO BY H,L
        JMP     LE0CB           ;GO BACK TO ALS-8 COMND MODE
;
;MESSAGES
;
EMES:   DB      'WHAT?', 0DH
EMES1:  DB      'FULL', 0DH
EMES2:  DB      'FCON', 0DH
FERR:   DB      ' FILE ERR', 0DH
;
;THIS IS THE PROCESSING PORTION OF THE ENTER COMMAND
;
ENTS:   CALL    CRLF            ;START ON A NEW LINE
        CALL    READ            ;GET A LINE OF INPUT
        LXI     H,IBUF          ;POINT TO IT'S START
        SHLD    PNTR            ;SAVE POINTER
ENT1:   CALL    ZBUF            ;CLEAR ASCII BUFFER
        CALL    SBLK            ;SCAN TO CHARACTERS IN IBUF
        JC      ENTS            ;C/R FOUND, GET ANOTHER LINE
        CPI     '/'             ;ENTER TERMINATION CHAR ?
        RZ                      ;YES - DONE THEN
        CALL    ALPS            ;LOAD ABUF FROM IBUF
        XCHG                    ;
        LXI     B,ABUF          ;POINT TO THE LOADED BUFR
        LDA     SMODE           ;GET THE CURRENT MODE
        ORA     A               ;SET FLAGS
        JNZ     LE837           ;JUMP IF OCTAL
        CALL    AHEX            ;CONVERT ASCII HEX TO BINARY
        JMP     LE83A           ;TO CONTINUE
LE837:  CALL    AOCT            ;CONVERT OCTAL TO BINARY
LE83A:  RC                      ;RETURN IF ERROR
        LDAX    D               ;GET CHAR FROM INPUT LINE
        CPI     ':'             ;NEW ADDRESS OR DATA ?
        JZ      LE84C           ;NEW ADDRESS
        MOV     A,L             ;DATA TO A
        LHLD    BBUF            ;GET ENTER POINTER
        MOV     M,A             ;PUT DATA IN MEMORY
        CALL    ACH1            ;INCREMENT ENTER POINTER
        JMP     ENT1            ;DO NEXT BYTE
LE84C:  SHLD    BBUF            ;SAVE AS NEW ENTER ADDR
        XCHG                    ;IBUF POINTER TO H,L
        INX     H               ;INCREMENT IT
        SHLD    PNTR            ;SAVE IT
        JMP     ENT1            ;DO MORE BYTES
;
;HERE ARE THE ROUTINES THAT ADD OR REPLACE LINES IN FILES
;BASED ON THEIR LINE NUMBERS.  DELETE IS HANDLED BY A
;SEPARATE ROUTINE
;
LINE:   CPI     '0'             ;MAKE SURE 1ST CHAR IS NUMERIC
        JC      WHAT            ;IF IT'S NOT
        LXI     H,3030H         ;TWO ASCII ZEROES
        SHLD    IBUF-4          ;PUT TWO BEFORE THE LINE
        SHLD    IBUF-2          ;TWO MORE MAKES FOUR
        LXI     H,IBUF-1        ;POINT TO SPACE BEFORE LINE
        MVI     C,4             ;MAX NO DIGITS IN LINE NO
LE86A:  INX     H               ;POINT TO 1ST CHAR
        MOV     A,M             ;GET IT
        CPI     '0'             ;NUMERIC ?
        JC      LE87C           ;NO
        CPI     '9'+1           ;NUMERIC ?
        JNC     LE87C           ;NO
        DCR     C               ;YES, NUMERIC
        JNZ     LE86A           ;COUNT NUMERIC CHRS IN LINE NO.
;
;IF WE GET HERE THE LINE HAD A FULL 4-DIGIT NO. TO BEGIN WITH
;NOW MAKE SURE LINE NO. IS FOLLOWED BY AT LEAST ONE SPACE
;PUT ONE IN IF IT'S NOT
;
        INX     H               ;POINT AFTER THE LINE NO
        MOV     A,M             ;GET CHAR
LE87C:  CPI     ' '             ;IS CHAR AFTER LINE NO. SPACE ?
        MOV     B,C             ;COUNT OF LEADING 0'S TO ADD
        DCX     H               ;POINT TO LAST DIGIT OF LINE NO
        JZ      LE893           ;IF LINE NO FOLLOWED BY SPACE
        INR     C               ;INR COUNT OF TOTAL CHRS TO ADD
        MOV     B,C             ;SAVE IN B
        MVI     C,4             ;NEED TO MOVE 4 DIGIT LINE NO.
        MVI     A,' '           ;TO INSERT A SPACE
LE889:  MOV     D,M             ;GET CHAR
        MOV     M,A             ;PUT CHAR IN A INTO LINE
        MOV     A,D             ;CHAR REMOVED FROM LINE TO A
        DCX     H               ;DECREMENT LINE POINTER
        DCR     C               ;AND COUNT OF CHRS LEFT TO MOVE
        JNZ     LE889           ;IF NOT DONE
        MOV     M,A             ;REPLACE LAST CHARACTER
        MOV     C,B             ;COUNT OF 0'S TO INSERT
LE893:  LXI     H,IBUF          ;POINT TO IBUFFER
LE896:  DCX     H               ;DECREMENT LINE POINTER
        DCR     C               ;DCR COUNT OF 0'S TO INSERT
        JP      LE896           ;LOOP TILL IT GOES NEGATIVE
        SHLD    SAVL            ;H,L IS NEW START OF LINE
        LDA     CCNT            ;GET OLD LINE LENGTH COUNT
        ADD     B               ;ADD COUNT OF CHARACTERS ADDED
        STA     CCNT            ;UPDATED COUNT
        CPI     7               ;7 ==> ORIG LINE WAS BLANK
        JZ      LE92B           ;TO DELETE THE LINE
        MOV     M,A             ;PUT LEN BYTE AT HEAD OF LINE
        LXI     D,4             ;LENGTH OF LINE NO.
        DAD     D               ;H,L POINTS LAST CHR OF LINE NO
        SHLD    ADDS            ;SAVE ADDR.
        LXI     D,MAXL+3        ;END OF HIGHEST LIN # IN FILE
        CALL    COM0            ;COMPARE LINE NO. TO MAXL
        JNC     INSRT           ;LINE GOES INTO MIDDLE OF FILE
;
;LINE GOES AT END OF FILE
;
        INX     H               ;POINT TO LINE NO.
        CALL    LODM            ;GET IT IN BCDE
        LXI     H,MAXL+3        ;POINT TO END OF MAXL
        CALL    STOM            ;THIS LINE BECOMES NEW MAXL
        LHLD    SAVL            ;GET ADDR OF LINE
        XCHG                    ;D,E IS LINE POINTER
        LHLD    EOFP            ;H,L IS CURRENT END OF FILE
        MVI     C,1             ;1 IS THE MOVE TERMINATOR CHAR
        CALL    LMOV            ;MOVE LINE TO END OF FILE
        MOV     M,C             ;PUT EOF MARK IN FILE
        SHLD    EOFP            ;UPDATE END OF FILE
        JMP     EORNS           ;DONE
;
;GET HERE TO INSERT A LINE INTO FILE
;
INSRT:  CALL    LE947           ;FIND LINE NO IN FILE
        MVI     C,2             ;FLAG INITIALIZATION
        JZ      EQUL            ;LINE IN FILE HAS SAME NO.
        DCR     C               ;C=1 ==> NO. NOT NOW IN FILE
EQUL:   MOV     B,M             ;GET LENGTH OF LINE IN FILE
        DCX     H               ;ADDR TO INSERT LINE AFTER
        MVI     M,2             ;PUT MARKER THERE
        SHLD    APNT            ;AND SAVE AS POINTER
        LDA     CCNT            ;LENGTH OF LINE TO ADD
        DCR     C               ;THE FLAG - 2 = REPLACEMENT
        JZ      LT              ;IF NEW LINE # NOT = OLD LINE #
        SUB     B               ;COMPUTE DIFF IN LINE LENGTHS
        JZ      ZERO            ;IF LINES ARE SAME LENGTH
        JC      GT              ;IF NEW LINE > OLD LINE
;
;GET HERE IF OLD LINE > NEW LINE OR DOING
;ADDITION RATHER THAN REPLACEMENT
;
LT:     LHLD    EOFP            ;GET OLD END OF FILE
        MOV     D,H             ;DUPLICATE IN D,E
        MOV     E,L             ;
        CALL    ADR             ;ADD DIFF. TO OLD EOFP
        SHLD    EOFP            ;OLD + ADDED = NEW
        MVI     C,2             ;MOVE TERMINATION MARK IN FILE
        CALL    RMOV            ;DO THE MOVE
        JMP     ZERO            ;INSERT LINE INTO GAP CREATED
;
;GET HERE IF OLD LINE < NEW LINE
;
GT:     CMA                     ;MAKE DIFFERENCE NEGATIVE
        INR     A               ;TWO'S COMPLEMENT
        MOV     D,H             ;DUPLICATE H,L IN D,E
        MOV     E,L             ;
        CALL    ADR             ;SUB. DIFF FROM OLD EOFP
        XCHG                    ;
        CALL    LMOV            ;DELETE EXCESS CHAR IN FILE
        MVI     M,1             ;E-O-F INDICATOR
        SHLD    EOFP            ;E-O-F ADDRESS
;
;GET HERE TO INSERT CURRENT LINE INTO FILE AREA
;
ZERO:   LHLD    SAVL            ;LINE ADDRESS
        XCHG                    ;TO D,E
        LHLD    APNT            ;INSERT ADDRESS
        MVI     M,0DH           ;REPLACE MOVE TERM. MARK W/CR
        INX     H               ;POINT TO WHERE NEW LINE GOES
        MVI     C,1             ;MOVE TERMINATION MARK
        CALL    LMOV            ;PUT LINE INTO FILE
        JMP     EORNS           ;DONE
;
;DELETE LINE IF LINE NO. ENTERED WITH NO TEXT
;
LE92B:  INX     H               ;POINT TO LINE NO TO DELETE
        CALL    LODM            ;GET IT IN B,C,D,E
        LXI     H,ABUF+3        ;DESTINATION ADDRESS
        CALL    STOM            ;STORE DELETE ADDRESS
        LXI     H,ABUF+10       ;ADDRESS OF END OF 2ND PARM
        CALL    STOM            ;STORE LINE NO. AGAIN
        CALL    LEA2F           ;MAKE LIKE A DELT N,N COMMAND
        JMP     EORNS           ;LINE DELETED, DONE
;
;FIND - SEARCH FILE FOR SPECIFIED LINE
;
FIND:   LXI     H,ABUF+3        ;ADDRS. OF TEMP BUFFER
FIND1:  SHLD    ADDS            ;ADDR OF NO. TO LOOK UP
LE947:  LHLD    BOFP            ;START OF FILE TO SEARCH
FI1:    CALL    E01             ;SEE IF AT END OF FILE
        XCHG                    ;FILE ADR TO D,E
        LHLD    ADDS            ;GET ADDR OF NO TO LOOK UP
        XCHG                    ;SET UP
        MVI     A,4             ;LENGTH OF LINE NO.
        CALL    ADR             ;POINT TO END OF LINE NO.
        CALL    COM0            ;SEE IF LINE NOS. ARE SAME
        RC                      ;NO IN FILE > SUBJECT
        RZ                      ;NOS. ARE THE SAME
FI2:    MOV     A,M             ;GET LENGTH BYTE
        CALL    ADR             ;POINT TO NEXT LINE
        JMP     FI1             ;TEST NEXT LINE
;
;ROUTINE TO CHECK FOR END OF FILE
;
EOF:    INX     H               ;
E01:    MVI     A,1             ;EOF INDICATOR
        CMP     M               ;SAME AS CHAR POINTED TO ?
        RNZ                     ;NO - RETURN
        JMP     EORNS           ;YES - ABORT COMMAND
;
;ROUTINE TO ADD ONE BYTE NO IN A TO ADDR IN H,L
;
ADR:    ADD     L               ;ADD A TO L
        MOV     L,A             ;RESULT REPLACES L
        RNC                     ;DONE IF NO CARRY
        INR     H               ;ELSE INCREMENT H
        RET                     ;DONE NOW
;
;CHARACTER MOVEMENT ROUTINE TO MOVE FROM D,E TO H,L
;INCREMENTING D,E (START AT LOW ADDRESS END OF BLOCK TO MOVE)
;MOVE ENDS WHEN CHAR IN REG C IS ENCOUNTERED.
;
LMOV:   LDAX    D               ;GET SOURCE CHAR
        INX     D               ;INR SOURCE POINTER
        CMP     C               ;TERMINATION CHAR ?
        RZ                      ;YES - DONE THEN
        MOV     M,A             ;NO - MOVE TO DEST.
        INX     H               ;INR DESTINATION POINTER
        JMP     LMOV            ;DO NEXT CHAR
;
;CHARACTER MOVEMENT ROUTINE TO MOVE FROM D,E TO H,L
;DECREMENTING POINTERS (START AT HIGH END OF BLOCK)
;MOVE ENDS WHEN CHAR IN REG C IS ENCOUNTERED
;
RMOV:   LDAX    D               ;GET SOURCE CHAR
        DCX     D               ;DECR SOURCE POINTER
        CMP     C               ;TERMINATION CHAR ?
        RZ                      ;YES - DONE
        MOV     M,A             ;NO - STORE CHAR AT DEST.
        DCX     H               ;DECREMENT DEST. POINTER
        JMP     RMOV            ;DO NEXT CHARACTER
;
;LOAD FOUR CHARS FROM MEMORY INTO REGS B,C,D,E
;
LODM:   MOV     B,M
        INX     H
        MOV     C,M
        INX     H
        MOV     D,M
        INX     H
        MOV     E,M
        RET
;
;STORE FOUR CHARS FROM REGS B,C,D,E INTO MEMORY
;
STOM:   MOV     M,E
        DCX     H
        MOV     M,D
        DCX     H
        MOV     M,C
        DCX     H
        MOV     M,B
        RET
;
;ROUTINE TO COMPARE TWO CHAR STRINGS OF LENGTH 4.
;ZERO FLAG SET ==> STRINGS EQUAL. CARRY ==> STRING
;ADDRESSED BY D,E GREATER THAN OR = STRING ADDR BY
;H,L
;
COM0:   MVI     C,4             ;LENGTH OF COMPARISON
COM1:   MVI     B,1             ;ASSUME EQUAL
        ORA     A               ;CLEAR CARRY
C01:    LDAX    D               ;FETCH CHARACTER
        SBB     M               ;SET FLAGS & COMPARE
        JZ      C02             ;THESE BYTES ARE THE SAME
        INR     B               ;MAKE EQUAL FLAG = 2
C02:    DCX     D               ;DECREMENT 1 STR. POINTER
        DCX     H               ;DECR. OTHER POINTER
        DCR     C               ;DECR. CNT OF BYTES TO COMPARE
        JNZ     C01             ;DO NEXT BYTE
        DCR     B               ;DECREMENT ZERO FLAG
        RET                     ;DONE W/ZERO SET
;
;ROUTINE TO TAKE ASCII CHARS AND ADD LEADING ASCII
;ZEROS TO YIELD A 4 CHARACTER ASCII VALUE
;
NORM:   CALL    LODM            ;GET 4 DIGITS
        XRA     A               ;GET A BINARY ZERO
        CMP     B               ;ARE ALL DIGITS ZERO ?
        RZ                      ;YES - NOTHING TO NORMALIZE
NOR1:   CMP     E               ;NORMALIZED ?
        CNZ     STOM            ;YES, PUT DIGITS BACK
        RNZ                     ;AND RETURN
        MOV     E,D             ;ELSE MOVE DIGITS UP
        MOV     D,C
        MOV     C,B
        MVI     B,'0'           ;AND ADD A ZERO
        JMP     NOR1            ;THEN TEST FOR NORMALIZATION
;
;ROUTINE TO MOVE A LINE FROM THE FILE BUFFER INTO 
;THE ASSEMBLERS LINE BUFFER
;
MOVEL:  LHLD    APNT            ;GET ASSEMBLER POINTER
        MOV     A,M             ;GET 1ST CHAR OF LINE
        CPI     1               ;END OF FILE ?
        RZ                      ;YES, RETURN
        XCHG                    ;LINE ADDR TO D,E
        MOV     L,A             ;GET LENGTH AS 16 BITS IN H,L
        MVI     H,0             ;LENGTH < 128
        DAD     D               ;ADD LENGTH TO START ADDR
        DCX     H               ;POINT TO LAST CHAR OF LINE
        MOV     A,M             ;GET IT
        CPI     0DH             ;C/R ?
        JNZ     LE718           ;REPORT FCHK ERROR
        INX     D               ;POINT TO 1ST ACTUAL CHAR
        LXI     H,IBUF-5        ;ASSEMBLERS BUFFER
        MVI     C,0DH           ;TERMINATION CHAR
        CALL    LMOV            ;MOVE THE LINE
        MOV     M,C             ;PUT C/R AT END OF LINE
        XCHG                    ;1ST CHAR OF NEXT LINE TO H,L
        SHLD    APNT            ;SAVE UPDATED ASSEM. POINTER
        ORA     A               ;CLEAR FLAGS
        RET                     ;DONE
;
;LIST AND TEXT (LIST W/O LINE NUMBERS) COMMANDS
;
LIST:   CMA                     ;MAKE A NON-ZERO
TEXT:   STA     NOLIN           ;SAVE AS FORMAT FLAG
        CALL    CRLF            ;START ON FRESH LINE
        LXI     H,ABUF          ;GET STARTING LINE NO.
        CALL    LODM            ;
        XRA     A               ;ZERO FLAGS & ACCUM.
        CMP     B               ;LINE NO. GIVEN ?
        JNZ     LE9EF           ;JUMP IF LINE NO. GIVEN
        MVI     B,0FFH          ;MAKE LINE NO. > 9999
LE9EF:  LXI     H,ABUF+10
        CMP     M               ;ENDING LINE NO. GIVEN ?
        JNZ     LE9F9           ;IF ENDING NO. GIVEN
        CALL    STOM            ;STORE ENDING LINE NO > 9999
LE9F9:  CALL    FIND            ;FIND STARTING LINE NO.
        SHLD    APNT            ;SAVE AS POINTER
LE9FF:  CALL    MOVEL           ;MOVE LINE TO OUTPUT BUFFER
        JZ      EOR             ;IF END OF FILE FOUND
        LXI     H,IBUF-2        ;POINT TO LAST DIGIT OF LINE NO
        LXI     D,ABUF+10       ;D POINTS TO ENDING LINE NO.
        CALL    COM0            ;COMPARE LINE NO. & END LIN NO.
        RC                      ;RETURN IF > ENDING LINE NO
        LDA     LFMT            ;GET FORMAT FLAG
        ORA     A               ;SET FLAGS
        CNZ     LF361           ;TO FORMAT LINE
        LXI     H,IBUF-5        ;POINT TO START OF LINE
        LDA     NOLIN           ;GET LINE NO. FLAG
        ORA     A               ;SET FLAGS
        JNZ     LEA23           ;PRINT WITH LINE NO.
        LXI     H,IBUF          ;POINT PAST LIN NO.
LEA23:  CALL    SCRN            ;PRINT LINE FROM H,L POINTER
        CALL    CRLF            ;PUT CR/LF AFTER LINE
        JMP     LE9FF           ;TO DO NEXT LINE
;
;DELT - THE DELETE LINE COMMAND
;
DELT:   CALL    VCHK            ;CHECK FOR REQD PARAMETERS
LEA2F:  CALL    FIND            ;GET STARTING DELT ADDR
        SHLD    DELP            ;SAVE IT
        LXI     H,ABUF+10       ;SEE IF 2ND PARM GIVEN
        MOV     A,M             ;GET CHAR
        ORA     A               ;SET FLAGS
        JNZ     LEA40           ;IF 2ND PARM GIVEN
        LXI     H,ABUF+3        ;FIRST & LAST LINE ARE SAME
LEA40:  SHLD    ADDS            ;SAVE 1ST LINE FIND ADDR
        XCHG
        LXI     H,MAXL+3        ;HIGHEST LINE NO.
        CALL    COM0            ;COMPARE TO ENDING DELT LINE
        LHLD    DELP            ;GET DELT POSITION TO H,L
        JC      NOVR            ;DELETE DOES NOT INVOLVE END
;
;GET HERE IF DELETE INVOLVES END OF FILE
;
;THIS GETS COMPLICATED BECAUSE:
;  A) THE LAST LINE'S C/R IS NOT FOLLOWED BY ANY LINE NO.
;                      AND
;  B) IN GENERAL, A 0DH MIGHT BE EITHER A LENGTH BYTE OR C/R
;
LEA50:  SHLD    EOFP            ;DELT POS. ==> NEW EOF
        MVI     M,1             ;PUT NEW EOF MARK IN FILE
        MOV     B,M             ;FLAG WHICH ISN'T REALLY USED
        XCHG                    ;D,E = EOF
        LHLD    BOFP            ;H,L = BOF
        XCHG                    ;D,E = BOF & H,L = EOF
        DCX     H               ;SO WE MISS LAST LINE'S C/R
DEL2:   MOV     A,L             ;SUBTRACT BOF FROM EOF
        SUB     E               ;
        MOV     A,H             ;
        SBB     D               ;
        MVI     A,0DH           ;SETTING UP FOR COMPARE
        JC      DEL4            ;NO 0DH IN FILE==> MAXL=0
        DCX     H               ;MOVE UP 1 CHAR
        CMP     M               ;0DH ?
        JNZ     DEL2            ;NO, THEN TRY NEXT CHAR
        DCX     H               ;YES - SEE IF TWO IN A ROW
        MOV     A,L             ;SUBTRACTING BOF FROM POINTER
        SUB     E
        MOV     A,H             ;
        SBB     D               ;
        JC      DEL4            ;0DH WAS 1ST CHAR==>LENGTH BYTE
        MVI     A,0DH           ;COMPARE THIS CHAR TO 0DH
        CMP     M               ;
        INX     H               ;POINT TO NEXT CHAR
        INX     H               ;POINT TO NEXT CHAR
        JZ      DEL3            ;WERE TWO 0DH'S, AT LINE # NOW
LEA7A:  INX     H               ;ONLY 1 0DH, 1 MORE==>AT LINE #
DEL3:   CALL    LODM            ;GET LINE NO IN REGS
        LXI     H,MAXL+3        ;WHERE IT GOES
        CALL    STOM            ;PUT IT THERE
        RET                     ;MAXL IS UPDATED
DEL4:   XCHG                    ;FOR PROPER RETURN
        DCR     B               ;ALWAYS 1 ==> 0
        JNZ     LEA7A           ;NEVER TAKE JUMP
        STA     MAXL            ;MAKES MAXL VERY SMALL ALWAYS
        RET                     ;DONE, EDITOR THINKS FILE EMPTY
;
;GET HERE IF DELETE IS IN THE MIDDLE OF THE FILE
;
NOVR:   CALL    FI1             ;FIND END OF DELETE AREA
        CZ      FI2             ;NEXT LINE IF LINE NOS. EQUL
        XCHG                    ;
        LHLD    DELP            ;DEST = START OF DELT AREA
        MVI     C,1             ;TERM ON EOF MARK
        CALL    LMOV            ;DO THE MOVE TO DELETE
        SHLD    EOFP            ;UPDATE EOF ADDRESS
        MVI     M,1             ;PUT EOF MARK IN FILE
        RET                     ;DONE
;
;STAB COMMAND - SET SYMBOL TABLE ADDRESS
;
STAB:   CALL    VCHK            ;CHECK FOR REQD PARM
        LHLD    BBUF            ;GET IT
        SHLD    SYMADD          ;SAVE AS SYMBOL TABLE ADDRESS
        JMP     EORMS           ;DONE
;
; ***** END OF ALS8COM MODULE
;
;
;THE ASSEMBLER STARTS HERE
;
;ASPC IS THE PROGRAM COUNTER (GOES ON LISTING), AND
;BBUF+2 IS THE STORAGE COUNTER (WHERE OBJECT CODE GOES)
;
ASSM:   CALL    VCHK            ;AT LEAST 1 PARM. REQD
        LDA     ABUF+7          ;2ND PARM. (OFFSET) GIVEN ?
        ORA     A               ;SET FLAGS
        JNZ     ASM4            ;USE 2ND PARM. IF GIVEN
        LHLD    BBUF            ;ELSE GET 1ST PARM
        SHLD    BBUF+2          ;USE 1ST PARM. AS STORAGE CNTR
ASM4:   LDA     IBUF+4          ;GET LETTER AFTER ASSM OR ASSI
        CPI     'E'             ;'E' - OMIT NON-ERROR LINES ?
        JZ      LEAD6           ;SKIP CHECKING FOR 'S' OR 'X'
        CPI     'S'             ;'S' - LIST SYMBOL TABLE ?
        JZ      LEAD6           ;SKIP CHECKING FOR 'X'
        CPI     'X'             ;'X' - CROSS REFERENCE ?
        JZ      LEAD6           ;HAVE PRINT FLAG
        CPI     ' '             ;MUST BE SPACE, E, S, OR X
        JNZ     WHAT            ;WHOOPS - ILLEAGLE CHAR.
LEAD6:  STA     SYMX            ;SAVE SYMBOL/PRINT FLAG
        LDA     IBUF+3          ;GET ASSEMBLY TYPE (ASSM/ASSI)
        STA     ASMTY           ;SAVE AS ASSEMBLY TYPE
        XRA     A               ;GET A ZERO
        LHLD    SYMADD          ;POINT TO SYMBOL TABLE START
        MOV     M,A             ;PUT IN END OF TABLE MARK
        STA     ALST            ;ASSUME LIST TURNED 'ON'
        STA     PASI            ;SET PASS INDICATOR TO PASS 1
LEAEA:  LHLD    BBUF            ;GET PGM. ORG
        SHLD    ASPC            ;USE IT TO INIT. PROG. COUNTER
        LHLD    BOFP            ;GET START OF SOURCE
        SHLD    APNT            ;USE IT TO INIT. ASS. POINTER
;
;THIS IS THE START OF THE MAIN ASSEMBLER PROCESSING LOOP
;
LEAF6:  LXI     SP,SMODE
        LXI     H,OBUF          ;POINT TO OUTPUT BUFFER
        MVI     A,IBUF AND 0FFH ;LOW BYTE OF END OF OBUF
        CALL    CLER            ;CLEAR OBUF TO SPACES
        LDA     ASMTY           ;GET ASSM/ASSI FLAG
        CPI     'I'             ;USING EXTERNAL INPUT ?
        JZ      LEB12           ;YES - CALL IT
        CALL    MOVEL           ;ELSE MOVEL PUTS LINE IN IBUF
        JZ      LEE39           ;IF END OF FILE DETECTED
        JMP     LEB15           ;SKIP CALL TO EXT. INPUT
LEB12:  CALL    IN8             ;CALL ASSI DRIVER FOR EXT INPUT
LEB15:  LDA     PASI            ;GET PASS INDICATOR
        ORA     A               ;SET FLAGS
        JNZ     ASM2            ;DO THE 2ND PASS
        CALL    PAS1            ;ELSE DO PASS 1
        JMP     LEAF6           ;THEN BACK TO TOP OF LOOP
ASM2:   CALL    PAS2            ;PROCESS PASS 2 LINE
        LXI     H,OBUF          ;POINT TO OUTPUT BUFFER
        CALL    AOUT            ;PRINT LINE
        JMP     LEAF6           ;THEN BACK TO TOP OF LOOP
;
;END OF MAIN LOOP
;
;FOLLOWING ROUTINE PRINTS LINE DURING 2ND PASS
;
AOUT:   LDA     OBUF+18         ;GET ERROR TYPE IN OUTPUT LINE
        CPI     ' '             ;WAS THERE AN ERROR ?
        JNZ     LEB41           ;YES - PRINT LINE
        LDA     AERR            ;GET SYMBOL/ERROR PRINT FLAG
        CPI     'E'             ;ERRORS-ONLY LISTING ?
        RZ                      ;YES - DONE WITH THIS LINE
        LDA     ALST            ;GET ASSEMBLER LIST FLAG
        ORA     A               ;SET FLAGS
        RNZ                     ;DONE IF LISTING TURNED OFF
LEB41:  LDA     PASI            ;GET PASS INDICATOR
        CPI     2               ;GREATER THAN 1 (E.G. 3RD PASS)
        RNC                     ;DONE THEN
        LDA     LFMT            ;GET FORMAT/NO FORMAT FLAG
        ORA     A               ;SET FLAGS
        CNZ     LF361           ;FORMAT LINE IF NECESSARY
        LXI     H,OBUF          ;POINT TO LINE TO OUTPUT
        CALL    CRLF            ;DO CARRIAGE RET FROM PREV LINE
        JMP     SCRN            ;PRINT LINE & RET.
;
;PASS 1 OF THE ASSEMBLER USED TO FORM SYMBOL TABLE
;
PAS1:   CALL    ZBUF            ;CLEAR OUT THE ASCII BUFFER
        STA     PASI            ;PUT PASS INDICATOR BACK
        LXI     H,IBUF          ;POINT TO 1ST CHAR OF LINE
        SHLD    PNTR            ;SAVE AS POINTER INTO LINE
        MOV     A,M             ;GET 1ST CHAR
        CPI     ' '             ;BLANK ?
        JZ      OPC             ;NO LABEL THEN
        CPI     COMCHR          ;COMMENT CHARACTER ?
        RZ                      ;DONE IF COMMENT LINE
;
;PROCESS LABEL POINTED TO IN LINE BY H,L
;
        CALL    SLAB            ;SEE IF LABEL ALREADY IN TABLE
        JC      LF0E0           ;ERROR IN LABEL
        JZ      ERRD            ;DUPLICATE LABEL
        CALL    LCHK            ;CHECK CHAR AFTER LABEL
        JC      LF0E0           ;IF NOT SPACE OR COLON
        CALL    LEBA5           ;TO MOVE LABEL INTO TABLE
        XCHG                    ;SYM. TABLE ADDR TO H,L
        SHLD    TABA            ;SAVE AS SYMBOL TABLE PNTR
        LDA     ASPC+1          ;PUTTING P.C. IN SYMBOL TABLE
        MOV     M,A             ;PUT 1ST BYTE IN TABLE
        INX     H               ;INR. TABLE POINTER
        LDA     ASPC            ;GET 2ND BYTE
        MOV     M,A             ;PUT IT IN TABLE
        INX     H               ;POINT PAST VALUE
        MVI     M,0             ;END OF TABLE MARK
;
;PROCESS OPCODE
;
OPC:    CALL    ZBUF            ;BLANK OUT ABUF
        CALL    SBLK            ;SCAN TO OPCODE
        JC      OERR            ;IF FOUND C/R INSTEAD
        CALL    ALPS            ;MOVE OPCODE INTO ABUF
        CPI     ' '             ;CHECK FOR SPACE AFTER OPCODE
        JC      OPCD            ;OPCODE FOLLOWED BY C/R
        JNZ     OERR            ;OPCODE FOLLOWED BY CHAR.
        JMP     OPCD            ;TO CONTINUE OPCODE PROC.
;
;ROUTINE TO MOVE LABEL INTO SYMBOL TABLE
;
LEBA5:  MVI     C,LLAB          ;LENGTH OF LABEL
        LXI     H,ABUF          ;SOURCE
MLAB:   MOV     A,M             ;GET CHAR FROM LABEL
        STAX    D               ;PUT IT IN TABLE
        INX     D               ;DEST. POINTER
        INX     H               ;SOURCE POINTER
        DCR     C               ;LENGTH POINTER
        JNZ     MLAB            ;IF MORE CHARS TO MOVE
        RET                     ;ELSE DONE
;
;CHECK LABELS FOR VALID TERMINATOR
;
LCHK:   LHLD    PNTR            ;GET POINTER
        MOV     A,M             ;GET CHAR
        CPI     ' '             ;SPACE ?
        RZ                      ;OK THEN, RETURN
        CPI     ':'             ;COLON ?
        RNZ                     ;IF NOT, RETURN ZERO FLAG OFF
        INX     H               ;ELSE OK, INR. PAST NON-SPACE
        SHLD    PNTR            ;AND UPDATE POINTER
        RET                     ;THEN RETURN
;
;PROCESS PSEUDO OPS IN PASS 1
;H,L POINTS INTO LINE, D,E POINTS INTO OPCODE TABLE
;
PSU1:   CALL    SBLK            ;SCAN TO OPERAND
        LDAX    D               ;FETCH TABLE VALUE
        ORA     A               ;SET FLAGS
        JZ      ORG1            ;ORG PSEUDO OP
        JM      DAT2A           ;DB PSEUDO OP
        CPI     2               ;
        JC      EQU1            ;EQU PSEUDO OP
        JZ      RESI            ;DS PSEUDO OP
        CPI     8               ;
        JZ      LEC26           ;ASC PSEUDO OP
        CPI     5               ;
        RNC                     ;PSEUDO-OPS NOT USED 1ST PASS
        JPE     LEE39           ;END PSEUDO OP
;
;DO DW PSEUDO OP
;
ACO1:   MVI     C,2             ;DW IS A 2-BYTE INSTR.
        XRA     A               ;CLEAR CARRY & FLAGS
        JMP     OCN1            ;TO INCREMENT PGM. COUNTER
;
;DO ORG PSEUDO OP
;
ORG1:   CALL    ASCN            ;GET OPERAND TO H,L
        LDA     OBUF+18         ;GET ERROR INDICATOR
        CPI     ' '             ;ANY ERRORS IN THIS LINE ?
        RNZ                     ;IF SO, DON'T PROCESS
        SHLD    ASPC            ;ELSE PUT VALUE IN PGM. CTR.
        LDA     IBUF            ;GET 1ST CHAR OF LINE
        CPI     ' '             ;SPACE OR LABEL CHAR ?
        RZ                      ;DONE IF SPACE
        JMP     EQUS            ;ELSE CHANGE LABEL VALUE
;
;DO EQU PSEUDO OP
;
EQU1:   CALL    ASCN            ;GET VALUE TO H,L
        LDA     IBUF            ;POINT AT 1ST CHAR OF LINE
        CPI     ' '             ;SPACE OR LABLE CHAR ?
        JZ      ERRM            ;EQU REQUIRES A LABEL ALWAYS
EQUS:   XCHG                    ;VALUE TO D,E
        LHLD    TABA            ;TABLE ADDR TO H,L
        MOV     M,D             ;STORE LABEL VALUE IN S. TABLE
        INX     H               ;2ND BYTE
        MOV     M,E             ;
        LDA     OBUF+18         ;GET ERROR FLAG
        CPI     ' '             ;LINE O.K. ?
        RZ                      ;DONE THEN
        JMP     LEB41           ;ELSE PRINT THE LINE
;
;DO DS PSEUDO OP
;
RESI:   CALL    ASCN            ;GET OPERAND VALUE TO H,L
        MOV     B,H             ;MOVING TO B,C
        MOV     C,L             ;
        LDA     OBUF+18         ;GET ERROR INDICATOR
        CPI     ' '             ;LINE O.K. ?
        JNZ     LEB41           ;PRINT IT IF NOT
        JMP     RES21           ;ELSE ADD B,C TO PGM. CTR.
;
;DO ASC PSEUDO OP
;
LEC26:  MOV     A,M             ;GET 1ST NON-SPACE CHAR
        CPI     0DH             ;CARRIAGE RETN ?
        RZ                      ;DONE
        MOV     D,A             ;SAVE DELIMITER IN D
        LXI     B,0             ;INITIALIZE CHAR COUNT TO 0
LEC2E:  INX     H               ;POINT TO NEXT CHAR
        MOV     A,M             ;GET IT
        CPI     0DH             ;END OF LINE ?
        JZ      RES21           ;DONE WITH STRING
        CMP     D               ;2ND DELIMETER ?
        JZ      RES21           ;DONE IF SO
        INX     B               ;ELSE INR. CHAR COUNT
        JMP     LEC2E           ;AND KEEP ON COUNTING
;
;PERFORM PASS 2 OF THE ASSEMBLER
;
PAS2:   LXI     H,OBUF          ;SET OUTPUT BUFFER ADDRESS
        LDA     ASPC+1          ;GET HIGH BYTE OF PGM. CTR.
        CALL    BINH+3          ;CONVERT TO ASCII & STORE
        INX     H               ;POINT PAST BYTE
        LDA     ASPC            ;GET LOW ORDER BYTE
        CALL    BINH+3          ;PUT IT IN ADDR FIELD, TOO
        SHLD    OIND            ;SAVE H,L AS OUTPUT POINTER
        CALL    ZBUF            ;CLEAR ABUFF
        LXI     H,IBUF          ;POINT TO LINE TO ASSEMBLE
PABL:   SHLD    PNTR            ;SAVE AS POINTER
        MOV     A,M             ;GET 1ST CHAR OF LINE
        CPI     ' '             ;SPACE ?
        JZ      OPC             ;YES, NO LABEL, PROC. OPCODE
        CPI     COMCHR          ;IS IT A COMMENT LINE ?
        RZ                      ;DONE THEN
        CALL    SLAB            ;LOOK UP LABEL IN TABLE
        JC      LF0E0           ;LABEL ERROR
        CALL    LCHK            ;CHECK TERM. CHAR
        JNZ     LF0E0           ;NOT FOLLOWED BY '$' OR ':'
        JMP     OPC             ;PROCESS THE OPCODE
;
;PROCESS PSEUDO OPS FOR THE 2ND PASS
;
PSU2:   LDAX    D               ;GET TABLE VALUE
        ORA     A               ;SET FLAGS
        JZ      ORG2            ;ORG PSEUDO OP
        JM      DAT2            ;DB PSEUDO OP
        CPI     2
        RC                      ;EQU WAS PROCESSED ON 1ST PASS
        JZ      RES2            ;DS PSEUDO OP
        CPI     8
        JZ      LED01           ;ASC PSEUDO OP
        CPI     5
        JZ      LECD3           ;COM PSEUDO OP
        JNC     LECCD           ;BOTH LST AND NLST
        JPE     LEE39           ;END PSEUDO OP
;
;DO DW PSEUDO OP
;
ACO2:   CALL    TYS6            ;GET VALUE & PUT IN MEM
        JMP     ACO1            ;INR P.C. & RET
;
;DO DS PSEUDO OP IN PASS 2
;
RES2:   CALL    ASBL            ;GET VALUE IN H,L
        MOV     B,H             ;MOVING TO B,C
        MOV     C,L
        LHLD    BBUF+2          ;GET STORAGE COUNTER
        DAD     B               ;ADD NO BYTES IN DS STMNT
        SHLD    BBUF+2          ;UPDATE STORAGE COUNTER
RES21:  XRA     A               ;CLEAR ZERO FLAG
        JMP     OCN2            ;TO UPDATE PGM. COUNTER
;
;DO PASS 2 DB PSEUDO OP
;
DAT2:   CALL    TYS5            ;GET OPERNAD
DAT2A:  XRA     A               ;MAKE ACC. ZERO
        MVI     C,1             ;BYTE COUNT
        JMP     OCN1            ;TO INR. PGM. COUNTER
;
;DO ORG PSEUDO OP
;
;P.T. SCREWED THIS ONE UP, AS THEY STORE THE NEW ORG NOT ONLY
;AS THE ASSEMBLERS PROGRAM COUNTER, BUT ALSO AS THE STORAGE
;COUNTER, WHICH POINTS TO WHERE THEY ARE STORING THE OBJECT
;CODE. THUS IF YOU WERE USING AN OFFSET (E.G. ASSM XXXX YYYY)
;BEFORE THE ORG STATEMENT, YOU WON'T BE AFTERWARDS.  WHAT'S
;SURPRISING IS THAT IMSAI'S SCS-1, WRITTEN BY THE SAME CO.
;THAT WROTE ALS-8 FOR P.T. (MICRO-TEC OF SUNNYVALE CALIF.),
;HAS THIS FIXED, EVEN THOUGH IT WAS AN EARLIER PRODUCT.
;
ORG2:   CALL    ASBL            ;GET ORG VALUE
        LDA     OBUF+18         ;FETCH ERROR FLAG
        CPI     ' '             ;LINE O.K. ?
        RNZ                     ;IGNORA IF NOT
        XCHG                    ;VALUE TO D,E
        LHLD    ASPC            ;GET CUR. PGM COUNTER
        XCHG                    ;NEW ORG H,L - PC IN D,E
        SHLD    ASPC            ;MAKE ORG NEW P.C.
        SHLD    BBUF+2          ;AND MAKE IT STORAGE CNTR ALSO
        RET                     ;DONE, AND YOU ARE SCREWED
;
;PROCESS THE $ SYMBOL (CUR. VALUE OF PGM. COUNTER)
;
LECC3:  INX     H               ;INCREMENT POINTER
        SHLD    PNTR            ;SAVE POINTER
        LHLD    ASPC            ;GET CUR. PGM. COUNTER
        JMP     AVAL            ;TO FINISH EXPR. EVALUATION
;
;PROCESS NLST AND LST PSEUDO OPS.
;
LECCD:  SUI     6               ;CONVERT LST/NLST TO 0 OR 1
        STA     ALST            ;SAVE AP LIST FLAG
        RET                     ;DONE
;
;PROCESS COM PSEUDO OP
;
LECD3:  CALL    ZBUF            ;CLEAR ABUF
        CALL    SBLK            ;SCAN TO LABEL TO ENTER
        CALL    SLAB            ;LOOKUP LABEL, VALUE IN H,L
        JC      ERRA            ;IF ERROR IN LABEL
        JNZ     ERRU            ;IF LABEL NOT FOUND
LECE2:  LXI     D,SYSYM         ;SYSTEM SYMBOL TABLE
LECE5:  SHLD    OPRD            ;SAVE VALUE IN OPRD
        MVI     B,LLAB          ;LENGTH OF LABELS
        CALL    COMS            ;SEE IF ALREADY IN SYSTEM TABLE
        JZ      LECF9           ;ALREADY IN TABLE, CHG VALUE
        CALL    LEBA5           ;NOT IN TABLE, ADD IT
        INX     D               ;POINT PAST (UNDEFINED) VALUE
        INX     D               ;2ND BYTE
        XRA     A               ;GET BINARY ZERO
        STAX    D               ;TABLE'S NEW END MARK
        DCX     D               ;POINT TO 2ND BYTE OF VALUE
        INR     A               ;TURN ZERO FLAG OFF
LECF9:  LHLD    OPRD            ;GET SYMBOL'S VALUE
        XCHG                    ;TO D,E
        MOV     M,E             ;STORE 1 BYTE
        DCX     H               ;POINT FROM 2ND TO 1ST BYTE
        MOV     M,D             ;STORE IT, TOO.
        RET                     ;DONE
;
;DO ASC PSEUDO OP
;
LED01:  CALL    SBLK            ;SCAN TO 1ST NON BLANK CHAR
        JC      ERRA            ;IF C/R FOUND
        MOV     C,A             ;SAVE AS DELIM.
        INX     H               ;POINT TO 1ST CHR OF STRING
        MOV     A,M             ;GET IT
        CPI     0DH             ;END OF LINE ?
        JZ      ERRA            ;ERROR THEN
        CMP     C               ;SECOND DELIMITER ?
        JZ      ERRA            ;ERROR THEN, TOO
        MVI     B,0F9H          ;******MAY BE ADDRESS DEPENDANT
LED15:  MOV     A,M             ;GET THE CHAR FROM STRING
        CPI     0DH             ;END OF LINE ?
        JZ      LED57           ;DONE WITH STRING THEN
        CMP     C               ;2ND DELIMITER ?
        JZ      LED57           ;DONE WITH STRING THEN
        SHLD    PNTR            ;SAVE STRING POINTER
        LHLD    ASPC            ;GET PROGRAM COUNTER
        INX     H               ;THIS USED UP 1 BYTE
        SHLD    ASPC            ;PUT UPDATED POINTER BACK
        INR     B               ;
        PUSH    B               ;SAVE B,C
        CALL    ASTO            ;PUT BYTE IN MEMORY & LISTING
        MOV     A,L             ;GET LOW BYTE OF LISTING PTR
        CPI     (OBUF+15) AND 0FFH
        JNZ     LED4F           ;NO, DO NEXT BYTE
        CALL    AOUT            ;YES, PRINT THE LINE
        POP     B               ;GET BACK B,C
        MVI     B,0             ;RESET FLAG
        PUSH    B               ;SAVE B,C AGAIN
        LXI     H,2020H         ;TWO SPACES
        SHLD    OBUF            ;BLANK ADDR FIELD BYTES 1 & 2
        SHLD    OBUF+2          ;3RD & 4TH BYTES
        MVI     A,0DH           ;GET C/R
        STA     OBUF+16         ;LINE ENDS WITH OBJECT CODE
        LXI     H,OBUF+3        ;LAST BYTE OF ADDR FIELD
        SHLD    OIND            ;OUTPUT POINTER FOR NXT LINE
LED4F:  LHLD    PNTR            ;GET STRING POINTER
        POP     B               ;RESTORE B,C
        INX     H               ;INCREMENT STRING POINTER
        JMP     LED15           ;DO NEXT BYTE OF STRING
;
;GET HERE WHEN HAVE REACHED 2ND DELIMITER OR C/R
;
LED57:  MOV     A,B             ;GET 1ST LINE FLAG TO A
        ORA     A               ;SET FLAGS
        RM                      ;ON 1ST LINE, DONE
        JZ      LEAF6           ;FOR S.T./XREF, MAIN LOOP
        LHLD    OIND            ;GET OUTPUT LINE POINTER
        INX     H               ;POINT PAST LAST BYTE
        MVI     M,0DH           ;TERMINATE LINE
        RET                     ;DONE
;
;PROCESS STAX AND LDAX INST.
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
;PROCESS PUSH, POP, INX, DCX, DAD INSTRUCTIONS
;
TYP3:   CALL    ASBL            ;FETCH OPERAND
        CNZ     ERRR            ;ILLEGAL REGISTER
        MOV     A,L             ;GET LOW ORDER OPERAND
        RRC                     ;CHECK LOW ORDER BIT
        CC      ERRR            ;ILLEGAL REGISTER
        RAL                     ;RESTORE
        CPI     8               ;
        CNC     ERRR            ;ILLEGAL REGISTER
TY31:   RLC                     ;SHIFT LEFT 3 TIMES
        RAL                     ;
        RAL                     ;
TY32:   MOV     B,A             ;SAVE REGISTER
        LDAX    D               ;FETCH OPCODE BASE
        ADD     B               ;FORM OPCODE W/REGISTER
        CPI     76H             ;CHECK FOR MOV M,M
        CZ      ERRR            ;ILLEGAL REGISTER
        JMP     ASTO            ;PUT IN MEM & INR P.C.
;
;PROCESS ACCUMULATOR, INR, DCR, MOV, RST INSTRUCTIONS
;
TYP4:   CALL    ASBL            ;FETCH OPERAND
        CNZ     ERRR            ;ILLEGAL REGISTER
        MOV     A,L             ;GET LOW ORDER OPERAND
        CPI     8               ;
        CNC     ERRR            ;ILLEGAL REGISTER
        LDAX    D               ;FETCH OPCODE BASE
        CPI     40H             ;CHECK FOR MOV INST.
        JZ      TY41            ;PROCESS THEM SEPARATELY
        CPI     0C7H            ;
        MOV     A,L             ;OPERAND IN ACCUM.
        JZ      TY31            ;RST INSTRUCTION
        JM      TY32            ;ACCUMULATOR INST.
        JMP     TY31            ;INR, DCR INSTRUCTIONS
;
;PROCESS MOV INSTRUCTIONS
;
TY41:   DAD     H               ;MULT OPRND BY 8 (3 LFT SHIFTS)
        DAD     H               ;
        DAD     H               ;
        ADD     L               ;FORM OPCODE
        STAX    D               ;SAVE OPCODE
        CALL    MPNT            ;
        CALL    ASCN            ;
        CNZ     ERRR            ;
        MOV     A,L             ;FETCH LOW ORDER OPERAND
        CPI     8               ;
        CNC     ERRR            ;ILLEGAL REGISTERS
        JMP     TY32            ;
;
;PROCESS IMMEDIATE INSTRUCTIONS
;IMMEDIATE BYTE CAN BE -256 TO +255
;MVI INSTRUCTION IS A SPECIAL CASE AND HAS 2 OPERANDS
;
TYP5:   CPI     6               ;CHECK FOR MVI INST
        CZ      TY56            ;SPECIAL CASE
        CALL    ASTO            ;PUT OBJ. IN MEM & ON LISTING
TYS5:   CALL    ASBL            ;GET IMMEDIATE ARG.
        INR     A               ;
        CPI     2               ;CHECK OPERAND FOR RANGE
        CNC     ERRV            ;OPERAND OUT OF RANGE
        MOV     A,L             ;
        JMP     ASTO            ;PUT OBJECT IN MEM & ON LISTING
;
;FETCH 1ST ARGUMENT FOR MVI AND LXI INSTRUCTIONS
;
TY56:   CALL    ASBL            ;FETCH ARGUMENT
        CNZ     ERRR            ;ILLEGAL REGISTER
        MOV     A,L             ;GET LOW ORDER ARGUMENT
        CPI     8               ;
        CNC     ERRR            ;ILLEGAL REGISTER
        DAD     H               ;MULT BY 8 (3 LEFT SHIFTS)
        DAD     H               ;
        DAD     H               ;
        LDAX    D               ;FETCH OPCODE BASE
        ADD     L               ;FORM OPCODE
        MOV     E,A             ;SAVE OBJECT BYTE
MPNT:   LHLD    PNTR            ;FETCH POINTER
        MOV     A,M             ;FETCH CHARACTER
        CPI     ','             ;CHECK FOR COMMA
        INX     H               ;INCREMENT POINTER
        SHLD    PNTR            ;AND SAVE IT
        JNZ     ERRS            ;SYNTAX ERROR IF NO COMMA
        MOV     A,E             ;
        RET                     ;
;
;PROCESS 3 BYTE INSTRUCTIONS
;LXI INSTRUCTION IS A SPECIAL CASE
;
TYP6:   CPI     1               ;CHECK FOR LXI INSTRUCTION
        JNZ     TY6             ;JUMP IF NOT LXI
        CALL    TY56            ;GET REGISTER
        ANI     8               ;CHECK FOR ILLEGAL REGISTER
        CNZ     ERRR            ;REGISTER ERROR
        MOV     A,E             ;GET OPCODE
        ANI     0F7H            ;CLEAR BIT IN ERROR
TY6:    CALL    ASTO            ;STORE OBJECT BYTE
TYS6:   CALL    ASBL            ;FETCH OPERAND
        MOV     A,L             ;TO ACCUM
        MOV     D,H             ;
        CALL    ASTO            ;PUT 2ND BYTE IN MEMORY
        MOV     A,D             ;
;
;THIS ROUTINE IS USED TO STORE OBJECT CODE PRODUCED BY
;THE ASSEMBLER DURING THE 2ND PASS INTO MEMORY
;
ASTO:   LHLD    BBUF+2          ;FETCH STORAGE COUNTER
        MOV     B,A             ;SAVE OBJECT BYTE
        LDA     PASI            ;GET PASS INDICATOR
        CPI     2               ;3RD PASS ? (S.T./XREF)
        RNC                     ;SKIP OBJECT STORAGE THEN
        MOV     A,B             ;ELSE RESTORE OBJECT BYTE
        MOV     M,A             ;AND PUT IT IN MEMORY
        INX     H               ;INCREMENT STORAGE POINTER
        SHLD    BBUF+2          ;AND PUT IT BACK
        LHLD    OIND            ;GET OUTPUT ADDRESS
        INX     H               ;INCR. TO POINT PAST LAST BYTE
        INX     H               ;AGAIN, PUT SPACE BETWEEN BYTES
        CALL    BINH+3          ;CONVERT OBJECT BYTE TO ASCII
        SHLD    OIND            ;AND SAVE UPDATED POINTER
        RET                     ;OBJECT IN MEM & ON LISTING
;
;PROCESS THE END PSEUDO OP
;
LEE39:  LXI     H,PASI          ;POINT TO THE PASS INDICATOR
        MOV     A,M             ;GET IT
        CPI     1               ;JUST FINISHED 2ND PASS ?
        JZ      LEE4C           ;YES - DONE OR START 3RD PASS
        JNC     LEE9D           ;IF 3RD PASS COMPLETE
        CALL    CRLF            ;DO CRLF AT START OF EA. PASS
        INR     M               ;INCREMENT PASS COUNTER
        JMP     LEAEA           ;START NEXT PASS
;
;GET HERE AT END OF 2ND PASS
;
LEE4C:  LDA     SYMX            ;GET S.T./XREF FLAG
        CPI     'E'+1           ;'S' OR 'X' OPTION REQUESTED ?
        JC      EORMS           ;NO - ASSY DONE, BACK TO ALS8
        INR     M               ;YES - INCREMENT PASS IND.
        CALL    CRLF            ;CRLF AFTER EA. PASS
LEE58:  CALL    CRLF            ;AGAIN BEFORE S.T./XREF
        XRA     A               ;GET A ZERO
LEE5C:  STA     SCNT            ;SAVE AS SYMBOL COUNT
        LHLD    SYMADD          ;GET ADDR OF SYMBOL TABLE
        MOV     D,H             ;DUPLICATE IN D,E
        MOV     E,L             ;
;
;TEST NAMES POINTED TO BY D,E AND H,L
;LEAVE NAME WHICH COMES 1ST IN ALPHABETICAL ORDER
;POINTED TO BY D,E
;
LEE64:  LXI     B,LLAB-1        ;OFFSET TO LAST CHAR OF NAME
        MOV     A,M             ;GET 1ST CHAR OF NAME
        ORA     A               ;ZERO ?
        JZ      LEE82           ;REACHED END OF SYMBOL TABLE
        DAD     B               ;POINT TO LAST CHAR
        XCHG                    ;
        DAD     B               ;POINT TO LAST CHAR
        XCHG                    ;
        INR     C               ;C = LENGTH OF NAME
        CALL    COM1            ;COMPARE NAMES
        INX     H               ;POINT BACK AT 1ST CHAR
        INX     D               ;POINT BACK AT 1ST CHAR
        LXI     B,LLAB+2        ;LENGTH OF TABLE ENTRY
        JC      LEE7E           ;ALREADY IN CORRECT ORDER
        MOV     D,H             ;MAKE D,E POINT TO "1ST" NAME
        MOV     E,L             ;
LEE7E:  DAD     B               ;H,L POINT TO NEXT NAME
        JMP     LEE64           ;TST NAMES POINTED TO BY DE, HL
;
;GET HERE WHEN PASS COMPLETED THRU SYMBOL TABLE
;D,E POINT TO NAME WHICH COMES "1ST" IN ALPH. ORDER
;NOTE THAT THE SYMBOLS ARE NOT MOVED IN THE TABLE, BUT
;THAT THEIR 1ST CHAR IS REPLACED WITH 0FFH AS THE ARE PRINTED
;
LEE82:  XCHG                    ;"1ST" NAME TO D,E
        MOV     A,M             ;GET 1ST CHAR OF "LOWEST" NAME
        DCR     A               ;IS IT 0FFH ? (IT WASN'T ZERO)
        JM      EOR             ;IF SO, RETURN TO ALS-8
        INR     A               ;ELSE RESTORE
        SHLD    SYMSV           ;AND SAVE IT'S ADDRESS
        CALL    LEEA8           ;PRINT IT AND IT'S VALUE
        LDA     SYMX            ;GET THE S.T./XREF FLAG
        CPI     'X'             ;XREF REQUESTED ?
        JZ      LEAEA           ;DO THE CROSS REFERENCE SEARCH
        LDA     SCNT            ;ELSE GET SYMBOL COUNT
        INR     A               ;INCREMENT IT
        CPI     4               ;MAX NO. SYMBOLS PRINTED/LINE
LEE9D:  LHLD    SYMSV           ;GET SYMBOL POINTER
        MVI     M,0FFH          ;MAKE SYMBOL LAST
        JC      LEE5C           ;DO MORE SYMBOLS THIS LINE
        JMP     LEE58           ;TO START A NEW LINE
;
;THIS ROUTINE PRINTS NAMES AND THEIR VALUES.
;IT IS USED FOR BOTH THE S.T./XREF AND FOR THE FILES/IODR
;COMMANDS TO LIST THE DIRECTORY CONTENTS
;
LEEA8:  MVI     C,NMLEN         ;CAREFUL IF LLAB <> NMLEN !
LEEAA:  MOV     B,A             ;SAVE CHAR IN B
        ORA     A               ;SET FLAGS
        JNZ     LEEB1           ;IF CHAR IS NOT HEX 00
        MVI     B,' '           ;PRINT SPACE TO PAD SHORT NAMES
LEEB1:  CALL    OUT8            ;PRINT IT
        INX     H               ;INCREMENT POINTER
        MOV     A,M             ;GET CHAR TO PRINT
        DCR     C               ;DCR COUNT OF CHARS LEFT
        JNZ     LEEAA           ;DO REMAINING CHARS
        MVI     C,3             ;NO OF SPACES TO FOLLOW NAME
        CALL    BLKO            ;PRINT THEM
        MOV     D,M             ;GET SYMBOL VALUE IN D,E
        INX     H               ;2ND BYTE
        MOV     E,M             ;IN D,E
        PUSH    H               ;SAVE POINTER
        CALL    ADOUT           ;PRINT VALUE
        POP     D               ;RESTORE POINTER TO D,E
        MVI     C,5             ;NO OF SPACES AFTER VALUE
        JMP     BLKO            ;PRINT THEM & RETURN
;
;ROUTINE TO SCAN THROUGH SPACES UNTIL NON-BLANK CHAR FOUND
;
SBLK:   LHLD    PNTR            ;GET POINTER
SBL1:   MOV     A,M             ;GET CHAR AT POINTER
        CPI     ' '             ;SPACE ?
        RNZ                     ;NO - DONE
SBL2:   INX     H               ;YES - INCREMENT POINTER
        SHLD    PNTR            ;AND UPDATE IT
        JMP     SBL1            ;THEN CHECK NEW CHAR
;
;THIS ROUTINE IS USED TO CHECK THE CONDITION CODE
;MNEMONICS FOR CONDITIONAL JUMPS, CALLS AND RETURNS.
;
COND:   LXI     H,ABUF+1        ;POINT TO 2ND CHAR
        SHLD    ADDS            ;SAVE IT'S ADDR.
        MVI     B,2             ;MAX. LENGTH OF CONDITION
        JMP     COPC            ;TO LOOKUP IN TABLE
;
;HERE IS THE ASSEMBLER OPCODE TABLE
;
;PSEUDO OPS WITH LENGTH OF 4 (OR PADDED TO 4 WITH HEX 0'S)
;COME FIRST
;
OTAB:   DB      'ORG'
        DW      0
        DB      'EQU'
        DW      0100H
        DB      'DB'
        DW      0
        DB      0FFH
        DB      'DS'
        DW      0
        DB      2
        DB      'DW'
        DW      0
        DB      3
        DB      'END'
        DW      0400H
        DB      'COM'
        DW      0500H
        DB      'LST'
        DW      0600H
        DB      'NLST'
        DB      7
        DB      'ASC'
        DW      0800H
        DB      0
;
;THESE ARE FOLLOWED BY 3 CHAR OPCODES (OR 2 CHAR PADDED TO 3)
;
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
        DW      0FB00H
        DB      'DI'
        DW      0F300H
        DB      'NOP'
        DW      0
        DB      'XCHG'
        DB      235
        DB      'XTHL'
        DB      227
        DB      'SPHL'
        DB      249
        DB      'PCHL'
        DW      00E9H
        DB      'STAX'
        DB      2
        DB      'LDAX'
        DW      000AH
        DB      'PUSH'
        DB      197
        DB      'POP'
        DW      0C100H
        DB      'INX'
        DW      0300H
        DB      'DCX'
        DW      0B00H
        DB      'DAD'
        DW      0900H
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
        DW      00C7H
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
        DW      0DB00H
        DB      'OUT'
        DB      211
        DB      'MVI'
        DW      0006H
;
;WHICH ARE FOLLOWED BY 4 CHAR OPCODES (OR LESS PADDED TO 4)
;
        DB      'JMP'
        DW      0C300H
        DB      'CALL'
        DB      205
        DB      'LXI'
        DW      0100H
        DB      'LDA'
        DW      3A00H
        DB      'STA'
        DW      3200H
        DB      'SHLD'
        DB      34
        DB      'LHLD'
        DW      002AH
;
;WHICH IS FINALLY FOLLOWED BY THE 2 CHAR CNDX CODE TABLE
;
        DB      'NZ'
        DB      0
        DB      'Z'
        DW      0800H
        DB      'NC'
        DB      16
        DB      'C'
        DW      1800H
        DB      'PO'
        DB      32
        DB      'PE'
        DB      40
        DB      'P'
        DW      3000H
        DB      'M'
        DW      3800H
        DB      0
;
;THIS ROUTINE IS USED TO CHECK A GIVEN OPCODE
;AGAINST THE LEGAL OPCODES IN THE OPCODE TABLE
;
COPC:   LHLD    ADDS            ;GET POINTER
        LDAX    D               ;FETCH CHARACTER
        ORA     A               ;SET FLAGS
        JZ      COP1            ;END OF TABLE
        MOV     C,B             ;LENGTH OF STRING
        CALL    SEAR            ;SEARCH THE TABLE
        LDAX    D               ;
        RZ                      ;RETURN IF MATCH
        INX     D               ;NEXT STRING
        JMP     COPC            ;CONTINUE SEARCH
COP1:   INR     A               ;CLEAR ZERO FLAG
        INX     D               ;INCREMENT ADDRESS
        RET                     ;NO MATCH
;
;THIS ROUTINE CHECKS THE LEGAL OPCODES IN BOTH PASS 1
;AND PASS 2.  IN PASS 1 THE PROGRAM COUNTER IS INCREMENTED
;BY THE CORRECT NUMBER OF BYTES. AN ADDRESS IS
;ALSO SET SO THAT AN INDEXED JUMP CAN BE MADE TO
;PROCESS THE OPCODE FOR PASS 2.
;
OPCD:   LXI     H,ABUF          ;GET ADDRESS
        SHLD    ADDS            ;SAVE IT
        LXI     D,OTAB          ;OPCODE TABLE ADDR.
        MVI     B,4             ;LENGTH OF PSEUDO-OP ENTRIES
        CALL    COPC            ;LOOK IT UP IN TABLE
        JZ      PSEU            ;PSEUDO-OP IF FOUND
        DCR     B               ;ELSE TRY 3-CHAR OPCODES
        CALL    COPC            ;LOOK IT UP
        JZ      OP1             ;IF FOUND IN THIS GROUP
        INR     B               ;THEN TRY FOUR CHAR OPCODES
        CALL    COPC            ;LOOK IT UP
OP1:    LXI     H,ASTO          ;TYPE 1 INSTRUCTIONS
OP2:    MVI     C,1             ;1 BYTE INST.
        JZ      OCNT            ;IF FOUND
;
OPC2:   CALL    COPC            ;CHECK FOR STAX, LDAX
        LXI     H,TYP2          ;
        JZ      OP2             ;
        CALL    COPC            ;CHK FOR PUSH, POP, INX, DCX, DAD
;
        LXI     H,TYP3          ;
        JZ      OP2             ;
        DCR     B               ;3 CHAR OPCODES
        CALL    COPC            ;ACC INST: INR, DCR, MOV, RST
;
        LXI     H,TYP4          ;
        JZ      OP2             ;
;
OPC3:   CALL    COPC            ;IMMEDIATE INSTRUCTIONS
        LXI     H,TYP5
        MVI     C,2             ;2 BYTE INSTRUCTIONS
        JZ      OCNT            ;
        INR     B               ;4 CHAR OPCODES
        CALL    COPC            ;JMP, CALL, LXI, LDA, STA,
;                               ;LHLD, SHLD
        JZ      OP4             ;
        CALL    COND            ;CONDITIONAL INST.
        JNZ     OERR            ;ILLEGAL OPCODE
        ADI     192             ;ADD BASE VALUE TO RETURN
        MOV     D,A             ;
        MVI     B,3             ;3 CHARACTER OPCODES
        LDA     ABUF            ;FETCH 1ST CHAR
        MOV     C,A             ;SAVE IT
        CPI     'R'             ;CONDITIONAL RETURN
        MOV     A,D             ;
        JZ      OP1             ;
        MOV     A,C             ;
        INR     D               ;FORM CONDITIONAL JUMP
        INR     D               ;
        CPI     'J'             ;CONDITIONAL JUMP
        JZ      OPAD            ;
        CPI     'C'             ;CONDITIONAL CALL
        JNZ     OERR            ;ILLEGAL OPCODE
        INR     D               ;FORM CONDITIONAL CALL
        INR     D               ;
OPAD:   MOV     A,D             ;GET OPCODE
OP4:    LXI     H,TYP6          ;
OP5:    MVI     C,3             ;3 BYTE INSTRUCTIONS
OCNT:   STA     TEMP            ;SAVE OPCODE
;
;CHECK FOR OPCODE ONLY CONTAINING CORRECT NO. OF CHARS.
;THUS ADDQ, SAY, WOULD GIVE AN ERROR
;
        MVI     A,ABUF AND 0FFH ;LOAD BUFFER ADDRESS
        ADD     B               ;LENGTH OF OPCODE
        MOV     E,A             ;
        MVI     A,ABUF/256      ;
        ACI     0               ;GET HIGH ORDER ADDRESS
        MOV     D,A             ;
        LDAX    D               ;FETCH CHAR AFTER OPCODE
        ORA     A               ;IT SHOULD BE ZERO
        JNZ     OERR            ;OPCODE ERROR
        LDA     PASI            ;FETCH PASS INDICATOR
OCN1:   MVI     B,0             ;
        XCHG                    ;
OCN2:   LHLD    ASPC            ;FETCH PROGRAM COUNTER
        DAD     B               ;ADD IN BYTE COUNT
        SHLD    ASPC            ;STORE PC
        ORA     A               ;WHICH PASS
        RZ                      ;RETURN IF PASS 1
        LDA     TEMP            ;FETCH OPCODE
        XCHG                    ;
        PCHL                    ;
;
OERR:   LXI     H,ERRO          ;SET ERROR ADDRESS
LF0DB:  MVI     C,3             ;LEAVE 3 BYTES FOR PATCH
        JMP     OCN1-3          ;
;
LF0E0:  LXI     H,ERRL
        JMP     LF0DB
;
PSEU:   LXI     H,ABUF+4        ;SET BUFFER ADDRESS
        MOV     A,M             ;FETCH CHAR AFTER OPCODE
        ORA     A               ;IT SHOULD BE ZERO
        JNZ     OERR            ;IF NOT
        LDA     PASI            ;GET PASS INDICATOR
        ORA     A               ;PASS 1 ?
        JZ      PSU1            ;YES - JUMP
        JMP     PSU2            ;NO - DO 2ND PASS OPERATIONS
;
;THIS ROUTINE IS USED TO PROCESS LABELS.
;IT CHECKS TO SEE IF A LABEL IS IN THE SYMBOL TABLE
;OR NOT.  ON RETURN, Z=1 INDICATES A MATCH WAS FOUND
;AND H,L CONTAIN THE VALUE ASSOCIATED WITH THE LABEL.
;THE REGISTER NAMES A, B, C, D, E, H, L, AND M ARE
;PRE-DEFINED AND NEED NOT BE ENTERED BY THE USER.
;ON RETURN, C=1 INDICATES A LABEL ERROR.
;
SLAB:   CPI     'A'             ;CHECK FOR LEGAL CHAR
        RC                      ;ILLEGAL CHAR
        CPI     'Z'+1           ;CHECK FOR LEAGAL CHAR
        CMC                     ;SETTING CARRY FOR PROPER RET
        RC                      ;ILLEGAL CHAR
        CALL    ALPS            ;PLACE SYMBOL IN ABUF
        LXI     H,ABUF          ;POINT TO SYMBOL
        LDA     PASI            ;GET PASS INDICATOR
        CPI     2               ;DOING XREF/S.T. PASS ?
        INR     A               ;BUMP PASS INDICATOR IN ACCUM.
        RNC                     ;RETURN IF ON XREF/S.T. PASS
        SHLD    ADDS            ;SAVE ADDRESS OF SYMBOL
        DCR     B               ;CHECK IF ONLY 1 CHAR SYMBOL
        JNZ     SLA1            ;LOOK THEM UP IN REG. TABLE
        INR     B               ;SET B=1
        LXI     D,RTAB          ;POINT TO REGISTER TABLE
        CALL    COPC            ;LOOK FOR MATCH
        JNZ     SLA1            ;NOT FOUND, TRY SYMBOL TABLE
        MOV     L,A             ;FOUND, VALUE TO L
        MVI     H,0             ;AND MAKE H ZERO
        STC                     ;SET CARRY
        CMC                     ;COMPLEMENT CARRY
        RET                     ;DONE
SLA1:   LHLD    SYMADD          ;GET SYMBOL TABLE ADDR
        XCHG                    ;TO D,E
        MVI     B,LLAB          ;LENGTH OF LABELS
        CALL    COMS            ;CHECK TABLE
        RET                     ;WITH FLAGS SET
;
;HERE IS THE INTERNAL REGISTER TABLE
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
        DB      0
;
;THIS ROUTINE SCANS THE INPUT LINE AND PLACES THE
;OPCODES AND LABELS IN THE ABUF BUFFER.  THE SCAN TERMINATES
;WHEN A NON ALPHANUMERIC CHARACTER (A-Z, 0-9) IS FOUND.
;
ALPS:   MVI     B,0             ;SET COUNT
ALP1:   STAX    D               ;STORE CHAR IN BUFFER
        INR     B               ;INCREMENT COUNT
        MOV     A,B             ;FETCH COUNT
        CPI     15              ;MAX BUFFER SIZE
        RNC                     ;RETURN IF BUFFER FILLED
        INX     D               ;INCREMENT ABUF POINTER
        INX     H               ;INCREMENT INPUT LINE POINTER
        SHLD    PNTR            ;SAVE INPUT POINTER
        MOV     A,M             ;FETCH CHARACTER
        CPI     '0'             ;CHECK FOR LEGAL CHAR
        RC                      ;ILLEGAL CHAR
        CPI     '9'+1           ;AGAIN
        JC      ALP1            ;NUMERIC = O.K.
LF154:  CPI     'A'             ;TEST FOR LETTER
        RC                      ;ILLEGAL CHAR
        CPI     'Z'+1           ;
        JC      ALP1            ;VALID LETTER
        RET                     ;W/CARRY SET, ILLEGAL CHAR
;
;THIS ROUTINE IS USED TO SCAN THE INPUT LINE AND DETERMINE
;THE VALUE OF AN OPERAND FIELD.  ON RETURN, VALUE IS IN H,L.
;
ASBL:   CALL    SBLK            ;GET THE FIRST ARGUMENT
ASCN:   LHLD    PNTR            ;FETCH SCAN POINTER
        MOV     A,M             ;FETCH CHARACTER
        LXI     H,0             ;GET A ZERO
        SHLD    OPRD            ;INITIALIZE OPERAND
        INR     L               ;L = 1 ==> BECOMES OPRI
        MOV     H,A             ;CHAR TO H, BECOMES GTLT
        SHLD    OPRI            ;SAVE OPRI AND GTLT
NXT1:   LHLD    PNTR            ;GET SCAN POINTER
        DCX     H               ;INIT. FOR LOOP BELOW
        CALL    ZBUF            ;CLEAR OUT ABUFF
        STA     SIGN            ;ZERO SIGN INDICATOR
NXT2:   INX     H               ;INCREMENT POINTER
        MOV     A,M             ;GET CHAR
        CPI     '<'             ;HIGH ORDER BYTE SIGN ?
        JZ      NXT2            ;ALREADY TAKEN CARE OF (GTLT)
        CPI     '>'             ;LOW ORDER BYTE SIGN ?
        JZ      NXT2            ;ALREADY TAKEN CARE OF (GTLT)
        CPI     ' '+1           ;CAR. RETURN OR SPACE ?
        JC      SEND            ;END OF OPERAND FIELD THEN
        CPI     ','             ;FIELD SEPARATOR ?
        JZ      SEND            ;ALSO END OF FIELD
;
;CHECK FOR OPERATORS
;
        CPI     '+'             ;CHECK FOR PLUS
        JZ      LF1A6           ;STORE 0 AS SIGN
        INR     B               ;MAKE B 01
        CPI     '-'             ;MINUS SIGN ?
        JZ      LF1A6           ;SAVE NON-ZERO SIGN
        INR     B               ;MAKE B 02
        CPI     '*'             ;CHECK FOR MULTIPLY
        JZ      LF1A6           ;STORE B AS 02
        INR     B               ;MAKE B 03
        CPI     '/'             ;CHECK FOR DIVIDE
        JNZ     ASC2            ;IF NOT SIGN, CHK FOR OPERAND
LF1A6:  MOV     A,B             ;MOVE B TO A
        STA     SIGN            ;SET SIGN BYTE
ASC1:   LDA     OPRI            ;FETCH OPERAND INDICATOR
        CPI     2               ;CHECK FOR 2 OPERATORS TOGETHER
        JZ      ERRS            ;SYNTAX ERROR
        MVI     A,2             ;NOW SET UP OPRI AS 2
        STA     OPRI            ;AND STORE IT
        JMP     NXT2            ;ANOTHER WILL NOT BE ALLOWED
;
;CHECK FOR OPERANDS
;
ASC2:   MOV     C,A             ;SAVE CHAR
        LDA     OPRI            ;GET OPERATOR
        ORA     A               ;CHECK FOR TWO OPERANDS
        JZ      ERRS            ;TWO IN A ROW NOT ALLOWED
        MOV     A,C             ;RESTORE CHAR
        CPI     '$'             ;PGM. COUNTER ?
        JZ      LECC3           ;GET IT & CONTINUE
;
;CHECK FOR ASCII CHARACTERS
;
ASC3:   CPI     27H             ;QUOTE MARK ?
        JNZ     ASC5            ;NO - MUST BE SOMETHING ELSE
        LXI     D,0             ;INITIALIZE CHAR. VALUE
        MVI     C,3             ;MAX CHAR COUNT + 1
ASC4:   INX     H               ;POINT TO NEXT CHAR
        SHLD    PNTR            ;SAVE POINTER
        MOV     A,M             ;GET CHAR
        CPI     0DH             ;CARRIAGE RETURN ?
        JZ      ERRA            ;ARGUMENT ERROR THEN
        CPI     27H             ;2ND QUOTE MARK ?
        JNZ     SSTR            ;NO - PROCESS CHAR
        INX     H               ;POINT PAST 2ND QUOTE
        SHLD    PNTR            ;UPDATE POINTER
        MOV     A,M             ;FETCH NEXT CHAR
        CPI     27H             ;CHECK FOR TWO IN A ROW
        JNZ     AVAL+1          ;WAS TERMINAL QUOTE
SSTR:   DCR     C               ;CHECK COUNT
        JZ      ERRA            ;TOO MANY CHARS
        MOV     D,E             ;1ST CHAR (IF ANY) IN D
        MOV     E,A             ;THIS CHAR IN E
        JMP     ASC4            ;GET NEXT CHAR
ASC5:   CPI     '0'             ;SEE IF NUMERIC
        JC      ERRA            ;NO CHARS < 30H LEGAL
        CPI     '9'+1           ;TRY HIGH END
        JNC     ALAB            ;ALPHABETIC - TRY LABEL
        CALL    NUMS            ;NUMERIC - CONVERT
        JC      ERRA            ;ARGUMENT ERROR
AVAL:   XCHG                    ;OPERAND VALUE IN H,L
        LHLD    OPRD            ;FETCH OPERAND VALUE
        XRA     A               ;GET A ZERO
        STA     OPRI            ;AND CLEAR OPERAND INDICATOR
        LDA     SIGN            ;GET SIGN
        CPI     1               ;MINUS SIGN ?
        JZ      ASUB            ;THEN DO A SUBTACTION
        JNC     LF227           ;MULTIPLY OR DIVIDE
        DAD     D               ;ADD NEW OPERAND DE TO OLD HL
ASC7:   SHLD    OPRD            ;AND UPDATE THE RESULT
        JMP     NXT1            ;NOW SEE IF MORE
ASUB:   MOV     A,L             ;GET OLD OPRAND IN H,L
        SUB     E               ;SUBTRACT NEW OPRAND IN D,E
        MOV     L,A             ;RESTORE RESULT TO H,L
        MOV     A,H             ;NOW DO HIGH ORDER BYTE
        SBB     D               ;
        MOV     H,A             ;PUT IT BACK
        JMP     ASC7            ;AND FINISH UP
;
;DO MULTIPLY DIVIDE
;
LF227:  MOV     B,D             ;MOVE CURRENT OPRD IN DE TO BC
        MOV     C,E             ;
        XCHG                    ;AND OLD OPRND TO D,E
        LXI     H,0             ;INITIALIZE ANSWER
        CPI     3               ;DOING DIVISION ?
        JZ      LF247           ;YES
        CALL    LF238           ;NO, MULITPLICATION - DO IT
        JMP     ASC7            ;THEN FINISH UP
;
;MULTIPLICATION ROUTINE
;
;THIS ALGORITHM IS INTERESTING BUT DIFFICULT TO UNDERSTAND
;UNTIL STUDIED FOR A WHILE. BUT IT DOES WORK
;
LF238:  MVI     A,16            ;16 BITS
LF23A:  DAD     H               ;MULTIPLY PARTIAL ANS. BY 2
        XCHG                    ;CHANGE REGISTERS
        DAD     H               ;MULT. MULTIPLIER BY 2
        XCHG                    ;RESTORE ORIG. REGISTERS
        JNC     LF242           ;IF 2X MULTIPLIER ==> NO CARRY
        DAD     B               ;ADD MULTIPLICAND TO INTER ANS
LF242:  DCR     A               ;DCR NO OF BITS LEFT TO CHECK
        JNZ     LF23A           ;DO IT OVER IF NOT DONE
        RET                     ;ELSE FINAL ANS. IS IN H,L
;
;DIVISION ROUTINE
;
;FIRST CHECK FOR DIVIDE BY ZERO
;
LF247:  MOV     A,B             ;GET BYTE OF DIVISOR
        ORA     C               ;OR IT WITH OTHER BYTE
        JZ      ASC7            ;CAN'T DIVIDE BY ZERO
        CALL    LF253           ;DO ACTUALLY DO THE DIVISION
        XCHG                    ;ANSWER TO H,L
        JMP     ASC7            ;TO FINISH UP
;
;DIVISION ROUTINE FOLLOWS. IT IS SIMILAR IN PRINCIPLE TO
;THE MULTIPLICATION ROUTINE
;
LF253:  MVI     A,16            ;16 BITS
LF255:  STA     SCNT            ;SAVE AS COUNT
        DAD     H               ;SHIFT LEFT
        XCHG                    ;SWITCH REGISTERS
        DAD     H               ;SHIFT LEFT
        XCHG                    ;RESTORE REGISTERS
        JNC     LF260           ;IF NO CARRY
        INX     H               ;ELSE INCREMENT H,L
LF260:  MOV     A,L             ;AND SUBTRACT DIVISOR FROM IT
        SUB     C               ;
        MOV     L,A             ;
        MOV     A,H             ;
        SBB     B               ;
        MOV     H,A             ;
        INX     D               ;INCREMENT PARTIAL ANSWER
        JNC     LF26C           ;OK IF NO CARRY
        DAD     B               ;ADD DIVISOR TO H,L
        DCX     D               ;AND DECREMENT ANSWER
LF26C:  LDA     SCNT            ;GET COUNT
        DCR     A               ;DECREMENT IT
        JNZ     LF255           ;IF NOT DONE, DO MORE
        RET                     ;ELSE DONE, ANS IN D,E
;
;CHECK FOR SYMBOLS FROM SYMBOL TABLE
;
ALAB:   CALL    SLAB            ;SEE IF LABEL IN TABLE
        JZ      AVAL            ;IF FOUND
        JC      ERRA            ;IF LABEL ERROR
        LDA     PASI            ;NOT FOUND, GET PASS IND.
        CPI     2               ;DOING S.T./XREF PASS ?
        JNC     LF291           ;YES - PROCESS
        LXI     D,SYSYM         ;ELSE TRY SYSTEM SYMBOL TABLE
        CALL    COMS            ;SEARCH IT
        JZ      AVAL            ;IF FOUND
        JMP     ERRU            ;ELSE UNDEFINED SYMBOL ERROR
;
;THE FOLLOWING ROUTINE IS USED TO HANDLE SYMBOLS DURING A
;SYMBOL TABLE/XREF PASS (3RD PASS, PASI = 2).
;DOES CROSS REFERENCE
;
LF291:  XCHG                    ;D,E ==> SYMBOL NAME IN ABUF
        LHLD    SYMSV           ;SYMBOL DOING XREF PASS FOR
        MVI     C,LLAB          ;LENGTH OF SYMBOL
        CALL    SEAR            ;SAME ?
        JNZ     LF2C2           ;NO
        LXI     H,SCNT          ;POINT TO SYMBOL COUNT
        INR     M               ;INCREMENT IT
        MOV     A,M             ;GET IT
        CPI     12              ;MAX. NO ON ONE LINE ?
        JC      LF2B1           ;NO - DO MORE THIS LINE
        MVI     M,1             ;YES - RESET COUNT
        CALL    CRLF            ;DO CAR RET/LINE FEED
        MVI     C,17            ;AND PRINT 17 SPACES
        CALL    BLKO            ;DO IT
LF2B1:  LXI     H,IBUF-5        ;POINT TO LINE NO.
        MVI     C,4             ;LENGTH OF LINE NO
LF2B6:  MOV     B,M             ;GET A CHAR
        CALL    OUT8            ;PRINT IT
        INX     H               ;POINT TO NEXT CHAR
        DCR     C               ;DECR CHAR COUNT
        JNZ     LF2B6           ;DO NEXT CHAR IF NOT DONE
        CALL    LE35C           ;PUT OUT A SPACE AFTER LINE NO
LF2C2:  XRA     A               ;CLEAR FLAGS
        MVI     H,0             ;AND H,L
        JMP     AVAL            ;AND CONTINUE SEARCH
;
;ROUTINE TO CHECK FOR TERMINATING CHARACTERS IN LABEL FILEDS
;
SEND:   LDA     OPRI            ;FETCH OPERAND INDICATOR
        ORA     A               ;SET FLAGS
        JNZ     ERRS            ;SYNTAX ERROR
        LHLD    OPRD            ;GET OPERAND VALUE
        LDA     GTLT            ;GET SHIFT INDICATOR
        CPI     '<'             ;USING HIGH HALF ONLY ?
        JNZ     LF2DE           ;NO - OK AS IS
        MOV     L,H             ;YES - MOVE HIGH HALF TO L
        JMP     LF2E3           ;AND CLEAR HIGH HALF
LF2DE:  CPI     '>'             ;USING LOW HALF ONLY ?
        JNZ     SEN1            ;NO - OK AS IS THEN
LF2E3:  MVI     H,0             ;ELSE CLEAR HIGH HALF
SEN1:   MOV     A,H             ;GET HIGH ORDER BYTE
        LXI     D,TEMP          ;GET ADDRESS
        ORA     A               ;SET FLAGS
        RET                     ;DONE
;
;GET A NUMERIC VALUE, HEX, OCTAL, OR DECIMAL
;
NUMS:   CALL    ALPS            ;PUT CHARS IN ABUFF
        DCX     D               ;POINT TO LAST CHAR
        LDAX    D               ;GET LAST CHAR
        LXI     B,ABUF          ;B,C POINT TO NUMBER
        CPI     'H'             ;HEX ?
        JZ      NUM2            ;YES - PROCESS
        CPI     'Q'             ;OCTAL ?
        JZ      NUM3            ;YES - PROCESS
        CPI     'D'             ;DECIMAL ?
        JNZ     ADEC            ;PROCESS DECIMAL BY DEFAULT
NUM1:   XRA     A               ;GET BINARY ZERO
        STAX    D               ;CLEAR THE TYPE LETTER
        JMP     ADEC            ;PROCESS DECIMAL
NUM2:   XRA     A               ;GET A BINARY ZERO
        STAX    D               ;CLEAR THE TRAILING H
        JMP     AHEX            ;PROCESS HEX
NUM3:   XRA     A               ;GET A BINARY ZERO
        STAX    D               ;CLEAR THE TRAILING Q
        JMP     AOCT            ;PROCESS OCTAL
;
;FOLLOWING ARE THE ERROR REPORTING ROUTINES
;
ERRR:   MVI     A,'R'           ;REGISTER ERROR
        LXI     H,0             ;CLEAR H,L
        STA     OBUF+18         ;WHERE THE ERROR LETTER GOES
        RET                     ;DONE
ERRS:   MVI     A,'S'           ;SYNTAX ERROR
        STA     OBUF+18         ;WHERE THE ERROR LETTER GOES
        LXI     H,0             ;CLEAR H,L
        JMP     SEN1            ;FINISH UP & GO HOME
ERRU:   MVI     A,'U'           ;UNDEFINED SYMBOL
        JMP     ERRS+2
ERRV:   MVI     A,'V'           ;VALUE ERROR
        JMP     ERRR+2
ERRM:   MVI     A,'M'           ;MISSING LABEL
        STA     OBUF+18         ;WHERE THE ERROR LETTER GOES
        JMP     LEB41           ;FINISH STATEMENT PROCESSING
ERRA:   MVI     A,'A'           ;ARGUMENT ERROR
        JMP     ERRS+2
ERRO:   MVI     A,'O'           ;OPCODE ERROR
        STA     OBUF+18         ;WHERE ERROR LETTER GOES
        LDA     PASI            ;GET PASS INDICATOR
        ORA     A               ;SET FLAGS
        RZ                      ;RET IF ON 1ST PASS
        MVI     C,3             ;ELSE SET UP TO LEAVE 3 BYTES
ERO1:   XRA     A               ;GET A BINARY ZERO
        CALL    ASTO            ;PUT IT IN MEMORY & LISTING
        DCR     C               ;DECR COUNT
        JNZ     ERO1            ;DO MORE IF NOT DONE
        RET                     ;DONE - ROOM LEFT FOR PATCH
ERRL:   MVI     A,'L'           ;LABEL ERROR
        JMP     ERRO+2
ERRD:   MVI     A,'D'           ;DUPLICATE LABEL
        STA     OBUF+18         ;WHERE ERROR LETTER GOES
        CALL    LEB41           ;DISPLAY THE ERROR
        JMP     OPC             ;PROCESS OPCODE
;
;FOLLOWING IS THE LINE FORMATING ROUTINE
;
;IT IS BEING LEFT UN-COMMENTED SO THAT YOU MAY TRY TO
;COMMENT IT YOURSELF & THEREFORE HAVE A BETTER APPRECIATION
;OF THE WORK I HAVE GONE TO - NOTE THAT IT IS LESS THAN TWO
;PAGES LONG, AND WE ARE NOW OVER 50 PAGES INTO THIS MONSTER.
;HAVE FUN.  (PS - I HAVE GONE TO THE TROUBLE OF PUTING
;IN SYMBOLIC REFERENCES TO MAKE THE CODE RE-ASSEMBLEABLE)
;
LF361:  MVI     C,0
        LXI     H,IBUF
        MOV     A,M
        CPI     COMCHR
        RZ
        MVI     B,2
        CALL    LF37A
        RC
        INR     B
        CALL    LF37A
        RC
        MVI     D,2
        JMP     LF37C
LF37A:  MVI     D,1
LF37C:  MOV     A,B
        ADD     D
        MOV     B,A
        LDA     TERMW
        SUI     (IBUF+81) AND 0FFH
        JZ      LF392
        JNC     LF38E
        DCR     B
        JMP     LF392
LF38E:  MOV     A,D
        RAL
        ADD     B
        MOV     B,A
LF392:  MOV     A,B
        ADI     5
        MOV     B,A
LF396:  MOV     E,D
LF397:  INR     C
        MOV     A,M
        CPI     20H
        RC
        INX     H
        JNZ     LF396
        DCR     E
        JNZ     LF397
LF3A4:  INR     C
        MOV     A,M
        CPI     20H
        RC
        INX     H
        JZ      LF3A4
        MOV     A,C
        SUB     B
        RNC
        DCX     H
        DCX     H
        MVI     M,0
        MOV     C,A
        DCR     D
        JNZ     LF3C5
        LHLD    PNTR
        CMA
        INR     A
        MOV     E,A
        MVI     D,0
        DAD     D
        SHLD    PNTR
LF3C5:  MOV     L,C
        MVI     H,0FFH
        MVI     C,0
        LXI     D,IBUF+82
        DAD     D
        XCHG
        CALL    RMOV
        XCHG
LF3D3:  INX     H
        MVI     M,20H
        MOV     A,L
        CMP     E
        JNZ     LF3D3
        INX     H
        INX     H
        MOV     C,B
        RET
LF3DF:  LDA     TERMW
        INR     A
        RET
;
;FOLLOWING IS THE CODE TO SET THE TERMINAL WIDTH
;NOTE THAT WHAT IS ACTUALLY STORED IS THE LOW ORDER BYTE
;OF THE 1ST ADDR BEYOND THE BUFFER END, NOT THE WIDTH ITSELF.
;
TERM:   LXI     B,ABUF          ;POINT TO KEYED IN VALUE
        CALL    ADEC            ;CONVERT TO BINARY
        MOV     A,L             ;VALUE TO A
        SUI     (-(IBUF+1)) AND 0FFH
        CPI     (IBUF+120) AND 0FFH
        JNC     WHAT            ;IF TERMW > 120 SPECIFIED
        STA     TERMW           ;SAVE IT
        JMP     EORMS           ;DONE
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
;
; ****** END OF ALS8ASM MODULE
;
;
;THIS IS THE INTERPRETIVE DEBUGGING SIMULATOR
;IT'S BASIC OPERATION IS TO SIMULATE ALL OF THE 8080
;REGISTERS IN MEMORY, & THEN MOVE THE INST. TO BE
;SIMULATED INTO A 3 BYTE AREA IN RAM AND EXECUTE IT
;FOLLOWED BY A JUMP INST WHICH RETURNS CONTROL TO THE
;ROUTINE.  A SIMULATOR SIMILAR TO THIS WAS DESCRIBED IN
;DETAIL IN SEPT. 77 KILOBAUD PG. 64.  HOWEVER ALTHOUGH
;THE PRINCIPLE IS THE SAME, THE CODE IS QUITE DIFFERENT.
;THE SIMULATOR ENTRY POINT IS AT F75C.
;
;COME HERE TO "SIMULATE" A SINGLE INSTRUCTION
;
LF400:  LXI     H,0             ;GET TWO ZEROS
        SHLD    LD14C+1         ;CLEAR 2ND & 3RD INST BYTES
        LHLD    LD13C           ;GET PGM COUNTER
        MOV     A,M             ;GET NEXT INST TO SIMULATE
        STA     LD14C           ;SAVE IT IN INST REG
        MOV     B,A             ;AND IN B
        CPI     40H             ;TRYING TO DETERMINE TYPE INST
        JC      LF422           ;FOR FURTHER TESTING
        CPI     0C0H            ;MORE TESTING
        JNC     LF475           ;INST >= C0 HEX
        JMP     LF459           ;1 BYTE ACCUM INST
;
;RETURN HERE TO PROCESS 1 BYTE ACCUM. INST
;
LF41B:  INX     H               ;INR PGM COUNTER
        SHLD    LD13E           ;SAVE ADDR OF NEXT INST
        JMP     LF634           ;PROCESS THE INST
;
;GET HERE FOR INST WITH OPCODES LESS THAN 40H
;
LF422:  ANI     7               ;MASK LOW 3 BITS
        JZ      LF41B           ;NOP IS ONLY LEGAL 8080 INST
        CPI     3               ;MASK LOW ORDER TWO BITS
        JC      LF439           ;01,02,09,0A,..,31,32,39,3A ETC
        CPI     6               ;CHECK FOR MVI
        JNZ     LF41B           ;JMP IF NOT MVI
;
;COME HERE FOR ACCUMULATOR IMMEDIATE INST (MVI, ADI, CPI, ETC)
;
LF431:  INX     H               ;POINT TO 2ND BYTE
        MOV     A,M             ;GET IT
        STA     LD14C+1         ;SAVE AS 2ND BYTE TO SIMULATE
        JMP     LF41B           ;SIMULATE INST. EXECUTION
;
;COME HERE FOR LXI, STAX, DAD, LDAX, SHLD, LHLD, STA, LDA
;
LF439:  CPI     2               ;STAX/LDAX,SHLD/LHLD,STA/LDA
        MOV     A,B             ;RESTORE THE FULL OPCODE
        JZ      LF451           ;FOR LIST 2 INST PREV.
        ANI     8               ;CHECK FOR DAD
        JNZ     LF41B           ;PROCESS DAD INST
LF444:  INX     H               ;POINT TO 2ND BYTE
        MOV     A,M             ;GET IT
        STA     LD14C+1         ;PUT IT IN SIM'S INST REG
        INX     H               ;POINT TO 3RD BYTE
        MOV     A,M             ;GET IT
        STA     LD14C+2         ;PUT IT IN AS 3RD BYTE
        JMP     LF41B           ;SIMULATE EXECUTION OF THE INST
;
;COME HERE FOR STAX, LDAX, SHLD, LHLD, STA, LDA
;
LF451:  CPI     20H             ;DIVIDE GROUP IN 2 PARTS
        JNC     LF444           ;FOR SHLD/LHLD, STA/LDA
        JMP     LF41B           ;FOR LDAX/STAX
;
;GET HERE FOR 1 BYTE ACCUM. INST > 40H AND < C0H
;MOV,CMP,ANA,ADD,ADC,SUB,SBB,XRA,ORA AND HLT
;
LF459:  CPI     76H             ;HALT INST ?
        JZ      LF461           ;YES - PROCESS IT
        JMP     LF41B           ;NO - PROCESS OTHERS
;
;PROCESS HALT INST
;
LF461:  CALL    CRLF            ;DO CR/LF
        CALL    LF6B8           ;PRINT OUT SIMULATORS P.C.
        LXI     H,LF470         ;POINT TO HALT MESSAGE
        CALL    SCRN            ;PRINT "HALT"
        JMP     LF774           ;AND THEN RESTART SIMULATOR
LF470:  DB      'HALT',0DH
;
;COME HERE FOR INST WITH OPCODES >= C0 HEX
;
LF475:  ANI     7               ;MASK LOW 3 BITS
        JZ      LF49F           ;CONDITIONAL RETURN
        CPI     2               ;CONDITIONAL JUMP ?
        JZ      LF4CA           ;YES - PROCESS CNDX JMP
        JC      LF4B0           ;POP, RET, PCHL, SPHL
        CPI     4               ;CONDITIONAL CALL ?
        JZ      LF505           ;YES - PROCESS IT
        JC      LF4EF           ;NO-JMP,IN,OUT,EI,DI,XTHL,XCHG
        CPI     6               ;ACCUM. IMMEDIATE ?
        JZ      LF431           ;YES - PROCESS
        JC      LF542           ;PUSH, CALL & ILLEGAL
;
;GET HERE TO PROCESS RST INSTRUCTIONS
;
        XRA     A               ;GET A ZERO
        STA     LD13E+1         ;CLEAR HIGH ORDER CALLED ADDR
        MOV     A,B             ;RESTORE OPCODE TO A
        ANI     38H             ;MASK RST TYPE
        STA     LD13E           ;STORE AS LOW ORDER BYTE
        JMP     LF511           ;PROCESS AS NORMAL CALL
;
;COME HERE FOR CONDITIONAL RETURN
;
LF49F:  CALL    LF551           ;OPERATE ON OPCODE & FLAG REG
        JNZ     LF4DA           ;CNDX NOT MET,CLEAN UP, GO HOME
;
;COME HERE TO PROCESS RETURN
;
LF4A5:  XCHG                    ;SAVE PGM CTR IN D,E
        LHLD    LD140           ;GET STACK POINTER H,L
        CALL    LF57F           ;POP STK & SAVE AS NEXT ADDR
        INX     H               ;INR STACK POINTER FOR 2ND BYTE
        JMP     LF521           ;RESET STK PTR & GO TO NXT INST
;
;COME HERE FOR POP, RET, PCHL, SPHL
;
LF4B0:  MOV     A,B             ;RESTORE INST TO A
        ANI     8               ;POP INST ?
        JZ      LF41B           ;YES - PROCESS IT
        MOV     A,B             ;NO - RESTORE INST AGAIN
        CPI     0D9H            ;ILLEGAL OPCODE ?
        JZ      LF589           ;YES - PROCESS ILLEGAL OPCODE
        JC      LF4A5           ;NO - RETURN INST IF CARRY
        CPI     0E9H            ;NOT RET - PCHL ?
        JNZ     LF41B           ;NO - MUST BE SPHL
        LHLD    LD144           ;PCHL - GET H,L
        JMP     LF4DB           ;MAKE ADDR OF NEXT INST & RET
;
;COME HERE FOR CONDITIONAL JUMP
;
LF4CA:  CALL    LF551           ;EVALUATE CONDITION
;
;GET HERE FOR STD JUMP
;
LF4CD:  CALL    LF4E1           ;FILL IN 2ND & 3RD BYTES
        JNZ     LF4D9           ;JMP IF CNDX NOT MET
        CALL    LF57F           ;UPDATE ADDR OF NXT INST
        JMP     LF66A           ;DONE W/SIMULATED JUMP
LF4D9:  INX     H               ;POINT TO 3RD BYTE
LF4DA:  INX     H               ;POINT TO NEXT INST
LF4DB:  SHLD    LD13E           ;SAVE H,L AS ADDR OF NXT INST
        JMP     LF66A           ;DONE WITH THIS INST
;
;UPDATE 2ND & 3RD BYTES OF INST AT D14C
;DONE EVEN IF INST IS NOT REALLY EXECUTED SO DISPLAY
;OF INST WILL BE CORRECT.
;NOTE REVERSAL OF "BACKWARD" 8080 ADDRESSES
;
LF4E1:  PUSH    PSW             ;SAVE PSW
        INX     H               ;POINT TO INST 2ND BYTE
        MOV     A,M             ;GET IT
        STA     LD14C+2         ;SAVE AS SIM 2ND (3RD?) BYTE
        INX     H               ;POINT TO INST 3RD BYTE
        MOV     A,M             ;GET IT
        STA     LD14C+1         ;SAVE AS SIM 3RD (2ND?) BYTE
        DCX     H               ;POINT BACK AT 2ND INST BYTE
        POP     PSW             ;RESTORE PSW
        RET                     ;DONE
;
;GET HERE FOR JMP, OUT, IN, XTHL, XCHG, EI, DI INSTRUCTIONS
;
LF4EF:  MOV     A,B             ;GET WHOLE INST
        ANI     38H             ;MASK
        JZ      LF4CD           ;JUMP INST
        CPI     10H             ;
        JZ      LF5F2           ;OUT INST
        JC      LF589           ;ILLEGAL OPCODE
        CPI     18H             ;MASK AGAIN
        JZ      LF58F           ;INPUT INST
        JMP     LF41B           ;EI, DI, XTHL, XCHG
;
;COME HERE FOR CONDITIONAL CALL INST
;
LF505:  CALL    LF551           ;EVALUATE CONDITION
        JNZ     LF4CD           ;JUMP IF NOT MET
;
;GET HERE FOR CALL INST
;
LF50B:  CALL    LF4E1           ;UPDATE DISPLAY INST
        CALL    LF57F           ;CALLED ADDR ==> NXT INST
LF511:  INX     H               ;POINT TO INST AFTR CALL
        XCHG                    ;SAVE RET ADDR IN D,E
        LDA     LD135           ;GET REAL-TIME RUN FLAG
        ORA     A               ;SET FLAGS
        JNZ     LF527           ;JUMP IF SIMULATING
LF51A:  LHLD    LD140           ;GET STACK POINTER
        DCX     H               ;POINT TO NEXT ENTRY 1ST BYTE
        MOV     M,D             ;PUT HIGH ORDER RET ADDR IN
        DCX     H               ;NOW DO LOW ORDER BYTE
        MOV     M,E             ;RET ADDR ON STACK
LF521:  SHLD    LD140           ;UPDATE STACK POINTER
        JMP     LF66A           ;SIMULATED CALL DONE
LF527:  LHLD    LD13E           ;GET CALLED ADDR
        LDA     LD13A           ;GET LOW ORDER R.T. RUN ADDR
        CMP     L               ;SAME ?
        JNZ     LF51A           ;NO - SIMULATE CALL
        LDA     LD13A+1         ;GET HIGH ORDER R.T. RUN ADDR
        CMP     H               ;SAME ?
        JNZ     LF51A           ;NO - SIMULATE CALL
        SHLD    LD14C+1         ;SAME - PUT CALL ADDR IN
        XCHG                    ;RET ADDR TO H,L
        SHLD    LD13E           ;NEXT SIM. ADDR TO EXEC
        JMP     LF634           ;EXECUTE CALL FOR REAL
;
;GET HERE FOR PUSH, CALL & ILLEGAL OPCODES
;
LF542:  MOV     A,B             ;RESTORE INSTRUCTION
        ANI     8               ;MASK
        JZ      LF41B           ;PUSH, POP
        MOV     A,B             ;RESTORE OPCODE AGAIN
        CPI     0CDH            ;CALL ?
        JZ      LF50B           ;YES - PROCESS IT
        JMP     LF589           ;ILLEGAL OPCODE
;
;GET HERE TO EVALUATE CONDITION FOR CONDITIONAL JMP, CALL, RET
;
LF551:  MOV     A,B             ;RESTORE OP CODE
        ANI     8               ;MASK CNDX TRUE/FALSE BIT
        MOV     C,A             ;SAVE TRUE/FALSE STATUS
        JZ      LF55A           ;JUMP IF NZ,NC,PO,P
        MVI     C,0C5H          ;PSW COND FLAG MASK
LF55A:  MOV     A,B             ;RESTORE OPCODE AGAIN
        ANI     30H             ;ZERO FLAG CONDITIONAL ?
        JNZ     LF565           ;NO - TEST OTHERS
        MVI     D,40H           ;PSW ZERO FLAG MASK
        JMP     LF579           ;EVALUATE THE CONDITION
LF565:  CPI     20H             ;PARITY/SIGN FLAG CONDITION ?
        JZ      LF572           ;YES - PARITY FLAG CNDX
        JNC     LF577           ;YES - SIGN FLAG CNDX
        MVI     D,1             ;CARRY FLAG MASK
        JMP     LF579           ;EVALUATE CONDITION
LF572:  MVI     D,4             ;PARITY FLAG BIT MASK
        JMP     LF579           ;EVALUATE THE CONDITION
LF577:  MVI     D,80H           ;SIGN FLAG BIT MASK
LF579:  LDA     LD142           ;GET SIMULATOR'S FLAG REGISTER
        XRA     C               ;SET CNDX & T/F INTO ACC
        ANA     D               ;MASK OUT PROPER FLAG BIT
        RET                     ;AND GO BACK W/ZERO FLAG SET
;
;SAVE ADDR POINTED TO BY H,L AS NEXT INST TO EXECUTE
;
LF57F:  MOV     A,M             ;GET 1ST BYTE
        STA     LD13E           ;SAVE IT AS NEXT INST LOW BYTE
        INX     H               ;POINT TO 2ND BYTE
        MOV     A,M             ;GET IT
        STA     LD13E+1         ;SAVE AS HIGH BYTE
        RET                     ;DONE
;
;ROUTINE TO PROCESS ILLEGAL OPCODES
;
LF589:  CALL    LF6CA           ;DISPLAY INST & ALL REGS
        CALL    LF949           ;PRINT ?, RESTART (NEVER RET)
;
;ROUTINE TO PROCESS INPUT INST
;
LF58F:  INX     H               ;POINT TO PORT
        MOV     A,M             ;GET IT
        INX     H               ;POINT TO NEXT INST
        SHLD    LD13E           ;AND MAKE IT NEXT TO EXEC
        STA     LD14C+1         ;PUT PORT INTO SIM'S INST REG
        CALL    LF8A1           ;SEARCH INPUT PORT TABLE
        JNZ     LF5AE           ;JUMP IF PORT NOT IN TABLE
        MOV     A,M             ;GET PORT I/O TYPE
        ORA     A               ;SET FLAGS
        JZ      LF5AE           ;IF PORT SIMULATED
        JM      LF634           ;TO REALLY EXEC I/O INST
        INX     H               ;ELSE PRESET, POINT TO VALUE
        MOV     A,M             ;GET IT
        STA     LD14A           ;TO SIM'S ACCUM (WHERE ELSE ?)
        JMP     LF66A           ;TO DISPLAY & DO NEXT INST
;
;COME HERE TO EXECUTE SIMULATED INPUT INST
;
LF5AE:  LXI     D,LF5D2         ;POINT TO INPUT MESSAGE
        CALL    LF5D9           ;PRINT PGM CTR & MESSAGE
        CALL    READ            ;GET USER'S ANSWER
        CALL    ZBUF            ;CLEAR ABUF
        LXI     H,IBUF          ;POINT TO INPUT BUFFER
        CALL    SBL1            ;SCAN PAST SPACES
        CALL    NUMS            ;CONVERT TO BINARY
        JC      LF949           ;IF ERROR IN ANSWER
        MOV     A,H             ;GET HIGH ORDER BYTE
        ORA     A               ;SET FLAGS
        JNZ     LF5AE           ;MAKE USER DO IT AGAIN IF > 255
        MOV     A,L             ;GET LOW ORDER BYTE
        STA     LD14A           ;STUFF IT INTO SIM'S ACCUM
        JMP     LF626           ;DO CR/LF & FINSIH UP
;
LF5D2:  DB      'INPUT '
        DB      0DH
;
;ROUTINE TO PRINT INPUT OR OUTPUT MESSAGE FOR SIMULATED I/O
;
LF5D9:  CALL    CRLF            ;DO CR/LF
        PUSH    D               ;SAVE D,E
        CALL    LF6B8           ;PRINT PGM COUNTER & SPACES
        POP     D               ;RESTORE REGS
        XCHG                    ;MESSAGE ADDR TO H,L
        CALL    SCRN            ;PRINT INPUT OR OUTPUT MSG
        LHLD    LD13C           ;GET SIM'S PGM CTR
        INX     H               ;INCR, POINTING TO PORT
        MOV     A,M             ;GET PORT IN ACCUM
        CALL    DUMO            ;PRINT PORT
        MVI     B,'='           ;FOLLOWED BY EQUALS SIGN
        JMP     OUT8            ;AND THEN RETURN
;
;ROUTINE TO PROCESS OUTPUT INSTRUCTION
;
LF5F2:  INX     H               ;POINT TO PORT
        MOV     A,M             ;GET IT
        INX     H               ;POINT TO NEXT INST
        SHLD    LD13E           ;SAVE AS ADDR OF NEXT INST
        STA     LD14C+1         ;PUT PORT IN SIM'S INST REG
        CALL    LF897           ;SEARCH OUTPUT TABLE
        MVI     A,1             ;ASSUME SIMULATED
        JNZ     LF604           ;IF PORT NOT IN TABLE
        MOV     A,M             ;GET TYPE OUTPUT
LF604:  ORA     A               ;SET FLAGS
        JM      LF634           ;TO REALLY EXECUTE OUT INST
        PUSH    PSW             ;SAVE ACCUM & FLAGS
        LXI     D,LF62C         ;POINT TO OUTPUT MESSAGE
        CALL    LF5D9           ;PRINT IT
        POP     PSW             ;RESTORE FLAGS & ACCUM
        ORA     A               ;SET FLAGS
        LDA     LD14A           ;GET CONTENTS OF SIM'S ACCUM
        MOV     B,A             ;MOVE TO B
        JZ      LF61E           ;ASCII MODE (UNLESS CNTL CHAR)
LF618:  CALL    LF747           ;PRINT ACC USING CUR MODE
        JMP     LF626           ;DO CR/LF, PRINT REGS & DO NEXT
LF61E:  CPI     20H             ;TEST FOR CONTROL CHAR
        JC      LF618           ;PRT USING CUR MODE, NOT ASCII
        CALL    OUT8            ;NOT CNTL CHAR, OK TO OUTPUT
LF626:  CALL    CRLF            ;NOW DO CR/LF TO MAKE IT PRETTY
        JMP     LF66A           ;THEN PRINT REGS & DO NEXT INST
;
LF62C:  DB      'OUTPUT '
        DB      0DH
;
;COME HERE TO EXECUTE AN INST.
;LOAD REAL 8080 REGS FROM SIMULATORS REGS, EXEC INST
;AND THEN UNLOAD 8080'S REGS TO SIMULATORS REGS
;
LF634:  LXI     SP,LD140        ;POINT STACK AT SIMULATORS REGS
        MVI     A,0C3H          ;JUMP INST
        STA     LD14F           ;STORE JUST AFTER INST TO SIMUL
        LXI     H,LF652         ;RETURN POINT AFTER INST EXECUT
        SHLD    LD14F+1         ;STORE IT JUST AFTER JUMP INST
        POP     H               ;LOAD 8080 H,L FROM SIM STK PTR
        POP     PSW             ;LOAD 8080 PSW FROM SIMULATOR
        INX     SP              ;SKIP L FOR NOW
        INX     SP              ;SKIP H FOR NOW
        POP     D               ;LOAD 8080 D,E FROM SIMULATOR
        POP     B               ;LOAD 8080 B,C FROM SIMULATOR
        SPHL                    ;MOVE SIM STACK TO 8080 STACK
        LHLD    LD144           ;LOAD 8080 H,L FROM SIM H,L
        LDA     LD14A           ;GET SIMULATED ACCUM
        JMP     LD14C           ;EXECUTE INST TO BE SIMULATED
;
;RETURN HERE AFTER EXECUTING THE "SIMULATED" INST
;
LF652:  STA     LD14A           ;SAVE 8080 ACCUM INTO SIMULTR
        SHLD    LD144           ;SAVE 8080 H,L INTO SIMULATOR
        RAL                     ;SAVE CARRY FLAG DURING DAD
        LXI     H,0             ;SETTING UP ....
        DAD     SP              ;TO GET THE STACK POINTER
        LXI     SP,LD14A        ;POINT CPU STACK TO REG AREA
        PUSH    B               ;SAVE 8080 B,C INTO SIMULATOR
        PUSH    D               ;SAVE 8080 D,E INTO SIMULATOR
        DCX     SP              ;SKIP H (ALREADY STORED)
        DCX     SP              ;SKIP L (ALREADY STORED)
        RAR                     ;RESTORE CARRY FLAG
        PUSH    PSW             ;SAVE 8080 PSW INTO SIMULATOR
        PUSH    H               ;SAVE 8080 STK PNTR INTO SIM
        LXI     SP,LD12B        ;RELOAD 8080'S STACK POINTER
LF66A:  IN      0FFH            ;GET SENSE SWITCHES
        ANI     80H             ;MASK HIGH ORDER BIT
        JNZ     LF697           ;FORCE RETURN TO COMMAND MODE
        CALL    STAT            ;SEE IF KEYBOARD CHAR WAITING
        JZ      LF67F           ;NO - CONTINUE PROCESSING
        CALL    IN8             ;YES - GET CHAR
        CPI     'X'-40H         ;CONTROL-X ?
        JZ      LF697           ;YES - FORCE RET TO COMND MODE
LF67F:  LDA     LD134           ;GET BREAKPOINT FLAG
        ORA     A               ;SET FLAGS
        JZ      LF69D           ;JMP IF NO BKPT SET
        LHLD    LD138           ;BKPT ADDR TO H,L
        LDA     LD13C+1         ;HIGH BYTE OF SIM P.C.
        CMP     H               ;COMPARE TO BKPT
        JNZ     LF6AF           ;BKPT NOT REACHED
        LDA     LD13C           ;GET LOW BYTE OF P.C.
        CMP     L               ;COMPARE TO BKPT
        JNZ     LF6AF           ;BKPT NOT REACHED
;
;COME HERE WHEN BREAKPOINT IS REACHED
;
LF697:  CALL    LF6CA           ;PRINT REGS, P.C., ETC.
        JMP     LF774           ;RESTART SIMULATOR
;
;GET HERE AFTER EA. INST IN SIMULATED CONT. RUN MODE
;
LF69D:  CALL    LF6CA           ;PRINT REGS, PC, ETC
        IN      0FFH            ;GET SENSE SWITCHES
        ANI     40H             ;MASK CONTINUOUS RUN BIT
        CZ      IN8             ;IF CONT RUN, CK KYBD PORT
        CPI     'X'-40H         ;CONTROL-X WAITING ?
        JZ      LF774           ;YES - RETURN TO SIM COMND MODE
;
;SIMULATOR "GO" COMMAND
;
LF6AC:  CALL    CRLF            ;DO CR/LF
LF6AF:  LHLD    LD13E           ;GET ADDR OF INST TO SIMULATE
        SHLD    LD13C           ;AND SAVE IT AS SIM PGM COUNTER
        JMP     LF400           ;SIMULATE THE INSTRUCTION
;
;ROUTINE TO PRINT OUT SIMULATORS P.C. AND TWO SPACES
;
LF6B8:  LDA     LD13C+1         ;GET HIGH ORDER SIM P.C. BYTE
        MOV     D,A             ;TO D
        LDA     LD13C           ;NOW LOW ORDER BYTE
        MOV     E,A             ;D,E HAVE P.C.
        CALL    ADOUT           ;PRINT IT IN HEX
        CALL    LE35C           ;PRINT SPACE
        CALL    LE35C           ;PRINT SPACE
        RET                     ;DONE
;
;ROUTINE TO PRINT OUT SIMULATORS PC AND ALL FLAGS & REGS
;
LF6CA:  CALL    LF6B8           ;PRINT PC & SPACES
        LDA     LD142           ;GET FLAG REGISTER
        MOV     C,A             ;SAVE IT IN C
        ANI     01H             ;MASK CARRY BIT
        CALL    LF73C           ;PRINT A 0 OR 1
        ANI     80H             ;MASK SIGN BIT
        CALL    LF73C           ;PRINT A 0 OR 1
        ANI     10H             ;MASK AUX CARRY BIT
        CALL    LF73C           ;PRINT A 0 OR 1
        ANI     04H             ;MASK PARITY BIT
        CALL    LF73C           ;PRINT A 0 OR 1
        ANI     40H             ;MASK ZERO BIT
        CALL    LF73C           ;PRINT A 0 OR 1
        CALL    LE35C           ;PRINT A SPACE
        LXI     D,LD14A         ;POINT TO ACCUM
        LDAX    D               ;GET IT
        CALL    LF756           ;PRINT IT USING CUR. MODE
        CALL    LE35C           ;PRINT SPACE
        MVI     C,04H           ;NO OF REGS TO OUTPUT
LF6F9:  DCX     D               ;POINT TO NEXT REG
        LDAX    D               ;GET IT
        CALL    LF756           ;PRINT IT USING CUR MODE
        DCR     C               ;NO OF REGS LEFT
        JNZ     LF6F9           ;DO NEXT IF NOT DONE
        CALL    LE35C           ;PRINT SPACE
        DCX     D               ;POINT TO H
        LDAX    D               ;GET IT
        CALL    LF756           ;PRINT IT
        DCX     D               ;POINT TO L
        LDAX    D               ;GET IT
        CALL    LF756           ;PRINT IT
        LHLD    LD144           ;GET H,L
        MOV     A,M             ;GET "M" CHAR
        CALL    LF756           ;PRINT IT
        CALL    LE35C           ;SPACE
        LDA     LD140+1         ;STACK PTR HIGH BYTE
        MOV     D,A             ;TO D
        LDA     LD140           ;STACK PTR LOW BYTE
        MOV     E,A             ;D,E HAS STK PTR ADDR
        CALL    ADOUT           ;PRINT STACK POINTER
        LXI     D,LD14C         ;POINT TO CURRENT INST
        CALL    LE35C           ;PRINT SPACE
        CALL    LF733           ;PRINT BYTE & SPACE
        CALL    LF733           ;PRINT BYTE & SPACE
        JMP     LF733           ;A BIT OF SLOPPY CODE
LF733:  CALL    LE35C           ;PRINT A SPACE
        LDAX    D               ;GET BYTE POINTED TO BY D,E
        INX     D               ;POINT TO NEXT
        CALL    LF747           ;PRINT THE BYTE
        RET                     ;DONE
;
;PRINT A 1 OR A ZERO (USED TO PRINT FLAG BYTES)
;
LF73C:  MVI     B,'1'           ;ASSUME 1
        JNZ     LF742           ;MUST BE IF IT'S NOT ZERO
        DCR     B               ;NOT 1, MAKE IT 0
LF742:  CALL    OUT8            ;PRINT IT, WHATEVER
        MOV     A,C             ;RESTORE THE FLAG BYTE IN A
        RET                     ;DONE
;
;PRINT A BYTE USING CURRENT SIMULATOR MODE
;
LF747:  LXI     H,LD14B         ;POINT TO SIMULATOR MODE
        MOV     B,M             ;GET IT
        DCR     B               ;SETTING FLAGS
        JP      LF753           ;IF MODE WASN'T ZERO
        CALL    DUMO            ;USE ALS-8 MODE
        RET                     ;DONE
LF753:  JMP     DOUT            ;USE DECIMAL
;
;ROUTINE TO PRINT BYTE FOLLOWED BY SPACE
;
LF756:  CALL    LF747           ;PRINT THE BYTE
        JMP     LE35C           ;THEN DO SPACE & RETURN
;
;THIS IS THE SIMULATOR MAIN ENTRY POINT
;
SIMU:   LDA     IBUF+4          ;GET CHAR AFTER 'SIMU'
        CPI     ' '+1           ;COMPARE TO BLANK
        JNC     LF774           ;SKIP INIT. IF NOT BLANK
        XRA     A               ;ELSE GET A ZERO
        STA     LD136           ;CLEAR INPUT PORT ASSIGN FLAG
        STA     LD14B           ;CLEAR SIMULATOR MODE
        STA     LD137           ;CLEAR OUTPUT PORT ASSIGN FLAGS
        STA     LD135           ;CLEAR REAL TIME RUN FLAG
LF771:  STA     LD134           ;CLEAR BRKPT FLAG
;
;WARM RESTART ENTRY POINT
;
LF774:  LXI     SP,SMODE        ;SET STACK
        CALL    CRLF            ;DO CR/LF
        MVI     B,'*'           ;PROMPT
        CALL    OUT8            ;PRINT IT
        CALL    READ            ;GET A COMND LINE IN IBUF
        LDA     IBUF            ;GET 1ST CHAR
        CPI     'P'             ;SET PGM COUNTER ?
        JNZ     LF796           ;NO - TRY OTHER COMMANDS
        CALL    LF932           ;GET OPERAND VALUE TO H,L
        SHLD    LD13E           ;? - THIS IS NOT THE SIM P.C.
        SHLD    LD13C           ;THE SIMULATORS PGM. COUNTER
        JMP     LF774           ;GET NEXT COMMAND
LF796:  CPI     'B'             ;SET BREAKPOINT ?
        JNZ     LF7A4           ;NO - TRY ANOTHER COMMAND
        CALL    LF932           ;GET ADDR TO H,L
        SHLD    LD138           ;SET BREAKPOINT ADDR
        JMP     LF771           ;SET BKPT FLAG & RESTART
LF7A4:  CPI     'R'             ;SET REAL-TIME RUN ADDR ?
        JNZ     LF7B7           ;NO - TRY ANOTHER COMMAND
        CALL    LF932           ;GET ADDR TO H,L
        SHLD    LD13A           ;SET REAL-TIME RUN ADDR
        MVI     A,1             ;GET A ONE
        STA     LD135           ;AND USE IT TO SET R.T. RUN FLG
        JMP     LF774           ;RESTART & GET NEXT COMMAND
;
;TEST FOR AND PROCESS THE CLEAR COMMANDS
;
LF7B7:  CPI     'C'             ;CLEAR COMMAND ?
        JNZ     LF7DE           ;NO - TRY OTHER COMMANDS
        LDA     IBUF+1          ;YES - GET NEXT CHAR
        LXI     H,LD134         ;POINT TO BKPT FLAG
        CPI     'B'             ;CLEAR BREAKPOINT ?
        JZ      LF7D9           ;YES - DO IT
        INX     H               ;NO, POINT TO R.T. RUN FLAG
        CPI     'R'             ;CLEAR R.T. RUN ADDR ?
        JZ      LF7D9           ;YES - DO IT
        INX     H               ;NO, POINT TO INPT PT ASGN FLAG
        CPI     'I'             ;CLEAR INPUT PORT ASSIGNMENTS ?
        JZ      LF7D9           ;YES - DO IT
        INX     H               ;NO, POINT TO OUTP PT ASGN FLAG
        CPI     'O'             ;CLEAR OUTPUT PORT ASSGNMENTS ?
        JNZ     LF949           ;NO - INVALID CLEAR COMMAND
LF7D9:  MVI     M,0             ;CLEAR FLG H,L POINT TO
        JMP     LF774           ;AND RESTART SIMULATOR
LF7DE:  CPI     'G'             ;GO COMMAND ?
        JZ      LF6AC           ;YES - DO IT
        CPI     'D'             ;DUMP COMMAND ?
        JNZ     LF7F1           ;NO - TRY NEXT COMMAND
        CALL    LF932           ;GET ADDR IN H,L
        CALL    DUMP+3          ;AND USE ALS-8 DUMP CODE
        JMP     LF774           ;THEN RESTART THE SIMULATOR
LF7F1:  CPI     'E'             ;ENTER COMMAND ?
        JNZ     LF802           ;NO, TEST OTHER COMMANDS
        CALL    LF932           ;GET ADDRESS TO H,L
        CALL    ENTS            ;USE ALS-8 ENTER CODE
        JC      LF949           ;IF ERROR
        JMP     LF774           ;ELSE RESTART
LF802:  CPI     'X'             ;EXIT SIMULATOR COMMAND ?
        JZ      EORMS           ;YES - RET TO ALS-8
        CPI     'M'             ;MODE COMMAND ?
        JNZ     LF81F           ;NO - TEST OTHER COMMANDS
        CALL    LF932           ;GET VALUE IN H,L
        MOV     A,L             ;NOW TO A
        MVI     B,1             ;ASSUME HEX
        CPI     16              ;HEX ?
        JZ      LF818           ;YES - SKIP NEXT STMNT
        DCR     B               ;MAKE IT 0 ==> OCTAL
LF818:  MOV     A,B             ;GET FLAG TO A
        STA     LD14B           ;AND UPDATE MODE
        JMP     LF774           ;THEN RESTART SIMULATOR
;
;PROCESS THE VARIOUS INPUT COMMANDS
;
LF81F:  CPI     'I'             ;SET INPUT COMMAND ?
        JNZ     LF863           ;NO - TRY OTHER COMMANDS
        CALL    LF932           ;GET VALUE TO H,L
        MOV     A,H             ;HIGH ORDER BYTE TO A
        ORA     A               ;SET FLAGS
        JNZ     LF949           ;ERROR IF > 256
        MOV     A,L             ;PORT TO A
        CALL    LF8A1           ;SEARCH INPUT PORT TABLE
        MOV     A,C             ;GET NO OF PORTS IN TABLE
        CPI     16              ;TABLE FULL ?
        JNC     LF949           ;ERROR THEN
        STA     LD136           ;ELSE UPDATE # OF TABLE ENTRIES
        MVI     M,0FFH          ;AND MAKE PORT REAL-TIME
        LDA     IBUF+1          ;GET CHAR AFTER 'I'
        CPI     'S'             ;PRESET ?
        JNZ     LF854           ;NO - TRY REAL-TIME & CLEAR
        XCHG                    ;SAVE TABLE POINTER IN D,E
        LHLD    BBUF+2          ;GET VALUE OF PRESET
        MOV     A,H             ;GET HIGH BYTE
        ORA     A               ;SET FLAGS
        JNZ     LF949           ;ERROR IF > 255
        XCHG                    ;TABLE POINTER BACK TO H,L
        MVI     M,1             ;SAY PRESET
        INX     H               ;MOVE 1 BEYOND INPUT TYPE
        MOV     M,E             ;AND STORE PRESET VALUE
        JMP     LF774           ;THEN RESTART SIMULATOR
LF854:  CPI     'R'             ;REAL-TIME INPUT ?
        JZ      LF774           ;RESTART SIMULATR - ALREADY SET
        MVI     M,0             ;ELSE ASSUME SIMULATED
        CPI     'C'             ;TEST FOR SIMULATED
        JZ      LF774           ;OK IF IT REALLY IS
        JMP     LF949           ;ELSE ERROR
;
;TEST FOR AND PROCESS THE OUTPUT COMMANDS
;
LF863:  CPI     'O'             ;SET OUTPUT INST ?
        JNZ     LF8BC           ;NO - TRY OTHER COMMANDS
        CALL    LF932           ;ELSE GET PORT NO IN H,L
        MOV     A,H             ;HIGH BYTE TO A
        ORA     A               ;SET FLAGS
        JNZ     LF949           ;ERROR IF > 255
        MOV     A,L             ;PORT TO A
        CALL    LF897           ;LOOK UP PORT IN TABLE
        MOV     A,C             ;GET NO OF PORTS IN TABLE
        CPI     16              ;TABLE FULL ?
        JNC     LF949           ;ERROR THEN
        STA     LD137           ;ELSE UPDATE # OF TABLE ENTRIES
        MVI     B,0FFH          ;ASSUME REAL-TIME
        LDA     IBUF+1          ;GET NEXT CHAR OF COMMAND
        CPI     'R'             ;REAL-TIME ?
        JZ      LF890           ;ENTER R.T. IN TABLE & RESTART
        INR     B               ;NOT R.T., ASSUME ASCII
        CPI     'A'             ;ASCII ?
        JZ      LF890           ;YES - PUT IN TABLE
        INR     B               ;NOT R.T. OR ASCII - SIMULATED?
        CPI     'C'             ;CLEAR (SIMULATED) ?
LF890:  MOV     M,B             ;SET OUTPUT MODE
        JZ      LF774           ;MODE SET, RESTART
        JMP     LF949           ;ERROR, SIMULATED ASSUMED
LF897:  LXI     H,LD162         ;POINT TO OUTPUT TABLE
        MOV     B,A             ;PORT NO TO B
        LDA     LD137           ;NO OF TABLE ENTRIES
        JMP     LF8A8           ;SEARCH TABLE
LF8A1:  LXI     H,LD152         ;POINT TO INPUT TABLE
        MOV     B,A             ;PORT NO TO B
        LDA     LD136           ;NO OF TABLE ENTRIES
LF8A8:  MOV     C,A             ;TABLE ENTRIES TO C
        ORA     A               ;SET FLAGS
        MOV     A,B             ;PORT NO BACK TO A
        MOV     B,C             ;NO OF ENTRIES TO B
        JZ      LF8B8           ;IF TABLE CURRENTLY EMPTY
LF8AF:  CMP     M               ;POINTING AT CORRECT PORT ?
        INX     H               ;POINT TO I/O TYPE FOR PORT
        RZ                      ;IF WE HAVE PORT
        INX     H               ;POINT TO I/O BYTE VALUE
        INX     H               ;AND TO NEXT PORT
        DCR     B               ;DCR NO OF TABLE ENTRIES LEFT
        JNZ     LF8AF           ;AND CHECK REMAINING ENTRIES
LF8B8:  MOV     M,A             ;PUT PORT IN TABLE
        INX     H               ;AND POINT TO I/O TYPE
        INR     C               ;INR NO OF ENTRIES IN TABLE
        RET                     ;AND RETURN DONE
;
;PROCESS THE SET REGISTER/FLAG COMMANDS
;
LF8BC:  CPI     'S'             ;SET REGISTERS ?
        JNZ     LF949           ;THERE AREN'T ANY COMMANDS LEFT
        LXI     H,IBUF+1        ;POINT TO NEXT BYTE
        SHLD    PNTR            ;AND SAVE CHAR POINTER
LF8C7:  CALL    ZBUF            ;CLEAR ABUF
        CALL    SBLK            ;SCAN TO TEXT
        JC      LF774           ;RESTART IF C/R FOUND
        XCHG                    ;LINE PTR TO D,E
        MVI     C,10            ;NO OF REGISTERS IN TABLE
        LXI     H,LF954         ;POINT TO TABLE
LF8D6:  CMP     M               ;SAME ?
        JZ      LF8E2           ;FOUND REGISTER TO SET
        INX     H               ;ELSE POINT TO NEXT REGISTER
        DCR     C               ;AND DCR COUNT
        JNZ     LF8D6           ;THEN TEST IT FOR MATCH
        JMP     LF949           ;NO MATCH IN TABLE, ERROR
LF8E2:  MOV     A,C             ;GET DISPLACEMENT INTO TABLE
        STA     TEMP            ;AND SAVE IT
        XCHG                    ;POINTER TO H,L
        INX     H               ;POINT TO NEXT CHAR
        MOV     A,M             ;GET IT
        CPI     '='             ;EQUALS SIGN ?
        JNZ     LF949           ;ERROR IF NOT
        INX     H               ;POINT TO ARGUMENT
        MOV     A,M             ;GET IT
        LXI     D,ABUF          ;POINT TO ABUF
        CALL    LF912           ;CONVERT ARGUMENT TO BINARY
        JC      LF949           ;IF ERROR
        LDA     TEMP            ;GET DISP. INTO TABLE BACK
        CPI     1               ;SETTING STACK POINTER ?
        JZ      LF92C           ;HANDLE THAT SEPARATELY
        MOV     C,A             ;DISPLACEMENT TO C
        MOV     A,H             ;GET HIGH ORDER BYTE OF VALUE
        ORA     A               ;SET FLAGS
        JNZ     LF949           ;ERROR IF > 255
        MOV     A,L             ;GET VALUE TO A
        LXI     H,LD140         ;REGISTER TABLE IN RAM
        MVI     B,0             ;B,C HAS DISPLACEMENT INTO TBL
        DAD     B               ;H,L HAS ABSOLUTE ADDR
        MOV     M,A             ;UPDATE SIM/8080 REGISTER
        JMP     LF8C7           ;DO NEXT REGISTER(S) IF ANY
;
;ROUTINE TO CONVERT KEYED IN VALUE TO 16 BIT VALUE IN H,L
;
LF912:  CALL    ALPS            ;PUT ARGUMENT INTO ABUF
        DCX     D               ;POINT TO LAST CHAR
        LDAX    D               ;GET IT
        LXI     B,ABUF          ;POINT TO ARGUMENT
        CPI     'D'             ;DECIMAL ?
        JZ      NUM1            ;YES
        CPI     'Q'             ;OCTAL ?
        JZ      NUM3            ;YES
        CPI     'H'             ;HEX ?
        JNZ     EMODE           ;NO, USE STD MODE
        JMP     NUM2            ;ELSE HEX
LF92C:  SHLD    LD140           ;UPDATE SIM/8080 STACK PNTR
        JMP     LF8C7           ;AND DO NEXT REGISTER(S) IF ANY
;
;ROUTINE TO SET SIMULATED PROGRAM COUNTER
;
LF932:  CALL    ETRA            ;GET VALUES FROM COMMAND LINE
        JC      LF949           ;IF ERROR
        CALL    LE31E           ;VALUES TO BIN IN BBUF & BBUF+2
        JC      LF949           ;ON ERROR
        LDA     ABUF            ;GET FIRST CHAR OF VALUE
        ORA     A               ;AND SET FLAGS
        JZ      LF949           ;ERROR IF NO VALUE GIVEN
        LHLD    BBUF            ;GET VALUE
        RET                     ;AND RETURN WITH IT IN H,L
;
;ERROR HANDLER ROUTINE
;
LF949:  CALL    CRLF            ;DO CR/LF
        MVI     B,'?'           ;GET QUESTION MARK
        CALL    OUT8            ;AND PRINT IT
        JMP     LF774           ;THEN RESTART SIMULATOR
;
;TABLE OF REGISTER NAMES FOR THE SET COMMANDS
;VALUES ARE STORED IN A TABLE IN MEMORY AT D143, BUT
;IN REVERSE ORDER.  THERE IS ONE ENTRY HERE FOR EACH
;ENTRY IN THAT TABLE.  NOTE THAT THE ACCUM. APPEARS
;TWICE, ONCE BY ITSELF & ONCE WITH THE FLAGS (AS PSW)
;
LF954:  DB      'A'
        DB      'B'
        DB      'C'
        DB      'D'
        DB      'E'
        DB      'H'
        DB      'L'
        DB      'A'
        DB      'F'
        DB      'S'
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
;
;COME HERE TO EXECUTE AUTO-NUMBER INPUT MODE
;
;WHAT THIS CODE ACTUALLY DOES IS TO MODIFY THE INPUT ROUTINE
;TO COME INTO A PRE-PROCESSOR AT F99A INSTEAD OF THE NORMAL
;IN8 INPUT ROUTINE.  THAT ROUTINE ACTUALLY DOES THE LINE
;NUMBER PROCESSING.
;
AUTO:   LHLD    IN8+1           ;GET ADDR OF CUR INPUT DRIVER
        SHLD    BBUF+1          ;AND SAVE IN BBUF
        LDA     ABUF            ;SEE IF OPT PARAMETER GIVEN
        ORA     A               ;SET FLAGS
        JZ      LF983           ;NOT GIVEN, START WITH 1
        LHLD    MAXL            ;GET HALF OF CUR MAX LINE NO
        SHLD    ABUF            ;SAVE IT
        LHLD    MAXL+2          ;GET OTHER HALF
        JMP     LF989           ;SKIP INITIALIZATION
LF983:  LXI     H,3030H         ;TWO ZERO'S
        SHLD    ABUF            ;SAVE AS TWO HIGH DIGITS
LF989:  SHLD    ABUF+2          ;AND SAVE H,L AS LOW ORDER DGTS
        MVI     A,0C3H          ;JUMP INST
        STA     BBUF            ;BBUF = JUMP INTO AUTO CODE NOW
        LXI     H,LF99A         ;GET ADDR OF LINE NO PROCESSOR
        SHLD    IN8+1           ;USE IT FOR NEW INPUT ROUTINE
        JMP     EORNS           ;EVERYTHING'S SET UP
;
;HERE IS THE ROUTINE WHICH ACTUALLY PUTS LINE NOS. ON LINES.
;IT IS SUBSTITUTED FOR THE STANDARD ROUTINE BY THE CODE ABOVE.
;
LF99A:  CALL    BBUF            ;CALL THE STD. INPUT ROUTINE
        MOV     A,L             ;GET LOW BYTE ADDR OF CHAR PTR
        CPI     IBUF AND 0FFH   ;AT 1ST CHAR OF INPUT BUFFER ?
        JNZ     LF9AE           ;NO - SKIP BEG OF LINE PROC.
        MVI     A,1BH           ;YES - SEE IF ESCAPE CHAR
        CMP     B               ;
        JZ      EORMS           ;IT IS ESC - DO COMPLETE RESET
        LXI     H,IBUF+5        ;POINT TO NEW CHAR DEST
        MVI     E,7             ;AND RESET LINE LENGTH
LF9AE:  MVI     A,0DH           ;GET ASCII CAR. RET.
        CMP     B               ;END OF LINE ?
        MOV     A,B             ;PUT CHAR IN ACCUM.
        RNZ                     ;AND RETURN IF NOT END OF LINE
        PUSH    H               ;ELSE SAVE IBUF POINTER
        PUSH    D               ;AND LINE LENGTH
        LXI     D,ABUF          ;POINT TO CUR LINE NO.
        LXI     H,ABUF+3        ;AND TO END OF LINE NO.
LF9BB:  MOV     A,M             ;GET RIGHT-HAND DIGIT
        INR     A               ;INCREMENT IT
        CPI     '9'+1           ;GONE PAST 9 ?
        JNC     LF9D8           ;YES - FIX IT UP
        MOV     M,A             ;ELSE PUT IT BACK
        LXI     H,IBUF          ;POINT TO IBUF
        MVI     B,4             ;LENGTH OF LINE NO
LF9C8:  LDAX    D               ;GET CHAR FROM LINE NO.
        MOV     M,A             ;AND PUT IT INTO IBUF
        INX     H               ;INR IBUF POINTER
        INX     D               ;INR LINE NO. POINTER
        DCR     B               ;DCR COUNT
        JNZ     LF9C8           ;AND CONTINUE TILL DONE
        MVI     M,' '           ;THEN PUT A SPACE IN IBUF
        POP     D               ;AND RESTORE REGISTERS
        MVI     B,0DH           ;AND CHARACTER
        MOV     A,B             ;WHICH READ WANTS IN ACCUM
        POP     H               ;RESTORE REGS
        RET                     ;AND GO HOME, DONE
LF9D8:  MVI     A,'0'           ;REPLACE DIGIT WITH A ZERO
        MOV     M,A             ;AND PUT IT BACK
        DCX     H               ;POINT TO HIGHER DIGIT
        JMP     LF9BB           ;AND TRY TO INR IT
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
        NOP
;
; ****END OF ALS8SIM MODULE
;
;
;THE TXT-2 TEXT EDITOR STARTS HERE BUT ENTRY IS AT TXT2 (FA0B)
;
;ROUTINE TO CREATE A ONE LINE FILE NECESSARY FOR THE
;EDITOR WHEN STARTING A NEW (E.G. NULL OR EMPTY) FILE
;
LFA00:  MVI     M,2             ;LENGTH BYTE
        INX     H               ;POINT TO NEXT BYTE
        MVI     M,0DH           ;CARRIAGE RETURN
        INX     H               ;POINT TO NEXT
        MVI     M,1             ;END OF FILE MARK
        SHLD    EOFP            ;NOW WE HAVE A NON-EMPTY FILE
;
;THIS IS THE EDITOR ENTRY POINT
;
TXT2:   LXI     SP,SMODE        ;SET STACK POINTER
        LHLD    BOFP            ;GET BEG OF FILE POINTER
        LXI     D,EOFP          ;AND POINT TO END OF FILE PTR
        CALL    LFD42           ;COMPARE THEM
        JZ      LFA00           ;CREATE ONE LINE FILE IF EMPTY
        JC      WHAT            ;ERROR IF BOFP > EOFP
        PUSH    H               ;SAVE BEG. OF FILE POINTER
        CALL    LFD4E           ;CLEAR SCREEN & INITIALIZE VARS
        SUB     A               ;GET A ZERO
        MOV     B,A             ;MAKE IT ROW
        CALL    LFF1A           ;CALC CUR ADDR & TURN IT ON
        POP     H               ;RESTORE BEG OF FILE PNTR
LFA27:  SHLD    LD19A           ;
        CALL    LFAD9
;
;THIS IS THE BEGINNING OF THE MAIN EDITOR COMMAND LOOP
;
LFA2D:  CALL    LFA33           ;GET AND EXECUTE A COMMAND
        JMP     LFA2D           ;DO IT AGAIN
;
;AND THAT WAS THE END OF IT
;
LFA33:  CALL    LFF13           ;CALC CUR ADDR & TURN CURSR ON
        CALL    IN8             ;GET A CHAR OF COMMAND
LFA39:  CPI     ' '             ;CONTROL CHAR ?
        JNC     LFC04           ;PROCESS PRINTING CHAR IF NOT
        CPI     'A'-40H         ;CURSOR LEFT COMMAND ?
        JZ      LFB57           ;PROCESS IF SO
        CPI     'S'-40H         ;CURSOR RIGHT COMMAND ?
        JZ      LFC10           ;PROCESS IF SO
        PUSH    PSW             ;SAVE COMMAND
        CALL    LFCA4
        POP     B               ;RESTORE COMMAND TO B
        LXI     H,LFA65         ;EDITOR COMMAND TABLE
LFA50:  MOV     A,M             ;GET TABLE COMMAND
        ORA     A               ;END OF TABLE ?
        RZ                      ;YES
        CMP     B               ;NO - SAME AS COMMAND ?
        INX     H               ;POINT TO COMMAND ADDR
        JZ      LFA60           ;DISPATCH TO IT ON MATCH
        INX     H               ;NO MATCH - POINT PAST COMMAND
        INX     H               ;TO NEXT COMMAND
        JMP     LFA50           ;AND TEST NEXT COMMAND
;
;ROUTINE TO DISPATCH TO COMMAND IN TABLE
;
LFA5D:  LXI     H,LD1A1         ;POINT TO UNDEFINED COMMAND
LFA60:  MOV     A,M             ;COMMAND LOW BYTE TO A
        INX     H               ;POINT TO HIGH BYTE
        MOV     H,M             ;HIGH BYTE TO H
        MOV     L,A             ;H,L NOW HAS COMPLETE ADDR
        PCHL                    ;DISPATCH TO COMMAND ROUTINE
;
;HERE IS THE COMMAND TABLE FOR THE EDITOR COMMANDS
;
LFA65:  DB      'W'-40H         ;UP
        DW      LFACC
        DB      'Z'-40H         ;DOWN
        DW      LFB62
        DB      'E'-40H         ;SCROLL UP ONE LINE
        DW      LFB8E
        DB      'X'-40H         ;SCROLL DOWN ONE LINE
        DW      LFAC6
        DB      'R'-40H         ;SCROLL UP 1 PAGE
        DW      LFB12
        DB      'C'-40H         ;SCROLL DOWN 1 PAGE
        DW      LFB21
        DB      'J'-40H         ;ERASE EOF & SCROLL UP (L/F)
        DW      LFB7B
        DB      'B'-40H         ;INSERT LINE
        DW      LFBC8
        DB      'M'-40H         ;SCROLL UP & INSERT LINE (C/R)
        DW      LFB9D
        DB      'T'-40H         ;TOGGLE INSERT MODE SW
        DW      LFB72
        DB      'P'-40H         ;DELETE LINE
        DW      LFBD2
        DB      'H'-40H         ;DELETE CHAR
        DW      LFC2E
        DB      'F'-40H         ;EXIT TO ALS-8
        DW      LFAA5
        DB      'O'-40H         ;EDITOR SEARCH
        DW      LFF58
        DB      'I'-40H         ;CONTINUE SEARCH
        DW      LFF64
        DB      'Y'-40H         ;REPEAT COMMAND
        DW      LFAAE
        DB      'Q'-40H         ;HOME COMMAND
        DW      LFA9C
        DB      'U'-40H         ;UNDEFINED COMMAND
        DW      LFA5D
        DB      0
;
;HOME CURSOR
;
LFA9C:  XRA     A               ;GET A ZERO
        STA     NCHAR           ;MAKE IT THE COLUMN
        MVI     A,6             ;HOME=LINE 7 (START W/0 AT TOP)
        JMP     LFAD6           ;PUT THE CURSOR THERE
;
;EXIT EDITOR TO ALS-8
;
LFAA5:  CALL    LFD4E           ;CLEAR VDM-1 SCREEN
        CALL    FCHK            ;FCHK FILE
        JMP     EORMS           ;GO BACK TO ALS-8
;
;REPEAT COMMAND
;
LFAAE:  CALL    IN8             ;GET COMMAND TO REPEAT
        STA     BBUF            ;SAVE COMMAND
        CALL    IN8             ;GET REPEAT COUNT
        SUI     30H             ;CONVERT TO BINARY
LFAB9:  PUSH    PSW             ;SAVE COUNT
        LDA     BBUF            ;GET COMMAND
        CALL    LFA39           ;EXECUTE IT ONE TIME
        POP     PSW             ;RESTORE COUNT
        DCR     A               ;DECREMENT COUNT
        RZ                      ;DONE IF COUNT=0
        JMP     LFAB9           ;ELSE DO IT AGAIN
;
;SCROLL DOWN 1 LINE
;
LFAC6:  CALL    LFC72
        JNZ     LFAD9
;
;CURSOR UP
;
LFACC:  LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        DCR     A
        JP      LFAD6
        LDA     LD199
LFAD6:  STA     SLINE
LFAD9:  LHLD    LD19A
        SHLD    LD19C
        XRA     A
        STA     LD198           ;CLEAR INSERT MODE
        PUSH    PSW
LFAE4:  STA     LD199
        XCHG
        LXI     H,SLINE
        CMP     M
        XCHG
        JNZ     LFAF3
        SHLD    LD19E
LFAF3:  CALL    LFE07
LFAF6:  POP     PSW
        INR     A
        CPI     16
        RZ
        MOV     B,A
        CALL    LFC5F
        XCHG
        MOV     A,B
        PUSH    PSW
        JNZ     LFAE4
        MVI     B,0
        MVI     C,'#'
        LXI     H,4001H
        CALL    LFE4E
        JMP     LFAF6
;
;SCROLL UP 1 PAGE (16 LINES)
;
LFB12:  MVI     A,16
LFB14:  PUSH    PSW
        CALL    LFC5F
        CNZ     LFB94
        POP     PSW
        DCR     A
        JNZ     LFB14
        RET
;
;SCROLL DOWN 1 PAGE (16 LINES)
;
LFB21:  MVI     A,16
LFB23:  PUSH    PSW
        CALL    LFC72
        CNZ     LFAD9
        POP     PSW
        DCR     A
        JNZ     LFB23
        RET
;
;ROUTINE TO GIVE WARNING WHEN TEXT BUFFER GETS FULL
;
LFB30:  LXI     D,LFB46         ;POINT TO WARNING MESSAGE
        MVI     C,17            ;LENGTH OF MESSAGE
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        CALL    LFE0C           ;PRINT MESSAGE
LFB3B:  CALL    IN8             ;GET CHAR FROM CONSOLE
        CPI     'Q'-40H         ;CONTROL-Q ?
        JNZ     LFB3B           ;NO - WAIT INDEFINATELY
        JMP     TXT2            ;YES - RESTART EDITOR
;
LFB46:  DB      'FULL--TYPE CTRL Q'
;
;CURSOR LEFT COMMAND
;
LFB57:  CALL    LFF2D           ;CURSOR OFF
        MOV     A,B             ;COLUMN TO A
        DCR     A               ;DECREMENT IT
LFB5C:  ANI     3FH             ;KEEP IT TO 0-63
        MOV     M,A             ;PUT IT BACK AS NEW COLUMN
        JMP     LFF16           ;TURN CURSOR ON & RET
;
;CURSOR DOWN
;
LFB62:  LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        LXI     H,LD199
        CMP     M
        JNZ     LFB6E
        MVI     A,0FFH
LFB6E:  INR     A
        JMP     LFAD6
;
;TOGGLE CHAR INSERT MODE
;
LFB72:  LDA     LD198           ;GET INSERT MODE SWITCH
        XRI     1               ;TOGGLE LOW ORDER BIT
        STA     LD198           ;PUT IT BACK
        RET                     ;DONE
;
;ERASE TO THE END OF LINE AND SCROLL UP
;
LFB7B:  CALL    LFDED           ;CALC CURSOR ADDR
LFB7E:  MVI     M,' '           ;MOVE SPACE TO LINE
        INR     L               ;INR LINE POINTER
        MOV     A,L             ;MOVE IT TO A
        ANI     3FH             ;END OF LINE YET ?
        JNZ     LFB7E           ;MORE SPACES IF NOT
        CALL    LFCA4           ;YES - NOW UPDATE FILE
        SUB     A               ;GET A ZERO
        STA     NCHAR           ;AND MAKE IT NEW COLUMN
;
;SCROLL UP 1 LINE
;
LFB8E:  CALL    LFC5F
        JZ      LFB62
LFB94:  LXI     H,LD19A
        CALL    LFC62
        JMP     LFAD9
;
;SCROLL UP AND INSERT 1 LINE
;
LFB9D:  LXI     H,LD19E
        CALL    LFC62
        JNZ     LFBAC
        LHLD    EOFP
        SHLD    LD19E
LFBAC:  CALL    LFC51
        XRA     A
        STA     NCHAR
        LDA     LD199
        CPI     0FH
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        JNZ     LFB6E
        LXI     H,LD19A
        CALL    LFC62
        JMP     LFAD9
;
        DB      01H             ;THIS IS A FOOL'S BYTE
;
;INSERT LINE COMMAND
;
LFBC8:  CALL    LFC51
        XRA     A
        STA     NCHAR
        JMP     LFAD9
;
;DELETE LINE COMMAND
;
LFBD2:  SUB     A               ;GET A ZERO
        STA     LD198           ;USE IT TO CLEAR INSERT MODE
        LHLD    BOFP
        MOV     A,M
        CALL    LFD32
        MOV     A,M
        CPI     1
        RZ
        CALL    LFC5F
        JNZ     LFBEA
        CALL    LFC72
LFBEA:  PUSH    PSW
        LHLD    LD19E
        MOV     A,M
        CMA
        INR     A
        CALL    LFCD1
        POP     PSW
        JNZ     LFAD9
        LHLD    LD19E
        MOV     A,M
        CPI     1
        JZ      LFACC
        JMP     LFAD9
;
;
;
LFC04:  MOV     C,A
        LDA     LD198           ;GET INSERT MODE FLAG
        ORA     A               ;SET CPU FLAGS
        JNZ     LFC18           ;JUMP IF IN INSERT MODE
        CALL    LFDED
        MOV     M,C
;
;CURSOR RIGHT COMMAND
;
LFC10:  CALL    LFF2D           ;TURN CURSOR OFF
        MOV     A,B             ;COLUMN TO A
        INR     A               ;INCREMENT IT
        JMP     LFB5C           ;CURSOR BACK ON & RET
;
;
;
LFC18:  CALL    LFDED
        MOV     A,C
        ORI     80H
        MOV     C,A
LFC1F:  MOV     A,M
        ANI     7FH
        MOV     M,C
        MOV     C,A
        INR     L
        MOV     A,L
        ANI     3FH
        JNZ     LFC1F
        JMP     LFC10
;
;DELETE CHARACTER
;
LFC2E:  CALL    LFDED           ;CALC CURSOR POSITION
        LDA     NCHAR           ;GET CURRENT COLUMN
        CPI     3FH             ;COLUMN 63 (LAST) ?
        JC      LFC3C           ;NO - DELETE CHAR
        MVI     M,' '+80H       ;MAKE LAST COL SPACE W/CURSOR
        RET                     ;DONE
;
;ROUTINE TO DELETE A CHAR FROM MIDDLE OF LINE
;
LFC3C:  MOV     A,L             ;GET LINE POINTER
        MOV     E,A             ;SAVE IT
        ORI     3FH             ;MAKE IT POINT TO LAST COL
        MOV     L,A             ;PUT IT BACK
        MOV     A,E             ;RESTORE ORIG PTR TO A
        MVI     C,' '           ;SPACE TO C
LFC44:  MOV     B,M             ;GET CHAR FROM LINE
        MOV     M,C             ;REPL WITH CHAR ON IT'S RIGHT
        MOV     C,B             ;AND MAKE IT NXT CHAR ON RIGHT
        DCR     L               ;DECR COLUMN POINTER
        CMP     L               ;AND COMPARE TO INITIAL COL
        JNZ     LFC44           ;AND KEEP UP IF NOT DONE
        MOV     A,B             ;GET LAST CHAR TO STORE IN A
;
;TURN ON THE CURSOR
;
LFC4D:  ORI     80H             ;MAKE CURSOR BIT HIGH
        MOV     M,A             ;PUT CHAR W/CURSOR ON SCREEN
        RET                     ;DONE, CURSOR ON
;
;
;
LFC51:  MVI     A,2
        CALL    LFCD1
        LHLD    LD19E
        MVI     M,2
        INX     H
        MVI     M,0DH
        RET
;
;
;
LFC5F:  LXI     H,LD19C
LFC62:  MOV     E,M
        INX     H
        MOV     D,M
        LDAX    D
        XCHG
        CALL    LFD32
        XCHG
        LDAX    D
        DCR     A
        RZ
        MOV     M,D
        DCX     H
        MOV     M,E
        RET
;
;
;
LFC72:  LHLD    LD19A
        CALL    LFC95
        RZ
        DCX     H
LFC7A:  DCX     H
        CALL    LFC95
        JZ      LFC90
        MOV     A,M
        CPI     0DH
        JNZ     LFC7A
        DCX     H
        MOV     A,M
        CPI     0DH
        JZ      LFC8F
        INX     H
LFC8F:  INX     H
LFC90:  INR     A
        SHLD    LD19A
        RET
;
;
;
LFC95:  LDA     BOFP
        SUB     L
        LDA     BOFP+1
        SBB     H
        RNZ
        LDA     BOFP
        SUB     L
        ORA     A
        RET
;
;
;
LFCA4:  CALL    LFF2D           ;TURN ON THE CURSOR
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        CALL    LFE33           ;BACK UP TO LAST NON BLANK CHAR
        INR     C
        INR     C
        MOV     A,C
        CPI     6
        JNC     LFCB8
        MVI     A,6
        MOV     C,A
LFCB8:  LHLD    LD19E
        SUB     M
        CALL    LFCD1
        LHLD    LD19E
        MOV     M,C
        DCR     C
        DCR     C
        INX     H
        XCHG
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        CALL    LFE24
        XCHG
        MVI     M,0DH
        RET
;
;
;
LFCD1:  MOV     B,A
        ORA     A
        RZ
        JM      LFD1A
        LHLD    EOFP
        CALL    LFD32
        LXI     D,LD191         ;POINT TO ESET BUFFER LIMIT
        CALL    LFD42
        JC      LFB30
        XCHG
        LHLD    LD19E
        DCX     H
        MVI     M,81H
        LHLD    EOFP
LFCF0:  MOV     A,M
        CPI     81H
        JZ      LFCFC
        STAX    D
        DCX     H
        DCX     D
        JMP     LFCF0
;
;
;
LFCFC:  MVI     M,0DH
LFCFE:  LHLD    EOFP
        MOV     A,B
        CALL    LFD32
        SHLD    EOFP
        LHLD    LD19C
        LXI     D,LD19E
        CALL    LFD42
        RZ
        MOV     A,B
        CALL    LFD32
        SHLD    LD19C
        RET
;
;
;
LFD1A:  CMA
        INR     A
        LHLD    LD19E
        CALL    LFD32
        XCHG
        LHLD    LD19E
LFD26:  LDAX    D
        MOV     M,A
        CPI     01H
        JZ      LFCFE
        INX     H
        INX     D
        JMP     LFD26
;
;
;
LFD32:  ORA     A
        JM      LFD3B
        ADD     L
        MOV     L,A
        RNC
        INR     H
        RET
;
;
;
LFD3B:  ADD     L
        MOV     L,A
        MVI     A,0FFH
        ADC     H
        MOV     H,A
        RET
;
;
;
LFD42:  LDAX    D
        SUB     L
        INX     D
        LDAX    D
        SBB     H
        DCX     D
        RC
        RNZ
        LDAX    D
        SUB     L
        ORA     A
        RET
;
;ROUTINE TO CLEAR ALL OR PART OF VDM-1 DISPLAY SCREEN
;
LFD4E:  LXI     H,VDM1          ;ENTER HERE TO CLR WHOLE SCREEN
LFD51:  MVI     M,' '           ;FILL WITH SPACES
        INX     H               ;POINT TO NEXT CHAR POSITION
        MOV     A,H             ;GET HIGH ORDER BYTE
        CPI     0D0H            ;PAST END OF SCREEN ?
        JC      LFD51           ;CONTINUE IF NOT
        XRA     A               ;ELSE GET A ZERO TO CLEAR:
        STA     BOSL            ;BEGINNING OF SCREEN LINE (0-F)
        STA     BOTL            ;BEGINNING OF TEXT LINE (0-F)
        STA     NCHAR           ;CURRENT COLUMN POSITION
        MVI     A,0FH           ;THEN GET A 15
        STA     SLINE           ;TO PUT US ON THE BOTTOM LINE
;
;THIS ROUTINE OUTPUTS A PROPER COMMAND BYTE TO THE VDM-1/SOL-20
;VIDEO DISPLAY GENERATOR BASED ON SCREEN PARAMTETERS
;
LFD69:  LDA     BOSL            ;BEGINNING OF SCREEN LINE (0-F)
        RLC                     ;SHIFT LEFT TO HIGH NIBBLE
        RLC                     ;
        RLC                     ;
        RLC                     ;
        LXI     H,BOTL          ;BEGINNING OF TEXT LINE (0-F)
        ORA     M               ;A NOW HAS VDM CONTROL BYTE
        OUT     0C8H            ;FOR VDM-1
        OUT     0FEH            ;FOR SOL'S
        RET                     ;DONE
;
;
;
LFD79:  INR     L
        MOV     A,L
        ANI     3FH
        RNZ
        MOV     A,L
        ADI     0C0H
        MOV     L,A
        RET
;
;THIS ROUTINE IS PART OF THE VDM-1 DRIVER USED TO DISPLAY
;NORMAL CHARS AFTER CHECKING FOR SPEED CNTL HAS BEEN DONE
;
LFD83:  ANI     7FH             ;CLEAR PARITY BIT
        MOV     C,A             ;SAVE IN C
        LXI     H,NCHAR         ;CURRENT COLUMN
        MOV     B,M             ;COL IN B
        CPI     0DH             ;IS CHAR A C/R ?
        JZ      LFDB1           ;PROCESS IT SEPERATELY
        CPI     5FH             ;BACKSPACE ?
        JZ      LFEED           ;IT GET'S SPECIAL TREATMENT TOO
        CPI     'A'-40H         ;CURSOR ON-OFF TOGGLE ?
        JZ      LFF06           ;ANOTHER SPECIAL
        CPI     'Z'-40H         ;CLEAR SCREEN ?
        JZ      LFD4E           ;LAST SPECIAL CHAR
        CPI     20H             ;TESTING FOR CONTROL CHAR
        RC                      ;SKIP THEM IF FOUND
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        CALL    VDAD            ;AND CALCULATE CHAR ADDRESS
        MOV     M,C             ;PUT CHAR ON SCREEN
        LDA     NCHAR           ;NOW GET COLUMN
        INR     A               ;AND INCREMENT IT
        CPI     64              ;GONE OFF END OF LINE ?
        JNZ     LFDBD           ;NO - OK THEN
;
;COME HERE TO PROCESS CARRIAGE RETURN
;
LFDB1:  LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        CALL    LFE46           ;TURN CURSOR OFF
        MVI     A,1
        CALL    LFDCD           ;CLEAR TO END OF LINE & SCROLL
        SUB     A               ;GET A ZERO
LFDBD:  STA     NCHAR           ;PUT UPDATED COL. POSITION BACK
        MOV     B,A             ;AND MOVE IT TO B
        LDA     LD197           ;GET CURSOR ON-OFF FLAG
        CPI     0               ;SET FLAGS
        RZ                      ;DONE IF CURSOR OFF
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        JMP     LFF1A           ;TURN CURSOR ON & RETURN
;
;
;
LFDCD:  LXI     H,BOTL          ;BEGINNING OF TEXT LINE (0-F)
        ORA     A
        JZ      LFDD9
        MOV     A,M
        INR     M
        JMP     LFDDD
LFDD9:  DCR     M
        MOV     A,M
        ANI     0FH
LFDDD:  SUB     M
        MVI     C,0
        CALL    LFE0C
        LXI     H,BOTL          ;BEGINNING OF TEXT LINE (0-F)
        MOV     A,M
        ANI     0FH
        MOV     M,A
        JMP     LFD69
;
;
;
LFDED:  LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        LXI     H,NCHAR         ;GET COLUMN POSITION
        MOV     B,M             ;TO B
;
;FOLLOWING ROUTINE CALCULATES ADDR OF CHAR AT LINE
;IN "A" AND COL "B"
;
VDAD:   MOV     L,A             ;LINE NO. (0-F) TO L
        LDA     BOTL            ;BEGINNING OF TEXT LINE (0-F)
        ADD     L               ;ADD OFFSET TO LINE
        RRC                     ;DIVIDE BY TWO
        RRC                     ;AGAIN, NOW /4
        MOV     L,A             ;SAVE # PAGES (256) OFFSET
        ANI     3               ;MAKE IT 0-3 ONLY
        ADI     (VDM1 SHR 8) AND 0FFH
        MOV     H,A             ;H NOW HI SCREEN ADDR
        MOV     A,L             ;RESTORE # 256 BYTE PGS. OFFSET
        ANI     0C0H            ;CHG TO # LINES FROM PG BOUNDRY
        ADD     B               ;ADD IN COL ON LINE
        MOV     L,A             ;H,L NOW COMPLETE ADDR
        RET                     ;DONE
;
;
;
LFE07:  MOV     C,M
        DCR     C
        DCR     C
        INX     H
        XCHG
LFE0C:  MVI     B,0
        CALL    VDAD
        MVI     B,64
LFE13:  XRA     A
        CMP     C
        MVI     A,' '
        JZ      LFE1D
        LDAX    D
        INX     D
        DCR     C
LFE1D:  MOV     M,A
        INR     L
        DCR     B
        JNZ     LFE13
        RET
;
;
;
LFE24:  MVI     B,0
        CALL    VDAD
        INR     C
LFE2A:  DCR     C
        RZ
        MOV     A,M
        STAX    D
        INX     H
        INX     D
        JMP     LFE2A
;
;
;
LFE33:  MVI     B,3FH
        CALL    VDAD
        MVI     C,40H
LFE3A:  MOV     A,M
        ANI     7FH
        CPI     20H
        RNZ
        DCX     H
        DCR     C
        JNZ     LFE3A
        RET
;
;ROUTINE TO TURN CURSOR OFF
;
LFE46:  CALL    VDAD            ;CALCULATE SCREEN ADDR
        MOV     A,M             ;GET CHAR UNDER CURSOR
        ANI     7FH             ;TURN CURSOR BIT OFF
        MOV     M,A             ;PUT IT BACK
        RET                     ;DONE, CURSOR OFF
;
;
;
LFE4E:  XCHG
LFE4F:  PUSH    PSW
        PUSH    D
        CALL    VDAD
LFE54:  MOV     M,C
        CALL    LFD79
        DCR     D
        JNZ     LFE54
        POP     D
        POP     PSW
        DCR     E
        RZ
        INR     A
        JMP     LFE4F
;
;
;
LFE64:  CPI     1BH             ;ESCAPE CHAR ?
        JZ      EORMS           ;BACK TO ALS-8 THEN
        CPI     20H             ;SPACE BAR ?
        JNZ     LFEC1           ;NO - CLEAR CHRR AND RETURN
LFE6E:  CALL    STAT            ;YES - CHAR TYPED ?
        JZ      LFE6E           ;NO - WAIT FOREVER IF NECESSARY
        JMP     LFEC2           ;THEN SAVE THAT CHAR AS CHRR
;
;VDM-1 DRIVER STARTS HERE
;
LFE77:  MOV     A,B             ;GET CHAR TO A
        CPI     7FH             ;DELETE CHAR ?
        RZ                      ;RETURN IF SO
        PUSH    H               ;ELSE SAVE REGISTERS
        PUSH    D               ;
        PUSH    B               ;
        LDA     SPEED           ;GET SPEED CONTROL BYTE
        MOV     H,A             ;TO H
        MVI     L,80H           ;COUNTER NOW SET
        CALL    LFE9D           ;SEE IF CHAR WAITING
        XRA     A               ;MAKE ACC. ZERO
LFE88:  DCX     H               ;DCR TIMER COUNTER
        CMP     H               ;H = 0 YET ?
        JNZ     LFE88           ;NO - THEN DELAY SOME MORE
        POP     B               ;RESTORE CHAR TO B
        PUSH    B               ;AND RE-SAVE B,C
        MOV     A,B             ;GET CHAR AGAIN
        CPI     'S'-40H         ;CONTROL-S ?
        CZ      LFEC6           ;CALL SPEED SET ROUTINE
        CALL    LFD83           ;PUT CHAR ON SCREEN
        POP     B               ;RESTORE REGS
        POP     D               ;
        POP     H               ;
        MOV     A,B             ;PUT CHAR IN ACC ON EXIT
        RET                     ;DONE
;
;ROUTINE TO CHECK FOR KEYBOARD INPUT & SCREEN CONTROL VALUE
;
LFE9D:  LDA     CHRR            ;DEFAULT SPEED
        MOV     B,A             ;SAVE IT IN B
        CALL    STAT            ;KEYBOARD CHAR WAITING ?
        CNZ     IN8             ;YES - THEN GET IT
        MOV     A,B             ;DEFAULT SPEED OR CHAR TO A
LFEA8:  ORA     A               ;SET FLAGS
        RZ                      ;NO CHG IF CHR/DEFAULT=0
        CPI     '9'+1           ;NUMERIC ?
        JNC     LFE64           ;NO, NOT NUMERIC
        CPI     '1'             ;NUMERIC ?
        JC      LFE64           ;NO, NOT NUMERIC
        ANI     0FH             ;CONVERT TO BINARY
        MOV     C,A             ;SAVE IN C
        XRA     A               ;CLEAR ACC & FLAGS
        STC                     ;SET CARRY
LFEB9:  STA     SPEED           ;SPEED CONTROL BYTE
        RAL                     ;MOVE ACCUM/CARRY 1 BIT LEFT
        DCR     C               ;DCR SPEED COUNT
        JNZ     LFEB9           ;IF SPEED COUNT <> 0, CONTINUE
LFEC1:  XRA     A               ;GET A ZERO
LFEC2:  STA     CHRR            ;MAKE IT ZERO FOR NEXT TIME
        RET                     ;DONE
;
;ROUTINE TO SET SPEED
;
LFEC6:  MVI     B,0             ;COLUMN = 0
        CALL    LFDB1           ;CLEAR LINE
        LXI     H,LFEE6         ;POINT TO "SPEED ?" MESSAGE
LFECE:  MOV     A,M             ;GET CHAR FROM MSG
        PUSH    H               ;SAVE H,L
        CALL    LFD83           ;PRINT MESSAGE
        POP     H               ;RESTORE H,L
        INX     H               ;POINT TO NEXT CHAR
        MOV     A,M             ;GET IT
        ORA     A               ;SET FLAGS
        JNZ     LFECE           ;PRINT IT IF NOT ZERO
        CALL    IN8             ;GET CHAR
        CALL    LFEA8           ;TEST NUMERIC & SET SPEED
        CALL    CRLF            ;DO CR/LF
        JMP     EORNS           ;THEN BACK TO ALS-8
;
LFEE6:  DB      'SPEED?'
        DB      0
;
;
;ROUTINE TO PROCESS BACKSPACE (5FH)
;
LFEED:  LDA     NCHAR           ;GET COLUMN
        MOV     B,A             ;TO B
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        CALL    LFE46           ;TURN CURSOR OFF
        DCX     H               ;POINT TO PREV CHAR
        MVI     M,' '           ;REPLACE IT WITH SPACE
        MOV     A,B             ;OLD COLUMN ADDR TO A
        ORA     A               ;SET FLAGS
        JZ      LFF00           ;DON'T BACK INTO PREV LINE
        DCR     B               ;DECREMENT COLUMN
LFF00:  LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        JMP     LFF1A           ;UPDATE ROW, COL, CUR ON & RET
;
;CURSOR ON-OFF TOGGLE ROUTINES
;
LFF06:  LDA     LD197           ;GET CURSOR ON-OFF FLAG
        XRI     01H             ;TOGGLE LOW ORDER BIT
        STA     LD197           ;PUT IT BACK
        ANI     01H             ;MASK LOW ORDER BIT
        JNZ     LFF2A           ;JUMP IF CURSOR OFF
LFF13:  LDA     NCHAR           ;GET COLUMN
LFF16:  MOV     B,A             ;TO A
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
LFF1A:  ANI     0FH             ;KEEP IT TO 0-15
        STA     SLINE           ;UPDATE IT
        CALL    VDAD            ;CALCULATE CURSOR ADDRESS
        MOV     A,B             ;COLUMN TO A
        STA     NCHAR           ;UPDATE IT
        MOV     A,M             ;GET CHAR TO A
        JMP     LFC4D           ;TURN ON CURSOR & RETURN
LFF2A:  STA     LD197           ;UPDATE CURSOR FLAG
LFF2D:  LXI     H,NCHAR         ;POINT TO COLUMN
        MOV     B,M             ;GET IT TO B
        LDA     SLINE           ;GET CURRENT LINE NO (0-F)
        CALL    LFE46           ;TURN CURSOR OFF
        LXI     H,NCHAR         ;FOR PROPER EXIT
        RET                     ;DONE, CURSOR OFF
;
;THIS IS THE ALS-8 (NOT TXT-2) FIND COMMAND
;
FIND2:  CALL    CRLF            ;DO CR/LF
        CALL    LFF80           ;GET SEARCH STRING
        LHLD    BOFP            ;LOAD SEARCH STARTING POINT
LFF44:  CALL    LFFC2           ;SEARCH FOR STRING
        CPI     1               ;EOF FOUND ?
        RZ                      ;DONE IF YES
        LHLD    INSP            ;ELSE POINT TO LINE W/STRING
        INX     H               ;POINT PAST LENGTH BYTE
        CALL    SCRN            ;PRINT LINE
        CALL    CRLF            ;CAR. RET AT END OF LINE
        INX     H               ;INR TO NEXT LINE
        JMP     LFF44           ;AND CONTINUE SEARCH
;
;EDITOR STRING SEARCH
;
LFF58:  CALL    LFD4E           ;CLEAR SCREEN
        CALL    LFF80           ;GET SEARCH STRING
        LHLD    BOFP            ;POINT TO BEGINNING OF FILE
        SHLD    LD19C           ;SAVE AS SEARCH STARTING POINT
;
;CONTINUE SEARCH
;
LFF64:  LXI     SP,SMODE        ;RESET STACK POINTER
        LHLD    LD19C           ;GET SEARCH STARTING PT.
        CALL    LFFC2           ;SEARCH FROM H,L FOR STRING
        CPI     1               ;END OF FILE FOUND ?
        JZ      TXT2            ;RESTART EDITOR THEN
        CALL    LFD4E           ;
        XRA     A               ;GET A ZERO
        MOV     B,A             ;MAKE IT THE ROW
        CALL    LFF1A           ;TURN CURSOR ON
        LHLD    INSP            ;GET POINTER TO LINE W/STRING
        JMP     LFA27           ;PUT TEXT ON SCRN & RET
;
;ROUTINE TO FIND STRING
;
LFF80:  MVI     B,':'           ;GET PROMPT
        CALL    OUT8            ;PRINT IT
        MVI     E,0             ;INITIALIZE STRING LENGTH
        LXI     H,IBUF+81       ;POINT TO STRING LOCATION
LFF8A:  CALL    IN8             ;GET STRING CHAR
        CPI     7FH             ;DELETE ?
        JNZ     LFFA1           ;NO - MUST BE CHAR
        MOV     A,E             ;ELSE MOVE LENGTH TO A
        ORA     A               ;ZERO ?
        JZ      LFF8A           ;NO ACTION IF ZERO
        DCR     E               ;ELSE DCR STRING LENGTH
        DCX     H               ;AND STRING POINTER
        MVI     B,5FH           ;GET A BACKSPACE CHAR
LFF9B:  CALL    OUT8            ;AND PRINT IT
        JMP     LFF8A           ;THEN GET THE NEXT CHAR
LFFA1:  CPI     0DH             ;CARRIAGE RETURN ?
        JNZ     LFFB6           ;NO - MUST BE CHAR
        MOV     A,E             ;ELSE GET LENGTH TO A
        ORA     A               ;SET FLAGS
        JNZ     LFFB1           ;IF NOT ZERO, DONE
        CALL    CRLF            ;ELSE PRINT CRLF
        JMP     LFF80           ;AND GET STRING (AGAIN)
LFFB1:  MOV     M,B             ;MOVE C/R TO STRING BUFFER
        CALL    CRLF            ;PRINT CR/LF
        RET                     ;AND RETURN W/STRING IN BUFFER
LFFB6:  CPI     20H             ;IS CHAR A CNTL CHAR ?
        JC      LFF8A           ;IGNORE IT THEN
        INR     E               ;ELSE INR LENGTH
        MOV     M,B             ;PUT CHAR IN STRING
        INX     H               ;INR POINTER
        JMP     LFF9B           ;AND DO NEXT CHAR
;
;ROUTINE TO SEARCH FOR STRING
;
LFFC1:  INX     H               ;POINT PAST LENGTH BYTE
LFFC2:  SHLD    INSP            ;AND SAVE AS POINTER
        MVI     C,0             ;INITIALIZE COUNTER
LFFC7:  LXI     D,IBUF+81       ;GET STRING LENGTH
        MOV     A,M             ;GET CHAR FROM LINE
        CPI     1               ;END OF FILE ?
        RZ                      ;IF SO, RETURN
LFFCE:  INX     H               ;ELSE INCR LINE POINTER
        INR     C               ;AND COUNT OF CHARS TESTED
        LDAX    D               ;GET CHAR FROM STRING
        CMP     M               ;STRING & LINE AGREE ?
        INX     D               ;INR STRING POINTER
        JZ      LFFE7           ;AND KEEP TESTING
        CPI     0DH             ;WAS NON-MATCH DUE TO C/R ?
        RZ                      ;YES - STRING MATCHED THEN
        MOV     A,M             ;ELSE MOVE LINE CHAR TO A
        CPI     0DH             ;AND SEE IF END OF LINE
        JZ      LFFC1           ;IT WAS - GO TO NEXT LINE
LFFDF:  DCR     C               ;ELSE DCR MATCH COUNT
        JZ      LFFC7           ;BACK TO WHERE WE STARTED ?
        DCX     H               ;NO - DCR LINE POINTER
        JMP     LFFDF           ;AND TEST AGAIN
LFFE7:  CPI     0DH             ;MATCH - END OF STRING TOO ?
        RZ                      ;THAT'S STILL A MATCH
        MOV     A,M             ;ELSE GET LINE CHAR
        CPI     0DH             ;AND SEE IF END OF LINE
        JZ      LFFC1           ;IF SO TEST NEXT LINE
        JMP     LFFCE           ;ELSE GO ON WITH TEST
;
;ESET COMMAND TO SET UPPER LIMIT ON EDITOR TEXT BUFFER
;
LFFF3:  LHLD    BBUF            ;GET VALUE OF UPPER LIMIT
        SHLD    LD191           ;SAVE IT
        JMP     EORMS           ;AND GO BACK TO ALS-8
;
; **** END OF ALS8TXT MODULE
;
;
        ORG     DATA            ;START OF SYSTEM GLOBAL
;
;SYSTEM GLOBAL AREA
;
UDATA   EQU     1               ;DATA PORT NUMBER
;DAV    EQU     40H             ;DATA AVAILABLE AT BIT 6
DAV     EQU     01H             ;*UM*
TBE     EQU     80H             ;TRANS. BUFFER EMPTY AT BIT 7
USTA    EQU     0               ;UART STATUS PORT
UDAI    EQU     1               ;UART DATA
UDAO    EQU     1               ;UART DATA
SWCH    EQU     0FFH            ;SENSE SWITCH
;
;FILE AREA PARAMETERS
;
MAXFIL  EQU     6               ;MAX # OF FILES
NMLEN   EQU     5               ;NAME LENGTH
FELEN   EQU     NMLEN+8         ;DIRECTORY ENTRY LENGTH
;
;FILE TABLE
;
;FILE 0 IS THE "CURRENT FILE" ALL OTHER FILES
;ARE STORED IN THE FILE TABLE IN THIS FORMAT
;
FILE0:  DS      NMLEN           ;CURRENT FILE LOCATION
BOFP:   DS      2               ;BEGINNING OF FILE POINTER
EOFP:   DS      2               ;END OF FILE POINTER
MAXL:   DS      4               ;MAXIMUM LINE NUMBER IN FILE
;
FILTB:  DS      (MAXFIL-1)*FELEN;REST OF FILE TABLE
;
;I/O DRIVER TABLES
;
IOFLE:  DS      (MAXFIL-1)*FELEN
SYSIO:  DS      NMLEN
SYSIN:  DS      2
SYSOT:  DS      2               ;OUTPUT DRIVER ADDRESS
;
;THE INPUT DRIVER
;
INDR:   DS      OUTP8-INP8
;
;THE OUTPUT DRIVER
;
OUTDR:  DS      CRLF-OUTP8      ;OUTPUT DRIVER
IN8:    DS      3               ;DRIVER JUMP POINTS
OUT8:   DS      3
STAT    EQU     INDR+12
NOCHR   EQU     OUTDR+0FH
;
;SYSTEM PARAMETERS
;
INSP:   DS      2               ;INSERT LINE POSITION
DELP    EQU     INSP            ;DELETE LINE POSITION
ASCR    EQU     13
ESC     EQU     1BH
COMCHR  EQU     '*'             ;ASSEMBLER COMMENT CHARACTER
HCON:   DS      3               ;CONVERSION AREA
ADDS    EQU     HCON            ;FIND ADDRESS
FBUF:   DS      NMLEN           ;FILE NAME BUFFER
FREAD:  DS      2               ;FREE ADDRESS IN DIRECTORY
FEF:    DS      1               ;FREE ENTRY FOUND FLAG
FOCNT   EQU     FEF             ;OUTPUT COUNTER
ABUF:   DS      16              ;ASCII BUFFER AREA
BBUF:   DS      4               ;BINARY BUFFER
SCNT:   DS      1               ;SYMBOL COUNT
DCNT:   DS      1               ;DUMP ROUTINE COUNTER
TABA:   DS      2               ;SYMBOL TABLE END ADDRESS
ASPC:   DS      2               ;ASSEMBLER PROGRAM COUNTER
PASI:   DS      1               ;PASS INDICATOR
LFMT:   DS      1               ;FORMAT CONTROL
NOLIN:  DS      1
IOSWC:  DS      1               ;I/O SWITCH INDICATOR
SWCH1:  DS      1
SWCH2:  DS      1
XOUT:   DS      1               ;SCRN PARAMETER
ASMTY:  DS      1               ;ASSEMBLY TYPE FLAG
PNTR:   DS      2               ;LINE POINTER STORAGE
NOLA:   DS      1               ;NUMBER OF LABELS
SIGN:   DS      1               ;SIGN STORAGE FOR SCAN
OPRD:   DS      2
OPRI:   DS      1
GTLT:   DS      1
TEMP:   DS      1
APNT    EQU     INSP
AERR    EQU     DCNT
ALST:   DS      1
OIND:   DS      2
LLAB    EQU     5
        DS      16
AREA:   DS      14
LD12B:  DS      4
SMODE:  DS      1
SYMSV:  DS      2
CCNT    EQU     SCNT
SYMX    EQU     AERR
SYMADD: DS      2
LD134:  DS      1               ;SIM. BREAKPOINT FLAG
LD135:  DS      1               ;SIM. REAL-TIME RUN FLAG
LD136:  DS      1               ;SIM. INPUT PORT ASSIG. FLAG
LD137:  DS      1               ;SIM. OUTPUT PORT ASSIG. FLAG
LD138:  DS      2               ;SIM. BREAKPOINT ADDR.
LD13A:  DS      2               ;SIM. REAL TIME RUN ADDR
LD13C:  DS      2               ;SIM. PROGRAM COUNTER
LD13E:  DS      2               ;ADDR FOR NEXT INST TO EXECUTE
LD140:  DS      2               ;SIM. STACK POINTER
LD142:  DS      2               ;SIM. FLAG REGISTER & ACCUM.
LD144:  DS      1               ;SIM. "L" REGISTER
LD145:  DS      1               ;SIM. "H" REGISTER
LD146:  DS      1               ;SIM. "E" REGISTER
LD147:  DS      1               ;SIM. "D" REGISTER
LD148:  DS      1               ;SIM. "C" REGISTER
LD149:  DS      1               ;SIM. "B" REGISTER
LD14A:  DS      1               ;SIM. ACCUM (2ND COPY)
LD14B:  DS      1               ;SIM. MODE
LD14C:  DS      3               ;STORAGE FOR INST TO BE SIM.
LD14F:  DS      3               ;STORAGE FOR JMP RETN TO SIM.
LD152:  DS      16              ;SIMULATOR'S INPUT TABLE
LD162:  DS      16              ;SIMULATOR'S OUTPUT TABLE
        ORG     DATA+18FH
TERMW:  DS      1
CHRR:   DS      1
LD191:  DS      2               ;ESET UPPER TEXT BUFFER LIMIT
BOTL:   DS      1               ;BEGINNING OF TEXT LINE (0-F)
BOSL:   DS      1               ;BEGINNING OF SCREEN LINE (0-F)
SLINE:  DS      1               ;CURRENT LINE (0-F)
NCHAR:  DS      1               ;CURRENT COLUMN POSITION (0-63)
LD197:  DS      1               ;CURSOR ON-OFF FLAG
LD198:  DS      1               ;INSERT MODE FLAG
LD199:  DS      1
LD19A:  DS      2
LD19C:  DS      2
LD19E:  DS      2
SPEED:  DS      1               ;DISPLAY DRIVER SPEED BYTE
LD1A1:  DS      2               ;UNDEFINED COMMAND ADDRESS
USARE:  DS      38              ;USER AREA
OBUF:   DS      22
        DS      5
IBUF:   DS      120             ;INPUT BUFFER
EDIT    EQU     0FA0EH
SAVL    EQU     OBUF
CUCOM:  DS      70              ;CUSTOM COMMAND TABLE
        ORG     DATA+300H
SYSYM:  DS      2               ;START OF SYSTEM SYMBOL TABLE
SYMT    EQU     CODE
;
        END
;
; **** END OF EVERYTHING
;

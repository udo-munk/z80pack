; **************************************************************
;
; MITS ALTAIR 8800 ROM MONITOR
;
; **************************************************************
;
; BASED ON THE ORIGINAL ALTAIR TURNKEY SYSTEM MONITOR
;
; MODIFIED BY:  SCOTT LABOMBARD
;               8/9/02
;
; MODIFIED TO ASSEMBLE WITH INTEL 8080 CROSS ASSEMBLER
;               UDO MUNK
;               10/11/08
;
; **************************************************************

STACK   EQU     0FC00H          ;MUST BE VALID RAM, NOT ROM!
CONSTAT EQU     020Q            ;IO STATUS PORT
CONDATA EQU     021Q            ;IO DATA PORT

        ORG     0FD00H          ;ROM BASED CODE

MON:    MVI     A,3             ;RESET 2SIO BOARD
        OUT     CONSTAT
        MVI     A,021Q          ;INITIALIZE 2SIO BOARD
        OUT     CONSTAT
ENTER:  LXI     SP,STACK        ;INITIALIZE STACK POINTER
        CALL    CRLF            ;PRINT CARRIAGE RET+LINE FEED
        CALL    CRLF            ;PRINT CARRIAGE RET+LINE FEED
        MVI     A,'.'           ;MONITOR PROMPT
        CALL    OUTCHK          ;PRINT CHAR TO CONSOLE
        CALL    INCH            ;GET CHAR FROM CONSOLE
        CPI     'M'
        JZ      MEM             ;DO MEMORY EXAMINE/ALTER
        CPI     'D'
        JZ      DMP             ;DO MEMORY DUMP
        CPI     'J'
        JNZ     ENTER

; **************************************************************
; GET JUMP ADDRESS, LOAD TO PC, AND GO
; **************************************************************
        CALL    OCTL6           ;GET 6 OCTAL DIGITS IN HL
        PCHL

; **************************************************************
; MEMORY FUNCTION - DISPLAY AND/OR ALTER MEMORY
; **************************************************************
MEM:    CALL    OCTL6           ;GET 6 OCTAL DIGITS IN HL
        JMP     CONT1
CONT:   INX     H               ;POINT TO NEXT ADDRESS
CONT1:  CALL    CRLF            ;PRINT CARRIAGE RET+LINE FEED
        MOV     D,H             ;SAVE ADDR TO DE
        MOV     E,L
        CALL    PRT6            ;CVT TO ASCII + PRINT
        LDAX    D               ;LOAD DATA FROM CURRENT MEM LOC
        MOV     H,A
        CALL    PRT3            ;CVT TO ASCII + PRINT
        CALL    OCTL3           ;GET 3 OCTAL DIGITS IN HL
        XCHG                    ;EXCHANGE HL AND DE
        JC      CONT
        MOV     M,A             ;STORE USER SPECIFIED BYTE
        CMP     M               ;VALIDATE DATA BYTE IN MEMORY
        JZ      CONT            ;IF BYTE OKAY, KEEP GOING
ERR:    MVI     A,'?'           ;WE HAVE A PROBLEM
        CALL    OUTCHK          ;PRINT ERROR CHAR TO CONSOLE
        JMP     ENTER

; **************************************************************
; DUMP FUNCTION - DISPLAY DATA BETWEEN TWO SPECIFIED MEM LOCS
; **************************************************************
DMP:    CALL    OCTL6           ;GET 6 OCTAL DIGITS IN HL
        XCHG                    ;SAVE START ADDR TO DE
        CNC     SPACE
        CALL    OCTL6           ;GET 6 OCTAL DIGITS IN HL
        PUSH    H               ;SAVE END ADDR
DCONT:  MOV     H,D             ;MOV ADDR IN DE TO HL FOR PRINT
        MOV     L,E
        CALL    CRLF            ;PRINT CARRIAGE RET+LINE FEED
        CALL    PRT6            ;CVT TO ASCII + PRINT
        CALL    SPACE
        LXI     B,020Q          ;PRINT 16 MEM LOCATIONS PER LINE
DO20:   LDAX    D               ;LOAD DATA FROM CURRENT MEM LOC
        MOV     H,A
        PUSH    B               ;SAVE PRINT LOCATION COUNTER
        MVI     A,010Q          ;IS HALF THE LINE PRINTED?
        CMP     C
        JNZ     NXTMEM
        MVI     A,'-'           ;MAKES EACH LINE EASIER TO READ
        CALL    OUTCHK
        CALL    SPACE
NXTMEM: CALL    PRT3            ;CVT TO ASCII + PRINT MEM DATA
        POP     B               ;RESTORE PRINT LOCATION COUNTER
        POP     H               ;RESTORE END ADDR
        MOV     A,H             ;COMPARE CURRENT ADDR WITH END
        CMP     D
        JNZ     DAGN
        MOV     A,L
        CMP     E
        JZ      ENTER           ;PROCESSED LAST ADDRESS SO DONE
DAGN:   PUSH    H               ;SAVE END ADDR TO USE AGAIN
        INX     D               ;NEXT MEMORY LOCATION TO PRINT
        DCR     C               ;CURRENT PRINT LOCATION COUNTER
        JNZ     DO20            ;16 LOCATIONS PRINTED YET?              
        JMP     DCONT           ;NEXT LINE IF 16 LOCATIONS DONE

; **************************************************************
; PRINT CARRIAGE RETURN AND LINE FEED
; **************************************************************
CRLF:   MVI     A,015Q
        CALL    OUTCHK          ;PRINT CHAR TO CONSOLE
        MVI     A,012Q
        JMP     OUTCHK          ;PRINT CHAR TO CONSOLE

; **************************************************************
; BUILD 3/6 OCTAL DIGITS IN HL
; **************************************************************
OCTL6:  MVI     B,6             ;SET DIGIT COUNTER
        JMP     OCTL
OCTL3:  MVI     B,3             ;SET DIGIT COUNTER
OCTL:   LXI     H,0             ;CLEAR ALL 16 BITS OF HL REG
AGN:    CALL    INCH            ;GET CHAR FROM CONSOLE
        MOV     C,A
        CPI     ' '             ;CHECK FOR SPACE CHAR
        STC
        RZ                      ;SPACE CHAR ENTERED SO QUIT
        ANI     270Q            ;CHECK FOR VALID OCTAL DIGIT
        XRI     060Q
        JNZ     ERR             ;NOT OCTAL SO LEAVE
        MOV     A,C             ;CONVERT ASCII TO BINARY
        ANI     007Q            ;STRIP ASCII
        DAD     H               ;SHIFT HL LEFT 3 BITS
        DAD     H
        DAD     H
        ADD     L
        MOV     L,A             ;PUT OCTAL IN H
        DCR     B               ;MORE DIGITS?
        JNZ     AGN
        RET

; **************************************************************
; PRINT 3 OR 6 OCTAL DIGITS FROM H OR HL
; **************************************************************
PRT6:   MVI     B,6             ;SET DIGIT COUNTER
        XRA     A
        JMP     NEXT1
PRT3:   MVI     B,3             ;SET DIGIT COUNTER
        XRA     A
        JMP     NXT3
NEXT3:  DAD     H               ;SHIFT 1 BIT
NXT3:   RAL
        DAD     H               ;SHIFT 1 BIT
        RAL
NEXT1:  DAD     H               ;SHIFT 1 BIT
        RAL
        ANI     7               ;STRIP OFF OCTAL
        ORI     060Q            ;CONVERT TO ASCII
        CALL    OUTCHK          ;PRINT CHAR TO CONSOLE
        DCR     B
        JNZ     NEXT3
SPACE:  MVI     A,' '           ;ASCII SPACE CHARACTER
        JMP     OUTCHK          ;PRINT CHAR TO CONSOLE

; **************************************************************
; INPUT AND ECHO CHARACTER
; **************************************************************
INCH:   IN      CONSTAT
        RRC
        JNC     INCH            ;CHECK READ STATUS
        IN      CONDATA         ;READ CHARACTER
        ANI     177Q            ;STRIP PARITY BIT
OUTCHK: PUSH    PSW             ;SAVE CHARACTER
        ADD     C               ;ADD IN CHECKSUM
        MOV     C,A             ;RESTORE CHECKSUM
LOOP:   IN      CONSTAT
        RRC
        RRC
        JNC     LOOP            ;GET READ STATUS
        POP     PSW
        OUT     CONDATA         ;PRINT USER TYPED CHARACTER
        RET

        END

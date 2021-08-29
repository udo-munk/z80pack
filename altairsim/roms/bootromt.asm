;******************************************************************
;       PROM BASED BOOT ROUTINE FOR TARBELL SINGLE DENSITY CP/M
;******************************************************************
;
;       THIS PROGRAM IS DESIGNED TO BOOT SECTOR 1 OF TRACK 0
;       ON A TARBELL DISK USING THE TARBELL SINGLE DENSITY
;       DISK CONTROLLER BOARD. TRACK 0 IS FORMATTED SINGLE
;       DENSITY SO THE CPU CAN DO THE DATA TRANSFER. THE BOOT
;       ROUTINE LOADS A PROGRAM FROM DISK INTO MEMORY AND THEN
;       JUMPS TO THAT ROUTINE. THE PROGRAM LOADED STARTS AT
;       MEMORY ADDRESS 0000H.
;               THE PART OF THIS PROGRAM THAT ACTUALLY DOES THE
;       THE BOOT OPERATION IS MOVED FROM A DATA AREA IN THE 1702A
;       TURNKEY PROMS AT ADDRESS 0FE00H TO BE EXECUTED FROM WITHIN
;       CPU RAM AT ADDRESS 01000H. THIS IS NECESSARY DUE TO THE
;       EXTENDED WAITSTATE TIME AFFORDED BY THE TURNKEY MODULE
;       WHICH COULD RESULT IN DATA LOST ERRORS AT THE DISK CONTROLLER
;       BOARD. THE FIRST PROM ONLY CONTAINS A BLOCK MOVE ROUTINE
;       TO TRANSFER THE DATA TO THE EXECUTION AREA AT 01000H.
;
;               MICHAEL J. KARAS
;               MICRO RESOURCES
; THESE DAYS MIKE CAN BE REACHED AT MKARAS@CAROUSEL-DESIGN.COM (MARCH 23, 2009)
;
; MODIFIED SO THAT THE BOOT CODE CAN BE MOVED TO RAM @ 1000H
; UDO MUNK, AUGUST 2016
;
;******************************************************************
;
RUNLOC  EQU    1000H            ;RELOCATE LOADER HERE
;
;MAKE A BLOCK MOVE ROUTINE TO SEND EXECUTABLE CODE INTO
;SYSTEM RAM AT 1000H
 
        ORG     0FE00H          ;PUT THE MOVE ROUTINE HERE
 
        DI                      ;NO INTERRUPTS
        LXI     H,RCODE         ;POINT TO BLOCK TO BE MOVED
        LXI     B,0180H         ;SET COUNT FOR ONE AND A HALF 1702'S
        LXI     D,RUNLOC        ;POINT TO DESTINATION OF MOVE
 MOVE:
        MOV     A,M             ;GET A BYTE OF CODE FROM PROM
        STAX    D               ;STORE IT INTO RAM
        INX     H               ;INCREMENT PROM POINTER
        INX     D               ;INCREMENT RAM POINTER
        DCX     B               ;DECREMENT BYTE COUNT
        MOV     A,C             ;CHECK IF DONE
        ORA     B
        JNZ     MOVE            ;IF NOT DONE GO DO IT AGAIN
;
        JMP     RUNLOC          ;TRANSFER CONTROL TO ROUTINE
                                ;JUST MOVED TO RAM
;
;
;BASE THIS ROUTINE INTO SYSTEM RAM AT 1000H
;
;       ORG     01000H
;
RCODE:  DI                      ;DISABLE INTERRUPTS
;
OFFSET  EQU     RCODE-RUNLOC    ;CALCULATE CODE ADDRESS OFFSET FOR RELOCATION
;
        LXI     SP,STACK-OFFSET ;SET THE STACK DUMMY?
        JMP     INIT-OFFSET     ;GO INITIALIZE THIS BUGGER 
;
;
;SYSTEM EQUATES FOR TARBELL CONTROLLER
;
DWAIT   EQU     0FCH            ;WAIT FOR DISK PORT
DCOM    EQU     0F8H            ;DISK COMMAND PORT
DDATA   EQU     0FBH            ;DISK DATA PORT
DSTAT   EQU     0F8H            ;DISK STATUS PORT
DSEC    EQU     0FAH            ;DISK SECTOR PORT
DTRK    EQU     0F9H            ;DISK TRACK PORT
DSEL    EQU     0FCH            ;DISK SELECT PORT
;
;
;SYSTEM VARIABLES AND ADDRESS POINTERS
;
SBOOT   EQU     007DH           ;SINGLE DENSITY BOOT ENTRY
RDCMD   EQU     008CH           ;READ COMMAND FOR 1791 CONTROLLER
;
;
;DEFINE SI/O RS-232 CONSOLE I/O PARAMETERS
;
CCTRL   EQU     010H            ;CONSOLE COMMAND/STATUS PORT
CDATA   EQU     011H            ;CONSOLE DATA PORT
CRRDY   EQU     001H            ;RECEIVER READY BIT
CTRDY   EQU     002H            ;TRANSMITTER READY BIT
;
;
;COLD BOOT LOADER CONSOLE I/O INTERFACE ROUTINES
;
;
;       CONSOLE  OUTPUT ROUTINE
;
CO:
        IN      CCTRL
        ANI     CTRDY
        JZ      CO-OFFSET
        MOV     A,C
        OUT     CDATA
        RET
;
;       BYTE PRINT CONVERSION ROUTINE
;
BYTEO:
        PUSH    PSW
        CALL    BYTO1-OFFSET
        MOV     C,A
        CALL    CO-OFFSET
        POP     PSW
        CALL    BYTO2-OFFSET
        MOV     C,A
        JMP     CO-OFFSET
;
;
;
BYTO1:
        RRC
        RRC
        RRC
        RRC
BYTO2:
        ANI     0FH
        CPI     0AH
        JM      BYTO3-OFFSET
        ADI     7
BYTO3:
        ADI     30H
        RET
;
;       MESSAGE PRINTING ROUTINE
;
MSG:
        PUSH    PSW             ;FOLLOWING OUTPUTS MESSAGES
        PUSH    B               ;  TO CONSOLE
        MOV     B,M
        INX     H
MSGA:
        MOV     C,M
        CALL    CO-OFFSET
        INX     H
        DCR     B
        JNZ     MSGA-OFFSET
        POP     B
        POP     PSW
        RET
;
;
;INITIALIZE
;
INIT:
        LXI     H,00FFFH        ;DELAY SI/O INIT FOR
                                ; MESSAGE IN PROGRESS
LOOP:
        DCX     H
        MOV     A,H
        ORA     L
        JNZ     LOOP-OFFSET
        MVI     A,003H          ;INITIALIZE SI/O WITH RESET
        OUT     CCTRL
        MVI     A,011H          ;INITIALIZE SIO WITH 16X,8 BITS, NO PAR
                                ;2 STOP BITS
        OUT     CCTRL
;
;
;START OF COLD BOOT LOADER CODE
;
START:
        LXI     H,CBMSG-OFFSET  ;OUTPUT "CP/M COLD BOOT" TO THE CONSOLE
        CALL    MSG-OFFSET
        MVI     A,0F2H          ;SELECT DISK A: AT SINGLE DENSITY
        OUT     DSEL
        MVI     A,0D0H          ;CLEAR ANY PENDING COMMAND
        OUT     DCOM
        NOP                     ;ALLOW TIME FOR COMMAND SETTLING
        NOP
        NOP
        NOP
HOME:
        IN      DSTAT           ;GET STATUS
        RRC
        JC      HOME-OFFSET     ;WAIT FOR NOT BUSY COMPLETION
        MVI     A,002H          ;ISSUE RESTORE CMND (10 MSEC. STEP RATE)
        OUT     DCOM
        NOP                     ;ALLOW TIME FOR COMMAND SETTLING
        NOP
        NOP
        NOP
        IN      DWAIT           ;WAIT FOR COMPLETION
        ORA     A               ;SET FLAGS FOR ERROR ON "DRQ",NOT "INTRQ"
        JM      DRQER-OFFSET
        IN      DSTAT           ;GET DISK STATUS
        ANI     004H            ;MASK FOR TRACK 00 STATUS BIT
        JZ      TK0ER-OFFSET
        XRA     A               ;ZERO ACCUMULATOR
        MOV     L,A             ;SETUP MEMORY LOAD ADDRESS 0000H
        MOV     H,A
        INR     A               ;SETUP FOR SECTOR 01
        OUT     DSEC
        MVI     A,RDCMD         ;SETUP READ COMMAND
        OUT     DCOM
        NOP                     ;ALLOW TIME FOR COMMAND SETTLING
        NOP
        NOP
        NOP
RLOOP:
        IN      DWAIT           ;WAIT FOR DISK CONTROLLER
        ORA     A               ;SET FLAGS
        JP      RDONE-OFFSET    ;ARE WE DONE YET
        IN      DDATA           ;GET DATA FROM DISK
        MOV     M,A             ;MOVE IT INTO MEMORY
        INX     H               ;INCREMENT MEMORY POINTER
        JMP     RLOOP-OFFSET    ;GO GET NEXT BYTE
RDONE:
        IN      DSTAT           ;GET DISK READ STATUS
        ORA     A               ;CHECK FOR ERRORS
        JZ      SBOOT           ;NO ERRORS?
                                ;THEN GO BOOT SINGLE DENSITY CP/M
        PUSH    PSW             ;OOPS...GOT AN ERROR,SAVE STATUS
        LXI     H,LEMSG-OFFSET  ;OUTPUT "BOOT LOAD ERROR=" TO CONSOLE
        JMP     LERR-OFFSET
DRQER:  PUSH    PSW             ;SAVE ERROR STATUS
        LXI     H,RQMSG-OFFSET  ;OUTPUT "COMMAND COMPLETION ERROR=" TO CONSOLE
        JMP     LERR-OFFSET
TK0ER:  PUSH    PSW             ;SAVE ERROR STATUS
        LXI     H,REMSG-OFFSET  ;OUTPUT "RESTORE ERROR=" TO CONSOLE
LERR:   CALL    MSG-OFFSET
        POP     PSW             ;GET ERROR STATUS BACK
        CALL    BYTEO-OFFSET    ;INDICATE ERROR AND DO CRLF
        MVI     C,0AH
        CALL    CO-OFFSET
        MVI     C,0DH
        CALL    CO-OFFSET
;
HERE:
;       JMP     START           ;GO TRY BOOTING AGAIN
        HLT                     ;OR HALT SYSTEM
;
;
;COLD BOOT ROUTINE MESSAGES
;
;
CBMSG:  DB      16,'CP/M COLD BOOT',0AH,0DH
;
LEMSG:  DB      18,0DH,0AH,'BOOT LOAD ERROR='
;
RQMSG:  DB      27,0DH,0AH,'COMMAND COMPLETION ERROR='
;
REMSG:  DB      16,0DH,0AH,'RESTORE ERROR='
;
        DS      64      ;SETUP STORAGE FOR A RAM BASED STACK
STACK   EQU     $
;
        END

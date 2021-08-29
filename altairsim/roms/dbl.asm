;***************************************************************
;*                                                             *
;* ALTAIR DISK BOOT LOADER                                     *
;* VERSION 4.1                                                 *
;*                                                             *
;* DISASSEMBLED BY MARTIN EBERHARD, 4 MARCH 2012               *
;* FROM AN EPROM WITH A PRINTED LABEL THAT SAID 'DBL 4.1'.     *
;* THIS EPROM WAS FOUND SOCKETED IN A MITS TURNKEY BOARD.      *
;*                                                             *
;* BECAUSE OF THE SLOW EPROM ACCESS TIME, THIS EPROM-BASED     *
;* PROGRAM FIRST COPIES ITSELF INTO RAM AT ADDRESS 2C00H       *
;* (RUNLOC), AND THEN EXECUTES THERE.                          *
;*                                                             *
;* ONCE IN RAM, THIS PROGRAM READS FROM THE DISK STARTING AT   *
;* TRACK 00, SECTOR 00. SECTOR DATA (WHICH INCLUDES THE ACTUAL *
;* DATA PAYLOAD, AS WELL AS HEADER AND TRAILER BYTES) IS FIRST *
;* LOADED INTO A RAM BUFFER IN MEMORY JUST AFTER THIS PROGRAM. *
;* THE DATA PAYLOAD THEN GETS MOVED INTO MEMORY STARTING AT    *
;* ADDRESS 0000H (DMAADR), CHECKING THE CHECKSUM ALONG THE WAY.*
;*                                                             *
;* EACH SECTOR HAS A 16-BIT VALUE IN ITS HEADER THAT IS THE    *
;* BYTE COUNT FOR THE FILE TO LOAD - THIS MANY BYTES ARE READ  *
;* FROM THE DISK. WHEN DONE (ASSUMING NO ERRORS), THIS PROGRAM *
;* JUMPS TO 0000 (DMAADR), TO EXECUTE THE LOADED CODE.         *
;*                                                             *
;* SECTORS ARE INTERLEAVED 2:1 ON THE DISK, THE EVEN SECTORS   *
;* ARE READ FIRST, AND THEN THE ODD SECTORS.                   *
;*                                                             *
;* WHEN DATA IS MOVED FROM THE RAM BUFFER TO ITS FINAL MEMORY  *
;* LOCATION, IT IS READ BACK TO VERIFY CORRECT WRITE. ANY      *
;* FAILURE WILL RESULT IN AN ABORT WITH A 'M' ERROR.           *
;*                                                             *
;* ANY READ ERRORS (EITHER A CHECKSUM ERROR OR AN INCORRECT    *
;* SYNC BYTE) WILL CAUSE A RETRY OF THE SECTOR READ. AFTER     *
;* 10H RETRIES, THIS PROGRAM WILL ABORT WITH A 'C' ERROR.      *
;*                                                             *
;* IF THE PROGRAM ABORTS BECAUSE OF AN ERROR, IT WILL ALSO     *
;* TURN THE FRONT PANEL 'INTE' LED ON.                         *
;*                                                             *
;*   DISK SECTOR FORMAT               BUFFER ADDRESS           *
;*     1 BYTE:   ?                       2CEBH                 *
;*     2 BYTES: 16-BIT FILE-SIZE         2CECH                 *
;*   128 BYTES: DATA PAYLOAD             2CEEH                 *
;*     1 BYTE:  SYNC (FFH)               2D6EH                 *
;*     1 BYTE:  CHECKSUM                 2D6FH                 *
;*     1 BYTE:  ?                        2D70H                 *
;*                                                             *
;* MODIFIED TO ASSEMBLE WITH INTEL 8080 CROSS ASSEMBLER        *
;* JULY 2018, UDO MUNK                                         *
;***************************************************************

DMAADR  EQU     0000H           ;JUMPS HERE ONCE LOAD IS DONE
RUNLOC  EQU     2C00H           ;RELOCATE LOADER HERE

RETRIES EQU     10H             ;MAX NUMBER OF RETRIES

SENSE   EQU     0FFH            ;FRONT PANEL SENSE SWITCHES

; 2SIO REGISTERS

S2C0    EQU     10H     ;ACIA 0 CONTROL OUTPUT PORT
S2S0    EQU     10H     ;ACIA 0 STATUS INPUT PORT
S2T0    EQU     11H     ;ACIA 0 TX DATA REGISTER
S2R0    EQU     11H     ;ACIA 0 RX DATA REGISTER
S2C1    EQU     12H     ;ACIA 1 CONTROL OUTPUT PORT
S2S1    EQU     12H     ;ACIA 1 STATUS INPUT PORT
S2T1    EQU     13H     ;ACIA 1 TX DATA REGISTER
S2R1    EQU     13H     ;ACIA 1 RX DATA REGISTER

; 2SIO EQUATES

SIO2RST EQU     00000011B       ;MASTER RESET

; SIO REGISTERS

SIOCTRL EQU     0               ;CONTROL PORT
SIOSTAT EQU     0               ;STATUS
SIOTXD  EQU     1               ;TRANSMIT DATA
SIORXD  EQU     1               ;RECEUVE DATA

; PIO REGISTERS 

PIOCTRL EQU     4               ;CONTROL PORT
PIOSTAT EQU     4               ;STATUS
PIOTXD  EQU     5               ;TRANSMIT DATA
PIORXD  EQU     5               ;RECEUVE DATA

; 4PIO REGISTERS        

P4CA0   EQU     20H             ;PORT 0 SECTION A CTRL/STATUS
P4DA0   EQU     21H             ;PORT 0 SECTION A DATA
P4CB0   EQU     22H             ;PORT 0 SECTION B CTRL/STATUS
P4DB0   EQU     23H             ;PORT 0 SECTION B DATA

; DISK CONTROLLER INPUT EQUATES

DSTAT   EQU     08H             ;DISK STATUS REGISTER
MOVEOK  EQU             02H     ;HEAD MOVEMENT ALLOWED
ENABLD  EQU             08H     ;0 MEANS CONTROLLER IS ENABLED
TRACK0  EQU             40H     ;TRACK 0 DETECT

DSECTR  EQU     09H             ;DISK SECTOR NUMBER
SECTRU  EQU             01H     ;SECTOR VALUE IS TRUE

DDATA   EQU     0AH             ;READ DATA HERE

;DISK CONTROLLER OUTPUT EQUATES
DSLCT   EQU     08H             ;SELECT DISK NO.
DISABL  EQU             80H     ;DSLCT VALUE TO DISABLE ALL

DCTRL   EQU     09H             ;DISK CONTROL REG
STEPIN  EQU             01H     ;STEP IN
STEPOT  EQU             02H     ;STEP OUT
HDLOAD  EQU             04H     ;HEAD LOAD
HDUNLD  EQU             08H     ;HEAD UNLOAD
INTEN   EQU             10H     ;INTERRUPT ENABLE
INTDE   EQU             20H     ;INTERRUPT DISABLE
HCS     EQU             40H     ;HEAD CURRENT SWITCH
WRITEN  EQU             80H     ;WRITE ENABLE


;***************************************************************
; CODE MOVER: MOVES LOADER INTO LOW MEMORY
;***************************************************************

        ORG     0FF00H

        LXI     H,RCODE         ;SOURCE
        LXI     D,RUNLOC        ;DESTINATION
        MVI     C,EOP-RCODE     ;BYTE COUNT

MLUP:   MOV     A,M             ;GET SOURCE BYTE
        STAX    D               ;PUT IT IN PLACE
        INX     H               ;BUMP POINTERS
        INX     D
        DCR     C               ;DONE YET?
        JNZ     MLUP            ;NO: KEEP MOVING
        JMP     RUNLOC          ;YES: GO EXECUTE IT

;***************************************************************
; THE FOLLOWING LOADER CODE GETS RELOCATED TO 'RUNLOC' BY THE
; ABOVE MOVER. ALL ADDRESSES ARE ADJUSTED FOR WHERE THIS CODE
; WILL ACTUALY RUN.
;***************************************************************

RCODE:  DI                      ;FRONT PANEL INTE LED OFF
                                ;BECAUSE NO ERROR YET.

;CALCULATE CODE ADDRESS OFFSET FOR RELOCATED CODE

OFFSET  EQU     RCODE-RUNLOC    ;SUBTRACT FROM ALL ADDRESSES

;---------------------------------------------------------------
; INITIALIZATION
;---------------------------------------------------------------
;INITIALIZE 4PIO

        XRA     A               ;ACCESS DDR REGISTER, ETC.
        OUT     P4CB0  

        CMA                     ;SET PORT B AS INPUT
        OUT     P4DB0  

        MVI     A,2CH           ;READY BIT ON E PULSE, ETC.
        OUT     P4CB0  

;INITIALIZE THE 2SIO. READ TEH SENSE SWITCHES TO DETERMINE THE
;NUMBER OF STOP BITS. IF SWITCH A12 IS UP, IT'S ONE STOP BIT.
;OTHERWISE. IT'S 2 STOP BITS. ALWAYS SET UP FOR 8-BIT DATA AND
;NO PARITY.

        MVI     A,SIO2RST       ;RESET COMMAND
        OUT     S2C0  
        IN      SENSE           ;READ SENSE SWITCHES
        ANI     10H             ;GET STOP BIT SELECT FOR 2SIO
        RRC                     ;MAKE IT ACIA WORD SELECT 0
        RRC
        ADI     10H             ;WORD SELECT 2 FOR 8 BIT DATA
        OUT     10H             ;8 BITS, 1-2 STOPS, NO PARITY

;SET UP THE STACK IN MEMORY AFTER THIS PROGRAM AND AFTER
;THE DISK DATA BUFFER

        LXI     SP,STACK        ;SET UP STACK

;WAIT FOR CONTROLLER TO BE ENABLED (INCLUDING DOOR SHUT)

WAITEN: XRA     A               ;SELECT DRIVE 0
        OUT     DSLCT

        IN      DSTAT
        ANI     ENABLD          ;THIS BIT 0 WHEN ENABLED
        JNZ     WAITEN-OFFSET   ;KEEP WAITING

;LOAD THE HEAD

        MVI     A,HDLOAD
        OUT     DCTRL
        JMP     CHK00-OFFSET    ;ALREADY AT TRACK 00?

; STEP OUT ONE TRACK AT A TIME TO SEEK TRACK 00

SEEK00: IN      DSTAT           ;WAIT FOR HEAD MOVEMENT ALLOWED
        ANI     MOVEOK
        JNZ     SEEK00-OFFSET   ;KEEP WAITING

        MVI     A,STEPOT        ;STEP OUT A TRACK
        OUT     DCTRL

CHK00:  IN      DSTAT           ;ARE WE AT TRACK 0 ALREADY?
        ANI     TRACK0
        JNZ     SEEK00-OFFSET   ;NO: KEEP STEPPING


        LXI     D,DMAADR        ;PUT DISK DATA STARTING HERE

;---------------------------------------------------------------
; READ DISK DATA UNTIL WE'VE READ AS MEANY BYTES AS INDICATED
; AS THE FILE SIZE IN THE SECTOR HEADERS, AND PUT IT AT (DE)
;---------------------------------------------------------------
NXTRAC: MVI     B,0             ;INITIAL SECTOR NUMBER

NXTSEC: MVI     A,RETRIES       ;INITIALIZE RETRY COUNTER

;READ ONE SECTOR INTO THE BUFFER
; ON ENTRY:
;    A = RETRIES
;    B = SECTOR NUMBER
;   DE = MEMORY ADDRESS FOR SECTOR DATA

RDSECT: PUSH    PSW             ;SAVE RETRY COUNTER
        PUSH    D               ;SAVE DEST ADDRESS FOR RETRY
        PUSH    B               ;SAVE B=SECTOR NUMBER
        PUSH    D               ;SAVE DEST ADDRESS FOR MOVE
        LXI     D,8086H         ;E=BYTES PER SECTOR, D=JUNK
        LXI     H,BUFFER        ;HL POINTS TO DISK BUFFER

; WAIT UNTIL THE RIGHT SECTOR

WSECT:  IN      DSECTR          ;READ SECTOR STATUS
        RAR                     ;TEST BIT 0 = SECTRU
        JC      WSECT-OFFSET    ;SPIN UNTIL SECTOR IS READY

        ANI     1FH             ;GET THE SECTOR NUMBER
        CMP     B               ;IS IT THE ONE WE WANT?
        JNZ     WSECT-OFFSET    ;NO: WAIT FOR OUR SECTOR

;---------------------------------------------------------------
; LOOP TO READ 128 + 6  BYTES FROM THE DISK AND PUT INTO THE RAM
; BUFFER. THIS READING IS DONE 2 BYTES AT A TIME FOR SPEED
;---------------------------------------------------------------
DWAIT:  IN      DSTAT           ;DATA READY?
        ORA     A               ;MSB CLEARED WHEN READY
        JM      DWAIT-OFFSET

        IN      DDATA           ;GET A BYTE OF DISK DATA
        MOV     M,A             ;PUT IT IN MEMORY
        INX     H               ;BUMP MEMORY POINTER
        DCR     E               ;BUMP & TEST BYTE COUNT
        JZ      SECDON-OFFSET   ;QUIT IF BYTE COUNT = 0

        DCR     E               ;BUMP & TEST BYTE COUNT AGAIN
        IN      DDATA           ;GET ANOTHER BYTE OF DATA
        MOV     M,A             ;PUT IT IN MEMORY
        INX     H               ;BUMP MEMORY POINTER
        JNZ     DWAIT-OFFSET    ;AGAIN, UNLESS BYTE COUNT = 0
SECDON:

;---------------------------------------------------------------
; MOVE THE DATA TO ITS FINAL LOCATION, AND CHECK THE CHECKSUM AS
; WE MOVE THE DATA. ALSO VERIFY THE MEMORY WRITE.
;---------------------------------------------------------------
        POP     H                       ;RECOVER DEST ADDRESS
        LXI     D,BUFFER+3              ;START OF DATA PAYLOAD
        LXI     B,0080H                 ;B=INITIAL CHECKSUM,
                                        ;C=DATA BYTES/SECTOR

MOVLUP: LDAX    D               ;GET A BYTE FROM THE BUFFER
        MOV     M,A             ;WRITE IT TO RAM
        CMP     M               ;SUCCESSFUL WRITE TO RAM?
        JNZ     MEMERR-OFFSET   ;NO: GIVE UP

        ADD     B               ;COMPUTE CHECKSUM
        MOV     B,A

        INX     D               ;BUMP SOURCE POINTER
        INX     H               ;BUMP DESTINATION POINTER
        DCR     C               ;NEXT BYTE
        JNZ     MOVLUP-OFFSET   ;KEEP GOING THROUGH 128 BYTES


        LDAX    D               ;THE NEXT BYTE MUST BE FF
        CPI     0FFH
        JNZ     RDDONE-OFFSET   ;OTHERWISE IT'S A BAD READ

        INX     D               ;THE NEXT BYTE IS THE CHECKSUM
        LDAX    D
        CMP     B               ;MATCH THE COMPUTED CHECKSUM?

RDDONE: POP     B               ;RESTORE SECTOR NUMBER
        XCHG                    ;PUT MEMORY ADDRESS INTO DE
                                ;AND BUFFER POINTER INTO HL
        JNZ     BADSEC-OFFSET   ;CHECKSUM ERROR OR MISSING FF?

        POP     PSW             ;CHUCK OLD SECTOR NUMBER
        POP     PSW             ;CHUCK OLD RAM ADDRESS
        LHLD    BUFFER+1        ;GET FILE BYTE COUNT FROM HEADER
        CALL    CMP16-OFFSET    ;COMPARE TO NEXT RAM ADDRESS
        JNC     DONE-OFFSET     ;DONE IF ADDRESS > FILE SIZE

;---------------------------------------------------------------
; SET UP FOR NEXT SECTOR
; THE DISK HAS A 2:1 SECTOR INTERLEAVE - 
; FIRST READ ALL THE EVEN SECTORS, THEN READ ALL THE ODD SECTORS
;---------------------------------------------------------------
        INR     B               ;BUMP SECTOR NUMBER BY 2
        INR     B
        MOV     A,B             ;LAST EVEN OR ODD SECTOR ALREADY?
        CPI     20H
        JC      NXTSEC-OFFSET   ;NO: KEEP READING

        MVI     B,1             ;START READING THE ODD SECTORS
        JZ      NXTSEC-OFFSET   ;UNLESS WE FINISHED THEM TOO

; SEEK THE NEXT TRACK

WAITHD: IN      DSTAT           ;WAIT UNTIL WE CAN MOVE THE HEAD
        ANI     MOVEOK
        JNZ     WAITHD-OFFSET

        MVI     A,STEPIN        ;SEND STEP-IN CMD TO CONTROLLER
        OUT     DCTRL
        JMP     NXTRAC-OFFSET   ;BEGINNING OF THE NEXT TRACK


DONE:   MVI     A,DISABL        ;DISABLE DISKS
        OUT     DSLCT
        JMP     DMAADR          ;GO EXECUTE WHAT WE LOADED

;---------------------------------------------------------------
; SECTOR ERROR:
; RESTORE TO BEGINNING OF SECTOR AND SEE IF WE CAN RETRY
;---------------------------------------------------------------
BADSEC: POP     D               ;RESTORE MEMORY ADDRESS
        POP     PSW             ;GET RETRY COUNTER
        DCR     A               ;BUMP RETRY COUNTER
        JNZ     RDSECT-OFFSET   ;NOT ZERO: TRY AGAIN

; FALL INTO SECERR

;---------------------------------------------------------------
;ERROR ABORT ROUTINE: WRITE ERROR INFO TO MEMORY AT 0, HANG
;FOREVER, WRITING A ONE-CHARACTER ERROR CODE TO ALL OUTPUT PORTS
; ENTRY AT SECERR PRINTS 'C', SAVES BUFFER POINTER AT 0001H
;   THE BUFFER POINTER WILL BE 2D6EH IF IT WAS A SYNCHRONIZATION
;   ERROR, AND IT WILL BE 2D6FH IF IT WAS A CHECKSUM ERROR
; ENTRY AT MEMERR PRINTS 'M', SAVES OFFENDING ADDRESS AT 0001H
; THE FRONT PANEL INTE LED GETS TURNED ON TO INDICATE AN ERROR.
;---------------------------------------------------------------
SECERR: MVI     A,'C'           ;ERROR CODE

        DB      01              ;USE "LXI B" TO SKIP 2 BYTES

MEMERR: MVI     A,'M'           ;MEMORY ERROR

        EI                      ;TURN FORNT PANEL INTE LED ON

        STA     DMAADR          ;SAVE ERROR CODE AT 0000
        SHLD    DMAADR+1        ;SAVE OFFENDING ADDRESS AT 0001

        MOV     B,A             ;SAVE EROR CODE FOR A MOMENT
        MVI     A,DISABL        ;DESELECT ALL DISKS
        OUT     DSLCT
        MOV     A,B             ;RECOVER ERROR CODE

;HANG FOREVER, WRITING ERROR CODE (IN A) TO EVERY KNOWN PORT

ERHANG: OUT     SIOTXD          ;WRITE ERROR CODE TO SIO
        OUT     S2T0            ;WRITE ERROR CODE TO 2SIO
        OUT     PIOTXD          ;WRITE ERROR CODE TO PIO
        OUT     P4DB0           ;WRITE ERROR CODE TO 4PIO
        JMP     ERHANG-OFFSET   ;HANG FOREVER

;---------------------------------------------------------------
; SUBROUTINE TO COMPARE DE to HL
; C SET IF HL>DE
;---------------------------------------------------------------
CMP16:  MOV     A,D             ;LOW BYTES EQUAL?
        CMP     H
        RNZ                     ;NO: RET WITH C CORRECT
        MOV     A,E             ;HIGH BYTES EQUAL?
        CMP     L
        RET                     ;RETURN WITH RESULT IN C

EOP:                            ;END OF PROGRAM CODE
        DW      00H             ;FILLS THE EPROM OUT WITH 0

;---------------------------------------------------------------
;DISK BUFFER IN RAM AFTER RELOCATED LOADER
;---------------------------------------------------------------
        ORG     2CEBH
BUFFER: DS      132

;---------------------------------------------------------------
; AND FINALLY THE STACK, WHICH GROWS DOWNWARD
;---------------------------------------------------------------
        DS      10              ;SPACE FOR STACK
STACK   EQU     $

        END

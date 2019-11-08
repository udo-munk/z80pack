;
;       CP/M 2.2 CBIOS FOR IMSAI 8080 WITH FIF FDC AND VIO VIDEO
;
;       COPYRIGHT (C) 2017,2019 BY UDO MUNK
;
;       CAN BE ASSEMBLED WITH INTEL MACRO 8080 ASSEMBLER
;       OR DRI 8080 ASSEMBLER, SO ALL UPPERCASE, NO TABS
;

MSIZE   EQU     54              ;MEMORY SIZE IN KILOBYTES

BIAS    EQU     (MSIZE-20)*1024 ;OFFSET FROM 20K SYSTEM
CCP     EQU     BIAS+3400H      ;BASE OF CCP
BDOS    EQU     CCP+806H        ;BASE OF BDOS
BIOS    EQU     CCP+1600H       ;BASE OF BIOS

NSECTS  EQU     (BIOS-CCP)/128  ;WARM START SECTOR COUNT

CDISK   EQU     04H             ;CURRENT DISK NUMBER 0=A,1=B,...,15=P
IOBYTE  EQU     03H             ;INTEL I/O BYTE
INITIO  EQU     10010100B       ;CON:=TTY, RDR:=PTR:, PUN:=PTP, LST:=LPT:

FIF     EQU     40H             ;STORAGE FOR FIF DISK DESCRIPTOR
DDCMD   EQU     0               ;OFFSET FOR UNIT/COMMAND
DDRES   EQU     1               ;OFFSET FOR RESULT CODE
DDHTRK  EQU     2               ;OFFSET FOR TRACK HIGH
DDLTRK  EQU     3               ;OFFSET FOR TRACK LOW
DDSEC   EQU     4               ;OFFSET FOR SECTOR
DDLDMA  EQU     5               ;OFFSET FOR DMA ADDRESS LOW
DDHDMA  EQU     6               ;OFFSET FOR DMA ADDRESS HIGH

;
;       IMSAI 8080 I/O PORTS
;
TTY     EQU     02H             ;TTY DATA PORT
TTYS    EQU     03H             ;TTY STATUS PORT

KBD     EQU     04H             ;KEYBOARD DATA PORT
KBDS    EQU     05H             ;KEYBOARD STATUS PORT

SIO1C   EQU     08H             ;CONTROL PORT FOR FIRST SIO BOARD

AUX     EQU     22H             ;AUX DATA PORT
AUXS    EQU     23H             ;AUX STATUS PORT

USR     EQU     24H             ;USER DATA PORT
USRS    EQU     25H             ;USER STATUS PORT

SIO2C   EQU     28H             ;CONTROL PORT FOR SECOND SIO BOARD

PRINTER EQU     0F6H            ;IMSAI PTR-300 LINE PRINTER PORT
PIC8    EQU     0F7H            ;INTERRUPT PRIORITY CONTROLLER
FDC     EQU     0FDH            ;FDC PORT
LEDS    EQU     0FFH            ;PROGRAMMED OUTPUT LEDS

;
;       IMSAI VIO VIDEO DISPLAY BOARD
;
VIOINIT EQU     0F800H          ;INIT CALL IN VIO FIRMWARE
VIOOUT  EQU     0F803H          ;CHARACTER OUTPUT CALL IN VIO FIRMWARE
VIOTST  EQU     0FFFDH          ;FIRMWARE SIGNATURE WITH STRING VI0

        ORG     BIOS            ;ORIGIN OF BIOS

;
;       JUMP VECTOR FOR INDIVIDUAL SUBROUTINES
;
        JMP     BOOT            ;COLD BOOT
WBE:    JMP     WBOOT           ;WARM START
        JMP     CONST           ;CONSOLE STATUS
        JMP     CONIN           ;CONSOLE CHARACTER IN
        JMP     CONOUT          ;CONSOLE CHARACTER OUT
        JMP     LIST            ;LIST CHARACTER OUT
        JMP     PUNCH           ;PUNCH CHARACTER OUT
        JMP     READER          ;READER CHARACTER IN
        JMP     HOME            ;MOVE DISK HEAD TO HOME POSITION
        JMP     SELDSK          ;SELECT DISK DRIVE
        JMP     SETTRK          ;SET TRACK NUMBER
        JMP     SETSEC          ;SET SECTOR NUMBER
        JMP     SETDMA          ;SET DMA ADDRESS
        JMP     READ            ;READ DISK SECTOR
        JMP     WRITE           ;WRITE DISK SECTOR
        JMP     LISTST          ;LIST STATUS
        JMP     SECTRAN         ;SECTOR TRANSLATE

;
;       DATA TABLES
;
SIGNON: DB      MSIZE / 10 + '0',MSIZE MOD 10 + '0'
        DB      'K CP/M VERS 2.2 B01',13,10,0
VIOERR: DB      13,10,'NO VIO',13,10,0

;       BYTES FOR SIO INITIALIZATION
SIOSTR: DB      0AEH,40H,0AEH,37H,0

;       DISK PARAMETER HEADER FOR DISK 0
DPBASE: DW      TRANS,0000H
        DW      0000H,0000H
        DW      DIRBF,DPBLK
        DW      CHK00,ALL00
;       DISK PARAMETER HEADER FOR DISK 1
        DW      TRANS,0000H
        DW      0000H,0000H
        DW      DIRBF,DPBLK
        DW      CHK01,ALL01
;       DISK PARAMETER HEADER FOR DISK 2
        DW      TRANS,0000H
        DW      0000H,0000H
        DW      DIRBF,DPBLK
        DW      CHK02,ALL02
;       DISK PARAMETER HEADER FOR DISK 3
        DW      TRANS,0000H
        DW      0000H,0000H
        DW      DIRBF,DPBLK
        DW      CHK03,ALL03

;       SECTOR TRANSLATE TABLE FOR IBM 8" SD DISKS
TRANS:  DB      1,7,13,19       ;SECTORS 1,2,3,4
        DB      25,5,11,17      ;SECTORS 5,6,7,8
        DB      23,3,9,15       ;SECTORS 9,10,11,12
        DB      21,2,8,14       ;SECTORS 13,14,15,16
        DB      20,26,6,12      ;SECTORS 17,18,19,20
        DB      18,24,4,10      ;SECTORS 21,22,23,24
        DB      16,22           ;SECTORS 25,26

;       DISK PARAMETER BLOCK FOR IBM 8" SD DISKS
DPBLK:  DW      26              ;SECTORS PER TRACK
        DB      3               ;BLOCK SHIFT FACTOR
        DB      7               ;BLOCK MASK
        DB      0               ;EXTENT MASK
        DW      242             ;DISK SIZE-1
        DW      63              ;DIRECTORY MAX
        DB      192             ;ALLOC 0
        DB      0               ;ALLOC 1
        DW      16              ;CHECK SIZE
        DW      2               ;TRACK OFFSET

;       DISK PARAMETER HEADER HARDDISK
DPHHD:  DW      0000H,0000H
        DW      0000H,0000H
        DW      DIRBF,HDBLK
        DW      CHKHD1,ALLHD1

;       DISK PARAMETER BLOCK HARDDISK
HDBLK:  DW      128             ;SECTORS PER TRACK
        DB      4               ;BLOCK SHIFT FACTOR
        DB      15              ;BLOCK MASK
        DB      0               ;EXTENT MASK
        DW      2039            ;DISK SIZE-1
        DW      1023            ;DIRECTORY MAX
        DB      255             ;ALLOC 0
        DB      255             ;ALLOC 1
        DW      0               ;CHECK SIZE
        DW      0               ;TRACK OFFSET

;
;       PRINT A MESSAGE TO THE CONSOLE
;       POINTER TO STRING IN HL
;
PRTMSG: MOV     A,M             ;GET NEXT MESSAGE BYTE
        ORA     A               ;IS IT ZERO?
        RZ                      ;YES, DONE
        MOV     C,A             ;NO, PRINT CHARACTER ON CONSOLE
        CALL    CONOUT
        INX     H               ;AND DO NEXT
        JMP     PRTMSG

;
;       COLD START
;
BOOT:   LXI     SP,80H          ;USE SPACE BELOW BUFFER FOR STACK
        LXI     H,SIOSTR        ;INITIALIZE ALL SIOS
        MOV     A,M             ;GET NEXT BYTE FROM INIT STRING
BO1:    OUT     TTYS            ;OUTPUT TO THE SIO STATUS PORTS
        OUT     KBDS
        OUT     AUXS
        OUT     USRS
        INX     H               ;NEXT ONE
        MOV     A,M
        ORA     A               ;IS IT ZERO ?
        JNZ     BO1             ;NO, ONE MORE
        XRA     A               ;TURN OFF INTERRUPTS AND
        OUT     SIO1C           ;CARRIER DETECT ON ALL SIO CHANNELS
        OUT     SIO2C
        MVI     A,00001001B     ;DISABLE ALL INTERRUPTS BUT CHANNEL 7
        OUT     PIC8
        XRA     A               ;ZERO IN THE ACCUMULATOR
        STA     CDISK           ;SELECT DISK DRIVE 0
        STA     FIF+DDHTRK      ;NOT USED FOR FLOPPY DISK
        STA     VIOF            ;CLEAR VIO FLAG
        MVI     A,10H           ;SETUP FDC DISK DESCRIPTOR
        OUT     FDC
        MVI     A,FIF AND 0FFH
        OUT     FDC
        MVI     A,FIF SHR 8
        OUT     FDC
        MVI     A,INITIO        ;INITIALIZE IOBYTE
        STA     IOBYTE
        LXI     H,VIOTST        ;CHECK IF VIO BOARD AVAILABLE
        MOV     A,M             ;VERIFY FIRST SIGNATURE BYTE
        CPI     'V'
        JNZ     BO2
        INX     H               ;VERIFY SECOND SIGNATURE BYTE
        MOV     A,M
        CPI     'I'
        JNZ     BO2
        INX     H               ;VERIFY THIRD SIGNATURE BYTE
        MOV     A,M
        CPI     '0'
        JNZ     BO2
        MVI     A,0FFH          ;VIO FIRMWARE FOUND, SET VIO FLAG
        STA     VIOF
        MVI     A,INITIO OR 1   ;AND SET IOBYTE CON:=CRT:
        STA     IOBYTE
        CALL    VIOINIT         ;AND INITIALIZE THE BOARD
BO2:    IN      PRINTER         ;GET STATUS PRINTER PORT, F4 IS OK
        ANI     04H             ;MASK THE TELLING BIT
        JZ      BO3             ;SKIP INIT IF BIT OFF, MIGHT HANG US
        MVI     A,80H           ;ELSE SEND INIT COMMAND
        OUT     PRINTER
BO3:    LXI     H,SIGNON        ;PRINT MESSAGE
        CALL    PRTMSG
        JMP     GOCPM           ;INITIALIZE AND GO TO CP/M

;
;       WARM START
;
WBOOT:  LXI     SP,80H          ;USE SPACE BELOW BUFFER FOR STACK
        MVI     C,0             ;SELECT DISK 0
        CALL    SELDSK
        CALL    HOME            ;GO TO TRACK 0
        MVI     B,NSECTS        ;B COUNTS # OF SECTORS TO LOAD
        MVI     C,0             ;C HAS THE CURRENT TRACK #
        MVI     D,2             ;D HAS THE NEXT SECTOR TO LOAD
        LXI     H,CCP           ;BASE OF CP/M
LOAD1:  PUSH    B               ;SAVE SECTOR COUNT AND CURRENT TRACK
        PUSH    D               ;SAVE NEXT SECTOR TO READ
        PUSH    H               ;SAVE DMA ADDRESS
        MOV     C,D             ;GET SECTOR ADDRESS TO C
        CALL    SETSEC          ;SET SECTOR ADDRESS
        POP     B               ;RECALL DMA ADDRESS TO BC
        PUSH    B               ;AND REPLACE ON STACK FOR LATER RECALL
        CALL    SETDMA          ;SET DMA ADDRESS FROM BC
        CALL    READ            ;READ SECTOR
        ORA     A               ;ANY ERRORS?
        JZ      LOAD2           ;NO, CONTINUE
        HLT                     ;OTHERWISE HALT THE MACHINE
LOAD2:  POP     H               ;RECALL DMA ADDRESS
        LXI     D,128           ;DMA = DMA + 128
        DAD     D               ;NEXT DMA ADDRESS NOW IN HL
        POP     D               ;RECALL SECTOR ADDRESS
        POP     B               ;RECALL # OF SECTORS REMAINING
        DCR     B               ;SECTORS = SECTORS - 1
        JZ      GOCPM           ;TRANSFER TO CP/M IF ALL LOADED
        INR     D               ;NEXT SECTOR
        MOV     A,D             ;SECTOR = 27 ?
        CPI     27
        JC      LOAD1           ;NO, CONTINUE
        MVI     D,1             ;ELSE BEGIN WITH SECTOR 1 ON NEXT TRACK
        INR     C
        CALL    SETTRK
        JMP     LOAD1           ;FOR ANOTHER SECTOR
GOCPM:  MVI     A,0C3H          ;C3 IS A JMP INSTRUCTION
        STA     0               ;FOR JMP TO WBOOT
        LXI     H,WBE           ;WBOOT ENTRY POINT
        SHLD    1               ;SET ADDRESS FOR JMP AT 0
        STA     5               ;FOR JMP TO BDOS
        LXI     H,BDOS          ;BDOS ENTRY POINT
        SHLD    6               ;SET ADDRESS FOR JMP AT 5
        LXI     B,80H           ;DEFAULT DMA ADDRESS IS 80H
        CALL    SETDMA
        LDA     CDISK           ;GET CURRENT DISK NUMBER
        MOV     C,A             ;SEND TO THE CCP
        JMP     CCP             ;GO TO CP/M FOR FURTHER PROCESSING

;***************************************************************************
;       LOGICAL DEVICE ROUTINES
;
;  THESE ROUTINES USE THE PHYSICAL DEVICE ROUTINES
;  DEPENDING ON CONTENTS OF IOBYTE.
;***************************************************************************

;
;       DISPATCH TO ONE OF 4 FOLLOWING ADDRESSES
;       DEPENDING ON CONTENTS OF TWO BITS OF IOBYTE.
;       SPECIFIC BITS OF IOBYTE ARE SPECIFIED BY
;       SHIFT COUNT FOLLOWING THE SUBROUTINE CALL.
;
DISPATCH:
        XTHL                   ;SAVE CALLERS HL, GET TABLE ADDR
        MOV     D,M            ;GET SHIFT COUNT INTO D
        INX     H              ;POINT TO TABLE
        LDA     IOBYTE         ;GET IOBYTE
DSHIFT: RLC                    ;SHIFT TO POSITION BITS
        DCR     D
        JNZ     DSHIFT
        ANI     06H            ;MASK BITS
        MOV     E,A            ;D ALREADY CLEAR
        DAD     D              ;INDEX INTO TABLE
        MOV     A,M            ;GET TABLE WORD INTO HL
        INX     H
        MOV     H,M
        MOV     L,A
        XTHL                   ;PUT ADDR OF ROUTINE, GET CALLERS HL
        RET                    ;GO TO ROUTINE

;
;       CONSOLE STATUS, RETURN 0FFH IF CHARACTER READY, 00H IF NOT
;
CONST:  CALL    DISPATCH        ;GO TO ONE OF THE PHYSICAL DEVICE ROUTINES
        DB      1               ;USE BITS 1-0 OF IOBYTE
        DW      TTYIST          ;00 - TTY:
        DW      CRTIST          ;01 - CRT:
        DW      AUXIST          ;10 - BAT:
;       DW      AUXIST          ;11 - UC1:
        DW      USRIST          ;11 - UC1:

;
;       CONSOLE INPUT CHARACTER INTO REGISTER A
;
CONIN:  CALL    DISPATCH        ;GO TO ONE OF THE PHYSICAL DEVICE ROUTINES
        DB      1               ;USE BITS 1-0 OF IOBYTE
        DW      TTYIN           ;00 - TTY:
        DW      CRTIN           ;01 - CRT:
        DW      AUXIN           ;10 - BAT:
;       DW      AUXIN           ;11 - UC1:
        DW      USRIN           ;11 - UC1:

;
;       CONSOLE OUTPUT CHARACTER FROM REGISTER C
;
CONOUT: CALL    DISPATCH        ;GO TO ONE OF THE PHYSICAL DEVICE ROUTINES
        DB      1               ;USE BITS 1-0 OF IOBYTE
        DW      TTYOUT          ;00 - TTY:
        DW      CRTOUT          ;01 - CRT:
        DW      LPTOUT          ;10 - BAT:
;       DW      AUXOUT          ;11 - UC1:
        DW      USROUT          ;11 - UC1:

;
;       PRINTER STATUS, RETURN 0FFH IF CHARACTER READY, 00H IF NOT
;
LISTST: CALL    DISPATCH        ;GO TO ONE OF THE PHYSICAL DEVICE ROUTINES
        DB      3               ;USE BITS 7-6 OF IOBYTE
        DW      TTYOST          ;00 - TTY:
        DW      CRTOST          ;01 - CRT:
        DW      LPTST           ;10 - LPT:
        DW      NOST            ;11 - UL1:

;
;       LIST OUTPUT CHARACTER FROM REGISTER C
;
LIST:   CALL    DISPATCH        ;GO TO ONE OF THE PHYSICAL DEVICE ROUTINES
        DB      3               ;USE BITS 7-6 OF IOBYTE
        DW      TTYOUT          ;00 - TTY:
        DW      CRTOUT          ;01 - CRT:
        DW      LPTOUT          ;10 - LPT:
        DW      NULLOUT         ;11 - UL1:

;
;       PUNCH CHARACTER FROM REGISTER C
;
PUNCH:  CALL    DISPATCH        ;GO TO ONE OF THE PHYSICAL DEVICE ROUTINES
        DB      5               ;USE BITS 5-4 OF IOBYTE
        DW      TTYOUT          ;00 - TTY:
        DW      AUXOUT          ;01 - PTP:
        DW      USROUT          ;10 - UP1:
        DW      CRTOUT          ;11 - UP2:

;
;       READ CHARACTER INTO REGISTER A FROM READER
;
READER: CALL    DISPATCH        ;GO TO ONE OF THE PHYSICAL DEVICE ROUTINES
        DB      7               ;USE BITS 3-2 of IOBYTE
        DW      TTYIN           ;00 - TTY:
        DW      AUXIN           ;01 - RDR:
        DW      USRIN           ;10 - UR1:
        DW      CRTIN           ;11 - UR2:

;***************************************************************************
;       PHYSICAL DEVICE ROUTINES
;
;  ACCESSED VIA LOGICAL DEVICE ROUTINES ABOVE
;***************************************************************************

;
;       TTY INPUT STATUS
;
TTYIST: IN      TTYS            ;GET TTY STATUS
        ANI     02H             ;MASK BIT
        RZ                      ;NOT READY
        MVI     A,0FFH          ;READY, SET FLAG
        RET

;
;       TTY OUTPUT STATUS
;
TTYOST: IN      TTYS            ;GET TTY STATUS
        RRC                     ;TEST BIT 0
        JNC     TTYO1
        MVI     A,0FFH          ;READY
        RET
TTYO1:  XRA     A               ;NOT READY
        RET

;
;       TTY INPUT
;
TTYIN:  IN      TTYS            ;GET STATUS
        ANI     02H             ;MASK BIT
        JZ      TTYIN           ;WAIT UNTIL CHARACTER AVAILABLE
        IN      TTY             ;GET CHARACTER
        RET

;
;       TTY OUTPUT
;
TTYOUT: IN      TTYS            ;GET STATUS
        RRC                     ;TEST BIT 0
        JNC     TTYOUT          ;WAIT UNTIL TRANSMITTER READY
        MOV     A,C             ;GET CHAR TO ACCUMULATOR
        OUT     TTY             ;SEND TO TTY
        RET

;
;       AUX INPUT STATUS
;
AUXIST: IN      AUXS            ;GET AUX STATUS
        ANI     02H             ;MASK BIT
        RZ                      ;NOT READY
        MVI     A,0FFH          ;READY, SET FLAG
        RET

;
;       AUX INPUT
;
AUXIN:  IN      AUXS            ;GET STATUS
        ANI     02H             ;MASK BIT
        JZ      AUXIN           ;WAIT UNTIL CHARACTER AVAILABLE
        IN      AUX             ;GET CHARACTER
        RET

;
;       AUX OUTPUT
;
AUXOUT: IN      AUXS            ;GET STATUS
        RRC                     ;TEST BIT 0
        JNC     AUXOUT          ;WAIT UNTIL TRANSMITTER READY
        MOV     A,C             ;GET CHAR TO ACCUMULATOR
        OUT     AUX             ;SEND TO AUX
        RET

;
;       USER INPUT STATUS
;
USRIST: IN      USRS            ;GET USER STATUS
        ANI     02H             ;MASK BIT
        RZ                      ;NOT READY
        MVI     A,0FFH          ;READY, SET FLAG
        RET

;
;       USER INPUT
;
USRIN:  IN      USRS            ;GET STATUS
        ANI     02H             ;MASK BIT
        JZ      USRIN           ;WAIT UNTIL CHARACTER AVAILABLE
        IN      USR             ;GET CHARACTER
        RET

;
;       USER OUTPUT
;
USROUT: IN      USRS            ;GET STATUS
        RRC                     ;TEST BIT 0
        JNC     USROUT          ;WAIT UNTIL TRANSMITTER READY
        MOV     A,C             ;GET CHAR TO ACCUMULATOR
        OUT     USR             ;SEND TO USR
        RET

;
;       VIO CRT (KBD) INPUT STATUS
;
CRTIST: IN      KBDS            ;GET STATUS
        ANI     02H             ;MASK BIT
        RZ                      ;NOT READY
        MVI     A,0FFH          ;READY, SET FLAG
        RET

;
;       VIO CRT OUTPUT STATUS
;
CRTOST: MVI     A,0FFH          ;RETURN READY
        RET

;
;       VIO CRT (KBD) INPUT
;
CRTIN:  IN      KBDS            ;GET STATUS
        ANI     02H             ;MASK BIT
        JZ      CRTIN           ;WAIT UNTIL CHARACTER AVAILABLE
        IN      KBD             ;GET CHARACTER
        RET

;
;       VIO CRT OUTPUT
;
CRTOUT: LDA     VIOF            ;VIO BOARD AVAILABLE?
        ORA     A
        JNZ     CRT1            ;YES, DO OUTPUT
        MVI     A,INITIO        ;NO, RESET IOBYTE
        STA     IOBYTE
        LXI     H,VIOERR        ;PRINT ERROR MESSAGE
        CALL    PRTMSG
        JMP     WBOOT           ;AND WARM BOOT SYSTEM
CRT1:   MOV     A,C             ;GET CHARACTER INTO ACCUMULATOR
        CALL    VIOOUT          ;AND OUTPUT VIA VIO FIRMWARE
        RET

;
;       LINE PRINTER STATUS
;
LPTST:  IN      PRINTER         ;GET PRINTER STATUS
        CPI     0FFH            ;NO PRINTER
        JZ      LPT1
        ANI     04H             ;CHECK READY BIT
        JZ      LPT1            ;NOT READY
        MVI     A,0FFH          ;ELSE RETURN WITH READY STATUS
        RET
LPT1:   XRA     A               ;PRINTER NOT READY
        RET

;
;       LINE PRINTER OUTPUT
;
LPTOUT: IN      PRINTER         ;GET PRINTER STATUS
        CPI     0FFH            ;NO PRINTER
        RZ
        ANI     04H             ;CHECK READY BIT
        JZ      LPTOUT          ;NOT READY, TRY AGAIN
        MOV     A,C             ;GET CHARACTER TO ACCUMULATOR
        OUT     PRINTER         ;OUTPUT IT
        RET

;
;       NULL DEVICE INPUT/OUPUT STATUS
;       ALWAYS READY SO THAT WE WON'T HANG
;
NIST:
NOST:
        MVI     A,0FFH          ;RETURN READY
        RET

;
;       NULL DEVICE INPUT
;
NULLIN: MVI     A,01AH          ;RETURN EOF
        RET

;
;       NULL DEVICE OUTPUT
;
NULLOUT:RET

;
;       MOVE TO TRACK 0 POSITION ON CURRENT DISK
;
HOME:   MVI     C,0             ;SELECT TRACK 0
        JMP     SETTRK

;
;       SELECT DISK GIVEN BY REGISTER C
;
SELDSK: LXI     H,0             ;ERROR RETURN CODE
        MOV     A,C             ;GET DISK # TO ACCUMULATOR
        CPI     0               ;DISK DRIVE 0 ?
        JZ      SEL0
        CPI     1               ;DISK DRIVE 1 ?
        JZ      SEL1
        CPI     2               ;DISK DRIVE 2 ?
        JZ      SEL2
        CPI     3               ;DISK DRIVE 3 ?
        JZ      SEL3
        CPI     8               ;HARDDISK ?
        JZ      SELHD
        RET                     ;NO, RETURN WITH ERROR
SEL0:   MVI     A,1             ;UNIT 1
        JMP     SELX
SEL1:   MVI     A,2             ;UNIT 2
        JMP     SELX
SEL2:   MVI     A,4             ;UNIT 3
        JMP     SELX
SEL3:   MVI     A,8             ;UNIT 4
SELX:   STA     FIF+DDCMD       ;SET DISK UNIT IN FIF DD
        MOV     L,C             ;HL = DISK #
        DAD     H               ;*2
        DAD     H               ;*4
        DAD     H               ;*8
        DAD     H               ;*16 (SIZE OF EACH HEADER)
        LXI     D,DPBASE
        DAD     D               ;HL=.DPBASE(DISKNO*16)
        RET
SELHD:  MVI     A,15            ;UNIT 15
        STA     FIF+DDCMD       ;SET DISK UNIT IN FIF DD
        LXI     H,DPHHD         ;HL = DISK PARAMETER HEADER
        RET

;
;       SET TRACK GIVEN BY REGISTER C
;
SETTRK: MOV     A,C             ;GET TO ACCUMULATOR
        STA     FIF+DDLTRK      ;SET IN FIF DD
        RET

;
;       SET SECTOR GIVEN BY REGISTER C
;
SETSEC: MOV     A,C             ;GET TO ACCUMULATOR
        STA     FIF+DDSEC       ;SET IN FIF DD
        RET

;
;       SET DMA ADDRESS GIVEN BY REGISTERS B AND C
;
SETDMA: MOV     A,C             ;LOW ORDER ADDRESS
        STA     FIF+DDLDMA      ;SET IN FIF DD
        MOV     A,B             ;HIGH ORDER ADDRESS
        STA     FIF+DDHDMA      ;SET IN FIF DD
        RET

;
;       PERFORM READ OPERATION
;
READ:   LDA     FIF+DDCMD       ;GET UNIT/COMMAND
        ANI     0FH             ;MASK OUT COMMAND
        ORI     20H             ;MASK IN READ COMMAND
        STA     FIF+DDCMD       ;SET IN FIF DD
        JMP     DOIO            ;DO I/O OPERATION

;
;       PERFORM WRITE OPERATION
;
WRITE:  LDA     FIF+DDCMD       ;GET UNIT/COMMAND
        ANI     0FH             ;MASK OUT COMMAND
        ORI     10H             ;MASK IN WRITE COMMAND
        STA     FIF+DDCMD       ;SET IN FIF DD

;
;       PERFORM READ/WRITE I/O
;
DOIO:   XRA     A               ;ZERO ACCUMULATOR
        STA     FIF+DDRES       ;RESET RESULT CODE
        OUT     FDC             ;ASK FDC TO EXECUTE THE DD
IO1:    LDA     FIF+DDRES       ;WAIT FOR FDC
        ORA     A
        JZ      IO1             ;NOT DONE YET
        ANI     0FEH            ;STATUS = 1 ?
        RZ                      ;IF YES, RETURN OK
        LDA     FIF+DDRES       ;GET STATUS AGAIN
        CMA                     ;COMPLEMENT BITS BECAUSE LEDS ARE INVERTED
        OUT     LEDS            ;DISPLAY THE ERROR CODE
        RET                     ;AND RETURN WITH ERROR

;
;       TRANSLATE THE SECTOR GIVEN BY BC USING
;       THE TRANSLATION TABLE GIVEN BY DE
;
SECTRAN:MOV     A,D             ;DO WE HAVE A TRANSLATION TABLE ?
        ORA     E
        JNZ     SECT1           ;YES, TRANSLATE
        MOV     L,C             ;NO, RETURN UNTRANSLATED
        MOV     H,B
        INR     L               ;SECTOR NUMBERS START WITH 1
        RET
SECT1:  XCHG                    ;HL=.TRANS
        DAD     B               ;HL=.TRANS(SECTOR)
        XCHG
        LDAX    D
        MOV     L,A             ;L=TRANS(SECTOR)
        MVI     H,0             ;HL=TRANS(SECTOR)
        RET                     ;WITH VALUE IN HL

;
;       THE REMAINDER OF THE CBIOS IS RESERVED UNINITIALIZED
;       DATA AREA, AND DOES NOT NEED TO BE PART OF THE SYSTEM
;       MEMORY IMAGE. THE SPACE MUST BE AVAILABE, HOWEVER,
;       BETWEEN "BEGDAT" AND "ENDDAT".
;
BEGDAT  EQU     $               ;BEGIN OF DATA AREA

VIOF:   DS      1               ;FLAG IF VIO BOARD AVAILABLE

DIRBF:  DS      128             ;SCRATCH DIRECTORY AREA
ALL00:  DS      31              ;ALLOCATION VECTOR 0
ALL01:  DS      31              ;ALLOCATION VECTOR 1
ALL02:  DS      31              ;ALLOCATION VECTOR 2
ALL03:  DS      31              ;ALLOCATION VECTOR 3
ALLHD1: DS      255             ;ALLOCATION VECTOR HARDDISK
CHK00:  DS      16              ;CHECK VECTOR 0
CHK01:  DS      16              ;CHECK VECTOR 1
CHK02:  DS      16              ;CHECK VECTOR 2
CHK03:  DS      16              ;CHECK VECTOR 3
CHKHD1: DS      0               ;CHECK VECTOR HARDDISK

ENDDAT  EQU     $               ;END OF DATA AREA
DATSIZ  EQU     $-BEGDAT        ;SIZE OF DATA AREA

        END                     ;OF BIOS

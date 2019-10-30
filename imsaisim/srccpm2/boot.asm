;
;       CP/M 2.2 BOOT LOADER FOR IMSAI 8080 WITH FIF FDC
;
;       COPYRIGHT (C) 2017,2019 BY UDO MUNK
;
;       CAN BE ASSEMBLED WITH INTEL MACRO 8080 ASSEMBLER
;       OR DRI 8080 ASSEMBLER, SO ALL UPPERCASE, NO TABS
;

        ORG     0               ;BASE OF BOOT CODE

MSIZE   EQU     54              ;MEMORY SIZE IN KBYTES

BIAS    EQU     (MSIZE-20)*1024 ;OFFSET FROM 20K SYSTEM
CPMB    EQU     BIAS+3400H      ;START OF CP/M
BOOTE   EQU     CPMB+1600H      ;COLD BOOT ENTRY POINT
SECTS   EQU     51              ;# OF SECTORS TO LOAD (26 * 2 - 1)

;
;       IMSAI 8080 I/O PORTS
;
MEMCTL  EQU     0F3H            ;MEMORY CONTROL MPU-B
FDC     EQU     0FDH            ;FDC PORT
LEDS    EQU     0FFH            ;PROGRAMMED OUTPUT LEDS

        JMP     COLD

FIF:    DB      21H             ;FDC COMMAND/UNIT READ SEC FIRST DRIVE
        DB      00H             ;FDC RESULT CODE
        DB      00H             ;DISK TRACK (HIGH) 0
        DB      00H             ;DISK TRACK (LOW) 0
        DB      02H             ;DISK SECTOR 2
        DB      CPMB AND 0FFH   ;DMA ADDRESS LOW
        DB      CPMB SHR 8      ;DMA ADDRESS HIGH

;
;       BOOT LOADER ENTRY
;
COLD:   LXI     SP,0FFH         ;SOME SPACE FOR THE STACK
        MVI     A,0C0H          ;REMOVE MPU-B FROM ADDRESS SPACE
        OUT     MEMCTL
        MVI     A,10H           ;SETUP FDC DISK DESCRIPTOR
        OUT     FDC
        MVI     A,FIF AND 0FFH
        OUT     FDC
        MVI     A,FIF SHR 8
        OUT     FDC
        LXI     B,2             ;B=TRACK 0, C=SECTOR 2
        MVI     D,SECTS         ;D=# SECTORS TO LOAD

;
;       LOAD THE NEXT SECTOR
;
LSECT:  XRA     A               ;TELL FDC TO EXECUTE THE DD
        OUT     FDC
LS1:    LDA     FIF+1           ;WAIT FOR FDC
        ORA     A
        JZ      LS1
        CPI     1               ;RESULT = 1?
        JZ      LS2             ;YES, CONTINUE
        CMA                     ;COMPLEMENT ERROR CODE
        OUT     LEDS            ;OUTPUT TO LEDS
        HLT                     ;AND HALT CPU
LS2:    DCR     D               ;SECTS=SECTS-1
        JZ      BOOTE           ;GO TO CP/M IF ALL SECTORS DONE
        INR     C               ;SECTOR = SECTOR + 1
        MOV     A,C
        CPI     27              ;LAST SECTOR OF TRACK?
        JC      LS3             ;NO, DO NEXT SECTOR
        MVI     C,1             ;SECTOR = 1
        INR     B               ;TRACK = TRACK + 1
LS3:    MOV     A,B             ;SETUP DD
        STA     FIF+3           ;SAVE TRACK
        MOV     A,C
        STA     FIF+4           ;SAVE SECTOR
        XRA     A
        STA     FIF+1           ;RESET RESULT
        PUSH    D
        LHLD    FIF+5           ;GET DMA ADDRESS
        LXI     D,80H           ;AND INCREASE IT BY 128
        DAD     D
        POP     D
        SHLD    FIF+5           ;SET NEW DMA ADDRESS
        JMP     LSECT           ;FOR NEXT SECTOR

        END

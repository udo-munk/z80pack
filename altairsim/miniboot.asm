;
; MINIMAL BOOTSTRAP PROGRAM FOR TARBELL SD CONTROLLER
; SUITABLE FOR FAST 82S123 ROM OR ENTERING PER FRONT PANEL
;
WAIT    EQU  0FCH
DCOM    EQU  0F8H
DDATA   EQU  0FBH
STAT    EQU  0F8H
SECT    EQU  0FAH

        ORG  0FE00H             ;FOR EXTERNAL BOOT ROM, 0 FOR ONBOARD ROM

        IN   WAIT               ;WAIT FOR HOME
        XRA  A                  ;SETS A TO 0
        MOV  L,A                ;START LOCATION IN RAM = 0
        MOV  H,A
        INR  A                  ;SETS A TO 1
        OUT  SECT               ;SET SECTOR REGISTER
        MVI  A,08CH             ;GET READ COMMAND
        OUT  DCOM               ;ISSUE COMMAND TO 1771
NEXT:   IN   WAIT               ;WAIT FOR INTRQ OR DRQ
        ORA  A                  ;SET FLAGS
        JP   DONE               ;DONE IF INTRQ
        IN   DDATA              ;READ BYTE FROM CONTROLLER
        MOV  M,A                ;MOVE IT TO RAM
        INX  H                  ;ADVANCE ONE BYTE
        JMP  NEXT               ;READ NEXT BYTE
DONE:   IN   STAT               ;READ DISK STATUS
        ORA  A                  ;SET FLAGS
        JZ   7DH                ;GO TO SBOOT IF ZERO
        HLT                     ;ERROR - HALT

        END

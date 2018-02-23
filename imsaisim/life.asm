; LIFE .... VERSION 2.0
; WRITTEN BY ED HALL
; ASSEMBLED BY GORDEN FRENCH
; AND RDM
;
; RETYPED FROM MANUAL APRIL 2015, UDO MUNK
; THIS VERSION MODIFIED FOR IMSAI SIO-2
;
;
; CNTL-A IS USED TO DEPOSIT A CELL OF LIFE ON THE SCREEN.
; CNTL-N, O, I AND H STEP THE CURSOR UP, DOWN, RIGHT, AND
; LEFT RESPECTIVELY. CNTL-B CAN BE USED TO ERASE THE SCREEN.
; ONCE THE INITIAL COLONY IS COMPLETE, CNTL-D IS USED TO
; START THE EVOLUTION OF THE CELLS. DURING THE COLORFUL
; EVOLUTION OF THE CELLS THE LETTER F ON YOUR KEYBOARD
; CAN BE USED TO FREEZE THE PICTURE. HIT THE LETTER G TO
; GO AND THE LETTER S TO STOP.

RED     EQU  9
GREEN   EQU  10
BLUE    EQU  12
STACK   EQU  800H
DISPLY  EQU  800H
TIME    EQU  0D800H
;DAV     EQU  32                 ; FOR ALTAIR 88SIO REV 0
;TBE     EQU  2
DAV     EQU  2                  ; FOR IMSAI SIO-2
TBE     EQU  1
DELAY   EQU  0

        JMP  START
START:  LXI  SP,STACK
        CALL INIT
MAN20:  CALL SETUP
MAN30:  CALL GEN
FRZE:   IN   2
        CPI  'F'
        JZ   FRZE
        CALL CHANGE
STP:    IN   2
        CPI  'S'
        JZ   MAN20
MAN40:  IN   255
        RAL
        JC   MAN40
        RAL
        JC   MAN20
        JMP  MAN30
GEN:    LXI  B,0
GEN20:  CALL UPDATE
        MVI  A,64
        INR  C
        CMP  C
        JNZ  GEN20
        MVI  C,0
        INR  B
        CMP  B
        JNZ  GEN20
        RET
CHANGE: LXI  B,0
CHN15:  CALL GTCOL
        CPI  RED
        JNZ  CHN20
        MVI  A,0
        CALL PTCOL
        JMP  CHN30
CHN20:  CPI  GREEN
        JNZ  CHN30
        MVI  A,BLUE
        CALL PTCOL
CHN30:  MVI  A,64
        INR  C
        CMP  C
        JNZ  CHN15
        MVI  C,0
        INR  B
        CMP  B
        JNZ  CHN15
        RET
CDISP:  LXI  H,DISPLY
        LXI  D,-2048
CLEAR:  MVI  B,0
FILL:   MVI  A,0
FLL12:  CMP  D
        JNZ  FLL20
        CMP  E
        RZ
FLL20:  MOV  M,B
        INX  D
        INX  H
        JMP  FLL12
UPDATE: PUSH B
        MVI  A,0
        CMP  C
        JNZ  UPD10
        DCR  C
        CALL UPROW
        STA  ROWA
        POP  B
        PUSH B
        CALL UPROW
        STA  ROWB
        POP  B
        PUSH B
UPD10:  INR  C
        CALL UPROW
        LXI  H,ROWB
        MOV  B,M
        MOV  M,A
        ADD  B
        DCX  H
        MOV  C,M
        MOV  M,B
        ADD  C
        MOV  D,A
        POP  B
        RZ
        PUSH D
        CALL GTCOL
        POP  D
        CPI  0
        JZ   UPD20
        MOV  A,D
        CPI  3
        RZ
        CPI  4
        RZ
        MVI  A,RED
        JMP  PTCOL
UPD20:  MOV  A,D
        CPI  3
        RNZ
        MVI  A,GREEN
        JMP  PTCOL
UPROW:  DCR  B
        MVI  D,0
        CALL UPONE
        INR  B
        CALL UPONE
        INR  B
        CALL UPONE
        MOV  A,D
        RET
UPONE:  PUSH D
        CALL GTCOL
        POP  D
        CPI  0
        RZ
        CPI  10
        RZ
        INR  D
        RET
GTCOL:  CALL FNDCOL
        MOV  A,M
        JC   GT20
        ANI  15
        RET
GT20:   ANI  240
        RLC
        RLC
        RLC
        RLC
        RET
PTCOL:  ANI  15
        DB   0F5H
        CALL FNDCOL
        POP  D
        JC   PTC20
        MOV  A,M
        ANI  240
        ADD  D
        MOV  M,A
        RET
PTC20:  MOV  A,D
        RLC
        RLC
        RLC
        RLC
        MOV  D,A
        MOV  A,M
        ANI  15
        ADD  D
        MOV  M,A
        RET
FNDCOL: LXI  H,DISPLY
        MOV  A,B
        ANI  32
        JZ   FND20
        LXI  D,512
        DAD  D
FND20:  MOV  A,C
        ANI  32
        JZ   FND30
        LXI  D,1024
        DAD  D
FND30:  MOV  A,C
        ANI  31
        RLC
        RLC
        RLC
        RAL
        MOV  E,A
        MVI  A,0
        RAL
        MOV  D,A
        DAD  D
        MOV  A,B
        RAR
        DB   0F5H
        ANI  15
        MOV  E,A
        MVI  D,0
        DAD  D
        DB   0F1H
        RET
CHECK:  IN   3
        ANI  DAV
        RNZ
        INR  C
        JNZ  CHECK
        INR  B
        JNZ  CHECK
        RET
TTYONE: IN   3
        ANI  TBE
        JZ   TTYONE
        MOV  A,B
        OUT  2
        RET
TTYOUT: MOV  B,M
        MVI  A,0
        CMP  B
        RZ
        CALL TTYONE
        INX  H
        JMP  TTYOUT
INIT:   CALL CDISP
        MVI  A,132
        OUT  14
        MVI  A,176
        OUT  15
        LXI  H,IDENT
        CALL TTYOUT
INT20:  CALL CHECK
;       JZ   INT20               ; INSERT JZ INT20 HERE FOR INST
        IN   2
        MOV  B,A
        CALL TTYONE
        ANI  127
        CPI  89
        RET                     ; INSERT RZ HERE FOR INST
        LXI  H,INST
        CALL TTYOUT
SETUP:  LXI  H,ENTER
        CALL TTYOUT
        CALL CDISP
        LXI  B,0
STP20:  CALL GTCHR
        LXI  H,TAB
        ANI  7FH
        MOV  D,A
STP30:  SUB  A
        CMP  M
        JZ   STP20
        MOV  A,D
        CMP  M
        JZ   STP40
        INX  H
        INX  H
        INX  H
        JMP  STP30
STP40:  INX  H
        MOV  E,M
        INX  H
        MOV  D,M
        XCHG
        CALL INDEX
        JMP  STP20
INDEX:  PCHL
TAB:    DB   1
        DW   ON
        DB   2
        DW   OFF
        DB   4
        DW   THRU
        DB   8
        DW   BACK
        DB   9
        DW   FWD
        DB   15
        DW   DOWN
        DB   11
        DW   HOME
        DB   13
        DW   RETURN
        DB   14
        DW   UP
        DB   0
        DB   0
        DB   0
        DB   0
THRU:   POP  B
        RET
HOME:   LXI  B,0
        RET
RETURN: MVI  B,0
DOWN:   INR  C
        RET
ON:     MVI  A,15
BOTH:   CALL PTCOL
FWD:    INR  B
        RET
OFF:    MVI  A,0
        JMP  BOTH
UP:     DCR  C
        RET
BACK:   DCR  B
        RET
GTCHR:  CALL GTCOL
        DB   0F5H
GTC20:  PUSH B
        MVI  A,12
        CALL PTCOL
        LXI  B,DELAY
        CALL CHECK
        JNZ  GTC40
        POP  B
        PUSH B
        MVI  A,0
        CALL PTCOL
        LXI  B,DELAY
        CALL CHECK
        POP  B
        JZ   GTC20
GTC30:  POP  6
        CALL PTCOL
        IN   2
        OUT  2
        RET
GTC40:  POP  B
        JMP  GTC30
ROWA:   DW   0
ROWB:   DW   0
IDENT:  DW   'IL'
        DW   'EF'
        DW   '..'
        DW   '..'
        DW   'EV'
        DW   'SR'
        DW   'OI'
        DW   ' N'
        DW   '.2'
        DW   ' 0'
        DW   0A0DH
        DW   0A0AH
        DW   0
INST:   DW   0
ENTER:  DW   'NE'
        DW   'ET'
        DW   ' R'
        DW   'AD'
        DW   'AT'
        DW   0A0DH
        DW   0

        END

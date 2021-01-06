;
; CROMEMCO DAZZLER KALEIDOSCOPE
; WRITTEN BY LI-CHEN WANG
;

        ORG  0

        LXI  SP,0100H
        MVI  A,081H
        OUT  14
        MVI  A,030H
        OUT  15
        MOV  A,B
        RRC
        RRC
        ANA  D
        ADD  C
        MOV  C,A
        RRC
        RRC
        ANA  D
        MOV  L,A
        MOV  A,B
        SUB  L
        MOV  B,A
        PUSH B
        PUSH D
        PUSH H
        LXI  D,0
        MOV  A,H
        ANI  01FH
        RAR
        JC   002BH
        MOV  E,A
        RLC
        RLC
        RLC
        RLC
        MOV  D,A
        MVI  H,08H
        CALL 005DH
        MOV  A,B
        CMA
        MOV  B,A
        MVI  H,06H
        CALL 005DH
        MOV  A,C
        CMA
        MOV  C,A
        MVI  H,02H
        CALL 005DH
        MOV  A,B
        CMA
        MOV  B,A
        MVI  H,04H
        CALL 005DH
        POP  H
        POP  D
        POP  B
        DCR  E
        JNZ  000BH
        INR  B
        INR  C
        MVI  E,03FH
        DCR  H
        JNZ  0000BH
        INR  D
        MVI  H,01FH
        JMP  000BH
        MOV  A,C
        ANI  0F8H
        RAL
        MOV  L,A
        MOV  A,H
        ACI  00H
        MOV  H,A
        MOV  A,B
        ANI  0F8H
        RAR
        RAR
        RAR
        RAR
        PUSH PSW
        ADD  L
        MOV  L,A
        POP  PSW
        MOV  A,M
        JC   007AH
        ANI  0F0H
        ADD  E
        MOV  M,A
        RET
        ANI  0FH
        ADD  D
        MOV  M,A
        RET

        END

;
;  KILL THE BIT GAME BY DEAN MCDANIEL, MAY 15, 1975
;
;  OBJECT: KILL THE ROTATING BIT. IF YOU MISS THE LIT BIT, ANOTHER
;          BIT TURNS ON LEAVING TWO BITS TO DESTROY. QUICKLY 
;          TOGGLE THE SWITCH, DON'T LEAVE THE SWITCH IN THE UP
;          POSITION. BEFORE STARTING, MAKE SURE ALL THE SWITCHES
;          ARE IN THE DOWN POSITION.
;
        ORG     0

        LXI     H,0             ;INITIALIZE COUNTER
        MVI     D,080H          ;SET UP INITIAL DISPLAY BIT
        LXI     B,0EH           ;HIGHER VALUE = FASTER
BEG:    LDAX    D               ;DISPLAY BIT PATTERN ON
        LDAX    D               ;...UPPER 8 ADDRESS LIGHTS
        LDAX    D
        LDAX    D
        DAD     B               ;INCREMENT DISPLAY COUNTER
        JNC     BEG
        IN      0FFH            ;INPUT DATA FROM SENSE SWITCHES
        XRA     D               ;EXCLUSIVE OR WITH A
        RRC                     ;ROTATE DISPLAY RIGHT ONE BIT
        MOV     D,A             ;MOVE DATA TO DISPLAY REG
        JMP     BEG             ;REPEAT SEQUENCE

        END

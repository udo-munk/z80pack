; HAND DISASSEMBLY OF THE DAZZLEMATION MEMORY IMAGE PROVIDED IN THE
; ORIGINAL 14 PAGE DAZZLEMATION MANUAL.
;
; THIS CODE WAS ORIGINALLY CONCEIVED AND COMPOSED BY:
;
; --------------------------------------------------------------------
; -----------------------STEVE DOMPIER--------------------------------
; --------------------------------------------------------------------
;
; THIS DISASSEMBLY, ANALYSIS AND DOCUMATION OF THE MEMORY IMAGE WAS
; PRODUCED BY BILL SUDBRINK IN APRIL 2016.
;
; WHEN ASSEMBLED WITH THE CP/M 2.2 ASSEMBLER PROGRAM, THIS FILE WILL
; PRODUCE THE MEMORY IMAGE PRINTED IN THE MANUAL.
;
; MINOR MODIFICATIONS TO ASSEMBLE WITH INTEL MACRO 80
; UDO MUNK MAY 2016

; PORT TO READ THE FRONT PANEL SWITCHES
SWITPORT        EQU     0FFH

; THESE VALUES ARE ORIGINAL AND WORK WITH THE CONSOLE PORT ON A CROMEMCO
; FDC SERIES FLOPPY CONTROLLER:
; PORT TO READ AND WRITE BYTES TO AND FROM THE CONSOLE SERIAL PORT
DATAPORT        EQU     01H
; PORT TO CHECK THE STATUS OF THE CONSOLE SERIAL PORT
STATPORT        EQU     00H
STATRXOK        EQU     40H
STATTXOK        EQU     80H

; THESE VALUES WORK WITH A MITS 2-SIO SERIAL CARD CONFIGURED FOR MITS
; PROM MONITOR
; DATAPORT      EQU     11H
; STATPORT      EQU     10H
; STATRXOK      EQU     01H
; STATTXOK      EQU     02H


; THE ORIGINAL CONSOLE DEVICE WAS APPARENTLY INTENDED TO BE A TELETYPE
; WITH AN EVEN PARITY KEYBOARD.  IN THAT CASE, CRTL-Z COMES IN AS:
CTRLZ   EQU     9AH
; IF YOU WANT THE ORIGINAL IMAGE, USE THE ABOVE.  IF YOU WANT CONTROL Z
; TO WORK, USE:
; CTRLZ EQU     1AH


        ORG     0

        JMP     START

; DATA

COORDS:                         ; CURSOR X,Y COORDINATES
        DB      1FH             ; INITIALIZED TO (0X1F, 0X1F) AND SET AGAIN IN START TO POSITION
        DB      1FH             ;   THE CURSOR AT THE MIDDLE OF THE SCREEN
CUSORDAT:                       ; TWO BYTE DATA INITIALIZED TO 0X00AA AND SET AGAIN IN START
        DB      00H             ; THIS BYTE APPARENTLY UNUSED
CURONSCR:                       ; ROTATED TO INDICATE CURSOR STATUS.
        DB      0AAH            ; WHEN LOW BIT IS SET, CURSOR IS ON SCREEN.  BINARY VALUE 10101010
COLOR:
        DB      05H             ; COLOR VALUE TO PAINT, COMBINED WITH INTENSITY
INTENS:
        DB      08H             ; INTENSITY CONTROL BYTE, COMBINED WITH COLOR
YCURMOT:
        DB      00H             ; Y CURSOR MOTION
XCURMOT:
        DB      00H             ; X CURSOR MOTION
CFLSHFLG:
        DB      00H             ; STOP CURSOR FLASH FLAG... NON-SERO IF CURSOR NOT FLASHING
CUROFFFLG:
        DB      00H             ; CURSOR OFF FLAG (NON-ZERO IF CURSOR IS OFF)
RUNFLG:
        DB      00H             ; ANIMATION RUN FLAG... ONE - RUN, ANYTHING ELSE - STOP
CMDBUFP:                        ; COMMAND BUFFER POINTER
        DB      00H             ; POINTER TO 0X1800.  COMMAND CHARACTERS STORED THERE
        DB      18H             ; INCREMENTED AS CHARACTERS ARE STORED.

; FUNCTIONS

; JUMPED TO BY DOT AND COMMA FUNCTIONS
XSETMOTION:
        MVI     L,00H           ; NO Y
        MOV     H,A             ; ONLY X
        JMP     STORMOTNMOV     ; JUMP TO STORE MOTION AND MOVE THE CURSOR

; JUMPED TO BY M AND N FUNCTIONS
YSETMOTION:
        MVI     H,00H           ; NO X
        MOV     L,A             ; ONLY Y

; STORE THE NEW MOTION AND MOVE THE CURSOR
STORMOTNMOV:
        SHLD    YCURMOT         ; STORE CURSOR MOTION
MOVNOSTORE:                     ; JUMP HERE FROM SEMI-COLON FUNCTION
        LHLD    COORDS          ; CURSOR COORDINATES IN HL
        MOV     B,H             ; MOVE TO BC
        MOV     C,L
        JMP     ADJCUR

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H

; POINT HL TO 0X0001 (CALLED VIA RST 5)
        LXI     H,0001H
        RET

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H

; SET RUN FLAG (CALLED VIA RST 6)
        MVI     A,01H
        STA     RUNFLG
        JMP     SWTDLY

; GO INTO TIGHT LOOP (CALLED VIA RST 7) SEEMS DEAD
TGHTLOOP:
        JMP     TGHTLOOP

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 00H

; TOGGLE CURSOR WHILE PRESERVE HL... SEEMS TO BE DEAD (NEVER CALLED)
        PUSH    H
        CALL    TGLCURSOR
        POP     H
        RET

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H

PROCESS:
        JMP     RUNCHECK
NOTRUNNING:
        CPI     00H             ; WE JUMP BACK HERE IF THE RUN FLAG (RUNFLG) IS NOT 1...
                                ; REGISTER A CONTAINS CURSOR OFF FLAG (CUROFFFLG)
        JNZ     SKIPCUR         ; SKIP CURSOR CALL IF CURSOR OFF FLAG IS NON-ZERO
        CALL    TGLCURSOR
SKIPCUR:
        LXI     D,0C800H        ; DE USED AS LOOP COUNT.  LOOP UNTIL WRAP TO ZERO, 37FF TIMES
NOFLSHLOOP:
        IN      STATPORT        ; CHECK DATA PORT STATUS
        ANI     STATRXOK        ; IS A BYTE AVAILABLE?
        JNZ     GETKEY          ; JUMP IF BYTE AVAILABLE
        LDA     CFLSHFLG        ; TEST CURSOR FLASH FLAG
        CPI     00H             ; FOR ZERO
        JNZ     NOFLSHLOOP      ; TIGHT LOOP IF NOT FLASHING (NOFLSHLOOP)
        INX     D               ; INCREMENT COUNTER
        MOV     A,D             ; SEE IF WE WRAPPED
        CPI     00H
        JNZ     NOFLSHLOOP      ; LOOP BACK TO TEST FOR BYTE AVAILABLE
        JMP     PROCESS         ; LOOP BACK TO START WHICH FLASHES CURSOR

; SEEMS DEAD
        DB      00H

; TOGGLE CURSOR ROUTINE... PUT A 'T' AROUND THE CURSOR POSITION...
; USES XOR SO EVERY OTHER CALL "TURNS OFF" THE CURSOR
TGLCURSOR:
        LHLD    COORDS          ; CURSOR X,Y INTO HL
        MOV     A,L             ; Y IN A
        SUI     02H             ; DECREMENT BY 2
        NOP
        NOP
        NOP
        MOV     C,A             ; MOVE TO C 
        MOV     B,H             ; X TO B ... BC CURSOR X,Y WITH Y ADJUSTED BY TWO
        CALL    XORPIXVAL
        MOV     A,C
        ADI     04H             ; ADJUST Y AGAIN - LINE 383
        NOP
        NOP
        NOP
        MOV     C,A
        CALL    XORPIXVAL
        LHLD    COORDS          ; GET ORIGINAL CURSOR AGAIN
        MOV     A,H
        SUI     02H             ; ADJUST X
        NOP
        NOP
        NOP
        MOV     B,A
        MOV     C,L             ; RELOAD Y
        CALL    XORPIXVAL
        MOV     A,B
        ADI     04H             ; ADJUST X AGAIN
        NOP
        NOP
        NOP
        MOV     B,A
        CALL    XORPIXVAL
        LDA     CURONSCR        ; ROTATE CURSOR STATUS BYTE
        RRC
        STA     CURONSCR
        RET

; XOR A PIXEL WITH 0X0F
XORPIXVAL:
        CALL    GETPVAL
        XRI     0FH             ; XOR IT WITH 0X0F TO MAKE IT FLASH
        CALL    SETPIXVAL
        RET

; TEST RUN FLAG AND RETURN CURSOR FLAG IN A IF NOT RUNNING
RUNCHECK:
        LDA     RUNFLG          ; GET THE RUN FLAG
        CPI     01H             ; TEST AGAINST ONE
        JZ      RUNNING         ; IF ONE, WE'RE RUNNING
        LDA     CUROFFFLG       ; GET THE CURSOR FLAG
        JMP     NOTRUNNING

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H

; GET PIXEL VALUE AT X,Y INTO A, COORDINATES COME IN IN BC
GETPVAL:
        CALL    GETPADDR        ; CONVERT PIXEL COORDINATE IN BC TO HL POINTER INTO DAZZLER BUFFER
                                ; CARRY BIT INDICATES WHICH FOUR BITS OF BYTE TO USE
        MOV     A,M             ; GET BOTH PIXELS
        JC      GETHIGH         ; IF CARRY, GET THE HIGH BITS
        ANI     0FH             ; MASK OFF HIGH BITS
        RET
GETHIGH:
        ANI     0F0H            ; MASK OFF LOW BITS
        RLC                     ; ROTATE THE HIGH BITS LOW
        RLC
        RLC
        RLC
        RET

; SEVEN BYTES OF 0X01 SEEMS DEAD
        DB      01H, 01H, 01H, 01H, 01H, 01H, 01H

; CONVERT X,Y COORDINATES IN BC TO HL POINTER INTO DAZZLER BUFFER AT 0X1000
; CARRY BIT INDICATES WHICH HALF-BYTE TO USE
; DOES NOT CHANGE B OR C
GETPADDR:
        LXI     H,1000H         ; HL POINT TO START OF BUFFER
        MOV     A,B             ; GET X
        RAL
        RAL
        RAL
        JNC     SKP200          ; SEE IF X IS 20 OR GREATER
        LXI     D,0200H         ; IF IT IS...
        DAD     D               ; INCREMENT HL BY 0X0200
SKP200:
        MOV     A,C             ; GET Y
        RAL
        RAL
        RAL
        JNC     SKP400          ; SEE IF Y IS 20 OR GREATER
        LXI     D,0400H         ; IF IT IS...
        DAD     D               ; INCREMENT HL BY 0X0400
SKP400:
        MOV     A,C             ; GET Y AGAIN
        ANI     1FH             ; CLEAR OUT HIGH THREE BITS
        RLC
        RLC
        RLC                     ; LOW THREE BITS ARE NOW ZERO
        RAL                     ; ALL FOUR LOW BITS ARE NOW ZERO
        MOV     E,A             ; STORE Y IN E
        MVI     A,00H           ; PRESERVED CARRY BIT SO THAT ...
        RAL                     ; A IS NOW ONE
        MOV     D,A             ; DE IS NOW 0X01?0 WITH ? BEING THE FOUR BITS PRESERVED FROM C ABOVE
        DAD     D               ; ADD IT TO HL - CARRY SHOULD ALWAYS BE CLEAR
        MOV     A,B             ; GET X AGAIN
        RAR                     ; DIVIDE BY TWO WITH EVEN/ODD BIT IN CARRY (HIGH OR LOW HALF-BYTE)
        PUSH    PSW
        NOP
        ANI     0FH             ; LOW FOUR BITS OF X/2
        MOV     E,A
        MVI     D,00H
        DAD     D               ; I'M SURE THIS MUST WORK, BUT I NEED TO SIT DOWN WITH PENCIL AND PAPER...
        POP     PSW
        RET

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 00H, 00H

; SETPIXVAL ... PIXEL VALUE IN A AND COORDINATES IN BC
SETPIXVAL:
        ANI     0FH             ; EVERYBODY DOES THIS... BUT JUST TO BE SURE, I GUESS
        PUSH    PSW             ; PUSHES A AND FLAGS
        CALL    GETPADDR
        POP     D               ; POPS D AND E (PRESUMABLY A IS NOW IN D)
        JC      SPTOP4          ; IF CARRY, SET THE TOP FOUR BITS
; PIXEL VALUE IN LOW FOUR BITS...
        MOV     A,M             ; GET CURRENT VALUE (TWO PIXELS)
        ANI     0F0H            ; CLEAR OLD VALUE
        ADD     D               ; ADD NEW VALUE
        MOV     M,A             ; SET NEW VALUE
        RET
; PIXEL VALUE IN TOP FOUR BITS...
SPTOP4:
        MOV     A,D             ; SHIFT NEW VALUE WHERE WE NEED IT
        RLC
        RLC
        RLC
        RLC
        MOV     D,A
        MOV     A,M             ; GET CURRENT VALUE (TWO PIXELS)
        ANI     0FH             ; CLEAR OLD VALUE - LINE 575 OK
        ADD     D               ; ADD NEW VALUE
        MOV     M,A             ; SET NEW VALUE
        RET

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H

; L FUNCTION CONTINUED
LOWINT:
        MVI     A,00H           ; CLEAR INTENSITY
        STA     INTENS
        JMP     DOCURRENT

; H FUNCTION CONTINUED
HIGHINT:
        MVI     A,08H           ; SET INTENSITY
        STA     INTENS
        JMP     DOCURRENT

; SEEMS DEAD
        LXI     H,1000H
        XRA     A
        MOV     M,A
        INX     H
        MOV     A,H
        CPI     18H
        DB      0C2H

; COLOR KEY DECODE TABLE, REFERENCED BELOW
CLRKEYTBL:
        DB      7FH             ; <DEL>, <RUBOUT> - BLACK
        DB      52H             ; R               - RED
        DB      47H             ; G               - GREEN
        DB      59H             ; Y               - YELLOW (RED AND GREEN)
        DB      42H             ; B               - BLUE
        DB      50H             ; P               - PURPLE (RED AND BLUE)
        DB      43H             ; C               - CYAN (GREEN AND BLUE)
        DB      57H             ; W               - WHITE (RED, GREEN AND BLUE)
        DB      0FFH            ; END OF TABLE

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 00H

; CALLED IF RX DATA IS AVAILABLE ON THE CONSOLE PORT
GETKEY:
        LDA     CURONSCR
        RRC
        CC      TGLCURSOR       ; IF THE CURSOR IS ON SCREEN, CLEAR IT
        CALL    GETPORTBYTE     ; GET THE KEYSTROKE AND STORE IT IF NOT "SPECIAL"

; THE RECORDING RUNNER JUMPS IN HERE WITH A RECORDED CHARACTER
PROCCHAR:
        MOV     A,B             ; INCOMING KEYSTROKE IN B, MOVE IT TO A
        CPI     20H             ; CHECK FOR SPACE CHARACTER
        JZ      DOCURRENT       ; IF SPACE KEY, REPEAT LAST ACTION
        LXI     H,CLRKEYTBL     ; SEE IF THE INCOMING CHARACTER IS A COLOR KEY
        NOP
        LXI     D,0FFFFH
PCLOOP1:
        INR     E               ; FIRST TIME, E TO 0X00
        MOV     A,B             ; GET KEYSTROKE INTO A
        NOP
        NOP
        NOP
        CMP     M               ; CHECK IF IT IS IN THE COLOR TABLE
        JZ      KEYCOLOR        ; IF IT IS IN THE TABLE, DECODE THE COLOR
        MOV     A,M
        CPI     0FFH            ; CHECK FOR END OF TABLE
        JZ      FUNCCHAR        ; IF END OF TABLE, DID NOT FIND THE KEY, CHECK FOR FUNCTION KEY
        INX     H               ; NOT END OF TABLE, CHECK NEXT
        JMP     PCLOOP1

; KEYSTROKE INTERPRETED AS COLOR
KEYCOLOR:
        MOV     A,E             ; GET TABLE INDEX IN A
        NOP                     ; NO OPS FOR TIMING MAYBE?
        NOP
        NOP
        NOP
        NOP
        STA     COLOR           ; STORE TABLE INDEX IN COLOR
        NOP                     ; MORE NOP TIMING?
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
; DROP INTO DOCURRENT, BELOW

; CALLED AFTER <H KEY>, <L KEY> AND </ KEY> PROCESSING
; OR IF KEYSTROKE DETECTED AVAILABLE ON PORT
; SET THE CURRENT COLOR AT THE CURRENT CURSOR COORDINATES AND FALL
; THROUGH TO MOVE THE CURSOR COORDINATES USING THE CURRENT MOTION
; XY (LOTSA CURRENT)
DOCURRENT:
        LHLD    COORDS          ; CURSOR X,Y IN HL
        MOV     B,H             ; CURSOR X,Y IN BC
        MOV     C,L
        NOP
        LHLD    COLOR           ; COLOR AND INTENSITY IN HL
        MOV     A,H             ; COMBINE
        ADD     L
        NOP                     ; MORE TIMING?
        NOP
        NOP
        NOP
        CALL    SETPIXVAL
; DROP INTO ADJCUR, BELOW

; MOTION FUNCTIONS (N, M, DOT AND COMMA) JUMP HERE
; ADJUST THE CURSOR POSITION CURRENTLY IN BC WITH THE CURRENT MOTION VALUES AND THEN STORE IT
ADJCUR:
        LHLD    YCURMOT         ; LOAD THE CURRENT X AND Y MOTION VALUES
        MOV     A,H             ; ADJUST CURSOR COORDINATES
        ADD     B
        MOV     H,A
        MOV     A,L
        ADD     C
        MOV     L,A
        SHLD    COORDS          ; STORE NEW CURSOR POSITION
        CALL    CHKCURPOS       ; CHECK FOR COORDINATES OUT OF BOUNDS, WRAP THEM
        JMP     PROCESS         ; GO BACK FOR ANOTHER RX BYTE OR COMMAND BUFFER BYTE

; JUMPED TO BY RST 6 (SET RUN FLAG)
; DELAYS VIA SENSE SWITCHES
SWTDLY:
        LXI     D,0000H
        IN      SWITPORT        ; GET SENSE SWITCHES
        MOV     D,A             ; THE MORE SWITCHES, THE SHORTER THE DELAY (OR DO SENSE SWITCHES GIVE ZERO BITS WHEN ON?)
DLYLOOP1:
        INX     D
        MOV     A,D
        CPI     0FFH            ; TEST FOR END OF DELAY
        RZ                      ; RETURN IF END OF DELAY (RETURNING FROM RST 6)
        JMP     DLYLOOP1

; LOOKS LIKE ANOTHER KEYSTROKE DECODE TABLE, BUT SEEMS DEAD
        DB      7FH
        DB      52H
        DB      47H
        DB      59H
        DB      42H
        DB      50H
        DB      43H
        DB      57H
        DB      2EH
        DB      2CH
        DB      4DH
        DB      4EH
        DB      2FH
        DB      1BH

; INITIALIZE THINGS AND THEN JUMP TO KEYSTROKE PROCESSING
START:
        LXI     SP,0FFFH        ; STACK DOWN FROM 0X0FFF
        MVI     A,88H           ; DAZZLER SETUP, ON, START ADDRESS 0X1000
        OUT     0EH
        MVI     A,30H           ; DAZZLER SETUP, FORMAT 2K COLOR (64X64)
        OUT     0FH
        LXI     H,1F1FH         ; SET CURSOR TO MIDDLE OF THE SCREEN (0X1F,0X1F)
        SHLD    COORDS
        LXI     H,0AA00H        ; SET BLINK CONTROL
        SHLD    CUSORDAT
        LXI     H,1800H         ; INITIALIZE COMMAND BUFFER POINTER TO 0X1800
        SHLD    CMDBUFP
        LXI     H,YCURMOT       ; ZERO BYTES FROM YCURMOT TO CUROFFFLG
STCLRLOOP:
        XRA     A               ; ZERO A
        MOV     M,A             ; PUT A ZERO AT HL
        INX     H               ; INCREMENT HL
        MOV     A,L             ; TEST L FOR...
        CPI     0DH             ; 0X0D (RUNFLG) ... DOESN'T ACTUALLY CLEAR RUNFLG, WHY NOT?
        JNZ     STCLRLOOP
        CALL    CLEARBUF
        JMP     PROCESS         ; START LOOKING FOR KEYSTROKES
        NOP

; CLEAR THE BUFFER STARTING AT 0X1000... MAY CLEAR ALL THE WAY UP
; TO 0X1FFF OR MAY BE HACKED TO STOP AT 0X17FF
CLEARBUF:
        LXI     H,1000H
CLRBLOOP:
        XRA     A               ; ZERO OUT A
        MOV     M,A             ; MOVE ZERO TO [HL]
        INX     H               ; INCREMENT HL
        MOV     A,H             ; COMPARE HIGH ADDRESS...
CLRBCMP:                        ; NOTE: THIS LABEL IS USED TO HACK END POINT BY THE CODE BELOW
        CPI     20H             ; COMPARE HIGH ADDRESS TO 0X20 (OR MAY BE HACKED TO 0X18
        JNZ     CLRBLOOP
        RET

; SEEMS DEAD
        DB      00H, 00H, 00H

; INTERPRET CHARACTER IN B, NOT COLOR KEY, LOOK FOR FUNCTION CHARACTERS
FUNCCHAR:
        LXI     H,JUMPMTCHTBL   ; POINT TO CHARACTER MATCH TABLE
        LXI     D,1580H         ; DE SET TO 0X1580 D IS THE TABLE LENGTH
                                ; E IS ADDRESS TABLE LOW ADDRESS BYTE
CHKNXT:
        MOV     A,B             ; GET CHARACTER
        CMP     M               ; COMPARE TO TABLE
        JZ      TBLMATCH        ; GOT A MATCH? YES? JUMP TO TBLMATCH
        INX     H               ; NO? TRY THE NEXT ONE
        DCR     D               ; KEEP TRACK OF THE COUNT
        JZ      PROCESS         ; OUT OF TABLE ENTRIES? JUMP BACK TO GET ANOTHER KEY
        INR     E               ; INCREMENT ADDRESS TABLE LOW ADDRESS BYTE
        JMP     CHKNXT          ; CHECK NEXT ENTRY

TBLMATCH:
        MOV     L,E             ; GET LOW BYTE OF FUNCTION ADDRESS
        MOV     L,M
        INR     H               ; FUNCTION ADDRESS HIGH BYTE IS ALWAYS 0X03
        PCHL                    ; JUMP TO FUNCTION

; SEEMS DEAD
        NOP
        LHLD    COORDS
        MOV     B,H
        MOV     C,L
        JMP     ADJCUR

; SEEMS DEAD
        JMP     GETPADDR
        NOP
        NOP
        NOP
        NOP

; START OF JUMP TABLE COMPARE TARGETS. 0X15 IN LENGTH (21 DECIMAL)
JUMPMTCHTBL:
        DB      2EH             ; DOT, PERIOD
        DB      2CH             ; COMMA
        DB      4DH             ; M
        DB      4EH             ; N
        DB      2FH             ; /
        DB      11H             ; <DC1><CTRL-Q>
        DB      3BH             ; <SEMI-COLON>
        DB      3EH             ; <GREATER THAN>
        DB      3CH             ; <LESS THAN>
        DB      5DH             ; <RIGHT SQUARE BRACKET>
        DB      5EH             ; <CARET>
        DB      4CH             ; L
        DB      48H             ; H
        DB      03H             ; <ETX><CTRL-C>
        DB      02H             ; <STX><CTRL-B>
        DB      10H             ; <DLE><CTRL-P>
        DB      40H             ; <AT SIGN>
        DB      1BH             ; <ESCAPE>
        DB      12H             ; <DC2><CTRL-R>
        DB      13H             ; <DC3><CTRL-S>
        DB      00H             ; <NUL> ?? TEST AGAINST NULL ??
; LEFT OVER/UNUSED
        DB      00H, 00H, 00H

; LOW BYTES FOR TARGET FUNCTION ADDRESSES
FUNCLADDR: ; LABEL IS NEVER DIRECTLY ADDRESSED, ACCESSED BY MATH ABOVE
        DB      00H             ; DOT FUNCTION AT 0X0300
        DB      05H             ; COMMA FUNCTION AT 0X0305
        DB      0AH             ; M FUNCTION AT 0X030A
        DB      0FH             ; N FUNCTION AT 0X030F
        DB      14H             ; / FUNCTION AT 0X0314
        DB      1DH             ; <DC1><CTRL-Q> FUNCTION AT 0X031D
        DB      0EDH            ; <SEMI-COLON> FUNCTION AT 0X03ED
        DB      23H             ; <GREATER THAN> FUNCTION AT 0X0323
        DB      2BH             ; <LESS THAN> FUNCTION AT 0X032B
        DB      33H             ; <RIGHT SQUARE BRACKET> FUNCTION AT 0X0333
        DB      3BH             ; <CARET> FUNCTION AT 0X033B
        DB      43H             ; L FUNCTION AT 0X0343
        DB      46H             ; H FUNCTION AT 0X0346
        DB      50H             ; <ETX><CTRL-C> FUNCTION AT 0X0350
        DB      5AH             ; <STX><CTRL-B> FUNCTION AT 0X035A
        DB      0C0H            ; <DLE><CTRL-P> FUNCTION AT 0X03C0
        DB      8BH             ; <AT SIGN> FUNCTION AT 0X038B
        DB      98H             ; <ESCAPE> FUNCTION AT 0X0398
RUNTBLENT:                      ; THIS BYTE IS MODIFIED BY CODE BELOW
        DB      0AFH            ; <DC2><CTRL-R> FUNCTION AT 0X03AF
        DB      0A8H            ; <DC3><CTRL-S> FUNCTION AT 0X03A8
        DB      0A5H            ; <NUL> FUNCTION AT 0X03A5
; LEFT OVER/UNUSED... SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 07H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 70H, 01H, 50H, 00H, 50H
        DB      00H, 30H, 01H, 08H, 00H, 90H, 00H, 00H
        DB      09H, 00H, 00H, 30H, 05H, 00H, 05H, 00H
        DB      05H, 30H, 01H, 00H, 00H, 60H

; CHECK THE CURSOR POSITION FOR WRAPPING
; CALLED FROM ADJCUR
CHKCURPOS:
        LHLD    COORDS          ; LOAD CURSOR X,Y INTO HL
RECHECK:                        ; JUMPS HERE TO RECHECK AFTER A TOP/BOTTOM WRAP
        MOV     A,H             ; GET X
        ANI     0C0H            ; SEE IF X WRAPPED (RANGE IS 0 TO 63... 0X00 TO 0X3F)
        JZ      CHKY            ; NO WRAP? THEN JUMP TO CHECK Y
        RAL                     ; SEE IF WE WRAPPED LEFT OR RIGHT
        JNC     WRPRIGHT        ; DID WE WRAP LEFT? NO? THEN JUMP TO WRAP RIGHT HANDLING
        MVI     H,3FH           ; WE WRAPPED OFF OF THE LEFT SO GO TO RIGHT (63 OR 0X3F)
        DCR     L               ; DECREASE Y (GO UP A LINE)
        JMP     CHKY
WRPRIGHT:                       ; WE SRAPPED OFF OF THE RIGHT SIDE SO...
        MVI     H,00H           ; MOVE TO LEFT SIDE, X=0
        INR     L               ; DOWN A LINE, INCREASE Y
        NOP
        NOP
        NOP
CHKY:                           ; X IS CORRECT WHEN WE GET HERE, CHECK Y NOW
        MOV     A,L             ; GET Y
        ANI     0C0H            ; SEE IF Y WRAPPED (RANGE IS 0 TO 63... 0X00 TO 0X3F)
        JZ      CHKDONE         ; NO WRAP? STORE IT AND DONE
        RAL                     ; SEE IF WE WRAPPED TOP OR BOTTOM
        JNC     WRPBTM          ; DID WE WRAP TOP? NO? THEN JUMP TO BOTTOM WRAP HANDLING
        MVI     L,3FH           ; WE WRAPPED OFF OF THE TOP, SO GO TO THE BOTTOM
        DCR     H               ; AND MOVE ONE LEFT
        JMP     RECHECK
WRPBTM:
        MVI     L,00H           ; WE WRAPPED OFF OF THE BOTTOM, SO GO TO THE TOP
        INR     H               ; AND MOVE ONE RIGHT
        JMP     RECHECK
CHKDONE:
        SHLD    COORDS          ; COORDINATES OK, STORE THEM AND...
        RET                     ; RETURN

; SEEMS DEAD
        ADI     02H
        MVI     L,3FH
        DCR     H
        JMP     RECHECK
        NOP
        NOP

; FUNCTION TABLE TARGETS
; NOTE THAT THE ADDRESSES THAT THESE FUNCTIONS END UP AT MUST MATCH THE
; JUMP TABLE ABOVE.  GOTTA FIGURE OUT HOW TO FILL THAT TABLE FROM LABELS...

; DOT FUNCTION
        MVI     A,01H
        JMP     XSETMOTION

; COMMA FUNCTION
        MVI     A,0FFH
        JMP     XSETMOTION

; M FUNCTION
        MVI     A,01H
        JMP     YSETMOTION

; N FUNCTION
        MVI     A,0FFH
        JMP     YSETMOTION

; / FUNCTION (CLEARS CURSOR MOTION)
        LXI     H,0000H
        SHLD    YCURMOT         ; ZERO OUT X AND Y MOTION
        JMP     DOCURRENT

; <DC1><CTRL-Q> FUNCTION
        JMP     0000H           ; RESTART THE WHOLE THING

; SEEMS DEAD
        DB      00H, 00H, 00H

; THE "STORE" FUNCTIONS, WHEN COMBINED WITH ANOTHER MOTION
; KEY, RESULT IN DIAGONAL MOTION.

; <GREATER THAN> FUNCTION (STORE RIGHT)
        MVI     A,01H
        STA     XCURMOT
        JMP     PROCESS

; <LESS THAN> FUNCTION (STORE LEFT)
        MVI     A,0FFH
        STA     XCURMOT
        JMP     PROCESS

; <RIGHT SQUARE BRACKET> (<SHIFT-M ON TTY) FUNCTION (STORE DOWN)
        MVI     A,01H
        STA     YCURMOT
        JMP     PROCESS

; <CARET> (<SHIFT-N> ON TTY) FUNCTION (STORE UP)
        MVI     A,0FFH
        STA     YCURMOT
        JMP     PROCESS

; L FUNCTION (LOW INTENSITY)
        JMP     LOWINT

; H FUNCTION (HIGH INTENSITY)
        JMP     HIGHINT

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H

; <ETX><CTRL-C> FUNCTION (TOGGLE HIDE CURSOR)
        LDA     CUROFFFLG
        CMA
        STA     CUROFFFLG
        JMP     PROCESS

; <STX><CTRL-B> FUNCTION (TOGGLE CURSOR FLASHING)
        LDA     CFLSHFLG
        CMA
        STA     CFLSHFLG
        JMP     PROCESS

; OUTPUT 0X20 CHARACTERS STORED AT 0X0001 (ZERO OR NUL BYTES)
; USED BY <AT SIGN><SHIFT-P> (PUNCH ANIMATION)
NULOUT:
        MVI     D,20H
        RST     5               ; SETS HL TO 0X0001
NOLOOP:
        CALL    CHAROUT
        DCR     D
        JNZ     NOLOOP
        NOP
        RET

; CONTINUATION OF <AT SIGN>
BUFOUT:
        LXI     H,1800H         ; OUTPUT CHARACTERS STORED AT 0X1800 UNTIL...
BOLOOP1:
        NOP
        CALL    CHAROUT
        CPI     13H
        JZ      OUTDONE         ; YOU HIT 0X13, THE STOP CHARACTER <CTRL-S>
        INX     H
        JMP     BOLOOP1

; OUTPUT ONE CHARACTER POINTED TO BY HL WHEN THE PORT IS READY
; CHARACTER IS LEFT IN A, HL NOT MODIFIED
; USED BY <AT SIGN>
CHAROUT:
        IN      STATPORT        ; PORT STATUS
        ANI     STATTXOK        ; READY BIT
        JZ      CHAROUT         ; LOOP UNTIL PORT IS READY
        MOV     A,M             ; GET THE CHARACTER
        OUT     01H
        RET

; <AT SIGN> (OR <SHIFT-P> ON A TELETYPE) FUNCTION ENTRY POINT (PUNCH ANIMATION)
        CALL    NULOUT
        JMP     BUFOUT

; FINISH UP OF <AT SIGN>
OUTDONE:
        CALL    NULOUT
        JMP     PROCESS
        NOP

; <ESCAPE> FUNCTION (CLEAR THE SCREEN ONLY)
; NOTE: THIS HACKS THE END TEST IN THE CLEARBUF FUNCTION TO MAKE IT STOP BEFORE THE COMMAND BUFFER
        MVI     A,18H           ; STOP AT HIGH ADDRESS 0X18 INSTEAD OF 0X20
        STA     CLRBCMP+1
        CALL    CLEARBUF
        MVI     A,20H           ; RESTORE TO 0X20
        STA     CLRBCMP+1
; <ESCAPE> DROPS INTO <NUL> (OR <NUL> JUMPS INTO <ESCAPE>)
; <NUL> FUNCTION
        JMP     PROCESS

; <DC3><CTRL-S> FUNCTION (STOP ANIMATION)
        XRA     A               ; CLEAR RUN FLAG
        STA     RUNFLG
        JMP     JTBLUNHACK

; <DC2><CTRL-R> FUNCTION (START ANIMATION)
        LXI     H,17FFH         ; ONE BEFORE THE COMMAND BUFFER START
        SHLD    CMDBUFP

; IF RUN FLAG (0X000D) IS ONE, WE COME HERE
RUNNING:
        RST     6               ; CALL SET RUN FLAG AND DELAY VIA SENSE SWITCHES
        NOP
        NOP
        JMP     MORERUN

RUN2:
        INX     H               ; INCREMENT COMMAND POINTER IN HL
        MOV     B,M             ; PUT RECORDED CHARACTER IN B
        JMP     INSRT1

; <DLE><CTRL-P> FUNCTION (OUTPUT EVERYTHING STARTING AT ZERO  --LOOK OUT-- GET READY TO RESET)
        LXI     H,0000H
CPWAIT:
        IN      STATPORT        ; PORT STATUS
        ANI     STATTXOK        ; READY BIT
        JZ      CPWAIT
        MOV     A,M
        OUT     01H
        INX     H
        JMP     CPWAIT

; GET A CHARACTER
GETPORTBYTE:
        LXI     H,GPBTABLE      ; DATA AREA BELOW...
        IN      DATAPORT        ; GET THE CHARACTER
        ANI     7FH             ; CLEAR THE HIGH BIT
        MOV     B,A             ; MOVE IT TO B
GPBLOOP:
        INX     H               ; INCREMENT THE DATA POINTER
        MOV     A,B
        CMP     M
        RZ                      ; IF IT IS A CHARACTER IN OUR TABLE, RETURN RIGHT HERE DON'T STORE IT
        MOV     A,M             ; GET THE TABLE VALUE SO WE CAN... - LINE 1229 OK
        CPI     0FFH            ; CHECK FOR THE END OF THE TABLE
        JNZ     GPBLOOP         ; NOT THE END? LOOP BACK AND CHECK THE NEXT VALUE
        MOV     A,B             ; IF WE'RE HERE, IT'S NOT IN THE TABLE...
        LHLD    CMDBUFP         ; STORE IT TO THE COMMAND BUFFER POINTER
        MOV     M,A
        INX     H               ; INCREMENT THE POINTER
        SHLD    CMDBUFP         ; STORE THE INCREMENTED POINTER (CMDBUFP)
        RET

; <SEMI-COLON> FUNCTION (JUST MOVE IN THE SAME DIRECTION, NO PIXEL CHANGE)
        JMP     MOVNOSTORE

; COMPARE TABLE (DON'T STORE THESE BYTES)
GPBTABLE:
        DB      03H
        DB      02H
        DB      10H
        DB      40H
        DB      11H
        DB      00H
        DB      0FFH            ; END OF TABLE MARKER (SEE ABOVE)

; SEEMS DEAD
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H

INSRT1:
        SHLD    CMDBUFP         ; STORE HL TO CMDBUFP
        JMP     PROCCHAR

MORERUN:
        LHLD    CMDBUFP         ; COMMAND BUFFER POINTER IN HL (CMDBUFP)
        IN      DATAPORT        ; JUST GET A CHARACTER, REGARDLESS OF PORT STATUS
        CPI     11H             ; SEE IF WE GOT A <DC1><CTRL-Q>
        JNZ     CHKCZ           ; NO? CHECK FOR NEXT SPECIAL CHARACTER
        MVI     A,00H           ; CLEAR THE RUN FLAG
        STA     RUNFLG
        JMP     0000H           ; RESET EVERYTHING AND START OVER
CHKCZ:
        CPI     CTRLZ           ; SEE IF WE GOT A 0X1A <CTRL-Z>?
                                ; WRITTEN ASSUMING AN EVEN PARITY TTY SO HIGH BIT IS SET
        JNZ     CHK18           ; NO, JUMP TO CHECK18
        MVI     A,00H           ; CLEAR THE RUN FLAG
        STA     RUNFLG
        JMP     PROCESS         ; JUMP BACK TO KEYSTROKE LOOP
CHK18:
        CPI     18H             ; SEE IF WE GOT A <CAN> (CANCEL) <CTRL-X>
        JNZ     RUN2            ; NO? KEEP GOING
; MAKE <CTRL-R> ACTION FUNCTION SAME AS <CTRL-S>
JTBLHACK:
        MVI     A,0A8H          ; CHANGE RUN FUNCTION TO STOP FUNCTION
        STA     RUNTBLENT       ; MODIFY JUMP TABLE FOR <CTRL-R>
        JMP     RUN2

; RESTORE <CTRL-R> ACTION FUNCTION
JTBLUNHACK:
        MVI     A,0AFH          ; CHANGE RUN FUNCTION BACK TO RUN FUNCTION
        STA     RUNTBLENT       ; MODIFY JUMP TABLE FOR <CTRL-R>
        JMP     PROCESS         ; GO BACK TO KEYSTROKE LOOP

; SEEMS DEAD
        DB      07H, 07H, 07H, 07H, 07H, 07H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H, 00H, 00H, 00H
        DB      00H, 00H, 00H, 00H, 00H
        DB      0DH, 0DH, 0DH, 0DH, 0DH, 0DH, 0DH, 0DH
        DB      0DH, 0DH, 0DH, 0DH, 0DH, 0DH, 0DH, 0DH

        END

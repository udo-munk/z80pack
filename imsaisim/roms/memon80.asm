;==============================================================
; Simple, configurable 1K-byte or 2K-byte ROM-based monitor for
; an 8080- or Z80-based system, supporting a variety of serial
; ports and floppy disk controllers.
;
; See comments further down for a list and description of
; Memon/80's commands.
;
; Memon/80 uploads and downloads data either the console or a
; second serial port (called the Transfer Port).
;
; Memon/80 supports the following serial ports for either the
; console or the Transfer Port, as selected with assembly
; options below.
;    Altair 88-SIO
;    Altair 88-2SIO or my own 88-2SIOJP
;    Altair 8800b Turnkey Module's serial port
;    Altair (MITS) 88-UIO's serial port
;    California Comouter Systems 2718's serial ports
;    California Computer Systems 2719
;    California Computer Systems 2810 CPU'S serial port
;    Compupro Interfacer 1
;    Compupro Interfacer II
;    Cromemco 4FDC/16FDC/64FDC disk controller's serial port
;    Cromemco TU-ART
;    Electronic Control Technology (ECT) R2IO
;    Heathkit H8-4 quad serial port
;    Heathkit H8-5 Console/Cassette controller's console port
;      (assuming a PAM-8 or XCON-8 ROM is installed in the H8)
;    IMSAI SIO-2
;    Ithaca Intersystems Series II VIO's serial ports
;    Jade Serial/Parallel I/O Board's serial ports
;    Micromation Doubler disk controller's serial port
;    Processor Technology 3P+S's serial port
;    Salota I/O 2+1's serial ports
;    Solid State Music IO4's serial ports (Configured as one of
;      the other supported serial port boards)
;    Solid State Music IO5's serial ports
;    Tarbell 4044 4S2P's serial ports
;    Vector Graphic BitStreamer
;    Vector Graphic BitStreamer II
;    Wameco (WMC) IO-1B's serial port
;
; Memon/80's BO command boots from the following floppy
; disk controllers, selected with assembly options below. 
;    Altair 88-DCDD 8"
;    Altair 88-MDS Minidisk
;    California Computer Systems 2422
;    Cromemco 4FDC/16FDC/64FDC
;    Heathkit H17 Minifloppy
;    IMSAI FIF (FIB & IFM)
;    IMSAI MDC-DIO
;    Micromation Doubler
;    Micropolis - FD Control B
;    Northstar MDC-A Minidisk
;    Northstar MDS-A  Minidisk
;    Northstar MDS-D Double-density  Minidisk
;    Salota FDC
;    SD Systems Versafloppy (Single-density)
;    SD Systems Versafloppy II (Double-density)
;    Tarbell 1011 8" (Single-density)
;    Tarbell 2022 8" (Double-density)
;
; Formatted to assemble with digital Research'S ASM.
;==============================================================
FALSE	equ	0
TRUE	equ	NOT FALSE

;==========================================================
;=               Memon/80 Option Selection                =
;=                                                        =
;= Set the required options in the following 10 sections  =
;=                                                        =
;= Select Memon/80 functionality by setting these         =
;= variables according to the guidelines in each section. =
;==========================================================

;============================---------------------------
;= 1. Specify the CPU Speed =
;============================
;(Only used to adjust time-out loops for floppy disk booting)
;-------------------------------------------------------
CPUMHz	equ	4	;Integer CPU speed in MHz

;===========================----------------------
;= 2. Specify the ROM Size =
;===========================
;2K-Byte EPROM option adds several commands,
;and extends a few other commands.
;   ROM2K equ TRUE selects a 2K EPROM (e.g. 2716)
;   ROM2K equ FALSE selects a 1K EPROM (e.g. 2708)
;-------------------------------------------------
ROM2K	equ	TRUE

;============================================---------------
;= 3. Specify Memon/80's Memory Utilization =
;============================================
;These specify where Memon/80 is place in memory and where
;its buffers, stack, and variables are. Additionally, you
;can disable some Memon/80 features to reduce the code size. 
;-----------------------------------------------------------
MEBASE	equ	0F800h	;Base address of Memon/80 code

RAMCOD	equ	FALSE	;Set to true if executing from RAM in
			;low memory (e.g. at 0100h for CP/M).

RAMHNT	equ	TRUE	;TRUE causes Memon to hunt for the
			;highest contiguous RAM page for its
			;stack, buffer, and variables.
			;FALSE uses RAM just below RAMEND,
			;and saves at least 26 bytes. Must be
			;FALSE for I/O boards using interrupts.

RAMEND	equ	0FFFFh	;Last allowed RAM address for MEMON/80.
			;(Ignored if RAMHNT equ TRUE.)
			;All of Memon's RAM must fit in one
			;256-byte page. This means the low 2
			;hex digits must be larger than 70h. (a
			;few more if your I/O ports use
			;interrupts.) -->Must be xxFFh for
			;Altair disk drives<--

HELPC	equ	TRUE	;TRUE includes a minimal help command.
			;FALSE saves 40 to 60 bytes.

LOWERC	equ	TRUE	;TRUE allows lowercase input too.
			;FALSE saves 11 bytes.

HLRECS	equ	TRUE	;TRUE: HL command reports total number
			;of records received when done.
			;FALSE: saves 25 bytes.

CORPT	equ	TRUE	;TRUE includes the <RPT> option for the
			;CO (copy) command.
			;FALSE saves 36 bytes.

EXOPT	equ	TRUE	;TRUE includes the EX command option to
			;disable the ROM on IN from FFh (for
			;Altair-type boards).
			;FALSE saves 17 bytes.

DUPRTY	equ	TRUE	;TRUE makes the DU output a little
			;prettier
			;FALSE saves 17 bytes

;===============================------------
;= 4. Specify the Console Port =
;===============================
;Exactly one of these must be TRUE
;-------------------------------------------
CALSIO	equ	FALSE	;Altair 88-SIO
CA2SIO	equ	FALSE	;Altair 88-2SIO (Port A or B)
			;or 8800b Turnkey Module or 88-UIO
CC2718A	equ	FALSE	;CCS 2718 Serial Port A
CC2718B	equ	FALSE	;CCS 2718 Serial Port B
CC2719A	equ	FALSE	;CCS 2719 Port A
CC2719B	equ	FALSE	;CCS 2719 Port B
CC2810	equ	FALSE	;CCS 2810 CPU'S serial port
CIFAC	equ	FALSE	;Compupro Interfacer 1 (Port A or B)
CIFAC2	equ	FALSE	;Compupro Interfacer II
CTUART	equ	FALSE	;Cromemco TU-ART (Port A or B)
CCFDCS	equ	FALSE	;Cromemco 4FDC/16FDC/64FDC serial port
CER2IO	equ	FALSE	;ECT R2IO (Port A, B, or C)
CH84	equ	FALSE	;Heathkit H8-4 Serial (any of the 4)
CH85	equ	FALSE	;Heathkit H8-5's console port
CISIO2A	equ	TRUE	;IMSAI SIO-2 Port A
CISIO2B	equ	FALSE	;IMSAI SIO-2 Port B
CIVIO2	equ	FALSE	;Ithaca Intersystems VIO-II Port A or B
CJADSP	equ	FALSE	;Jade Serial/Parallel I/O (port A or B)
CMDUBLR	equ	FALSE	;Micromation Doubler serial port
CPT3PS	equ	FALSE	;Processor Technology 3P+S
CSAL21	equ	FALSE	;Salota I/O 2+1 (Port A or B)
CSIO5	equ	FALSE	;Solid State Music IO5 Serial (A or B)
CTARBL	equ	FALSE	;Tarbell 4044 (Port A, B, C, or D)
CBITS1	equ	FALSE	;Vector Graphic BitStreamer
CBITS2	equ	FALSE	;Vector Graphic BitStreamer II Port A,B,C
CWIOB1	equ	FALSE	;Wameco IOB-1's serial port

;================================---------------------------
;= 5. Specify the Transfer Port =
;================================
;At most one of these must be TRUE. If none is TRUE then the
;following commands will be deleted: TB (set Transfer Port
;baud rate), TE (Terminal mode), and TP (Select transfer
;Port). The Transfer Port must not also be the Console Port
;-----------------------------------------------------------
TALSIO	equ	FALSE	;Altair 88-SIO
TA2SIO	equ	FALSE	;Altair 88-2SIO (Port A or B)
			;or 8800b Turnkey Module or 88-UIO
TC2718A	equ	FALSE	;CCS 2718 Serial Port A
TC2718B	equ	FALSE	;CCS 2718 Serial Port B
TC2719A	equ	FALSE	;CCS 2719 Port A
TC2719B	equ	FALSE	;CCS 2719 Port B
TC2810	equ	FALSE	;CCS 2810 CPU'S serial port
TIFAC	equ	FALSE	;Compupro Interfacer 1 (Port A or B)
TIFAC2	equ	FALSE	;Compupro Interfacer II
TTUART	equ	FALSE	;Cromemco TU-ART (Port A or B)
TCFDCS	equ	FALSE	;Cromemco 4FDC/16FDC/64FDC serial port
TER2IO	equ	FALSE	;ECT R2IO (Port A, B, or C)
TH84	equ	FALSE	;Heathkit H8-4 (any of the 4)
TH85	equ	FALSE	;Heathkit H8-5's console port
TISIO2A	equ	TRUE	;IMSAI SIO-2 Port A
TISIO2B	equ	FALSE	;IMSAI SIO-2 Port B
TIVIO2	equ	FALSE	;Ithaca Intersystems VIO-II Port A or B
TJADSP	equ	FALSE	;Jade Serial/Parallel I/O (port A or B)
TMDUBLR	equ	FALSE	;Micromation Doubler serial port
TPT3PS	equ	FALSE	;Processor Technology 3P+S
TSAL21	equ	FALSE	;Salota I/O 2+1 (Port A or B)
TSIO5	equ	FALSE	;Solid State Music IO5 Serial (A or B)
TTARBL	equ	FALSE	;Tarbell 4044 (Port A, B, C, or D)
TBITS1	equ	FALSE	;Vector Graphic BitStreamer
TBITS2	equ	FALSE	;Vector Graphic BitStreamer II Port A,B,C
TWIOB1	equ	FALSE	;Wameco IOB-1's serial port

;=========================================--------------------
;= 6. Set the Serial port base addresses =
;=========================================
;Here are the default addresses for various boards. Make sure
;that CBASE is not the same as TBASE, and that their address
;ranges do not overlap,  unless they are on the same serial
;board, sharing address space. (some board combinations will
;require you to change the address of at least one board to a
;non-default address.) Also make sure that neither serial port
;interferes with the addresses used by the disk controller,
;if you are enabling the BO (boot from floppy) command.
;
;                                    Port      Board
;                                    Base     Address
;   Serial Port Board        Port   Address    Range
;-------------------------------------------------------------
; Altair 88-SIO                       00h     00h-01h
;
; Altair 88-2SIO             Port A   10h     10h-13h
;                            Port B   12h
;
; Altair 8800b Turnkey       Serial   10h     10h-11h
;
; MITS 88-UIO                Serial   10h     10h-11h
;
; CCS 2718                    Ser A  00h     00h-03h
; (Jumpers SPR=N,SAI-N,SBI=I) Ser B  00h
;
; CCS 2719                   Port A   50h     50h-57h
;                            Port B   50h
;
; CCS 2810                   Serial   20h     20h-26h
;
; Compupro Interfacer 1      Port A   00h     00h-03h
;                            Port B   02h
;
; Compupro Interfacer II              00h     00h-01h
;
; Cromemco TU-ART            Port A   20h     20h-23h
;                            Port B   50h     50h-53h
;
; Cromemco 4FDC/16FDC/64FDC  Serial   00h     00h-03h
;
; ECT R2I/O                  Port A   00h     00h-07h
;                            Port B   02h
;                            Port C   04h
;
; Heathkit H8-4              Console  0E8h   0E8h-0EDh
;                        Lineprinter  0E0h   0E0h-0E6h
;                         Alt Term 0  0C0h   0C0h-0C6h
;                         Alt Term 1  0C8h   0C8h-0CDh
;                         Alt Term 2  0D0h   0D0h-0D6h
;                         Alt Term 3  0D8h   0D8h-0DDh
;
; Heathkit H8-5              Console  0FAh   0F8h-0FBh
;
; IMSAI SIO-2                Port A   02h     02h-06h
;                            Port B   04h
;
; Ithaca Intersystems VIO-II Port A   00h     00h-1Fh
;                            Port B   04h
;
; Jade Serial/Parallel I/O   Port A   00h     00h-02h
;                            Port B   01h    & 80h-82h
;
; Micromation Doubler        Serial 0F800h  0F800h-0FFFFh
;   (memory-mapped)
;
; Processor Technology 3P+S   C & D   00h     00h-03h
;
; Salota I/O 2+1            Serial A  80h     80h-8Eh
;                           Serial B  82h
;
; Solid State Music IO5     Serial A  00h     00h-0Dh
;                           Serial B  02h
;
; Tarbell 4044 4S2P IO       Port A   11h     10h-18h
;                            Port B   13h
;                            Port C   15h
;                            Port D   17h
;
; VG BitStreamer       -              02h     02h-03h
;
; VG BitStreamer II          Port A   02h     02h-09h
;                            Port B   04h
;                            Port C   06h
;
; Wameco IOB-1               Serial   04h     00h-05h
;-------------------------------------------------------------
CBASE	equ	002h	;Console Port base address
TBASE	equ	022h	;Transfer Port base address

;=====================================--------------------
;= 7. Set the Serial Port Baud Rates =
;=====================================
;Set the Console baud rate and the initial Transfer Port
;baud rate, for boards that support software-controlled
;baud rates. (Note that the TB command is only available
;if ROM2K is TRUE and a Transfer Port has been specified.)
;The following supported boards allow software to set the
;baud rate:
;
;  Serial Port Board                Max Rate      Notes
;---------------------------------------------------------
; CCS 2718                           19200
; CCS 2719                           57600
; CCS 2810                           38400
; Cromemco TU-ART                    76800     no 600 baud
; Cromemco 4FDC/16FDC/64FDC          76800     no 600 baud
; Heathkit H8-4                      38400
; Ithaca Intersystems System II VIO  19200
;
;Specify the baud rates from this table:
;
;    Baud  Stop   CBAUD/TBAUD
;    Rate  Bits      Value
;----------------------------
;     110    2         0
;     150    1         1
;     300    1         2
;     600    1         3
;    1200    1         4
;    2400    1         5
;    4800    1         6
;    9600    1         7
;   19200    1         8
;   38400    1         9
;   57600    1        10
;   76800    1        11
;---------------------------------------------------------
CBAUD	equ	9	;Console baud rate
TBAUD	equ	10	;Initial Transfer Port baud rate

;==================================------------------
;= 8. Specify the Disk controller =
;==================================
;The disk controller is only used for the BO command.
;At most one of these may be TRUE. If none is TRUE,
;then the BO (boot) command will be deleted.
;----------------------------------------------------
A88DCD	equ	FALSE	;MITS Altair 88-DCDD
A88MCD	equ	FALSE	;MITS Altair 88-MDS
CC2422	equ	FALSE	;CCS 2422 Disk controller
C4FDC	equ	FALSE	;Cromemco 4FDC
C16FDC	equ	FALSE	;Cromemco 16FDC
C64FDC	equ	FALSE	;Cromemco 64FDC
H17R	equ	FALSE	;Heathkit H17 (onboard boot ROM),
IFIF	equ	TRUE	;IMSAI FIF
IMD8R	equ	FALSE	;IMSAI MDC-DIO with 8" disk (onboard
			;..boot ROM
IMDMR	equ	FALSE	;IMSAI MDC-DIO with minidisk (onboard
			;..boot ROM
MDUBLR	equ	FALSE	;Micromation Doubler (onboard boot
			;..ROM
MICROP	equ	FALSE	;Micropolis FD controller B
MICROR	equ	FALSE	;Micropolis FD Controller B (onboard
			;..boot ROM
NSTARS	equ	FALSE	;Northstar MDC-A and MDS-A
NSTRSR	equ	FALSE	;Northstar MDC-A and MDS-A (onboard
			;..boot ROM
NSTARD	equ	FALSE	;Northstar MDS-D
NSTRDR	equ	FALSE	;Northstar MDS-D (onboard boot ROM)
SALFDC	equ	FALSE	;Salota FDC (same as Versafloppy II)
VERSA1  equ     FALSE	;SD Systems Versafloppy
VERSA2  equ     FALSE	;SD Sales Versafloppy II
TARBL1	equ	FALSE	;Tarbell 1011
TARBL2	equ	FALSE	;Tarbell 2022

;=================================================-------------
;= 9. Set the Disk Controller's Base Addresse(s) =
;=================================================
;Here are the default base addresses for various boards. Note
;that some disk controllers use I/O address, some use memory
;addresses, and some use both. Also, some of these boards do
;not allow you to change their I/O addresses and/or memory
;addresses.
;                             I/O     I/O      Memory   Memory
;                             Base    Top       Base     Top
;     Disk Controller        Address Address   Address  Address
;--------------------------------------------------------------
; MITS Altair 88-DCDD/88-MDS  08h     0Ah
; CCS 2422 Disk controller   {04h & 30h-34h}
; Cromemco 4FDC/16FDC/64FDC  {04h & 30h-34h}
; Heathkit H17                                 01800h    01FFFh
; IMSAI FIF                   0FDH    0FDH
; IMSAI MDC-DIO               0D0h    0DFh     0E000h    0EFFFh
; Micromation Doubler                          0F800h    0FFFFh
; Micropolis FD controller B                   0F400h    0F7FFh
; Northstar MDC-A and MDS-A                    0E800h    0EBFFh
; Northstar MDS-D                              0E800h    0EBFFh
; SD Systems Versafloppy      060h    067h
; Tarbell 1011/2022           0F8h    0FFH
;--------------------------------------------------------------
DIBASE	equ	0fdh	;Disk Controller I/O Base Address
DMBASE	equ	0000h	;Disk controller memory Base Address

;=====================================================---------
;= 10. Specify whether or not interrupts are enabled =
;=====================================================
;"ENINTS equ TRUE" will generally leave interrupts enabled,
;masking them only during initialization and when time-critical
;code is running. Unless your computer requires interrupts to
;be enabled, set this to FALSE.
;
;"ENINTS equ FALSE" will mask interrupts always, and save
;several bytes of code space, and protect you from random
;interrupts and potential code crashes in a computer where
;interrupts are not set up.
;
;Some I/O ports (e.g. the Heathkit H8-5) require interrupts.
;If such a board is specified above, then interrupts will get
;enabled anyway, (and the necessary interrupt code will be
;installed) regardless of what you enter here.
;--------------------------------------------------------------
ENINTS equ	FALSE

;===============================
;=   End of Assembly Options   =
;= No user options below here. =
;===============================


;==============================================================
;Memon/80 Commands (all values are in HEX)
;
; BO <DRIVE>  {Some controllers require ROM2K}
;    Boot from floppy disk. The <DRIVE> option ( a number
;    between 0 and 3) is only available for the Cromemco disk
;    controllers, because CDOS is one of the few S100 operating
;    systems that allows booting from anything but the first
;    drive.
;
; CE [command line]  {Requires ROM2K}
;    Execute CP/M program: Copy Command line (any text string)
;    to RAM at 0080h, install WBOOT jump at 0000h, and jump to
;    program at 0100h.
;
; CO <SRC> <DST> <BCNT> [<RPT>]
;    Copy <BCNT> bytes of memory from address <SRC> to address
;    <DST>. The <RPT> option is available only if CORPT=TRUE.
;    This optionally repeats the copy <RPT> times. (This is for
;    programming EPROMS with e.g. a Cromemco bytesaver).
;
; DU [<ADR> [<BCNT>]]
;    Dump <BCNT> (which defaults to 1) bytes of memory in both
;    hex and ASCII, starting at address <ADR> (defaults to 0).
;
; EN [<ADR>]
;    Enter hex data into memory at <ADR>, which defaults to 0.
;    values are separated with spaces or CR'S. Quit EN command
;    with a blank line.
;
; EX [[<ADR>] <OPT>]
;    Execute at <ADR>, which defaults to 0. ProgramS can RET
;    to Memon/80'S main loop.
;    <OPT> is available only of ROM2K=TRUE. If <OPT>=1 then
;    Memon/80 executes an IN from port FFh before jumping to
;    the specified address. (This disables PROM on the
;    MITS 8800b Turnkey Module and on my 88-2SIOJP.)
;
; FI [<ADR> [<BCNT> [<VAL>]]]
;    Fill <BCNT> bytes of memory starting at <ADR> with <VAL>,
;    <VAL> and <ADR> default to 0. <BCNT> defaults to all RAM.
;    Stop when fill reaches Memon/80'S RAM page.
;
; HD <ADR> <BCNT>
;    Intel hex dump <BCNT> bytes of memory starting at <ADR>,
;    to the Transfer Port
;
; HL [<OFST>]
;    Load Intel hex file to memory from the Transfer Port. Add
;    optional address offset <OFST> to each record address.
;    This will generate a "Mem" error if a record will write
;    over Memon/80's RAM page (or RAM image, if RAMCOD=TRUE).
;    HL will report the total number of records received when
;    an Intel hex EOF record is received, if ROM2K=TRUE.
;
; IN <PORT>
;    Input from <PORT> and print result on Console
;
; MT <ADR> <CNT>  {Requires ROM2K}
;    Test <CNT> bytes of memory, starting at <ADR>. This is a
;    destructive test - RAM will be filled with garbage. Errors
;    are reported to the Console. This will skip over the
;    portion of RAM that Memon uses for its stack.   
;
; OT <PORT> <data>
;    Output <data> to <PORT>
;
; SE <ADR> <Byte1> [<byte2> [<byte3> [..<Byten>]]]
;     or
; SE <ADR> 'text string'  {Requires ROM2K}
;    Search for string in memory, starting at address <ADR>
;    Can also mix forms e.g.
; SE 100 'Hello world' 0D 0A 'second line'
;
; TB <BAUD>  {Requires ROM2K}
;            {deleted if no Transfer Port specified}
;    Set TRansfer Port baud rate, <BAUD> from this table:
;    Value  Baud Rate
;      0       110 (2 stop bits)
;      1       150
;      2       300
;      3       600
;      4      1200
;      5      2400
;      6      4800
;      7      9600
;      8     19200
;      9     38400
;      A     76800
;
;    This command is only available if the serial port board
;    that's selected to be the Transfer Port allows software to 
;    set its baud rate. Not all baud rates are available for
;    every board. Attempting to select an unsupported baud rate
;    will result in a command error.
;
; TE [<EXCHR>] {deleted if no Transfer Port specified}
;    Terminal mode: Console keyboard input goes to Port B
;    output, and Port B input goes to the Console output.
;    Type CTRL-<EXCHR> (which defaults to CTRL-C) to exit.
;
; TP [<0/1>] {deleted if no Transfer Port specified}
;    Enable the Transfer Port. "TP 0" disables the Transfer
;    port, causing Transfer Port operations to use the Console
;    Port instead. defaults to 1 (enabled).
;
; VE <SRC> <DST> <BCNT>
;    Verify <BCNT> bytes of memory, starting at <SRC> and <DST>
;==============================================================
;REVISION HISTORY
;
;VERS. 2.6 by M. Eberhard  1 May 2019 2019
;  Fix bug with "?" (help) command (Thanks Thomas)
;
;VERS. 2.5 by M. Eberhard  11 April 2019
;  Comment cleanup, fix problem with MITS disk controller
;  selection.
;
;VERS. 2.4 by M. Eberhard 26 December 2018
;  Work around ASM bug: "set" variables that control
;  conditional assembly must be declared as 0 (FALSE) first,
;  before any other value gets assigned to them. Some
;  additional cleanup as well.
;
;VERS. 2.3 by M. Eberhard 3 November 2018
;  Support the Heathkit H8-4. Support the H8-5, which requires
;  interrupts for receiving data. Allow interrupts to be
;  enabled, generally. Allow for no transfer port to be
;  specified and eliminate all transfer port commands if no
;  transfer port is defined. Support H17 disk controller. Allow
;  simple disk boots to work without ROM2K=FALSE. Allow more
;  assembly options to save code space.
;
;VERS. 2.2 by M. Eberhard 1 October 2018
;  Fix pseudo-op bug when assembling with ROM2k=FALSE
;
;VERS. 2.1 by M. Eberhard  26 April 2018
;  Correct comments for IMSAI 2SIO ports. (Thanks, Len B.)
;  Support many more I/O boards and disk controllers. Correct a
;  few comments. Add CE command.
;
;VERS. 2.0 by M. Eberhard  31 December 2016
;  Eliminate Module interface. default TP to enabled.
;  ROM2K option adds a bunch of commands, requires 2K EPROM.
;  (Unreleased Version)
;
;VERS. 1.1 by M. Eberhard 9 November 2016
;  Add IN and OT commands. Better defaults and RAM overwrite
;  protection for FI. check each byte for RAM page overwrite
;  during HL. disable ints for the IMSAI SIO-2. add support for
;  several more serial ports. Squeeze code. Eliminate record-
;  type test on HL. Improve comments & labels a lot.
;
;VERS. 1.00 by M. Eberhard 14 April 2014
;  Created
;==============================================================

VERSION	equ	26h		;BCD-encoded version number

 if not ROM2K
ROMSIZ	equ	400h		;1K EPROM size (e.g. 2708)
 endif ;not ROM2K
 if ROM2K
ROMSIZ	equ	800h		;2K EPROM size (e.g. 2716)
 endif ;ROM2K

;***********************
;General program Equates
;***********************
CTRLC	equ	03h		;ASCII control-C
BS	equ	08h		;ASCII backspace
LF	equ	0Ah		;ASCII linefeed
CR	equ	0DH		;ASCII carriage return
QUOTE	equ	27h		;ASCII single quote
DEL	equ	7Fh		;ASCII delete

PROMPT	equ	'>'		;prompt character
CABKEY	equ	CTRLC		;command abort character
DTEXIT	equ	CTRLC		;default Terminal mode exit chr
PCFIER	equ	'.'		;Console pacifier chr
HDRLEN	equ	16		;Intel hex record length for HD

STKSIZ	equ	32		;Max stack size
LBSIZE	equ	80		;input line buffer size

;*********************
;RAM Page Organization
;*********************
 if RAMHNT and not (A88DCD or A88MCD)
RAMBUF	equ	100h-LBSIZE	;buffer offest in RAM page
 endif ;RAMHNT and not (A88DCD or A88MCD)

 if not RAMHNT and not (A88DCD or A88MCD)
RAMBUF	equ	RAMEND-LBSIZE+1	;buffer address in RAM page
 endif ;not RAMHNT and not (A88DCD or A88MCD)

;The Altair disk controller code requires the buffer to be at
;the end of a 256-byte page

 if  RAMHNT and (A88DCD or A88MCD)
RAMBUF	equ	100h-85h	;Exactly room for 1 sector
				;SECSIZ=BPS+HDRSIZ+TLRSIZ
 endif ;RAMHNT and (A88DCD or A88MCD)

 if  (not RAMHNT) and (A88DCD or A88MCD)
RAMBUF	equ	RAMEND-85h+1	;Exactly room for 1 sector
				;SECSIZ=BPS+HDRSIZ+TLRSIZ
 endif ;(not RAMHNT) and (A88DCD or A88MCD)

;The stack goes just below the RAM buffer

STACK	equ	RAMBUF		;SP decrements before writing

;Assign for interrupt flags, even if we don't use them
;Note: if interrupts are used then RAMHNT MUST be FALSE

CIFLAG	equ	STACK-STKSIz-1	;console interrupt Rx flag
CRXBUF	equ	STACK-STKSIz-2	;console interrupt Rx buffer
TIFLAG	equ	STACK-STKSIz-3	;T-port interrupt Rx flag 
TRXBUF	equ	STACK-STKSIz-4	;T-port interrupt Rx buffer

;************************************************************
;CP/M Equates (for CE command)
;The CE command creates a CP/M BIOS-like jump to Memon/80's
;warm-boot entry point, which is followed by a CP/M BIOS-
;like jump table to its I/O routines. The CE command also
;moves the command-line text entered by the user to the
;CP/M-like COMBUF. These make execution possible for simple
;CP/M programs that only call the BIOS for console I/O, and
;make no BDOS calls. This command is especially useful for
;building a CP/M disk form scratch, allowing you to load
;(via the HL command) and run (via the CE command) programs
;like FORMAT and PUTSYS. Once you have a diskette with a bare
;CP/M and a working BIOS, you can uses these commands to load
;CP/M's non-built in commands onto the diskette.
;************************************************************
 if ROM2K
WBOOT	equ	0000H		;Jump to "BIOS" warm boot
WBOOTA	equ	WBOOT+1		;Address of Warm Boot
COMBUF	equ	WBOOT+80H	;command line buffer
USAREA	equ	WBOOT+100H	;User program area
 endif ;ROM2K

;==================================================
;= Option Selection Logic                         =
;=                                                =
;= The following long section of equates performs =
;= the logic for supporting the various serial    =
;= ports and disk controllers, as selected above. =
;= Generally, these should not be modified.       =
;==================================================

;-------------------------------------------
;Initialize some assembler variables that
;may get changed, based on options set above
;-------------------------------------------
TMEMAP	set	FALSE
CMEMAP	set	FALSE
TMRCTC	set	FALSE
T5511	set	FALSE
C6850	set	FALSE
T6850	set	FALSE
C8250	set	FALSE
T8250	set	FALSE
C8251	set	FALSE
T8251	set	FALSE
DART	set	FALSE
CISTAT	set	FALSE
TISTAT	set	FALSE
CSBAUD	set	FALSE
TSBAUD	set	FALSE
BD192	set	FALSE
BD384	set	FALSE
BD576	set	FALSE
BD768	set	FALSE
CRXINT	set	FALSE
TRXINT	set	FALSE
TPORT	set	FALSE

;-------------------------------------------------------
;Altair 88-SIO Equates
;
;The 88-SIO is built around a 2502-type generic UART,
;with a jumper selectable baud rate generator and with
;external logic providing status and control registers.
;
;Note that the 88-SIO requires no initialization.
;-------------------------------------------------------
 if CALSIO

CSTAT	equ	CBASE		;1st SIO status input port
CDATA	equ	CBASE+1		;1st SIO Rx & Tx data regs

CRXRDY	equ	00000001B	;Console Rx data reg full
CTXRDY	equ	10000000B	;Console Tx data reg empty

CISTAT	set	TRUE		;Status bits are active-low
 endif ;CALSIO

 if TALSIO
TPORT	set	TRUE		;Transfer port is defined

TSTAT	equ	TBASE		;2nd SIO status input port
TDATA	equ	TBASE+1		;2nd SIO Rx & Tx data regs

TRXRDY	equ	00000001B	;Transfer Port Rx data reg full
TTXRDY	equ	10000000B	;Trans. port Tx data reg empty

TISTAT	set	TRUE		;Status bits active-low
 endif ;TALSIO

;-------------------------------------------------------------
;Altair 88-2SIO, 8800b Turnkey Module, and MITS 88-UIO Equates
;
;The 88-2SIO, the 8800b Turnkey Module, the 88-UIO, and my own
;88-2SIOJP are all built around a pair of MC6850 ACIA'S with a
;jumper-selectable baud rate generator.
;-------------------------------------------------------------
 if CA2SIO
C6850	set	TRUE		;Altair 6850-type UART present
CB6850	equ	CBASE
 endif ;CA2SIO

 if TA2SIO
TPORT	set	TRUE		;Transfer port is defined
T6850	set	TRUE		;Altair 6850-type UART present
TB6850	equ	TBASE
 endif ;TA2SIO

;--------------------------------------------------------------
;CCS 2718 Equates
;
;The CCS 2718's serial port A is built around an 1883/1602/1611
;generic UART. Its serial port B is built around an 8251 UART.
;Both UARTs are clocked by an external baud rate generator that
;can be forced to a chosen baud rate for each port with
;juimpers, or jumpered for software baud rate control. This
;code assumes that software can control the baud rates.
;Additionally, the control register for serial port A is
;configured with jumpers. This code assumes the configuration
;is as shown on page 2-8 of the CCS 2718 manual.
;--------------------------------------------------------------
 if CC2718A
CSTAT	equ	CBASE	;serial port A status
CCTRL	equ	CBASE	;channel a control
CDATA	equ	CBASE+1	;serial port A data

CTXRDY	  equ	  01h	;Console Tx ready bit
CRXRDY	  equ	  02h	;Console Rx ready bit

CBD110	equ	10Fh	;0= 110 baud
CBD150	equ	10Eh	;1= 150 baud
CBD300	equ	10Dh	;2= 300 baud
CBD600	equ	106h	;3= 600 baud
CBD1200	equ	10Bh	;4= 1200 baud
CBD2400	equ	10Ch	;5= 2400 baud
CBD4800	equ	109h	;6= 4800 baud
CBD9600	equ	108H	;7= 9600 baud
CBD192	equ	0	;8= 19200 baud
CBD384	equ	0	;9= 38400 baud (not supported)
CBD576	equ	0	;a= 57600 baud (not supported)
CBD768	equ	0	;b= 76800 baud (not supported)

CSBAUD	set	TRUE	;software Console baud rate control
 endif ;CCS2718A

 if CC2718B
C8251 set TRUE
CB8251	equ	CBASE+2	;Base of Console 8251 registers

CBD110	equ	1F0h	;0= 110 baud
CBD150	equ	1E0h	;1= 150 baud
CBD300	equ	1D0h	;2= 300 baud
CBD600	equ	160h	;3= 600 baud
TBD1200	equ	1B0h	;4= 1200 baud
CBD2400	equ	1C0h	;5= 2400 baud
CBD4800	equ	190h	;6= 4800 baud
CBD9600	equ	180H	;7= 9600 baud
CBD192	equ	0	;8= 19200 baud
CBD384	equ	0	;9= 38400 baud (not supported)
CBD576	equ	0	;a= 57600 baud (not supported)
CBD768	equ	0	;b= 76800 baud (not supported)

CSBAUD	set	TRUE	;software Console baud rate control
 endif ;CCS2718B

 if TC2718A
TPORT	set	TRUE	;Transfer port is defined

TSTAT	equ	TBASE	;serial port A status
TCTRL	equ	TBASE	;channel a control
TDATA	equ	TBASE+1	;serial port A data

TTXRDY	  equ	  01h	;Transfer Port Tx ready bit
TRXRDY	  equ	  02h	;Transfer Port Rx ready bit

TBD110	equ	10Fh	;0= 110 baud
TBD150	equ	10Eh	;1= 150 baud
TBD300	equ	10Dh	;2= 300 baud
TBD600	equ	106h	;3= 600 baud
TBD1200	equ	10Bh	;4= 1200 baud
TBD2400	equ	10Ch	;5= 2400 baud
TBD4800	equ	109h	;6= 4800 baud
TBD9600	equ	108H	;7= 9600 baud
TBD192	equ	100h	;8= 19200 baud
TBD384	equ	0	;9= 38400 baud (not supported)
TBD576	equ	0	;a= 57600 baud (not supported)
TBD768	equ	0	;b= 76800 baud (not supported)

TSBAUD	set	TRUE	;software transfer port baud control
 endif ;TCS2718A

 if TC2718B
T8251 set TRUE
TB8251	equ	CBASE+2		;Base of T-port 8251 registers

TBD110	equ	1F0h	;0= 110 baud
TBD150	equ	1E0h	;1= 150 baud
TBD300	equ	1D0h	;2= 300 baud
TBD600	equ	160h	;3= 600 baud
TBD1200	equ	1B0h	;4= 1200 baud
TBD2400	equ	1C0h	;5= 2400 baud
TBD4800	equ	190h	;6= 4800 baud
TBD9600	equ	180H	;7= 9600 baud
TBD192	equ	100h	;8= 19200 baud
TBD384	equ	0	;9= 38400 baud (not supported)
TBD576	equ	0	;a= 57600 baud (not supported)
TBD768	equ	0	;b= 76800 baud (not supported)

TSBAUD	set	TRUE	;software transfer port baud control
BD192	set	TRUE	;19200 baud supported
 endif ;CCS2718B

;-------------------------------------------------------
;CCS 2719 Equates
;
;The CCS 2719 is built around a Z80-DART (dual UART) and
;a Z80-CTC (counter/timer, which generates baud rates).
;-------------------------------------------------------
 if CC2719A or CC2719B or TC2719A or TC2719B

DART	set	TRUE	;DART present

DWR1	equ	1	;Write Register 1
DWR2	equ	2	;Write Register 2
DWR3	equ	3	;Write Register 3
DWR4	equ	4	;Write Register 4
DWR5	equ	5	;Write Register 5

DART1S	equ	44h	;1 stop bit
DART2S	equ	4CH	;2 stop bits

;DART initialization sequence, both ports
;note: INIT code assumes that DI7 is UNIQUE

DI1	equ	28h	;channel Reset
DI2	equ	DWR1	;Access WR1
DI3	equ	00h	;Disable interrupts
DI4	equ	DWR3	;Access WR3
DI5	equ	0C1h	;Rx 8-bit bytes, enable Rx
DI6	equ	DWR5	;Access WR5
DI7	equ	0EAh	;Tx 8-bit bytes, enable Tx, 
			;handshakes true

 endif ;CC2719A or CC2719B or TC2719A or TC2719B

 if CC2719A
CCTC	equ	CBASE	;CTC channel for Console baud
CDATA	equ	CBASE+4	;channel A data
CSTAT	equ	CBASE+5	;channel A status
CCTRL	equ	CBASE+5	;channel A control
 endif ;CC2719A

 if CC2719B
CCTC	equ	CBASE+1	;CTC channel for Console baud
CDATA	equ	CBASE+6	;channel B data
CSTAT	equ	CBASE+7	;channel B status
CCTRL	equ	CBASE+7	;channel B control
 endif ;CC2719B

 if TC2719A
TCTC	equ	TBASE	;CTC channel for TR port baud
TDATA	equ	TBASE+4	;channel A data
TSTAT	equ	TBASE+5	;channel A status
TCTRL	equ	TBASE+5	;channel A control
 endif ;TC2719A

 if TC2719B
TCTC	equ	TBASE+1	;CTC channel for TR port baud
TDATA	equ	TBASE+6	;channel B data
TSTAT	equ	TBASE+7	;channel B status
TCTRL	equ	TBASE+7	;channel B control
 endif ;TC2719B

 if CC2719A or CC2719B
CTXRDY	  equ	  04h	 ;Console Tx ready bit
CRXRDY	  equ	  01h	 ;Console Rx ready bit

;Console Port baud rate values
;high byte must be written 1st

CBD110	equ	078DH	;0= 110 baud 2 stop bits
CBD150	equ	0768h	;1= 150 baud
CBD300	equ	0734h	;2= 300 baud
CBD600	equ	47C0h	;3= 600 baud
CBD1200	equ	4760h	;4= 1200 baud
CBD2400	equ	4730h	;5= 2400 baud
CBD4800	equ	4718h	;6= 4800 baud
CBD9600	equ	470CH	;7= 9600 baud
CBD192	equ	4706h	;8= 19200 baud
CBD384	equ	4703h	;9= 38400 baud
CBD576	equ	4702h	;a= 57600 baud
CBD768	equ	0	;b= 76800 baud (not supported)

CSBAUD	set	TRUE	;software Console
			;..baud rate control
CSTOP1	equ	DART1S
CSTOP2	equ	DART2S

CSTOPS	set	CSTOP2
 endif ;CC2719A or CC2719B

  if (CC2719A or CC2719B) and ((0-CBAUD) SHR 15)
CSTOPS	set	CSTOP1
 endif ;(CC2719A or CC2719B) and ((0-CBAUD) SHR 15)

 if TC2719A or TC2719B
TPORT	set	TRUE	;Transfer port is defined

TTXRDY	  equ	  04h	;Transfer Port Tx ready bit
TRXRDY	  equ	  01h	;Transfer Port Rx ready bit

;Transfer Port baud rate values
;high byte must be written 1st

TBD110	equ	078DH	;0= 110 baud 2 stop bits
TBD150	equ	0768h	;1= 150 baud
TBD300	equ	0734h	;2= 300 baud
TBD600	equ	47C0h	;3= 600 baud
TBD1200	equ	4760h	;4= 1200 baud
TBD2400	equ	4730h	;5= 2400 baud
TBD4800	equ	4718h	;6= 4800 baud
TBD9600	equ	470CH	;7= 9600 baud
TBD192	equ	4706h	;8= 19200 baud
TBD384	equ	4703h	;9= 38400 baud
TBD576	equ	4702h	;a= 57600 baud
TBD768	equ	0	;b= 76800 baud (not supported)

BD192	set	TRUE	;19200 baud supported
BD384	set	TRUE	;38400 baud supported
BD576	set	TRUE	;57600 baud supported

BRATE0	equ	TCTC	;LOW baud byte goes here
BRATE1	equ	TCTC	;high baud byte goes here

TMRCTC	set	TRUE	;Z80-CTC used as timer
TSBAUD	set	TRUE	;software Transfer Port

TSTOP1	equ	DART1S
TSTOP2	equ	DART2S

TSTOPS	set	TSTOP2
 endif ;TC2719A or TC2719B

  if (TC2719A or TC2719B) and ((0-TBAUD) SHR 15)
TSTOPS	set	TSTOP1
 endif ;(TC2719A or TC2719B) and ((0-TBAUD) SHR 15)

;--------------------------------------------------
;CCS 2810 Serial Port Equates
;
;The serial port on the CCS 2810 CPU board is built
;around the National Semiconductor INS8250 UART,
;which has a buad rate generator built into it.
;--------------------------------------------------

;CCS2810 serial port as the Console

 if CC2810
C8250	set	TRUE
CB8250	equ	CBASE
 endif ;CC2810

;CCS2810 serial Port As the Transfer Port

 if TC2810
T8250	set	TRUE
TB8250	equ	TBASE
 endif ;TC2810

;--------------------------------------------------------
;Compupro Interfacer 1 Equates
;
;The Interfacer 1 is built around a pair of 1602-type
;generic UARTs, with jumper selectable baud rate
;generators and with external logic providing status
;and control registers.
;
;Control bits are configured with jumpers. the board will
;XOR whatever you write to the control port with the
;jumper setting. This assumes all jumpers are set to 0.
;--------------------------------------------------------
 if CIFAC
CSTAT	equ	CBASE	;Console status
CCTRL	equ	CBASE	;Console control
CDATA	equ	CBASE+1	;Console data

CTXRDY	  equ	  01h	;channel a Tx ready bit
CRXRDY	  equ	  02h	;channel a Rx ready bit
 endif ;CIFAC

 if TIFAC
TPORT	set	TRUE	;Transfer port is defined

TSTAT	equ	TBASE	;Transfer Port status
TCTRL	equ	TBASE	;Transfer Port control
TDATA	equ	TBASE+1	;Transfer Port data

TTXRDY	  equ	  01h	;channel B Tx ready bit
TRXRDY	  equ	  02h	;channel B Rx ready bit
 endif ;TIFAC

 if CIFAC or TIFAC
IFRST	equ	0ACH	;reset: 8 data, no parity, flow
			;..control outputs high, no ints
 endif	;CIFAC or TIFAC

;---------------------------------------------------------
;Compupro Interfacer II Equates
;The Interfacer II is built around a  1602-type generic
;UART, with jumper selectable baud rate generator and with
;external logic providing status and control registers.
;
;Control bits are configured with jumpers. The board will
;XOR whatever you write to the control port with the
;jumper setting. This assumes all jumpers are set to 0.
;---------------------------------------------------------
 if CIFAC2
CSTAT	equ	CBASE	;Console status
CCTRL	equ	CBASE	;Console control
CDATA	equ	CBASE+1	;Console data

CTXRDY	  equ	  01h	;Console Tx ready bit
CRXRDY	  equ	  02h	;Console Rx ready bit

CIFRST	equ	0ACH	;reset: 8 data, no parity, flow
			;..control outputs high, no ints

 endif ;CIFAC2

 if TIFAC2
TPORT	set	TRUE	;Transfer port is defined

TSTAT	equ	TBASE	;Transfer Port status
TCTRL	equ	TBASE	;Transfer Port control
TDATA	equ	TBASE+1	;Transfer Port data

TTXRDY	  equ	  01h	;Transfer Port Tx ready bit
TRXRDY	  equ	  02h	;Transfer Port Rx ready bit

TIFRST	equ	0ACH	;reset: 8 data, no parity,
			;..controls high, no ints
 endif ;TIFAC2

;------------------------------------------------------
;Cromemco TU-ART Equates
;Cromemco 4FDC/16FDC/64FDC serial port Equates
;
;The TU-ART is built around a pair of TMS5511 UARTs
;with integral software-settable baud rate generators.
;
;The serial ports on the Cromemco xFDC disk controllers
;are built around the (very similar) TMS5501 UART
;------------------------------------------------------
 if CTUART or TTUART or CCFDCS or TCFDCS
TURST	equ	  01h	;reset UART
TUHBD	equ	  10h	;high baud rate
 endif ;CTUART or TTUART or CCFDCS or TCFDCS

 if CTUART or CCFDCS
CSTAT	equ	CBASE	;Console status
CDATA	equ	CBASE+1	;Console data
TCPBAUD	equ	CBASE	;Console baud rate
TCCTRL	equ	CBASE+2	;Console control
TCINTE	equ	CBASE+3	;Console interrupt enable

CRXRDY	equ	40h	;Console Rx data buffer full
CTXRDY	equ	80h	;Console Tx data buf available

CSTOP1	equ	80h	;1 stop bit
CSTOP2	equ	0	;2 stop bits

;Console Port TU-ART baud rates
; high byte is the command register value
; low byte is the baud rate register value
; bit 7 of the low byte selects 2 stop bits if 0

CBD110	equ	0001h or CSTOP2	;110 baud
CBD150	equ	0002h or CSTOP1	;150 baud
CBD300	equ	0004h or CSTOP1	;300 baud
CBD600	equ	0 		;600 baud (not supported)
CBD1200	equ	0008h or CSTOP1	;1200 baud
CBD2400	equ	0010h or CSTOP1	;2400 baud
CBD4800	equ	0020h or CSTOP1	;4800 baud
CBD9600	equ	0040h or CSTOP1	;9600 baud
CBD192	equ	1010h or CSTOP1	;19200 baud
CBD384	equ	1020h or CSTOP1	;38400 baud
CBD576	equ	0 		;57600 baud (not supported)
CBD768	equ	1040h or CSTOP1	;76800 baud

CSBAUD	set	TRUE	;software Console
			;..baud rate control
CSTOPS	set	CSTOP2
 endif ;CTUART or CCFDCS

  if (CTUART or CCFDCS) and ((0-CBAUD) SHR 15)
CSTOPS	set	CSTOP1
 endif ;(CTUART or CCFDCS) and ((0-CBAUD) SHR 15)

 if TTUART or TCFDCS
TPORT	set	TRUE	;Transfer port is defined

TSTAT	equ	TBASE	;Transfer Port status
TDATA	equ	TBASE+1	;Transfer Port data
TTPBAUD	equ	TBASE	;Transfer Port baud rate
TTCTRL	equ	TBASE+2	;Transfer Port control
TTINTE	equ	TBASE+3	;Transfer Port interrupt enable

TRXRDY	equ	40h	;Tx Port Rx data buffer full
TTXRDY	equ	80h	;Tx Port Tx data buf available

BRATE0	equ	TTPBAUD	;LOW baud byte goes here
BRATE1	equ	TTCTRL	;high baud byte goes here

TSTOP1	equ	80h	;1 stop bit
TSTOP0	equ	0	;2 stop bits

;Transfer Port TU-ART baud rates
; high byte is the command register value
; low byte is the baud rate register value

TBD110	equ	0001h or TSTOP2	;110 baud
TBD150	equ	0002h or TSTOP1	;150 baud
TBD300	equ	0004h or TSTOP1	;300 baud
TBD600	equ	0		;600 baud (not supported)
TBD1200	equ	0008h or TSTOP1	;1200 baud
TBD2400	equ	0010h or TSTOP1	;2400 baud
TBD4800	equ	0020h or TSTOP1	;4800 baud
TBD9600	equ	0040h or TSTOP1	;9600 baud
TBD192	equ	1010h or TSTOP1	;19200 baud
TBD384	equ	1020h or TSTOP1	;38400 baud
TBD576	equ	0 		;57600 baud (not supported)
TBD768	equ	1040h or TSTOP1	;76800 baud

BD192	set	TRUE	;19200 baud supported
BD384	set	TRUE	;38400 baud supported
BD768	set	TRUE	;76800 baud supported

TSBAUD	set	TRUE	;software T-port baud rate control
T5511	set	TRUE	;Transfer port baud rates via TMS5511

TSTOPS	set	TSTOP2
 endif ;TTUART or TCFDCS

  if (TTUART or TCFDCS) and ((0-TBAUD) SHR 15)
TSTOPS	set	TSTOP1
 endif ;(TTUART or TCFDCS) and ((0-TBAUD) SHR 15)

;-------------------------------------------------
;Electronic Control Technology (ECT) R2I/O Equates
;
;The ECT R2I/O is built around three TMS6011
;generic UARTS with jumper-selectable baud rates.
;-------------------------------------------------
 if CER2IO
CSTAT	equ	CBASE	;Console status
CDATA	equ	CBASE+1	;Console data

CRXRDY	equ	01h	;Console Rx data buffer full
CTXRDY	equ	80h	;Console Tx data buf available

CISTAT	set	TRUE	;Status bits are active-low
 endif ;CER2IO

 if TER2IO
TPORT	set	TRUE	;Transfer port is defined

TSTAT	equ	TBASE	;Transfer Port status
TDATA	equ	TBASE+1	;Transfer Port data

TRXRDY	equ	01h	;Transfer port Rx data buffer full
TTXRDY	equ	80h	;Transfer port Tx data buf available

TISTAT	set	TRUE	;Status bits are active-low
 endif ;TER2IO

;--------------------------------------------------------------
;Heathkit H8-4 Serial Port Equates
;
; The Heathkit H8-4 has 4 independent, seperately addressable
; serial ports, each built around a National Semiconductor
; INS8250 UART.
;
; CPU Interrupts must be enabled so that the Heathkit H8's
; front panel will work correctly. The H8-4 may or may not be
; connected to one of the interrupt lines. However, this code
; disables the H8-4's interrupts in software, and polls its
; UART(s) as with most other supported ports.
;--------------------------------------------------------------

;H8-4 serial port as the Console

 if CH84
C8250	set	TRUE
CB8250	equ	CBASE
 endif ;CH84

;H8-4 serial Port As the Transfer Port

 if TH84
T8250	set	TRUE
TB8250	equ	TBASE
 endif ;TH84

;--------------------------------------------------------------
;Heathkit H8-5 Serial Port Equates
;
; The H8-5 is built around an Intel 8251 (not the 8251A) UART,
; which has some peculiarities when operating with interrupts.
; In particular, disabling the transmitter (which is the only
; way to disable the transmit interrupt) will stop transmitting
; mid-byte, if the transmitter is busy. This makes the transmit
; interrupt pretty worthless. Heathkit's recommended jumpers
; only enable interrupts for receive data, not transmit. I
; suspect it is for this reason.
;
; Memon/80 expects the H8-5 to be jumpered the standard way
; (per the H8-5 manual):
;
;  Port Select            Serial Interrupt Select   
;     T-3            (Jumper group labeled "SERIAL INT. S")
;     W-7                        RXR
;     X-2                       S-/I3
;     Y-0                       INT ON
;
; Interrupts must be enabled so that the Heathkit H8's front
; panel will work correctly. The H8-5 must be connected to
; Interrupt level 3 so that other H8 software will work. There
; is no way in software to disable the h8-5's interrupt, so
; Memon/80 must use interrupts for receiving data from the
; H8-5. Memon/80 assumes that either the PAM-8 or the XCON-8
; ROM is installed in the H8 computer and maps the interrupts
; to the standard PAM-8 vector addresses.
;--------------------------------------------------------------
 if CH85
C8251	set	TRUE	;8251 UART
CRXINT	set	TRUE	;Interrupt-driven receive (only)
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CH85

 if TH85
TPORT	set	TRUE	;Transfer port is defined
T8251	set	TRUE	;8251 UART
TRXINT	set	TRUE	;Interrupt-driven receive (only)
TB8251	equ	CBASE	;Base of T-port 8251 registers
 endif ;TH85

 if CH85 or TH85
P8INT3	equ	2025h	;Address of PAM-8/XCON-8 Int 3 vector 

 endif ;CH85 or TH85

;-----------------------------------------------------
;IMSAI SIO-2 I/O Equates
;
;The SIO-2 is built around a pair of 8251A UARTs
;with a jumper-selectable baud rate generator and
;logic to provide an interrupt-control port.
;The SIO-2 can be jumpered for either memory-mapped
;I/O or port-mapped I/O. this assumes port-mapped I/O.
;-----------------------------------------------------
 if CISIO2A or CISIO2B
C8251	set	TRUE	;8251 UART
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CISIO2A or CISIO2B

 if TISIO2A or TISIO2B
TPORT	set	TRUE	;Transfer port is defined
T8251	set	TRUE	;8251 UART
TB8251	equ	TBASE	;Base of T-port 8251 registers
 endif ;TISIO2A or TISIO2B

 if CISIO2A or TISIO2A
SIOCTL	equ	CBASE+6	;SIO-2 int control port
 endif ;CISIO2A or TISIO2A

 if CISIO2B and not TISIO2A
SIOCTL	equ	CBASE+2	;SIO-2 int control port
 endif ;CISIO2B and not TISIO2A

 if TISIO2B and not CISIO2A
SIOCTL	equ	TBASE+2	;SIO-2 int control port
 endif ;TISIO2B and not CISIO2A

;-----------------------------------------------------
;Ithaca Intersystems Series II VIO Serial Port Equates
;
;The two serial ports on the Series II VIO are built
;around Signetics 2651 UARTS, which have internal baud
;rate generators.
;-----------------------------------------------------
 if CIVIO2 or TIVIO2

IVSTOP1	equ	04E00h	;1 stop bit
IVSTOP2	equ	0CE00h	;2 stop bits

CTL2651	equ	27h	;Ctrl reg initialization value
 endif ;CIVIO2 or TIVIO2

 if CIVIO2
CDATA	equ	CBASE
CSTAT	equ	CBASE+1
CMODE	equ	CBASE+1
CCTRL	equ	CBASE+3

CTXRDY	equ	01h	;Console transmitter ready
CRXRDY	equ	02h	;Console receiver ready

;Console Port 2651 baud rates

CBD110	equ	IVSTOP2 or 32h	;110 baud (2 stop bits)
CBD150	equ	IVSTOP1 or 34h	;150 baud
CBD300	equ	IVSTOP1 or 35h	;300 baud
CBD600	equ	IVSTOP1 or 36h	;600 baud
CBD1200	equ	IVSTOP1 or 37h	;1200 baud
CBD2400	equ	IVSTOP1 or 3Ah	;2400 baud
CBD4800	equ	IVSTOP1 or 3Ch	;4800 baud
CBD9600	equ	IVSTOP1 or 3Eh	;9600 baud
CBD192	equ	IVSTOP1 or 3Fh	;19200 baud
CBD384	equ	0		;38400 baud (Not supported)
CBD576	equ	0 		;57600 baud (not supported)
CBD768	equ	0		;76800 baud (Not supported)

CSBAUD	set	TRUE	;software Console baud control

 endif ;CIVIO2

 if TIVIO2
TPORT	set	TRUE	;Transfer port is defined

TDATA	equ	TBASE
TSTAT	equ	TBASE+1
TMODE	equ	TBASE+1
TCTRL	equ	TBASE+3

BRATE0	equ	TMODE	;Low byte of baud rate goes here
BRATE1	equ	TMODE	;High byte of baud rate goes here

TTXRDY	equ	01h	;Transfer Port transmitter ready
TRXRDY	equ	02h	;Transfer Port receiver ready

;Transfer Port 2651 baud rates
;(The high byte of these words gets written first)

TBD110	equ	IVSTOP2 or 32h	;110 baud (2 stop bits)
TBD150	equ	IVSTOP1 or 34h	;150 baud
TBD300	equ	IVSTOP1 or 35h	;300 baud
TBD600	equ	IVSTOP1 or 36h	;600 baud
TBD1200	equ	IVSTOP1 or 37h	;1200 baud
TBD2400	equ	IVSTOP1 or 3Ah	;2400 baud
TBD4800	equ	IVSTOP1 or 3Ch	;4800 baud
TBD9600	equ	IVSTOP1 or 3Eh	;9600 baud
TBD192	equ	IVSTOP1 or 3Fh	;19200 baud
TBD384	equ	0		;38400 baud (Not supported)
TBD576	equ	0 		;57600 baud (not supported)
TBD768	equ	0		;76800 baud (Not supported)

BD192	set	TRUE	;19200 baud supported

TSBAUD	set	TRUE	;software transfer port
			;..baud rate control
 endif ;TIVIO2

;-----------------------------------------------------
;Jade Serial/Parallel I/O Port Equates
;
;The serial port of the Jade Serial/Parallel board are
;built around the AY61013A or TR1602B "generic" UART.
;-----------------------------------------------------
 if CJADSP or TJADSP
JRESET	equ	10110000B	;UART reset value
 endif ;CJADSP or TJADSP

 if CJADSP
CDATA	equ	CBASE
CSTAT	equ	CBASE or 80h
CCTRL	equ	CBASE or 80h

CTXRDY	equ	80h	;Console transmitter ready
CRXRDY	equ	10h	;Console receiver ready
 endif ;CJADSP

 if TJADSP
TPORT	set	TRUE	;Transfer port is defined

TDATA	equ	TBASE
TSTAT	equ	TBASE or 80h
TCTRL	equ	TBASE or 80h

TTXRDY	equ	80h	;T-port transmitter ready
TRXRDY	equ	10h	;T-port receiver ready
 endif ;TJADSP

;--------------------------------------------------
;Micromation Doubler Serial Port Equates
;
;The serial port on the Doubler is built around the
;Intel 8251 UART, with jumper-selectable baud rates.
;This UART is accessed via memory accesses, rather
;than the usual I/O accesses.
;---------------------------------------------------
 if CMDUBLR
C8251	set	TRUE	;8251 UART
CMEMAP	set	TRUE	;memory-mapped I/O

CDATA	equ	CBASE+0602h	;Console data port
CSTAT	equ	CBASE+060Ah	;Console status port
CCTRL	equ	CBASE+060Ah	;Console control port

CTXRDY	  equ	  01h	;Console transmitter ready
CRXRDY	  equ	  02h	;Console receiver ready

 endif ;CMDUBLR

 if TMDUBLR
TPORT	set	TRUE	;Transfer port is defined
T8251	set	TRUE	;8251 UART
TMEMAP	set	TRUE	;memory-mapped I/O

TDATA	equ	TBASE+0602h	;Transfer data port
TSTAT	equ	TBASE+060Ah	;Transfer status port
TCTRL	equ	TBASE+060Ah	;Transfer control port

TTXRDY	  equ	  01h	;Transfer transmitter ready
TRXRDY	  equ	  02h	;Transfer receiver ready

 endif ;TMDUBLR

;--------------------------------------------------
;Tarbell 4044 4P2S Serial Port Equates
;
;The Tarbell 4044 is built around four Intel 8251A
;UARTs, with switch-selectable baud rates.
;--------------------------------------------------
 if CTARBL
C8251	set	TRUE	;8251 UART
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CTARBL

 if TTARBL
TPORT	set	TRUE	;Transfer port is defined

T8251	set	TRUE	;8251 UART
TB8251	equ	TBASE	;Base of T-port 8251 registers
 endif ;TTARBL

;------------------------------------------------------
;Salota I/O 2+1 Serial Port Equates
;
;The Salota I/O 2+1's serial ports are built around two
;Intel 8251A UARTs, with jumper-selectable baud rates.
;------------------------------------------------------
 if CSAL21
C8251	set	TRUE	;8251 UART
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CSAL21

 if TSAL21
TPORT	set	TRUE	;Transfer port is defined

T8251	set	TRUE	;8251 UART
TB8251	equ	TBASE	;Base of T-port 8251 registers
 endif ;TSAL21

;--------------------------------------------------
;SOlid State Music IO5 Serial Port Equates
;
;The IO5's serial ports are built around two Intel
;8251A UARTs, with switch-selectable baud rates.
;--------------------------------------------------
 if CSIO5
C8251	set	TRUE	;8251 UART
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CSIO5

 if TSIO5
TPORT	set	TRUE	;Transfer port is defined

T8251	set	TRUE	;8251 UART
TB8251	equ	TBASE	;Base of T-port 8251 registers
 endif ;TSIO5

;---------------------------------------------------
;Processor Technology 3P+S Serial Port Equates
;
;The 3P+S is built around a TMS6011 or AMI S1883
;UART, with a jumper selectable baud rate generator.
;This board has many jumpers on it, allowing you to
;set up the bits within the control port any way you
;like. Memon/80 assumes it is set up the way that
;Processor Technology'S CUTER expects it to be.
;---------------------------------------------------
 if CPT3PS
CSTAT	equ	CBASE	;Console status
CCTRL	equ	CBASE	;Console control
CDATA	equ	CBASE+1	;Console data

CTXRDY	  equ	  80h	;transmitter buffer empty
CRXRDY	  equ	  40h	;receive data available

 endif ;CPT3PS

 if TPT3PS
TPORT	set	TRUE	;Transfer port is defined

TSTAT	equ	TBASE	;Transfer Port status
TCTRL	equ	TBASE	;Transfer Port control
TDATA	equ	TBASE+1	;Transfer Port data

TTXRDY	  equ	  80h	;transmitter buffer empty
TRXRDY	  equ	  40h	;receive data available
 endif ;TPT3PS

;-----------------------------------------------
;Vector Graphic BitStreamer Equates
;
;The BitStreamer is built around an 8251 UART,
;with a jumper selectable baud rate generator.
;-----------------------------------------------
 if CBITS1
C8251	set	TRUE	;8251 UART
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CBITS1

 if TBITS1
TPORT	set	TRUE	;Transfer port is defined

T8251	set	TRUE	;8251 UART
TB8251	equ	TBASE	;Base of T-port 8251 registers
 endif ;TBITS1

;---------------------------------------------------
;Vector Graphic BitStreamer II Equates
;
;The BitStreamer II is built around three 8251A
;UARTs with a jumper-selectable baud rate generator.
;---------------------------------------------------
 if CBITS2
C8251	set	TRUE	;8251 UART
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CBITS2

 if TBITS2
TPORT	set	TRUE	;Transfer port is defined

T8251	set	TRUE	;8251 UART
TB8251	equ	TBASE	;Base of T-port 8251 registers
 endif ;TBITS2

;-------------------------------------------------------
;Wameco IOB-1 Equates
;
;The serial port of the Wameco IOB-1 is built around
;the Intel 8251 UART with a switch-selectable baud rate.
;-------------------------------------------------------
 if CWIOB1
C8251	set	TRUE	;8251 UART
CB8251	equ	CBASE	;Base of Console 8251 registers
 endif ;CWIOB1

 if TWIOB1
TPORT	set	TRUE	;Transfer port is defined

T8251	set	TRUE	;8251 UART
TB8251	equ	TBASE	;Base of T-port 8251 registers
 endif ;TWIOB1

;---------------------------
;General MC6850 ACIA Equates
;---------------------------
 if C6850 or T6850
;MOTOROLA 6850 ACIA control/status values
U68RES	equ	00000011B	;master reset
U68ST2	equ	00010001B	;2 stop bits, /16
U68ST1	equ	00010101B	;1 stop bit, /16
 endif ;C6850 or T6850

; MC6850 as the Console Port

 if C6850
CCTRL	equ	CB6850		;ACIA 0 control output port
CSTAT	equ	CB6850		;ACIA 0 status input port
CDATA	equ	CB6850+1	;ACIA 0 Rx & Tx data regs

CRXRDY	equ	00000001B	;Console Rx data reg full
CTXRDY	equ	00000010B	;Console Tx data reg empty
 endif ;C6850

;MC6850 as the Transfer Port

 if T6850
TCTRL	equ	TB6850		;ACIA 1 control output port
TSTAT	equ	TB6850		;ACIA 1 status input port
TDATA	equ	TB6850+1	;ACIA 1 Rx & Tx data regs

TRXRDY	equ	00000001B	;Transfer Port Rx data reg full
TTXRDY	equ	00000010B	;Tr. Port Tx data reg empty
 endif ;T6850

;----------------------------
;General INS8250 UART Equates
;----------------------------

;INS8250 UART as the Console Port

 if C8250

CRXRDY	equ	00000001b	;Rx data ready
CTXRDY	equ	00100000b	;Tx buffer empty
CDLAB	equ	80h		;Baud rate access

CSBAUD	set	TRUE	;software Console
			;..baud rate control
;Console Port baud rates

CBD110	equ	1047	;0= 110 baud (2 stop bits)
CBD150	equ	768	;1= 150 baud
CBD300	equ	384	;2= 300 baud
CBD600	equ	192	;3= 600 baud
CBD1200	equ	96	;4= 1200 baud
CBD2400	equ	48	;5= 2400 baud
CBD4800	equ	24	;6= 4800 baud
CBD9600	equ	12	;7= 9600 baud
CBD192	equ	6	;8= 19200 baud
CBD384	equ	3	;9= 38400 baud
CBD576	equ	0	;a= 57600 baud (not supported)
CBD768	equ	0	;b= 76800 baud (not supported)

CSTOP1	equ	03h	;1 stop bit
CSTOP2	equ	07h	;2 stop bits

CDATA	equ	CB8250	;data port
CSINTEN	equ	CDATA+1	;interrupt enable port
CSIDENT	equ	CDATA+2	;interrupt ID port
CSLCTRL	equ	CDATA+3	;line control port
CSMDMCT	equ	CDATA+4	;modem control port
CSTAT	equ	CDATA+5	;line status port
CMSTAT	equ	CDATA+6	;modem status port

CSTOPS	set	CSTOP2
 endif ;C8250

 if C8250 and ((0-CBAUD) SHR 15)
CSTOPS	set	CSTOP1
 endif ;CC8250 and ((0-CBAUD) SHR 15)

;INS8250 UART as the Transfer Port

 if T8250

TRXRDY	equ	00000001b	;Rx data ready
TTXRDY	equ	00100000b	;Tx buffer empty
TDLAB	equ	80h		;Baud rate access

TPORT	set	TRUE	;Transfer port is defined

TSBAUD	set	TRUE	;software Transfer Port
			;..baud rate control

;Transfer Port baud rates

TBD110	equ	1047	;0= 110 baud (2 stop bits)
TBD150	equ	768	;1= 150 baud
TBD300	equ	384	;2= 300 baud
TBD600	equ	192	;3= 600 baud
TBD1200	equ	96	;4= 1200 baud
TBD2400	equ	48	;5= 2400 baud
TBD4800	equ	24	;6= 4800 baud
TBD9600	equ	12	;7= 9600 baud
TBD192	equ	6	;8= 19200 baud
TBD384	equ	3	;9= 38400 baud
TBD576	equ	0	;a= 57600 baud (not supported)
TBD768	equ	0	;b= 76800 baud (not supported)

BD192	set	TRUE	;19200 baud supported
BD384	set	TRUE	;38400 baud supported
TSTOP1	equ	03h	;1 stop bit
TSTOP2	equ	07h	;2 stop bits

TDATA	equ	TB8250	;data port
TSINTEN	equ	TDATA+1	;interrupt enable port
TSIDENT	equ	TDATA+2	;interrupt ID port
TSLCTRL	equ	TDATA+3	;line control port
TSMDMCT	equ	TDATA+4	;modem control port
TSTAT	equ	TDATA+5	;line status port
TMSTAT	equ	TDATA+6	;modem status port

BRATE0	equ	TDATA	;Baud rate divisor low byte
BRATE1	equ	TSINTEN	;Baud rate divisor high byte

TSTOPS	set	TSTOP2
 endif ;T8250

 if T8250 and ((0-TBAUD) SHR 15)
TSTOPS	set	TSTOP1
 endif ;T8250 and ((0-TBAUD) SHR 15)

;-------------------------------
;General 8251/8251A UART Equates
;-------------------------------

;Port Relative Addresses and Ready Bits for I/O-mapped 8251s

 if C8251 and (not CMEMAP)
CDATA	equ	CB8251		;Console data
CSTAT	equ	CB8251+1	;Console status
CCTRL	equ	CB8251+1	;Console control

CTXRDY	  equ	  01h	  ;Console transmitter ready
CRXRDY	  equ	  02h	  ;receiver ready
CTXMTY	  equ	  04h	  ;Console Transmitter empty
 endif ;C8251 and (not CMEMAP)

 if T8251 and (not TMEMAP)
TDATA	equ	TB8251		;Transfer port data
TSTAT	equ	TB8251+1	;Transfer port status
TCTRL	equ	TB8251+1	;Transfer port control

TTXRDY	  equ	  01h	  ;Transfer port transmitter ready
TRXRDY	  equ	  02h	  ;Transfer port receiver ready
TTXMTY	  equ	  04h	  ;Transfer port Transmitter empty
 endif ;T8251 and (not TMEMAP)

;initialization values for all 8251s

 if C8251 or T8251
UMOD16	  equ	  4Eh	  ;8 bits, 1 stop, no parity, X16
UCMDRE	  equ	  40h	  ;reset UART
UCMDEN	  equ	  37h	  ;enable Tx & Rx, DTR, RTS ON
UCMDEI	  equ	  36h	  ;enable Rx, disable Tx

;8251 reset sequence
;note: code assumes UCMDEN is UNIQUE here

SI21	equ	0AAh	;Fake SYNC chr
SI22	equ	UCMDRE	;reset command
SI23	equ	UMOD16	;Clock divisor, etc.
SI24	equ	UCMDEN	;enable Rx & Tx
 endif ;C8251 or T8251

;---------------------------
;Compute Baud Rate Constants
;---------------------------
 if CSBAUD
CPBAUD	set	CBD110
 endif ;CSBAUD

 if CSBAUD and ((0-CBAUD)SHR 15)
CPBAUD	set	CBD150
 endif ;CSBAUD and ((0-CBAUD)SHR 15)

 if CSBAUD and ((1-CBAUD)SHR 15)
CPBAUD	set	CBD300
 endif ;CSBAUD and ((1-CBAUD)SHR 15)

 if CSBAUD and ((2-CBAUD)SHR 15)
CPBAUD	set	CBD600
 endif ;CSBAUD and ((2-CBAUD)SHR 15)

 if CSBAUD and ((3-CBAUD)SHR 15)
CPBAUD	set	CBD1200
 endif ;CSBAUD and ((3-CBAUD)SHR 15)

 if CSBAUD and ((4-CBAUD)SHR 15)
CPBAUD	set	CBD2400
 endif ;CSBAUD and ((4-CBAUD)SHR 15)

 if CSBAUD and ((5-CBAUD)SHR 15)
CPBAUD	set	CBD4800
 endif ;CSBAUD and ((5-CBAUD)SHR 15)

 if CSBAUD and ((6-CBAUD)SHR 15)
CPBAUD	set	CBD9600
 endif ;CSBAUD and ((6-CBAUD)SHR 15)

 if CSBAUD and ((7-CBAUD)SHR 15)
CPBAUD	set	CBD192
 endif ;CSBAUD and ((7-CBAUD)SHR 15)

 if CSBAUD and ((8-CBAUD)SHR 15)
CPBAUD	set	CBD384
 endif ;CSBAUD and ((8-CBAUD)SHR 15)

 if CSBAUD and ((9-CBAUD)SHR 15)
CPBAUD	set	CBD576
 endif ;CSBAUD and ((9-CBAUD)SHR 15)

 if CSBAUD and ((0Ah-CBAUD)SHR 15)
CPBAUD	set	CBD768
 endif ;CSBAUD and ((0Ah-CBAUD)SHR 15)

 if TSBAUD
TPBAUD	set	TBD110
 endif ;TSBAUD

 if TSBAUD and ((0-TBAUD)SHR 15)
TPBAUD	set	TBD150
 endif ;TSBAUD and ((0-TBAUD)SHR 15)

 if TSBAUD and ((1-TBAUD)SHR 15)
TPBAUD	set	TBD300
 endif ;TSBAUD and ((1-TBAUD)SHR 15)

 if TSBAUD and ((2-TBAUD)SHR 15)
TPBAUD	set	TBD600
 endif ;TSBAUD and ((2-TBAUD)SHR 15)

 if TSBAUD and ((3-TBAUD)SHR 15)
TPBAUD	set	TBD1200
 endif ;TSBAUD and ((3-TBAUD)SHR 15)

 if TSBAUD and ((4-TBAUD)SHR 15)
TPBAUD	set	TBD2400
 endif ;TSBAUD and ((4-TBAUD)SHR 15)

 if TSBAUD and ((5-TBAUD)SHR 15)
TPBAUD	set	TBD4800
 endif ;TSBAUD and ((5-TBAUD)SHR 15)

 if TSBAUD and ((6-TBAUD)SHR 15)
TPBAUD	set	TBD9600
 endif ;TSBAUD and ((6-TBAUD)SHR 15)

 if TSBAUD and ((7-TBAUD)SHR 15)
TPBAUD	set	TBD192
 endif ;TSBAUD and ((7-TBAUD)SHR 15)

 if TSBAUD and ((8-TBAUD)SHR 15)
TPBAUD	set	TBD384
 endif ;TSBAUD and ((8-TBAUD)SHR 15)

 if TSBAUD and ((9-TBAUD)SHR 15)
TPBAUD	set	TBD576
 endif ;TSBAUD and ((9-TBAUD)SHR 15)

 if TSBAUD and ((0Ah-TBAUD)SHR 15)
TPBAUD	set	TBD768
 endif ;TSBAUD and ((0Ah-TBAUD)SHR 15)

;******************************************************
;Compute Memon80's RAM beginning for the banner message
;******************************************************

 if not (TRXINT or CRXINT)
RAMBEG	equ	STACK-STKSIZ-1
 endif ;not (TRXINT or CRXINT)


 if TRXINT or CRXINT
RAMBEG	equ	TRXBUF
 endif ;TRXINT or CRXINT

;***************************
;Disk Controller Definitions
;***************************
;Initialize some assembler vasriables
BOOTER	set	FALSE		;Assume no BO command for now
JMPBOT	set	FALSE
WD179X	set	FALSE

;-----------------------------------------------------------
;Altair 8800 Floppy Disk controller Equates (These are the
;same for the 88-DCDD controller and the 88-MDS controller.)
;-----------------------------------------------------------
 if A88DCD or A88MCD
BOOTER	set	TRUE		;Boot command exists

DENABL	equ	DIBASE		;Drive enable output
DDISBL	  equ	  80h		  ;disable disk controller

DSTAT	equ	DIBASE		;status input (active low)
ENWDAT	  equ	  01h		  ;-enter write data
MVHEAD	  equ	  02h		  ;-Move Head OK
HDSTAT	  equ	  04h		  ;-Head status
DRVRDY	  equ	  08h		  ;-Drive Ready
INTSTA	  equ	  20h		  ;-interrupts enabled
TRACK0	  equ	  40h		  ;-Track 0 detected
NRDA	  equ	  80h		  ;-new Read data Available

DCTRL	equ	DIBASE+1	;Drive control output
STEPIN	  equ	  01h		  ;Step-In
STEPOT	  equ	  02h		  ;Step-Out
HEDLOD	  equ	  04h		  ;8" disk: load head
				  ;Minidisk: restart
				  ;..6.4 S timer
HDUNLD	  equ	  08h		  ;unload head (8" only)
IENABL	  equ	  10h		  ;enable sector interrupt
IDSABL	  equ	  20h		  ;Disable interrupts
WENABL	  equ	  80h		  ;enable drive write circuits

DSECTR	equ	DIBASE+1	;Sector position input
SVALID	  equ	  01h		  ;Sector valid (1st 30 uS
				  ;..of sector pulse)
SECMSK	  equ	  3Eh		  ;Sector mask for MDSEC

DDATA	equ	DIBASE+2	;Disk data (input/output)

;Floppy Disk Parameters

BPS	equ	128		;data bytes/sector
MDSPT	equ	16		;Minidisk sectors/track
				;this code assumes SPT for 8"
				;disks=MDSPT * 2.

HDRSIZ	equ	3		;header bytes before data
TLRSIZ	equ	2		;trailer bytes read after data

SECSIZ	equ	BPS+HDRSIZ+TLRSIZ ;total bytes/sector

MAXTRYS	equ	16		;max retries per sector

;sector buffer component offsets
SFSIZE	equ	RAMBUF+1	;file size
SDATA	equ	RAMBUF+HDRSIZ	;sector data
SMARKR	equ	SDATA+BPS	;marker byte
SCKSUM	equ	SMARKR+1	;checksum byte
 endif ;A88DCD or A88MCD

;------------------------
;CCS 2422 Disk controller
;------------------------
 if CC2422

BOOTER	set	TRUE		;Boot command exists
WD179X	set	TRUE		;WD1793-type controller

MAXTRYS	equ	16		;max retries
SECSIZ	equ	128		;bytes/sector
BOTDSK	equ	0		;Boot disk
BOTSEC	equ	1		;Boot sector
BOTADR	equ	80h		;Load & Run boot sector here
				;Code assumes xx80h

;CCS 2422 floppy disk controller addresses

BCTRL	equ	4		;Control port 2
BSTAT	equ	4		;status port 2
DSTAT	equ	30h		;disk status port
DCMMD	equ	DSTAT		;disk command port
DTRCK	equ	DSTAT+1		;track port
DSCTR	equ	DSTAT+2		;sector port
DDATA	equ	DSTAT+3		;data port
DFLAG	equ	DSTAT+4		;disk flag port
DCTRL	equ	DSTAT+4		;disk control port

;Control register 1 (DCTRL) output bits

DCDS1	equ	00000001b	;drive select 1
DCDS2	equ	00000010b	;drive select 2
DCDS3	equ	00000100b	;drive select 3
DCDS4	equ	00001000b	;drive select 4
DCMAXI	equ	00010000b	;0=Minidisk, 1=8" disk
DCMDMO	equ	00100000b	;Minidisk motor on
DCDDEN	equ	01000000b	;double density enable
DCAUTOW	equ	10000000b	;autowait

;Control register 2 (BCTRL) output bits

BCEJECT	equ	00000100b	;remote eject control (PerSci)
BCFSEEK	equ	00010000b	;fast seek: voicecoil drives
BCSIDE0	equ	01000000b	;0=side 1, 1=side 0
BCROMEN	equ	10000000b	;enable onboard ROM

;Composite BCTRL values used to select disk side
; Note: ROM is actually disabled via the page register
; Note: fast seek disabled via hardware jumper

SIDE0	equ	BCROMEN+BCFSEEK+BCSIDE0
SIDE1	equ	BCROMEN+BCFSEEK

;Status register 1 (DFLAG) input bits

DFINTRQ	equ	00000001b	;WD1793 INTRQ signal,
				;active high
DFDS1	equ	00000010b	;DS1 in control reg 1
DFDS2	equ	00000100b	;DS2 in control reg 1
DFDS3	equ	00001000b	;DS3 in control reg 1
DFDS4	equ	00010000b	;DS4 in control reg 1
DFSHLD	equ	00100000b	;WD1793 HLD signal, active high
DFAUTOB	equ	01000000b	;autoboot jumper, 0=autoboot
DFSDRQ	equ	10000000b	;WD1793 DRQ signal, 1=ready

;Status reg 2 (BSTAT) input bits

BTRK0	equ	00000001b	;when on track 0: 1=8" disk
BMINI	equ	00000010b	;-DCTRL reg bit: 1=Minidisk
BWPRT	equ	00000100b	;0=disk is write-protected
BSIDE0	equ	00001000b	;SIDE0 bit in control reg 2
BINDEX	equ	00010000b	;index signal, active low
BDDEN	equ	00100000b	;DDEN signal in ctrl register 1
BSIDED	equ	01000000b	;0=selected disk is 2-sided

 endif ;CC2422

;---------------------------------
;Cromemco 4FDC/16FDC/64FDC Equates
;---------------------------------
 if C4FDC or C16FDC or C64FDC

BOOTER	set	TRUE		;Boot command exists
WD179X	set	TRUE		;WD1793-type controller

MAXDRV	equ	3		;drives 0-3 allowed
BOTADR	equ	0080h		;boot and run address
				;(code assumes xx80h)
BOTSEC	equ	1		;boot sector
MAXTRYS	equ	10		;boot retries

;16FDC Ports & Bits

ASTAT	equ	04h		;Aux stat input
ACTRL	equ	04h		;Auxiliary cmd output
DSTAT	equ	30h		;Status input
DCMMD	equ	30h		;cmd output
DTRCK	equ	31h		;Track I/O
DSCTR	equ	32h		;Sector I/O
DDATA	equ	33h		;Data I/O
DFLAG	equ	34h		;Flags input
DCTRL	equ	34h		;Control output

;DCTRL output bits

DCDS0	equ	00000001B ;Drive Select 0
DCDS1	equ	00000010B ;Drive Select 1
DCDS2	equ	00000100B ;Drive Select 2
DCDS3	equ	00001000B ;Drive Select 3
DCSMSK	equ	11110000B ;Drive Select mask
DCMAXI	equ	00010000B ;0=Mini, 1=Maxi
DCMOTO	equ	00100000B ;Motor On
DCMMSK	equ	11011111B ;Motor mask
DCDDEN	equ	01000000B ;Enable Double Density
DCAUTO	equ	10000000B ;Enable auto-wait

;ACTRL output bits (These are active-low)

ACSID0	 equ	00000010B ;-Side select
ACCTRL	 equ	00000100B ;-Control Out
ACRSTR	 equ	00001000B ;-Restore
ACFSEK	 equ	00010000B ;-Fast Seek
ACDSO	 equ	00100000B ;-Drive Select Override
ACEJCT	 equ	01000000B ;-EJECT

;DFLAG input bits

DFSEOJ  equ	00000001B ;Disk cmd complete
DFAWTO	equ	00000010B ;Auto-wait timeout
DFSMTO	equ	00000100B ;Motor timeout
DFSMON	equ	00001000B ;1793 is requesting motors on
DSFhLD	equ	00100000B ;1793 is requesting head load
DSFDRQ	equ	10000000B ;1793 is requesting data

;ASTAT bits

DASSIP	equ	01000000B ;Seek in progress
DASDRQ	equ	10000000B ;Data Request

 endif ;C4FDC or C16FDC or C64FDC

;-------------------------
;IMSAI FIF Disk Controller
;-------------------------
 if IFIF

BOOTER	set	TRUE		;Boot command exists

MAXTRYS	equ	16		;max retries

;Disk Command Port

IDISK	equ	DIBASE

;Command Port Commands

IEXEC0	equ	0		;Execute string command 0
ISSPTR	equ	10h		;Set string pointer 0
IRESTR	equ	21h		;Reestore to track 0

;RAM string commands

IRDSEC	equ	21h		;Read sector

;RAM location for command strings

IBCMD	equ	80h		;Beginning of disc command string
				;..and command byte therEOF
IBSTAT	equ	IBCMD+1		;Status byte
IBTRK	equ	IBCMD+2		;Track (2 bytes)
IBSECT	equ	IBCMD+4		;Sector
IBUFAD	equ	IBCMD+5		;Buffer Address

;Disk status

ISUCCS	equ	01h		;Success

 endif ;IFIF

;------------------------------------------
;Micropolis FD controller B Disk Controller
;------------------------------------------
 if MICROP

BOOTER	set	TRUE		;Boot command exists

FDCMD	equ	DMBASE+200h	;Command register
FDSECT	equ	DMBASE+200h	;Sector register
FDSTAT	equ	DMBASE+201h	;Status register
FDDATA	equ	DMBASE+203h	;Data in/out	

SCLEN 	equ	268	;sector len without sync byte and checksum
PTROFF	equ	010Ah	;offset of load address
			;..pointer within sector 0
EXEOFF	equ	0CH	;offset into sector data for execution

MAXTRYS	equ	16	;max retries

;Sector register is at FDCBASE+0

SMASK	equ	00Fh	;Sector mask
SFLG	equ	080h	;sector start flag
SIFLG	equ	040h	;sector interrupt flag
DTMR	equ	020h	;2mhz/4mhz jumper for timers

;Status register is at FDCBASE+1

TFLG	equ	80h	;transfer flag
INTE	equ	40h	;S-100 INTE signal
RDY	equ	20h	;disk ready
WPT	equ	10h	;write protect
TK0	equ	08h	;track zero
USLT	equ	04h	;unit selected

;Command register at FDCBASE+0 (and again at FDCBASE+1)
;  bits 1-0 command modifier
;  bits 7-5 command

SLUN	equ	020h	;select unit
			;Modifier is unit address

SINT	equ	040h	;set interrupt
			;Modifier=1 enable int
			;  	    0 disable int

STEP	equ	060h	;step carriage
STEPO	equ	000h	;step out modifier
STEPI	equ	001h	;step in modifier

WTCMD	equ	080h	;enable write
			;No modifier used

FRESET	equ	0A0h	;reset FDC
			;No modifier used

;RAM locations that must be initialized for DOS

ROMADR	equ	00A0h	;onboard ROM base address
FDCADR	equ	00A2h	;Save FDC address here for DOS
LODADR	equ	00A4h	;load address of sector
RDSEC	equ	00A6h	;Read Sector subroutine
			;..(once its moved)

RDSECH	equ	RDSEC+3	;enter here with hl=FDC address
			;(entry point used during boot)

 endif ;MICROP

;--------------------------------------------------------
;Northstar MDC-A and MDS-A Single-Density Disk Controller
;--------------------------------------------------------
 if NSTARS

BOOTER	set	TRUE		;Boot command exists

;The Northstar MDC-A and MDS-A disk controllers work by reading
;and writing to memory addresses - no INs or outs occur.
;Commands are encoded into the low byte of the address.

;Disk controller memory space allocation

ROMADR	equ	DMBASE+100h	;Standard ROM address
CTLADR	equ	DMBASE+300h	;Disk Control base address

;controller commands (are at memory addresses)

CTLDS0	  equ	  CTLADR+00h	  ;Select drive 0 (illegal)
CTLDS1	  equ	  CTLADR+01h	  ;Select drive 1
CTLDS2	  equ	  CTLADR+02h	  ;Select drive 2
CTLDS3	  equ	  CTLADR+03h	  ;Select drive 3
CTLWRT	  equ	  CTLADR+04h	  ;Initiate write
CTLSTC	  equ	  CTLADR+08h	  ;Clear track-step f-f
CTLSTS	  equ	  CTLADR+09h	  ;Set track-step f-f
CTLDI	  equ	  CTLADR+0CH	  ;Disarm interrupts
CTLEI	  equ	  CTLADR+0DH	  ;Arm interrupts
CTLNOP	  equ	  CTLADR+10h	  ;No operation
CTLRSF	  equ	  CTLADR+14h	  ;Reset sector flag
CTLRES	  equ	  CTLADR+18h	  ;Reset controller
CTLSTO	  equ	  CTLADR+1CH	  ;select step out
CTLSTI	  equ	  CTLADR+1DH	  ;Select step in
CTLBST	  equ	  CTLADR+20h	  ;Read B status
CTLRD	  equ	  CTLADR+40h	  ;Read Data
CTLMO	  equ	  CTLADR+80h	  ;Drive motors on command

;A-Status Bits

SATR0	  equ	  01h		  ;Track 0
SAWP	  equ	  02h		  ;Write protect
SABDY	  equ	  04h		  ;Sync chr found
SAWRT	  equ	  08h		  ;Ready for write data
SAMO	  equ	  10h		  ;Motor is on
SAWN	  equ	  40h		  ;Status read during window
SASF	  equ 	  80h		  ;Sector flag

;B-Status Bits

SBSECT	  equ	  0Fh		  ;Sector position mask
SBMO	  equ	  10h		  ;Motor is on
SBWN	  equ	  40h		  ;Status read during window
SBSF	  equ 	  80h		  ;Sector flag

;Constants

UNINIT	equ	59h		;uinitialized RAM value
MAXTRYS	equ	10		;Max retries
BDRIVE	equ	1		;Boot drive
				;..(Code assumes this is 01.)
BTRACK	equ	0		;Boot track
BSECTR	equ	4		;Boot sector
LODADR	equ	2000h		;Standard MDS-A load address
EXEADR	equ	2000h		;..and execution address

 endif ;NSTARS

;----------------------------------------------
;Northstar MDS-D Double-Density Disk Controller
;----------------------------------------------
 if NSTARD

BOOTER	set	TRUE		;Boot command exists

;The Northstar MDS-D disk controller works by reading and
;writing to memory addresses - no INs or outs occur.
;Commands are encoded into the low byte of the address.

;-----------------------------
;MDS-D Disk Controller Equates
;-----------------------------
CTLWD	equ	DMBASE+100h	;Write data
CTLORD	equ	DMBASE+200h	;controller Orders
CTLCMD	equ	DMBASE+300h	;controller Commands

;Order Register Bits

ORDDS0	equ	00		;No drive selected
ORDDS1	equ	01h		;Select drive 1
ORDDS2	equ	02h		;Select dribe 2
ORDDS3	equ	04h		;Select drive 3
ORDDS4	equ	08h		;Select drive 4
ORDST	equ	10h		;Step level
ORDSIN	equ	20h		;Step in
ORDSS	equ	40h		;Disk side
ORDDD	equ	80h		;Double-density

;Control register bits

CTLRSF	equ	01h		;reset sector flag
CTLDI	equ	02h		;Disarm interrupt
CTLEI	equ	03h		;Arm interrupt
CTlsb	equ	04h		;Set Body (diag)
CTLMO	equ	05h		;Motors on
CTLWR	equ	06h		;Begin write
CTLRES	equ	07h		;reset controller

CTLAS	equ	10h		;Select A-Status
CTLBS	equ	20h		;Select B-Status
CTLCS	equ	30h		;Select C-Status
CTLRD	equ	40h		;Select read data

;A-Status Bits

SABD	equ	01h		;sync chr detected (body)
SARE	equ	04h		;Read enabled
SAWI	equ	08h		;Sector window
SAMO	equ	10h		;Motor on
SADD	equ	20h		;Double density detected
SAIX	equ	40h		;Index hole on previous sector
SASF	equ	80h		;Sector flag
				;..(cleared by software)

;B-Status Bits

SBT0	equ	01h		;Track 0 detected
SBWP	equ	02h		;Write protected
SBWR	equ	08h		;Write operation in progress
SBMO	equ	10h		;Motor on
SBDD	equ	20h		;Double density detected
SBIX	equ	40h		;Index hole on previous sector
SBSF	equ	80h		;Sector flag
				;..(cleared by software)

;C-Status Bits

SCSM	equ	0Fh		;Sector mask
SCMO	equ	10h		;Motor on
SCDD	equ	20h		;Double density detected
SCIX	equ	40h		;Index hole on previous sector
SCSF	equ	80h		;Sector flag
				;..(cleared by software)

;---------
;Constants
;---------
MAXTRYS	equ	10		;max retries
DDBSCR	equ	4		;Double-density boot sector
SDBSCR	equ	8		;Single-density boot sector

 endif ;NSTARD

;-----------------------------------------
;SD Systems Versafloppy and Versafloppy II
;Single- or Double-Density Disk Controller
;for minidisks or 8" disks
;-----------------------------------------
 if VERSA1 or VERSA2 or SALFDC

BOOTER	set	TRUE		;Boot command exists
WD179X	set	TRUE		;WD179x (or WD177x)

SECSIZ	equ	128		;boot sector size
VLOAD	equ	0800h		;Boot code Load address
VBOOT	equ	0080H		;Boot code exec address
MAXTRYS	equ	16		;Max retries

;Port addresses

VDRSEL	equ	DIBASE+3	;Drive select port
VDCOM	equ	DIBASE+4	;WD1793 Command port
VDSTAT	equ	DIBASE+4	;WD1793 Status port
VTRACK	equ	DIBASE+5	;WD1793 Track port
VSECT	equ	DIBASE+6	;WD1793 Sector Register
VDDATA	equ	DIBASE+7	;WD1793 Data Register

 endif ;VERSA1 or VERSA2 or SALFDC

 if VERSA1
;Versafloppy VDRSEL bit assignments (all active-low)

VDSEL0N	equ	00000001b	;select drive 0
VDSEL1N	equ	00000010b	;select drive 1
VDSEL2N	equ	00000100b	;select drive 2
VDSEL3N	equ	00001000b	;select drive 3
VSIDE1N	equ	00010000b	;Select side 1
VRSTORN	equ	00100000b	;optional Restore
VWAITN	equ	01000000b	;Enable auto-wait circuit
VINTEN	equ	10000000b	;Interrupt Enable
 endif ;VERSA1

 if VERSA2 or SALFDC
;Versafloppy II VDRSEL bit assignments (all active-high)

VDSEL0	equ	00000001b	;select drive 0
VDSEL1	equ	00000010b	;select drive 1
VDSEL2	equ	00000100b	;select drive 2
VDSEL3	equ	00001000b	;select drive 3
VSIDE1	equ	00010000b	;Select side 1
VMINI	equ	00100000b	;Set up for minidisk
VDDEN	equ	01000000b	;Enable double-density
VWAIT	equ	10000000b	;Enable auto-wait circuit
 endif ;VERSA2 or SALFDC

;-------------------------------------------------
;Tarbell Single- or Double-Density Disk Controller
;-------------------------------------------------
 if TARBL1 or TARBL2

BOOTER	set	TRUE		;Boot command exists
WD179X	set	TRUE		;WD179x (or WD177x)

SBOOT	equ	007DH		;Boot code exec address
SLOAD	equ	0		;Code load address
MAXTRYS	equ	16		;Max retries

TDCOM	equ	DIBASE		;WD1793 Command port
TDSTAT	equ	DIBASE		;WD1793 Status port
TTRACK	equ	DIBASE+1	;WD1793 Track port
TSECT	equ	DIBASE+2	;WD1793 Sector Register
TDDATA	equ	DIBASE+3	;WD1793 Data Register
TWAIT	equ	DIBASE+4	;Wait Input Register
 endif ;TARBL1 or TARBL2

 if TARBL1
TEXTP	equ	DIBASE+4h	;Extended Output Port

;Extended Output Port bits

TPADE32	equ	00000000b	;Write to option pad E-32
TOUTSO	equ	00000001b	;Write to SOn line
TDSEL	equ	00000010b	;Write to Drive Select latch
TDSEL1N	equ	00010000b	;Drive Select 1, active low
TDSEL2N	equ	00100000b	;Drive Select 2, active low
TDSEL3N	equ	01000000b	;Drive Select 3, active low
TDSEL4N	equ	10000000b	;Drive Select 4, active low

TSEL0	equ	TDSEL + (TDSEL1N xor 0F0h) ;select drive 0	
 endif ;TARBL1

 if TARBL2
TEXTP0	equ	DIBASE+4h	;Extended Output Port 0
TEXTP1	equ	DIBASE+5h	;Ext. Out Port 1 (bank select)

;Extended Output Port 0 bits

TDDEN	equ	00001000b	;Double density enable
TDSEL	equ	00110000b	;Drive select mask
TSIDE1	equ	01000000b	;Side 1 select
 endif ;TARBL2

;-------------------------------------------
;Common Western digital WD179x Constants
;(Also correct for WD177x-type controllers.)
;-------------------------------------------
 if WD179X

;WD1793 Commands

RSTRCM	equ	00000000B ;(I)Restore
SEEKCM	equ	00010000B ;(I)Seek
STEPCM	equ	00100000B ;(I)Step
STPICM	equ	01000000B ;(I)Step In
STPOCM	equ	01100000B ;(I)Step Out
RSECCM	equ	10000000B ;(II)Read Record
WSECCM	equ	10100000B ;(II)Write Record
RADRCM	equ	11000000B ;(III)Read addr
RTRKCM	equ	11100000B ;(III)Read track
WTRKCM	equ	11110000B ;(III)Write track
FINTCM	equ	11010000B ;(IV)Force int

;1793 Command Options (1st letter matches 1793 spec)

RS03MS	equ	00000000b ;(I) 3 mS step rate
RS06MS	equ	00000001b ;(I) 6 mS step rate
RS10MS	equ	00000010b ;(I) 10 mS step rate
RS15MS	equ	00000011b ;(I) 15 mS step rate
VERTRK	equ	00000100b ;(I) verify on destination track
HDLOAD	equ	00001000b ;(I) head load
UPDTTR	equ	00010000b ;(I) update track reg
F1SCMP	equ	00000010b ;(II)enable side compare
F2SCS1	equ	00001000b ;(II)side compare, side 1
MULTCM	equ	00010000b ;(II) read/write multiple
ED15MS	equ	00000100b ;(II & III) settling delay
A0DDM	equ	00000001B ;(III)Write deleted-data mark

;Composite 1793 commands
;Note: For some controllers, disk will always appear
;ready if not HDLOAD.

RESTOR	equ	RSTRCM+RS15MS+HDLOAD ;Restore
RDSECT	equ	RSECCM+ED15MS+F1SCMP ;read sector, side 0

;WD1793 status after a command

SBUSY	equ	00000001b ;(All) busy
SCRCER	equ	00001000b ;(All) CRC error
SWPROT	equ	01000000b ;(All) write protect
SNORDY	equ	10000000b ;(All) drive not ready
SINDEX	equ	00000010b ;(I) index
STRAK0	equ	00000100b ;(I) track 0
SSEKER	equ	00010000b ;(I) seek error
SHEADL	equ	00100000b ;(I) head loaded
SWFALT	equ	00100000b ;(Write cmds) write fault
SDATRQ	equ	00000010b ;(II & III) data request state
SLOSTD	equ	00000100b ;(II & III) lost data error
SRNFER	equ	00010000b ;(II & III) record not found
SRCTYF	equ	00100000b ;(II & III) record type fault
SWFALT	equ	00100000b ;(II & III) Write fault

 endif ;WD179x

;==================
;End of Definitions
;==================

;========================================
;= Memon/80 fixed-location Entry points =
;= These are organized so that most of  =
;= them align with CP/M BIOS offsets:   =
;= MEINIT ~ BOOT                        =
;= MECSTA ~ CONST                       =
;= MECIN  ~ CONOUT                      =
;= MECOUT ~ CONOUT                      =
;= METPDO ~ PUNCH                       =
;= METPDI ~ READER                      =
;========================================
	org	MEBASE		;Memon/80 ROM start

MEINIT:	di			;(00)Cold-start entry point
				;ints off for a little while

	xra	a
	mov	d,a		;So code can recognize Memon
	
MEWARM:	jmp	INIT

;the following 7 entry points can be called by CP/M,
;and have been designed with minimal stack usage.

MECSTA:	jmp	KSTAT		;(06)Test Console keyboard
				;Z set, 0 means no data
				;Z clear, a=FF if data AVAIL
MECIN:	jmp	KDATA		;(09)Get Console chr in a
MECOUT:	jmp	PRINTC		;(0C)Send C to Console
				;a=c=chr on ret

 if TPORT
METPSI:	jmp	TPISTA		;(0F)Test Transfer Port input
				;Z set, a=0 if no data
				;Z clear, a=FF if data
METPDO:	jmp	TPDATC		;(12)Send C to  Transfer Port
METPDI:	jmp	TPIDAT		;(15)Get Transfer Port chr in a
METPSO:				;Get T-Port output status
				;a=0 & Z set if not ready
				;a=FF & Z clear if ready

;Fall into TPOSTA
 endif ;TPORT

;***CP/M External Subroutine************************
;Get Transfer Port Output Status
;On Exit:
;  Z clear and a=FFh if the printer can accept a chr
;  Z set and a=0 of the print queue is full
;***************************************************
TPOSTA:

 if TPORT and (not TMEMAP)
	in	TSTAT
 endif ;TPORT and (not TMEMAP)

 if TPORT and TMEMAP
	lda	TSTAT
 endif ;TPORT and TMEMAP

 if TPORT and TISTAT
	cma			;inverted status bit
 endif ;TPORT and TISTAT

 if TPORT
	ani	TTXRDY
	jmp	DOSTAT

 endif ;TPORT

;=============================
;= Cold-Start Initialization =
;=============================
;On Entry:
;  a=0
;  Interrupts are disabled
;-----------------------------
INIT:

;****************************************
;Serial Port Initialization
;(conditional assembly depending on which
;serial port options are selected above)
;****************************************

;------------------------------------------------------
;Initialize RAM flags for any interrupt-driven I/O port
;On Entry and Exit:
;  a=0
;------------------------------------------------------
 if CRXINT
	sta	CIFLAG
 endif ;CRXINT

 if TRXINT
	sta	TIFLAG
 endif ;TRXINT

;--------------------------------
;Initialize any/all INS8250 UARTs
;On Entry:
; a=0
;--------------------------------
;INS8250 as Console

 if C8250 and (not T8250)
	out	CSINTEN		;Interrupts off
	out	CSTAT

	mvi	a,0Fh		;modem control reg
	out	CSMDMCT		;Handshakes true

	mvi	a,CSTOPS or CDLAB ;baud rate divisor access
	out	CSLCTRL		;and default stop bits

	mvi	a,cPBAUD SHR 8	;Divisor high byte
	out	CSINTEN
	mvi	a,cPBAUD and 0FFh
	out	CDATA		;Divisor low byte

	mvi	a,CSTOPS	;stop pointing to divisor
	out	CSLCTRL

 endif ;C8250 and (not T8250)

;INS8250 as Transfer Port

 if T8250 and (not C8250)
	out	TSINTEN		;Interrupts off
	out	TSTAT

	mvi	a,0Fh		;modem control reg
	out	TSMDMCT		;Handshakes true

	mvi	a,TSTOPS or TDLAB ;baud rate divisor access
	out	TSLCTRL		;and default stop bits

	mvi	a,TPBAUD SHR 8	;Divisor high byte
	out	TSINTEN
	mvi	a,TPBAUD and 0FFh
	out	TDATA		;Divisor low byte

	mvi	a,TSTOPS	;stop pointing to divisor
	out	TSLCTRL
 endif ;T8250 and (not C8250)

 if C8250 and T8250
	out	CSINTEN		;Interrupts off
	out	CSTAT
	out	TSINTEN
	out	TSTAT

	mvi	a,0Fh		;modem control reg
	out	CSMDMCT		;Handshakes true
	out	TSMDMCT		;Handshakes true

	mvi	a,CSTOPS or CDLAB ;baud rate divisor access
	out	CSLCTRL		;and default stop bits

	mvi	a,cPBAUD SHR 8	;Divisor high byte
	out	CSINTEN
	mvi	a,cPBAUD and 0FFh
	out	CDATA		;Divisor low byte

	mvi	a,CSTOPS	;stop pointing to divisor
	out	CSLCTRL


	mvi	a,TSTOPS or TDLAB ;baud rate divisor access
	out	TSLCTRL		;and default stop bits

	mvi	a,TPBAUD SHR 8	;Divisor high byte
	out	TSINTEN
	mvi	a,TPBAUD and 0FFh
	out	TDATA		;Divisor low byte

	mvi	a,TSTOPS	;stop pointing to divisor
	out	TSLCTRL

 endif ;C8250 and T8250

;---------------------------------------------
;Initialize any/all I/O or memory-mapped 8251
;UARTs for async polled access, no interrupts.
; On Exit:
;   a=0
;---------------------------------------------
 if C8251 or T8251
	lxi	h,IN8251	;8251 INIT table

SI2LUP:	mov	a,m
 endif ;C8251 or T8251

 if C8251 and (not CMEMAP)
	out	CCTRL		;I/O-mapped 8251
 endif ;C8251 and (not CMEMAP)

 if C8251 and CMEMAP
	sta	CCTRL		;memory-mapped 8251
 endif ;C8251 and CMEMAP

 if T8251 and (not TMEMAP)
	out	TCTRL		;I/O-mapped 8251
 endif ;T8251 and (not TMEMAP)

 if T8251 and TMEMAP
	sta	TCTRL		;memory-mapped 8251
 endif ;T8251 and TMEMAP

 if C8251 or T8251
	inx	h
	xri	SI24		;last one?
	jnz	SI2LUP
 endif ;C8251 or T8251

 if CISIO2A or CISIO2B or TISIO2A or TISIO2B
	out	SIOCTL		;a=0: disable SIO-2 interrupts
 endif ;CISIO2A or CISIO2B or TISIO2A or TISIO2B

;-------------------------------
;Initialize any/all MC6850 ACIAs
;-------------------------------
 if C6850 or T6850
	mvi	a,U68RES	;ACIA reset
 endif ;C6850 or T6850 

 if C6850
	out	CCTRL		;2SIO port
 endif ;C6850

 if T6850
	out	TCTRL		;2SIO port
 endif ;T6850

 if C6850 or T6850
	mvi	a,U68ST2	;Port set up
 endif ;C6850 or T6850 

 if C6850
	out	CCTRL		;2SIO port
 endif ;C6850

 if T6850
	out	TCTRL		;2SIO port
 endif ;T6850
;------------------------------------------------------------
;Initialize the baud rates for both CCS 2718 serial channels,
;including setting their baud rates. (If port B is used, it
;will be initialized in the 8251 initialization below.) If
;one of the CCS 2718 serial ports is not used, then its baud
;rate will default to a value of 0, which is 19200 baud.
;------------------------------------------------------------
CBD18	set	0
TBD18	set	0
 if CC2718A or CC2718B
CBD18	set	CPBAUD
BASE18	set	CBASE
 endif ;CC2718A or CC2718B

 if TC2718A or TC2718B
TBD18	set	TPBAUD
BASE18	set	TBASE
 endif ;TC2718A or TC2718B

 if CC2718A or CC2718B or TC2718A or TC2718B
	mvi	a,(CBD18 or TBD18) and 0FFh
	out	BASE18
 endif ;CC2718A or CC2718B or TC2718A or TC2718B	

;----------------------------------
;Initialize both CCS 2719 channels,
;including setting their baud rates
;----------------------------------
 if DART

;Initialize both DART channels

	lxi	h,DITAB

DILOOP:	mov	a,m
	inx	h
 endif ;DART

 if CC2719A or CC2719B
	out	CCTRL
 endif ;CC2719A or CC2719B

 if TC2719A or TC2719B
	out	TCTRL
 endif ;TC2719A or TC2719B

 if DART
	cpi	DI7		;last one?
	jnz	DILOOP

;set the stop bits for each channel

	mvi	a,DWR4
 endif ;DART

 if CC2719A or CC2719B
	out	CCTRL
 endif ;CC2719A or CC2719B

 if TC2719A or TC2719B
	out	TCTRL
 endif ;TC2719A or TC2719B

 if CC2719A or CC2719B
	mvi	a,CSTOPS
	out	CCTRL
 endif ;CC2719A or CC2719B

 if TC2719A or TC2719B
	mvi	a,TSTOPS
	out	TCTRL
 endif ;TC2719A or TC2719B

;Initialize baud rates in the CTC

 if CC2719A or CC2719B
	mvi	a,cPBAUD SHR 8	;set up Console baud
	out	CCTC
	mvi	a,cPBAUD and 0FFh
	out	CCTC
 endif ;CC2719A or CC2719B

 if TC2719A or TC2719B
	mvi	a,TPBAUD SHR 8	;set up Transfer Port baud
	out	TCTC
	mvi	a,TPBAUD and 0FFh
	out	TCTC
 endif ;TC2719A or TC2719B

;--------------------------------------------
;Initialize both Compupro Interfacer channels
;--------------------------------------------
 if CIFAC or TIFAC
	mvi	a,IFRST
 endif ;CIFAC or TIFAC

 if CIFAC
	out	CCTRL
 endif ;CIFAC

 if TIFAC
	out	TCTRL	
 endif ;TIFAC

;---------------------------------
;Initialize Compupro Interfacer II
;---------------------------------
 if CIFAC2
	mvi	a,CIFRST
	out	CCTRL	
 endif ;CIFAC2

 if TIFAC2
	mvi	a,TIFRST
	out	TCTRL	
 endif ;TIFAC2

;----------------------------------------
;Initialize both Cromemco TU-ART channels
;----------------------------------------
 if CTUART or TTUART
	mvi	a,TURST		;reset
 endif ;CTUART or TTUART

 if CTUART
	out	TCCTRL
 endif ;CTUART

 if TTUART
	out	TTCTRL
 endif ;TTUART

 if CTUART or TTUART
	xra	a		;disable interrupts
 endif ;CTUART or TTUART

 if CTUART
	out	TCINTE
 endif ;CTUART

 if TTUART
	out	TTINTE
 endif ;TTUART

 if CTUART
	mvi	a,cPBAUD SHR 8	;Console high baud rate bit
	out	TCCTRL

	mvi	a,cPBAUD and 0FFh ;set Console baud rate
	out	TCPBAUD
 endif ;CTUART

 if TTUART
	mvi	a,TPBAUD SHR 8	;Transfer PT high baud rate bit
	out	TTCTRL

	mvi	a,TPBAUD and 0FFh ;set Transfer Port baud rate
	out	TTPBAUD	
 endif ;TTUART

;----------------------------------------
;Install jump vectors for H8-5 interrupts
;----------------------------------------
 if CH85
;	mvi	a,JMP
	mvi	a,0C3H
	sta	P8INT3	;addrss of PAM-8/XCON-8 Int 3 vector
	lxi	h,CONINT
	shld	P8INT3+1
 endif ;CH85

 if TH85
;	mvi	a,JMP
	mvi	a,0C3H
	sta	P8INT3	;addrss of PAM-8/XCON-8 Int 3 vector
	lxi	h,TBINT
	shld	P8INT3+1
 endif ;TH85

;----------------------------------
;Initialize the Ithaca Intersystems
;Series II VIO serial port(s)
;----------------------------------
 if CIVIO2
	in	CCTRL		;reset mode byte flipflop

	mvi	a,CPBAUD shr 8 ;set Console baud rate
	out	CMODE		;<MR17:MR10>

	mvi	a,CPBAUD and 0FFh
	out	CMODE		;<MR27:MR20>
 endif ;CIVIO2

 if TIVIO2
	in	TCTRL		;reset mode byte flipflop

	mvi	a,TPBAUD shr 8 ;default t-port baud rate
	out	TMODE		;<MR17:MR10>

	mvi	a,TPBAUD and 0FFh
	out	TMODE		;<MR27:MR20>
 endif ;TIVIO2

 if CIVIO2 or TIVIO2
	mvi	a,CTL2651	;Control reg initialization
 endif ;CIVIO2 or TIVIO2

 if CIVIO2
	out	CCTRL
 endif ;CIVIO2

 if TIVIO2
	out	TCTRL
 endif ;TIVIO2

;---------------------------------------------------------
;Initialize any/all Jade Serial/Parallel I/O  serial ports
;---------------------------------------------------------
 if CJADSP or TJADSP
	mvi	a,JRESET
 endif ;CJADSP or TJADSP

 if CJADSP
	out	CCTRL
 endif ;CJADSP

 if TJADSP
	out	TCTRL
 endif ;TJADSP

;*********************************
;End of Serial Port Initialization
;*********************************

;----------------------------------------------------
;Hunt for the highest contiguous RAM page, and create
;a stack at its top. (This assumes memory comes in
;256-byte pageS.) Leave all memory as we found it.
;If Memon/80 is running in low memory (e.g. for
;debugging) then don't hunt in Memon's memory space.
;----------------------------------------------------
HUNTA	set	STACK		;Hunt starting at 0

 if RAMCOD			;Running in low memory?
HUNTA	set	(CODEND and 0FF00h) or STACK
				;y: Hunt after Memon/80
 endif ;RAMCOD

 if RAMHNT
	lxi	h,HUNTA

FSLOOP:	inr	h		;next RAM page
	jz	FSEND		;end of RAM?

	mov	a,m		;get original RAM
	mov	b,a		;and remember it
	cma
	mov	m,a		;write the inverse
	cmp	m		;Did it write right?
	mov	m,b		;Put original RAM back
	jz	FSLOOP		;keep looking if RAM OK
FSEND:

	dcr	h		;last RAM page
	sphl			;stack in RAM page

	mvi	l,RAMBEG	;For announcement

 endif ;RAMHNT

 if not RAMHNT
;Set up stack in RAM, and load hl with our 1st RAM address

	lxi	sp, STACK
	lxi	h,RAMBEG	;For announcement
 endif ;not RAMHNT

 if TPORT
;set up default: Transfer Port enabled (H<>0 here)

	push	h		;h=port flag at top of stack
 endif ;TPORT

;-------------------
;print Signon Banner
;-------------------
	call	CILPRT		;Enables ints too, if ENINTS
	db	'Memon/80 '
	db	((VERSION and 0F0h)/16)+'0','.'
	db	(VERSION and 0Fh) +'0'
	db	' by M.Eberhard',CR

 if not HELPC	;HELPC option also adds LF's after CR's
	db	LF
 endif ;not HELPC

	db	'RAM:',' '+80h

;Announce RAM location. hl currently points
;tO the address of MEMON's 1st RAM usage

	call	PHLCHX		;Trashes bc

;Fall into MAIN

;***************************
;Command Processor Main Loop
;Get and process commands
;***************************

;Print the prompt, and get a line of keyboard input

MAIN:	lxi	b,MAIN		;create command-return
	push	b		;..address on the stack

	call	CILPRT		;print CR,LF, prompt
	db	PROMPT+80h	;Enables ints too, if ENINTS

	call	GETLIN		;get user input line
				;de=beginning of line
				;Z set if no chr found
				;0 at end of line

	rc			;No command? just ignore.

;Search the command list, and execute a command if found

	xchg			;command address to hl
	lxi	d,COMTAB-1	;point to command table

	mov	c,m		;1st command byte in c
	inx	h		;2nd command byte in m

;Search through table at de for a 2-chr match of c,m. Note
;that c & m have their parity bits stripped. The msb of the
;2nd table chr is the high bit of the address offset, so
;that the range of address offsets is 512 bytes.

NXTCOM:	inx	d		;skip over address offset

	ldax	d
	ora	a		;test for table end
	jz	CMDERR		;not in table

	xra	c		;test 1st chr

	mov	b,a		;temp save result
	inx	d		;2nd table chr

	ldax	d
	xra	m		;test 2nd chr
	inx	d		;point to address offset

	ora	b		;both chrs match?

 if LOWERC
	ani	('a'-'A') xor 0FFh ;lowercase ok
 endif ;LOWERC

	rlc			;address offset msb to lsb
	cpi	2		;0 or 1 means match
	jnc	NXTCOM

;Got a match. Compute routine address and put it on stack
;a=msb of address offset. Z is clear here, due to cpi above

	inx	h		;skip past 2nd command letter

	mov	b,a		;offset msb
	xchg			;(hl)=offset addr of routine
				;de=input buffer pointer

	mov	c,m		;bc=address offset (b=0 or 1)
	lxi	h,CMDBAS	;base of command routines
	dad	b		;hl=address of routine

	push	h		;command address on stack

;Special case: the TE command gets a non-hex parameter.
;The address offset for the TE command is 0. a=msb, c=lsb.

 if (TPORT and (not ROM2K)) or (ROM2K and (not TPORT))
	ora	c
	rz
 endif ;(TPORT and (not ROM2K)) or (ROM2K and (not TPORT))

;ROM2K and TPORT special case: the 1st 2 commands have a non-
;hex 1st parameter, and the 1st 1 is only a jmp (3 bytes).
; CE <command line text> (execute a CP/M program)
; TE <exit chr>  (terminal mode)

 if ROM2K and TPORT 
	rrc			;put msb in high bit
	ora	c		;combine low bits
	ani	0FCh		;0 if offset = 0 or 3
	rz
 endif ;ROM2K and TPORT

;Get following hex parameter (if any) and put it in hl. Set the
;carry flag if no parameter present. Leave de pointing to the
;1st chr after the 1st parameter. 'return' to the command
;routine on the stack.

	db	21h		;"LXI H" opcode skips 2

;Skip over PHFHEX and into FNDHEX

;***Subroutine*************************************
;Push h, then scan past spaces and get a hex value
;On Entry:
;  de=address of next item in the input line buffer
;On Exit:
;  Top-of-stack=original HL value
;  hl=value from input buffer
;  de advanced past chr
;  Z set, carry clear if value found
;  carry set, a=0 if no value found
;**************************************************
PHFHEX:	xthl			;push hl
	push	h		;..beneath return address

;Fall into FNDHEX

;***Subroutine*************************************
;Scan past spaces and get a hex value
;On Entry:
;  de=address of next item in the input line buffer
;On Exit:
;  hl=value of last 4 digits found, defaults to 0
;  Z set
;  de advanced past last digit found
;  carry clear if value found
;  carry set if no value found
;**************************************************
FNDHEX:	lxi	h,0		;default & initial value

FHEXLP:	call	SSPACE		;skip spaces, get 1st digit
	rc			;carry set if no digits

	dad	h		;make room for the new digit
	dad	h
	dad	h
	dad	h

 if LOWERC
	cpi	'a'		;Lowercase letter?
	jc	FXHL1
	ani	('a'-'A') xor 0FFh ;y: convert to uppercase
FXHL1:
 endif ;LOWERC

	call	HEXCON		;convert a to binary
	jnc	CMDERR		;not valid hexidecimal value?

	add	l
	mov	l,a		;move new digit in
	inx	d		;bump the pointer

	ldax	d		;get next character
	ora	a		;End of line?
	rz			;Y: a=0, carry clear

	sui	' '		;value separator?
	jnz	FHEXLP		;N: it's another hex digit

	ret			;a=0 for exit, keep carry clear

;=========================
;base address for commands
;=========================
CMDBAS	equ	$

;***ROM2K Command Routine*******************************
;CE Command Entry
;Jump to actual routine, more than 512 bytes from CMDBAS
; ---> Must be first, at CMDBAS <---
;*******************************************************
 if ROM2K
CECMD:	jmp	GCECMD
 endif ;ROM2K

 if TPORT
;***Command Routine************************************
;TE [<EXchr>] (Simple Terminal mode)
;
; --->not ROM2K: TECMD must be first, at CMDBAS<---
; --->ROM2K: TECMD must be second, at CMDBAS+3<---
;
;Send all Console keyboard data to Transfer Port, and
;send all Transfer Port data to the Console. <EXchr>
;from the keyboard exits. (default to DTEXIT.)
;On Entry:
;  de=points to the 1st chr in RAMBUF past the 'TE' cmd
;******************************************************
TECMD:	call	CILPRT		;announce exit chr
	db	'Exit ','^'+80h

	call	SSPACE		;get optional exit character
	ora	a		;nothing entered?
	jnz	TMNL1		;got an exit value in a
	mvi	a,DTEXIT	;default exit chr

;Convert exit chr to uppercase, non-control,
;and print exit character message

TMNL1:	ani	1Fh		;make it a CTRL chr
	mov	l,a		;remember exit chr

	ori	'C'-CTRLC	;make it printable
	call	PRINTA

	call	CILPRT		;CR,LF,CR to be pretty
	db	CR+80h		;(start on a new line)

;Be a Terminal until we get an exit character=l

TLOOP:	call	KSTAT		;anything typed?
	cnz	KDATA		;Y:get the kbd data

	cmp	l		;exit chr?
	rz			;Y: done

	ora	a		;typed chr? (ignore null)
	cnz	PBODAT		;kbd data to Transfer Port

	call	TPISTA		;any Transfer Port data?
				;NZ if so

	cnz	TPIDAT		;get Transfer Port data
				;always returnS w/ NZ
	cnz	PRINTA		;and send it to Console
	jmp	TLOOP
 endif ;TPORT

;***Command Routine*************************************
;? Command Entry
;Jump to actual routine, more than 512 bytes from CMDBAS
;*******************************************************
 if HELPC
HELP:	jmp	GHELP
 endif ;HELPC

;***ROM2K Command Routine*******************************
;MT Command Entry
;Jump to actual routine, more than 512 bytes from CMDBAS
;*******************************************************
 if ROM2K
MTCMD:	jmp	GMTCMD
 endif ;ROM2K

;***ROM2K Command Routine*******************************
;SE Command Entry
;Jump to actual routine, more than 512 bytes from CMDBAS
;*******************************************************
 if ROM2K
SECMD:	jmp	GSECMD
 endif ;ROM2K

;***ROM2K Command Routine*******************************
;TB Command Entry
;Jump to actual routine, more than 512 bytes from CMDBAS
;*******************************************************
 if ROM2K and  TSBAUD
TBCMD:	jmp	GTBCMD
 endif ;ROM2K and  TSBAUD

;***Command Routine*************************************
;BO Command Entry
;Jump to actual routine, more than 512 bytes from CMDBAS
;*******************************************************
BOCMD:
JMPBOT	set	TRUE

 if H17R or MDUBLR or MICROR or NSTRSR or NSTRDR or IMD8R or IMDMR
BOOTER	set	TRUE
JMPBOT	set	FALSE
 endif ;H17R or MDUBLR or MICROR or NSTRSR or NSTRDR or IMD8R or IMDMR

 if ROM2K and BOOTER
	call	CILPRT		;2K ROM, so be verbose
	db	'Booting',CR+80h
 endif ;ROM2K and BOOTER

;----------------------------------------
;Boot using disk controller's onboard ROM
;----------------------------------------
 if H17R or MDUBLR or MICROR or NSTRSR or NSTRDR
	jmp	DMBASE	;use the H17's onboard boot code
 endif ;H17R or MDUBLR or MICROR or NSTRSR or NSTRDR

;-------------------------------------------------------
;Boot from IMSAI MDC-DIO Floppy Controller's onboard ROM
;(Boot address depend on 8" or minidisk)
;-------------------------------------------------------
 if IMD8R
DBOOT	equ	DMBASE		;Boot from 8" disk
 endif ;IMD8R

 if IMDMR
DBOOT	equ	DMBASE+3	;Boot from minidisk
 endif ;IMDMR

 if IMD8R or IMDMR
IDRSEL	equ	DIBASE+0Eh	;disable controller, enable RAM
IDCSEL	equ	DIBASE+0Fh	;disable RAM, enable controller

	out	IDCSEL	;disable RAM, enable controller
			;(a's value doesn't matter)

	jmp	DBOOT	;Use onboard boot code

 endif ;IMD8R or IMDMR

;------------------------------------------
;The boot code might be too big to fit here
;------------------------------------------
 if BOOTER and JMPBOT
	jmp	GBOCMD
 endif ;BOOTER and JMPBOT

;***Command Routine**************************************
;EN [<ADR>] (enter data into memory)
;
;Get hex values from the keyboard and enter them
;Sequuentially into memory, starting at <ADR>. a blank
;line ends the routine and returns control to the
;command mode. Values may be separated by spaces or CR'S.
;Print the current address at the beginning of each line.
;On Entry:
;  hl=address, defaultING to 0
;  carry set if none entered
;********************************************************
ENCMD:	call	PHLADR		;print hl as an address
				;trash c, b=0

	call	GETLIN		;get a line of user input
	rc			;Z=blank line terminates

;Get hex data from the user input line and write it to memory

ENLOOP:	call	PHFHEX		;push memory address and
				;..get/convert next value

	mov	a,l		;get low byte as converted

	pop	h		;recover memory address
	mov	m,a		;put in the value
	inx	h		;next address

	call	SSPACE		;scan to next input value
	jnc	ENLOOP		;not end of line: continue

	jmp	ENCMD		;END of line: start new line

;***Command Routine*********************************
;FI [<ADR> [<BCNT> [<VAL>]]] (fill memory)
;
;Fill <BCNT> bytes of memory with <VAL> from <ADR>.
;If <VAL> is not provided, then fill the specified
;range with 00. If <BCNT> is not provided, then fill
;memory until fill reaches the RAM page. If <ADR> is
;not provided, then fill from 0.
;On Entry:
;  hl=<ADR>
;  Carry set if none entered
;  de points to <BCNT>, <VAL> follows, if any
;***************************************************
FICMD:

 if RAMCOD			;Running in low memory?
	push	h
	lxi	b,-CODEND	;Will this wipe out Memon/80?
	dad	b		;16-bit compare
	pop	h
	jnc	CMDERR		;y: error
 endif ;RAMCOD

	call	PHFHEX		;push address and
				;..get hl=byte count default=0

	call	PHFHEX		;push byte count and
				;..get fill data, default 0
	mov	d,l		;fill data in d

	pop	b		;bc has byte count

 if RAMHNT
	call	RAMPAG		;get h=RAM page
	mov	e,h
 endif ;RAMHNT

 if not RAMHNT
	mvi	e,RAMEND/256
 endif ;not RAMHNT

	pop	h		;hl has start address

FILOOP:	mov	a,h		;filling Memon/80's RAM page?
	cmp	e
	rz			;Y: done

	mov	m,d		;fill data

	inx	h
	dcx	b		;done yet?
	mov	a,b
	ora	c
	jnz	FILOOP

	ret

;***Command Routine********************
;HD <ADR> <BCNT> (Intel hex dump)
;
;Dump the specified memory range to the
;Transfer Port as an Intel hex file
;On Entry:
;  hl=ADR
;  de points to <BCNT>
;**************************************
HDCMD:	call	GETHEX		;save 1st address
				;get hl=byte count

	pop	d		;recover start address
	xchg			;hl has start, de has count

;Loop to send requested data in HDRLEN-byte records

;Send record-start

HDLINE:	call	TPCRLF		;send CRLF to Transfer Port

	lxi	b,hDRLEN*256	;b=bytes/line, 
				;..b<>0 for TPOUT,
				;..c=0 initial checksum

	mvi	a,':'		;record start
	call	TPOUT

;Compute this record length (b=HDRLEN here)
	
	mov	a,e		;short last line?
	sub	b		;normal bytes/line
	mov	a,d		;16-bit subtract
	sbb	c		;c=0 here
	jnc	HDLIN1		;N: full line

	mov	b,e		;Y:short line
HDLIN1:

;If byte count is 0 then go finish EOF record

	mov	a,b
	ora	a
	jz	HDEOF

;Send record byte count=a to the Transfer Port (b<>0)
;checksum=0 in c here

	call	PAHCSM		;send byte count

;Send the address at the beginning of each line,
;computing the checksum in c

	call	PHLHEX		;hl=address, b<>0

;Send the record type (00), checksum in C

	xra	a
	call	PAHCSM

;Send b bytes of hex data on each line, computing
;the checksum in c

HDLOOP:	call	PMHCSM		;send chr to Transfer Port

	dcx	d
	inx	h
	dcr	b		;next
	jnz	HDLOOP

;Compute & send the checksum

	xra	a
	sub	c
	inr	b		;To Transfer Port
	call	PAHCSM

;Give the user a chance to break in at the end of each line

	call	CHKKBD
	jmp	HDLINE		;next record

;***Command Routine******************************
;IN <PORT> (input from port)
;
;On Entry:
;  l=PORT
;Creates this routine on the stack, then executes
;it, then returns through PCAHEX to print value
;
;    nop
;    in   <PORT>
;    ret
;    <PCAHEX address for return>
;*************************************************
INCMD:	call	PCCOLS		;': ' as separator

	lxi	d,PCAHEX	;create return address
	push	d		;ret through PCAHEX

;	mvi	h,RET		;opcode
	mvi	h,0C9H
	push	h		;l=<PORT>

;	lxi	h,IN*256	;NOP, in opcodes
	lxi	h,0DBH*256

	jmp	IOCMD		;recycle some code

;***Command Routine*********************************
;OT <PORT> <data> (output to port)
;
;On Entry:
;  l=PORT
;  de points to data
;
;Creates this routine on the stack, then executes it
;   NOP
;   OUT  <PORT>
;   RET
;**************************************************
OTCMD:
;	mvi	h,RET		;ret opcode, l=PORT
	mvi	h,0C9H

	call	GETHEX		;push <PORT>, RET opcode
				;get l=<data>

	mov	a,l		;port data

;	lxi	h,out*256	;NOP, out opcodes
	lxi	h,0D3H*256

;Fall into IOCMD

;--------------------
;Recycle some code
;On Entry:
;  l=0
;  h=IN or OUT opcode
;--------------------
IOCMD:	push	h		;install IN or OUT opcode

 if EXOPT
	jmp	EXSTK		;recycle some code
 endif ;EXOPT

 if not EXOPT
	mov	h,l		;hl=0
	dad	sp		;hl points to routine

	pop	d		;repair sp so that
	pop	d		;..we can return

;Fall into EXCMD to execute RAM routine at (hl)
 endif ;not EXOPT

;***Command Routine******************************************
;EX [<ADR> [<OPT>]](execute)
;
;EXOPT=TRUE adds <OPT> to optionally disable the EPROM on an
;Altair-type board (such as on the Turnkey Module or my own
;88-2SIOJP) on the way to executing at the requested address.
;On Entry:
;  hl=address, default to 0
;  Carry set if none entered
;  Top-of-stack has main address
;************************************************************
 if not EXOPT
EXCMD:	pchl
 endif ;not EXOPT

 if EXOPT
EXCMD:	call	PHFHEX		;push <ADR>, get l=<OPT>
	dcr	l		;Anything but 1 just
	rnz			;..executes at <ADR>

;Fall into EXECDP

;***Subroutine****************************************
;Execute 'IN FF' from RAM to disable PROM, then return
;(Some boards disable PROM on any IN FF)
;*****************************************************
EXECDP:
;	mvi	e,RET		;ret, don't care
	mvi	e,0C9H
	push	d

	lxi	d,0FFDBh	;IN FF opcode
	push	d

;Fall into EXSTK

;***Subroutine************
;Execute code on the stack
;*************************
EXSTK:	lxi	h,0		;Find our RAM page to find
	dad	sp		;..the code we just pushed

	pop	d		;restore stack
	pop	d

	pchl			;execute the code we pushed
 endif ;EXOPT

 if TPORT
;***Command Routine****************************
;TP [<0/1>] enable/disable Transfer Port
;
;0 means disabled (use Console), anything else
;means Transfer Port enabled.
;On Entry:
;  l=enable state
;  Top-of-stack=return address to MAIN
;  next-on-stack=TP flag word
;**********************************************
TPCMD:	mov	a,l		;get state
	lxi	h,3		;TP flag is 3 back
	dad	sp		;..on the stack
	mov	m,a		;New state

	ret
 endif ;TPORT

;***Command Routine**********************************
;DU [<ADR> [<BCNT>]] (dump memory)
;
;Print <BCNT> bytes of memory contents from <ADR> on
;the Console in HEX.  If no count is specified, then
;then print the contents of all memory, 10000h bytes.
;Pause with the space bar, abort with control-C.
;
;On Entry:
;  hl=address
;  de points to <BCNT>, if anY
;****************************************************
DUCMD:	call	PHFHEX		;push address and
				;..get hl=byte count or 0
				;a=0
	adc	l		;carry set if none entered
	mov	l,a		;hl defaults to 1

	pop	d		;recover start address
	xchg			;hl has start, de has count

;Print the address at the beginning of each line

DULINE:	call	PHLADR		;print hl as an address
				;Trashes c, sets b=0
	mov	c,b		;line byte counter=0

;Print up to 16 bytes of hex data
;(separated by spaces) on each line

	push	h		;save mem pointer for ASCII dump

DULOOP:	mov	a,m
	call	PCAHEX		;chr to Console in hex, trash b
	inr	c		;line byte counter

	call	ILPRNT		;space separates bytes
	db	' '+80h
	inx	h

	dcx	d		;all done with DU data?
	mov	a,d
	ora	e
	jz	DLDONE		;Y: print last ASCII too

	mvi	a,0Fh		;all done with row?
	ana	l		;new line Every XXX0 hex
	jnz	DULOOP
DLDONE:

 if DUPRTY
;Pad out the rest of the line so that the ASCII lines up
;for short lines. c=number of bytes printed so far
;(This just makes the display a little prettier.)

	mvi	a,10h		;a full line has 16 bytes
	sub	c		;how short is this line?
	jz	DNOPAD		;No pading for full lines

	mov	b,a		;loop counter

DPLOOP:	call	ILPRNT
	db	'  ',' '+80h	;3 bytes per hex digit
	dcr	b
	jnz	DPLOOP
DNOPAD:
 endif ;DUPRTY

;print up to 16 ASCII characters, or '.' if unprintable

	pop	h		;recover this line's address

ADLOOP:	mov	a,m		;get a character
	inr	a		;DEL (7Fh) is also non-printing
	ani	7Fh		;strip high (parity) bit
	cpi	' '+1		;Everything below space
	jnc	PRNTBL		;..is non-printing

	mvi	a,'.'+1		;A dot for non-printing chr

PRNTBL:	dcr	a		;undo inc
	call	PRINTA

	inx	h		;next byte on this ROW
	dcr	c		;this line's byte count
	jnz	ADLOOP

	mov	a,d		;all done with dump?
	ora	e
	rz			;done with DU command
	
;Give the user a chance to break in at the end of each line.
;Pause if anything except ^C typed. abort if ^C.
	call	CPAUSE
	jmp	DULINE		;next line

;***Command Routine***************************************
;HL [<OFST>] (Intel Hex Load)
;
;Load an Intel hex file from the Transfer Port into memory
;AT the addresses specified in the hex file, with optional
;address offset <OFST>. Ends with any record with 0 data
;bytes, or if control-C is typed.
;
;HLRECS=TRUE adds record counting, with a report of the
;total number of records received when done.
;
;ROM2K will enable RAM read-back test when writing to RAM
;
;On Entry:
;  hl=offset (defaults to 0)
;
;Register usage during hex load:
;  b: Scratch
;  c: record byte count
;  d: record checksum
;  e: RAM page address
;  hl: memory address
;  top of stack: address offset
;  Next on stack: Record count
;*********************************************************
HLCMD:

 if HLRECS
	lxi	d,0		;initialize record count
	push	d		;keep it on the stack
 endif ;HLRECS

	push	h		;address offset onto stack

;Eat all chrs until we get record-start colon

GETCOL:	call	GETTPD
	cpi	':'
	jnz	GETCOL

;Restart checksum, then get 4-byte record header: (a=0 here)
; c gets 1st byte=data byte count
; h gets 2nd byte=address high byte
; l gets 3rd byte=address low byte
; b gets 4th byte=record type (ignored)
; d computes checksum

	lxi	d,4		;Initialize checksum in d
				;get e=4 header bytes

;Shift in the four header bytes: c <- h <- l <- b

HLHDR:	mov	c,h		;c=byte 1: byte count
	mov	h,l		;h=byte 2: address msb
	mov	l,b		;l=byte 3: address lsb
	call	GTPBYT		;get header byte, do checksum
	dcr	e
	jnz	HLHDR

;Offset the record address by the value on top of the stack
;a=checksum so far here

	pop	d		;get offset

 if HLRECS
	xthl			;bump record count
	inx	h
	xthl
 endif ;HLRECS

	dad	d		;offset the address
	push	d		;save offset

 if RAMCOD			;Running in low memory?
	push	h
	lxi	d,-CODEND	;Wipe out Memon/80?
	dad	d		;16-bit compare
	pop	h
	jnc	OVMERR		;y: error
 endif ;RAMCOD

	mov	d,a		;d=checksum so far

;c=byte count
;d=checksum so far
;hl=RAM address for this record=record address+offset

;Test for 0-byte record, which is the EOF

	mov	a,c		;c=byte count
	ora	a		;0-byte record?
	jz	GETCSM		;Y: done

;Get e=RAM page address to test for overwriting. this
;bocks out a 256-byte region of memory Wherever this
;program found RAM for its stack.

 if RAMHNT
	push	h
	call	RAMPAG		;get h=RAM page
	mov	e,h
	pop	h
 endif ;RAMHNT

 if not RAMHNT
	mvi	e,RAMEND/256
 endif ;not RAMHNT

;Loop to get data into memory at hl.

HLLOOP:	mov	a,h		;overwrite our RAM?
	cmp	e
	jz	OVMERR		;Y: abort load

	call	GTPBYT		;data byte in b, cksum in d
	mov	m,b

 if ROM2K
	mov	a,b		;readback compare
	cmp	m		;successful write?
	jnz	MEMERR		;N: error
 endif ;ROM2K

	inx	h
	dcr	c
	jnz	HLLOOP

	inr	c		;remember >0 data bytes

;Get and test record checksum

GETCSM:	call	GTPBYT		;get checksum in Z
	jnz	CSMERR		;NZ means checksum error

;All done with this record. Check its data byte count
;On Entry: c=0 for 0-byte record, c=1 otherwise

	call	ILPRNT		;pacifier on Console
	db	PCFIER+80h

	dcr	c		;0-byte record?
	jz	GETCOL		;N: go get another record

	pop	h		;remove offset from stack

 if HLRECS
	pop	h		;record count
	call	 CILPRT
	db	'Recs:',' '+80h
	jmp	PHLCHX		;hl in hex on Console
 endif ;HLRECS

 if not HLRECS
	ret			;done:return to MAIN
 endif ;not HLRECS

;***Command Routine********************
;VE <SRC> <DST> <BCNT> (verify memory)
;
;Compare <BCNT> bytes of memory from
;<SRC> to <DST> and report differences
;On Entry:
;  hl=<SRC>
;  carry set if none entered
;  de points to <DST>, <BCNT> follows
;*************************************
VECMD:	call	GETHX2		;save source address
				;get & save destination
				;get byte count

	push	h		;save byte count

	jmp	VERIFY		;actually verify

;***command Routine Continuation****************
;CO <SRC> <DST> <BCNT> [<RPT>](copy memory)
; (RPT only available if CORPT is TRUE)
;
;Copy <BCNT> bytes of memory from <SRC> to <DST>
;repeat <RPT> times (for EPROM programming)
;On Entry:
;  hl=<SRC>
;  carry set if none entered
;  de points to <DST>, <BCNT>, <RPT> follow
;***********************************************
COCMD:	call	GETHX2		;save source address
				;get & save destination
				;get byte count


 if CORPT
	call	CILPRT
	db	'Copyin','g'+80h

	call	PHFHEX		;push byte count and
				;..get repeat count
	adc	l		;On ret, a=0
				;carry set if no value given

;Repeat copy the specified number of times (for
;programming an EPROM, e.g. a 2708 on a bytesaver)

CORLUP:
 endif ;CORPT

	pop	b		;bc=count
	pop	d		;de=destination
	pop	h		;hl=source

	push	h		;save source
	push	d		;save destination
	push	b		;save count

 if CORPT
	push	psw		;save a=repeat count
 endif ;CORPT

;Loop to copy bc bytes from (hl) to (de)

COLOOP:	mov	a,m
	staX	d
	inx	h
	inx	d
	dcx	b
	mov	a,b
	ora	c
	jnz	COLOOP

 if CORPT
;Repeat the copy as requested by the user

	call	ILPRNT		;pacifier on Console
	db	PCFIER+80h

	pop	psw		;recover repeat count

	dcr	a		;repeat as requested
	jnz	CORLUP
 endif ;CORPT

;Fall into VERIFY to verify the copy

;***Subroutine End***********************
;Verify memory. Report errors to Console.
;On Entry:
;  top of stack=byte count
;  next on stack=destination address
;  next on stack - source address
;  next on stack=return address (to MAIN)
;****************************************
VERIFY:	pop	b		;byte count
	pop	d		;de=destination
	pop	h		;hl=source

 if CORPT
	call	CILPRT
	db	'checkin','g'+80h
 endif ;CORPT

;Loop to compare memory, reporting mismatches

VELOOP:	ldax	d
	cmp	m		;match?
	cnz	VERROR		;N:done

	inx	h
	inx	d
	dcx	b
	mov	a,b
	ora	c
	jnz	VELOOP

;Fall into RAMPAG to save a byte on the way
;TO returning to MAIN

;***Subroutine********************
;Get high byte of RAM page address
;On Entry:
;  sp is valid
;On Exit:
;  h=RAM page
;Trashes l
;*********************************
 if RAMHNT
RAMPAG:	lxi	h,0
	dad	sp
 endif ;RAMHNT

	ret

;***ROM2K Command Routine Continuation*****************
;CE [command line] 
;
;Execute CP/M program
; 1. Move command line into COMBUF
; 2. Install WBOOT jump at 0
; 3. Jump to USAREA
;On Entry:
;  de=points to the 1st chr in RAMBUF past the 'CE' cmd
;  top-of-stack points to MAIN
;******************************************************
 if ROM2K

GCECMD:

;Copy null-terminated command line to CP/M's
;command line buffer

	lxi	h,COMBUF
	push	h
	mvi	b,0FFh		;chr count (minus null term)

CLLOOP:	inx	h
	ldax	d
	mov	m,a
	inx	d
	inr	b		;count chrs

	ora	a		;copy 'till null termination
	jnz	CLLOOP

	pop	h		;install chr count
	mov	m,b		;..at 1st position in buffer

;Install WBOOT jump at 0
;a=0 here

	mov	l,a		;hl=0
;	mvi	m,JMP
	mvi	m,0C3H

	lxi	h,MEWARM
	shld	WBOOTA

;Execute

	jmp	USAREA
 endif ;ROM2K

;***ROM2K Command Routine Continuation*********
;TB <BAUD> 
;
;Set and announce the Transfer Port's baud Rate
;(Only available for serial port boards with
;software-set baud rates)
;On Entry:
;  l=<BAUD>
;**********************************************
 if TSBAUD and ROM2K

GTBCMD:	jc	CMDERR		;Must supply baud rate

;Look up entry in baud rate table

	mov	a,l		;User selection
	add	a		;Table has 4 bytes
	add	a		;..per entry
	cpi	BTEND-BTABLE
	jnc	CMDERR

	mov	c,a		;c=selection*4 (0 for 110 baud)
	mvi	b,0
	lxi	h,BTABLE
	dad	b		;hl points to entry

;Get c=BRATE0 and m=BRATE1 from the table. check
;for unsupported baud rate (BRATE0 & BRATE1 are 0)
;b=0

	mov	d,m		;Value for BRATE0
	inx	h

	mov	a,m		;value for BRATE1
	ora	d
	jz	CMDERR

;Set baud rate, and set the stop bits to 1
;except for 110 baud, which gets 2 stop bits
;b=0

 endif ;TSBAUD and ROM2K

 if TSBAUD and ROM2K and (TC2718A or TC2718B)
;Both CCS2718 serial ports use the same I/O port for
;setting their baud rates. Here, we combine the fixed
;Console baud rate (if any) with the new transfer port
;baud rate.
	mvi	a,CBD18 and 0FFh ;default Console baud
	ora	d		;new baud rate
	out	BASE18
 endif ;TSBAUD and ROM2K and (TC2718A or TC2718B)

 if TSBAUD and ROM2K and (TIVIO2)
;Reset mode byte flipflop before setting baud rate

	in	TCTRL		;reset mode byte flipflop

	mov	a,m 		;stop bits, etc,
	out	BRATE1		;<MR17:MR10>

	mov	a,d		;baud rate
	out	BRATE0		;<MR27:MR20>

 endif ;TSBAUD and ROM2K and (TIVIO2)

 if TSBAUD and ROM2K and T5511
;Stop bits are encoded with the baud rates.

	mov	a,m
	out	BRATE1		;Divisor high byte
	mov	a,d
	out	BRATE0		;Divisor low byte
 endif ;TSBAUD and ROM2K and T5511

 if TSBAUD and ROM2K and TMRCTC
	mvi	a,DWR4		;point to DART
	out	TCTRL		;..write register 4

	mov	a,c		;110 baud (c=0)?
	ora	a
	mvi	a,DART1S	;1 stop bit
	jnz	TBC1
	mvi	a,DART2S	;110 baud gets 2 stop bits
TBC1:	out	TCTRL		;set the DART stop bits

	mov	a,m
	out	BRATE1		;CTC Divisor high byte
				;(must be 1st)
	mov	a,d
	out	BRATE0		;CTC Divisor low byte
 endif ;TSBAUD and ROM2K and TMRCTC

 if TSBAUD and ROM2K and T8250
	mov	a,c		;110 baud (c=0)?
	ora	a
	mvi	a,TDLAB or TSTOP1 ;baud rate access, 1 stop bit
	jnz	TBC1
	mvi	a,TDLAB or TSTOP2 ;110 baud gets 2 stop bits
TBC1:	out	TSLCTRL
	mov	e,a		;temp save

	mov	a,m
	out	BRATE1		;Baud rate divisor high byte
	mov	a,d
	out	BRATE0		;Baud rate divisor low byte

	mov	a,e		;stop pointing to divisor
	xri	TDLAB
	out	TSLCTRL

 endif ;TSBAUD and ROM2K and T8250

 if TSBAUD and ROM2K

;Print resulting baud rate, using BCD from BTABLE,
;suppressing up to 2 leading zeros, and appending
;a 0. b=0 here (for PAHEX1)

	call	CILPRT
	db	CR+80h

	inx	h	;point to the rate BCD

	mov	a,m

	rrc		;high nibble
	rrc
	rrc
	rrc
	ani	0Fh	;Leading 0 to suppress?
	cnz	PAHEX1	;n: print it

	mov	a,m
	ora	a	;2 leading 0s to suppress?
	cnz	PAHEX1	;(Strips off high nibble)

	inx	h	;low byte
	mov	a,m
	call	PAHEX2	;..always gets printed

	call	ILPRNT
	db	'0 Bau','d'+80h

	ret
 endif ;TSBAUD and ROM2K

;***ROM2K Command Routine Continuation**************
;SE <ADR> <byte1> [<byte2> [<byten>]]]
;
;    Search for string of bytes, starting at <ADR>
;    <byten> can be either hex byte or 'text string'
;
;On Entry:
;  hl=<ADR>
;  Carry set if none entered
;  de points to <bytes>
;***************************************************
 if ROM2K
GSECMD:	push	h		;remember <ADR>

;Get search string from input buffer, convert each byte
;to binary, and save result in the RAM buffer
 endif ;ROM2K

 if RAMHNT and ROM2K
	call	RAMPAG		;Find our line buffer
	mvi	l,RAMBUF	;Overwrite with converted data		
 endif ;RAMHNT and ROM2K

 if not RAMHNT and ROM2K
	lxi	h,RAMBUF
 endif ;not RAMHNT and ROM2K

 if ROM2K

	push	h		;binary string address
	lxi	b,QUOTE		;b=byte count, c=QUOTE
	
;--------------------------------------
;Loop to get either a 2-digit hex value
;or a text string (in quotes) each pass
;--------------------------------------
SCHLUP:	call	SSPACE		;returns a=found chr, 0 if none

	cmp	c		;is 1st chr a quote?
	cz	SSTRNG		;y:search for a string
	cnz	SCHHEX		;n: search for hex
				;returns carry set if end
	jnc	SCHLUP		;loop to get all input

;----------------------------------------	
;Search RAM for the requested string
; b=string length
; top-of-stack=binary string address
; next-on-stack=starting search address
;----------------------------------------
	pop	d		;binary string address
	pop	h		;search start address

	mov	a,b		;anything to search for?
	ora	a
	jz	CMDERR		;error if not

SLOOP1:	push	h		;search start address
	push	d		;binary string address

	mov	c,b		;string byte count

;Loop through all bytes of the requested string
;until either all bytes match or 1st non-matching byte

SLOOP2:	mov	a,d
	cmp	h		;don't search our own RAM page
	jz	NOMTCH

	ldax	d		;search string
	cmp	m		;current RAM
	jnz	NOMTCH

	inx	h		;test next byte
	inx	d
	dcr	c		;tested all bytes yet?
	jnz	SLOOP2

;String match found. Print address, ask to continue search

	pop	d		;binary string address
	pop	h		;search start address

	call	CILPRT
	db	'Found',' '+80h
	push	b
	call	PHLCHX		;print match address, trash bc
	pop	b

	call	CILPRT
	db	'More (Y/N)?',' '+80h

	call	GETKBD		;user response
	call	PRINTA		;echo
 endif ;ROM2K

 if ROM2K and LOWERC
	ani	('y'-'Y') XOR 0FFh	;make it uppercase
 endif ;ROM2K and LOWERC

 if ROM2K
	cpi	'Y'
	rnz			;anything but y ends

	push	h		;search start address
	push	d		;binary string address

;Search again, starting at the next byte after hl.
;Quit if we've reached the end of memory, FFFFh

NOMTCH:	pop	d		;binary string address
	pop	h		;search start address

	inx	h		;next RAM
	mov	a,h
	ora	l		;End of memory?
	jnz	SLOOP1

	call	CILPRT
	db	'Not foun','d'+80h

	ret

;---Local Subroutine-----------------------
;Get a text string from user input at (de),
;store string at (hl), bump count in b
;On Entry:
;  b=byte count
;  c=QUOTE
;  de points to initial quote
;  hl points to destintion for binary
;On Exit:
;  b=b+number of string chrs
;  de incremented past the string, and past
;      the terminating quote, if it's there
;  hl incremented for each string chr
;  Z set
;  Carry clear
;------------------------------------------
SSTRNG:	inx	d		;skip over quote

STLOOP:	ldax	d
	ora	a		;end quote is not required
	rz	

	inx	d		;point past this input chr

	cmp	c		;end of string?
	rz	

	mov	m,a		;store a string byte
	inx	h
	inr	b

	jmp	STLOOP		;get more of this string

;---Local Subroutine-------------------------------
;Get one hex value from user input at (de), convert
;it to binary, store it at (hl), bump count in b
;On Entry:
;  b=byte count
;  de points to next inut chr
;  hl points to destintion for binary
;On Exit:
;  b=b+1 if hex byte found
;  de incremented past the hex byte, if found
;  hl incremented once of hex found
;  Carry set if no hex digit found
;--------------------------------------------------
SCHHEX:	call	PHFHEX		;push next string addr byte,
				;..and get a hex value.
				;hl=0 & carry set if none

	inr	h		;no high byte allowed
	dcr	h		;does not change carry
	jnz	CMDERR

	mov	a,l		;binary value from hex
	pop	h		;next string address byte

	rc			;carry set means end of input

	mov	m,a		;store the hex digit
	inx	h
	inr	b

	ret
 endif ;ROM2K

;***ROM2K Command Routine Continuation*************
;MT [<ADR> [<CNT>]] (Test Memory)
;
;On Entry:
;  Carry set if no parameters provided
;  hl=<ADR>
;  de points to <CNT>
;**************************************************
 if ROM2K
GMTCMD:
 endif ;ROM2K

 if RAMCOD and ROM2K		;Running in low memory?
	push	h
	lxi	b,-CODEND	;Wipe out Memon/80?
	dad	b		;16-bit compare
	pop	h
	jnc	CMDERR		;y: error
 endif ;RAMCOD and ROM2K

 if ROM2K
	call	PHFHEX		;push <ADR>, get hl=<CNT>
				;hl=0 if none entered
	call	CILPRT
	db	'Testin','g'+80h

	xchg			;de=byte count
 endif ;ROM2K

 if RAMHNT and ROM2K
	call	RAMPAG		;get RAM page
	mov	a,h		;a=RAM page
 endif ;RAMHNT and ROM2K

 if not RAMHNT and ROM2K
	mvi	a,RAMEND/256
 endif ;not RAMHNT and ROM2K

 if ROM2K

	pop	h		;hl=start address

	lxi	b,MTPAT		;Test pattern sequence

;Loop until all memory locations have seen each pattern byte

MTLOOP:	push	h		;Start address
	push	d		;Byte count
	Push	b		;Pattern position
	push	psw		;a=RAM page
	
;------------------------------------------------
;Fill memory with pattern, avoiding the stack. Do
;a read/invert/write twice to stress the memory.
;------------------------------------------------
FIL0:	pop	psw		;a=RAM page address
	push	psw

	cmp	h		;On RAM page?
	jnz	FIL1

	mov	a,l		;y: stack part of RAM page?
	cpi	RAMBEG and 0FFh	;Can't test above the stack
	jnc	FIL3

FIL1:	ldax	b		;Get a pattern byte

	ora	a		;Pattern end?
	jnz	FIL2
	lxi	b,MTPAT		;y: restart pattern

FIL2:	inx	b

;High-frequency memory byte test while
;we fill memory with the pattern

	mov	m,a		;Write pattern to memory

	mov	a,m		;Invert & write
	cma
	mov	m,a

	mov	a,m		;twice
	cma
	mov	m,a

FIL3:	inx	h		;next address

	dcx	d		;byte count
	mov	a,d		;end?
	ora	e
	jnz	FIL0		;n: keep filling

	pop	psw		;a=RAM page address
	pop	b		;Pattern position
	pop	d		;Byte count
	pop	h		;Start address

;-------------------------------------------------
;compare memory to the pattern, avoiding the stack
;-------------------------------------------------
	push	h		;Start address
	Push	d		;Byte count
	push	b		;Pattern position
	push	psw		;RAM page address

CMLOOP:	pop	psw		;a=RAM page
	push	psw

	cmp	h		;On RAM page?
	jnz	CML1		;n: okay

	mov	a,l		;y: stack part of RAM page?
	cpi	RAMBEG and 0FFh	;Can't test above the stack
	jnc	CML3

CML1:	ldax	b		;Get pattern byte

	ora	a		;Pattern end?
	jnz	CML2
	lxi	b,MTPAT		;y: restart pattern

CML2:	inx	b		;next pattern byte


	cmp	m		;compare pattern to memory
	cnz	CMPERR		;report any mismatch

CML3:	inx	h		;next RAM location

	dcx	d		;next byte count
	mov	a,d		;end?
	ora	e
	jnz	CMLOOP

;---------------------------------------------------
;Done with one pass. Print pacifier, test for abort,
;and do another pass, unless we are done.
;---------------------------------------------------
	call	ILPRNT		;print pacifier
	db	PCFIER+80h

	call	CHKKBD		;Chance to abort

	pop	h		;h=RAM page address

	pop	b		;Pattern position
	pop	d		;Byte count

	inx	b		;rotate pattern once
	ldax	b		;end of pattern?
	ora	a
	mov	a,h		;restore RAM page address

	pop	h		;Start address

	jnz	MTLOOP

	ret			;to main
		
;---Local Subroutine----------
;Report memory error, and give
;user an opportunity to abort
;On Entry:
;  a=expected data
;  hl=address
;  (hl)=found data
;-----------------------------
CMPERR:	push	b		;PHLADR trashes bc
	push	psw

	call	PHLADR		;address:

	call	ILPRNT
	db	'Wrote',' '+80h

	pop	psw		;expected data
	call	PAHCSM

	call	ILPRNT
	db	', read',' '+80h
	call	PMHCSM		;memory data

	pop	b
	jmp	CPAUSE		;Abort or pause from user?

;-----------------------------
; Memory Test Pattern Sequence
;-----------------------------
MTPAT:	db	000h,0FFh,055h,0AAh,033h,0CCH,0F0h,00Fh
	db	0C3h,03CH,066h,099h,078h,001h,0FEh,002h
	db	0FDH,004h,0FBh,008h,0F7h,010h,0EFh,020h
	db	0FDH,040h,0BFh,080h,07Fh
	db	00h	;End of table mark
 endif ;ROM2K

;***Command ROutine COntinuation****************
;BO (Boot from 8" or 5=1/4" ALtair Floppy Disk)
;Automatically detects 8" versus minidisk. Note
;that Altair disks do not contain a boot sector.
;This loader loads the entire operating system.
;***********************************************
 if (A88DCD or A88MCD)

GBOCMD:	
;-----------------------------------------------------------
;Wait for user to insert a diskette into the drive 0, and
;then load that drive's head. Do this first so that the disk
;has plenty of time to settle. Note that a minidisk will
;always report that it is ready. Minidisks will hang (later
;on) waiting for sector 0F, until a few seconds after the
;user inserts a disk.
;-----------------------------------------------------------
WAITEN:	xra	a		;boot from disk 0
	out	DENABL		;..so enable disk 0

	call	CHKKBD		;abort from user?

	in	DSTAT		;Read drive status
	ani	DRVRDY		;Diskette in drive?
	jnz	WAITEN		;no: wait for drive ready

	mvi	a,HEDLOD	;load 8" disk head, or enable
	out	DCTRL		;..minidisk for 6.4 Sec

;------------------------------------------------------
;Step in once, then step out until track 0 is detected.
;The first time through, delay at least 25 ms to force
;a minimum 43 ms step wait instead of 10ms. This meets
;the 8" spec for changing seek direction. (Minidisk
;step time is always 50ms, enforced by the mninidsk
;controller hardware.) See the 88-DCDD documentation
;for details. This loop ends with hl=0.
;------------------------------------------------------
	lxi	h,(25000/24)*CPUMHz ;25 mS delay 1st time thru
	mvi	a,STEPIN	;step in once first

SEEKT0:	out	DCTRL		;issue step command

	inr	l		;After the 1st time, the
				;..following loop goes 1 time.

T0DELY:	dcx	h		;(5)
	mov	a,h		;(5)
	ora	l		;(4)
	jnz	T0DELY		;(10)24 cycles/pass

WSTEP:	in	DSTAT		;wait for step to complete
	rrc			;put MVHEAD bit in Carry
	rrc			;is the servo stable?
	jc	WSTEP		;no: wait for servo to settle

	ani	TRACK0/4	;Are we at track 00?
	mvi	a,STEPOT	;Step-out command
	jnz	SEEKT0		;no: step out another track

;------------------------------------------------------
;Determine if this is an 8" disk or a minidisk, and set
;c to the correct sectors/track for the detected disk.
;an 8" disk has 20h sectors, numbered 0-1Fh. a minidisk
;has 10h sectors, numbered 0-0Fh.
;------------------------------------------------------

;Wait for the highest minidisk sector, sector number 0Fh

 endif ;(A88DCD or A88MCD)
 if (A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;(A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
 if (A88DCD or A88MCD)

CKDSK1:	in	DSECTR		;Read the sector position

	ani	SECMSK+SVALID	;mask sector bits, and hunt
	cpi	(MDSPT-1)*2	;..for minidisk last sector
	jnz	CKDSK1		;..only while SVALID is 0

;wait for this sector to pass

CKDSK2:	in	DSECTR		;Read the sector position
	rrc			;wait for invalid sector
	jnc	CKDSK2	

;Wait for and get the next sector number

CKDSK3:	in	DSECTR		;Read the sector position
	rrc			;put SVALID in Carry
	jc	CKDSK3		;wait for sector to be valid

 endif ;(A88DCD or A88MCD)
 if (A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
	ei			;denable interrupts
 endif ;(A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)

 if (A88DCD or A88MCD)


;The next sector after sector 0Fh will be 0 for a minidisk,
;and 10h for an 8" disk. Adding MDSPT (10h) to that value
;will compute c=10h (for minidisks) or c=20h (for 8" disks).

	ani	SECMSK/2	;mask sector bits
	adi	MDSPT		;compute SPT
	mov	c,a		;..and save SPT in c

;------------------------------------------
;Set up to load
;On Entry:
;  hl=0 (DMA address & execution address)
;  c=SPT (for either minidisk or 8" disk)
;------------------------------------------
	push	h		;execution address=0 onto stack
	push	h		;INIT DMA address
 endif ;(A88DCD or A88MCD)

 if RAMHNT and (A88DCD or A88MCD)
	dad	sp		;recover RAM page address
	mvi	l,RAMBUF	;Cleverly aligned buffer
 endif ;RAMHNT and (A88DCD or A88MCD)

 if (not RAMHNT) and (A88DCD or A88MCD)
	lxi	h,RAMBUF	;Cleverly aligned buffer
 endif ;(not RAMHNT) and (A88DCD or A88MCD)

 if (A88DCD or A88MCD)
	xthl			;push buffer address, recover
				;DMA address=0

	mov	b,l		;initial sector number=0

;------------------------------------------------------
;Read current sector over and over, until either the
;checksum is right, or there have been too many retries
;  b=current sector number
;  c=sectors/track for this kind of disk
; hl=current DMA address
; top-of-stack=buffer address
; next on stack=execution address
;------------------------------------------------------
NXTSEC:	mvi	a,MAXTRYS	;(7)Initialize sector retries

;-----------------------------------------
;Begin Sector Read
;  a=Remaining retries for this sector
;  b=Current sector number
;  c=Sectors/track for this kind of disk
; hl=current DMA address
; top-of-stack=RAMBUF address
; next on stack=execution address=0
;-----------------------------------------
RDSCTR:	pop	d		;(10)get RAMBUF address
	push	d		;(11)keep it on the stack
	push	psw		;(11)Remaining retry count

 endif ;(A88DCD or A88MCD)
 if (A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;(A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
 if (A88DCD or A88MCD)


;---------------------------------------------------
;Sector Read Step 1: hunt for sector specified in b.
;Data will become avaiable 250 uS after -SVALID goes
;low. -SVALID is low for 30 uS (nominal).
;---------------------------------------------------
FNDSEC:	in	DSECTR		;(10)Read the sector position

	ani	SECMSK+SVALID	;(7)yes: mask sector bits
				;..along with -SVALID bit
	rrc			;(4)sector bits to bits <4:0>
	cmp	b		;(4)found the desired sector
				;..with -SVALID low?
	jnz	FNDSEC		;(10)no: wait for it

;-----------------------------------------------------------
;Test for DMA address that would overwrite the sector buffer
;or the stack. Do this here, while we have some time.
;-----------------------------------------------------------
	mov	a,h		;(5)high byte of DMA address
	cmp	d		;(4)high byte of RAM code addr
	jz	OVMERR		;(10)report overlay error

;--------------------------------------
;Set up for the upcoming data move
;Do this here, while we have some time.
;--------------------------------------
	push	h		;(11)DMA address for retry
	push	b		;(11)Current sector & SPT
	lxi	b,BPS		;(10)b= init checksum,
				;c= byte count for MOVLUP

;-------------------------------------------------------
;Sector Read Step 2: Read sector data into RAMBUF at de.
;RAMBUF is positioned in memory such that e overflows
;exactly at the end of the buffer. Read data becomes
;available 250 uS after -SVALID becomes true (0).
;
;This loop must be << 32 uS per pass. 
;-------------------------------------------------------
DATLUP:	in	DSTAT		;(10)Read the drive status
	rlc			;(4)new Read data Available?
	jc	DATLUP		;(10)no: wait for data

	in	DDATA		;(10)Read data byte
	stax	d		;(7)store it in sector buffer
	inr	e		;(5)Move to next buffer address
				;..and test for end
	jnz	DATLUP		;(10)loop if more data

;------------------------------------------------
;Sector Read Step 3: Move sector data from RAMBUF
;into memory at hl. compute checksum as we go.
;
;8327 cycles for this section
;------------------------------------------------
	mvi	e,SDATA and 0FFh ;(7)de= address of sector data
				 ;..within the sector buffer

MOVLUP:	ldax	d		;(7)get sector buffer byte
	mov	m,a		;(7)store it at the destination
        cmp	m		;(7)Did it store correctly?
	jnz	MEMERR		;(10)no: abort w/ memory error

	add	b		;(4)update checksum
	mov	b,a		;(5)save the updated checksum

	inx	d		;(5)bump sector buffer pointer
	inx	h		;(5)bump DMA pointer
	dcr	c		;(5)more data bytes to copy?
	jnz	MOVLUP		;(10)yes: loop

;----------------------------------------------------
;Sector Read Step 4: check marker byte and compare
;computed checksum against sector's checksum. Retry/
;abort if wrong marker byte or checksum mismatch.
;On Entry and Exit:
;   a=computed checksum
;134 cycles for for this section
;----------------------------------------------------
	xchg			;(4)hl=1st trailer byte address
				;de=DMA address
	mov	c,m		;(7)get marker, should be FFh
	inr	c		;(5)c should be 0 now

	inx	h		;(5)(hl)=checksum byte
	xra	m		;(7)compare to computed cksum
	ora	c		;(4)..and test marker=ff

	pop	b		;(10)Current sector & SPT
	jnz	BADSEC		;(10)NZ: checksum error

;Compare next DMA address to the file byte count that came
;from the sector header. Done of DMA address is greater.

	mvi	l,SFSIZE and 0FFh ;(7)hl=address of file size
	mov	a,m		;(7)low byte
	inx	h		;(5)point to high byte
	mov	h,m		;(7)high byte
	mov	l,a		;(5)hl=SFSIZE

	xchg			;(4)put DMA address back in hl
				;..and file size into de

	mov	a,l		;(4)16-bit subtraction
	sub	e		;(4)
	mov	a,h		;(5)..throw away the result
	sbb	d		;(4)..but keep Carry (borrow)

	pop	d		;(10)chuck old DMA address
	pop	d		;(10)chuck old retry count

	jnc	FDEXEC		;(10)done loading if hl >= de

;------------------------------------------------------
;Next Sector: the sectors are interleaved by two.
;Read all the even sectors first, then the odd sectors.
;
;44 cycles for the next even or next odd sector
;------------------------------------------------------
	lxi	d,NXTSEC	;(10)for compact jumps
	push	d		;(10)

	inr	b		;(5)sector=sector + 2
	inr	b		;(5)

	mov	a,b		;(5)even or odd sectors done?
	cmp	c		;(4)c=SPT
	rc			;(5/11)no: go read next sector
				;..at NXTSEC

;Total sector-to-sector = 28+8327+134+44=8533 cycles=4266.5 uS
;one 8" sector time = 5208 uS, so with 2:1 interleave, we will
;make the next sector, no problem.

 endif ;(A88DCD or A88MCD)
 if (A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
	ei			;enable interrupts
 endif ;(A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
 if (A88DCD or A88MCD)

	mvi	b,01h		;1st odd sector number
	rz			;Z: must read odd sectors now
				;..at NXTSEC

;------------------------------------------------------------
;Next Track: Step in, and read again.
;Don't wait for the head to be ready (-MVHEAD), since we just
;read the entire previous track. Don't need to wait for this
;step-in to complete either, because we will definitely blow
;a revolution going from the track's last sector to sector 0.
;(One revolution takes 167 mS, and one step takes a maximum
;of 40 uS.) Note that NXTRAC will repair the stack.
;------------------------------------------------------------
	mov	a,b		;STEPIN happens to be 01h
	out	DCTRL

	dcr	b		;start with b=0 for sector 0
	ret			;go to NXTSEC

;-------------------------------------------------
;Execute successfully loaded code, after disabling
;the floppy drive and disabling the PROM
;On Entry:
;  Top of stack=RAMBUF address
;  Next on stack=execution address
;-------------------------------------------------
FDEXEC:	mvi	a,DDISBL	;Disable floppy controller
	out	DENABL

	pop	d		;chuck RAMBUF address
				;..to expose exec address

 endif ;(A88DCD or A88MCD)
 if (A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;(A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
 if (A88DCD or A88MCD)


	jmp	EXECDP		;disable PROM and execute code

;---Error Routine------------------------------------------
;Checksum error: attempt retry if not too many retries
;already. otherwise, abort, reporting the error 
;On Entry:
;  Top of stack=adress for first byte of the failing sector
;  next on stack=retry count
;----------------------------------------------------------
BADSEC:	mvi	a,HEDLOD	;Restart Minidisk 6.4 uS timer
	out	DCTRL

 endif ;(A88DCD or A88MCD)
 if (A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
	ei			;enable interrupts
 endif ;(A88DCD or A88MCD) and (ENINTS or CRXINT or TRXINT)
 if (A88DCD or A88MCD)


	pop	h		;Restore DMA address
	pop	psw		;get retry count
	dcr	a		;Any more retries left?
	jnz	RDSCTR		;yes: try reading it again

;Irrecoverable error in one sector: too many retries.
;these errors may be either incorrect marker bytes,
;wrong checksums, or a combination of both.

;Fall into CSMERR

 endif ;A88DCD or A88MCD

;---Error Routine--------------------------------------
;Checksum Error (for both HL and BO commands)
;On Entry:
;  if ROM2K: hl=RAM adress for first byte of the failed
;     sector, or last address of the Intel Hex block
;------------------------------------------------------
CSMERR:	call	CILPRT
	db	'Cs','m'+80h

 if ROM2K
	jmp	RPTERR		;go give details
 endif ;ROM2K

 if not ROM2K
	jmp	CMDERR		;error handler
 endif ;not ROM2K

;---Error Routine----------------------------------
;Memory Overwrite Error: Attempt to overwrite stack
;(for both HL and BO commands)
;On Entry:
;   if ROM2K: hl=offending RAM address
;--------------------------------------------------
OVMERR:	call	CILPRT
	db	'Ad','r'+80h

 if ROM2K
	jmp	RPTERR		;go give details
 endif ;ROM2K

 if not ROM2K
	jmp	CMDERR		;error handler
 endif ;not ROM2K

 if ROM2K
;---Error Routine---------------------
;Memory Error: memory readback failed
;On Entry:
;   if ROM2K: hl=offending RAM address
;-------------------------------------
MEMERR:	call	CILPRT
	db	'RA','M'+80h

;Fall into RPTERR

;---Error Routine--------------------------------------
;Report An error: turn the disk controller off, report
;the error on the Console and jump to the Console loop.
;On Entry:
;   hl=offending RAM address
;   sp=valid address in RAM page
;------------------------------------------------------
RPTERR:

 endif ;ROM2K
 if A88DCD or A88MCD

	mvi	a,DDISBL	;Disable floppy controller
	out	DENABL

 endif ;A88DCD or A88MCD
 if ROM2K

	call	ILPRNT
	db	' error at',' '+80h
	call	PHLCHX		;print hl in hex on Console

	jmp	CABORT		;Repair stack, go to MAIN

 endif ;ROM2K

;***Command Routine Continuation**********
;BO (Boot from CCS 2422 Floppy Controller)
;*****************************************
 if CC2422

GBOCMD:	mvi	c,MAXTRYS+1	;sector retry counter

	mvi	a,DCMDMO+(1 shl BOTDSK)
	out	DCTRL		;Assume minidisk for restore
	mov	d,a

	mvi	a,SIDE0		;set side 0
	out	BCTRL

	mvi	a,RESTOR	;Restore to track 0
	out	DCMMD
	mvi	b,SNORDY	;(7)The only possible error
	call	EXCCHK		;(17)test WD1793 status (17)

;Check for 8" disk and change setup if that's what we have

	in	BSTAT		;read BSTAT while on track 0
	ani	BTRK0		;1 if 8" disk while on track 0
	jz	REBO1

	mvi	a,DCMAXI	;set up for an 8" disk by
	ora	d		;..setting the maxidisk bit
	out	DCTRL
	mov	d,a		;remember DCTRL state
REBO1:

;Set up to read the boot sector

RBRTRY:	mvi	a,BOTSEC	;Sector to boot
	out	DSCTR		;WD1793 sector port

	lxi	h,BOTADR	;Put boot sector here

;Read boot sector into RAM at hl

 endif ;CC2422
 if CC2422 and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;CC2422 and (ENINTS or CRXINT or TRXINT)
 if CC2422

	mov	a,d		;(13)select
	ori	DCAUTOW		;(7)enable auto-wait
	out	DCTRL		;(11)output DCTRL value

	mvi	a,RDSECT	;read command
	out	DCMMD		;(11)disk command port


RDSLUP:	in	DDATA		;(10) 32 cycles in-to-in
	mov	m,a		;(7)
	inr	l		;(4)
	jm	RDSLUP		;(10)

	mvi	b,SNORDY+SRNFER+SCRCER+SLOSTD
	call	EXCCHK		;test WD1793 status

;Execute the loaded code if it's ok

	jz	BOTADR

;----------------------------------
;Read fail. Try again if possible.
;Give user feedback about retries.
;  c=retry count-down
;----------------------------------
	mov	b,a		;error code

	mvi	a,MAXTRYS+1
	cmp	c		;first retry?
	jnz	RBR1		;n: just print dot

	call	CILPRT		;Enables ints too, if ENINTS
	db	'Retryin','g'+80h

RBR1:	call	ILPRNT
	db	'.'+80h

	dcr	c		;too many retries?
	jnz	RBRTRY

;Fall into BOTERR

;---Fail-------------------------
;Fatal error when trying to boot.
;Report, and return to prompt
;On Entry:
;  b=error code
;--------------------------------
BOTERR:	call	CILPRT
	db	'Boot failed:',' '+80h

	mov	a,b		;error code
	call	PCAHEX

	call	ILPRNT
	db	'h'+80h

	jmp	CABORT

;---Local Subroutine----------
;Check disk controller status
;On Entry:
;  b=error mask
;On Exit:
;  Z set if no errors
;  Z clear if error or timeout
;Trashes a,e,hl
;-----------------------------
EXCCHK:	lxi	h,400*CPUMHz	;(7)about 2 seconds
				;(e a little random at first)

ECLOOP:	dcx	h		;(5)
	mov	a,l		;(5)
	ora	h		;(4)
	jz	TIMOUT		;(10)controller timeout?

ECLUP2:	dcr	e		;(5)inner loop: 9984 cycles
	jz	ECLOOP		;(10)

	in	DFLAG		;(10)wait for command to end
	rrc			;(4)test DFSEOJ
	jnc	ECLUP2		;(10) 39 cycles/pass

;Delay enough that we have at least 56 uS.
;We have 94 cycles already in the code.
;(See WD179x Application Notes, November 1980)

	mvi	e,3*CPUMHz	;(7)

ELDLP:	dcr	e		;(4)
	jnz	ELDLP		;(10) 14 cycles/pass

;check controller status

	in	DSTAT		;get status
	ani	b		;mask errors
	rp			;msb is drive not ready bit

;Fall into DNRERR

;---Fail---------------------
;Drive not ready.
;Report, and return to prompt
;----------------------------
DNRERR:	call	CILPRT
	db	'Drive not read','y'+80h
	jmp	CABORT

;---Fail---------------------
;Controller timeout.
;Report, and return to prompt
;----------------------------
TIMOUT:	call	CILPRT
	db	'FDC timeou','t'+80h

	jmp	CABORT

 endif ;CC2422

;***Command Routine Continuation***************************
;BO (Boot from Cromemco 4FDC/16FDC/64FDC Floppy Controller)
;On Entry:
;  hl=1st value after the command
; We allow booting from any valid drive because CDOS
; (and probably Cromix) can boot from any drive.
;**********************************************************
 if (C4FDC or C16FDC or C64FDC)

GBOCMD:	mvi	c,MAXTRYS+1	;sector retry counter

;Get the specified boot drive, if any, and set up the
;DCTRL value in d

	mov	a,l
	cmp	MAXDRV+1	;valid drive specified?
	jnc	CMDERR		;n: bogus

	mvi	a,80h		;compute drive select

DSLOOP:	rlc
	dcr	l
	jp	DSLOOP

	ori	DCMAXI or DCMOTO ;complete select bits
	mov	d,a

RBRTRY:	mov	a,d
	out	DCTRL		;select disk & maxi/mini

	mvi	a,7Fh		;side 0, slow-seek, etc.
	out	ACTRL

;Restore to track 0

	mvi	a,RESTOR	;restore to track 0
	out	DCMMD

	mvi	b,SNORDY	;(7)The only possible error
	call	EXCCHK		;test WD1793 status (17)

;Set up to read boot sector

	mvi	a,BOTSEC	;Boot sector
	out	DSCTR		;set sector reg

	lxi	h,BOTADR	;address to load & run

	mov	a,d		;select bits
	ori	DCAUTO		;turn on auto-wait
	out	DCTRL

;Read the sector

 endif ;(C4FDC or C16FDC or C64FDC)
 if (C4FDC or C16FDC or C64FDC) and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;(C4FDC or C16FDC or C64FDC) and (ENINTS or CRXINT or TRXINT)
 if (C4FDC or C16FDC or C64FDC)

	mvi	a,RDSECT
	out	DCMMD

;32 cycles=16 uS per byte (no problem for SD track)

DDREAD:	in	DDATA		;(10)get a byte
	mov	m,a		;(7)store it in RAM
	inr	l		;(5)
	jm	DDREAD		;(10)

;check status

	mvi	b,SNORDY+SRNFER+SCRCER+SLOSTD
	call	EXCCHK		;test WD1793 status

;Execute the loaded code if it's ok

	jz	BOTADR

;----------------------------------------
;Read fail. Try again with the other size
;size disk (maxi vs mini) if possible.
;Give user feedback about retries.
;  a=error code
;  c=retry count-down
;  d=select bits with mini/maxi
;----------------------------------------
	mov	b,a		;error code

	mvi	a,DCMAXI
	xra	d		;toggle maxi/mini
	mov	d,a

	ani	DCMAXI		;report only when
	jz	RBRTRY		;..retrying maxidisk

	mvi	a,MAXTRYS+1
	cmp	c		;first retry?
	jnz	RBR1		;n: just print dot

	call	CILPRT
	db	'Retryin','g'+80h

RBR1:	call	ILPRNT
	db	'.'+80h

	dcr	c		;too many retries?
	jnz	RBRTRY

;Fall into BOTERR

;---Fail--------------------------
;Fatal error when trying to boot.
;Report, and return to the prompt.
;On Entry:
;  b=error code
;---------------------------------
BOTERR:	call	CILPRT
	db	'Boot failed:',' '+80h

	mov	a,b		;error code
	call	PCAHEX

	call	ILPRNT
	db	'h'+80h

	jmp	CABORT

;---Local Subroutine---------------------------
;Check disk controller status
;On Entry:
;  b=error mask
;On Exit:
;  Z set if no errors
;  Z clear and a=error code if error or timeout
;Trashes a,e,hl
;----------------------------------------------
EXCCHK:	lxi	h,400*CPUMHz	;(7)about 2 seconds
				;(e a little random at first)

ECLOOP:	dcx	h		;(5)
	mov	a,l		;(5)
	ora	h		;(4)
	jz	TIMOUT		;(10)controller timeout?

ECLUP2:	dcr	e		;(5)inner loop: 9984 cycles
	jz	ECLOOP		;(10)

	in	DFLAG		;(10)wait for command to end
	rrc			;(4)test DFSEOJ
	jnc	ECLUP2		;(10) 39 cycles/pass

;Delay enough that we have at least 56 uS (224 cycles)
;for a 4 MHz CPU. We have 94 cycles already in the code.
;(See WD179x Application Notes, November 1980)

	mvi	e,10		;(7)

ELDLP:	dcr	e		;(4)
	jnz	ELDLP		;(10) 14 cycles/pass

;Check controller status

	in	DSTAT		;get status
	ani	b		;mask errors
	rp			;msb is drive not ready bit

;Fall into DNRERR

;---Fail-----------------------
;Drive not ready.
;report, and return to Memon/80
;------------------------------
DNRERR:	call	CILPRT
	db	'Drive not read','y'+80h

	jmp	CABORT

;---Fail-----------------------
;controller timeout.
;report, and return to Memon/80
;------------------------------
TIMOUT:	call	CILPRT
	db	'FDC timeou','t'+80h

	jmp	CABORT

 endif ;(C4FDC or C16FDC or C64FDC)

;***Command Routine Continuation***************
;BO (Boot from IMSAI FIF Floppy controller)
;(This code based on ABOOTSIM.ASM in the "IMSAI
;CP/M System User's Guide Version 1.31 Rev.2",
;3/21/77, page CP/M 2 - 18.)
;**********************************************
 if IFIF
GBOCMD:	mvi	c,MAXTRYS+1	;retry counter

	mvi	a,ISSPTR	;Set string pointer 0
	out	IDISK

;Set Floppy Disk Interface string pointer

	lxi	h,IBCMD		;Point hl at command string
	mov	a,l		;lo
	out	IDISK
	mov	a,h		;hi
	out	IDISK

;Set up string in RAM

	mvi	m,IRDSEC	;Read sector 0, unit 0
	inx	h		;Point at status byte
	inx	h		;Point at high order track
	xra	a		;Get 0 in a
	mov	m,a		;Zero high order track
	inx	h
	mov	m,a		;Zero lo order track
	inx	h
	mvi	m,1		;Sector 1
	inx	h
	mov	m,a		;Zero low order buffer address
	inx	h
	mov	m,a		;Zero high order buf address

;Initialization complete. Now read sector.

RBRTRY:	lxi	h,IBSTAT
	xra	a

	mov	m,a		;Stat must be 0 before command

;N.B. a=0 is disk command IEXEC0 to execute string 0

	out	IDISK		;Tell disk to go
IDWAIT:	add	m		;Look for non-0 status
	jz	IDWAIT		;Keep looking 'till it comes

	cpi	ISUCCS
	jz	0		;On success, go to bootstrap
				;routine read from disk

;--------------------------------------------
;Read fail. Restore the disk and try again if
;we can. Give user feedback about retries.
;  m=error code
;  c=retry count-down
;--------------------------------------------
	mvi	a,IRESTR	;Restore the disk
	out	IDISK

	mvi	a,MAXTRYS+1
	cmp	c		;first retry?
	jnz	RBR1		;n: just print dot

	call	CILPRT
	db	'Retryin','g'+80h

RBR1:	call	ILPRNT
	db	'.'+80h

	dcr	c		;too many retries?
	jnz	RBRTRY

;Fall into BOTERR

;---Fail--------------------------
;Fatal error when trying to boot.
;report, and return to the prompt.
;On Entry:
;  m=error code
;---------------------------------
BOTERR:	call	CILPRT
	db	'Boot failed:',' '+80h

	mov	a,m		;error code
	call	PCAHEX

	call	ILPRNT
	db	'h'+80h

	jmp	CABORT
 endif ;IFIF

;***Command Routine Continuation************
;BO (Boot from Micropolis Floppy Controller)
;*******************************************
 if MICROP

;Copy Read-Sector subroutine into RAM,
;since DOS expects it there.

GBOCMD:	lxi	d,RDSROM	;source
	lxi	h,RDSEC		;destination
	mvi	c,RDSLEN	;byte count

COPYB:	ldax	d
	mov	m,a
	inx	h
	inx	d
	dcr	c
	jnz	COPYB

	lxi	h,FDCMD		;command register address
	mvi	m,FRESET	;reset FDC

	mvi	b,MAXTRYS+1	;b=max read retries

;--------------------------------------
;Find and load track 0, sector 0, using
;the FDC we just found.
;On Entry:
;   b=retry counter
;   c=0
;   hl=command register address
;--------------------------------------

;Select drive 0 and give the drive time to spin up.
;The drive may not be spinning if the FDC has been
;modified with the motor-off mod. DELSTA will read
;the FDC's status register every pass, resetting
;the motor-off mod's 4-second timer.

	mvi	m,SLUN+0	;select drive zero

	mvi	d,85		;delay for 85*5.89=500 mS
	call	DELSTA		;returns a=status reg value
				;returns de=0	

RETRY:	dcr	b		;too many read retries?
	jz	RDERR		;y: read error
	push	b		;b=retries

;Verify drive is ready. If the controller board has been
;modified with the motor-off mod, then its RDY line is
;always true. So rather than check the drive ready bit,
;look at the sector register to see that it is changing,
;indicating that a disk is spinning. (We don't care if the
;sector is valid - just that it is changing.)

	mov	c,m		;c=1st check of sector register

	mvi	d,17		;delay for 17*5.89=100.3 mS
	call	DELSTA		;..1/2 revolution

	mov	a,m		;a=2nd check of sector register
	xra	c		;did sector register change?
				;any bits changing in sect reg
				;..indicate a spinning disk.

	jz	DNRERR		;same: drive not ready	

;Home the head: Step in to get off track 0, and then seek to
;track zero. The number of step-ins is equal to the remaining
;retry counter, meaning it will try more times to get off the
;track 0 sensor on early retries than on late retries. In
;fact, it should always take just one step to get off the
;track 0 sensor. If we happen to start out on the last track,
;then we will do one more step out, which is okay.
;hl=FDC command register address for HSTEP
;b=retries here
;e=0 here

;step away from track 0

	lxi	b,900h+STEP+STEPI ;b=8 steps, c=step in

UNSTUK:	call	HSTEP		;exits with de=0,
				;..a=status reg value
	ani	TK0		;still at track zero?
	jnz	UNSTUK		;y: try stepping away again

;seek track 0

	lxi	b,(90*256)+STEP+STEPO ;b=90 steps max,
				      ;..c=step out

SEEK0:	call	HSTEP		;exits with de=0,
				;..a=status reg value
	ani	TK0		;at track zero yet?
				;returns with de=0
	jz	SEEK0		;n: step out again

;Read sector zero into a RAM buffer (MICBUF)
;d=0
;hl=FDC command register address for RDSECH

	mvi	e,MICBUF	;de=RAM buffer addr in page 0
	call	READ0		;hl=control register
				;Z set if ok

;Unless the 1st read failed, get the load address for
;this sector from the sector data and re-read the same
;sector into the requested address

	xchg			;de=FDC address
	lhld	MICBUF+PTROFF	;requested load address
	xchg
	cz	READ0		;hl=control register

	pop	b		;b=retry counter, c=0
	jnz	RETRY		;failed, start over

;Successful load. Create stack for DOS, save addresses
;DOS uses (Note: we can't create DOS's stack until we
;are sure we will not return to MAIN.)

	lxi	sp,LODADR+2	;create stack for DOS
	push	d		;LODADR gets load address
	push	h		;FDCADR gets FDC address
	dcr	h		;compute ROM address
	dcr	h		;200h below FDC reg address
	push	h		;ROMADR	
	
;Go execute the loaded code, with registers and the sp
;where the onboard ROMs would have left them.

	xchg			;hl=LODADR
	pop	d		;de=ROMADR (for DOS)

	lxi	b,eXEOFF	;execute 12 bytes into sector
	dad	b		;hl=execution address

; sp=ROMADR+2=00A2
; de=address of onboard ROM
; hl=code exec address (12-bytes into sector data in RAM)
;
;00a0: ROMADR=address of onboard ROM
;00A2: FDCADR=address of FDC registers
;00A4: LODADR=address beginning of sector in RAM

 endif ;MICROP
 if MICROP and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;MICROP and (ENINTS or CRXINT or TRXINT)
 if MICROP

	pchl			;execute boot loader from disk

;---Local Subroutine--------------------------------------
;Read and validate sector 0
;On Entry:
;  hl=FDC data register address
;  de=target address
;On Exit:
;  a=0 and Z set = read successful
;  a<>0 and Z clear = read fail (checksum or wrong sector)
;  c=0
;Trashes b
;---------------------------------------------------------
READ0:	push	d		;buffer address

	lxi	b,SCLEN/2	;b=sector 0, c=bytes to read/2
				;hl=FDC command reg
	call	RDSECH		;read the sector into (de)
				;returns a<>0 if read error
				;returns c=0

	pop	h		;buffer address
	mov	e,c		;de=FDC address (c=0)

;Verify metadata from the loaded sector says track 0, sector 0
;(a=0 if RDSECH thought the read was successful)

	ora	m		;a=track number from disk data
	inx	h		;(hl)=sec number from disk data
	ora	m		;verify track and sec are zero
	dcx	h		;hl=buffer address

	xchg			;put pointers back
	ret

;---Local Subroutine-------------------
;Step head Once
;On Entry:
;  b=retry counter
;  c=command (Step in or step out)
;  e=0
;  hl=command register address
;On Exit:
;  Abort (rudely) if b decremetned to 0
;  a=status register value
;  b decremented
;  de=0
;--------------------------------------
HSTEP:	dcr	b
	jz	T0FAIL		;Can't find track 0
				;..or get off track 0

	mov	m,c		;Step in or out now

;delay for >40 mS

	mvi	d,7		;delay for 7*5.89=41.2 mS

;Fall into DELSTA to delay and read the status register

;---Local Subroutine---------------------------------
;Delay 5.89 mS times value specified in d, and return
;the FDC status register value. Looks at 2mhz/4mhz
;jumper on the FDC to determine CPU speed.
;
;On Entry:
;  d=delay value in 5.89 mS units (max value=127)
;  e=0 (low byte of delay counter, really)
;  hl=address of FDC control register
;On Exit:
;  a=status register value
;  de=0
;----------------------------------------------------
DELSTA:	mov	a,m		;a=sector register from FDC
	ani	DTMR		;see if 4mhz jumper in place	
	mov	a,d		;a=requested delay
	jnz	NOT4M		;not 4mhz
	rlc			;double the count for 4mhz CPU

NOT4M:	mov	d,a		;save adjusted delay count

;we re-read the status register every pass for delay

DLOOP:	dcx	d		;(5)
	mov	a,e		;(5)
	ora	d		;(4)
	inx	h		;(5) point to status register
	mov	a,m		;(7) read status register
	dcx	h		;(5) point to control register
	rz			;(5 not taken)
	jmp	DLOOP		;(10) 46 cycles/pass

;---Error Routines---------
;These all repair the stack
;and return to the prompt.
;--------------------------
T0FAIL:	call	CILPRT		;stuck or lost
	db	'Track ','0'+80h
	jmp	PFAIL		;adds ' error' and aborts

RDERR:	call	CILPRT		;too many retries on reading
	db	'Rea','d'+80h
	jmp	PFAIL		;adds ' error' and aborts

NCERR:	call	CILPRT		;can't find FDC
	db	'FD','C'+80h

;Fall into PFAIL

PFAIL:	call	ILPRNT
	db	' fai','l'+80h

	jmp	CABORT

DNRERR:	call	CILPRT		;no disk probably
	db	'Drive not read','y'+80h
	jmp	CABORT

;---RAM Subroutine--------------------------------------
;Read sector into RAM
;This code gets moved into RAM at RDSEC
;
;On Entry at RDSEC:
;  de=RAM buffer
;  b=sector to read
;  c=number of bytes to read/2
;On Entry at RDSEC+3:
;  hl=FDC control/sector register address
;  de=RAM buffer
;  b=sector to read
;  c=number of bytes to read/2
;On Exit:
;  a=0 and Z set = read successful
;  a<>0 and Z clear = checksum error
;  c=0
;  de=FDC data register address
;  hl=next RAM address past this sector's data
;Trashes b
;-------------------------------------------------------
RDSROM:	lhld	FDCADR		;FDC registers
				;(sector register)

wtSec:	mov	a,m		;sector status
	ani	SFLG		;wait for sector true
	jz	wtSec-OFFSET

	mov	a,m		;re-read to be safe
	ani	SMASK		;get sector number alone
	xra	b		;right sector number?
	jnz	wtSec-OFFSET	;no, keep looking

	inx	h		;hl=status register address

 endif ;MICROP
 if MICROP and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;MICROP and (ENINTS or CRXINT or TRXINT)
 if MICROP

wtXfer:	ora	m		;wait for transfer flag (bit 7)
	jp	wtXfer-OFFSET	;(10)

	inx	h		;(5)hl=data register address
	mov	a,m		;(7)read & chuck sync byte
	xra	a		;(4)start with carry clear
	xchg			;(4)de=data register address
				;hl=target RAM address

	mvi	b,0		;(7)initialize checksum
	nop			;(4)timing for FDC stall logic
	nop			;(4)

;Read sector data from disk into RAM two bytes at a time
;de=FDC data register
;hl=destination RAM address

movSec:	ldax	d		;read data byte
	mov	m,a		;save in buffer
	inx	h
	adc	b		;update checksum
	mov	b,a

	ldax	d		;repeat for 2nd byte
	mov	m,a
	inx	h
	adc	b
	mov	b,a

	dcr	c		;decrement byte count
	jnz	movSec-OFFSET	;repeat until done
 endif ;MICROP
 if MICROP and (ENINTS or CRXINT or TRXINT)
	ei			;enable interrupts
 endif ;MICROP and (ENINTS or CRXINT or TRXINT)
 if MICROP

;Read and verify checksum

	ldax	d		;a=checksum from disk
	xra	b		;compare to computed checksum	
	ret			;return result

RDSEND	equ	$
RDSLEN	equ	RDSEND-RDSROM
OFFSET	equ	RDSROM-RDSEC	;address offsets
MICBUF	equ	RDSEND-OFFSET	;Micropolis Sec RAM buffer

 endif ;MICROP

;***Command ROutine Continuation**************************
;BO (Boot from Northstar Double-Density Floppy Disk drive,
;   with either a single- or double-density disk)
;*********************************************************
 if NSTARD

GBOCMD:	lxi	b,0D00h+MAXTRYS	;Initialize c=retry counter
				;and b=index hunt counter

;----------------------------------------------------------
;Start the spindle motors, select drive 1, and delay enough
;time for the motor to spin up and the head to settle
;----------------------------------------------------------
	lda	CTLCMD+CTLAS+CTLMO ;Motor on

	lxi	d,3008h		;d=48 secs: motor spin-up delay
				;e=8: step-ins
	call	WSECTD

	lxi	h,CTLORD+ORDSIN+ORDDS1	;Select drive 1
					;..(step in mode)
	mov	a,m

;-------------------------------------------------------
;Wait for the index mark to ensure a disk is installed.
;(The MDS-D will generate fake sector pulses if not.)
;b=0DH here, for 12 sectors to try
;hl=CTLORD+ORDSIN+ORDDS
;-------------------------------------------------------
IWLOOP:	dcr	b		;Too many sectors?
	jz	INFERR		;y: no index hole

	call	WSECT1		;wait for next sector

	lda	CTLCMD+ORDST	;Get A-Status
	ani	SAIX		;Index hole?
	jz	IWLOOP		;n: keep looking

;----------------------
;Home: Seek track 0
;e=8 here
;hl=CTLORD+ORDSIN+ORDDS
;----------------------

;If already on track 0, step away.

HOME:	lda	CTLCMD+CTLBS		;Read B-Status
	rar				;test SBT0: On track 0?
	cnc	STEP

	jnc	T0STUK			;Stuck on track 0?

;Step out to track 0
;h=E8h here

	mvi	l,ORDDS1	;step out
	mov	e,h		;max number of tries
	call	STEP		;Step out to track 0
	jc	T0FAIL		;can't find track 0

;---------------------------------------------
;Hunt: Wait for the double-density boot sector
;h=E8h here
;---------------------------------------------
	mov	b,h		;long timeout for several loops

HUNT:	dcr	b		;timeout?
	jz	SNFERR

	call	WSECT1		;Wait for next sector

	lda	CTLCMD+CTLCS+CTLMO	;Read C-Status
	ani	SCSM			;Sector mask
	cpi	DDBSCR			;Found our boot sector?
	jnz	HUNT			;N: Keep looking

;Wait for hardware to say that read is enabled
;(timer b should be between DBh and E8h - huge)

WAITRD:	dcr	b		;(5)timeout?
	jz	SNFERR		;(10)

	lda	CTLCMD+CTLAS	;(13)Read A-Status
	ani	SARE		;(7)Read enabled?
	jz	WAITRD		;(10)n: wait more

;Stall for about 68 uS on a 2 MHz 8080
;or about 34 uS on a 4 MHz Z80

	mvi	e,9
STAL68:	dcr	e		;(5)
	jnz	STAL68		;(10)end with e=0

;------------------------------------
;Check for single-density disk, which
;loads from a different sector
;e=0 here
;h=E8h here
;------------------------------------
	lda	CTLCMD+CTLAS	;Read A-Status
	ani	SADD		;Double-density disk?
	jnz	GETSEC		;y: go read the sector

;---------------------------------------------
;Single-density boot
;Step in to track 1 and hunt for sector 8
;(Sector 8 is a special 512-byte boot sector.)
;---------------------------------------------
	inr	e		   ;e=1
	mvi	l,ORDSIN+ORDDS1	   ;Step in once to track 1
	call	STEP

SDRSCT:	call	WSECT1		   ;Wait for 1 sector time

	lda	CTLCMD+CTLCS+CTLMO ;Read C-Status
	ani	SCSM		   ;Sector mask
	cpi	SDBSCR		   ;single-density boot sector
	jnz	SDRSCT

;Fall into GETSEC

;----------------------------------------
;Wait for the sector body.
;Time out after a ridiculously long time.
;b still has a large timeout value here
;h=E8h here
;----------------------------------------
GETSEC:	lxi	d,CTLCMD+CTLRD	;Select Read Data

 endif ;NSTARD
 if NSTARD and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;NSTARD and (ENINTS or CRXINT or TRXINT)
 if NSTARD

WATSEC:	dcr	b		;(5)timeout?
	jz	NSERR		;(10)

	lda	CTLCMD+CTLAS	;(7)Read A-Status
	rrc			;(4)SABD set?
				;..(Sync chr detected)
	jnc	WATSEC		;(10)n: keep waiting

;------------------------------------------------------------
;Read and validate the sector data
;  Byte 0: <PA> High byte of target RAM address
;  Bytes 1-511: Sector data
;  Byte 512: CRC of Bytes 0-511
;Bytes1-511 get written into RAM at <PA>01 through <PA+1>FF.
; <PA> gets written to <PA>01 before getting overwritten by
; byte 1. Note that <PA>00 never gets written
;------------------------------------------------------------
	ldax	d		;read 1st disk data byte
	mov	h,a		;This is the target RAM page

	mvi	l,1		;byte 1 in that page
	mov	m,a		;save page address (why?)

	rlc			;Start CRC calculation
	mov	b,a		;b accumulates CRC

;Read the next 255 bytes (the remainder of the data that goes
;into the 1st RAM page) into the specified RAM page

RLOOP1:	ldax	d		;get a disk data byte
	mov	m,a		;write it to RAM
	xra	b		;compute CRC
	rlc
	mov	b,a		;..in b
	inr	l		;end of page data?
	jnz	RLOOP1		;n: get another byte

;Read the final 256 bytes into the next RAM page

	inr	h		;Next RAM page

RLOOP2:	ldax	d		;get a disk data byte
	mov	m,a		;write it to RAM
	xra	b		;compute CRC
	rlc
	mov	b,a		;..in b
	inr	l		;end of sector data?
	jnz	RLOOP2		;n: keep reading

	ldax	d		;Get CRC byte
	xra	b		;Does it match?
	jnz	NSERR		;n: retry if possible

;-----------------------------------------
;Success. Go execute the loaded code image
;Like the MDS-D boot ROM, d contains the
;high byte of the disk controller address,
;in case the loaded code looks for this.
;-----------------------------------------
	dcr	h		;get execution address
	mvi	l,0ah		;10th byte of load page

	pchl			;Go execute loaded code

;---Local Subroutine-----------------
;Wait 1 sector time (for next sector)
;On Exit:
;  a=0
;  d=0
;  Sector flag is cleared
;------------------------------------
WSECT1:	mvi	d,1

;Fall into WSECTD

;---Local Subroutine---------------
;Wait d sector times
;On Entry:
;  d=number of sector tines to wait
;On Exit:
;  a=0
;  d=0
;  Sector flag is cleared
;----------------------------------
WSECTD:	lda	CTLCMD+CTLAS+CTLRSF ;Reset sector flag

WSECTR:	lda	CTLCMD+CTLAS	    ;Read A-Status
	ora	a		    ;Test SASF (Sector flag)
				    ;clears carry too
	jp	WSECTR		    ;Wait for sector

	dcr	d		    ;next sector
	lda	CTLCMD+CTLAS+CTLRSF ;Reset sector flag
	jnz	WSECTD		    ;more sectors to wait?

	ret

;---Local Subroutine-------------
;Step e tracks
;On Entry:
;  e=number of steps
;  hl=step-in or step-out command
;On Exit:
;  Carry set if on track 0
;Trashes a,de,l
;--------------------------------
STEP:	mov	a,m		;issue command (Step level low)
	push	h		;remember original command

	mov	a,l		;set step level high
	ori	ORDST
	mov	l,a
	mov	a,m		;Step level high

	pop	h		;original command
	mov	a,m		;step level low again

	mvi	d,2		;Wait for 2 sector times	
	call	WSECTD

	lda	CTLCMD+CTLBS	;Read B-Status
	rar			;test SBT0: On track 0?
	rc			;y: done with Carry set

	dcr	e
	jnz	STEP

	ret			;with carry clear

;---Error Routines---------
;These all repair the stack
;and return to the prompt.
;--------------------------
NSERR:	dcr	c		;More retries left?
	jnz	HOME		;y: try again

SNFERR:	call	CILPRT
	db	'Secto','r'+80h
	jmp	NFERR		;recycle some code

INFERR:	call	CILPRT
	db	'Inde','x'+80h
	jmp	NFERR		;recycle some code

T0STUK:T0FAIL:	call	CILPRT
	db	'Track ','0'+80h

;Fall into NFERR

NFERR:	call	ILPRNT
	db	' erro','r'+80h
	jmp	CABORT

 endif ;NSTARD

;***Command ROutine Continuation********************
;BO (Boot from Northstar Single-Density Floppy Disk)
;***************************************************
 if NSTARS

GBOCMD:	mvi	b,MAXTRYS	;INit retry count (10)

;(Re)load the boot sector
;b=remaining retries. Stack is valid
;turn on the drive motor and select the boot drive

RETRY:	lda	CTLMO or CTLNOP	;Motors on & a status
	ani 	SAMO		;Motor already on? (retry?)
	jnz	SELECT		;Yes, no need to wait

	mvi	d,32h		;Wait 32 sector times
	call	WSECTS		;..for motor to spin up

SELECT:	lda	CTLDS0 or BDRIVE ;Select boot drive
	mvi	d,0DH		;Wait 13 sector tiems
	call	WSECTS		;..for head to settle

;Step in a few tracks in case we are lost

	mvi	e,17		;max step-ins+1

STIN:	dcr	e
	jz	T0STUK		;track 0 sensor stuck?

	lxi 	h,CTLSTI	;step-in command
	mvi	c,1		;step 1 track inward
	call	STEP
	jnz	STIN		;still on track 0?

;Step out to track 0

	lxi 	h,CTLSTO	;step-out command
	mvi	c,59h		;max 59 steps outward
	call	STEP
	jz	T0FAIL		;Can't find track 0

;------------------------
;Hunt for the boot sector
;------------------------
HUNT:	mvi	d,1		;Wait one sector time
	call	WSECTS

	lda	CTLBST or CTLNOP ;Read B-status
	ani	SBSECT		;Get sector number
	cpi	BSECTR		;Desired sector?
	jnz	HUNT		;N: wait for it

;---------------
;Set up for load
;---------------
	lxi	h,LODADR	;RAM load address
	mvi	c,8dh		;Timeout count

;-------------------------------------------------------
;Issue read command, and wait for the boot sector's data
;--------------------------------------------------------
	lxi	d,CTLRD or CTLNOP ;Read data command

WATSEC:	dcr	c		;timeout?
	jz	SNFERR

	lda	CTLNOP		;Get A-status
	ani	SABDY		;Wait for Body
	jz	WATSEC		;n: keep waiting

 endif ;NSTARS
 if NSTARS and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;NSTARS and (ENINTS or CRXINT or TRXINT)
 if NSTARS
;----------------------------------
;Read 256-byte sector data into RAM
;This loop assumes the load address
;low byte is 00.
;----------------------------------
	mov	c,l		;initial CRC (l=0)

RDLOOP:	ldax	d		;Read data byte
	mov	m,a		;Write to RAM

	xra	c		;Compute CRC
	rlc
	mov	c,a

	inr	l		;Next RAM address
	jnz	RDLOOP		;Read them all

	ldax	d		;Read CRC
	xra	c		;Does it match?
	jnz	CRCERR		;n: CRC error

;-----------------------
;Execute the loaded code
;-----------------------
	jmp	EXEADR

;--------------------------------
;CRC fail: retry sector if we can
;--------------------------------
CRCERR:	dcr	b		;any retries left?
	jnz	RETRY

;Too many retries

	call	CILPRT
	db	'CR','C'+80h
	jmp	RPTERR

;---Local Subroutine-------------
;Step specified number of tracks
;On Entry:
;  c=number of tracks to step
;  hl=step in or step out command
;On Exit:
;  Z clear if on track 0
;trashes a,c,d
;--------------------------------
STEP:	mov	a,m		;Set direction

STPLUP:	lda	CTLSTS		;Set track-step flip flop
	xthl			;Delay
	xthl			;Delay
	lda 	CTLSTC		;Clear track-step flip flop

	mvi	d,2		;Wait 2 sector times
	call	WSECTS

	lda	CTLNOP		;Read A-status
	ani	SATR0		;At track 0?
	jz	STEP1		;No, keep stepping

	ret			;Z clear: On track 0

STEP1:	dcr	c		;More steps to do?
	jnz	STPLUP		;n: step again

	ret			;Z set: not on track 0

;---Local Subroutine---------------
;Wait d sector times
;On Entry:
;  d=number of sector times to wait
;Trashes psw,d
;----------------------------------
WSECTS:	lda 	CTLRSF		;Reset sector flag

WSECT:	lda	CTLMO or CTLNOP	;Read A-status
	ani	SASF		;Sector pulse?
	jz	WSECT		;Wait for it
	dcr	d		;enough sectors?
	rz			;y: done

	jmp	WSECTS		;n: Keep waiting

;---Error Routines---------
;These all repair the stack
;and return to the prompt.
;--------------------------
T0STUK:
T0FAIL:	call	CILPRT
	db	'Track ','0'+80h
	jmp	NFERR

SNFERR:	call	CILPRT
	db	'Secto','r'+80h

;Fall into NFERR

NFERR:	call	ILPRNT
	db	' not foun','d'+80h
	jmp	CABORT


 endif ;NSTARS

;***Command ROutine Continuation******************
;BO (Boot from SD Systems Versafloppy disk or
;    Versafloppy II disk, either minidisk or 8")
;*************************************************
 if VERSA1 or VERSA2 or SALFDC

GBOCMD:	lxi	b,4000h+MAXTRYS	;b=home error code
				;c=Retry counter

  endif ;VERSA1 or VERSA2 or SALFDC
 if VERSA1

	mvi	a,(VDSEL0N + VINTEN) xor 0FFh ;select drive 0

 endif ;VERSA1
 if VERSA2 or SALFDC

	mvi	a,VDSEL0 + VMINI ;select drive 0,
				 ;..assume mini for now
 endif ;VERSA2 or SALFDC
 if VERSA1 or VERSA2 or SALFDC

	out	VDRSEL

	mvi	a,RESTOR	;Issue HOME command, load head
	out	VDCOM

	mvi	a,4*CPUMHZ	;delay for 52 uS min
WDELAY:	dcr	a		;(5)
	jnz	WDELAY		;(10)

HOME1:	in	VDSTAT		;Wait for restore to complete
	mov	d,a
	rrc			;test WD1793 busy bit
	jc	HOME1

	mvi	a,SNORDY	;drive ready?
	ana	d
	jnz	DNRERR		;Fail if not ready
	
	mvi	a,STRAK0
	ana	d
	jz	T0FAIL		;Error if can't find home

;Assume successful home without checking
;other error bits. Load at address 0.

TRETRY:	lxi	h,VLOAD		;load address

	mvi	a,1		;sector 0
	out	VSECT

;Enable auto-wait circuit

 endif ; VERSA1 or VERSA2 or SALFDC
 if VERSA1

	mvi	a,(VDSEL0N + VINTEN + VWAITN) xor 0FFh

 endif ;VERSA1
 if VERSA2 or SALFDC

	in	VDRSEL		;get drive sel, mini/maxi
	ori	VWAIT

 endif ;VERSA2 or SALFDC
 if (VERSA1 or VERSA2 or SALFDC)

	out	VDRSEL		;enable auto-wait	

 endif ;(VERSA1 or VERSA2 or SALFDC)
 if (VERSA1 or VERSA2 or SALFDC) and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;(VERSA1 or VERSA2 or SALFDC) and (ENINTS or CRXINT or TRXINT)
 if (VERSA1 or VERSA2 or SALFDC)

	mvi	a,RDSECT	;Read sector command
	out	VDCOM

;Loop to read the sector data into memory at hl

	mvi	b,SECSIZ	;byte count

RLOOP:	in	VDDATA		;read the data
	mov	m,a		;write it into memory
	inx	h		;Next memory address
	dcr	b
	jnz	RLOOP

;Turn off auto-wait

 endif ;VERSA1 or VERSA2 or SALFDC
 if VERSA1

	mvi	a,(VDSEL0N + VINTEN) xor 0FFh

 endif ;VERSA1
 if VERSA2 or SALFDC

	in	VDRSEL		;get drive sel, mini/maxi
	ani	VWAIT xor 0FFh

 endif ;VERSA2 or SALFDC
 if VERSA1 or VERSA2 or SALFDC

	out	VDRSEL		;disable auto-wait	

;Wait for command to complete

RWAIT:	in	VDSTAT		;Disk status
	mov	b,a
	rrc			;test WD1793 busy bit
	jc	RWAIT

;Check status for any errors

	mvi	a,SNORDY+SRNFER+SCRCER+SLOSTD
	ana	b		;any errors?
	jz	VBOOT		;N: go execute loaded code

;----------------------------------
;Read fail. Try again if possible.
;Give user feedback about retries.
;  c=retry count-down
;----------------------------------
	mov	b,a		;error code

 endif ;VERSA1 or VERSA2 or SALFDC
 if VERSA2 or SALFDC

;If we tried a minidisk, then try 8" instead
	in	VDRSEL
	xri	VMINI
	out	VDRSEL

	ani	VMINI		;Was it mini?
	jz	RBRTRY

 endif ;VERSA2 or SALFDC
 if VERSA1 or VERSA2 or SALFDC

	mvi	a,MAXTRYS+1
	cmp	c		;first retry?
	jnz	RBR1		;n: just print dot

	call	CILPRT
	db	'Retryin','g'+80h

RBR1:	call	ILPRNT
	db	'.'+80h

	dcr	c		;too many retries?
	jnz	RBRTRY

;Fall into BOTERR

;---Error Routines---------
;These all repair the stack
;and return to the prompt.
;On Entry:
;  b=error code
;--------------------------
BOTERR:	call	CILPRT
	db	'Boot failed:',' '+80h

	mov	a,b		;error code
	call	PCAHEX

	call	ILPRNT
	db	'h'+80h

	jmp	CABORT

T0FAIL:	call	CILPRT
	db	'Track 0 fai','l'+80h
	jmp	CABORT

DNRERR:	call	CILPRT
	db	'Drive not read','y'+80h
	jmp	CABORT

 endif ;VERSA1 or VERSA2 or SALFDC

;***Command ROutine Continuation******************
;BO (Boot from Tarbell Floppy Disk)
;*************************************************
 if TARBL1
GBOCMD:	mvi	a,TSEL0		;Select drive 0
	out	TEXTP
 endif ;TARBL1

 if TARBL2
GBOCMD:	xra	a
	out	TEXTP0		;select drive 0
	out	TESTP1		;Memory bank 0	
 endif ;TARBL2

 if (TARBL1 or TARBL2)
	lxi	b,4000h+MAXTRYS	;b=home error code
				;c=Retry counter

TRETRY:	mvi	a,FINTCM	;Cancel any pending command
	out	TDCOM

	mvi	a,4*CPUMHZ	;delay for 52 uS min
WDELAY:	dcr	a		;(5)
	jnz	WDELAY		;(10)

HOME1:	in	TDSTAT		;Wait for cancel to complete
	rrc			;test WD1793 busy bit
	jc	HOME1

	mvi	a,RESTOR	;Issue restore command
	out	TDCOM

	in	TWAIT		;Wait for restore to complete

	ora	a		;Test for success
	jm	DNRERR		;Fail if not ready

	in	TDSTAT		;controller status
	ani	STRAK0		;b is home error code too
	jz	T0FAIL		;Error if can't find home

;Assume successful home without checking
;other error bits. Load at address 0.

	lxi	h,SLOAD		;Load address

	mvi	a,1		;Load sector 1
	out	TSECT

	mvi	a,RDSECT	;Read sector command
	out	TDCOM

;Loop to read the sector data into memory at hl

 endif ;(TARBL1 or TARBL2)
 if (TARBL1 or TARBL2) and (ENINTS or CRXINT or TRXINT)
	di			;disable interrupts
 endif ;(TARBL1 or TARBL2) and (ENINTS or CRXINT or TRXINT)
 if (TARBL1 or TARBL2)

RLOOP:	in	TWAIT		;(10)Wait for DRQ or INTRQ

	ora	a		;(4)
	jp	RDONE		;(10)Done if INTRQ

	in	TDDATA		;(10)read the data
	mov	m,a		;(7)write it into memory
	inx	h		;(5)Next memory address
	jmp	RLOOP		;(10) 56 cycles=28 uS

;Check status for any errors

RDONE:	in	TDSTAT		;Disk status
	ani	SNORDY+SRNFER+SCRCER+SLOSTD
	jz	SBOOT		;N: go execute loaded code

;----------------------------------
;Read fail. Try again if possible.
;Give user feedback about retries.
;  c=retry count-down
;----------------------------------
	mov	b,a		;error code

	mvi	a,MAXTRYS+1
	cmp	c		;first retry?
	jnz	RBR1		;n: just print dot

	call	CILPRT
	db	'Retryin','g'+80h

RBR1:	call	ILPRNT
	db	'.'+80h

	dcr	c		;too many retries?
	jnz	RBRTRY

;Fall into BOTERR

;---Error Routines---------
;These all repair the stack
;and return to the prompt.
;On Entry:
;  b=error code
;--------------------------
BOTERR:	call	CILPRT
	db	'Boot failed:',' '+80h

	mov	a,b		;error code
	call	PCAHEX

	call	ILPRNT
	db	'h'+80h

	jmp	CABORT

T0FAIL:	call	CILPRT
	db	'Track 0 fai','l'+80h
	jmp	CABORT

DNRERR:	call	CILPRT
	db	'Drive not read','y'+80h
	jmp	CABORT

 endif ;TARBL1 or TARBL2

;========================
;= Memon/80 Subroutines =
;========================

;***Subroutine*******************
;Report a verify error
;On Entry:
;  de=destination address of fail
;  hl=source address of fail
;Trashes a
;********************************
VERROR:	push	b

	call	PHLADR		;Fail address on new line
				;also sets b=0, trashes c

	call	PMHCSM		;b=0 so source data to Console

	call	ILPRNT
	db	' '+80h

	ldax	d		;destination data
	call	PAHCSM		;b=0 so print on Console

	call	ILPRNT
	db	'?'+80h		;flag the error

	pop	b

;Fall into CPAUSE for Pause and abort opportunity

;***Subroutine**************************************
;Get a keyboard character, abort if control-C, pause
;until the next keyboard character if anything else.
;On Exit:
;  a=keyboard chr
;***************************************************
CPAUSE:	call	CHKKBD		;anything from the keyboard?
	rz			;N: done

;Fall into GETKBD to wait for the user

;***Subroutine****************************************
;Get a keyboard character, abort if CABKEY (control-C)
;On Exit:
;  a=keyboard chr
;  Z set if BS or delete
;*****************************************************
 if not TPORT
GETTPD:				;No transfer port
 endif ;not TPORT		;..so use the console

GETKBD:	call	CHKKBD		;get kbd chr, test for ^C
	jz	GETKBD		;wait for chr

	cpi	DEL		;delete?
	rz
	cpi	BS		;backspace?
	ret

;***Subroutine*********************************
;Get keyboard status. if a chr is waiting, then
;return it in a with parity stripped. Abort if
;CABKEY (control-C).
;On Exit:
;  if a chr is waiting, then chr is in a
;  if no chr waiting, Z set, a=0
;**********************************************	
CHKKBD:	call	KSTAT		;anything typed?
	rz			;N: ret W/ Z set

	call	KDATA		;Y:get chr, strip parity
	cpi	CABKEY		;abort character typed?
	rnz

	jmp	CABORT		;repair stack, go to MAIN

;***Subroutine*****************************************
;Get 2 hex digits from the Transfer Port, combine them
;into one byte, and add the result to the checksum in d
;On Entry:
;  d=checksum so far
;On Exit:
;  b=byte of data
;  a=d=new checksum value
;  Z flag set if checksum is now 0
;  all other registers preserved, unless error abort
;******************************************************
GTPBYT:	call	GTPNIB		;get high nibble
	add	a		;shift high nibble up
	add	a
	add	a
	add	a
	mov	b,a
	call	GTPNIB		;get low nibble

	ora	b		;combine nibbleS
	mov	b,a		;save result for return
	add	d		;Compute checksum
	mov	d,a		;ret with checksum in a & d
	ret

;---Local Subroutine--------------------
;Get a hex digit from the Transfer Port,
;validate it, and return it in a<3:0>
;---------------------------------------
GTPNIB:	call	GETTPD		;get Tx port byte
	call	HEXCON		;convert hex to binary
	rc			;carry means okay

;Abort: ASCII character error - not a valid hex digit

	call	CILPRT
	db	'Ch','r'+80h

	jmp	CMDERR		;error handler

;***Subroutine*****************************************
;Get two hex values from the keyboard input line buffer
;abort to CMDERR if none provided
;On Entry:
;  de=address of next item in the input line buffer
;On Exit:
;  1st hex value is at top of the stack
;  prior hl value is next on the stack
;  hl=2nd hex value
;  de advanced past both hex values
;  abort to CMDERR if no value found
;******************************************************
GETHX2:	xthl			;save old hl
	call	GETHEX		;get 1st hex value

;Fall into GETHEX to get 2nd hex value

;***Subroutine**************************************
;Get a hex value from the keyboard input line buffer
;abort to CMDERR if none provided
;On Entry:
;  de=address of next item in the input line buffer
;On Exit:
;  prior hl value is on the stack
;  hl=value
;  de advanced past chr
;  abort to CMDERR if no value found
;***************************************************
GETHEX:	xthl			;save old hl
	call	PHFHEX		;restore our return address
				;get hl=hex value
	rnc

;Fall into CMDERR if no hex value provided

;*********************
;command Error Handler
;*********************
CMDERR:	call	CILPRT
	db	'?'+80h

;Fall into CABORT

;*********************************************
;Command abort: repair stack and go to MAIN
;On Entry:
;
; if TPORT:
;  Bottom of stack (XXFF)=Transfer Port flag
;                  (XXFE)=junk
;  next on stack (XXFc, XXFD)=MAIN
;
; if not TPORT:
;  Bottom of stack (XXFE, XXFF)=MAIN
;*********************************************
CABORT:

 if RAMHNT and TPORT
	call	RAMPAG		;get hl=stack address
	mvi	l,(STACK-4) and 0FFh ;return address on stack
	sphl			;fix stack
	ret			;return to MAIN
 endif ;RAMHNT and TPORT

 if RAMHNT and (not TPORT)
	call	RAMPAG		;get hl=stack address
	mvi	l,(STACK-2) and 0FFh ;return address on stack
	sphl			;fix stack
	ret			;return to MAIN
 endif ;RAMHNT and (not TPORT)


 if (not RAMHNT) and TPORT
	lxi	sp,STACK-4
	ret			;return to MAIN
 endif ;(not RAMHNT) and TPORT

 if (not RAMHNT) and (not TPORT)
	lxi	sp,STACK-2
	ret			;return to MAIN
 endif ;not RAMHNT and (not TPORT)

;***Subroutine**************************************
;Read a command line from the keyboard, echoing and
;saving it in the input line buffer. CR input ends
;the sequence. LBCHR replaces the terminating CR
;with 0 for easy testing later.
;On Exit:
;  Complete command line is in the input line buffer
;  de=address of the first non-space chr on the line
;  Carry set if nothing but spaces found
;***************************************************
GETLIN:	push	h

 if RAMHNT
	call	RAMPAG		;get RAM address
	mvi	l,RAMBUF	;buffer location in page
 endif ;RAMHNT

 if not RAMHNT
	lxi	h,RAMBUF
 endif ;not RAMHNT

	push	h		;save input line buffer'S
				;..start address

;Get & echo chrs, stashing them in the input line buffer
;at hl, until a CR is encountered or the line overflows

GLLOOP:
 if not HELPC
	mvi	a,(RAMBUF+LBSIZE-1) and 0FFh ;input buf full?
 endif ;not HELPC
 if HELPC
	mvi	a,(RAMBUF+LBSIZE-2) and 0FFh ;input buf full?
 endif ;HELPC

	sub	l		;carry set if full

	cnc	LBCHR		;N:get another character
	jnc	GLLOOP		;carry means CR found

	xra	a
	mov	m,a		;replace CR with a null

 if HELPC
	inx	h		;install another null
	mov	m,a		;..for single-chr command
 endif ;HELPC

	pop	d		;de=line buffer address
	pop	h		;restore original hl

;Fall into SSPACE to skip leading spaces

;***Subroutine*************************************
;Scan past spaces in line buffer, looking for the
;first non-space character
;On Entry:
;  de=starting address within the input line buffer
;On Exit:
;  Carry set if no chr or only control chrs found
;  a=character value if found, 0 if end of line
;  de advanced past spaces
;**************************************************
SSPACE:	ldax	d	;get next character

	cpi	' '	;another space?
			;carry set for any ctrl chr
	rnz		;carry clear for all else

	inx	d	;next scan address
	jmp	SSPACE	;keep skipping

;***Subroutine****************************************
;Get, echo, and store a Console character in the input
;line buffer. Handle deletes and backspaces. This
;assumes that the line buffer is all in one page. CR's
;are not echoed, but are replaced with 0.
;On Entry:
;  hl=next free spot in the input line buffer
;On Exit (no deletes, no CR):
;  chr stored in buffer at original hl
;  hl=hl+1
;  Carry clear
;On Exit (CR)
;  Carry set
;On Exit (BS or DEL)
;  hl=hl-1
;  Carry clear
;*****************************************************
LBCHR:	call	GETKBD		;get a chr, test BS & DEL
	jz	LDELET		;BS or delete?

	cpi	CR
	stc			;flag CR found
	rz

	cmc			;clear carry for ret
	mov	m,a		;enqueue
	inr	l		;Bump line buffer pointer
	jmp	PRINTA		;echo & ret with carry clear

;Backspace if possible

LDELET:	mov	a,l		;buffer size
	cpi	RAMBUF and 0FFh	;back up if we can
	rz			;done if not (carry clear)
	
	call	ILPRNT		;back up, clear Carry
	db	BS,' ',BS+80h	;erase old chr & back up	

	dcr	l		;back up

	ret

;***Subroutine************************
;Convert ASCII hex digit to binary
;On Entry:
;  a=chr to convert
;On Exit:
;  a=binary
;  Carry set if OK, clear if bogus chr
;*************************************
HEXCON:	sui	'0'		;remove ASCII bias
	cpi	10
	rc			;if 0-9 then we're done

	sui	9+('A'-'9')	;should be 0-5 now
	cpi	6		;gap chr or too high?
	rnc			;error: return W/O carry

	sui	0F6h		;add 0Ah, set Carry
	ret

;***Subroutine***********************************
;Print CR LF, followed by inline message at (sp)
;on Console. (CallS to CILPRT are followed by the
;string.) The last string byte has its msb set
;On Exit:
;  carry & Z cleared
;  Interrupts enabled if ENINTS=TRUE
;Trashes a. All other registers preserved
;************************************************
CILPRT:
 if (ENINTS or CRXINT or TRXINT)
	ei			;enable interrupts
 endif ;(ENINTS or CRXINT or TRXINT)

	call	ILPRNT
 if not HELPC
	db	CR,LF+80h
 endif ;not HELPC

 if HELPC
	db	CR+80h
 endif ;HELPC

;Fall into ILPRNT

;***Subroutine********************************
;Print inline message at (sp) on Console.
;(Calls to ILPRNT are followed by the string.)
;The last string byte has its msb set.
;On Exit:
;  carry & Z cleared
;Trashes a. All other registers preserved
;
;The help screen has a lot of lines, making it
;more efficient to follow all CRs with LFs
;automatically.. Otherwise, it's not worth it.
;*********************************************
ILPRNT:	xthl			;save hl, get msg addr

IPLOOP:	mov	a,m		;LOOP through message
	ani	7Fh		;strip end-marker
	call	PRINTA

 if HELPC
	cpi	CR
	mvi	a,LF
	cz	PRINTA
 endif ;HELPC

	ora	m		;End? (clears carry too)
	inx	h
	jp	IPLOOP		;test msb in memory string

	xthl			;restore hl
				;..get ret address
	ret

;***Subroutine***************************************
;Test which port is the Transfer Port
;On Entry:
;  sp points into the RAM page
;  RAM page flag=0 for Console, <>0 for Transfer Port
;On Exit:
;  Z set only if the Transfer Port is the Console
;****************************************************
 if TPORT
TESTTP:	push	h
 endif ;TPORT

 if TPORT and RAMHNT
	call	RAMPAG		;find RAM page
	mvi	l,(STACK-1) and 0FFh ;get flag from RAM page
 endif ;TPORT and RAMHNT

 if TPORT and (not RAMHNT)
	lxi	h,STACK-1
 endif ;TPORT and (not RAMHNT)

 if TPORT
	inr	m		;test flag
	dcr	m		;Z means disabled

	pop	h
	ret
 endif ;TPORT

;***Subroutine******************************************
;Get a byte from the Transfer Port
;Strips parity,  checks for control-C abort
;from the Console keyboard
;On Entry:
;  sp points into the RAM page
;  RAM page byte FE=0 for Console, <>0 for Transfer Port
;On Exit:
;  chr in a, with parity stripped
;*******************************************************
 if TPORT
GETTPD:	call	TESTTP		;which port?
	jz	GETKBD		;Z means Console

GTPLUP:	call	CHKKBD		;user abort?
	call	TPISTA		;Transfer Port chr?
	jz	GTPLUP
 endif ;TPORT

 if TPORT and (not TMEMAP) and (not TRXINT)
	in	TDATA		;get Transfer Port chr
	ani	7Fh		;strip parity
	ret
 endif ;TPORT and (not TMEMAP) and (not TRXINT)

 if TPORT and TMEMAP and (not TRXINT)
	lda	TDATA		;get Transfer Port chr
	ani	7Fh		;strip parity
	ret
 endif ;TPORT and TMEMAP and (not TRXINT)

 if TPORT and TRXINT
	call	TPIDAI
	ani	7Fh		;strip parity
	ret
 endif ;TPORT and TRXINT

;***Subroutine*******************
;Print hl on a new line in hex on
;the Console, followed by ': '
;Trashes psw,bc
;********************************
PHLADR:	call	CILPRT		;CR LF space begins line
	db	' '+80h

	call	PHLCHX		;hl=address on Console
				;Trashes bc

;Fall into PCCOLS to print ': '

;***Subroutine*******************
;Print ': ' on the Console
;Trashes psw
;********************************
PCCOLS:	call	ILPRNT		;print colon space
	db	':',' '+80h
	ret

;***Subroutine********************
;print hl as 4 hex digits on the
;Console & accumulate the checksum
;On Entry:
;  (c=checksum so far)
;  hl=2 bytes to print
;On Exit:
;  b=0
;  (c=updated checksum)
;Trashes psw
;*********************************
PHLCHX:	mvi	b,0		;print on Console

;Fall into PHLHEX

;***Subroutine*********************************
;Print hl as 4 hex digits & accumulate checksum
;On Entry:
;  b=0 for Console, <>0 for Transfer Port
;  c=checksum so far
;  hl=2 bytes to print
;On Exit:
;  c=updated checksum
;Trashes psw
;**********************************************
PHLHEX:	mov	a,h		;h first
	call	PAHCSM		;returns with carry clear
	mov	a,l		;then l

;	db	CPI		;skip over PMHCSM
	db	0FEH

;Skip into PAHCSM

;***Subroutine********************************
;Print m as 2 hex digits & accumulate checksum
;On Entry:
;  (HL)=byte to print
;  b=0 for Console, <>0 for Transfer Port
;  c=checksum so far
;On Exit:
;  c=updated checksum
;Trashes psw
;*********************************************
PMHCSM:	mov	a,m

;Fall into PAHCSM

;***Subroutine********************************
;Print a as 2 hex digits & accumulate checksum
;On Entry:
;  a=byte to print
;  b=0 for Console, <>0 for Transfer Port
;  c=checksum so far
;On Exit:
;  c=opdated checksum
;Trashes psw
;*********************************************
PAHCSM:	push	psw
	add	c		;Compute checksum
	mov	c,a
	pop	psw		;recover chr

 if TPORT
;	db	CPI		;CPI opcode skips 1
	db	0FEH

;Skip into PAHEX2 (executing a NOP on the way)
 endif ;TPORT

;***Subroutine*********************
;Print a on Console as 2 hex digits
;On Entry:
;  a=byte to print
;Trashes psw, b
;**********************************
PCAHEX:

 if TPORT
	mvi	b,0		;print on the Console
 endif ;TPORT

;Fall into PAHEX2

;***Subroutine***************************
;Print a as 2 hex digits
;On Entry:
;  a=byte to print
;  b=0 for Console, <>0 for Transfer Port
;    (only if TPORT is TRUE)
;Trashes psw
;****************************************
PAHEX2:	push	psw		;save for low digit
	
	rrc			;move the high four down
	rrc
	rrc
	rrc
	call	PAHEX1		;print high digit
	pop	psw		;this time the low four

;Fall into PAHEX1 to print low digit

;***Subroutine***************************
;Print low nibble of a as 1 hex digit
;On Entry:
;  b=0 for Console, <>0 for Transfer Port
;    (only if TPORT is TRUE)
;On Exit:
;  psw trashed
;****************************************
PAHEX1:	ani	0Fh		;Four on the floor
	adi	'0'		;We work with ASCII here
	cpi	'9'+1		;0-9?
	jc	PNIB1		;Yup: print & return

	adi	'A'-'9'-1	;make it a letter
PNIB1:

 if TPORT
	inr	b		;which port?
	dcr	b
	jnz	TPOUT		;print on Transfer Port
 endif ;TPORT

;	db	CPI		;CPI opcode skips PRINTC
	db	0FEH

;Skip into PRINTA

;***CP/M External Subroutine*******
;Print C on the Console
;On Entry:
;  the character for output is in c
;On Exit:
;  a=c=chR
;  all other regs preserved
;**********************************
PRINTC:	mov	a,c

;Fall into PRINTA

;***Subroutine*************
;Print a on the Console
;On Entry:
;  a=the character to print
;all regs preserved
;**************************
PRINTA:	push	psw

 if not CMEMAP
PAWAIT:	in	CSTAT
	ani	CTXRDY		;wait for transmitter ready

 endif ;not CMEMAP

 if CMEMAP
PAWAIT:	lda	CSTAT
	ani	CTXRDY		;wait for transmitter ready
 endif ;CMEMAP

 if CISTAT
	jnz	PAWAIT

 endif ;CISTAT

 if not CISTAT
	jz	PAWAIT
 endif ;not CISTAT

 if not CMEMAP
	pop	psw		;recover chr
	out	CDATA
	ret
 endif ;not CMEMAP

 if CMEMAP
	pop	psw		;recover chr
	sta	CDATA
	ret
 endif ;CMEMAP


;***CP/M External Subroutine************
;Wait for and get Console keyboard data
;On Exit:
;  a=keyboard character, parity stripped
;  Z clear
;***************************************
KDATA:	call	KSTAT
	jz	KDATA

 if (not CMEMAP) and (not CRXINT)
	in	CDATA		;get keyboard chr
 endif ;(not CMEMAP) and (not CRXINT)

 if CMEMAP and (not CRXINT)
	lda	CDATA		;get keyboard chr
 endif ;CMEMAP and (not CRXINT)

 if CRXINT
	xra	a		;clear flag
	di			;mask while we work
	sta	CIFLAG
	lda	CRXBUF		;get data
	ei
 endif ;CRXINT

	ani	7Fh		;strip parity
	ret

;***CP/M External Subroutine************
;Get Console keyboard status
;On Exit:
;  a=0 and Z set if nothing waiting
;  a=FF and Z cleared if kbd chr waiting
;***************************************
KSTAT:
 if (not CMEMAP) and (not CRXINT)
	in	CSTAT
 endif ;(not CMEMAP) and (not CRXINT)

 if CMEMAP and (not CRXINT)
	lda	CSTAT
 endif ;CMEMAP and (not CRXINT)

 if CISTAT and (not CRXINT)
	cma			;inverted status bit
 endif ;CISTAT and (not CRXINT)

 if not CRXINT
	ani	CRXRDY
 endif ;not CRXINT

 if CRXINT
	lda	CIFLAG
	ora	a
 endif ;CRXINT

;Fall into DOSTAT

;***Subroutine*****************
;Create CP/M-style return value
;On Exit:
;  if Z then a=0
;  if NZ then a=0FFh
;******************************
DOSTAT:	rz			;CP/M-style return values
	mvi	a,0FFh
	ret

;***CP/M External Subroutine***
;Get Transfer Port Rx status
;On Exit:
;  a=0 & Z set if no data
;  a=FF  & Z clear if data
;******************************

TPISTA:
 if TPORT and (not TMEMAP) and (not TRXINT)
	in	TSTAT
 endif ;TPORT and (not TMEMAP) and (not TRXINT)

 if TPORT and TMEMAP and (not TRXINT)
	lda	TSTAT
 endif ;TPORT and TMEMAP and (not TRXINT)

 if TPORT and TISTAT and (not TRXINT)
	cma			;inverted status bit
 endif ;TPORT and TISTAT and (not TRXINT)

 if TPORT and (not TRXINT)
	ani	TRXRDY
	jmp	DOSTAT
 endif ;TPORT and (not TRXINT)

 if TPORT and TRXINT
	lda	TIFLAG
	ora	a	;a=0 or FF
	ret
 endif ;TPORT and TRXINT

;***CP/M External Subroutine***
;Get Transfer Port Rx data
;On Exit:
;  a=byte from port
;  Z cleared
;******************************
 if TPORT
TPIDAT:	call	TPISTA		;wait for chr
	jz	TPIDAT
 endif ;TPORT

 if TPORT and (not TMEMAP) and (not TRXINT)
	in	TDATA
	ret
 endif ;TPORT and (not TMEMAP) and (not TRXINT)

 if TPORT and TMEMAP and (not TRXINT)
	lda	TDATA
	ret
 endif ;TPORT and TMEMAP and (not TRXINT)

 if TPORT and TRXINT
TPIDAI:	xra	a		;clear flag
	di			;mask while we work
	sta	TIFLAG
	lda	TRXBUF		;Get data
	ei
	ret
 endif ;TPORT and TRXINT

;***Subroutine*******************************
;Finish writing Intel end-of-file hex record
;(the CRLF and colon have already been sent.)
;********************************************
HDEOF:	mvi	b,5		;5 bytes for EOF

HDELUP:	xra	a
	call	PAHEX2		;B<>0 for Transfer Port
	dcr	b
	jnz	HDELUP

;Fall into TPCRLF for a final CR LF

;***Subroutine************************************
;Send CR LF to the Transfer Port
;On Entry:
;  sp points into the RAM page
;  RAM page byte FE=1 for channel B, 0 for Console
;Trashes psw
;*************************************************
TPCRLF:	mvi	a,CR		;CR,LF to Transfer Port
	call	TPOUT

	mvi	a,LF

;Fall into TPOUT

;***Subroutine************************************
;Send a to the Transfer Port
;On Entry:
;  sp points into the RAM page
;  RAM page byte FE=1 for channel B, 0 for Console
;Trashes flags
;*************************************************
TPOUT:
 if TPORT
	call	TESTTP		;which physical port?
	jz	PRINTA		;Z means Console

;	db	CPI		;CPI opcode skips TPDATC
				;..TO Fall into PBODAT
	db	0FEH
 endif ;TPORT

 if not TPORT
	jmp	PRINTA
 endif ;not TPORT

;***CP/M External Subroutine***
;Send c to Transfer Port
;On Exit:
;  a=c=chr
;All other regs preserved
;******************************
 if TPORT
TPDATC:	mov	a,c

;Fall into PBODAT
 endif ;TPORT

;***Subroutine**********
;Send a to Transfer Port
;All regs preserved
;***********************
 if TPORT
PBODAT:	push	psw

PBODA1:	call	TPOSTA		;wait for transmitter
	jz	PBODA1
 endif ;TPORT

 if TPORT and (not TMEMAP)
	pop	psw		;recover chr
	out	TDATA
	ret
 endif ;TPORT and (not TMEMAP)

 if TPORT and TMEMAP
	pop	psw		;recover chr
	sta	TDATA
	ret
 endif ;TPORT and TMEMAP

;*************************
;8251 initialization table
;*************************
 if C8251 or T8251

IN8251:	db	SI21,SI22,SI23,SI24

 endif	;C8251 or T8251

;*********************************
;Z80-DART initialization table
;(Gets sent to both DART channels)
;*********************************
 if DART
DITAB:	db	DI1,dI2,dI3,dI4,dI5,dI6,dI7
 endif	;DART

;****************************************************
;Transfer Port Baud Rate Table
;Each entry has 4 bytes:
;  Byte 0=value for BRATE0 port (written 1st)
;  Byte 1=value for BRATE1 port (written 2nd)
;  Byte 3-4=Decimal value for baud rate string,
;             encoded as 4 hex digits. Leading zeros
;             get suppressed, and a "0" gets appended
;             to these digits when printed.
;****************************************************
 if TSBAUD and ROM2K
BTABLE:	dw	TBD110	;0= 110 baud 2 stop bits
	db	00h,11h	;string

	dw	TBD150	;1= 150 baud
	db	00h,15h

	dw	TBD300	;2= 300 baud
	db	00h,30h

	dw	TBD600	;3= 600 baud
	db	00h,60h

	dw	TBD1200	;4= 1200 baud
	db	01h,20h

	dw	TBD2400	;5= 2400 baud
	db	02h,40h

	dw	TBD4800	;6= 4800 baud
	db	04h,80h

	dw	TBD9600	;7= 9600 baud
	db	09h,60h
 endif ;TSBAUD and ROM2K

 if BD192 and TSBAUD and ROM2K
	dw	TBD192	;8= 19200 baud
	db	19h,20h
 endif ;BD192 and TSBAUD and ROM2K

 if BD384 and TSBAUD and ROM2K
	dw	TBD384	;9= 38400 baud
	db	38h,40h
 endif ;BD383 and TSBAUD and ROM2K

 if BD576 and TSBAUD and ROM2K
	dw	TBD576	;a= 57600 baud
	db	57h,60h
 endif ;BD576 and TSBAUD and ROM2K

 if BD768 and TSBAUD and ROM2K
	dw	TBD768	;b= 76800 baud
	db	76h,80h
 endif ;BD768 and TSBAUD and ROM2K

 if TSBAUD and ROM2K
BTEND:
 endif ;TSBAUD and ROM2K

;***Command Routine Continuation***
;? Print (Minimal) Help Screen
;**********************************
 if HELPC
GHELP:	call	CILPRT

;Memory commands

	db 'CO s d c [r]',CR
	db 'DU a c',CR
	db 'EN a',CR
	db 'FI [a [c [v]]]',CR
 endif ;HELPC

 if HELPC and ROM2K
	db 'MT a c',CR
	db 'SE a v v ',QUOTE,'t',QUOTE,'...',CR
 endif ;HELPC and ROM2K

 if HELPC
	db 'VE s d c',CR,LF

;I/O commands

	db 'IN p',CR
	db 'OT p v',CR,LF
 endif ;HELPC

;Disk commands

 if HELPC and BOOTER
	db 'BO'
 endif ;HELPC and BOOTER

 if HELPC and (C4FDC or C16FDC or C64FDC)
	db ' d'		;Cromemco can boot from any disk
 endif ;HELPC and (C4FDC or C16FDC or C64FDC)

 if HELPC and BOOTER
	db CR
 endif ;HELPC and BOOTER

 if HELPC and ROM2K
	db 'CE cl',CR
 endif ;HELPC and ROM2K

 if HELPC
	db 'EX a'
 endif ;HELPC

 if HELPC and EXOPT
	db ' [0/1]'
 endif ;HELPC and EXOPT

 if HELPC
	db CR,LF

;Transfer Port Commands

	db 'HD a c',CR
 endif ;HELPC

 if TPORT and HELPC
	db 'HL [o]',CR
 endif ;TPORT and HELPC

 if (not TPORT) and HELPC
	db 'HL [o]',CR+80h
 endif ;(not TPORT) and HELPC

 if HELPC and TSBAUD
	db 'TB 0-'
 endif ;HELPC and TSBAUD

 if (HELPC and TSBAUD) and not (BD192 or BD384 or BD576 or BD768)
	db '7'
 endif ;(HELPC and TSBAUD) and not (BD192 or BD384 or BD576 or BD768)

 if (HELPC and TSBAUD and BD192) and not (BD384 or BD576 or BD768)
	db '8'
 endif ;(HELPC and TSBAUD and BD192) and not (BD384 or BD576 or BD768)

 if (HELPC and TSBAUD and BD384) and not (BD576 or BD768)
	db '9'
 endif ;(HELPC and TSBAUD and BD384) and not (BD576 or BD768)

 if (HELPC and TSBAUD and BD576) and not BD768
	db 'A'
 endif ;(HELPC and TSBAUD and BD576) and not BD768

 if HELPC and TSBAUD and BD768
	db 'B'
 endif ;TSBAUD and HELPC and BD768

 if TPORT and HELPC and TSBAUD
	db CR
 endif ;TPORT and HELPC and TSBAUD

 if TPORT and HELPC
	db 'TE e',CR
	db 'TP 0/1',CR+80h
 endif ;TPORT and HELPC

 if HELPC
	ret
 endif ;HELPC

;==========================================
;command table
;EACH entry is 3 bytes:
;  byte 0:       1st command character
;  byte 1 <6:0>: 2nd command character
;  byte 1 <7>:   address offset msb
;  byte 2:       address offset from CMDBAS
;the table terminates with byte 0=0.
;==========================================
COMTAB:	db	'DU'		;Dump
	db	DUCMD-CMDBAS

	db	'EN'		;Enter
	db	ENCMD-CMDBAS

	db	'EX'		;Execute
	db	EXCMD-CMDBAS

	db	'FI'		;Fill memory
	db	FICMD-CMDBAS

	db	'IN'
	db	INCMD-CMDBAS	;In from port

	db	'OT'
	db	OTCMD-CMDBAS	;Out to port

 if TPORT
	db	'TE'		;Terminal mode
	db	TECMD-CMDBAS

	db	'TP'		;Set Transfer Port
	db	TPCMD-CMDBAS
 endif ;TPORT

VED	equ	VECMD-CMDBAS
	db	'V','E'+((VED/2) and 80h) ;verify
	db	VED and 0FFh

COD	equ	COCMD-CMDBAS
	db	'C','O'+((COD/2) and 80h) ;COPY
	db	COD and 0FFh

HDD	equ	HDCMD-CMDBAS
	db	'H','D'+((HDD/2) and 80h) ;Intel hex dump
	db	HDD and 0FFh

HLD	equ	HLCMD-CMDBAS
	db	'H','L'+((HLD/2) and 80h) ;Intel hex load
	db	HLD and 0FFh

TBD	set	0
 if TPORT and TSBAUD and ROM2K
TBD	set	TBCMD-CMDBAS
	db	'T','B'+((TBD/2) and 80h) ;Set T Port Baud Rate
	db	TBD and 0FFh
 endif ;TPORT and TSBAUD and ROM2K

BOD	set	0
 if BOOTER
BOD	set	BOCMD-CMDBAS
	db	'B','O'+((BOD/2) and 80h) ;Boot frm disk
	db	BOD and 0FFh
 endif ;BOOTER

SED	set	0
 if ROM2K
	db	'CE'			;Execute CP/M program
	db	CECMD-CMDBAS

	db	'MT'			;Memory Test
	db	MTCMD-CMDBAS

SED	set	SECMD-CMDBAS
	db	'S','E'+((SED/2) and 80h) ;Search
	db	SED and 0FFh
 endif ;ROM2K

 if HELPC
	db	'?',0			;Help
	db	HELP-CMDBAS
 endif ;HELPC

	db	0			;End of table mark

;========================================================
;Interrupt code for the H8-5 Rx Interrupt, as the console
;This code assumes RAMHNT = FALSE
;  RAM page variables
;   CRXBUF:  1-byte receive queue
;   CIFLAG: 0FFh means CRXBUF has a character to send
;========================================================
 if CH85
CONINT:	push	psw		;onto MEMON's stack

;Receive and enqueue a character, ignoring any overflow	

	in	CDATA		;get chr now
	sta	CRXBUF		;enqueue chr

	mvi	a,0FFh		;flag received chr
	sta	CIFLAG

;Restore registers and return from the interrupt

	pop	psw
	ei
	ret

 endif ;CH85

;==============================================================
;Interrupt code for the H8-5 Rx Interrupt, as the transfer port
;This code assumes RAMHNT = FALSE
;  RAM page variables
;   TRXBUF:  1-byte receive queue
;   TIFLAG: 0FFh means TRXBUF has a character to send
;==============================================================
 if TH85
TBINT:	push	psw		;onto MEMON's stack

:Receive and enqueue a character, ignoring any overflow	

	in	TDATA		;get chr now
	sta	TRXBUF		;enqueue chr

	mvi	a,0FFh		;flag received chr
	sta	TIFLAG

;Restore registers and return from the interrupt

	pop	h
	pop	psw
	ei
	ret

 endif ;TH85

;======================================
;Create an assembly error if MEMON/80's
;RAM is not all in the same page
;======================================
 if (not RAMHNT) and ((RAMBEG/256) - (RAMEND/256))
ERROR: RAMEND Problem. MEMON RAM spans 2 pages
 endif ;(not RAMHNT) and ((RAMBEG/256) - (RAMEND/256))

;==========================================
;Create an assembly error if interrupts are
;required and RAMHNT is true
;==========================================
 if RAMHNT and (ENINTS or CRXINT or TRXINT)
ERROR: RAMHNT with a port that uses interrupts
 endif ;RAMHNT and (ENINTS or CRXINT or TRXINT)

;==========================================
;Create an assembly error if any of the
;command execution routines is out of range
;==========================================
 if (SED or TBD or HLD or HDD or COD or VED)/512
ERROR: command routine out of range
 endif ;(SED or TBD or HLD or HDD or COD or VED)/512

;===================================
;Create an assembly error if the
;code is larger than the EPROM space
;===================================
CODEND	equ	$

 if (not RAMCOD) and ((CODEND-MEBASE-1)/ROMSIZ)
ERROR: code is larger than available EPROM space
 endif ;(not RAMCOD) and ((CODEND-MEBASE-1)/ROMSIZ)

	END

Usage:

z80asm -f{b|m|h} -s[n|a] -p<num> -e<num> -h<num> -x -8 -u
       -v -m -o<file> -l[<file>] -d<symbol> ... <file> ...

A maximum of 512 source files is allowed. If the file name of a source
doesn't have an extension the default extension ".asm" will be appended.
Source files have a maximum path length of 2048 characters.

Option f:
Format of the output file:

	-fb -> binary file
	-fm -> binary file with Mostek header
	-fh -> Intel hex

The default is Intel hex now, in earlier versions it was Mostek binary,
but this format is not used much anymore.
For z80sim produce Mostek binary or Intel hex files, it cannot load binary
files that don't include the load address.

Option s:
This option prints the symbol table unsorted (-s), sorted by name (-sn)
or sorted by address (-sa) into the list file. This option only works
together with option -l.

Option p:
Set the page length for the list file.
The default is 65 and the allowed range is 6 to 144 or 0.

Option e:
Set the significant number of characters in symbols to <num>.
The default is 8 and the allowed range is 6 to 32.

Option h:
Set the maximum number of bytes per hex record to <num>.
The default is 32 and the allowed range is 1 to 32.

Option x:
Don't fill binary files up to the last used logical address. This means,
that single parameter DEFS's at the end of the source file won't fill up
the output file with 0xff's.
Useful to make CP/M BIOS's, where unallocated data doesn't need to be a
part of the system image, fit on the system tracks.
This option is a no-op for Intel hex output.

Option 8:
Change default instruction set to 8080.

Option u:
Accept undocumented Z80 instructions.

Option v:
Verbose operation of the assembler.

Option m:
Modify macro expansion listing. Use once to list all macro expansions,
twice to list no macro expansions.
The default is to list only macro expansions that produce object code.

Option o:
To override the default name of the output file. Without this option the
name of the output file becomes the name of the input file, but with the
extension ".bin" or ".hex". The output file has a maximum path length of
2048 characters.

Option l:
Without this option no list file will be generated. With -l a list file
with the name of the source file but extension ".lis" will be generated.
An optional file name path (2048 characters maximum) may be added to
this option.

Option d:
This option predefines symbols with a value of 0 and may be used
multiple times.


Pseudo Operations:

Definition of symbols and allocation of memory:

	 ORG    <expression>    - set program address
         .PHASE <expression>    - set logical program counter
	 .DEPHASE               - end of logical phase block
<symbol> EQU    <expression>    - define constant symbol
<symbol> DEFL   <expression>    - define variable symbol
<symbol> DEFB   <exp,'str',...> - write bytes into memory
<symbol> DEFM   <exp,'str',...> - write characters into memory (same as DEFB)
<symbol> DEFC   <exp,'str',...> - as DEFM, but bit 7 in the last byte is set
<symbol> DEFZ   <exp,'str',...> - as DEFM, but an extra zero byte is added
<symbol> DEFW   <exp,exp...>    - write words (16 bits) into memory
<symbol> DEFS   <exp>[,<exp>]   - reserve space in memory, opt. initialized

The aliases ASET (and SET in 8080 mode) for DEFL, DB for DEFB, DC for
DEFC, DS for DEFS, and DW for DEFW are also accepted.

External symbol declarations:

PUBLIC <symbol>		- make symbol public
EXTRN  <symbol>		- symbol is defined external

The aliases ENT, ENTRY, and GLOBAL for PUBLIC, and EXT and EXTERNAL for
EXTRN are also accepted.

The pseudo operations for external symbols are accepted, but won't do
anything. Source modules can be concatenated or included, so the symbols
will be resolved, and the PUBLIC/EXTERN declarations can be left
unaltered, because the assembler ignores them.


Conditional assembly:

IF      <expression>    - assemble if expression evaluates to not 0
IFF     <expression>    - assemble if expression evaluates to 0
IFDEF   <symbol>        - assemble if symbol defined
IFNDEF  <symbol>        - assemble if symbol not defined
IFEQ    <exp1>,<exp2>   - assemble if values of expressions are equal
IFNEQ   <exp1>,<exp2>   - assemble if values of expressions are not equal
IF1                     - assemble if in pass 1
IF2                     - assemble if in pass 2
ELSE                    - else for all conditionals
ENDIF                   - end of conditional assembly

The aliases COND and IFT for IF, and IFE for IFF, and ENDC for ENDIF are
also accepted.


Intel-like macros:
<sym> MACRO <dummy>,... - define named macro
IRP     <id>,<<cl,...>> - inline macro, iterate over character lists
IRPC    <id>,<clist>    - inline macro, iterate over character list
REPT    <count>         - inline macro, repeat <count> times
ENDM                    - end macro
EXITM                   - exit macro expansion
MACLIB  <filename>      - include macro library
IFB     <seq>           - assemble if char sequence inside <> is empty
IFNB    <seq>           - assemble if char sequence inside <> is not empty
IFIDN   <seq1>,<seq2>   - assemble if char sequences inside <> are equal
IFDIF   <seq1>,<seq2>   - assemble if char sequences inside <> are not equal


Manipulation of list file:

PAGE    <expression>    - number of lines/page
EJECT                   - skip to new page
LIST                    - listing on
NOLIST                  - listing off
TITLE   <'string'>      - define title for page header


Others:

INCLUDE <filename>      - include another source file
PRINT   <'string'>      - print string to stdout
.8080                   - switch to 8080 instruction set
.Z80                    - switch to Z80 instruction set
.RADIX                  - change numbers radix (default 10)
ASEG                    - does nothing (like its alias ABS)


Precedence for expression operators:

()
unary + - ~ NOT HIGH LOW NUL TYPE
* / MOD SHR >> SHL <<
+ -
EQ = == NE <> != LT < LE <= GT > GE >=
AND & XOR OR |

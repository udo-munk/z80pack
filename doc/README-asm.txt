Usage:

z80asm -8 -u -v -U -e<num> -f{b|m|h|c} -x -h<num> -c<num> -m -T -p<num>
       -s[n|a] -o<file> -l[<file>] -d<symbol>[=<expr>] ... <file> ...

Note: z80asm can only process ASCII text files.

If the file name of a source doesn't have an extension the default
extension ".asm" will be appended.

Source lines have a maximum length of 128 characters, excluding the
new line character. Any characters after this limit are ignored.

Option 8:
Change default instruction set to 8080.

Option u:
Accept undocumented Z80 instructions.

Option v:
Verbose operation of the assembler.

Option U:
Convert everything to upper case for compatibility with old source code.

Option e:
Set the number of significant characters in symbols to <num>.
The default is 8 and the allowed range is 6 to 32.

Option f:
Format of the output file:

        -fb -> binary file
        -fm -> binary file with Mostek header
        -fh -> Intel HEX
        -fc -> C initialized array

The default is Intel HEX now, in earlier versions it was Mostek binary,
but this format is not used much anymore.
For z80sim use Mostek binary or Intel HEX files, it cannot load binary
files that don't include the load address.

Option x:
Don't fill binary files up to the last used logical address. This means,
that single parameter DEFS's at the end of the source file won't fill up
the output file with 0xff's.
Useful to make CP/M BIOS's, where unallocated data doesn't need to be a
part of the system image, fit on the system tracks.
This option is a no-op for Intel HEX output.

Option h:
Set the maximum number of bytes per HEX record to <num>.
The default is 32 and the allowed range is 1 to 32.

Option c:
Set the maximum number of bytes per C array line to <num>.
The default is 12 and the allowed range is 1 to 16.
Values greater than 12 omit the space between bytes.

Option m:
Modify macro expansion listing. Use once to list all macro expansions,
twice to list no macro expansions.
The default is to list only macro expansions that produce object code.

Option T:
Don't print time stamp in list file headers.

Option p:
Set the page length for the list file.
The default is 65 and the allowed range is 6 to 144 or 0.
If page length is 0 the assembler only prints a single header at the
start of the listing and inserts an empty line for PAGE/EJECT and
INCLUDE/MACLIB instead of starting a new page.

Option s:
This option prints the symbol table unsorted (-s), sorted by name (-sn)
or sorted by address (-sa) into the list file. This option only works
together with option -l.

Option o:
To override the default name of the output file. The extension ".bin",
".hex", or ".c" will be added when none is specified. Without this
option the name of the output file becomes the name of the first input
file, but with the extension ".bin", ".hex", or ".c".

Option l:
Without this option no list file will be generated. With -l a list file
with the name of the first source file but extension ".lis" will be
generated. An optional file name path may be added to this option, which
has the extension ".lis" appended when none is specified.

Option d:
This option predefines symbols with a value of 0 or the value of the
expression and may be used multiple times.


Pseudo Operations:

Definition of symbols and allocation of memory:

         ORG    <expression>    - set program address
         .PHASE <expression>    - set logical program counter
         .DEPHASE               - end of logical phase block
<symbol> EQU    <expression>    - define constant symbol
<symbol> DEFL   <expression>    - define variable symbol
<symbol> DEFB   <exp,'str',...> - write bytes into memory
<symbol> DEFM   <exp,'str',...> - same as DEFB
<symbol> DEFC   <exp,'str',...> - as DEFB, but bit 7 in the last byte is set
<symbol> DEFT   <exp,'str',...> - as DEFB, but a length byte is prepended
<symbol> DEFZ   <exp,'str',...> - as DEFB, but an extra zero byte is added
<symbol> DEFW   <exp,exp...>    - write words (16 bits) into memory
<symbol> DEFS   <exp>[,<exp>]   - reserve space in memory, opt. initialized

The aliases ASET (and SET in 8080 mode) for DEFL, DB for DEFB, DC for
DEFC, DS for DEFS, and DW for DEFW are also accepted.

External symbol declarations:

PUBLIC <symbol>         - make symbol public
EXTRN  <symbol>         - symbol is defined external

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
IRP     <id>,<<cl>,...> - inline macro, iterate over character lists
IRPC    <id>,<clist>    - inline macro, iterate over character list
REPT    <count>         - inline macro, repeat <count> times
ENDM                    - end macro
EXITM                   - exit macro expansion
MACLIB  <filename>      - include macro library (an alias for INCLUDE)
IFB     <seq>           - assemble if char sequence inside <> is empty
IFNB    <seq>           - assemble if char sequence inside <> is not empty
IFIDN   <seq1>,<seq2>   - assemble if char sequences inside <> are equal
IFDIF   <seq1>,<seq2>   - assemble if char sequences inside <> are not equal


Manipulation of list file:

PAGE    [<expression>]  - number of lines/page or eject
LIST                    - listing on
NOLIST                  - listing off
.SFCOND                 - suppress listing of conditional false blocks
.LFCOND                 - enable listing of conditional false blocks
.XALL                   - only list macro expansions lines that produce code
.LALL                   - list all macro expansions lines
.SALL                   - list no macro expansions lines
TITLE   <'string'>      - define title for page header

The aliases EJECT for PAGE, .LIST for LIST, .XLIST for NOLIST are
also accepted.


Others:

INCLUDE <filename>      - include another source file
PRINT   <'string'>      - print string to stdout
.PRINTX <d><string><d>  - print string with delimiters <d> to stdout
.8080                   - switch to 8080 instruction set
.Z80                    - switch to Z80 instruction set
.RADIX                  - change numbers radix (default 10)
ASEG                    - absolute code segment (the only mode z80asm supports)

The alias ABS for ASEG is also accepted.


Precedence for expression operators:

( )
unary + - ~ NOT HIGH LOW NUL TYPE
* / MOD % SHR >> SHL <<
+ -
EQ = == NE <> != LT < LE <= GT > GE >=
AND & XOR ^ OR |

Usage of the %, ^, >>, <<, <>, <, <=, >, and >= operators in macro
parameters and ^ in macros is not possible, since they clash with the
literalize character ^, the pass by value % and <> bracket lists.

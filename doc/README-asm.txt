Usage:

z80asm -f[b|m|h] -s[n|a] -e<num> {-h<num>} {-x} {-8} {-u} -v
       -ofile -l[file] -dsymbol ... file ...

A maximum of 512 source files is allowed. If the filename of a source
doesn't have an extension the default extension ".asm" will be
concatenated. Source file names may have a path, the maximum length of
a full qualified filename is 2048 characters.
For relative paths the extension must be used, because all characters
after a "." would be used as extension!

Option f:
Format of the output file:

	-fb -> binary file
	-fm -> binary file with Mostek header
	-fh -> Intel hex

The default is Intel hex now, in earlier versions it was Mostek binary,
but this format is not used much anymore.

Option s:
This option writes the unsorted symbol table (-s), sorted by name (-sn)
or sorted by address (-sa) into the list file. This option only works
together with option -l.

Option e:
Set maximal symbol lenght to <num>, the default is 8.

Option h:
Set the maximum number of bytes per hex record to <num>, the default is 32.

Option x:
Don't fill binary files up to the last used logical address. This means,
that single parameter DEFS's at the end of your source file won't fill up
the output file with 0xff's.
Useful for CP/M BIOS's, where unallocated data doesn't need to be
part of the system image, if the complete image won't fit on the system
tracks.
Only valid for binary output file formats.

Option 8:
Change default instruction set to 8080.

Option u:
Accept undocumented Z80 instructions.

Option v:
Verbose operation of the assembler.

Option o:
To override the default name of the output file. Without this option
the name of the output file becomes the name of the input file,
but with the extension ".bin" or ".hex". The output file may have a path,
the maximum length is limited to 2048 characters.

Option l:
Without this option no list file will be generated. With -l a list
file with the name of the source file but extension ".lis" will be
generated. An optional file name with path (2048 characters maximum)
may be added to this option.

Option d:
This option predefines symbols with a value of 0.
The number of this option is not limited in the command line.


Pseudo Operations:

Definition of symbols and allocation of memory:

	 ORG    <expression>    - set program address
<symbol> .PHASE <expression>    - set logical program counter
	 .DEPHASE               - end of logical phase block
<symbol> EQU    <expression>    - define constant symbol
<symbol> DEFL   <expression>    - define variable symbol
<symbol> DEFB   <exp,'char',..> - write bytes in memory
<symbol> DEFW   <exp,exp..>     - write words (16 bits) in memory
<symbol> DEFM   <'string'>      - write character string in memory
<symbol> DEFC   <'string'>      - as DEFM, but bit 7 in the last char is set
<symbol> DEFZ   <'string'>      - as DEFM, but an extra zero byte is added
<symbol> DEFS   <exp>[,<exp>]   - reserve space in memory, opt. initialized

The aliases ASET (and SET in 8080 mode) for DEFL, DB for DEFB, DC for DEFC,
DS for DEFS, and DW for DEFW are also accepted.

External symbol declarations:

PUBLIC <symbol>		- make symbol public
EXTRN  <symbol>		- symbol is defined external

The aliases ENT, ENTRY, and GLOBAL for PUBLIC, and
EXT and EXTERNAL for EXTRN are also accepted.

The pseudo operations for external symbols are accepted, but won't
do anything. Source modules can be concatenated or included, so the
symbols will be resolved, but the PUBLIC/EXTERN declarations can
be left unaltered in the source modules, because the assembler
ignores them.


Conditional assembly:

IF      <expression>    - assemble if expression evaluates to not 0
IFF     <expression>    - assemble if expression evaluates to 0
IFDEF   <symbol>        - assemble if symbol defined
IFNDEF  <symbol>        - assemble if symbol not defined
IFEQ    <exp1,exp2>     - assemble if equal
IFNEQ   <exp1,exp2>     - assemble if not equal
ELSE                    - else for all conditionals
ENDIF                   - end of conditional assembly

The aliases COND and IFT for IF, and IFE for IFF, and ENDC for ENDIF
are also accepted.


Manipulation of list file:

PAGE    <expression>    - number of lines/page
EJECT                   - skip to new page
LIST                    - listing on
NOLIST                  - listing off
TITLE   <'string'>      - define title for page header


Others:

INCLUDE <filename>      - include another source file
PRINT   <'string'>      - print string to stdout in pass one
.8080                   - switch to 8080 instruction set
.Z80                    - switch to Z80 instruction set
.RADIX                  - change numbers radix (default 10)


Operator precedence for the parser from Thomas Eberhardt:

() [] {}
unary + - ~ NOT HIGH LOW TYPE NUL
* / MOD SHR >> SHL <<
+ -
EQ = == NE <> != LT < LE <= GT > GE >=
AND & XOR OR |

(% for MOD, ^ for XOR, < for SHL, and > for SHR are no longer valid)


Operator precedence for the (old) node based parser from Didier Derny:

()
unary + - ~
* / %
+ -
< >
&
^
|


Operator precedence for my (original) recursive parser:

()

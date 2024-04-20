#!/bin/sh

# Simple minded script to test that I didn't broke the assembler

# These assemble, but no supplied HEX file to verify generated code
ASM_8080_NO_HEX="
../cpmsim/srccpm2/bios8080.asm
../imsaisim/srccpm2/bios.asm
../imsaisim/srccpm2/boot.asm
../imsaisim/srccpm3/boot.asm
"

# These assemble, but no supplied HEX file to verify generated code
ASM_Z80_NO_HEX="
../cpmsim/srccpm2/bios.asm
../cpmsim/srccpm2/boot.asm
../cpmsim/srccpm3/boot.asm
../cpmsim/srcmpm/boot.asm
../cpmsim/srcucsd-iv/bios.asm
../cpmsim/srcucsd-iv/boot.asm
../imsaisim/srcucsd-iv/bios.asm
../imsaisim/srcucsd-iv/boot.asm
"

# These assemble and match the supplied HEX and list file
ASM_8080="
../altairsim/basic8k78.asm
../altairsim/dzmation.asm
../altairsim/fdct1.asm
../altairsim/killbits.asm
../altairsim/killbits2.asm
../altairsim/kscope.asm
../altairsim/life.asm
../altairsim/microchess.asm
../altairsim/roms/als8-rom.asm
../altairsim/roms/apple.asm
../altairsim/roms/bootromt.asm
../altairsim/roms/bootromt-old.asm
../altairsim/roms/cuter-mits.asm
../altairsim/roms/dbl.asm
../altairsim/roms/mbl.asm
../altairsim/roms/miniboot.asm
../altairsim/roms/tinybasic-1.0.asm
../altairsim/roms/tinybasic-2.0.asm
../altairsim/roms/turnmon.asm
../cromemcosim/dzmation.asm
../cromemcosim/kscope.asm
../cromemcosim/life.asm
../cromemcosim/microchess.asm
../cromemcosim/roms/z1mon-1.0.asm
../imsaisim/dzmation.asm
../imsaisim/kscope.asm
../imsaisim/life.asm
../imsaisim/microchess.asm
../imsaisim/roms/basic8k.asm
../imsaisim/roms/memon80.asm
../imsaisim/roms/viofm1.asm
../imsaisim/scs1.asm
"

# These assemble and match the supplied HEX and list file
ASM_Z80="
../altairsim/roms/umzapex.asm
../altairsim/roms/zapple.asm
../cromemcosim/roms/rdos1.asm
../cromemcosim/roms/rdos252.asm
../cromemcosim/roms/z1mon-1.4.asm
../imsaisim/roms/basic4k.asm
"

RESULT=0

for i in $ASM_8080_NO_HEX
do
	BASE="`dirname "$i"`/`basename "$i" .asm`"
	ln -f "$i" test-tmp.asm
	echo "Checking $i"
	echo
	./z80asm -8 -p0 -e16 test-tmp.asm || RESULT=1
	echo "No HEX file for comparison supplied"
	echo "--------------------------------------------------------------"
	rm test-tmp.asm test-tmp.hex
done

for i in $ASM_Z80_NO_HEX
do
	BASE="`dirname "$i"`/`basename "$i" .asm`"
	ln -f "$i" test-tmp.asm
	echo "Checking $i"
	echo
	./z80asm -p0 -e16 test-tmp.asm || RESULT=1
	echo "No HEX file for comparison supplied"
	echo "--------------------------------------------------------------"
	rm test-tmp.asm test-tmp.hex
done

for i in $ASM_8080
do
	BASE="`dirname "$i"`/`basename "$i" .asm`"
	ln -f "$i" test-tmp.asm
	echo "Checking $i"
	echo
	./z80asm -8 -p0 -e16 -l -sn test-tmp.asm || RESULT=1
	diff test-tmp.hex "$BASE".hex || RESULT=1

	# strip listing lines that won't match
	grep -Ev '^(?Z80(/8080)?(-Macro)?-Assembler(	|  )Release|Source file:|Title:       Symbol ?table)' "$BASE".lis > test-tmp-1.lis
	grep -Ev '^(?Z80/8080-Macro-Assembler  Release|Source file:|Title:       Symbol table)' test-tmp.lis > test-tmp-2.lis

	diff test-tmp-1.lis test-tmp-2.lis || RESULT=1
	echo "--------------------------------------------------------------"
	rm test-tmp.asm test-tmp.hex test-tmp.lis test-tmp-1.lis test-tmp-2.lis
done

for i in $ASM_Z80
do
	BASE="`dirname "$i"`/`basename "$i" .asm`"
	ln -f "$i" test-tmp.asm
	echo "Checking $i"
	echo
	./z80asm -p0 -e16 -l -sn test-tmp.asm || RESULT=1
	diff test-tmp.hex "$BASE".hex || RESULT=1

	# strip listing lines that won't match
	grep -Ev '^(?Z80(/8080)?(-Macro)?-Assembler(	|  )Release|Source file:|Title:       Symbol ?table)' "$BASE".lis > test-tmp-1.lis
	grep -Ev '^(?Z80/8080-Macro-Assembler  Release|Source file:|Title:       Symbol table)' test-tmp.lis > test-tmp-2.lis

	diff test-tmp-1.lis test-tmp-2.lis || RESULT=1
	echo "--------------------------------------------------------------"
	rm test-tmp.asm test-tmp.hex test-tmp.lis test-tmp-1.lis test-tmp-2.lis
done

if [ $RESULT -eq 0 ]
then
	echo "Everything OK"
else
	echo "Something went wrong"
fi

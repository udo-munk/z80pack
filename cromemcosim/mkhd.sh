#!/bin/sh

#
# script to create empty disk images for the WDI-II hard disk controller
#
# History:
# August 02 2022	Udo Munk	first version
#

Usage () {
	echo "Usage: `basename $0` 0-3"
}

if [ $# -lt 1 ]; then
	Usage
	exit 1
fi

drive=$1
if [ $drive -lt 0 -o $drive -gt 3 ]; then
	Usage
	exit 1
fi

if [ -f disks/hd${drive}.hdd ]; then
	echo "disk image file already exists"
	exit 1
fi

dd if=/dev/zero of=disks/hd${drive}.hdd count=21240 bs=512
ret=$?
exit $ret

#!/bin/sh

#
# user friendly tool script to put a file from the hosts filesystem
# on the cpmsim disk images using cpmtools
#
# History:
# 07-01-2014	Udo Munk	first version
# 17-03-2016	Udo Munk	cpmsim disk images use extension .dsk now too
#

# the directory with the cpmsim disk images
# needs to be adjusted, default is dist dir in users home
diskdir=~/z80pack/cpmsim/disks

Usage () {
	echo "Usage: `basename $0` [-t] drive [user:]file"
}

# check arguments
if [ $# -lt 2 -o $# -gt 3 ]; then
	Usage
	exit 1
fi
if [ $# -eq 3 -a $1 != "-t" ]; then
	Usage
	exit 1
fi

# process arguments
if [ $# -eq 3 ]; then
	text="-t"
	disk=$2
	diskfile=$diskdir"/drive"$2".dsk"
	file=$3
else
	text=""
	disk=$1
	diskfile=$diskdir"/drive"$1".dsk"
	file=$2
fi
case $disk in
	"a") format="-f ibm-3740"
	;;
	"b") format="-f ibm-3740"
	;;
	"c") format="-f ibm-3740"
	;;
	"d") format="-f ibm-3740"
	;;
	"i") format="-f z80pack-hd"
	;;
	"j") format="-f z80pack-hd"
	;;
	"p") format="-f z80pack-hdb"
	;;
	*) echo "unknown disk $disk, cpmsim disks are a, b, c, d, i, j and p"
	exit 1
	;;
esac

user=`echo $file | cut -d ':' -f 1`
fn=`echo $file | cut -d ':' -f 2`

if [ ! -f $diskfile ]; then
	echo "disk $disk not mounted on $diskfile"
	exit 1
fi
if [ ! -f $fn ]; then
	echo "can't copy file $fn, it doesn't exist"
	exit 1
fi

if [ $user = $fn ]; then
	user='0'
fi

# put file with cpmtools
cmd="cpmcp $text $format $diskfile $fn $user:$fn"
#echo $cmd
$cmd

	Quickstart to run CP/M and MP/M on the Z80-CPU simulation

1. Change to directory ~/z80pack-x.y/cpmsim/srcsim
   make -f Makefile.operating-system
   make -f Makefile.operating-system clean
This compiles the CPU and hardware emulation needed to run CP/M and MP/M.

2. Change to directory ~/z80pack-x.y/cpmsim/srctools
   make
   make install
   make clean
This compiles and installs additional support programs, see below.

3. Make backup copies of your distribution disks!
   cd ~/z80pack-x.y/cpmsim/disks/library
   cp -p *.dsk ../backups

4. Change to directory ~/z80pack-x.y/cpmsim
   ./cpm2 - run CP/M 2.2
   ./cpm3 - run CP/M 3.0
   ./mpm  - this boots CP/M 2, run command mpmldr to boot MP/M 2

The CP/M program bye.com is included on all disk images and is used
to terminate the emulation.

Usage of the support programs:

mkdskimg:
	to create an empty disk image for the CP/M simulation.
	input: mkdskimg <a | b | c | d | i | j | p>
	output: in directory disks files drivea.dsk, driveb.dsk,
		drivec.dsk, drived.dsk, drivei.dsk, drivej.dsk
		and drivep.dsk

bin2hex:
	converts binary files to Intel hex.

receive:
	This is a process spawned by cpmsim under UNIX. It reads from
	the named pipe auxout and writes all input from the pipe to the
	file, which is given as first argument. cpmsim spawns this process
	with the output filename auxiliaryout.txt. Inside the simulator
	this pipe is connected to I/O-port 5, which is assigned
	to the CP/M device PUN:. So everything you write from CP/M
	to device PUN: goes into the file auxiliaryout.txt on the
	UNIX host.

send:
	This process is to send a file from the UNIX host to the
	simulator. Type send <filename> &, and then run cpmsim.
	The process writes all data from file into the named pipe
	auxin, which is also connected to I/O-port 5, which is
	assigned to the CP/M 2 device RDR:. You may use this to
	transfer a file from the UNIX host to the simulator.
	Under CP/M 2 type pip file=RDR: to read the data send from
	the process on the UNIX host. Under CP/M 3 the device name
	is AUX: for both directions.

If you use PIP to transfer files between the host system and the
simulator, you can only use ASCII files, because pip uses CNTL-Z
for EOF! To transfer a binary file from the host system to the
simulator convert it to Intel hex format with bin2hex. This
can be converted back to a binary file under CP/M with the LOAD
command.

Copy the the both shell scripts cpmr.sh and cpmw.sh to ~/bin
or /usr/local/bin. Edit the line with: 
	diskdir=~/z80pack-x.y/cpmsim/disks
to the path where you have extracted the z80pack distribution.

cpmr:
	Uses cpmtools to read a file from a CP/M disk image.
	Usage: cpmr [-t] drive [user:]file
	Option -t does the text file conversions between UNIX
	and CP/M for text files. The user number 0-15 is optional.

cpmw:
	Uses cpmstools to write a file to a CP/M disk image.
	Usage: cpmw [-t] drive [user:]file
	Option -t does the text file conversions between UNIX
	and CP/M for text files. The user number 0-15 is optional.

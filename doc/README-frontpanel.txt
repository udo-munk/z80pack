The original frontpanel library is available at:

	http://sourceforge.net/projects/frontpanel/

z80pack is integrated with the latest version of the frontpanel library.

The frontpanel configuration files have small modifications from
the released version at sourceforge, due to somewhat different directory
structure of the packages.

z80pack uses GNU make, so you must use "gmake" on BSD systems wherever
"make" is used in the following text.

To build the Altair 8800, IMSAI 8080 and Cromemco Z-1 emulations including
the frontpanel build the simulated machines as follows:

cd ~/z80pack-x.y/altairsim/srcsim
make
make clean

cd ~/z80pack-x.y/imsaisim/srcsim
make
make clean

cd ~/z80pack-x.y/cromemco/srcsim
make
make clean

If you don't want to include the frontpanel replace the first "make"
command with "make FRONTPANEL=NO".

To run the systems change into directory ~/z80pack-x.y/imsaisim
and run the program imsasim. To load memory with the included
programs use imsaisim -xbasic8k.hex. To boot an operating system
run the included scripts like ./imsai-cpm13.
The IMSAI emulation by default is the 3D model, if you want the
2D model change the symbolic link to the 2D configuration as follows:

cd ~/z80pack-x.y/imsaisim
rm conf
ln -s conf_2d conf

The 3D model also can be switched between 2D and 3D with the v-key pressed
in the frontpanel window.

The Cromemco Z-1 frontpanel was derived from the IMSAI frontpanel,
so it comes with 2D and 3D models, here 2D is the default.

Running the Altair emulation is the same, just change to directory
~/z80pack-x.y/altairsim and run program altairsim. The Altair emulation
comes with 2D model only.

The default CPU speed for the Altair and IMSAI emulations is 2 MHz
and for the Cromemco emulation 4 Mhz, as with the original machines.
This can be changed with the command line option -fx, where x is the
desired CPU speed in MHz. With a value of 0 the CPU speed is unlimited
and the emulation runs as fast as possible.

The Cromemco emulation is sensitive for the speed setting, best is to
run it at the default 4 Mhz. Higher settings have an influence on
timings and some software won't run correct anymore.

Some functions of I/O devices such as the emulated SIO boards can be
configured in the configuration files:

~/z80pack-x.y/imsaisim/conf/iodev.conf
~/z80pack-x.y/altairsim/conf/iodev.conf

These configuration files include comments, usage of the options should
be obvious.

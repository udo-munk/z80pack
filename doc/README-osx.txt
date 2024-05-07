For installation on OSX you need compiler command line tools, xcode is
not mandatory. If you execute gcc --version on a new system, it will
tell you the command to install compilers.

For the frontpanel machines you need X11, install the latest XQuartz
release from here: https://www.xquartz.org/

Apple still doesn't include JPEG support, probably because of
licence and patent issues. There are open source implementations
that private users are free to use without legal hassles. For this
you need to install brew first, which you will need anyway:

https://brew.sh/

Then install JPEG support with 'brew install jpeg'.

Now you are all set and should be able to build the z80pack machines
from source, following the other instructions here.

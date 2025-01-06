For installation on OSX you need compiler command line tools, xcode is
not mandatory. If you execute gcc --version on a new system, it will
tell you the command to install compilers.

If you want to use X11 with the frontpanel machines
---------------------------------------------------

Install the latest XQuartz release from here: https://www.xquartz.org/

You also need a JPEG library for the frontpanel machines. For this
you need to install brew first, which you will need anyway:

https://brew.sh/

Then install JPEG support with 'brew install jpeg'.

Now you are all set and should be able to build the z80pack machines
from source, following the other instructions here.

If you want to use SDL2 with the frontpanel machines
----------------------------------------------------

Install the SDL2 and SDL2_image frameworks from
https://github.com/libsdl-org/SDL/releases/
and
https://github.com/libsdl-org/SDL_image/releases/

Grab the latest SDL2-2.x.x.dmg and SDL2_image-2.x.x.dmg, and copy
the framework bundles into /Library/Frameworks (create this directory
if it doesn't exist).

Now you are all set and should be able to build the z80pack machines
from source, following the other instructions here.

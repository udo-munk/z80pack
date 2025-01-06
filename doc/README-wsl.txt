Getting z80pack to run on Windows 10/11:

Install Windows feature "Virtual Machine Platform".
On Windows 10 also install "Windows Subsystem for Linux".

Then install "Windows Subsystem for Linux" from the Microsoft Store.
On Windows 10 also install "Windows Terminal" from the Microsoft Store
for a nicer terminal.

Start Windows Terminal and enter "wsl --install Ubuntu".

In the Ubuntu shell enter "mkdir bin", then "exit".

Open a new Windows Terminal and start a "Ubuntu" session.

For using X11, enter:
sudo apt install gcc make libjpeg-turbo8-dev libgl1-mesa-dev libglu1-mesa-dev

Or for SDL2, enter:
sudo apt install gcc make libsdl2-dev libsdl2-image-dev

Now unpack the z80pack source distribution and follow, for
example, "README-frontpanel.txt".

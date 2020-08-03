Webfrontend README.md
===================
Installation notes for **civetweb** ref: [https://github.com/civetweb/civetweb](https://github.com/civetweb/civetweb)

A full copy of the civetweb project is ~70M can build both the `libcivetweb` library required here and a stand-alone webserver.

To simplify installation I have included the minimum set of files from the project, here in the folder `civetweb`. These files are from [https://github.com/civetweb/civetweb/tree/7259a80f1d1620f351dd93fb6f4acff48c9373db](https://github.com/civetweb/civetweb/tree/7259a80f1d1620f351dd93fb6f4acff48c9373db)
The `Makefile` has been modified to build the library by default, with the required options (see below).

To build the **civetweb** statically linkable library `libcivetweb.a`
```
cd civetweb
make
```
If this fails to build on your platform you will need to clone the **civetweb** project into this `webfrontend` folder and build it manually as follows:
```
git clone https://github.com/civetweb/civetweb.git
cd civetweb
git checkout 7259a80
make lib WITH_WEBSOCKET=1 COPT='-DNO_SSL -DNO_CACHING'
```

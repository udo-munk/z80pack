This library allows USB mass storage access together with USB CDC
stdio. Intended to be used together with
https://github.com/carlk3/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico .

Based on pico-sdk/src/rp2_common/pico_stdio_usb and
pico-sdk/lib/tinyusb/examples/device/cdc_msc .

See CMakeLists.txt, picosim.c, and simcfg.c in srcsim to see how to
use this.

Basically:

Add with "add_subdirectory" and "target_link_libraries(tinyusb_device
stdio_msc_usb)" to CMakeLists.txt. Remove "pico_enable_stdio_usb".

Call "tusb_init()" and "stdio_msc_usb_init()" in "main()". Activate
the mass storage access with "stdio_msc_usb_do_msc()". This will wait
until the medium is ejected by the host OS.

This is done this way, because the FatFS library needs exclusive
access to the storage medium. So USB access can only be activated in
the configuration menu to allow copying of files and disk images
to/from the host OS.

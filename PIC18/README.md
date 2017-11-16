= Building and flashing the firmware =

Before compiling you need to have:

* A checkout of https://github.com/hughsie/m-stack/tree/usb_dfu
* The xc8 compiler installed from Microchip

Then just issue 'make' and then you flash the `bootloader.hex` file to the
device using a hardware programmer, which will also remove the firmware image.

If you just want to flash the firmware do `make install` if you have fwupd
or `dfu-util -D firmware.dfu` will do the same thing.

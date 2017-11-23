Building a new bootloader for the XMEGA
=======================================

The DFU bootloader in the XMEGA does not have any way to boot from the runtime
code into the bootloader and to stay in the bootloader mode. To implement the
DFU runtime interface we need to create a new bootloader and flash it to the
developer board using a JTAG programmer.

This is a multi step process:

 * Cut the PDI/JTAG trace on the back of the XMEGA board
 * Download IAR and choose the 30 day time limited evaluation
 * Download http://www.atmel.com/Images/AVR1916.zip, decompress it somewhere
 * Decompress the ATXMEGA256A3BU source
 * Apply the `atxmega256a3bu.patch` in this folder
 * Load the workspace from `AVR1916/XMEGA_bootloaders_v104/source_code/
	common.services.usb.class.dfu_atmel.device.bootloader.atxmega256a3bu/
	common/services/usb/class/dfu_flip/device/bootloader/xmega/
	atxmega256a3bu/iar` (phew!)
 * Change the default configuration to be *Release* not *Debug*
 * Build everything
 * Connect the XPLAINED board "JTAG&PDI" plug to the *10* pin PDI connector
```
     ____________
     |           |
    ||  1    2   | 1:PDI_DATA    2:VCC
    ||  3    4   | 3:NC          4:NC
    ||  5    6   | 5:PDI_CLK     5:GND
     |___________|
```

 * Flash the XMEGA XPLAINED board using:

    `sudo avrdude -e -c avrispmkII -p x256a3bu -P usb -U flash:w:bootloader_xmega.a90:i -v`

 * Boot the XPLAINED board and flash the runtime using:

    `sudo dfu-tool write a3bu-xplained.hex --force`

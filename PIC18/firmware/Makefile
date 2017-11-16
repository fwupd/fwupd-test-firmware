all: firmware.dfu

PK2CMD_DIR="../../../../pk2cmd/pk2cmd"
CC="/opt/microchip/xc8/v1.34/bin/xc8"

# common to bootloader and firmware
CFLAGS="--chip=18F46J50 "
CFLAGS+="--asmlist "
CFLAGS+="--opt=+speed "
CFLAGS+="-w3 "
CFLAGS+="-nw=3004 "

%.dfu: %.hex
	dfu-tool convert dfu $< $@ 8000

SRC_H =							\
	../ch-config.h					\
	../ch-errno.h					\
	../ch-flash.h					\
	../ColorHug.h					\
	./usb_config.h
SRC_C =							\
	../ch-config.c					\
	../ch-errno.c					\
	../ch-flash.c					\
	../m-stack/usb/src/usb.c			\
	../m-stack/usb/src/usb_dfu.c			\
	../m-stack/usb/src/usb_winusb.c			\
	./firmware.c					\
	./usb_descriptors.c
firmware_CFLAGS =					\
	-I$(srcdir)					\
	-I$(top_builddir)				\
	-I$(top_srcdir)/src				\
	-I../m-stack/usb/include			\
	--codeoffset=0x8000				\
	--rom=0x8000-0xfbff				\
	$(CFLAGS)
firmware.hex: $(SRC_C) $(SRC_H)
	$(CC) $(firmware_CFLAGS) $(SRC_C) -o$@
firmware.dfu: firmware.hex Makefile
	dfu-tool convert dfu $< $@;			\
	dfu-tool set-target-size $@ 4000;		\
	dfu-tool set-vendor $@ 273f;			\
	dfu-tool set-product $@ 1009
install-firmware: firmware.dfu Makefile
	sudo dfu-tool write $< ;

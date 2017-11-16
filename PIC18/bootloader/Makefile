all: bootloader.dfu

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

#  __________________
# /                   0000
# |  Bootloader
# \__________________ 5bff
# /                   5c00
# | Shared Config Space
# \__________________ 5fff
# /                   6000
# |  Saved SRAM
# \__________________ 7fff
# /                   8000
# | User Firmware
# \__________________ ffff

SRC_H =								\
	../ch-config.h						\
	../ch-errno.h						\
	../ch-flash.h						\
	../ColorHug.h						\
	./usb_config.h
SRC_C =								\
	../ch-config.c						\
	../ch-errno.c						\
	../ch-flash.c						\
	../m-stack/usb/src/usb.c				\
	../m-stack/usb/src/usb_dfu.c				\
	../m-stack/usb/src/usb_winusb.c				\
	./bootloader.c						\
	./usb_descriptors.c
bootloader_CFLAGS =						\
	$(CFLAGS)						\
	-I$(srcdir)						\
	-I$(top_builddir)					\
	-I$(top_srcdir)/src					\
	-I../m-stack/usb/include				\
	--rom=0x0000-0x5bff					\
	-DCOLORHUG_BOOTLOADER
bootloader.hex: $(SRC_C) $(SRC_H)
	$(CC) $(bootloader_CFLAGS) $(SRC_C) -o$@
install-bootloader: bootloader.hex
	sudo $(PK2CMD_DIR)/pk2cmd -pPIC18F46J50 -f $< -b $(PK2CMD_DIR)/ -m -r

%.dfu: %.hex
	dfu-tool convert dfu $< $@;				\
	dfu-tool set-target-size $@ 4000

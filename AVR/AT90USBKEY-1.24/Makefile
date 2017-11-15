MCU          = at90usb1287
ARCH         = AVR8
BOARD        = USBKEY
F_CPU        = 8000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = Mouse
SRC          = ../$(TARGET).c ../Descriptors.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = ../../../lufa/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -I../Config \
               -DUSB_VID=0x273f -DUSB_PID=0x2000 \
               -DUSB_VENDOR="L\"Hughski Limited\"" \
               -DUSB_PRODUCT="L\"AT90USBKEY Mouse+DFU Demo\"" \
               -DVERSION_MAJOR=1 -DVERSION_MINOR=2 -DVERSION_MICRO=4
LD_FLAGS     =

# Default target
all:

# Include LUFA-specific DMBS extension modules
DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

# Include common DMBS build system modules
DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk

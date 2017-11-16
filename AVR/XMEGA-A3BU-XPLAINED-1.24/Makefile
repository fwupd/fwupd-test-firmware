MCU          = atxmega256a3bu
ARCH         = XMEGA
BOARD        = A3BU_XPLAINED
F_CPU        = 32000000
F_USB        = 48000000
OPTIMIZATION = s
TARGET       = a3bu-xplained
METAINFO     = a3bu-xplained.metainfo.xml
CABVERSION   = 124
SRC          = ../Mouse.c ../Descriptors.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = ../../../lufa/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -I../Config -I../ \
               -DUSB_VID=0x273f -DUSB_PID=0x2001 \
               -DUSB_VENDOR="L\"Hughski Limited\"" \
               -DUSB_PRODUCT="L\"XMEGA-A3BU-XPLAINED Mouse+DFU Demo\"" \
               -DVERSION_MAJOR=1 -DVERSION_MINOR=2 -DVERSION_MICRO=4
LD_FLAGS     =

all: a3bu-xplained$(CABVERSION).cab

clean:
	rm *.cab

%.cab: $(TARGET).hex $(METAINFO)
	gcab --create --nopath $@ $(TARGET).hex $(METAINFO)

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

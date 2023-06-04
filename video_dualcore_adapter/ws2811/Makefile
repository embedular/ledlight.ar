include $(LIB_EMBEDULAR_PATH)/embedul.ar/makefiles/system.mk

# Video framebuffer, but no standard video adapter
TARGET_SUBSYSTEMS ?= video

LIB_LPCOPEN_CHIP_DRIVERS += ritimer

# Video adapter code executed from SRAM to achieve required performance
TARGET_LPC4337_M0_EXECUTE_FROM_SRAM ?= yes

OBJS += \
	$(LIB_EMBEDULAR_ROOT)/source/arch/arm-cortex/m0_instdelay.o

APP_OBJS += \
    ./ws2811.o

$(BUILD)

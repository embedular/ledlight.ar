include $(LIB_EMBEDULAR_PATH)/embedul.ar/makefiles/system.mk

LIB_EMBEDULAR_CONFIG := SPLASH_SCREENS=0

BUILD_LIBS += 3rd_party/fatfs

APP_OBJS += \
    ./main.o

$(BUILD)

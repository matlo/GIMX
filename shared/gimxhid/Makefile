ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/windows/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/libusb/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/posix/*.c))
endif

CPPFLAGS += -Iinclude -I../
CFLAGS += -fPIC

ifneq ($(OS),Windows_NT)
LDLIBS += -lusb-1.0
else
LDLIBS += -lhid -lsetupapi
endif

include ../Makedefs

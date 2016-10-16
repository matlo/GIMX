ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/windows/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard libusb/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/posix/*.c))
endif

CPPFLAGS += -I../../include -I../
CFLAGS += -fPIC

ifneq ($(OS),Windows_NT)
LDLIBS += -lusb-1.0
else
LDLIBS += -lhid -lsetupapi
endif

include ../../../Makedefs

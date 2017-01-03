ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += ../gimxcommon/src/windows/async.o ../gimxcommon/src/windows/gerror.o
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/libusb/*.c))
OBJECTS += ../gimxcommon/src/posix/async.o
endif

CPPFLAGS += -Iinclude -I../
CFLAGS += -fPIC

ifneq ($(OS),Windows_NT)
LDLIBS += -lusb-1.0
else
LDLIBS += -lhid -lsetupapi
endif

include ../Makedefs

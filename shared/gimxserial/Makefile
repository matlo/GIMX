ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += ../gimxcommon/src/windows/async.o ../gimxcommon/src/windows/gerror.o
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/posix/*.c))
OBJECTS += ../gimxcommon/src/posix/async.o
endif

CPPFLAGS += -Iinclude -I../
CFLAGS += -fPIC

include ../Makedefs

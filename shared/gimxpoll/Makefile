ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += ../common/src/windows/gerror.o
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/posix/*.c))
endif

CPPFLAGS += -Iinclude -I../
CFLAGS += -fPIC

ifeq ($(OS),Windows_NT)
CFLAGS += `sdl2-config --cflags`
LDLIBS += -lws2_32
endif

include ../Makedefs

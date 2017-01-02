ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += ../common/src/windows/gerror.o
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/linux/*.c))
endif

CPPFLAGS += -Iinclude -I../
CFLAGS += -fPIC

include ../Makedefs

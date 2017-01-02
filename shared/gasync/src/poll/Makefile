ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard windows/*.c))
OBJECTS += ../common/windows/gerror.o
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard posix/*.c))
endif

CPPFLAGS += -I../../include -I../
CFLAGS += -fPIC

ifeq ($(OS),Windows_NT)
CFLAGS += `sdl2-config --cflags`
LDLIBS += -lws2_32
endif

LIBNAME=libgimx$(shell basename $(shell pwd))
include ../../../Makedefs

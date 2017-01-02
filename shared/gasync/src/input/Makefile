OBJECTS += $(patsubst %.c,%.o,$(wildcard *.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard hid/*.c))
ifneq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard linux/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard sdl/*.c))
endif

CPPFLAGS += -I../../include -I../
CFLAGS += -fPIC

LDFLAGS += -L../hid
LDLIBS += -lgimxhid

ifeq ($(OS),Windows_NT)
CFLAGS += `sdl2-config --cflags`
LDLIBS += -liconv -lsetupapi -lws2_32
LDFLAGS += -L../poll 
LDLIBS += -lgimxpoll
LDLIBS += `sdl2-config --libs`
else
ifeq ($(UHID),1)
CFLAGS += -DUHID
LDFLAGS += -L../uhid
LDLIBS += -lgimxuhid
endif
LDLIBS += -lXi -lX11
endif

LIBNAME=libgimx$(shell basename $(shell pwd))
include ../../../Makedefs

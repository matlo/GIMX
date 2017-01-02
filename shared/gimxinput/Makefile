OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/hid/*.c))
ifneq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/linux/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard src/sdl/*.c))
endif

CPPFLAGS += -Iinclude -I../
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

include ../Makedefs

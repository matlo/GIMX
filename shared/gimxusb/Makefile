OBJECTS += $(patsubst %.c,%.o,$(wildcard *.c))

CPPFLAGS += -Iinclude -I../
CFLAGS += -fPIC

LDFLAGS += -lusb-1.0

ifeq ($(OS),Windows_NT)
LDFLAGS += -L../timer -L../poll
LDLIBS += -lgimxtimer -lgimxpoll
endif

include ../Makedefs

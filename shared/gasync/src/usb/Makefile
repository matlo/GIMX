OBJECTS += $(patsubst %.c,%.o,$(wildcard *.c))

CPPFLAGS += -I../../include -I../
CFLAGS += -fPIC

LDFLAGS += -lusb-1.0

ifeq ($(OS),Windows_NT)
LDFLAGS += -L../timer -L../poll
LDLIBS += -lgimxtimer -lgimxpoll
endif

LIBNAME=libgimx$(shell basename $(shell pwd))
include ../../../Makedefs

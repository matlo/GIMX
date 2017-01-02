OBJECTS += $(patsubst %.c,%.o,$(wildcard linux/*.c))

CPPFLAGS += -I../../include -I../
CFLAGS += -fPIC

LIBNAME=libgimx$(shell basename $(shell pwd))
include ../../../Makedefs

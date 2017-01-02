OBJECTS += $(patsubst %.c,%.o,$(wildcard src/*.c))

CFLAGS += -fPIC -Iinclude

LIBNAME=lib$(shell basename $(shell pwd))
include ../Makedefs
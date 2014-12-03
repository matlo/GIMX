include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
OBJECTS := $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))
CPPFLAGS+=-I/usr/include/ -I../event/include -Iinclude -I/usr/include/libxml2 -I../controller/include

all: $(LIBNAME)($(OBJECTS))

clean:
	$(RM) $(LIBNAME)

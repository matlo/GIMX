include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(LIBNAME)($(OBJECTS))

clean:
	$(RM) $(LIBNAME)

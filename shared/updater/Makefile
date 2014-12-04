include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

all: $(LIBNAME)

$(LIBNAME): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $?

clean:
	$(RM) $(LIBNAME) $(OBJECTS)

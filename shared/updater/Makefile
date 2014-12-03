include ../../Makedefs

LIBNAME=$(shell basename $(shell pwd)).a
SOURCES = $(wildcard *.cpp)
OBJ = $(SOURCES:.cpp=.o)

all: $(LIBNAME)($(OBJ))

clean:
	$(RM) $(OBJ) $(LIBNAME)

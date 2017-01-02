OBJECTS += $(patsubst %.cpp,%.o,$(wildcard *.cpp))

CXXFLAGS += -fPIC

LDFLAGS += -lstdc++ `curl-config --libs`

LIBNAME=lib$(shell basename $(shell pwd))
include ../Makedefs
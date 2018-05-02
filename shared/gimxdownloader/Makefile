OBJECTS += $(patsubst %.cpp,%.o,$(wildcard *.cpp))

CXXFLAGS += -fPIC

LDFLAGS += -lstdc++ `curl-config --libs`

include ../Makedefs
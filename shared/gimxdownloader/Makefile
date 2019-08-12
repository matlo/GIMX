OBJECTS += $(patsubst %.cpp,%.o,$(wildcard *.cpp))

CXXFLAGS += -fPIC

LDLIBS += -lstdc++ `curl-config --libs`

LDFLAGS += -L../gimxfile
LDLIBS += -lgimxfile

include ../Makedefs
OBJECTS += $(patsubst %.cpp,%.o,$(wildcard *.cpp))

CXXFLAGS += -fPIC

LDFLAGS += -lstdc++

CPPFLAGS += -I../

LDFLAGS += -L../gimxfile
LDLIBS += -lgimxfile

LDFLAGS += -L../gimxdownloader
LDLIBS += -lgimxdownloader

include ../Makedefs
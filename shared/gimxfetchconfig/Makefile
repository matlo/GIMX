OBJECTS += $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

CXXFLAGS += -fPIC -Iinclude

LDLIBS += -lstdc++

LDFLAGS += $(GIMXINPUT_LDFLAGS)
LDLIBS  += $(GIMXINPUT_LDLIBS)

include ../Makedefs

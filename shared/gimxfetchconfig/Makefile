OBJECTS += $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

CPPFLAGS += -fPIC -Iinclude -std=c++14

LDLIBS += -lstdc++

LDFLAGS += $(GIMXINPUT_LDFLAGS)
LDLIBS  += $(GIMXINPUT_LDLIBS)

include ../Makedefs

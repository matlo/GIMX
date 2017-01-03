OBJECTS += $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

CXXFLAGS += -fPIC `xml2-config --cflags` -Iinclude

LDFLAGS += -lstdc++ -L../gimxhid -L../gimxinput -L../gimxtimer -L../gimxpoll -L../gimxcontroller
LDLIBS += `xml2-config --libs` -lgimxhid -lgimxinput -lgimxtimer -lgimxpoll -lgimxcontroller

ifeq ($(OS),Windows_NT)
CXXFLAGS += `sdl2-config --cflags`
else
ifeq ($(UHID),1)
LDFLAGS += -L../gimxuhid
LDLIBS += -lgimxuhid
endif
endif

include ../Makedefs
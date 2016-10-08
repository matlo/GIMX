OBJECTS += $(patsubst %.cpp,%.o,$(wildcard src/*.cpp))

CXXFLAGS += -fPIC `xml2-config --cflags` -Iinclude -I../controller/include -I../gasync/include

LDFLAGS += -lstdc++ -L../gasync/src/hid -L../gasync/src/input -L../gasync/src/timer -L../gasync/src/poll -L../controller
LDLIBS += `xml2-config --libs` -lgimxinput -lgimxtimer -lgimxpoll -lgimxcontroller

ifeq ($(OS),Windows_NT)
CXXFLAGS += `sdl2-config --cflags`
else
LDFLAGS += -L../gasync/src/uhid
LDLIBS += -lgimxuhid
endif

include ../Makedefs

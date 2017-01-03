CFLAGS += -s -O3
CXXFLAGS += -s -O3

CPPFLAGS = -I../..

LDFLAGS = -L../../gimxinput -L../../gimxhid -L../../gimxpoll -L../../gimxtimer
LDLIBS = -lgimxinput -lgimxhid -lgimxpoll -lgimxtimer

BINS=ginput_test ginput_haptic_test
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=ginput_test.exe ginput_haptic_test.exe
endif

ifeq ($(OS),Windows_NT)
CPPFLAGS += `sdl2-config --cflags`
LDLIBS += $(shell sdl2-config --libs) -liconv -lws2_32 -lwinmm -lhid -lsetupapi
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
else
LDLIBS += -lXi -lX11
endif

all: $(BINS)

clean:
	$(RM) $(OUT) *~

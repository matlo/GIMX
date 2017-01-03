CFLAGS += -s -O3
CXXFLAGS += -s -O3

CPPFLAGS = -I../..

LDFLAGS = -L../../gimxserial -L../../gimxpoll -L../../gimxtimer -L../../gimxprio
LDLIBS = -lgimxserial -lgimxpoll -lgimxtimer -lgimxprio

LDLIBS += -lm

ifeq ($(OS),Windows_NT)
CPPFLAGS += `sdl2-config --cflags`
LDLIBS += $(shell sdl2-config --libs) -lws2_32 -lpdcursesw -lintl
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
endif


BINS=gserial_test
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=gserial_test.exe
endif

all: $(BINS)

clean:
	$(RM) $(OUT) *~

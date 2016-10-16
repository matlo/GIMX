
OUT = ff_conv_test
OBJS = ff_conv_test.o
BINS = ff_conv_test
CFLAGS = -I../../include -I../../../shared/gasync/include -Wall -Wextra -g -O0
CXXFLAGS = -Wall -Wextra -g -O0

ifeq ($(OS),Windows_NT)
CPPFLAGS += $(shell sdl2-config --cflags)
CPPFLAGS := $(filter-out -Dmain=SDL_main,$(CPPFLAGS))
LDLIBS = -lintl
endif

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o

ff_conv_test: ../../haptic/ff_conv.o ../../haptic/ff_lg.o $(OBJS)

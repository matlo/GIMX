
OUT = ff_lg_test
OBJS = ../../haptic/common/ff_lg.o ../../haptic/haptic_tweaks.o
BINS = ff_lg_test
CFLAGS = -I../../include -I../../../shared -I../../../shared -Wall -Wextra -Werror -g -O0
CXXFLAGS = -Wall -Wextra -Werror -g -O0

ifeq ($(OS),Windows_NT)
CPPFLAGS += $(shell sdl2-config --cflags)
CPPFLAGS := $(filter-out -Dmain=SDL_main,$(CPPFLAGS))
LDLIBS = -lintl
endif

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o

ff_lg_test: $(OBJS)

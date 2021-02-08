
BINS = curses_test
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=gimx.exe
endif

CFLAGS = -Wall -Wextra -Werror -g -O0
CPPFLAGS = -DSDL_MAIN_HANDLED -I../../../shared -I../.. -I/mingw64/include/SDL2

LDFLAGS = -L../../../shared/gimxcontroller -L../../../shared/gimxtime
LDLIBS = -lgimxcontroller -lgimxtime -lintl

ifneq ($(PDCURSES),1)
LDLIBS += -lncursesw
else
CFLAGS += -DPDCURSES
LDLIBS += -lpdcursesw
endif

OBJECTS = ../../display.o ../../stats.o

all: $(BINS)

curses_test: $(OBJECTS)

clean:
	$(RM) $(OBJECTS) $(BINS)

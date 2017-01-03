CFLAGS += -s -O3
CXXFLAGS += -s -O3

CPPFLAGS = -I../..

LDFLAGS = -L../../gimxpoll -L../../gimxtimer -L../../gimxprio
LDLIBS = -lgimxpoll -lgimxtimer -lgimxprio

LDLIBS += -lm

BINS=gtimer_test
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=gtimer_test.exe
endif

all: $(BINS)

clean:
	$(RM) $(OUT) *~

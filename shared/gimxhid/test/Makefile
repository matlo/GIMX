CFLAGS += -s -O3
CXXFLAGS += -s -O3

CPPFLAGS = -I../..

LDFLAGS = -L../../gimxhid -L../../gimxpoll -L../../gimxtimer
LDLIBS = -lgimxhid -lgimxpoll -lgimxtimer

BINS=ghid_test
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=ghid_test.exe
endif

all: $(BINS)

clean:
	$(RM) $(OUT) *~

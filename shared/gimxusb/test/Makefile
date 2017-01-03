CFLAGS += -s -O3
CXXFLAGS += -s -O3

CPPFLAGS = -I../..

LDFLAGS = -L../../gimxusb -L../../gimxpoll -L../../gimxtimer
LDLIBS = -lgimxusb -lgimxpoll -lgimxtimer

BINS=gusb_test
ifneq ($(OS),Windows_NT)
OUT=$(BINS)
else
OUT=gusb_test.exe
endif

all: $(BINS)

clean:
	$(RM) $(OUT) *~

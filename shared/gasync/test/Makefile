CFLAGS += -O3
CXXFLAGS += -O3

CPPFLAGS = -I../include

LDFLAGS = -L../src/hid \
           -L../src/input \
           -L../src/poll \
           -L../src/prio \
           -L../src/serial \
           -L../src/timer \
           -L../src/usb

LDLIBS = -lgimxhid \
          -lgimxinput \
          -lgimxpoll \
          -lgimxprio \
          -lgimxserial \
          -lgimxtimer \
          -lgimxusb

ifeq ($(UHID),1)
LDFLAGS += -L../src/uhid
LDLIBS += -lgimxuhid
endif

OUT=ginput_test ghid_test gserial_test gtimer_test ginput_haptic_test gusb_test
OBJS=common.o
LDLIBS += -lm
BINS=ginput_test ghid_test gserial_test gtimer_test ginput_haptic_test gusb_test
ifeq ($(OS),Windows_NT)
CPPFLAGS += `sdl2-config --cflags`
LDLIBS += $(shell sdl2-config --libs) -lws2_32 -lwinmm -lhid -lsetupapi
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
else
LDLIBS += -lXi -lX11
endif
LDLIBS += `pkg-config libusb-1.0 --libs`

ifeq ($(UHID),1)
OUT += guhid_test
BINS += guhid_test
endif

all: $(BINS)

clean:
	$(RM) $(BINS) *~ *.o

ginput_test: $(OBJS)

ghid_test: $(OBJS)

guhid_test: $(OBJS)

gserial_test: $(OBJS)

gtimer_test: $(OBJS)

gusb_test: $(OBJS)

ginput_haptic_test: $(OBJS)

ifeq ($(OS),Windows_NT)
OBJECTS += $(patsubst %.c,%.o,$(wildcard windows/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/windows/*.c))
else
OBJECTS += $(patsubst %.c,%.o,$(wildcard libusb/*.c))
OBJECTS += $(patsubst %.c,%.o,$(wildcard ../common/posix/*.c))
endif

CPPFLAGS = -I.. -I../../include
CFLAGS += -fPIC

all: $(OBJECTS)

clean:
	$(RM) $(OBJECTS)

install:

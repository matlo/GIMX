OBJECTS += $(patsubst %.c,%.o,$(wildcard *.c))

CFLAGS += -fPIC -Iinclude

LDFLAGS += -L../gimxlog
LDLIBS += -lgimxlog

LDFLAGS += -L../gimxhid
LDLIBS += -lgimxhid

include ../Makedefs
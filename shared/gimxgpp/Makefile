OBJECTS += $(patsubst %.c,%.o,$(wildcard *.c))

CFLAGS += -fPIC -Iinclude

LDFLAGS += -L../gimxlog
LDLIBS += -lgimxlog

LDFLAGS += -L../gimxhid
LDLIBS += -lgimxhid

LDFLAGS += -L../gimxfile
LDLIBS += -lgimxfile

include ../Makedefs
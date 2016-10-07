OBJECTS += $(patsubst %.c,%.o,$(wildcard *.c))

CFLAGS += -fPIC -Iinclude -I../gasync/include

LDFLAGS += -L../gasync/src/hid
LDLIBS += -lgimxhid

include ../Makedefs

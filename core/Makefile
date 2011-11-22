prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
confdir=$(DESTDIR)/etc/emuclient
BINS=emu emuclient sixaddr bdaddr hcirevision
CC=gcc
CFLAGS=-Wall -O2
LIB=-L../../libsdl/lib -lSDL -lxml2 -lXi -lm -lpthread
INC=-I../../libsdl/include -I/usr/include/libxml2

all: $(BINS)

clean:
	rm -f $(BINS) *~ *.o

emu: emu.o sixaxis.o dump.o l2cap_con.o bt_utils.o
	$(CC) -o $@ $^ -lbluetooth
	
emu-serial: emu-serial.o sixaxis.o dump.o
	$(CC) -o $@ $^

emuclient: emuclient.o sixaxis.o dump.o macros.o config.o conversion.o config_writter.o config_reader.o sdl_tools.o tcp_con.o calibration.o serial_con.o lin_serial_con.o
	$(CC) -o $@ $^ $(LIB)

sixaddr: utils/sixaddr.c
	$(CC) -o $@ $^ -lusb

bdaddr: utils/bdaddr.c utils/oui.c
	$(CC) -o $@ $^ -lbluetooth
	
hcirevision: hcirevision.c
	$(CC) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< 
	
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	for i in $(BINS); do cp $$i $(bindir)/; done
	for i in $(BINS); do chmod ug+s $(bindir)/$$i; done
	mkdir -p $(confdir)/config
	mkdir -p $(confdir)/macros
	cp config/* $(confdir)/config
	cp macros/* $(confdir)/macros

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)
	-rm -rf $(confdir)

really-clean: clean uninstall

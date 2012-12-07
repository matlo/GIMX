include ../Makedefs

prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
BINS=emu emuclient sixaddr bdaddr hcirevision usbspoof
LIB=-lxml2 -lm -lncursesw
INC=-Iinclude -I../../libsdl/include -I/usr/include/libxml2

all: lin gpp $(BINS)

clean:
	rm -f $(BINS) *~ *.o */*.o

emu: emu.o sixaxis.o dump.o l2cap_con.o bt_utils.o linux/conversion.o
	$(CC) $(INC) -o $@ $^ -lbluetooth

emu-serial: emu-serial.o sixaxis.o dump.o
	$(CC) -o $@ $^

emuclient: emuclient.o\
       args.o\
       display.o\
       sixaxis.o\
       dump.o\
       macros.o\
       config.o\
       config_writter.o\
       config_reader.o\
       sdl_tools.o\
       calibration.o\
       connector.o\
       serial_con.o\
       report.o\
       tcp_con.o\
       gpp/pcprog.o\
       gpp_con.o\
       gpp/hid_LINUX.o\
       linux/prio.o\
       linux/serial.o\
       linux/mainloop.o\
       linux/events.o\
       linux/conversion.o\
       linux/timer.o
	$(CC) -o $@ $^ $(LIB) -lrt

gpp:
	$(CC) $(CFLAGS) -o gpp/hid_LINUX.o -c gpp/hid_LINUX.c
	$(CC) $(CFLAGS) -o gpp/pcprog.o -c gpp/pcprog.c
	$(CC) $(CFLAGS) -o gpp_con.o -c gpp_con.c

lin:
	$(CC) $(CFLAGS) $(INC) -o linux/prio.o -c linux/prio.c
	$(CC) $(CFLAGS) $(INC) -o linux/serial.o -c linux/serial.c
	$(CC) $(CFLAGS) $(INC) -o linux/mainloop.o -c linux/mainloop.c
	$(CC) $(CFLAGS) $(INC) -o linux/events.o -c linux/events.c
	$(CC) $(CFLAGS) $(INC) -o linux/conversion.o -c linux/conversion.c
	$(CC) $(CFLAGS) $(INC) -o linux/timer.o -c linux/timer.c

sixaddr: utils/sixaddr.c
	$(CC) -o $@ $^ -lusb
	
usbspoof: utils/usbspoof.c utils/pcapwriter.c
	$(CC) -o $@ $^ -lusb-1.0

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

uninstall:
	-for i in $(BINS); do rm $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)

really-clean: clean uninstall

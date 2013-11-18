include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

BINS=emuclient
ifneq ($(OS),Windows_NT)
BINS+= emu sixaddr bdaddr hcirevision usbspoof
OUT=$(BINS)
else
OUT=emuclient.exe
endif
LIB=../shared/event/GE.a
ifneq ($(OS),Windows_NT)
LIB+= -lxml2 -lm -lncursesw -lusb-1.0
else
LIB+= `sdl-config --libs` -llibxml2 -lws2_32 -liconv -lhid -lsetupapi -lpdcursesw -lintl -lusb-1.0
LIB:=$(filter-out -mwindows,$(LIB))
endif
INC=-Iinclude -I../shared/event/include
ifneq ($(OS),Windows_NT)
INC += -I/usr/include/libxml2
endif
DEP=../shared/event/GE.a

all: $(BINS)

clean:
	rm -f $(OUT) *~ *.o */*.o

EMUCLIENT_OBJ = \
       emuclient.o\
       args.o\
       display.o\
       sixaxis.o\
       macros.o\
       config.o\
       config_writter.o\
       config_reader.o\
       calibration.o\
       connector.o\
       report.o\
       tcp_con.o\
       gpp/pcprog.o\
       gpp_con.o\
       usb_spoof.o\
       controllers/controller.o\
       controllers/joystick.o\
       controllers/ds2.o\
       controllers/ds3.o\
       controllers/ds4.o\
       controllers/xbox.o\
       controllers/x360.o

ifneq ($(OS),Windows_NT)
EMUCLIENT_OBJ += \
       gpp/hid_LINUX.o\
       linux/prio.o\
       linux/serial.o\
       linux/mainloop.o
else
EMUCLIENT_OBJ += \
       gpp/hid_WINDOWS.o\
       windows/prio.o\
       windows/serial.o\
       windows/mainloop.o
endif

emuclient: $(DEP) $(EMUCLIENT_OBJ)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB)

ifneq ($(OS),Windows_NT)
emu: emu.o sixaxis.o dump.o l2cap_con.o bt_utils.o linux/prio.o controllers/ds3.o
	$(CC) $(INC) -o $@ $^ -lbluetooth

sixaddr: utils/sixaddr.o
	$(CC) -o $@ $^ -lusb
	
USBSPOOF_OBJ = \
       utils/usbspoof.o\
       usb_spoof.o\
       utils/pcapwriter.o

ifneq ($(OS),Windows_NT)
USBSPOOF_OBJ += \
       linux/serial.o
else
USBSPOOF_OBJ += \
       windows/serial.o
endif

usbspoof: $(USBSPOOF_OBJ)
	$(CC) -o $@ $^ -lusb-1.0

bdaddr: utils/bdaddr.o utils/oui.o
	$(CC) -o $@ $^ -lbluetooth

hcirevision: hcirevision.o
	$(CC) -o $@ $^
endif

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< 

ifneq ($(OS),Windows_NT)
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
endif
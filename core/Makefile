include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

BINS=gimx
ifneq ($(OS),Windows_NT)
BINS+= ds4tool sixaddr bdaddr hcirevision usbspoof
OUT=$(BINS)
else
OUT=gimx.exe
endif
LIB=../shared/event/GE.a -Wl,--whole-archive ../shared/controller/controller.a -Wl,--no-whole-archive
ifneq ($(OS),Windows_NT)
LIB+= -lxml2 -lm -lncursesw -lusb-1.0 -lbluetooth -lmhash
else
LIB+= $(shell sdl2-config --libs) `xml2-config --libs` -lws2_32 -liconv -lhid -lsetupapi -lpdcursesw -lintl -lusb-1.0 -lwinmm
LIB:=$(filter-out -mwindows,$(LIB))
endif
INC=-Iinclude -I../shared/event/include -I../shared/controller/include
ifneq ($(OS),Windows_NT)
INC += -I/usr/include/libxml2
endif
DEP=../shared/event/GE.a ../shared/controller/controller.a

all: $(BINS)

clean:
	rm -f $(OUT) *~ *.o */*.o */*/*.o

GIMX_OBJ = \
       emuclient.o\
       args.o\
       display.o\
       macros.o\
       config.o\
       config_writter.o\
       config_reader.o\
       calibration.o\
       connectors/connector.o\
       connectors/tcp_con.o\
       connectors/udp_con.o\
       connectors/gpp/pcprog.o\
       connectors/gpp_con.o\
       connectors/usb_spoof.o\
       adapter.o

ifneq ($(OS),Windows_NT)
GIMX_OBJ += \
       connectors/sixaxis.o\
       connectors/btds4.o\
       connectors/gpp/hid_LINUX.o\
       connectors/l2cap_con.o\
       connectors/bt_utils.o\
       linux/prio.o\
       connectors/linux/serial.o\
       linux/mainloop.o
else
GIMX_OBJ += \
       connectors/gpp/hid_WINDOWS.o\
       windows/prio.o\
       connectors/windows/serial.o\
       windows/mainloop.o
endif

gimx: $(DEP) $(GIMX_OBJ)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB)

ifneq ($(OS),Windows_NT)
sixaddr: utils/sixaddr.o
	$(CC) -o $@ $^ -lusb
	
USBSPOOF_OBJ = \
       utils/usbspoof.o\
       connectors/usb_spoof.o\
       utils/pcapwriter.o

ifneq ($(OS),Windows_NT)
USBSPOOF_OBJ += \
       connectors/linux/serial.o
else
USBSPOOF_OBJ += \
       connectors/windows/serial.o
endif

usbspoof: $(USBSPOOF_OBJ)
	$(CC) -o $@ $^ -lusb-1.0

ds4tool: utils/ds4tool.o
	$(CC) -o $@ $^ -lusb-1.0

bdaddr: utils/bdaddr.o utils/oui.o
	$(CC) -o $@ $^ -lbluetooth

hcirevision: connectors/hcirevision.o
	$(CC) -o $@ $^ -lbluetooth
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

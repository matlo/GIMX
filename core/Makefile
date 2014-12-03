include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

BINS=gimx
ifneq ($(OS),Windows_NT)
BINS+= ds4tool sixaddr bdaddr hcirevision
OUT=$(BINS)
else
OUT=gimx.exe
endif

ifneq ($(OS),Windows_NT)
LDLIBS += -lxml2 -lm -lncursesw -lusb-1.0 -lbluetooth -lmhash
else
LDLIBS += $(shell sdl2-config --libs) `xml2-config --libs` -lws2_32 -liconv -lhid -lsetupapi -lpdcursesw -lintl -lusb-1.0 -lwinmm
LDLIBS:=$(filter-out -mwindows,$(LDLIBS))
endif
CPPFLAGS+=-Iinclude -I../shared/event/include -I../shared/controller/include
ifneq ($(OS),Windows_NT)
CPPFLAGS += -I/usr/include/libxml2
endif
DEPS=../shared/event/GE.a ../shared/controller/controller.a

all: updated $(BINS)

clean:
	$(RM) $(OUT) *~ *.o */*.o */*/*.o

GIMX_OBJ = \
       mainloop.o\
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
       connectors/usb_con.o\
       connectors/ds4_wrapper.o\
       adapter.o\
       stats.o

ifneq ($(OS),Windows_NT)
GIMX_OBJ += \
       connectors/bt_mgmt.o\
       connectors/sixaxis.o\
       connectors/btds4.o\
       connectors/gpp/hid_LINUX.o\
       connectors/l2cap_con.o\
       connectors/bt_utils.o\
       linux/prio.o\
       connectors/linux/serial.o
else
GIMX_OBJ += \
       connectors/gpp/hid_WINDOWS.o\
       windows/prio.o\
       connectors/windows/serial.o
endif

gimx: $(GIMX_OBJ)

#this rule triggers an update when $(DEPS) is updated
updated: $(DEPS)
	touch updated
	$(RM) $(OUT)

ifneq ($(OS),Windows_NT)
sixaddr: utils/sixaddr.o
	$(CC) $^ -o $@ -lusb-1.0

ds4tool: utils/ds4tool.o
	$(CC) $^ -o $@ -lusb-1.0

bdaddr: utils/bdaddr.o utils/oui.o
	$(CC) $^ -o $@ -lbluetooth

hcirevision: connectors/hcirevision.o
	$(CC) $^ -o $@ -lbluetooth
endif

ifneq ($(OS),Windows_NT)
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	for i in $(BINS); do cp $$i $(bindir)/; done
	for i in $(BINS); do chmod ug+s $(bindir)/$$i; done

uninstall:
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-$(RM) $(bindir)
	-$(RM) $(prefix)

really-clean: clean uninstall
endif

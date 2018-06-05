include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

BINS=
ifneq ($(OS),Windows_NT)
BINS+=ds4tool sixaddr bdaddr hcirevision
OUT=$(BINS)
else
OUT=ds4tool.exe sixaddr.exe
endif

LDLIBS += $(GIMXPOLL_LDLIBS) $(GIMXUSB_LDLIBS)

LDFLAGS += $(GIMXPOLL_LDFLAGS) $(GIMXUSB_LDFLAGS)

all: $(BINS)

clean:
	$(RM) $(OUT) *.o */*.o

sixaddr: sixaddr.o

ds4tool: ds4tool.o

ifneq ($(OS),Windows_NT)
bdaddr: linux/bdaddr.o linux/oui.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -lbluetooth -o $@

hcirevision: linux/hcirevision.o
	$(LINK.o) $^ $(LOADLIBES) $(LDLIBS) -lbluetooth -o $@
endif

ifneq ($(OS),Windows_NT)
install: all
	mkdir -p $(prefix)
	mkdir -p $(bindir)
	for i in $(BINS); do cp $$i $(bindir)/; done
	for i in $(BINS); do chmod ug+s $(bindir)/$$i; done

uninstall:
	-for i in $(BINS); do $(RM) $(bindir)/$$i; done
	-rmdir $(bindir)
	-rmdir $(prefix)

really-clean: clean uninstall
endif

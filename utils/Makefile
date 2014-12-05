include ../Makedefs

ifneq ($(OS),Windows_NT)
prefix=$(DESTDIR)/usr
bindir=$(prefix)/bin
endif

BINS=ds4tool sixaddr
ifneq ($(OS),Windows_NT)
BINS+= bdaddr hcirevision
OUT=$(BINS)
else
OUT=
endif

CPPFLAGS :=

LDLIBS := -lusb-1.0
ifneq ($(OS),Windows_NT)
LDLIBS += -lbluetooth
endif

DEPS=

all: updated $(BINS)

clean:
	$(RM) $(OUT) *.o */*.o

#this rule triggers an update when $(DEPS) is updated
updated: $(DEPS)
	touch updated
	$(RM) $(OUT)

sixaddr: sixaddr.o

ds4tool: ds4tool.o

ifneq ($(OS),Windows_NT)
bdaddr: linux/bdaddr.o linux/oui.o
	$(CC) $^ -o $@ -lbluetooth

hcirevision: linux/hcirevision.o
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

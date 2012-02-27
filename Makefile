DIRS=shared core config bluetooth serial status fpsconfig

all:
	+for i in $(DIRS); do cd $$i; make all; cd ..; done

clean:
	+for i in $(DIRS); do cd $$i; make clean; cd ..; done

install: all
	for i in $(DIRS); do cd $$i; make install; cd ..; done
	mkdir -p $(DESTDIR)/usr/lib
	cp ../libsdl/lib/libSDL.so $(DESTDIR)/usr/lib/libSDL-9.2.so.0
	

uninstall:
	-for i in $(DIRS); do cd $$i; make uninstall; cd ..; done
	-rm $(DESTDIR)/usr/lib/libSDL-9.2.so.0
	-rmdir $(DESTDIR)/usr/lib
	-rmdir $(DESTDIR)/usr

really-clean: clean uninstall

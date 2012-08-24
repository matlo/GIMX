
all:
	

install:
	mkdir -p $(DESTDIR)/usr/share/locale
	for translation in *.po; \
    do \
        po=$$(basename $${translation} .po); \
        mkdir -p $(DESTDIR)/usr/share/locale/$$po/LC_MESSAGES; \
        msgfmt -o $(DESTDIR)/usr/share/locale/$$po/LC_MESSAGES/gimx.mo $$po.po; \
    done

uninstall:
	rm $(DESTDIR)/usr/share/locale/*/LC_MESSAGES/gimx.mo

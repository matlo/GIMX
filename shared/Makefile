DIRS=controller event updater configupdater configeditor

all:
	+for i in $(DIRS); do $(MAKE) -C $$i all; done

clean:
	+for i in $(DIRS); do $(MAKE) -C $$i clean; done

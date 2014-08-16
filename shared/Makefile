DIRS=controller event updater configupdater configeditor

all:
	+for i in $(DIRS); do cd $$i; $(MAKE) all; cd ..; done

clean:
	+for i in $(DIRS); do cd $$i; $(MAKE) clean; cd ..; done

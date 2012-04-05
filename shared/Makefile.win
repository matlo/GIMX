DIRS=updater configupdater configeditor

all:
	+for i in $(DIRS); do cd $$i; make all; cd ..; done

clean:
	+for i in $(DIRS); do cd $$i; make clean; cd ..; done

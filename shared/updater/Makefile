include ../../Makedefs

OBJ=updater.o

all: $(OBJ)
	ar rcsD updater.a.new $(OBJ)
	diff -N updater.a.new updater.a > /dev/null || mv updater.a.new updater.a
	rm -f updater.a.new

clean:
	rm -f $(OBJ) updater.a

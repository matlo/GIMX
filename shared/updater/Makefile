include ../../Makedefs

OBJ=updater.o

all: $(OBJ)
	$(AR) rcsD updater.a.new $(OBJ)
	diff -N updater.a.new updater.a > /dev/null || mv updater.a.new updater.a
	$(RM) updater.a.new

clean:
	$(RM) $(OBJ) updater.a

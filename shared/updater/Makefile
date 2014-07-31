include ../../Makedefs

OBJ=updater.o
INC=

all: $(OBJ)
	ar rcsD updater.a.new $(OBJ)
	diff -N updater.a.new updater.a > /dev/null || mv updater.a.new updater.a
	rm -f updater.a.new

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean:
	rm -f $(OBJ) updater.a

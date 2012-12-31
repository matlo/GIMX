include ../../Makedefs

OBJ=src/linux/events.o src/linux/conversion.o src/linux/timer.o
LIB=
INC=-I/usr/include/ -Iinclude -I../../../libsdl/include

all: $(OBJ)
	ar rcsD event.a.new $(OBJ)
	diff -N event.a.new event.a > /dev/null || mv event.a.new event.a
	rm -f event.a.new

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< $(LIB)

clean:
	rm -f $(OBJ) event.a

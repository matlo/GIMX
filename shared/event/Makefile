include Makedefs
-include ../../Makedefs

OBJ = src/GE.o
ifneq ($(OS),Windows_NT)
OBJ += src/linux/events.o src/linux/conversion.o src/linux/timer.o
else
OBJ += src/windows/events.o src/windows/conversion.o
endif
LIB=
INC=-I/usr/include/ -Iinclude -Isrc

all: $(OBJ)
	ar rcsD GE.a.new $(OBJ)
	diff -N GE.a.new GE.a > /dev/null || mv GE.a.new GE.a
	rm -f GE.a.new

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< $(LIB)

clean:
	rm -f $(OBJ) GE.a

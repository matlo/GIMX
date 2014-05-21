include Makedefs
-include ../../Makedefs

OBJ = src/GE.o
ifneq ($(OS),Windows_NT)
OBJ += src/linux/events.o src/linux/mkb.o src/linux/js.o src/conversion.o src/linux/conversion.o src/linux/timer.o
else
OBJ += src/windows/events.o src/conversion.o src/windows/conversion.o\
       src/windows/winmm/keycodes.o src/windows/winmm/manymouse.o src/windows/winmm/windows_wminput.o src/windows/timer.o
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

include ../../Makedefs

OBJ = src/GE.o src/queue.o
ifneq ($(OS),Windows_NT)
OBJ += src/linux/events.o src/linux/mkb.o src/linux/js.o src/conversion.o src/linux/conversion.o src/linux/timer.o src/linux/xinput.o
else
OBJ += src/windows/events.o src/conversion.o src/windows/conversion.o\
       src/windows/winmm/keycodes.o src/windows/winmm/manymouse.o src/windows/winmm/windows_wminput.o src/windows/timer.o
endif
CPPFLAGS +=-I/usr/include/ -Iinclude -Isrc

all: $(OBJ)
	$(AR) rcsD GE.a.new $(OBJ)
	diff -N GE.a.new GE.a > /dev/null || mv GE.a.new GE.a
	$(RM) GE.a.new

clean:
	$(RM) $(OBJ) GE.a

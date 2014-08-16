include ../../Makedefs

OBJ=src/report.o\
    src/control.o\
    src/controller.o\
    src/ds2.o\
    src/ds3.o\
    src/ds4.o\
    src/joystick.o\
    src/x360.o\
    src/xbox.o\
    src/xone.o
CPPFLAGS+=-Iinclude -Isrc

all: $(OBJ)
	ar rcsD controller.a.new $(OBJ)
	diff -N controller.a.new controller.a > /dev/null || mv controller.a.new controller.a
	$(RM) controller.a.new

clean:
	$(RM) $(OBJ) controller.a

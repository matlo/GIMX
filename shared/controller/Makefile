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
INC=-Iinclude -Isrc

all: $(OBJ)
	ar rcsD controller.a.new $(OBJ)
	diff -N controller.a.new controller.a > /dev/null || mv controller.a.new controller.a
	rm -f controller.a.new

src/%.o: src/%.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< 

clean:
	rm -f $(OBJ) controller.a

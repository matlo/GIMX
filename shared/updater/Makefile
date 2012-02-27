OBJ=updater.o
CC=g++
CFLAGS=-Wall -O2
LIB=
INC=

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< 

clean:
	rm *.o

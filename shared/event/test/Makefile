include ../../../Makedefs

BINS=linux_test
OBJ=linux_test.o common.o
LIB=../GE.a
INC=-I../include
DEPS=../GE.a

all: $(BINS)

clean:
	rm -f $(BINS) *~ *.o

linux_test: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o $@ $(OBJ) $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<


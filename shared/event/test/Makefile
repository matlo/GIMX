include ../../../Makedefs

BINS=linux_test
OBJ=linux_test.o common.o
LIB=../GE.a
INC=-I../include #-I/usr/include/ 

all: $(BINS)

clean:
	rm -f $(BINS) *~ *.o

linux_test: $(OBJ)
	$(CC) $(CFLAGS) $(INC) -o $@ $^ $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<


include ../Makedefs
-include ../../../Makedefs

OBJ=common.o
LIB=../GE.a
ifneq ($(OS),Windows_NT)
OBJ += linux_test.o 
BINS=linux_test
else
OBJ += windows_test.o
BINS=windows_test
LIB += -lmingw32 -lSDLmain -lSDL -liconv
endif

INC=-I../include
DEPS=../GE.a

all: $(BINS)

clean:
	rm -f $(BINS) *~ *.o

ifneq ($(OS),Windows_NT)
linux_test: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o $@ $(OBJ) $(LIB)
else
windows_test: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) $(INC) -o $@ $(OBJ) $(LIB)
endif

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<


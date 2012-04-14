include ../../Makedefs

OBJ=configupdater.o
LIB=
INC=

all: $(OBJ)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< 

clean:
	rm -f $(OBJ)

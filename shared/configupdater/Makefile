include ../../Makedefs

OBJ=configupdater.o
INC=

all: $(OBJ)
	ar rcsD configupdater.a.new $(OBJ)
	diff -N configupdater.a.new configupdater.a > /dev/null || mv configupdater.a.new configupdater.a
	rm -f configupdater.a.new

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

clean:
	rm -f $(OBJ) configupdater.a

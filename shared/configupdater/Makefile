include ../../Makedefs

OBJ=configupdater.o

all: $(OBJ)
	ar rcsD configupdater.a.new $(OBJ)
	diff -N configupdater.a.new configupdater.a > /dev/null || mv configupdater.a.new configupdater.a
	rm -f configupdater.a.new

clean:
	rm -f $(OBJ) configupdater.a

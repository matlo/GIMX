include ../../Makedefs

OBJ=configupdater.o

all: $(OBJ)
	$(AR) rcsD configupdater.a.new $(OBJ)
	diff -N configupdater.a.new configupdater.a > /dev/null || mv configupdater.a.new configupdater.a
	$(RM) configupdater.a.new

clean:
	$(RM) $(OBJ) configupdater.a

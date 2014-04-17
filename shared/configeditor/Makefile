include ../../Makedefs

OBJ=src/event_catcher.o src/XmlWritter.o src/XmlReader.o src/Trigger.o src/Intensity.o src/Event.o src/Device.o src/Controller.o src/ConfigurationFile.o src/Configuration.o src/ControlMapper.o src/MouseOptions.o
LIB=
INC=-I/usr/include/ -I../event/include -Iinclude -I/usr/include/libxml2 -I../controller/include

all: $(OBJ)
	ar rcsD configeditor.a.new $(OBJ)
	diff -N configeditor.a.new configeditor.a > /dev/null || mv configeditor.a.new configeditor.a
	rm -f configeditor.a.new

src/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< $(LIB)

clean:
	rm -f $(OBJ) configeditor.a

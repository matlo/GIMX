include ../../Makedefs

OBJ=src/event_catcher.o src/XmlWritter.o src/XmlReader.o src/Trigger.o src/Intensity.o src/Event.o src/Device.o src/Controller.o src/ConfigurationFile.o src/Configuration.o src/ControlMapper.o src/MouseOptions.o
CPPFLAGS+=-I/usr/include/ -I../event/include -Iinclude -I/usr/include/libxml2 -I../controller/include

all: $(OBJ)
	$(AR) rcsD configeditor.a.new $(OBJ)
	diff -N configeditor.a.new configeditor.a > /dev/null || mv configeditor.a.new configeditor.a
	$(RM) configeditor.a.new

clean:
	$(RM) $(OBJ) configeditor.a

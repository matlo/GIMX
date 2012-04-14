include ../../Makedefs

OBJ=src/event_catcher.o src/XmlWritter.o src/XmlReader.o src/Trigger.o src/Intensity.o src/Event.o src/Device.o src/Controller.o src/ConfigurationFile.o src/Configuration.o src/ButtonMapper.o src/AxisMapper.o
CFLAGS += `wx-config --cflags` -Winvalid-pch -include wx_pch.h -DWX_PRECOMP
LIB=
INC=-I/usr/include/ -Iinclude -I/usr/include/libxml2 -I../../../libsdl/include

all: $(OBJ)
	ar rcs configeditor.a $(OBJ)

src/%.o: src/%.cpp
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< $(LIB)

clean:
	rm -f $(OBJ) configeditor.a

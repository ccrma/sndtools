current: 
	@echo "[morse]: please use one of the following configurations:"; echo "   make linux-alsa, make linux-jack, make linux-oss, make osx, or make win32"

install:
	@echo "(must be root to install)"
	cp $(wildcard morse morse.exe) /usr/bin/; chmod 755 /usr/bin/$(wildcard morse morse.exe)

osx:
	-gcc -D__MACOSX_CORE__ morse.cpp RtAudio.cpp -o morse -lm -framework OpenGL -framework GLUT -framework coreaudio -framework coremidi -framework corefoundation -lstdc++

linux-oss: 
	-make -f makefile.oss 

linux-jack:
	-make -f makefile.jack

linux-alsa: 
	-gcc morse.cpp RtAudio.cpp -o morse -lm -lGL -lGL -lglut -L/usr/X11R6/lib -lXmu -lX11 -lXext -lXi

win32: 
	-make -f makefile.win32

clean:
	rm -f *.o $(wildcard morse morse.exe)

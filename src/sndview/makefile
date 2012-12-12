current: 
	@echo "[sndview]: please use one of the following configurations:"; echo "   make linux-alsa, make linux-jack, make linux-oss, make osx, or make win32"

install:
	@echo "(must be root to install)"
	cp $(wildcard sndview sndview.exe) /usr/bin/; chmod 755 /usr/bin/$(wildcard sndview sndview.exe)

osx: 
	-gcc -g -O3 -D__MACOSX_CORE__ -DBIGENDIAN sndview.c -o sndview -lm -framework OpenGL -framework GLUT -framework coreaudio -framework coremidi -framework corefoundation -lobjc

linux-oss: 
	-make -f makefile.oss 

linux-jack:
	-make -f makefile.jack

linux-alsa: 
	-gcc -DLITTLENDIAN sndview.c -o sndview -lm -lGL -lGL -lglut -L/usr/X11R6/lib -lXmu -lX11 -lXext -lXi

win32: 
	-make -f makefile.win32

clean:
	rm -f *.o $(wildcard sndview sndview.exe)

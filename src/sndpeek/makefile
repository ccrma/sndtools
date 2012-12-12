current: 
	@echo "[sndpeek build]: please use one of the following configurations:"; echo "   make linux-alsa, make linux-jack, make linux-oss, make osx, or make win32"

install:
	cp $(wildcard sndpeek sndpeek.exe) /usr/local/bin/; chmod 755 /usr/local/bin/$(wildcard sndpeek sndpeek.exe)

osx: 
	-make -f makefile.osx

osx-ub:
	-make -f makefile.osx-ub

linux-oss: 
	-make -f makefile.oss 

linux-jack:
	-make -f makefile.jack

linux-alsa: 
	-make -f makefile.alsa

win32: 
	-make -f makefile.win32

clean:
	rm -f *.o $(wildcard sndpeek sndpeek.exe)

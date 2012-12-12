current: 
	@echo "[rt_peaksfft]: please use one of the following configurations:"; echo "   make linux-alsa, make linux-jack, make linux-oss, make osx, or make win32"

install:
	@echo "(must be root to install)"
	cp $(wildcard rt_peaksfft rt_peaksfft.exe) /usr/bin/; chmod 755 /usr/bin/$(wildcard rt_peaksfft rt_peaksfft.exe)

osx: 
	-make -f makefile.osx

linux-oss: 
	-make -f makefile.oss 

linux-jack:
	-make -f makefile.jack

linux-alsa: 
	-make -f makefile.alsa

win32: 
	-make -f makefile.win32

clean:
	rm -f *.o $(wildcard rt_peaksfft rt_peaksfft.exe)

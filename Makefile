# Makefile for DtsEdit
CPP = g++
OBJS = DtsEdit.o EditMain.o Help.o common.o

DtsEdit: $(OBJS)
	$(CPP) -o $@ $(OBJS) /usr/local/lib/libgpac_static.a -lz
	#$(CPP) -o $@ $(OBJS) -lgpac -L/usr/local/lib

.cpp.o:
	$(CPP) -c $< -I /usr/local/include -I .

clean:
	@rm -f DtsEdit *.o *~

DtsEdit.o: stdafx.h Help.h EditMain.h windows.h
EditMain.o: stdafx.h Help.h EditMain.h debug.h
Help.o: Help.h
common.o: stdafx.h Help.h debug.h

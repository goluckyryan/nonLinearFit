CC=g++
#CFLAGS = -Wno-write-strings

all: analysis.o

analysis.o: analysis.cpp anaLibrary.h
	$(CC) analysis.cpp -o analysis.o


clean:
	rm -rfv *.o 

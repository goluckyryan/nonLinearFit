CC=g++
#CFLAGS = -Wno-write-strings

all: analysis

analysis: analysis.cpp anaLibrary.h Matrix.h anaFileIO.h
	$(CC) analysis.cpp -o analysis

clean:
	rm -rfv *.o 

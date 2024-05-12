CC=i686-pc-msdosdjgpp-gcc
CFLAGS=-march=i486 -mtune=i486 -Wall -O2
DEVFLAGS=-g
PROGRAM=view.exe

all:
	$(CC) $(CFLAGS) main.c -o $(PROGRAM)

debug:
	$(CC) $(CFLAGS) $(DEVFLAGS) main.c -o $(PROGRAM)

clean:
	rm -f view.exe	

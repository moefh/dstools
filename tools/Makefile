
CC = gcc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS =

LIBS =

all: hexdump binfloat

clean:
	-rm -f *.o
	-rm -f hexdump binfloat

hexdump: hexdump.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

binfloat: binfloat.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

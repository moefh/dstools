
CC = gcc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS =

LIBS = -lm

all: maps

clean:
	-rm -f *.o *.exe
	-rm -f out/*.objc

maps: genmap
	./genmap
	cp -v out/*.objc ../dsview/maps

genmap: genmap.o gennormals.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

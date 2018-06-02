
CC = gcc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS =

OBJS = genmap.o gennormals.o dir.o
LIBS = -lm

all: maps

clean:
	-rm -f *.o *.exe
	-rm -f out/*.objc

maps: genmap
	./genmap in out
	cp -v out/*.objc ../dsview/maps

genmap: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<


ifeq ($(OS),Windows_NT)
OS_CFLAGS = -I$(DEVROOT)/include
OS_LDFLAGS = 
OS_LIBS = -L$(DEVROOT)/lib
MAPS_DIR = ..\dsview\maps
else
OS_CFLAGS = -fsanitize=address
OS_LDFLAGS = -fsanitize=address
OS_LIBS =
MAPS_DIR = ../dsview/maps
endif

CC = gcc
CFLAGS = $(OS_CFLAGS) -O2 -Wall -Wextra
LDFLAGS = $(OS_LDFLAGS)

OBJS = genmap.o gennormals.o dir.o
LIBS = $(OS_LIBS) -lm

all: genmap

clean:
	-rm -f *.o genmap

maps: genmap
	-mkdir $(MAPS_DIR)
	-./genmap in ../dsview/maps

genmap: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

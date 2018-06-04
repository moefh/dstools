
ifeq ($(OS),Windows_NT)
OS_CFLAGS = -I$(DEVROOT)/include
OS_LDFLAGS = 
OS_LIBS = -L$(DEVROOT)/lib
else
OS_CFLAGS = -fsanitize=address
OS_LDFLAGS = -fsanitize=address
OS_LIBS =
endif

CC = gcc
CFLAGS = $(OS_CFLAGS) -O2 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-cast-function-type
LDFLAGS = $(OS_LDFLAGS)

LIBS = $(OS_LIBS) -lz -lm

all: dcxtool bndtool bhdtool hkxtool dump_nvm

clean:
	-rm -f *.o
	-rm -f dcxtool bndtool bhdtool hkxtool dump_nvm

dcxtool: dcxtool.o dcx.o reader.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

bndtool: bndtool.o bnd.o dcx.o reader.o dump.o util.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

bhdtool: bhdtool.o bhd.o dcx.o reader.o dump.o util.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

hkxtool: hkxtool.o hkx.o bhd.o dcx.o reader.o dump.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

dump_nvm: dump_nvm.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

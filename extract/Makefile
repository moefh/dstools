
CC = gcc
#CFLAGS = -fsanitize=address -g -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-cast-function-type -I$(DEVROOT)/include
#LDFLAGS = -fsanitize=address
CFLAGS = -O2 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-cast-function-type -I$(DEVROOT)/include
LDFLAGS =

LIBS = -L$(DEVROOT)/lib -lz

all: dcxtool bndtool bhdtool hkxtool dump_nvm

clean:
	-rm -f *.o *.exe
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

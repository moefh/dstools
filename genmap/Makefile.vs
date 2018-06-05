
CC = cl
CFLAGS = -nologo -O2 -D_CRT_SECURE_NO_WARNINGS -Drestrict= -I$(DEVROOT)
LDFLAGS = 

OBJS = genmap.obj gennormals.obj dir.obj 
LIBS =

all: genmap.exe

clean:
	-del *.obj genmap.exe

maps: genmap.exe
	-mkdir ..\dsview\maps
	-genmap in ..\dsview\maps

genmap.exe: $(OBJS)
	$(CC) $(LDFLAGS) -Fe$@ $(OBJS) $(LIBS)

.c.obj:
	$(CC) $(CFLAGS) -c $<

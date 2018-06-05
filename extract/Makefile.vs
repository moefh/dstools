
CC = cl
CFLAGS = -nologo -O2 -D_CRT_SECURE_NO_WARNINGS -Drestrict= -I$(DEVROOT)\vs-include
LDFLAGS = 

LIBS = $(DEVROOT)\vs-lib\zlibstatic.lib

all: dcxtool.exe bndtool.exe bhdtool.exe hkxtool.exe dump_nvm.exe

clean:
	-del *.obj dcxtool.exe bndtool.exe bhdtool.exe hkxtool.exe dump_nvm.exe

dcxtool.exe: dcxtool.obj dcx.obj reader.obj
	$(CC) $(LDFLAGS) -Fe$@ $** $(LIBS)

bndtool.exe: bndtool.obj bnd.obj dcx.obj reader.obj dump.obj util.obj
	$(CC) $(LDFLAGS) -Fe$@ $** $(LIBS)

bhdtool.exe: bhdtool.obj bhd.obj dcx.obj reader.obj dump.obj util.obj
	$(CC) $(LDFLAGS) -Fe$@ $** $(LIBS)

hkxtool.exe: hkxtool.obj hkx.obj bhd.obj dcx.obj reader.obj dump.obj
	$(CC) $(LDFLAGS) -Fe$@ $** $(LIBS)

dump_nvm.exe: dump_nvm.obj
	$(CC) $(LDFLAGS) -Fe$@ $** $(LIBS)

.c.obj:
	$(CC) $(CFLAGS) -c $<

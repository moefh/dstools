@echo off

set DSDIR=C:/Program Files (x86)/Steam/steamapps/common/DARK SOULS REMASTERED
set HKXTOOL=..\..\extract\hkxtool.exe

echo looking for files...
gnufind "%DSDIR%/map" -type f -print | grep h.._.._.._...hkxbhd | cut -d/ -f 7- > hkxlist.txt

for /f %%a in (hkxlist.txt) do (
    echo processing %%a...
    %HKXTOOL% x "%DSDIR%/%%a"
)
echo done.

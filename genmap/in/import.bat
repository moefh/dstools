@echo off

set DSDIR=C:\Program Files (x86)\Steam\steamapps\common\DARK SOULS REMASTERED\map
set HKXTOOL=..\..\extract\hkxtool

set MAPLIST=10_00_00_00 10_01_00_00 10_02_00_00 11_00_00_00 12_00_00_00 12_01_00_00 13_00_00_00 13_01_00_00 13_02_00_00 14_00_00_00 14_01_00_00 15_00_00_00 15_01_00_00 16_00_00_00 17_00_00_00 18_00_00_00 18_01_00_00

for %%M in (%MAPLIST%) do (
    echo - m%%M
    %HKXTOOL% x "%DSDIR%\m%%M\h%%M.hkxbhd"
)

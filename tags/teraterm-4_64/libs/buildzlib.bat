cd zlib

if not exist "win32\Makefile.msc.release" goto mkmf
for %%F in (win32\Makefile.msc.release) do set mftime=%%~tF
for %%F in (..\buildzlib.bat) do set battime=%%~tF
if "%battime%" leq "%mftime%" goto build

del zlibd.lib
nmake -f win32\Makefile.msc.release clean

:mkmf
perl -e "open(IN,'win32\Makefile.msc');while(<IN>){s/ -MD/ -MT/;print $_;}close(IN);" > win32\Makefile.msc.release

perl -e "open(IN,'win32\Makefile.msc');while(<IN>){s/ -MD -O2/ -MTd -Od/;s/ -release/ -debug/;s/ zlib.lib/ zlibd.lib/;s/ zlib.lib/ zlibd.lib/;print $_;}close(IN);" > win32\Makefile.msc.debug

:build
if exist zlibd.lib goto build_release
nmake -f win32\Makefile.msc.debug clean
nmake -f win32\Makefile.msc.debug
move zlibd.lib examples\
nmake -f win32\Makefile.msc.release clean
move examples\zlibd.lib .\

:build_release
nmake -f win32\Makefile.msc.release

cd ..
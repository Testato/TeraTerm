set HELP_COMPILER=C:\progra~1\htmlhe~1\hhc.exe
set updated=

CALL convtext.bat

for /f "delims=" %%i in ('perl htmlhelp_update_check.pl ja teratermj.chm') do @set updated=%%i
if "%updated%"=="updated" (
perl htmlhelp_index_make.pl ja html > ja\Index.hhk
%HELP_COMPILER% ja\teraterm.hhp
)

for /f "delims=" %%i in ('perl htmlhelp_update_check.pl en teraterm.chm') do @set updated=%%i
if "%updated%"=="updated" (
perl htmlhelp_index_make.pl en html > en\Index.hhk
%HELP_COMPILER% en\teraterm.hhp
)

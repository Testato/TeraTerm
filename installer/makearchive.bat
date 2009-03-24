CALL convtext.bat
CALL makechm.bat
CALL build.bat

rem  for XP or later
set today=snapshot-%date:~0,4%%date:~5,2%%date:~8,2%

for %%a in (%today%, %today%_1, %today%_2, %today%_3, %today%_4, %today%_5) do (
set dst=%%a
if not exist %%a goto create
)

:create
del /s /q %dst%\*.*
mkdir %dst%

copy /y ..\..\teraterm\visualc\bin\release\*.exe %dst%
copy /y ..\..\teraterm\visualc\bin\release\*.dll %dst%

copy /y ..\..\teraterm\cygterm\cygterm.exe %dst%
copy /y ..\..\teraterm\cygterm\cygterm.cfg %dst%
copy /y ..\..\teraterm\cygterm\cyglaunch.exe %dst%

copy /y ..\..\ttssh2\ttxssh\Release\ttxssh.dll %dst%
copy /y ..\..\TTProxy\Release\TTXProxy.dll %dst%
copy /y ..\..\teraterm\source\ttmenu\Release\ttpmenu.exe %dst%
copy /y ..\..\doc\jp\teratermj.chm %dst%
copy /y ..\..\doc\en\teraterm.chm %dst%

copy /y ..\..\teraterm\release\*.* %dst%
xcopy /s /e /y /i ..\..\teraterm\release\theme %dst%\theme
xcopy /s /e /y /i ..\..\teraterm\release\plugin %dst%\plugin
xcopy /s /e /y /i ..\..\teraterm\release\Collector %dst%\Collector
xcopy /s /e /y /i ..\..\teraterm\release\lang %dst%\lang
rmdir /s /q %dst%\plugin\CVS
rmdir /s /q %dst%\theme\CVS
rmdir /s /q %dst%\theme\scale\CVS
rmdir /s /q %dst%\theme\tile\CVS
rmdir /s /q %dst%\lang\CVS
del /f %dst%\lang\English.lng

perl setini.pl ..\..\teraterm\release\TERATERM.INI > %dst%\TERATERM.INI

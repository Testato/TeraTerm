
set dst=archive

del /s /q %dst%\*.*
mkdir %dst%

copy /y ..\..\teraterm\visualc\bin\release\*.exe %dst%
copy /y ..\..\teraterm\visualc\bin\release\*.dll %dst%

copy /y ..\..\teraterm\cygterm\cygterm.exe %dst%
copy /y ..\..\teraterm\cygterm\cygterm.cfg %dst%

copy /y ..\..\ttssh2\ttxssh\Release\ttxssh.dll %dst%
copy /y ..\..\TTProxy\Release\TTXProxy.dll %dst%
copy /y ..\..\teraterm\source\ttmenu\Release\ttpmenu.exe %dst%
copy /y ..\..\doc\jp\teratermj.chm %dst%
copy /y ..\..\doc\en\teraterm.chm %dst%

copy /y ..\..\teraterm\release\*.* %dst%
xcopy /s /e /y /i ..\..\teraterm\release\theme %dst%\theme
xcopy /s /e /y /i ..\..\teraterm\release\plugin %dst%\plugin
xcopy /s /e /y /i ..\..\teraterm\release\Collector %dst%\Collector
rmdir /s /q %dst%\plugin\CVS
rmdir /s /q %dst%\theme\CVS
rmdir /s /q %dst%\theme\scale\CVS
rmdir /s /q %dst%\theme\tile\CVS

pause

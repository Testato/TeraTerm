goto skip

set dst="C:\Program Files\teraterm"
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.exe %dst%
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.dll %dst%
copy /y C:\usr\cvs\ttssh2\ttxssh\Release\ttxssh.dll %dst%
copy /y C:\usr\cvs\teraterm\source\ttmenu\Release\ttpmenu.exe %dst%

:skip
set dst="C:\usr\src\utf8_teraterm_installer"
del /s /q %dst%\*.*
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.exe %dst%
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.dll %dst%
copy /y C:\usr\cvs\ttssh2\ttxssh\Release\ttxssh.dll %dst%
copy /y C:\usr\cvs\teraterm\source\ttmenu\Release\ttpmenu.exe %dst%

copy /y C:\usr\cvs\teraterm\release\*.* %dst%
xcopy /s /e /y /i C:\usr\cvs\teraterm\release\theme %dst%\theme
xcopy /s /e /y /i C:\usr\cvs\teraterm\release\plugin %dst%\plugin
rmdir /s /q %dst%\plugin\CVS
rmdir /s /q %dst%\theme\CVS
rmdir /s /q %dst%\theme\scale\CVS
rmdir /s /q %dst%\theme\tile\CVS

pause

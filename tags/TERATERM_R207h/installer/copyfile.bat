set dst="C:\Program Files\teraterm"
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.exe %dst%
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.dll %dst%
copy /y C:\usr\cvs\ttssh2\ttxssh\Release\ttxssh.dll %dst%
copy /y C:\usr\cvs\teraterm\source\ttmenu\Release\ttpmenu.exe %dst%

set dst="C:\usr\src\utf8_teraterm_installer"
del /q %dst%\*.*
copy /y C:\usr\cvs\teraterm\release\*.* %dst%
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.exe %dst%
copy /y C:\usr\cvs\teraterm\visualc\bin\release\*.dll %dst%
copy /y C:\usr\cvs\ttssh2\ttxssh\Release\ttxssh.dll %dst%
copy /y C:\usr\cvs\teraterm\source\ttmenu\Release\ttpmenu.exe %dst%

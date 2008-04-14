CALL makechm.bat
CALL build.bat %1

rem  for XP or later
set today=snapshot-%date:~0,4%%date:~5,2%%date:~8,2%

for %%a in (%today%, %today%_2, %today%_3, %today%_4, %today%_5) do (
set dst=%%a
if not exist %%a goto create
)

:create
del /s /q %dst%\*.*
mkdir %dst%

copy /y ..\teraterm\release\*.exe %dst%
copy /y ..\teraterm\release\*.dll %dst%
copy /y ..\ttssh2\ttxssh\Release\ttxssh.dll %dst%
copy /y ..\cygterm\cygterm.exe %dst%
copy /y ..\cygterm\cygterm.cfg %dst%
copy /y ..\cygterm\cyglaunch.exe %dst%
copy /y ..\ttpmenu\Release\ttpmenu.exe %dst%
copy /y ..\TTProxy\Release\TTXProxy.dll %dst%
copy /y ..\TTXKanjiMenu\Release\ttxkanjimenu.dll %dst%
copy /y ..\TTXSamples\Release\*.dll %dst%

rem Debug file
if "%1"=="debug" copy /y ..\teraterm\release\*.pdb %dst%
if "%1"=="debug" copy /y ..\ttssh2\ttxssh\Release\ttxssh.pdb %dst%
if "%1"=="debug" copy /y ..\ttpmenu\Release\ttxssh.pdb %dst%
if "%1"=="debug" copy /y ..\TTProxy\Release\TTXProxy.pdb %dst%
if "%1"=="debug" copy /y ..\TTXKanjiMenu\Release\ttxkanjimenu.pdb %dst%
if "%1"=="debug" copy /y ..\TTXSamples\Release\*.pdb %dst%

pushd %dst%
if exist TTXFixedWinSize.dll ren TTXFixedWinSize.dll _TTXFixedWinSize.dll
popd

copy /y ..\doc\jp\teratermj.chm %dst%
copy /y ..\doc\en\teraterm.chm %dst%

copy /y release\*.* %dst%
xcopy /s /e /y /i release\theme %dst%\theme
xcopy /s /e /y /i release\plugin %dst%\plugin
xcopy /s /e /y /i release\Collector %dst%\Collector
xcopy /s /e /y /i release\lang %dst%\lang
rmdir /s /q %dst%\plugin\CVS
rmdir /s /q %dst%\theme\CVS
rmdir /s /q %dst%\theme\scale\CVS
rmdir /s /q %dst%\theme\tile\CVS
rmdir /s /q %dst%\lang\CVS
del /f %dst%\lang\English.lng

perl setini.pl release\TERATERM.INI > %dst%\TERATERM.INI

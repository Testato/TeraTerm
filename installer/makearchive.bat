@echo off

SET debug=no
SET plugins=no
SET rebuild=
SET release=

if "%1"=="/?" goto help
@echo on
if "%1"=="debug" SET debug=yes
if "%1"=="plugins" SET plugins=yes
if "%1"=="rebuild" SET rebuild=rebuild
if "%1"=="release" SET release=yes

CALL makechm.bat
CALL build.bat %rebuild%

rem  change folder name
if not "%release%"=="yes" goto snapshot
set ver=
for /f "delims=" %%i in ('perl issversion.pl') do @set ver=%%i
set dst=Output\teraterm-%ver%
goto create

:snapshot
rem  for XP or later
for /f "delims=" %%a in ('perl -e "($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = localtime(time); $mon++; printf (1900+$year . substr(\"0\".$mon,-2) . substr(\"0\".$mday,-2));"') do @set today=snapshot-%%a

@for /l %%i in (1,1,10) do @(
if %%i==1 (
set dst=%today%
if not exist %today% goto create
) else (
set dst=%today%_%%i
if not exist %today%_%%i goto create
)
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
copy /y "..\cygterm\cygterm+.tar.gz" %dst%
copy /y "..\cygterm\cygterm.exe" %dst%
mkdir "%dst%\cygterm+-i686"
copy /y "..\cygterm\cygterm.exe" "%dst%\cygterm+-i686"
mkdir "%dst%\cygterm+-x86_64"
copy /y "..\cygterm\cygterm+-x86_64\cygterm.exe" "%dst%\cygterm+-x86_64"
copy /y ..\ttpmenu\Release\ttpmenu.exe %dst%
copy /y ..\TTProxy\Release\TTXProxy.dll %dst%
copy /y ..\TTXKanjiMenu\Release\ttxkanjimenu.dll %dst%
if "%plugins%"=="yes" copy /y ..\TTXSamples\Release\*.dll %dst%
if "%release%"=="yes" copy /y ..\TTXSamples\Release\*.dll %dst%

rem Debug file
if "%debug%"=="yes" copy /y ..\teraterm\release\*.pdb %dst%
if "%debug%"=="yes" copy /y ..\ttssh2\ttxssh\Release\ttxssh.pdb %dst%
if "%debug%"=="yes" copy /y ..\ttpmenu\Release\ttxssh.pdb %dst%
if "%debug%"=="yes" copy /y ..\TTProxy\Release\TTXProxy.pdb %dst%
if "%debug%"=="yes" copy /y ..\TTXKanjiMenu\Release\ttxkanjimenu.pdb %dst%
if "%debug%"=="yes" if "%plugins%"=="yes" copy /y ..\TTXSamples\Release\*.pdb %dst%

if "%plugins%"=="yes" (
pushd %dst%
if exist TTXFixedWinSize.dll ren TTXFixedWinSize.dll _TTXFixedWinSize.dll
if exist TTXResizeWin.dll ren TTXResizeWin.dll _TTXResizeWin.dll
popd
)
if "%release%"=="yes" (
pushd %dst%
if exist TTXOutputBuffering.dll ren TTXOutputBuffering.dll _TTXOutputBuffering.dll
if exist TTXFixedWinSize.dll ren TTXFixedWinSize.dll _TTXFixedWinSize.dll
if exist TTXResizeWin.dll ren TTXResizeWin.dll _TTXResizeWin.dll
if exist TTXShowCommandLine.dll ren TTXShowCommandLine.dll _TTXShowCommandLine.dll
if exist TTXtest.dll ren TTXtest.dll _TTXtest.dll
popd
)

copy /y ..\doc\ja\teratermj.chm %dst%
copy /y ..\doc\en\teraterm.chm %dst%

copy /y release\*.* %dst%
copy /y release\EDITOR.CNF %dst%\KEYBOARD.CNF
xcopy /s /e /y /i /exclude:archive-exclude.txt release\theme %dst%\theme
xcopy /s /e /y /i /exclude:archive-exclude.txt release\plugin %dst%\plugin
xcopy /s /e /y /i /exclude:archive-exclude.txt release\Collector %dst%\Collector
xcopy /s /e /y /i /exclude:archive-exclude.txt release\lang %dst%\lang
del /f %dst%\lang\English.lng
del /f %dst%\svnrev.exe

perl setini.pl release\TERATERM.INI > %dst%\TERATERM.INI

if "%release%"=="yes" (
copy nul %dst%\ttpmenu.ini
)

exit /b

:help
echo Tera Term���r���h���邽�߂ɕK�v�ȃ\�[�X�R�[�h�����ׂăR���p�C�����܂��B(Compiling ALL source codes)
echo.
echo   %0          �ʏ�̃r���h(Normal building)
echo   %0 rebuild  ���r���h(Re-building)
echo   %0 debug    �f�o�b�O���܂ރr���h(Building with Debug information)
echo   %0 plugins  �v���O�C�����܂ރr���h(Building with all plugins)
echo   %0 release  �ʏ�̃r���h + �v���O�C�����܂� + �t�H���_��������(Normal + Plugins building + unique folder naming)
echo      �A�[�J�C�u�Ń����[�X�쐬�p(for archive version released)
echo   %0 rebuild ^>build.log 2^>^&1  �r���h���O���̎悷��(Retrieve building log)
echo.
exit /b


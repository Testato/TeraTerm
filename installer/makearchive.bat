@echo off
rem ���g�p��
rem �ʏ�̃r���h
rem   makearchive.bat
rem ���r���h
rem   makearchive.bat rebuild
rem �f�o�b�O���܂�
rem   makearchive.bat debug
rem �v���O�C���܂�
rem   makearchive.bat plugins

SET debug=no
SET plugins=no

if "%1"=="/?" goto help
@echo on
if "%1"=="debug" SET debug=yes
if "%1"=="plugins" SET plugins=yes

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
copy /y "..\cygterm\cygterm+.tar.gz" %dst%
copy /y ..\ttpmenu\Release\ttpmenu.exe %dst%
copy /y ..\TTProxy\Release\TTXProxy.dll %dst%
copy /y ..\TTXKanjiMenu\Release\ttxkanjimenu.dll %dst%
if "%plugins%"=="yes" copy /y ..\TTXSamples\Release\*.dll %dst%

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

exit /b

:help
echo Tera Term���r���h���邽�߂ɕK�v�ȃ\�[�X�R�[�h�����ׂăR���p�C�����܂��B
echo.
echo   %0          �ʏ�̃r���h
echo   %0 rebuild  ���r���h
echo   %0 debug    �f�o�b�O���܂ރr���h
echo   %0 plugins  �v���O�C�����܂ރr���h
echo.
echo �����[�X�������쐬����ꍇ��"rebuild"���g�p���Ă��������B
exit /b


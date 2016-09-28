
if not "%VSINSTALLDIR%" == "" goto vsinstdir

if "%VS80COMNTOOLS%" == "" goto check_2008
if not exist "%VS80COMNTOOLS%\vsvars32.bat" goto check_2008
call "%VS80COMNTOOLS%\vsvars32.bat"
goto vs2005

:check_2008
if "%VS90COMNTOOLS%" == "" goto check_2010
if not exist "%VS90COMNTOOLS%\vsvars32.bat" goto check_2010
call "%VS90COMNTOOLS%\vsvars32.bat"
goto vs2008

:check_2010
if "%VS100COMNTOOLS%" == "" goto check_2012
if not exist "%VS100COMNTOOLS%\vsvars32.bat" goto check_2012
call "%VS100COMNTOOLS%\vsvars32.bat"
goto vs2010

:check_2012
if "%VS110COMNTOOLS%" == "" goto check_2013
if not exist "%VS110COMNTOOLS%\VsDevCmd.bat" goto check_2013
call "%VS110COMNTOOLS%\VsDevCmd.bat"
goto vs2012

:check_2013
if "%VS120COMNTOOLS%" == "" goto check_2015
if not exist "%VS120COMNTOOLS%\VsDevCmd.bat" goto check_2015
call "%VS120COMNTOOLS%\VsDevCmd.bat"
goto vs2013

:check_2015
if "%VS140COMNTOOLS%" == "" goto novs
if not exist "%VS140COMNTOOLS%\VsDevCmd.bat" goto novs
call "%VS140COMNTOOLS%\VsDevCmd.bat"
goto vs2015

:novs
echo "Can't find Visual Studio"
exit /b

:vsinstdir
rem Visual Studio�̃o�[�W��������
FOR /F "tokens=1,2 delims=\. " %%H IN ("%VSINSTALLDIR:*Visual Studio =Studio %") DO set VSVERSTR="%%H %%I"
if %VSVERSTR% == "Studio 8" goto vs2005
if %VSVERSTR% == "Studio 9" goto vs2008
if %VSVERSTR% == "Studio 10" goto vs2010
if %VSVERSTR% == "Studio 11" goto vs2012
if %VSVERSTR% == "Studio 12" goto vs2013
if %VSVERSTR% == "Studio 14" goto vs2015

rem ���Ғʂ�ɓ����Ȃ������ꍇ�ׁ̈A������������c���Ă���
echo %VSINSTALLDIR% | find "Studio 8"
if not ERRORLEVEL 1 goto vs2005
echo %VSINSTALLDIR% | find "Studio 9"
if not ERRORLEVEL 1 goto vs2008
echo %VSINSTALLDIR% | find "Studio 10"
if not ERRORLEVEL 1 goto vs2010
echo %VSINSTALLDIR% | find "Studio 11"
if not ERRORLEVEL 1 goto vs2012
echo %VSINSTALLDIR% | find "Studio 12"
if not ERRORLEVEL 1 goto vs2013
echo %VSINSTALLDIR% | find "Studio 14"
if not ERRORLEVEL 1 goto vs2015

echo Unknown Visual Studio version
exit /b

:vs2005
set TERATERMSLN=..\teraterm\ttermpro.sln
set TTSSHSLN=..\ttssh2\ttssh.sln
set TTPROXYSLN=..\TTProxy\TTProxy.sln
set TTXKANJISLN=..\TTXKanjiMenu\ttxkanjimenu.sln
set TTPMENUSLN=..\ttpmenu\ttpmenu.sln
set TTXSAMPLESLN=..\TTXSamples\TTXSamples.sln
goto vsend

:vs2008
set TERATERMSLN=..\teraterm\ttermpro.v9.sln
set TTSSHSLN=..\ttssh2\ttssh.v9.sln
set TTPROXYSLN=..\TTProxy\TTProxy.v9.sln
set TTXKANJISLN=..\TTXKanjiMenu\ttxkanjimenu.v9.sln
set TTPMENUSLN=..\ttpmenu\ttpmenu.v9.sln
set TTXSAMPLESLN=..\TTXSamples\TTXSamples.v9.sln
goto vsend

:vs2010
set TERATERMSLN=..\teraterm\ttermpro.v10.sln
set TTSSHSLN=..\ttssh2\ttssh.v10.sln
set TTPROXYSLN=..\TTProxy\TTProxy.v10.sln
set TTXKANJISLN=..\TTXKanjiMenu\ttxkanjimenu.v10.sln
set TTPMENUSLN=..\ttpmenu\ttpmenu.v10.sln
set TTXSAMPLESLN=..\TTXSamples\TTXSamples.v10.sln
goto vsend

:vs2012
set TERATERMSLN=..\teraterm\ttermpro.v11.sln
set TTSSHSLN=..\ttssh2\ttssh.v11.sln
set TTPROXYSLN=..\TTProxy\TTProxy.v11.sln
set TTXKANJISLN=..\TTXKanjiMenu\ttxkanjimenu.v11.sln
set TTPMENUSLN=..\ttpmenu\ttpmenu.v11.sln
set TTXSAMPLESLN=..\TTXSamples\TTXSamples.v11.sln
goto vsend

:vs2013
set TERATERMSLN=..\teraterm\ttermpro.v12.sln
set TTSSHSLN=..\ttssh2\ttssh.v12.sln
set TTPROXYSLN=..\TTProxy\TTProxy.v12.sln
set TTXKANJISLN=..\TTXKanjiMenu\ttxkanjimenu.v12.sln
set TTPMENUSLN=..\ttpmenu\ttpmenu.v12.sln
set TTXSAMPLESLN=..\TTXSamples\TTXSamples.v12.sln
goto vsend

:vs2015
set TERATERMSLN=..\teraterm\ttermpro.v14.sln
set TTSSHSLN=..\ttssh2\ttssh.v14.sln
set TTPROXYSLN=..\TTProxy\TTProxy.v14.sln
set TTXKANJISLN=..\TTXKanjiMenu\ttxkanjimenu.v14.sln
set TTPMENUSLN=..\ttpmenu\ttpmenu.v14.sln
set TTXSAMPLESLN=..\TTXSamples\TTXSamples.v14.sln
goto vsend

:vsend

set BUILD=build
if "%1" == "rebuild" (set BUILD=rebuild)

rem ���C�u�������R���p�C��
pushd ..\libs
CALL buildall.bat
popd

if "%BUILD%" == "rebuild" goto build

rem "rebuild"���w�肵�Ȃ��ꍇ�ASVN���r�W�������X�V����B
if exist ..\teraterm\release\svnrev.exe goto svnrev
devenv /build release %TERATERMSLN% /project svnrev /projectconfig release

:svnrev
..\teraterm\release\svnrev.exe ..\libs\svn\bin\svnversion.exe .. ..\teraterm\ttpdlg\svnversion.h

:build
devenv /%BUILD% release %TERATERMSLN%
if ERRORLEVEL 1 goto fail
devenv /%BUILD% release %TTSSHSLN%
if ERRORLEVEL 1 goto fail
devenv /%BUILD% release %TTPROXYSLN%
if ERRORLEVEL 1 goto fail
devenv /%BUILD% release %TTXKANJISLN%
if ERRORLEVEL 1 goto fail
devenv /%BUILD% release %TTPMENUSLN%
if ERRORLEVEL 1 goto fail
devenv /%BUILD% release %TTXSAMPLESLN%
if ERRORLEVEL 1 goto fail

rem cygterm ���R���p�C��
pushd ..\cygterm
if "%BUILD%" == "rebuild" make clean
make
popd

rem cygtool ���R���p�C��
pushd cygtool
nmake -f cygtool.mak
popd

exit /b 0

:fail
exit /b 1


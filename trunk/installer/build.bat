set BUILD=build
if "%1" == "rebuild" (set BUILD=rebuild)

rem ���C�u�������R���p�C��
pushd ..\libs
CALL buildall.bat
popd

if "%BUILD%" == "rebuild" goto build

rem "rebuild"���w�肵�Ȃ��ꍇ�ASVN���r�W�������X�V����B
if exist ..\teraterm\release\svnrev.exe goto svnrev
devenv /build release ..\teraterm\ttermpro.sln /project svnrev /projectconfig release

:svnrev
..\teraterm\release\svnrev.exe ..\libs\svn\bin\svnversion.exe .. ..\teraterm\ttpdlg\svnversion.h

:build
devenv /%BUILD% release ..\teraterm\ttermpro.sln
devenv /%BUILD% release ..\ttssh2\ttssh.sln
devenv /%BUILD% release ..\TTProxy\TTProxy.sln
devenv /%BUILD% release ..\TTXKanjiMenu\ttxkanjimenu.sln
devenv /%BUILD% release ..\ttpmenu\ttpmenu.sln
devenv /%BUILD% release ..\TTXSamples\TTXSamples.sln

rem cygterm ���R���p�C��
pushd ..\cygterm
if "%BUILD%" == "rebuild" make clean
make
popd

@echo off
setlocal
cd /d %~dp0

set opt=

:retry_cmake
set CMAKE=%~dp0cmake-3.11.4-win32-x86\bin\cmake.exe
echo 1. ������cmake.exe���g�p����(PATH��ʂ��Ă���)
echo 2. cmake���_�E�����[�h����(���̃t�H���_�ɃC���X�g�[������)
if exist %CMAKE% echo    �C���X�g�[���ς�(%CMAKE%)

set /p no="select no "
if "%no%" == "1" set CMAKE="cmake.exe" & goto finish_cmake

if exist %CMAKE% goto finish_cmake
echo @echo off > ..\cmake.bat
echo %CMAKE% %%1 %%2 %%3 %%4 %%5 %%6 %%7 %%8 %%9 >> ..\cmake.bat
call getcmake.bat

:finish_cmake

:retry_vs
echo 1. Visual Studio 15 2017
echo 2. Visual Studio 14 2015
echo 3. Visual Studio 12 2013
echo 4. Visual Studio 11 2012
echo 5. Visual Studio 10 2010
echo 6. Visual Studio 9 2008
echo 7. Visual Studio 8 2005
echo 9. Visual Studio NMake
echo z. Cygwin MinGW Release + Unix Makefiles
set /p no="select no "

echo %no%
if "%no%" == "1" set GENERATOR="Visual Studio 15 2017" & chcp 65001 & goto build_all
if "%no%" == "2" set GENERATOR="Visual Studio 14 2015" & goto build_all
if "%no%" == "3" set GENERATOR="Visual Studio 12 2013" & goto build_all
if "%no%" == "4" set GENERATOR="Visual Studio 11 2012" & goto build_all
if "%no%" == "5" set GENERATOR="Visual Studio 10 2010" & goto build_all
if "%no%" == "6" set GENERATOR="Visual Studio 9 2008" & goto build_all
if "%no%" == "7" set GENERATOR="Visual Studio 8 2005" & goto build_all
if "%no%" == "9" set GENERATOR="NMake Makefiles" & set OPT=-DCMAKE_BUILD_TYPE=Release & goto build_all
if "%no%" == "z" set GENERATOR="Unix Makefiles" & goto build_all
echo ? retry
goto retry_vs

:build_all
set C=%CMAKE% -DCMAKE_GENERATOR=%GENERATOR% %OPT% -P buildall.cmake
echo %C%
title %C%
pause
%C%

:finish
echo build complete
endlocal
pause

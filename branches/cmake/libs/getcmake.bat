@echo off
setlocal
cd /d %~dp0
echo cmake���_�E�����[�h����lib/cmake�ɓW�J���܂�
pause
powershell -NoProfile -ExecutionPolicy Unrestricted .\getcmake.ps1
endlocal
pause

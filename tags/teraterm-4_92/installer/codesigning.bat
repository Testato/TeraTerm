@echo off

rem
rem OpenSSL + signtool ���g���āA�I���I���R�[�h�T�C�j���O�ؖ�����t�^����B
rem

if "%1"=="/?" goto help
if "%1"=="" goto help

SET EXEFILE="%1"
SET OPENSSL=..\libs\openssl\out32\openssl.exe
SET SSLCONF=..\libs\openssl\apps\openssl.cnf
SET PASSWD="teraterm"
SET DAYS=365
SET PRIVKEY=cakey.pem
SET CERTKEY=cacert.pem
SET PFXKEY=certificate.pfx
SET TIMESERV=http://www.trustcenter.de/codesigning/timestamp
rem SET TIMESERV=http://timestamp.verisign.com/scripts/timstamp.dll

rem SSL�ؖ����Ɣ閧�����쐬����B

del /q %PRIVKEY% %CERTKEY% %PFXKEY%
echo �p�X�t���[�Y�� %PASSWD% �����Ă�������

%openssl% req -new -x509 -keyout %PRIVKEY% -out %CERTKEY% -days %DAYS% -config %SSLCONF%
%openssl% pkcs12 -export -out %PFXKEY% -inkey %PRIVKEY% -in %CERTKEY%
signtool sign /f %PFXKEY% /a /t %TIMESERV% /p %PASSWD% %EXEFILE%

exit /b

:help
echo OpenSSL + signtool ���g���āA�I���I���R�[�h�T�C�j���O�ؖ�����t�^����B
echo.
echo Usage:
echo   %0 Output\teraterm-4.72-RC1.exe
exit /b


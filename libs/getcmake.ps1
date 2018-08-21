$CMAKE_URL = "https://cmake.org/files/v3.11/cmake-3.11.4-win32-x86.zip"
$CMAKE_ZIP = ($CMAKE_URL -split "/")[-1]
$CMAKE_DIR = [System.IO.Path]::GetFileNameWithoutExtension($CMAKE_ZIP)

$CMAKE_ZIP = "download\cmake\" + $CMAKE_ZIP

echo $CMAKE_URL
echo $CMAKE_ZIP
echo $CMAKE_DIR

# TLS1.2 ��L���ɂ��� (cmake.org�� TLS1.2)
# [Net.ServicePointManager]::SecurityProtocol
# �����s���� "Ssl3, Tls" �Əo���ꍇ�ATLS1.2�͖����ƂȂ��Ă���
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12;

# �W�J�ς݃t�H���_������?
if((Test-Path $CMAKE_DIR) -eq $true) {
	# �폜����
	Remove-Item -Recurse -Force $CMAKE_DIR
	# �I������
	#exit
}

# �_�E�����[�h����
if((Test-Path $CMAKE_ZIP) -eq $false) {
	if((Test-Path "download\cmake") -ne $true) {
		mkdir "download\cmake"
	}
	wget $CMAKE_URL -Outfile $CMAKE_ZIP
}

# �W�J����
Expand-Archive $CMAKE_ZIP -DestinationPath .

==============================================================================
��SSH2�Ή�TTSSH             �y���p������z
==============================================================================

���͂��߂�
  �{�v���O������TTSSH(An SSH Extension to Teraterm)��SSH2�ɑΉ����������W���[���ł���A
  TeraTerm���SSH2(Secure Shell version 2)�𗘗p���邱�Ƃ��\�ƂȂ�܂��B


�����^�t�@�C��
  �A�[�J�C�u���𓀂���Ǝ��Ɏ����t�@�C��������܂��B

  ---------------------+-----------------------------------
  readme-j.txt            ���܂��ǂ݂̃e�L�X�g�t�@�C��
  ttxssh.dll              DLL�{��
  msvcr71.dll             MFC7.1�����^�C��
  TERATERM_sample.INI     INI�t�@�C���T���v��
  ---------------------+-----------------------------------


���C���X�g�[�����@
  TeraTerm���C���X�g�[������Ă���t�H���_��DLL�{�̂�MFC7.1�����^�C�����R�s�[���܂��B
  ���łɃI���W�i����TTSSH���i�[����Ă���ꍇ�́A���O�Ƀo�b�N�A�b�v������Ă����Ă���
  �㏑�����Ă��������B
  
  �����ӁF
    SSH2�Ή�TTSSH��UTF-8�Ή�TeraTerm Pro��ł������삵�܂���B
    UTF-8�Ή�TeraTerm Pro: http://hp.vector.co.jp/authors/VA013320/
  
  
��SSH�̗��p���@
  �ȉ��̎菇�ɂ��SSH(Secure Shell)���g�����ʐM���s�����Ƃ��ł��܂��B
  
  �P�DDLL�̃R�s�[
      UTF-8�Ή�TeraTerm Pro���C���X�g�[������Ă���t�H���_��"ttxssh.dll", "msvcr71.dll"��
      �R�s�[���܂��B
  
  �Q�DTeraTerm���j���[��"New connection"���Ăяo���A"Service"��'SSH'��
      �I������i�|�[�g�ԍ���22�j�B
      
  �R�D"SSH Authentication"�_�C�A���O���o�Ă���̂ŁA���[�U���ƃp�X���[�h��
      ���͂��āAOK�{�^������������B
      
  �S�D���[�U�F�؂ɐ�������΁A�T�[�o���̃v�����v�g���o�Ă���B


��SSH�������O�C��
  �ȉ��̃R�}���h���C���E�I�v�V�����̎g�p�ɂ��ASSH1�����SSH2�𗘗p���ă��[�U��
  �������O�C�����\�ƂȂ�܂��B
  
  �����j�p�X���[�h�����keyboard-interactive�F��
     ttermpro.exe �z�X�g��:22 /ssh [/1|/2] /auth=password /user=���[�U�� /passwd=�p�X���[�h
  �����j���J���F��
     ttermpro.exe �z�X�g��:22 /ssh [/1|/2] /auth=publickey /user=���[�U�� /passwd=�p�X���[�h /keyfile=�閧���t�@�C��

  ��jSSH1�ɂ��p�X���[�h�F�؃��O�C���i���[�U�� nike �p�X���[�h kukuri�j
     ttermpro.exe 192.168.1.3:22 /ssh /1 /auth=password /user=nike /passwd=kukuri

  ��jSSH2�ɂ��p�X���[�h�F�؃��O�C���i���[�U�� nike �p�X���[�h kukuri�j
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=password /user=nike /passwd=kukuri

  ��jSSH2�ɂ����J���F�؃��O�C���i���[�U�� kitakita �p�X���[�h oyaji 28 �閧���t�@�C�� d:\tmp\id_rsa�j
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=publickey /user=kitakita /passwd=oyaji@28 /keyfile=d:\tmp\id_rsa

  ���󔒂̈���
    �p�X���[�h����уp�X�ɋ󔒂��܂߂�ꍇ�A�X�y�[�X(' ')�̑���ɃA�b�g�}�[�N('@')��
    �g�p���Ă��������B�܂��A�A�b�g�}�[�N�Ƃ����������g�p����ꍇ�́A2������('@@')��
    �L�q���Ă��������B


���R�}���h���C���p�����[�^
  TTSSH�ł͈ȉ��̃R�}���h���C���p�����[�^���g�p�ł��܂��B
  
  �p�����[�^                ����
  ------------------------+-------------------------------------------------------
  /ssh-Lport1:host:port2    ���[�J���|�[�g(port1)���烊���[�g�|�[�g(port2)�փ|�[�g�t�H���[�f�B���O����
  /ssh-Rport1:host:port2    �����[�g�|�[�g(port1)���烍�[�J���|�[�g(port2)�փ|�[�g�t�H���[�f�B���O����
  /ssh-X                    X11���|�[�g�t�H���[�f�B���O����
  /ssh-f=file               teraterm.ini�t�@�C��(file)��ǂݍ���
  /ssh-v                    ���O���x���� VERBOSE ���[�h�ɂ���
  /ssh-autologin            ��̃p�X���[�h�ł̎������O�C�������݂�
  /ssh-autologon            ����
  /ssh-consume=file         teraterm.ini�t�@�C��(file)�̓ǂݍ��݌�A���Y�t�@�C�����폜����B
  /t=value                  TTSSH��L���ɂ���(value=2)�A�����ɂ���(value=2�ȊO)
  /f=file                   teraterm.ini�t�@�C��(file)��ǂݍ���
  /1                        SSH1�Őڑ�����
  /2                        SSH2�Őڑ�����
  /nossh                    TTSSH�𖳌��ɂ���
  /auth=mode                �������O�C�������݂�(mode=password, publickey)
  /user=username            ���[�U�����w�肷��
  /passwd=password          �p�X���[�h���w�肷��
  /keyfile=file             �閧�����w�肷��


  ��j
  ttermpro.exe server1:22 /ssh /ssh-L30025:smtpsrv1:25 /ssh-L30110:pop3srv1:110
  ttermpro.exe server2:22 /ssh /ssh-X
  ttermpro.exe server3:22 /ssh /2 /auth=password /user=guest /passwd=guest



��TTSSH�̃r���h���@
  TTSSH�̃r���h���@�ɂ��Ĉȉ��Ɏ����܂��B�r���h�ɂ�Visual Studio .NET 2003(VC++7.1)�AActivePerl���K�v�ł��B
  
  1. �\�[�X�R�[�h��SourceForge(http://sourceforge.jp/projects/ttssh2/)����`�F�b�N�A�E�g����
  2. zlib�̃\�[�X�R�[�h(http://www.zlib.net/)�� ttssh2\zlib �f�B���N�g���ɓW�J����
  3. OpenSSL�̃\�[�X�R�[�h(http://www.openssl.org/)�� ttssh2\openssl �f�B���N�g���ɓW�J����BOpenSSL���r���h����i�ȉ��Q�Ɓj�B

      - Build OpenSSL.
            + cd openssl
            + perl Configure VC-WIN32
                (Yes, you need perl to build OpenSSL!)
            + ms\do_ms
            + nmake -f ms\nt.mak
            + cd ..
                (Now you are back in PortForwarder folder.)
        See the instruction in the OpenSSL documentation for details.

  4. ttssh2\ttssh.sln ��Visual Studio�ŊJ��
  5. �\�����[�V�������r���h����
  6. �r���h�ɐ��������ttssh2\ttxssh �f�B���N�g����DLL�����������



���J����
  OS: Windows XP Professional
  �R���p�C��: Visual Studio .NET 2003
  
  �\�t�g�E�F�A�F
  TeraTerm Pro 2.3
  IPv6 0.81
  TTSSH 1.5.4
  

�����C�Z���X
Copyright (c) 2004-2005 Yutaka Hirata, TeraTerm Project. 
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice, 
     this list of conditions and the following disclaimer. 
  2. Redistributions in binary form must reproduce the above copyright notice, 
     this list of conditions and the following disclaimer in the documentation 
     and/or other materials provided with the distribution. 
  3. The name of the author may not be used to endorse or promote products derived 
     from this software without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY 
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.


�����쌠
  �{�v���O�����̓t���[�\�t�g�E�F�A�ł��BBSD���C�Z���X�̂��Ƃɔz�z����Ă��܂��B
  TeraTerm Pro�͎��������̒��앨�ł��B
  IPv6��TeraTerm Pro�͉����~�玁�̒��앨�ł��B
  TTSSH��Robert O'Callahan���̒��앨�ł��B
  TTSSH��SSH2���̃R�[�h�ɂ��ẮA���쌠�͕��c�L�ɂ���܂��B
  OpenSSL��OpenSSL Project�̒��앨�ł��B
  zlib��Greg Roelofs��Jean-loup Gailly���̒��앨�ł��B
  �{�v���O�����͖����ŗ��p�ł��܂����A��҂͖{�v���O�����̎g�p�ɂ�����
  �������Q����ɑ΂��Ĉ�؂̐ӔC�𕉂��܂���B

  e-mail : yutakakn@mail.goo.ne.jp
  URL    : http://hp.vector.co.jp/authors/VA013320/
  CVS    : http://sourceforge.jp/projects/ttssh2/
  TeraTerm Pro: http://hp.vector.co.jp/authors/VA002416/
  IPv6��TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  TTSSH original:http://www.zip.com.au/~roca/ttssh.html


�����ŗ���

2005.10.21 (Ver 2.18)
  �E�ڑ��_�C�A���O�� History �`�F�b�N�{�b�N�X��ǉ������B
  �EOpenSSL 0.9.8a�֍����ւ���

2005.9.5 (Ver 2.17)
  �ESSH�������O�C�����ɔF�؃_�C�A���O�� /I �I�v�V�����g�p���ɂ̂ݍŏ�������悤�ɂ����B

2005.8.27 (Ver 2.16)
  �ESSH�������O�C�����ɔF�؃_�C�A���O���ŏ�������悤�ɂ����B�����q�Y���Ɋ��ӂ��܂��B
  �Ezlib 1.2.3�֍����ւ���

2005.7.24 (Ver 2.15)
  �ESSH2�|�[�g�t�H���[�f�B���O���T�|�[�g�����B
  �ESSH2�p�P�b�g���k���T�|�[�g�����B
  �ESSH2�ڑ�����TTY���(VERASE)��n���悤�ɂ����B
  �ESSH1�ڑ��ň�x���[�U�F�؂Ɏ��s����ƁA���̌�F�؂��ł��Ȃ��Ȃ�o�O���C���B
  �E�ڑ��_�C�A���O��� Alt+P �� Alt+C �Ȃǂ̃A�N�Z�����[�^�L�[�������Ȃ��o�O���C���B
  �EOpenSSL 0.9.8�֍����ւ���
  �Ezlib 1.2.2��security fix(CAN-2005-2096)��ǉ��B

2005.5.28 (Ver 2.14)
  �E�閧���t�@�C���_�C�A���O��3�t�@�C���t�B���^��ǉ������B
  �Ezlib�̃o�[�W�������o�[�W�������ɒǉ������B

2005.4.24 (Ver 2.13)
  �E�L�[�쐬�_�C�A���O��ǉ������iSetup - SSH KeyGenerator�j�B
  �EOpenSSL 0.9.7g�֍����ւ���

2005.4.9 (Ver 2.12)
  �ESSH2�`���l�����b�N�A�b�v�@�\��ǉ�
  �E�L�[�č쐬���ɃA�v���P�[�V�����G���[�ƂȂ�o�O���C���i2.10�ł̃f�O���[�h�j�B
  �E"Duplicate session"�ɂ�����SSH�������O�C�����s���悤�ɂ����B


2005.3.31 (Ver 2.11)
  �ESSH2�F�؃_�C�A���O�� Use challenge/response to log in �ɃA�N�Z�����[�^�L�[�����蓖�Ă��B
  �E�V���A���|�[�g���J������Ԃ���Alt-N�ŐV�K�ڑ����J�����Ƃ����Ƃ��A�t�H�[�J�X�𓖂Ă�悤�ɂ����B
  �ESSH���O�t�@�C��(TTSSH.LOG)�̍̎�f�[�^�𑝉�����
  �ENetScreen(HITACHI) workaround�Ή��BSSH2_MSG_CHANNEL_REQUEST���M���ɂ����āAwantconfirm��false�ɂ����i�T�[�o����̃��v���C�����҂��Ȃ��j�B
  �EOpenSSL 0.9.7f�֍����ւ���


2005.3.13 (Ver 2.10)
   �Ettxssh.dll�̃o�[�W������񂪌Â��܂܂������̂ŏC�������B
   �ESSH client ID(SSH-2.0-TTSSH/X.XX Win32)�Ƀo�[�W���������܂߂�悤�ɂ����B
   �ETru64 UNIX workaround��ǉ��BMiguel���Ɋ��ӂ��܂��B
   �ESSH2 log dump�@�\��ǉ�����
   �ESSH server ID��CR+LF���܂܂�Ă����ꍇ�ACR�̏������ł��Ă��Ȃ������o�O���C���B
   �E���łɃ��O�C���������s���Ă���ꍇ�́ASSH2_MSG_SERVICE_REQUEST�̑��M�͍s��Ȃ��悤�ɂ����B
   �E�p�X���[�h�F�؂̑O�ɍs��keyboard-interactive���\�b�h�ŁA�f�t�H���g�ݒ�l([TTSSH] KeyboardInteractive)�𖳌�(0)�ɂ����BOpenSSH 4.0�ł͖����ȃ��\�b�h���g�p����ƃR�l�N�V�������؂��Ă��܂����߁B�܂��A�F�؃_�C�A���O�̃��x������ݒ�̗L���ɂ��ύX����悤�ɂ����B
   �ESSH2�ikeyboard-interactive�F�؁j�ڑ��ɂ�����challenge/response login�ɑΉ������B

2005.2.22 (Ver 1.09)
  �E�h�b�g�Ŏn�܂�f�B���N�g���ɂ���SSH2�閧���t�@�C�����ǂݍ��߂Ȃ����֑Ώ������B
  �EHeartBeat(keep-alive), SSH2 keyboard-interactive�̐ݒ��TTSSH�̃_�C�A���O�ォ��s����悤�ɂ����B

2005.1.30 (Ver 1.08)
  �Ekeyboard-interactive�F�؂��T�|�[�g�����B����ɂƂ��Ȃ��Ateraterm.ini �t�@�C����TTSSH�Z�N�V������"KeyboardInteractive"�G���g����ǉ������B
  �E�o�[�W�����_�C�A���O�Ƀz�[���y�[�W��URL�EOpenSSL�o�[�W������ǉ������B
  �ESSH2�F�؃_�C�A���O�ŁArhosts�����TIS���O���[�ɂȂ�O�ɁAEnter�L�[����������ƁA�A�v���P�[�V�����G���[�ƂȂ錻�ۂɑΏ������B
  �E���J���F�؎������O�C�����T�|�[�g�B/auth=publickey, /keyfile�R�}���h���C���I�v�V������ǉ��B

2005.1.6 (Ver 1.07)
  �Etelnet�ڑ����Ƀ^�[�~�i���T�C�Y�ύX���s���ƁA�A�v���P�[�V�����G���[�ƂȂ�o�O���C���i1.06�ł̃f�O���[�h�j�B

2005.1.5 (Ver 1.06)
  �E�^�[�~�i���T�C�Y�ύX�ɑΉ������B
  �E�L�[�č쐬���ɃA�v���P�[�V�����G���[�ƂȂ�o�O���C���i1.05�ł̃f�O���[�h�j�B

2004.12.27 (Ver 1.05)
  �E'Auto window close'�������̏ꍇ�A�ؒf��̐ڑ����ł��Ȃ������C�������B
  �ESSH2 KEX�����̃��������[�N���C�������B
  �ESSH2�閧���ǂݍ��ݎ��s���̃G���[���b�Z�[�W�o�͂����������B

2004.12.23 (Ver 1.04)
  �E���J���F�؂��T�|�[�g

2004.12.18 (Ver 1.03)
  �E�p�P�b�g��M���ɂ�����HMAC�`�F�b�N��ǉ�
  �E���b�Z�[�W�F�؃A���S���Y���� HMAC-MD5 ��ǉ�
  �EKEX�ɂ�����RSA�����DSS��key verify������ǉ�

2004.12.16 (Ver 1.02)
   �ESSH�������O�C���ɂ����āA�܂�ɃA�v���P�[�V�����G���[�ƂȂ錻�ۂ֑Ώ������B
   �ESSH1�ڑ�����"SECURITY WARNING"�_�C�A���O�ŁA�L�����Z������ƃA�v���P�[�V�����G���[�ƂȂ錻�ۂ֑Ώ������B

2004.12.11 (Ver 1.01)
  �ESSH�n�[�g�r�[�g�i�L�[�v�A���C�u�j�@�\��ǉ������B�n�[�g�r�[�g�Ԋu�́A teraterm.ini �t�@�C����TTSSH�Z�N�V������"HeartBeat"�G���g���Őݒ�\�i�f�t�H���g��60�b�j�B

2004.12.8 (Ver 1.00)
  �ESSH1/SSH2�������O�C�����T�|�[�g�ipassword�F�؂̂݁j�B/auth, /username, /passwd�I�v�V������ǉ��B
  �ESSH�̃f�t�H���g�o�[�W������ SSH2 �ɕύX����
  �E�ڑ��_�C�A���O��tab order���C�������B
  �E�ڑ��_�C�A���O�̋N�����A�z�X�g���Ƀt�H�[�J�X��������悤�ɂ����B
  �ESSH2�������O�C���E�T���v���}�N��(ssh2login.ttl)�𓯍�����

2004.11.7 (Ver 1.00 beta1)
  �E�Í��A���S���Y�� AES128 ���T�|�[�g����
  �EKEX�A���S���Y�� diffie-hellman-group14-sha1 (OpenSSH 3.9�ȍ~)���T�|�[�g����
  �EKEX�A���S���Y�� diffie-hellman-group-exchange-sha1 ���T�|�[�g����
  �E�z�X�g�L�[�A���S���Y�� ssh-dss ���T�|�[�g����
  �E�o�[�W�������_�C�A���O��SSH2�����o�͂���悤�ɂ���
  �EOpenSSL 0.9.7e�֍����ւ���
  �EREADME��Boris Maisuradze���ɉp�������Ă��������܂����B���ӂ��܂��B

2004.10.25 (Ver 1.00 alpha4)
  �E�E�B���h�E�T�C�Y����̕s����C�������B
  �E�f�[�^�ʐM�m�����DH���č쐬�ɑΉ������B
  �E���̑��ׂ����o�O�t�B�b�N�X

2004.10.17 (Ver 1.00 alpha3)
  �E�E�B���h�E�T�C�Y���䏈����ǉ�����
  �E�R�l�N�V������ؒf����ہA�T�[�o���� SSH2_MSG_CHANNEL_CLOSE �𑗂�悤�ɂ����B
  �Eterminal type�����ϐ�TERM�֓n�����悤�ɂ���

2004.10.11 (Ver 1.00 alpha2)
  �ESSH1���T�|�[�g����
  �ESSH session���ؒf�����o�O�i"Oversized packet received from server"�ƕ\������āA
    �R�l�N�V�������؂����j���C������
  �E/1, /2 �I�v�V�����̒ǉ�
  �ETERATERM.INI �t�@�C����SSH protocol version�̃G���g����ǉ�����

2004.8.8 (Ver 1.00 alpha1a)
  �EDLL��release build�ł֍����ւ���

2004.8.5 (Ver 1.00 alpha1)
  �E�����J

�ȏ�

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
  �������O�C�����\�ƂȂ�܂��B�������A�p�X���[�h�ɋ󔒂��܂߂邱�Ƃ͂ł��܂���B
  �܂��Apassword�F�؂݂̂̃T�|�[�g�ƂȂ��Ă��܂��B
  
  �����j
     ttermpro.exe �z�X�g��:22 /ssh [/1|/2] /auth=password /user=���[�U�� /passwd=�p�X���[�h

  ��jSSH1
     ttermpro.exe 192.168.1.3:22 /ssh /1 /auth=password /user=nike /passwd=kukuri

  ��jSSH2
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=password /user=nike /passwd=kukuri


���J����
  OS: Windows XP Professional
  �R���p�C��: Visual Studio .NET 2003
  
  �\�t�g�E�F�A�F
  TeraTerm Pro 2.3
  IPv6 0.81
  TTSSH 1.5.4
  

�����쌠
  �{�v���O�����̓t���[�\�t�g�E�F�A�ł��B
  TeraTerm Pro�͎��������̒��앨�ł��B
  IPv6��TeraTerm Pro�͉����~�玁�̒��앨�ł��B
  TTSSH��Robert O'Callahan���̒��앨�ł��B
  TTSSH��SSH2���̃R�[�h�ɂ��ẮA���쌠�͕��c�L�ɂ���܂��B
  �{�v���O�����͖����ŗ��p�ł��܂����A��҂͖{�v���O�����̎g�p�ɂ�����
  �������Q����ɑ΂��Ĉ�؂̐ӔC�𕉂��܂���B

  e-mail : yutakakn@mail.goo.ne.jp
  URL    : http://hp.vector.co.jp/authors/VA013320/
  CVS    : http://sourceforge.jp/projects/ttssh2/
  TeraTerm Pro: http://hp.vector.co.jp/authors/VA002416/
  IPv6��TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  TTSSH original:http://www.zip.com.au/~roca/ttssh.html


����������
  �E�p�P�b�g�̈��k�̓T�|�[�g���Ă��܂���
  �E�|�[�g�t�H���[�f�B���O�̓T�|�[�g���Ă��܂���


�����ŗ���

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

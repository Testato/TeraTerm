==============================================================================
��UTF-8�Ή�TeraTerm Pro             �y���p������z
==============================================================================

���͂��߂�
  �{�v���O�����̓^�[�~�i���\�t�g�uTeraTerm Pro�v��UTF-8�Ή����������̂ł��B


�����^�t�@�C��
  �A�[�J�C�u���𓀂���Ǝ��Ɏ����t�@�C��������܂��B

  ---------------------+-----------------------------------
  readme.txt              ���܂��ǂ݂̃e�L�X�g�t�@�C��
  cygterm.exe             Cygterm�{��
  cygterm.cfg             Cygterm�p�ݒ�t�@�C��
  TERATERM_sample.INI     ini�t�@�C���T���v��
  ttp*.dll                TeraTerm DLL
  tt*.exe                 TeraTerm ���s�t�@�C��
  keycode.exe             keycode
  ttxssh.dll              SSH2�Ή�TTSSH
  msvcr71.dll             MFC7.1�����^�C��
  ---------------------+-----------------------------------


���C���X�g�[�����@
  �{�v���O�����Ɋ܂܂��o�C�i���t�@�C�����ATeraTerm Pro2.3�I���W�i�����C���X�g�[��
  ����Ă���f�B���N�g���֏㏑���R�s�[���Ă��������B
  �܂��Acygterm.exe�����cygterm.cfg�𓯈�̃f�B���N�g���փR�s�[���Ă����܂��B
  
  �����ӁF���샂�[�h
    UTF-8�@�\�͓��{�ꃂ�[�h�ł̂ݗL���ƂȂ�܂��B�I���W�i��TeraTerm Pro�͓��{�ꃂ�[�h��
    �C���X�g�[�����Ă����Ă��������B
  
  
��Unicode�ݒ�
  UTF-8�����s���ɂ́ATeraTerm Pro�́uSetup�v���j���[����uTerminal�v��I������ƁA
  �_�C�A���O���o�Ă���̂ŁA�uKanji(receive)�v����сuKanji(transmit)�v�ɂāA
  �yUTF-8�z��I��ł��������B
  TeraTerm Pro�̍ċN���͕K�v����܂���B
  �R�}���h���C���́u/KT�v����сu/KR�v�I�v�V�����ɂ����āA"UTF8"���w�肷��ƁA
  ���M����ю�M�R�[�h��UTF-8��ݒ肷�邱�Ƃ��ł��܂��B
  
  ���{��ȊO�̌����\���ł���悤�ɂ��邽�߂ɂ́Ateraterm.ini�t�@�C���Ƀ��P�[��
  ����уR�[�h�y�[�W�̐ݒ肪�K�v�ƂȂ�܂��B���L�ɃT���v���������܂��B
  ���P�[������уR�[�h�y�[�W�ɐݒ�ł�����e�ɂ��ẮA���L�̃T�C�g���Q�Ƃ��Ă��������B
  
    http://www.microsoft.com/japan/msdn/library/default.asp?url=/japan/msdn/library/ja/vclib/html/_crt_language_strings.asp
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_81rn.asp
    
------------------------------------------------------------------------------------
; Locale for Unicode
Locale = japanese

; CodePage for Unicode
CodePage = 932
------------------------------------------------------------------------------------

  �����ӁFMac OS X
    �yUTF-8m�z��Mac OS X(HFS+)�����̃G���R�[�f�B���O�w��ł��B��M�݂̂̃T�|�[�g�ł��B
    �u/KR�v�R�}���h���C���ł�"UTF8m"���w�肷�邱�Ƃ��ł��܂��B


��ANSI�J���[�F�ύX���s�A���p�b�`
  �{�v���O�����́uANSI�J���[�F�ύX���s�A���p�b�`�v����荞��ł��܂��B
  teraterm.ini�t�@�C����[Tera Term]�Z�N�V�����ɁA���L�̋L�q��ǉ����邱�Ƃɂ��A
  ���Y�@�\��L���ɂ��邱�Ƃ��ł��܂��B

------------------------------------------------------------------------------------
;	ANSI color definition (in the case FullColor=on)
;	* UseTextColor should be off, or the background and foreground color of
;	  VTColor are assigned to color-number 0 and 7 respectively, even if
;	  they are specified in ANSIColor.
;	* ANSIColor is a set of 4 values that are color-number(0--15),
;	  red-value(0--255), green-value(0--255) and blue-value(0--255).
ANSIColor=0,0,0,0, 1,255,0,0, 2,0,255,0, 3,255,255,0, 4,128,128,255, 5,255,0,255, 6,0,255,255, 7,255,255,255, 8,0,0,0, 9,255,0,0, 10,0,255,0, 11,255,255,0, 12,128,128,255, 13,255,0,255, 14,0,255,255, 15,255,255,255

;	Enable continued-line copy
EnableContinuedLineCopy=on
------------------------------------------------------------------------------------

  ���q���g
    TermType��"xterm"�ɐݒ肷�邱�Ƃɂ��AANSI�J���[�ɑΉ����Ă���vim6�Ȃǂ�
    �J���[�\���������邱�Ƃ��ł��܂��B����̓I���W�i�����炠��@�\�ł��B


���z�C�[���}�E�X
  �I���W�i����TeraTerm�ł̓z�C�[���}�E�X�������܂��񂪁A�{�v���O�����ł̓z�C�[���{�^����
  �g���ăX�N���[���o�[���㉺�����邱�Ƃ��ł��܂��B


���}�E�X�J�[�\��
  �}�E�X�J�[�\���̎�ʂ�ݒ肷�邱�Ƃ��ł��܂��B
  teraterm.ini�t�@�C����[Tera Term]�Z�N�V�����ɁA���L�̋L�q��ǉ����邱�Ƃɂ��A
  ���Y�@�\��L���ɂ��邱�Ƃ��ł��܂��B

------------------------------------------------------------------------------------
; Mouse cursor type (arrow/ibeam/cross/hand)
MouseCursor = hand
------------------------------------------------------------------------------------

    arrow = �W���̖��J�[�\��
    ibeam = �A�C�r�[���i�c���j�J�[�\���i�f�t�H���g�j
    cross = �\���J�[�\��
    hand  = �n���h�J�[�\��


���������E�B���h�E
  �������E�B���h�E�̐ݒ�����邱�Ƃ��ł��܂��B�f�t�H���g��255�i�s�����j�ł��B
  teraterm.ini�t�@�C����[Tera Term]�Z�N�V�����ɁA���L�̋L�q��ǉ����邱�Ƃɂ��A
  ���Y�@�\��L���ɂ��邱�Ƃ��ł��܂��iWindows2000�ȍ~�j�B

------------------------------------------------------------------------------------
; Translucent window (0 - 255)
AlphaBlend = 230
------------------------------------------------------------------------------------


��Cygwin�ڑ�
  ���[�J����Cygwin�֐ڑ����邱�Ƃ��ł��܂��B������Cygterm�𗘗p���Ă��܂��B
  File���j���[����uCygwin connection�v��I������A��������Alt+G����������ƁA
  Cygwin�֐ڑ����܂��B
  ���炩���߁Ateraterm.ini�t�@�C����[Tera Term]�Z�N�V�����ɁACygwin��
  �C���X�g�[���p�X��ݒ肵�Ă����K�v������܂��B
  �t�H�[�}�b�g�́u�h���C�u���F���f�B���N�g���v�ł��B

------------------------------------------------------------------------------------
; Cygwin install path
CygwinDirectory = c:\cygwin
------------------------------------------------------------------------------------

  �����ӁF
    cygterm�����܂��N�����Ȃ��ꍇ�́Acygterm���r���h�������Ă��������B



���o�b�t�@�̑S�I��
  Edit���j���[����uSelect all�v��I�����邱�Ƃɂ��A�o�b�t�@�̑S�I����
  �s�����Ƃ��ł��܂��B

  ���q���g
    SHIFT�L�[�������Ȃ���}�E�X���h���b�O����Ɓu�u���b�N�I���v���ł���悤��
    �Ȃ�܂��B����̓I���W�i�����炠��@�\�ł��B


�����O�t�@�C���ւ̃R�����g�ǉ�
  File���j���[����"Comment to Log..."��I������ƁA���O�t�@�C���փR�����g��ǉ�
  ���邱�Ƃ��ł��܂��B�ǉ��ł���R�����g�͍ő�255�����܂łł��B



���ݒ�_�C�A���O
  Edit���j���[����"External setup"��I������ƁAUTF-8���ȍ~�ɒǉ������@�\�Ɋւ���
  �I�v�V�����ݒ肪�s���܂��B


�����C���o�b�t�@
  ���C���o�b�t�@�� 10000 �s�𒴂���ݒ���������ꍇ�Ateraterm.ini �t�@�C����
  MaxBuffSize �G���g�������킹�Đݒ������K�v������܂��B�ȉ��ɗ�������܂��B

------------------------------------------------------------------------------------
; Max scroll buffer size
MaxBuffSize=500000
------------------------------------------------------------------------------------


��SSH�̗��p
  �{�v���O�����ł� SSH2�Ή�TTSSH �𗘗p���邱�Ƃɂ��ASSH(Secure Shell)�𗘗p���邱�Ƃ�
  �ł��܂��BTTSSH�I���W�i������{���TTSSH�Ƃ̑g�ݍ��킹�ł͓��삵�܂���̂ŁA�����ӂ���
  �����B
  
  SSH�Ɋւ���R�}���h���C���I�v�V�����͈ȉ��̂��̂�����܂��B
  
    �I�v�V����    �Ӗ�
    /1            SSH1�Őڑ�����
    /2            SSH2�Őڑ�����i�T�[�o��SSH2���T�|�[�g���Ă��Ȃ��ꍇ��SSH1�ڑ��ɂȂ�j�B

  �ȉ��ɃR�}���h���C���I�v�V�����̗�������܂��iIPv4 & SSH2 & UTF-8�Őڑ��j�B

    ttermpro.exe /KR=utf8 /4 192.168.1.3:22 /ssh /2


���}�N�����t�@�����X: connect
  TeraTerm�}�N������ڑ�������@�ɂ́A3�̎�ނ�����܂��B
  
  - telnet�ڑ� (port 23) 
  - SSH1 ����� SSH2 �ڑ� (port 22) 
  - COM port �ڑ�
  
  �Etelnet�ڑ�
  
  connect 'myserver:23 /nossh' 
  or 
  connect 'myserver:23' 
  
  /nossh ���g�����������������܂��B���̃I�v�V�������Ȃ��ꍇ�ATeraTerm�� teraterm.ini ���Ō�ɕۑ����ꂽ�Ƃ��Ɠ������\�b�h�itelnet �������� SSH�j���g���āA�ڑ����悤�Ƃ��܂��B�����ASSH���g���Đڑ����悤�Ƃ���Ȃ�΁Aconnect�}�N���͎��s����ł��傤�B
  
  
  �ESSH�ڑ�
  
  connect 'myserver /ssh' 
  or 
  connect 'myserver /ssh /1' 
  or 
  connect 'myserver /ssh /2' 
  or 
  connect 'myserver /ssh /auth=password /user=username /passwd=password'
  or 
  connect 'myserver /ssh /1 /auth=password /user=username /passwd=password'
  or 
  connect 'myserver /ssh /2 /auth=password /user=username /passwd=password'
  or 
  connect 'myserver /ssh /auth=publickey /user=username /passwd=password /keyfile=private-key-file' 
  or 
  connect 'myserver /ssh /1 /auth=publickey /user=username /passwd=password /keyfile=private-key-file' 
  or 
  connect 'myserver /ssh /2 /auth=publickey /user=username /passwd=password /keyfile=private-key-file' 
  
  �ŏ��̕��@�́ASSH�o�[�W�������w�肹����SSH�ڑ������܂��B/1 �� /2 �̃I�v�V�����́ASSH1�����SSH2�ł��邱�Ƃ��w�肵�܂��B�Ō��6�̕��@�́A�}�N�����烆�[�U���ƃp�X���[�h��n�����Ƃɂ��ASSH�F�؃_�C�A���O���X�L�b�v�����邱�Ƃ��ł��܂��B/auth=password�̓p�X���[�h�F�؁A/auth=publickey�͌��J���F�؂ł��邱�Ƃ�\���܂��BTeraTerm�}�N���Ƀ��[�U���ƃp�X���[�h���w�肵���ꍇ�A���̃}�N���t�@�C�����v���[���e�L�X�g�Ƃ��ĕۑ������̂Ȃ�A�Z�L�����e�B�I�Ɉ��S�ȏꏊ�ɒu���K�v�����邱�Ƃ��̂ɖ����Ă����Ă��������B
  
  
  �ECOM�|�[�g�ڑ�
  
  connect '/C=x' 
  
  x��COM�|�[�g�ԍ���\���܂��B���Ƃ��΁ACOM1�ɐڑ��������̂Ȃ�A connect '/C=1' �Ƃ��܂��B
  
  source URL: http://www.neocom.ca/forum/viewtopic.php?t=6


���J����
  OS: Windows XP Professional
  �R���p�C��: Visual Studio .NET 2003
  
  �\�t�g�E�F�A�F
  TeraTerm Pro 2.3
  IPv6 0.81
  Cygterm 1.06
  TTSSH 1.5.4
  

�����쌠
  �{�v���O�����̓t���[�\�t�g�E�F�A�ł��B
  TeraTerm Pro�͎��������̒��앨�ł��B
  ANSI�J���[�F�ύX���s�A���p�b�`�͑���ޗR�����ɒ��쌠������܂��B
  UTF-8���̃R�[�h�ɂ��ẮA���쌠�͕��c�L�ɂ���܂��B
  IPv6��TeraTerm Pro�͉����~�玁�̒��앨�ł��B
  Cygterm��NSym���̒��앨�ł��B
  TTSSH��Robert O'Callahan���̒��앨�ł��B
  TeraTerm Menu�͑���M�����̒��앨�ł��B
  LogMeIn��NeoCom Solutions�̒��앨�ł��B
  �{�v���O�����͖����ŗ��p�ł��܂����A��҂͖{�v���O�����̎g�p�ɂ�����
  �������Q����ɑ΂��Ĉ�؂̐ӔC�𕉂��܂���B

  e-mail : yutakakn@mail.goo.ne.jp
  URL    : http://hp.vector.co.jp/authors/VA013320/
  CVS    : http://sourceforge.jp/projects/ttssh2/
  TeraTerm Pro: http://hp.vector.co.jp/authors/VA002416/
  ANSI�J���[�F�ύX���s�A���p�b�`: http://www.sakurachan.org/ml/teraterm/msg00264.html
  IPv6��TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  Cygterm: http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index.html
  TeraTerm Menu: http://park11.wakwak.com/~shinpei/freesoft/index.html
  LogMeIn: http://www.neocom.ca/freeware/LogMeIn/
  TeraTerm forum: http://www.neocom.ca/forum/index.php
  TeraTerm ML: http://www.freeml.com/info/teraterm@freeml.com


�����ӎ���
  �EUTF-8����Shift_JIS�֕ϊ��ł��Ȃ������͋^�╄�\���ƂȂ�܂��B
  �EUTF-8���ł�EUC�o�͂���ʂɔ��f����Ȃ����Ƃ�����܂��B


����������
  �EUTF-8�Ή��͌��ݓ��{��݂̂ł��B
  �ESSH�ɑ΂���"Duplicate session"�ł͏�� SSH2 �ł̐ڑ������݂܂��B


�����ŗ���

2005.1.30 (Ver 4.10)
  �E�ő剻�{�^����L���ɂ���
  �ETeraTerm Menu, LogMeIn�̃A�N�Z�����[�^�L�[���폜����
  �E���O�̎掞�̃f�t�H���g��(log_YYYYMMDD_HHMMSS.txt)��ݒ肷��悤�ɂ���
  �EBroadcast command���j���[��Control menu�z���ɒǉ������B
  �E�o�[�W�����_�C�A���O�Ƀz�[���y�[�W��URL��ǉ������B
  �E"Save setup"�_�C�A���O�̏����t�@�C���f�B���N�g�����A�ǂݍ��܂ꂽteraterm.ini������ӏ��֌Œ肷��悤�ύX�����B
  �E"File"�z����"View Log"���j���[��ǉ������B
  �E"Additional settings"��"View Log Editor"�{�b�N�X��ǉ������B
  �Eteraterm.ini��[Tera Term]�Z�N�V�����ɁA"ViewlogEditor"�G���g����ǉ������B
  �ETeraTerm Menu 1.02�֍����ւ��B
     - �f�t�H���g�I�v�V�����Ɋ����R�[�h(/KT=UTF8 /KR=UTF8)��ǉ������B
     - SSH�������O�C���ɂ����āA���[�U�p�����[�^���w��ł���悤�ɂ����B
     - SSH�������O�C���ɂ����āA�}�N���̎w����폜�����B
     - SSH�������O�C���ɂ����āA�閧���t�@�C���̎w����ł���悤�ɂ����B

2005.1.10 (Ver 2.09)
  �E�}���`�f�B�X�v���C���ɂ����āA�E�B���h�E�̃��T�C�Y���s���ƁA�v���C�}���f�B�X�v���C�֖߂��Ă��܂����ۂɑΏ��B�p�b�`�쐬�Ɋ��ӂ��܂����������F��

2005.1.6 (Ver 2.08)
  �E���O�̎�̃_�C�A���O���I�[�v���_�C�A���O����Z�[�u�_�C�A���O�֕ύX����
  �E"Save setup"�_�C�A���O�̏����t�@�C���f�B���N�g�����v���O�����{�̂�����ӏ��֌Œ肵��
  �ESSH2�Ή�TTSSH(1.07)�֍����ւ�

2005.1.5 (Ver 2.07h)
  �ESSH2�Ή�TTSSH(1.06)�֍����ւ�

2004.12.27 (Ver 2.07g)
  �ESSH2�Ή�TTSSH(1.05)�֍����ւ�

2004.12.23 (Ver 2.07f)
  �ESSH2�Ή�TTSSH(1.04)�֍����ւ�

2004.12.18 (Ver 2.07e)
  �ESSH2�Ή�TTSSH(1.03)�֍����ւ�

2004.12.16 (Ver 2.07d)
  �ESSH2�Ή�TTSSH(1.02)�֍����ւ�

2004.12.15 (Ver 2.07c)
  �E�t�H���g�̃t�@�C�������̌�����
  �E�C���X�g�[������TeraTerm�֘A�̃v���Z�X���E���悤�ɂ���

2004.12.14 (Ver 2.07b)
  �Eteraterm.ini�̃X�N���[���o�b�t�@�̏����l�� 20000 �֕ύX�B
  �E�t�H���g�̃C���X�g�[�����ɁA�t�@�C�������� ReadOnly �ɂ���悤�ɂ����B
  �ETeraTerm Menu 1.01�֍����ւ��Bttermpro.exe�̏����t�H���_�p�X���J�����g�f�B���N�g���ɕύX�B

2004.12.11 (Ver 2.07a)
  �ESSH2�Ή�TTSSH(1.01)�֍����ւ�

2004.12.8 (Ver 2.07)
  �E"External setup"��setup menu�z����"Additional settings"�Ƃ��Ĉړ�
  �EFile menu��"Duplicate session"��ǉ��i/DUPLICATE�I�v�V������ǉ��j
  �Ettmacro.exe�̃v���Z�X�D��x�́A���O�̎撆�݂̂ɉ�����悤�ɂ����B
  �E�s���A�����Ă���ꍇ�́A���O�t�@�C���ɉ��s�R�[�h���܂߂Ȃ��悤�ɂ����iEnableContinuedLineCopy�@�\���L���̏ꍇ�̂݁j�B
  �E�X�N���[���o�b�t�@�̍ő僉�C������400000�܂łɊg�������B
  �ELogMeIn 1.21�𓯍������B���^����Boris����肢�������܂����B���ӂ��܂��B
  �ETeraTerm Menu 0.94r2(+SSH2 support 1.00)�𓯍������B���^���𑁐�l��肢�������܂����B���ӂ��܂��B
  �ESSH2�Ή�TTSSH(1.00)�֍����ւ�
  �E���̑��ׂ����o�O�C��

2004.11.6 (Ver 2.06)
  �EDEC����t�H���g�ɂ��r���`�掞�ɃS�~���c������C�������B����ɂ��AUTF-8���ł͓��{��ȊO�̌���ɂ͖��Ή��ƂȂ�B�܂��ABOLD style���g�p����ƁA������̉E�[���؂�Ȃ��Ȃ�B
  �E2�߈ȍ~��TeraTerm�̋N�����ɂ��f�B�X�N���� teraterm.ini ��ǂނ悤�ɂ����B
  �E�G�N�X�v���[������̃f�B���N�g���̃h���b�O���h���b�v�ɑΉ������B
  �EREADME��Boris Maisuradze���ɉp�������Ă��������܂����B���ӂ��܂��B

2004.10.25 (Ver 2.05a)
  �ESSH2�Ή�TTSSH(1.00alpha4)�֍����ւ�

2004.10.11 (Ver 2.05)
  �ESSH2�Ή�TTSSH(1.00alpha2)�𓯍�����
  �EMFC7.1 runtime�𓯍�����
  �E���ϐ� TERATERM_EXTENSIONS �̐ݒ�L���Ɋւ�炸�A��� TTSSH �����[�h����悤�ɂ����B
  �E���p�J�i��UTF-8�����R��Ă����̂��C������
  �EINI�t�@�C���ɂ�����TTSSH���L���ɂȂ��Ă���ƁACygterm�����܂��N������Ȃ����ւ̑Ώ��i/nossh�I�v�V������ǉ��j�B

2004.9.29 (Ver 2.04a)
  �E�z�z�`�����p�b�`����o�C�i���֕ύX�����B
    TeraTerm����ҁi�����l�j��苖�����������܂����B���ӂ��܂��B

2004.9.5 (Ver 2.04)
  �E���O�̎撆�̃}�N�����s����~���邱�Ƃ�������ւ̑Ώ�
  �EUTF-8���ȍ~�ɒǉ������@�\��GUI����ݒ�ł���悤�ɂ����iEdit - External setup��ǉ��j
  �EANSI Color�̐ݒ���e��ini�t�@�C���֕ۑ�����悤�ɂ���

2004.8.6 (Ver 2.03)
  �ECygwin�ڑ��̃A�N�Z�����[�^�L�[��"Alt + G"�����蓖�Ă�
  �E���O�̎�_�C�A���O�Ƀt�@�C�����̃t���p�X�\����ǉ�����
  �E���O�̎撆�Ƀt�@�C���փR�����g��ǉ��ł���悤�ɂ���
  �ESetLayeredWindowAttributes() API��ÓI�����N���瓮�I���[�h�֕ύX�����iNT4.0�Ή��j�B
  �E�����t�@�C���쐬�c�[���� WDiff ���� udm �֕ύX����

2004.4.12 (Ver 2.02)
  �EMac OS X(HFS+)����UTF-8���T�|�[�g
  �E���{��ȊO�̌���\�����T�|�[�g�i���P�[���ƃR�[�h�y�[�W�ݒ�̒ǉ��j
  �E���{��ȊO�̌�����͂��T�|�[�g�iimm32.lib�������N�j
  �EBOM(Byte Order Mark)���폜����悤�ɂ���
  �E���̑��ׂ����o�O�t�B�b�N�X

2004.3.19 (Ver 2.00a)
  �EREADME�̋L�q���C��
  �EREADME�̉p����

2004.3.18 (Ver 2.00)
  �E�x�[�X�\�[�X���I���W�i������IPv6�ł֕ύX�����B
  �E�������E�B���h�E�ɑΉ�����
  �ECygwin�ڑ��ɑΉ�����
  �E�o�b�t�@�̑S�I�����ł���悤�ɂ���
  �E�}�E�X�J�[�\���̎�ʂ�ݒ�ł���悤�ɂ���
  �EWinSock1.1����2.2�֐؂�ւ���
  �E���̑��ׂ����o�O�t�B�b�N�X

2004.3.13 (Ver 1.02)
  �E�z�X�g���̍ő咷��80�֖߂���
  �E���M�ɂ�UTF-8�Ή�����

2004.3.9 (Ver 1.01a)
  �ESSH�̗��p�Ɋւ���L�q��README�ɋL�ځB
  �ESSH��TTSSH 1.5.4 + IPv6 0.81�ɂē���m�F���s�����B

2004.3.6 (Ver 1.01)
  �E�z�C�[���}�E�X�ɑΉ�����
  �E/KR �I�v�V������"UTF8"��ǉ�����
  �EUTF-8�ϊ��@�\�̋����i�e�[�u��������ǉ��j
  �E�z�X�g���̍ő咷��80����1024�֊g��

2004.3.2 (Ver 1.00)
  �E�����J

�ȏ�

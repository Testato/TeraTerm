TTXResizeMenu -- Tera Term��VT�E�B���h�E�̃T�C�Y��ύX���郁�j���[��\������

�@�\:
  Tera Term�̃��j���[�ɁAResize���j���[��ǉ����܂��B
  Resize���j���[���̍��ڂ��I�������ƁA���̃T�C�Y��Tera Term��VT�E�B���h�E��
  �T�C�Y���ύX����܂��B

���:
  TTXResizeWin�����j���[�`���ɂ��āA�����̃T�C�Y�̒�����I�ׂ�悤�ɂ���
  �����̂��̂ł��B
  ���T�C�Y�����̏ڍׂɂ��ẮATTXResizeWin�̉�����Q�Ƃ��Ă��������B

  ���j���[���̍��ڂ�ύX�������ꍇ�́ATERATERM.INI��[Resize Menu]�Z�N�V������
  �쐬���AResizeMenuN = X, Y �̌`���Ŏw�肵�ĉ������BN �� 1 ����̘A�Ԃł��B
  X �܂��� Y �̂����ꂩ�� 0 �̎��́A���̗v�f�͕ύX����܂���B�Ⴆ�� 0, 37 ��
  ���́A�����͕ύX�����ɏc���� 37 �s�ɕύX���܂��B
  X �܂��� Y �̒l�� -1 �ɂ����ꍇ�A��ʃT�C�Y�𒴂��Ȃ��͈͂ł̍ő�l�Ƃ���
  �����܂��B
  ���j���[�̍��ڐ��͍ő�� 20 �ł��B�ő吔�𑝂₵�������� TTXResizeMenu.c ��
  MAX_MENU_ITEMS �𑝂₵�Ă���r���h�������Ă��������B

�ݒ��:
  [Resize Menu]
  ;
  ; ���� 80��, �c�� 37�s
  ResizeMenu1 =  80, 37
  ;
  ; VT�̕W���T�C�Y
  ResizeMenu2 =  80, 24
  ;
  ; �����͕ύX�����ɁA�c����52�s�ɂ���B
  ResizeMenu3 =   0, 52
  ;
  ; �c���͕ύX�����ɁA������132���ɂ���B(VT��132�����[�h(DECCOLM)����)
  ResizeMenu4 = 132,  0

ToDo:
  ���j���[�̍��ڂ��A�ݒ�_�C�A���O����ύX�ł���悤�ɂ���B

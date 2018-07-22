#!/usr/bin/env ruby
# encoding: ASCII-8BIT

Encoding.default_external = "ASCII-8BIT" if RUBY_VERSION >= "1.9.0"

require 'timeout'
require 'kconv'

def rawio(t=nil, &blk)
  saved_mode = nil
  open("|stty -g") do |stty|
    saved_mode = stty.gets
  end
  begin
    system("stty raw -echo")
    if t
      return Timeout.timeout(t, &blk) 
    else
      return blk.call
    end
  ensure
    system("stty #{saved_mode}")
  end
end

def check_kcode
  begin
    rawio(1) do
      print "\r\xe6\xa4\xa3\xe6\x8e\xa7\e[6n\e[1K\r"
      buff = ""
      while c = STDIN.getc
	buff << c.chr
	if /(\x9c|\x1b\[)(\d+);(\d+)R/ =~ buff
	  case $3.to_i
	  when 5
	    $out_code = :toutf8
	  when 6
	    $out_code = :toeuc
	  when 7
	    $out_code = :tosjis
	  end
	  break
 	end
      end
    end
  rescue Timeout::Error
    $out_code = nil
  end
end

def msgout(msg)
  if $out_code
    puts msg.to_s.method($out_code).call
  else
    puts msg.to_s
  end
end

def getClipboard
  begin
    return rawio(1) do
      rdata = ""
      cbnum = ""

      print "\e]52;c;?\e\\"

      while (c = STDIN.getc)
	break if c.ord == 3 || c.ord == 4
	rdata << c
	if /(\e\]|\x9d)52;([cps0-7]+);/ =~ rdata
	  cbnum = $2
	  break
	end
      end

      rdata = ""
      if (cbnum != "")
	while (c = STDIN.getc)
	  break if c.ord == 3 || c.ord == 4
	  rdata << c
	  if /(\x9c|\x1b\\)/ =~ rdata
	    return $`.unpack("m")[0]
	    break
	  end
	end
      end
      nil
    end
  rescue Timeout::Error
    nil
  end
end

def setClipboard(data)
  printf "\e]52;c;#{[data].pack("m").chomp}\e\\"
end

def getBracketedString
  bracket_type = nil
  rdata = ""
  begin
    return rawio(30) do
      pdata = nil

      c = STDIN.getc
      if c.ord == 3 || c.ord == 4
	return [:interrupt, rdata]
      end
      rdata << c

      # ��ʸ���Ǥ��褿��Ĥ�Υ����ॢ���Ȥ� 1 �ä��ѹ�����
      Timeout.timeout(1) do
	while (c = STDIN.getc)
	  if c.ord == 3 || c.ord == 4
	    return [:interrupt, rdata]
	  end
	  rdata << c
	  if /(?:\e\[|\x9b)20([01])~/ =~ rdata
	    pdata = $`
	    bracket_type = $1.to_i
	    break
	  end
	end

	case bracket_type
	when nil
	  # ���ʤ��Ϥ�������
	  return [:interrupt, rdata]
	when 0
	  nil
	when 1
	  return [:nostart, pdata]
	else
	  # �����ʤ��Ϥ�
	  return [:invalid, bracket_type]
	end

	rdata = ""
	while (c = STDIN.getc)
	  if c.ord == 3 || c.ord == 4
	    return [:interrupt, rdata]
	  end
	  rdata << c
	  if /(\e\[|\x9b)201~/ =~ rdata
	    return [:ok, $`]
	  end
	end
      end
      [:noend, rdata]
    end
  rescue Timeout::Error
    case bracket_type
    when nil
      [:timeout, rdata]
    when 0
      [:noend, rdata]
    when 1
      [:nostart, rdata]
    else
      return [:invalid, bracket_type]
    end
  ensure
    print "\e[?2004l"
  end
end

def testBracketedPaste(msg, str, enableBracket)
  msgout msg
  setClipboard str
  print "\e[?2004h" if enableBracket
  msgout "Ž���դ����(�ޥ������ܥ��󥯥�å���Alt+v��)��ԤäƤ���������"
  result, data = getBracketedString

  case result
  when :ok
    if enableBracket
      if str == data
	msgout "���: ��"
      else
	msgout "���: �� - ����ȱ��������פ��ޤ��󡣼���ʸ����: \"#{data}\""
      end
    else
      if str == data
	msgout "���: �� - ��ͭ�����ʤΤ� Bracket �ǰϤޤ�Ƥ��ޤ���"
      else
	msgout "���: �� - ��ͭ�����ʤΤ� Bracket �ǰϤޤ�Ƥ��ޤ�������ȱ��������פ��ޤ��󡣼���ʸ����: \"#{data}\""
      end
    end
  when :interrupt
    msgout "���: �� - ���������Ǥ���ޤ�����"
  when :invalid
    msgout "���: �� - �۾�ʾ��֤Ǥ���"
  when :timeout
    if enableBracket
      if str == data
	msgout "���: �� - ������ Bracket �ǰϤޤ�Ƥ��ޤ���"
      else
	msgout "���: �� - �����ॢ���Ȥ��ޤ���������ʸ����: \"#{data}\""
      end
    else
      if str == data
	msgout "���: ��"
      else
	msgout "���: �� - ����ȱ��������פ��ޤ��󡣼���ʸ����: \"#{data}\""
      end
    end
  when :nostart
    msgout "���: �� - ���� Bracket ����ʤ��Τ˽�λ Bracket ����ޤ���������ʸ����: \"#{data}\""
  when :noend
    msgout "���: �� - ���� Bracket ����ޤ�������λ Bracket ����ޤ���Ǥ���������ʸ����: \"#{data}\""
  else
    msgout "���: �� - �۾�ʾ��֤Ǥ���"
  end
  puts ""
end

check_kcode
msgout "Bracketed Paste Mode �Υƥ��Ȥ�Ԥ��ޤ���"
msgout "������ע֤͡���¾������ע͡����楷�����󥹡פˤ���"
msgout "�֥�⡼�Ȥ���Υ���åץܡ��ɥ��������פ�Ƚ���Τߡɤޤ��ϡ��ɹ�/����ɤˤ��Ʋ�������"
msgout "���������褿��꥿���󥭡��򲡤��Ʋ�������"
STDIN.gets

testBracketedPaste("�ƥ���1: �̾�", "Bracketed Paste Test", true)
testBracketedPaste("�ƥ���2: ��ʸ����", "", true)
testBracketedPaste("�ƥ���3: ��ͭ����", "Non-Bracketed Mode Test", false)

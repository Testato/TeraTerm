==============================================================================
* UTF-8 support package for TeraTerm Pro        [User's Manual]
==============================================================================

* Introduction
  This software package adds UTF-8 support to terminal emulator "TeraTerm Pro".


* Content of archive file
  Archive (zip) file contains:

  File name               Description  
  ---------------------+-----------------------------------
  readme.txt              this file
  cygterm.exe             "Cygterm" main program
  cygterm.cfg             "Cygterm" configuration file
  TERATERM_sample.INI     sample ini file
  ttp*.dll                TeraTerm DLL files
  tt*.exe                 TeraTerm executable files
  keycode.exe             keycode program
  ttxssh.dll              SSH2 supporting version of TTSSH
  msvcr71.dll             MFC7.1 runtime library
  ---------------------+-----------------------------------


* Installation

  1. Original version of TeraTerm Pro has to be installed before applying this package.  
     It can be downloaded from http://hp.vector.co.jp/authors/VA002416/ttermp23.zip
  2. Backup all files from TeraTerm directory. 
  3. Extract content of UTF-8 support package archive into the TeraTerm directory. 
     Overwrite existing files if prompted. 
  4. Make sure that "cygterm.exe" and "cygterm.cfg" files are extracted into the TeraTerm 
     directory.
  
  [NOTE]
    UTF-8 function also supports the Japanese mode however to be able to use it you have 
    to install the original TeraTerm Pro with the Japanese language mode selected.

* Unicode configuration
  To use UTF-8, select "Terminal" from the TeraTerm Pro "Setup" menu.
  Inside the dialog-box, select "UTF-8" from "Kanji(receive)" or "Kanji(transmit)".
  There is no need to restart TeraTerm Pro to activate these configuration changes.
  When "UTF8" is specified with '/KT' or '/KR' option in the command line,
  UTF-8 encoding/decoding can be used during transmitting and receiving of data.
  
  To enable character sets other than Japanese, you have to set properly the locale and 
  codepage parameters in the 'teraterm.ini' file. See example of these values below.
------------------------------------------------------------------------------------
; Locale for Unicode
Locale = japanese

; CodePage for Unicode
CodePage = 932
------------------------------------------------------------------------------------
  Check the following web-sites to learn more about setting of locale and codepage in TeraTerm:

    http://www.microsoft.com/japan/msdn/library/default.asp?url=/japan/msdn/library/ja/vclib/html/_crt_language_strings.asp
    
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_81rn.asp


  [NOTE] for Mac OS X users
    For Mac OS X(HFS+) use "UTF-8m" encoding. Currently it only supports receiving mode.
    To use this mode specify "UTF8m" as the value of the command line parameter '/KR'.


* ANSI color change & concatenating multi-line patch
  This package includes "ANSI color & concatenating multi-line patch".
  The following description was added to clarify its functionality.

------------------------------------------------------------------------------------
;	ANSI color definition (in case FullColor=on)
;	* UseTextColor should be off, otherwise background and foreground colors of
;	  VTColor will be assigned to color-numbers 0 and 7 respectively, even if
;	  they were specified in ANSIColor.
;	* ANSIColor is a set of 4 values that are color-numbers(0--15),
;	  red-value(0--255), green-value(0--255) and blue-value(0--255).
ANSIColor=0,0,0,0, 1,255,0,0, 2,0,255,0, 3,255,255,0, 4,128,128,255, 5,255,0,255, 6,0,255,255, 7,255,255,255, 8,0,0,0, 9,255,0,0, 10,0,255,0, 11,255,255,0, 12,128,128,255, 13,255,0,255, 14,0,255,255, 15,255,255,255

;	Enable continued-line copy
EnableContinuedLineCopy=on
------------------------------------------------------------------------------------

  [HINT]
    When TermType is set to "xterm" the applications (i.e. vim6) supporting ANSI color
    will appear in color.
    This function is from the original TeraTerm.


* Wheel mouse
  This package supports wheel mouse button.


* Mouse cursor
  Mouse cursor can have one of the 4 shapes listed below

------------------------------------------------------------------------------------
; Mouse cursor type (arrow/ibeam/cross/hand)
MouseCursor = hand
------------------------------------------------------------------------------------

    arrow = standard arrow cursor
    ibeam = ibeam cursor(default)
    cross = cross cursor
    hand  = hand cursor


* Translucent window
  With this package installed TeraTerm can have translucent window (only in Windows 2000 
  and higher versions of Windows).
  Default value is 255 (opaque).

------------------------------------------------------------------------------------
; Translucent window (0 - 255)
AlphaBlend = 230
------------------------------------------------------------------------------------


* Cygwin connection
  This package makes it possible to connect local Cygwin by using built in Cygterm.
  Select "Cygwin connection" from "File" menu or press "Alt + G " to connect to Cygwin.
  Prior connecting to Cygwin, Cygwin installation path has to be entered into TeraTerm 
  ini file.
  The format is "drive-letter:\directory".

------------------------------------------------------------------------------------
; Cygwin install path
CygwinDirectory = c:\cygwin
------------------------------------------------------------------------------------

  [NOTE]
    If cygterm fails to start you might need to download its source and
    recompile it.
    Cygterm source is available from the following web-site:
      http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index-e.html


* Select all buffer
  Select "Select all" from "Edit" menu to select complete screen buffer and copy it
  into clipboard.
  
  [HINT]
    Dragging the mouse while SHIFT key is pressed selects rectangular block of text.
    This function is from the original TeraTerm.


* Add comment to the log file
  Select "Comment to Log..." from "File" menu to add comment to the log file.
  Maximum allowed length of comment is 255 characters.


* External Setup dialog
  Select "External setup" from "Edit" menu to configure the new TeraTerm options 
  introduced by the UTF-8 package.


* line buffer
  If you configurate 10000 lines over of TeraTerm line buffer, you need to 
  setup MaxBuffSize entry in the 'teraterm.ini' file. See example of this value below.

------------------------------------------------------------------------------------
; Max scroll buffer size
MaxBuffSize=500000
------------------------------------------------------------------------------------


* Availability of SSH
  SSH(Secure SHell) version 1 & 2 are supported when using the SSH2 compatible TTSSH version.

  These are the following options of SSH.
  
     option     explanation
     /1         connection with SSH1
     /2         connection with SSH2 (does not support SSH2 at server, do SSH1 connection)
  
  Example of command line option (IPv4 & SSH2 & UTF-8)

    ttermpro.exe /KR=utf8 /4 192.168.1.3:22 /ssh /2

  [NOTE]
      UTF-8 version of TeraTerm is not compatible with original TTSSH and
      with Japanese version of TTSSH. 
      It only works with the latest version of TTSSH supporting SSH2.


* Development Environment
  OS: Windows XP Professional
  Compiler: Visual Studio .NET 2003
  
  Software:
  TeraTerm Pro 2.3
  IPv6 0.81
  Cygterm 1.06
  TTSSH 1.5.4
  

* Copyright
  This program is free software.
  
  Copyright of original TeraTerm Pro belongs to T.Teranishi.
  Copyright of ANSI color change & concatenating multi-line patch belongs to N.Taga.
  Copyright of UTF-8 code belongs to Yutaka Hirata.
  Copyright of IPv6 version of TeraTerm Pro belongs to J.Kato.
  Copyright of Cygterm belongs to NSym.
  Copyright of original TTSSH belongs to Robert O'Callahan.
  Copyright of original TeraTerm Menu to Shinpei Hayakawa.
  Copyright of LogMeIn to NeoCom Solutions.

  This program is provided "as is" without warranties of any kind, either expressed or 
  implied, including, but not limited to, the implied warranties of merchantability and fitness 
  for a particular purpose. The entire risk as to the quality and performance of the program is 
  with you. Should the program prove defective, you assume the cost of all necessary servicing, 
  repair or correction. In no event will any copyright holder be liable to you for damages, 
  including any general, special, incidental or consequential damages arising out of the use or 
  inability to use the program (including but not limited to loss of data or data being rendered 
  inaccurate or losses sustained by you or third parties or a failure of the program to operate 
  with any other programs).

  e-mail : yutakakn@mail.goo.ne.jp
  URL    : http://hp.vector.co.jp/authors/VA013320/
  CVS    : http://sourceforge.jp/projects/ttssh2/
  TeraTerm Pro: http://hp.vector.co.jp/authors/VA002416/
  ANSI color change & concatenating multi-line patch: http://www.sakurachan.org/ml/teraterm/msg00264.html
  IPv6 version TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  Cygterm: http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index.html
  TeraTerm Menu: http://park11.wakwak.com/~shinpei/freesoft/index.html
  LogMeIn: http://www.neocom.ca/freeware/LogMeIn/


* Notes of current release
  - The characters that can't be converted from UTF-8 to Shift_JIS will be shown as '?'.
  - The EUC output in UTF-8 environment may not be shown on TeraTerm screen.


* Limitations of current release
  - UTF-8 version is only Japanese characters now.
  - At "Duplicate session" on SSH2 connected communication, TeraTerm always try to 
    connect the server with SSH2 protocol.


* History

2004.12.27 (Ver 2.07g)
  - upgraded TTSSH version supporting SSH2 to 1.05

2004.12.23 (Ver 2.07f)
  - upgraded TTSSH version supporting SSH2 to 1.04

2004.12.18 (Ver 2.07e)
  - upgraded TTSSH version supporting SSH2 to 1.03

2004.12.16 (Ver 2.07d)
  - upgraded TTSSH version supporting SSH2 to 1.02

2004.12.15 (Ver 2.07c)
  - fixed the attribute of font file in the installed directory
  - added killing the process relation to TeraTerm at installation (using taskkill command)

2004.12.14 (Ver 2.07b)
  - changed the initial value of scroll buffer size from 400000 to 20000 in the 'teraterm.ini' file.
  - added changing the attribute of font file to ReadOnly.
  - upgraded TeraTerm Menu supporting SSH2 to 1.01. Changed the initial folder path of ttermpro.exe to current directory.

2004.12.11 (Ver 2.07a)
  - upgraded TTSSH version supporting SSH2 to 1.01

2004.12.8 (Ver 2.07)
  - renamed "External setup" "Additional settings and removed the dialog to "Setup" menu
  - added "Duplicate session" to "File" menu (add /DUPLICATE option)
  - changed downing ttmacro.exe process priority only with logging
  - didn't include CR+LF in log file when multiple line has concatenated (as EnableContinuedLineCopy enabled)
  - extended the max line of scroll buffer to 400000 line
  - added LogMeIn 1.21. It is thankful to Boris Maisuradze.
  - added TeraTerm Menu 0.94r2(+SSH2 support 1.00). It is thankful to Shinpei Hayakawa.
  - upgraded TTSSH version supporting SSH2 to 1.00(RTM)
  - in addition, fine bug fix

2004.11.6 (Ver 2.06)
  - fixed drawing ruled line with DEC special font remains garbage graphics, moreover UTF-8 feature to display only Japanese characters. And also fixed BOLD style of font problem.
  - TeraTerm second later instances can read configuration from TERATERM.INI.
  - added drag&drop directory path in teraterm from explorer.
  - rewrote README(English). Special thanks to Boris Maisuradze for his cooperation.

2004.10.25 (Ver 2.05a)
  - upgraded TTSSH version supporting SSH2 to 1.00alpha4

2004.10.11 (Ver 2.05)
  - added SSH2 version TTSSH(1.00alpha2)
  - added MFC7.1 runtime library
  - setup environmental variable 'TERATERM_EXTENSIONS' is now ignored, TTSSH is always loaded.
  - fixed UTF-8 encoding of HANKAKU KANA.
  - added /nossh option (fixed problem that doesn't start Cyterm with TTSSH section(Enabled=1) of TERATERM.INI file)

2004.9.29 (Ver 2.04a)
  - changed patch to binary files for distribution
    I replied permission from the TeraTerm original author (Mr.Teranishi). I appreciate.

2004.9.5 (Ver 2.04)
  - fixed running macro halt problem on logging
  - added "External setup" dialog to "Edit" menu
  - saved ANSI color setting to ini file

2004.8.6 (Ver 2.03)
  - assigned the accelerator key to Cygwin connection menu to 'Alt + G'
  - added show of the full path in logging dialog
  - added support of users comments that can be included into log file 
  - replaced static link to dynamic load of SetLayeredWindowAttributes() API (for NT4.0)
  - replaced WDiff with udm of difference making tool

2004.4.12 (Ver 2.02)
  - added support of UTF-8 for Mac OS X(HFS+)
  - added feature to display other than Japanese characters (locale and codepage have to be set up)
  - added feature to input other than Japanese characters (link 'imm32.lib')
  - removed BOM(Byte Order Mark) in receive mode
  - In addition, fine bug fix

2004.3.19 (Ver 2.00a)
  - fixed README file

2004.3.18 (Ver 2.00)
  - changed original source to IPv6 version (thanks to Mr.J.Kato).
  - added support of translucence window
  - added support of Cygwin connection
  - added support of selecting all buffer
  - added support of different mouse cursor types and their configuration
  - switched from WinSock1.1 to 2.2
  - In addition, fine bug fix

2004.3.13 (Ver 1.02)
  - reverted hostname max-length to 80.
  - added support of UTF-8 for sending

2004.3.9 (Ver 1.01a)
  - fixed README file
  - checked that SSH is operational with TTSSH 1.5.4 + IPv6 0.81

2004.3.6 (Ver 1.01)
  - added support of wheel mouse button
  - added "UTF8" value for /KR option of command line
  - powered up UTF-8 conversion (addition referrer table)
  - extended hostname max-length from 80 to 1024

2004.3.2 (Ver 1.00)
  - first release


See you next version. Up to it, happy TeraTerm life!


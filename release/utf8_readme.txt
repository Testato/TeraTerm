==============================================================================
* UTF-8 support package for TeraTerm Pro        [User's Manual]
==============================================================================

* Introduction
  This software package adds UTF-8 & SSH2 support to terminal emulator "TeraTerm Pro".


* Installation
  If you use TeraTerm installer program, click installer program and install TeraTerm according to the instruction of the installer.
  If you use TeraTerm archives, extract TeraTerm zip file and copy all files to your directory.
  

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
    Dragging the mouse while Alt key is pressed selects rectangular block of text.
    This function is from the original TeraTerm.


* Add comment to the log file
  Select "Comment to Log..." from "File" menu to add comment to the log file.
  Maximum allowed length of comment is 255 characters.


* External Setup dialog
  Select "External setup" from "Edit" menu to configure the new TeraTerm options 
  introduced by the UTF-8 package.


* line buffer
  If you configure 10000 lines over of TeraTerm line buffer, you need to 
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


* macro reference: connect

There are 3 types of connection you can establish from TeraTerm macro: 

  - Telnet connection (port 23) 
  - SSH1 or SSH2 connection (port 22) 
  - Connection via COM port 
  
  Telnet connections 
  
  connect 'myserver:23 /nossh' 
  or 
  connect 'myserver:23' 
  
  Using /nossh is strongly recommended. Without this parameter TeraTerm will start connecting with the same method (telnet or SSH) that was used during last time when teraterm.ini file was saved. In case it was SSH than your macro will try to connect via SSH and will eventually fail. 
  
  SSH connections 
  
  connect 'myserver /ssh' 
  or 
  connect 'myserver /ssh /1' 
  or 
  connect 'myserver /ssh /2' 
  or 
  connect 'myserver /ssh /auth=password /user=username /passwd=password'' 
  or 
  connect 'myserver /ssh /1 /auth=password /user=username /passwd=password'' 
  or 
  connect 'myserver /ssh /2 /auth=password /user=username /passwd=password'' 
  or 
  connect 'myserver /ssh /auth=publickey /user=username /passwd=password /keyfile=private-key-file' 
  or 
  connect 'myserver /ssh /1 /auth=publickey /user=username /passwd=password /keyfile=private-key-file' 
  or 
  connect 'myserver /ssh /2 /auth=publickey /user=username /passwd=password /keyfile=private-key-file' 
  
  The first way will start SSH connection without defining whether SSH1 or SSH2 has to be used. Parameters /1 and /2 force to use SSH1 or SSH2 method. The last 6 ways allow to skip popup dialog and pass username and password directly from macro. Please note that /auth=password or /auth=publickey is the parameter saying that authentication will be done by entering password or private-key-file thus you should not replace the word 'password' or the word 'RSA/DSA' in it with actual password. Only the words shown above with italic font has to be substituted with actual values. 
  Please remember that entering actual username and password in TeraTerm macro will cause them to be stored as an open text and it is your responsibility to keep such macro is secure location. 
  
  Connections via COM port 
  
  connect '/C=x' 
  
  Here x represents COM port number. For example to connect via COM port 1 the command will look like: connect '/C=1'
  
  source URL: http://www.neocom.ca/forum/viewtopic.php?t=6



* background transparency
  TeraTerm supports window background transparency. Now TeraTerm has two types of transparency function. You can select either function. 
  
  (1) AlphaBlend window
  In this type TeraTerm window can be background transparency. You can setup the value of transparency. The range of transparency is from 0 to 255. The more transparency reduces the value, the more window transparency is blinding. You can configure the value in teraterm.ini file. The entry is 'AlphaBlend' of [teraterm] section. I show the example in the following:
  
	; Translucent window (0 - 255)
	AlphaBlend =255
     
  As a point that should be noted, this function is enabled on only Windows2000 later.
  
  (2) Eterm lookfeel window
  In this type TeraTerm window looks like Eterm background transparency. If this function is enabled, TeraTerm window becomes background transparency and mixes the wallpaper with your specified image file(*.jpg). And also this function is also enabled on Windows9x or NT4.0. You can configure this function in [BG] section of teraterm.ini file. If you switch it on or off, you should change 'BGEnable' value.

	[BG]
	; Use Background extension (On / Off)
	BGEnable = Off

  Other settings are the following entries.

	; Use AlphaBlend API (On / Off)
	BGUseAlphaBlendAPI = On

	; Susie plugin path
	BGSPIPath = plugin

	; Fast window sizing/moving
	BGFastSizeMove = On

	; Flickerless window moving
	BGFlickerlessMove = On

	; If HideTitle = on and BGNoFrame = on,use window without frame
	; you can resize window with Alt + Shift + LeftDrag
	BGNoFrame = On

	; wildcard => random select
	BGThemeFile = theme\*.ini


* Keyboard & Mouse operation

  Ctrl + Tab, Ctrl + Shift + Tab
    Switch to TeraTerm next/prev window
  
  Alt + Enter
    Maximize TeraTerm window(toggle)
  
  Alt + Space  Alt + X
    Maximize TeraTerm window
  
  Alt + N
    New connection
    
  Alt + D
    Duplicate session (SSH, telnet only)
    
  Alt + G
    Cygwin connection
    
  Ctrl + Up/Down cursor
    LineUp or LineDown

  Ctrl + PageUp/PageDown
    PageUp or PageDown

  Ctrl + Home
    Move to buffer top

  Ctrl + End
    Move to buffer bottom

  Alt + C, Ctrl + Insert, MouseDrag
    Copy selected string to the clipboard

  Alt + V, Shift + Insert, マウスホイールボタン・右クリック
    Paste from the clipboard
  
  F1 key
    Push Num pad NumLock key

  F2 key
    Push Num pad '/'
    
  F3 key
    Push Num pad '*'

  F4 key
    Push Num pad '-' 

  Alt + MouseDrag
    Select rectangular block of text
  
  Left click  Shift + Left click
    Select several pages of output from TeraTerm window. Or expand ans contract of text area that has been selected.
  
  Mouse wheel button scrolling
    LineUp or LineDown

  Dragging a text file and dropping it into the window
    Sends text in the file to the host.

  Dragging a folder and dropping it into the window
    Sends the folder full-path to the host.


* How to build TeraTerm
  To build TeraTerm source code is shown in the following step:
  And you need Visual Studio .NET 2003(VC++7.1) to build.
  
  1. Checkout TeraTerm source code from SourceForge(http://sourceforge.jp/projects/ttssh2/).
  2. Open teraterm\visualc\ttermpro.sln with Visual Studio.
  3. Build TeraTerm solution.
  4. TeraTerm execution program will be generated in teraterm\visualc\bin directory if the building is successful.


* How to build TeraTerm Menu
  To build TeraTerm Menu source code is shown in the following step:
  And you need Visual Studio .NET 2003(VC++7.1) to build.
  
  1. Checkout TeraTerm source code from SourceForge(http://sourceforge.jp/projects/ttssh2/).
  2. Open teraterm\source\ttmenu\ttpmenu.sln with Visual Studio.
  3. Build TeraTerm Menu solution.
  4. TeraTerm Menu execution program will be generated in teraterm\source\ttmenu directory if the building is successful.


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
  Copyright of LogMeTT to NeoCom Solutions.
  Copyright of Eterm lookfeel transparency window belongs to AKASI.
  Copyright of TTProxy to YebisuyaHompo.

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
  LogMeTT: http://www.neocom.ca/freeware/LogMeIn/
  Eterm lookfeel transparency window patch: http://www.geocities.co.jp/SiliconValley-PaloAlto/4954/av.html
  TTProxy: http://yebisuya.dip.jp/Software/TTProxy/
  TeraTerm forum: http://www.neocom.ca/forum/index.php
  TeraTerm ML: http://www.freeml.com/info/teraterm@freeml.com


* Notes of current release
  - The characters that can't be converted from UTF-8 to Shift_JIS will be shown as '?'.
  - The EUC output in UTF-8 environment may not be shown on TeraTerm screen.

NOTE: The links above contain Japanese version of installer. Users from other countries just press all default buttons that will show up during installation. When installation will complete, start Teraterm and switch the Language setting under Settings->General menu to meet your preference. Finally save changes into teraterm.ini file.


* Limitations of current release
  - UTF-8 version is only Japanese characters now.


* History

2005.5.29 (Ver 4.17)
  -changed scroll out the current buffer when the cursor position is only located on left-upper screen at receiving <ESC>[J(Clear screen from cursor-position up to end).

2005.5.28 (Ver 4.16)
  - added saving log type(plain text) information to teraterm.ini(LogTypePlainText).
  - added CygTerm settings on Additional settings dialog.
  - changed keybind of selecting the rectangular region from Shift+MouseDrag to Alt+MouseDrag.
  - added support of selecting several pages(also expansion ans contraction of text area that has been selected). Start selected region is saved with mouse left click. End selected region is saved with Shift + mouse left click.
  - added scroll out the current buffer when TeraTerm clear screen at receiving <ESC>[J.
  - added the workaround of CPU time 100% using TeraTerm macro(insert 100msec sleep after send and sendln). 

2005.4.24 (Ver 4.15)
  - added the accelerator key(ALT+Enter) that maximizes TeraTerm window.
  - added saving Eterm lookfeel alphablend settings to teraterm.ini file.
  - added changing Eterm lookfeel alphablend on/off on Additional settings dialog.
  - fixed the installer problem not copying font file on Windows98. Special thanks to Boris.
  - added TeraTerm License file(license.txt).
  - upgraded macro help file(English version). Special thanks to Boris.
  - upgraded LogMeTT to 2.4.4


2005.4.9 (Ver 4.14)
  - added clickable URL mechanism that mouse pointer overs on URL string. Special thanks to Kazuaki Ishizaki. And also added URLColor, EnableClickableUrl entry in teraterm.ini file.
  - added support of escape sequence(ESC[39m:foreground color reset, ESC[49m:background color reset). This support solves the problem that color can't be clear using w3m on screen(1) command. Special thanks to Iwamoto Kouichi.
  - added mouse cursor dynamically changing on Additional settings.
  - added support of clickable URL on TeraTerm version dialog.
  - cancelled the limitation that at "Duplicate session" on SSH2 connected communication, TeraTerm always try to connect the server with SSH2 protocol.
  - added SSH autologin with "Duplicate session".
  - upgraded LogMeTT to 2.4.3

2005.3.31 (Ver 4.13)
  - fixed TeraTerm version information can't be shown in version dialog at second later instance.
  - added the configuration of disabling paste string by clicking mouse right button in teraterm.ini(DisablePasteMouseRButton) and Additional settings dialog.
  - fixed the TeraTerm crashing problem when the sending file size exceeded 20MB with ZMODEM protocol. Special thanks to Thorsten Behrens.
  - upgraded LogMeTT to 2.4.2

2005.3.13 (Ver 4.12)
  - upgraded ttermpro.exe version information from 2.3 to 4.12.
  - fixed the multibyte problem that the cursor scrolls to upper direction when ZENKAKU(Japanese character) cross the end of line. Special thanks to Iwamoto Kouichi.
  - added TeraTerm title multibyte character converting EUC-JP to Shift_JIS(CP932).
  - upgraded LogMeTT to 2.1.2.

2005.2.22 (Ver 4.11)
  - fixed the problem that log menu doesn't become gray on logging
  - added Eterm lookfeel alphablend. Special thanks to AKASI.
  - added support of VisualStyle(WindowsXP)
  - added new Title Format ID(13). i.e, <host/port> - <title> VT/TEK
  - added the show of COM5 over when the caption was displayed
  - added new logginf type "plain text" on log saving dialog. If the option is enabled(default), TeraTerm doesn't include ASCII non-display characters in log file. Exceptionally the following characters(HT, CR, LF) include at any mode.
  - added TTProxy(2004/1/22 version) supporting to TELNET/HTTP/SOCK proxying. Special thanks to YebisuyaHompo.
  - added newly LogMeTT 2.02beta. Special thanks to Boris.
  - deleted LogMeIn
  - changed "Additional settings" dialog to tab control dialog
  - changed the setting default file name of logging to "teraterm.log"

2005.1.30 (Ver 4.10)
  - enabled maximum button
  - deleted the accelerator key of TeraTerm Menu and LogMeIn
  - added the setting default file name(log_YYYYMMDD_HHMMSS.txt) of logging
  - added the Broadcast command menu under Control menu
  - added the homepage URL at version dialog
  - fixed the initial directory of "Save setup" dialog at reading teraterm.ini directory
  - added "View Log" menu under File menu
  - added "View Log Editor" text-box on "Additional settings" dialog
  - added the "ViewlogEditor" entry at [Tera Term] section of teraterm.ini
  - upgraded TTSSH version supporting SSH2 to 1.08
  - upgraded TeraTerm Menu supporting SSH2 to 1.02.
     -- added Kanji code(/KT=UTF8 /KR=UTF8) of default option
     -- added specify of user paramater at SSH autologin
     -- deleted macro specification at SSH autologin
     -- added specify of private-key-file at SSH autologin

2005.1.10 (Ver 2.09)
  - fixed the bug of switching back to primary display when changing terminal size on multi display environment. Special thanks to Tsuruhiko Ando.

2005.1.6 (Ver 2.08)
  - changed the logging dialog type from open-dialog to save-dialog
  - fixed the initial directory of "Save setup" dialog at program directory
  - upgraded TTSSH version supporting SSH2 to 1.07

2005.1.5 (Ver 2.07h)
  - upgraded TTSSH version supporting SSH2 to 1.06

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
  - changed downing ttpmacro.exe process priority only with logging
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
  - added /nossh option (fixed problem that doesn't start Cygterm with TTSSH section(Enabled=1) of TERATERM.INI file)

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


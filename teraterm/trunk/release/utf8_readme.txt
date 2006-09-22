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

    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore98/html/_crt_language_strings.asp
    
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_81rn.asp


  [Example of WindowsXP Simplified Chinese]
  ------------------------------------------------------------------------------------
    ; Locale for Unicode 
    Locale = chs 
    
    ; CodePage for Unicode 
    CodePage = 936   
  ------------------------------------------------------------------------------------


  [NOTE] for Mac OS X users
    For Mac OS X(HFS+) use "UTF-8m" encoding. Currently it only supports receiving mode.
    To use this mode specify "UTF8m" as the value of the command line parameter '/KR'.


  [NOTE] Language Strings for Locale
Primary         Sublanguage     String          
---------------+--------------+-------------------------------------------------------
Chinese         Chinese         "chinese" 
Chinese         Chinese         (simplified) "chinese-simplified" or "chs" 
Chinese         Chinese         (traditional) "chinese-traditional" or "cht" 
Czech           Czech           "csy" or "czech" 
Danish          Danish          "dan"or "danish" 
Dutch           Dutch           (Belgian) "belgian", "dutch-belgian", or "nlb" 
Dutch           Dutch           (default) "dutch" or "nld" 
English         English         (Australian) "australian", "ena", or "english-aus" 
English         English         (Canadian) "canadian", "enc", or "english-can" 
English         English         (default) "english" 
English         English         (New Zealand) "english-nz" or "enz" 
English         English         (UK) "eng", "english-uk", or "uk" 
English         English         (USA) "american", "american english", "american-english", "english-american", "english-us", "english-usa", "enu", "us", or "usa" 
Finnish         Finnish         "fin" or "finnish" 
French          French          (Belgian) "frb" or "french-belgian" 
French          French          (Canadian) "frc" or "french-canadian" 
French          French          (default) "fra"or "french" 
French          French          (Swiss) "french-swiss" or "frs" 
German          German          (Austrian) "dea" or "german-austrian" 
German          German          (default) "deu" or "german" 
German          German          (Swiss) "des", "german-swiss", or "swiss" 
Greek           Greek           "ell" or "greek" 
Hungarian       Hungarian               "hun" or "hungarian" 
Icelandic       Icelandic               "icelandic" or "isl" 
Italian         Italian         (default) "ita" or "italian" 
Italian         Italian         (Swiss) "italian-swiss" or "its" 
Japanese        Japanese                "japanese" or "jpn" 
Korean          Korean          "kor" or "korean" 
Norwegian       Norwegian               (Bokmal) "nor" or "norwegian-bokmal" 
Norwegian       Norwegian               (default) "norwegian" 
Norwegian       Norwegian               (Nynorsk) "non" or "norwegian-nynorsk" 
Polish          Polish          "plk" or "polish" 
Portuguese      Portuguese              (Brazil) "portuguese-brazilian" or "ptb" 
Portuguese      Portuguese              (default) "portuguese" or "ptg" 
Russian         Russian         (default) "rus" or "russian" 
Slovak          Slovak          "sky" or "slovak" 
Spanish         Spanish         (default) "esp" or "spanish" 
Spanish         Spanish         (Mexican) "esm" or "spanish-mexican" 
Spanish         Spanish         (Modern) "esn" or "spanish-modern" 
Swedish         Swedish         "sve" or "swedish" 
Turkish         Turkish         "trk" or "turkish" 


  [NOTE] Code-Page Identifiers
Identifier      Name 
037             IBM EBCDIC - U.S./Canada 
437             OEM - United States 
500             IBM EBCDIC - International  
708             Arabic - ASMO 708 
709             Arabic - ASMO 449+, BCON V4 
710             Arabic - Transparent Arabic 
720             Arabic - Transparent ASMO 
737             OEM - Greek (formerly 437G) 
775             OEM - Baltic 
850             OEM - Multilingual Latin I 
852             OEM - Latin II 
855             OEM - Cyrillic (primarily Russian) 
857             OEM - Turkish 
858             OEM - Multlingual Latin I + Euro symbol 
860             OEM - Portuguese 
861             OEM - Icelandic 
862             OEM - Hebrew 
863             OEM - Canadian-French 
864             OEM - Arabic 
865             OEM - Nordic 
866             OEM - Russian 
869             OEM - Modern Greek 
870             IBM EBCDIC - Multilingual/ROECE (Latin-2) 
874             ANSI/OEM - Thai (same as 28605, ISO 8859-15) 
875             IBM EBCDIC - Modern Greek 
932             ANSI/OEM - Japanese, Shift-JIS 
936             ANSI/OEM - Simplified Chinese (PRC, Singapore) 
949             ANSI/OEM - Korean (Unified Hangeul Code) 
950             ANSI/OEM - Traditional Chinese (Taiwan; Hong Kong SAR, PRC)  
1026            IBM EBCDIC - Turkish (Latin-5) 
1047            IBM EBCDIC - Latin 1/Open System 
1140            IBM EBCDIC - U.S./Canada (037 + Euro symbol) 
1141            IBM EBCDIC - Germany (20273 + Euro symbol) 
1142            IBM EBCDIC - Denmark/Norway (20277 + Euro symbol) 
1143            IBM EBCDIC - Finland/Sweden (20278 + Euro symbol) 
1144            IBM EBCDIC - Italy (20280 + Euro symbol) 
1145            IBM EBCDIC - Latin America/Spain (20284 + Euro symbol) 
1146            IBM EBCDIC - United Kingdom (20285 + Euro symbol) 
1147            IBM EBCDIC - France (20297 + Euro symbol) 
1148            IBM EBCDIC - International (500 + Euro symbol) 
1149            IBM EBCDIC - Icelandic (20871 + Euro symbol) 
1200            Unicode UCS-2 Little-Endian (BMP of ISO 10646) 
1201            Unicode UCS-2 Big-Endian  
1250            ANSI - Central European  
1251            ANSI - Cyrillic 
1252            ANSI - Latin I  
1253            ANSI - Greek 
1254            ANSI - Turkish 
1255            ANSI - Hebrew 
1256            ANSI - Arabic 
1257            ANSI - Baltic 
1258            ANSI/OEM - Vietnamese 
1361            Korean (Johab) 
10000           MAC - Roman 
10001           MAC - Japanese 
10002           MAC - Traditional Chinese (Big5) 
10003           MAC - Korean 
10004           MAC - Arabic 
10005           MAC - Hebrew 
10006           MAC - Greek I 
10007           MAC - Cyrillic 
10008           MAC - Simplified Chinese (GB 2312) 
10010           MAC - Romania 
10017           MAC - Ukraine 
10021           MAC - Thai 
10029           MAC - Latin II 
10079           MAC - Icelandic 
10081           MAC - Turkish 
10082           MAC - Croatia 
12000           Unicode UCS-4 Little-Endian 
12001           Unicode UCS-4 Big-Endian 
20000           CNS - Taiwan  
20001           TCA - Taiwan  
20002           Eten - Taiwan  
20003           IBM5550 - Taiwan  
20004           TeleText - Taiwan  
20005           Wang - Taiwan  
20105           IA5 IRV International Alphabet No. 5 (7-bit) 
20106           IA5 German (7-bit) 
20107           IA5 Swedish (7-bit) 
20108           IA5 Norwegian (7-bit) 
20127           US-ASCII (7-bit) 
20261           T.61 
20269           ISO 6937 Non-Spacing Accent 
20273           IBM EBCDIC - Germany 
20277           IBM EBCDIC - Denmark/Norway 
20278           IBM EBCDIC - Finland/Sweden 
20280           IBM EBCDIC - Italy 
20284           IBM EBCDIC - Latin America/Spain 
20285           IBM EBCDIC - United Kingdom 
20290           IBM EBCDIC - Japanese Katakana Extended 
20297           IBM EBCDIC - France 
20420           IBM EBCDIC - Arabic 
20423           IBM EBCDIC - Greek 
20424           IBM EBCDIC - Hebrew 
20833           IBM EBCDIC - Korean Extended 
20838           IBM EBCDIC - Thai 
20866           Russian - KOI8-R 
20871           IBM EBCDIC - Icelandic 
20880           IBM EBCDIC - Cyrillic (Russian) 
20905           IBM EBCDIC - Turkish 
20924           IBM EBCDIC - Latin-1/Open System (1047 + Euro symbol) 
20932           JIS X 0208-1990 & 0121-1990 
20936           Simplified Chinese (GB2312) 
21025           IBM EBCDIC - Cyrillic (Serbian, Bulgarian) 
21027           Extended Alpha Lowercase 
21866           Ukrainian (KOI8-U) 
28591           ISO 8859-1 Latin I 
28592           ISO 8859-2 Central Europe 
28593           ISO 8859-3 Latin 3  
28594           ISO 8859-4 Baltic 
28595           ISO 8859-5 Cyrillic 
28596           ISO 8859-6 Arabic 
28597           ISO 8859-7 Greek 
28598           ISO 8859-8 Hebrew 
28599           ISO 8859-9 Latin 5 
28605           ISO 8859-15 Latin 9 
29001           Europa 3 
38598           ISO 8859-8 Hebrew 
50220           ISO 2022 Japanese with no halfwidth Katakana 
50221           ISO 2022 Japanese with halfwidth Katakana 
50222           ISO 2022 Japanese JIS X 0201-1989 
50225           ISO 2022 Korean  
50227           ISO 2022 Simplified Chinese 
50229           ISO 2022 Traditional Chinese 
50930           Japanese (Katakana) Extended 
50931           US/Canada and Japanese 
50933           Korean Extended and Korean 
50935           Simplified Chinese Extended and Simplified Chinese 
50936           Simplified Chinese 
50937           US/Canada and Traditional Chinese 
50939           Japanese (Latin) Extended and Japanese 
51932           EUC - Japanese 
51936           EUC - Simplified Chinese 
51949           EUC - Korean 
51950           EUC - Traditional Chinese 
52936           HZ-GB2312 Simplified Chinese  
54936           Windows XP: GB18030 Simplified Chinese (4 Byte)  
57002           ISCII Devanagari 
57003           ISCII Bengali 
57004           ISCII Tamil 
57005           ISCII Telugu 
57006           ISCII Assamese 
57007           ISCII Oriya 
57008           ISCII Kannada 
57009           ISCII Malayalam 
57010           ISCII Gujarati 
57011           ISCII Punjabi 
65000           Unicode UTF-7 
65001           Unicode UTF-8 



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


* Additional settings dialog
  Select "Additional settings" from "Edit" menu to configure the new TeraTerm options 
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



* macro reference: waitregex

Format: 
        waitregex <string1 with regular expression> <string2 with regular expression> ...
        
        Supports Oniguruma regular expressions: RE.txt

Pauses until the string(maximum 256 characters), which matches one or more character strings with regular expression is received from the host, or until the timeout occurs. Maximum number of the regular expression strings is 10.

If the system variable "timeout" is greater than zero, the timeout occurs when <timeout> seconds have passed. If the "timeout" is less than or equal to zero, the timeout never occurs.

The "waitregex" command returns one of the following values in the system variable "result":

Value   Meaning 
----------------------- 
0       Timeout. No string, which satisfies the condition, has been received. 
1       <string1 with regular expression> has received.
2       <string2 with regular expression> has received.
:       :
:       :
n       <stringn with regular expression> has received. n=1..10.

The received line is stored in the system variable "inputstr".
The first matched pattern is stored in the system variable "matchstr".
Group matched patterns of regular expressions are stored in 9 system variables from "groupmatchstr1" to "groupmatchstr9".

Example:
timeout = 30
waitregex 'Longhorn' '.*@sai' 'Pentium\d+' 'TeraTermX{3}'
int2str s result
messagebox s 'result'
messagebox inputstr 'inputstr'
messagebox matchstr 'matchstr'


* macro reference: mpause

Format:
        mpause <time>

Pauses for <time> milliseconds.

Example:
mpause 100              Pause for 100 milliseconds.



* macro reference: random

Format:
      random <intvar> <value>

Generates the random number from 0 to <value>.
The value is returned in the integer variable <intvar>.

Example:
random val 100            val=0..100



* macro reference: clipb2var

Format:
      clipb2var <strvar>

Copys text data in clipboard to <strvar>.

The "clipb2var" command returns one of the following values in the system variable "result":

Value   Meaning 
----------------------- 
0       Couldn't open the clipboard, or data is not a text data.
1       Copy to <strvar> was successful.



* macro reference: var2clipb

Format:
      var2clipb <string>

Copys <strvar> to clipboard.

The "clipb2var" command returns one of the following values in the system variable "result":

Value   Meaning 
----------------------- 
0       Couldn't open the clipboard.
1       Copy to clipboard was successful.



* macro reference: ifdefined

Format:
      ifdefined <var>

Sets the type of <var>. One of the following values in the system variable "result":

Value   Meaning 
----------------------- 
0       undefined
1       integer
3       string
4       label



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

  Alt + V, Shift + Insert, Mouse Middle/Right button
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


* macro error message

Error message              Meaning
-------------------------+------------------------------------------------------
Too many labels.           MACRO can not handle more than 512 labels.
Too many variables.        MACRO can not handle more than 256 integer variables and 256 string variables.


* License
Copyright (c) TeraTerm Project. 
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


* Copyright
  This program is free software. TeraTerm is licensed under BSD style license.
  
  Copyright of original TeraTerm Pro belongs to T.Teranishi.
  Copyright of ANSI color change & concatenating multi-line patch belongs to N.Taga.
  Copyright of UTF-8 and SSH2 codes belongs to Yutaka Hirata.
  Copyright of IPv6 version of TeraTerm Pro belongs to J.Kato.
  Copyright of Cygterm belongs to NSym.
  Copyright of original TTSSH belongs to Robert O'Callahan.
  Copyright of original TeraTerm Menu to Shinpei Hayakawa.
  Copyright of LogMeTT to NeoCom Solutions.
  Copyright of Eterm lookfeel transparency window belongs to AKASI.
  Copyright of TTProxy to YebisuyaHompo.
  Copyright of Japanized TeraTerm belongs to Yasuhide Takahashi and Taketo Hashii.
  Copyright of Clickable URL to monkey magic.
  Copyright of Oniguruma to K.Kosako.
  Copyright of preventing scroll at end of line to IWAMOTO Kouichi.
  Copyright of supporting multi display to Tsuruhiko Ando.
  Copyright of general patch about both TeraTerm and TTSSH to NAGATA Shinya.
  Copyright of TeraTerm and TTSSH icon file to Tatsuhiko Sakamoto.
  Copyright of HTML help file to Mitsuyasu Ichimura, Masateru KUWATA, Taketo Hashii and Yutaka Hirata.

  This program is provided "as is" without warranties of any kind, either expressed or 
  implied, including, but not limited to, the implied warranties of merchantability and fitness 
  for a particular purpose. The entire risk as to the quality and performance of the program is 
  with you. Should the program prove defective, you assume the cost of all necessary servicing, 
  repair or correction. In no event will any copyright holder be liable to you for damages, 
  including any general, special, incidental or consequential damages arising out of the use or 
  inability to use the program (including but not limited to loss of data or data being rendered 
  inaccurate or losses sustained by you or third parties or a failure of the program to operate 
  with any other programs).

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
  Oniguruma: http://www.geocities.jp/kosako3/oniguruma/
  TeraTerm forum: http://www.neocom.ca/forum/index.php
  TeraTerm ML: http://www.freeml.com/info/teraterm@freeml.com


* Notes of current release
  - The characters that can't be converted from UTF-8 to Shift_JIS will be shown as '?'.
  - The EUC output in UTF-8 environment may not be shown on TeraTerm screen.


* History

2006.9.23 (Ver 4.46)
  - Bug fix: default save dir of teraterm.ini is setted to desktop on Windows Vista.
  - added 'clipb2var' macro command. This command enables the macro script to get data from clipboard.
  - added 'var2clipb' macro command. This command enables the macro script to set data to clipboard.
  - Bug fix: /L= paramter in New Connection dialog is not used on first TeraTerm window.
  - added 'ifdefined' macro command. This command returns the type of variable.

2006.9.16 (Ver 4.45)
  - updated HTML help document.
  - upgraded TTSSH version supporting SSH2 to 2.33.
  - upgraded CygTerm+ to 1.06_8.
  - upgraded Oniguruma to 4.4.1
  - add default log filename setting.
        * added new edit control to 'Additional settings' dialog.
        * added LogDefaultName entry in teraterm.ini file.
        * expanded the log filename which can include a format of strftime.
          SYNOPSIS:
              %a  Abbreviated weekday name 
              %A  Full weekday name 
              %b  Abbreviated month name 
              %B  Full month name 
              %c  Date and time representation appropriate for locale 
              %d  Day of month as decimal number (01 - 31) 
              %H  Hour in 24-hour format (00 - 23) 
              %I  Hour in 12-hour format (01 - 12) 
              %j  Day of year as decimal number (001 - 366) 
              %m  Month as decimal number (01 - 12) 
              %M  Minute as decimal number (00 -  59) 
              %p  Current locale's A.M./P.M. indicator for 12-hour clock 
              %S  Second as decimal number (00 - 59) 
              %U  Week of year as decimal number, with Sunday as first day of week (00 - 53) 
              %w  Weekday as decimal number (0 - 6; Sunday is 0) 
              %W  Week of year as decimal number, with Monday as first day of week (00 - 53) 
              %x  Date representation for current locale 
              %X  Time representation for current locale 
              %y  Year without century, as decimal number (00 - 99) 
              %Y  Year with century, as decimal number 
              %z, %Z  Either the time-zone name or time zone abbreviation, depending on registry settings; no characters if time zone is unknown 
              %%  Percent sign 
  - disable automatically connection to serial port on startup when /M command line parameter is specified.

2006.8.19 (Ver 4.44)
  - Bug fix: 'ttpmacro.exe' terminated after startup on Windows98/NT4.0.
  - added the mutex object to ttermpro.exe, ttpmacro.exe, ttpmenu.exe, keycode.exe and cygterm.exe.
  - upgraded TTSSH version supporting SSH2 to 2.32.
  - Changed TeraTerm installer software to InnoSetup.
  - updated HTML help document.
  - upgraded CygTerm+ to 1.06_6.
        * Limit a parameter length of both '-s' and '-t' option.
        * Create mutex object for running check by installer.
        * Add 'LOGIN_SHELL' entry which the shell is invoked as the login shell in cygterm          LOGIN_SHELL: yes | no
        * Add 'HOME_CHDIR' entry which home directory is changed in cygterm.cfg file.
          HOME_CHDIR: yes | no

2006.8.4 (Ver 4.43)
  - expanded the number of macro variables which can be used as user variable from 241 to 256. This is bug because some variables were used as system variable.
  - expanded the inputbox dialog of macro. It is resized automaticaly when the message is long than default size.
  - changed '\n' character not raw character but meta character as LF code.
  - changed the message box of macro from dialog border to resizable.
  - set '23' to port number when the hostname start with 'telnet://' and port number was not specified.
  - upgraded TTProxy to 2006/8/23 version. Special thanks to YebisuyaHompo.
  - upgraded TTSSH version supporting SSH2 to 2.31

2006.7.26 (Ver 4.42)
  - updated to new TeraTerm icon file(teraterm.ico, vt.ico). Special thanks to Tatsu Sakamoto.
  - upgraded LogMeTT to 2.8.5
  - Bug fix: Collector resource english text.
  - upgraded TTSSH version supporting SSH2 to 2.30

2006.7.24 (Ver 4.41)
  - added timestamp option on log saving dialog. And also added LogTimestamp entry in teraterm.ini file.
  - upgraded Kantan Installer version to 2.30a. The installer could record the previous installed folder when TeraTerm installation package was upgraded.
  - Bug fix: Collector resource english text.
  - upgraded Oniguruma to 4.2.0

2006.7.19 (Ver 4.40)
  - upgraded LogMeTT to 2.8.4
  - updated HTML help document.
  - upgraded both ttermpro.exe and ttpmacro.exe version information.

2006.7.1 (Ver 4.39)
  - Bug fix: UTF-8 miss deconding when the one byte character(ASCII) comes after the two byte characters.
  - changed Copyright string.

2006.6.23 (Ver 4.38)
  - upgraded TTSSH version supporting SSH2 to 2.28
  - updated HTML help document.

2006.6.18 (Ver 4.37)
  - Bug fix: Title string could not been shown as Japanese character from TeraTerm setup dialog. Special thanks to NAGATA Shinya.
  - upgraded TTSSH version supporting SSH2 to 2.27
  - Bug fix: Collector resource english text.

2006.6.11 (Ver 4.36)
  - upgraded TTSSH version supporting SSH2 to 2.26

2006.5.25 (Ver 4.35b)
  - added 'Collector  3.2.3'(tabbed multi window tool) to TeraTerm installation package. Special thanks to Hiroshi Takahashi.

2006.5.7 (Ver 4.35a)
  - upgraded TTSSH version supporting SSH2 to 2.25

2006.4.7 (Ver 4.35)
  - changed the owner of HTML help file from TeraTerm instance to Windows desktop.
  - upgraded TTSSH version supporting SSH2 to 2.24

2006.4.4 (Ver 4.34)
  - upgraded TTSSH version supporting SSH2 to 2.23

2006.4.1 (Ver 4.33)
  - Bug fix: TeraTerm window flickers though the user doesn't use transparency window.

2006.3.30 (Ver 4.32)
  - upgraded TTSSH version supporting SSH2 to 2.22

2006.3.28 (Ver 4.31)
  - changed TeraTerm Help file from .hlp to .chm style.
  - enabled the transparency value of TeraTerm window to immediately changing on Additional settings dialog.
  - Bug fix: The background color of text matches the background color of screen patch. Special thanks to 337.
  - enabled VisualStyle feature on WindowsXP.
  - deleted one millisecond sleep on VT window.
  - upgraded TTSSH version supporting SSH2 to 2.21
  - upgraded LogMeTT to 2.8.3

2006.3.6 (Ver 4.30)
  - upgraded TTSSH version supporting SSH2 to 2.20
  - upgraded ttpmacro.exe version information.

2006.3.3 (Ver 4.29)
  - added the background color of text matches the background color of screen. And added "Always use Normal text's BG" checkbox on Window setup dialog. And also added UseNormalBGColor entry in teraterm.ini file. Special thanks to 337.
  - Bug fix: don't work the font copying batch file on Windows9x. Special thanks to Takashi SHIRAI.

2006.2.24 (Ver 4.28)
  - changed the compiler to Visual Studio 2005 Standard Edition.
  - replaced stricmp() to _stricmp().
  - replaced strnicmp() to _strnicmp().
  - replaced strdup() to _strdup().
  - replaced chdir() to _chdir().
  - replaced getcwd() to _getcwd().
  - replaced strupr() to _strupr().
  - deleted compiler warning message for time_t has 64bit size.
  - added the color change on Window setup dialog was in real time done.
  - limited the macro commands displaying on macro window for the speed-up of the macro operation.
  - upgraded Oniguruma to 4.0.1
  - upgraded TTSSH version supporting SSH2 to 2.19
  - upgraded TeraTerm Menu to 1.0.4
     * fixed build error with VS2005.
     * updated version information in .exe file.

2006.2.11 (Ver 4.27)
  - expanded the number of macro variable from 128 to 256 and the number of macro label from 256 to 512.
  - added the workaround about CPU load goes up to 100% using 'pause' macro on TEK active window.
  - deleted 'TeraTerm Menu' under File menu.
  - fixed the flicker phenomenon of TeraTerm window when 'Broadcast Command' is used. 
  - changed the accelerator key bind under Setup menu.
  - added 'mpause' macro command. This command enables the macro script to pause the specified time in millisecond. 
  - added the sample TTL macro 'mpause.ttl'.
  - added 'random' macro command. This command enables the macro script to generate the random value.
  - added the sample TTL macro 'random.ttl'.
  - upgraded CygTerm to 1.06_2. Special thanks to BabyDaemon.
        * Generated CygTerm's ICON by "DotWork 2.50".
          "DotWork 2.50" is available at http://www5a.biglobe.ne.jp/~suuta
          Special Thanks to "suuta at hamal dot freemal dot ne dot jp"
        * Makefile was changed, "clean" target was changed to don't remove cygterm.ico.
        * The following configuration file reading was supported:
            + /etc/cygterm.conf
            + ~/.cygtermrc
        * Shell is invoked with user's home-directory.
        * Get account configuration from /etc/passwd by getpwnam(3),
          getting account name from USERNAME environment by getenv(3).
        * Makefile was changed, default build is with icon executable.

2006.1.21 (Ver 4.26)
  - added the user confirmation dialog before sending file content with Drag and Drop to TeraTerm window.
  - upgraded LogMeTT to 2.8.1
  - upgraded Oniguruma to 3.9.1

2005.12.13 (Ver 4.25)
  - Bug fix: can't save TeraTerm configuration to ini file without 'teraterm.ini' filename.

2005.12.1 (Ver 4.24)
  - expanded serial connection maximum to COM99.
  - added new serial baud rate(230400, 460800, 921600).
  - upgraded LogMeTT to 2.7.4

2005.11.3 (Ver 4.23)
  - added checking the writable permission if saving teraterm.ini file.
  - changed "Term type" edit box on TCP/IP setup dialog to be always enabled.

2005.10.21 (Ver 4.22)
  - added the system variable from 'groupmatchstr1' to 'groupmatchstr9' which will store the group matched pattern with regular expression.
  - added duplicating Cygwin connection using Alt + D accelerator key.
  - Bug fix: DAKUTEN(the dot marks) might not be correctly shown in UTF-8 for Mac OS X.
  - Bug fix: TeraTerm Menu popup menu couldn't be shown at right position using multi monitor system. upgraded TeraTerm Menu to 1.03.
  - upgraded LogMeTT to 2.7.2
  - upgraded TTSSH version supporting SSH2 to 2.18

2005.10.7 (Ver 4.21)
  - expanded scroll range width from 16bit to 32bit.
  - added 'waitregex' macro command. This command enables the macro script to wait the specified keyword with regular expression. 
  - added the system variable 'matchstr' which will store the matched pattern with regular expression.
  - added the sample TTL macro 'wait_regex.ttl'.
  - added regular expression library 'Oniguruma' version information on TeraTerm version dialog.

2005.9.5 (Ver 4.20)
  - upgraded TTSSH version supporting SSH2 to 2.17

2005.8.27 (Ver 4.19)
  - Buf fix: division result is invalid in TeraTerm macro script. Special thanks to Takashi SHIRAI.
  - changed not downing ttpmacro.exe process priority with logging
  - upgraded LogMeTT to 2.6.1

2005.7.24 (Ver 4.18)
  - added showing the line number and data of TTL file on macro dialog.
  - Bug fix: DAKUTEN(the dot marks) might not be correctly shown in UTF-8 for Mac OS X.
  - upgraded LogMeTT to 2.5.6

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
  - added clickable URL mechanism that mouse pointer overs on URL string. Special thanks to monkey magic. And also added URLColor, EnableClickableUrl entry in teraterm.ini file.
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


==============================================================================
■UTF-8対応TeraTerm Pro             【利用手引き】
==============================================================================

●はじめに
  本プログラムはターミナルソフト「TeraTerm Pro」をUTF-8 & SSH2対応させたものです。


●インストール方法
  インストーラを利用する場合は、インストーラの指示に従ってインストールしてください。
  
  
●Unicode設定
  UTF-8化を行うには、TeraTerm Proの「Setup」メニューから「Terminal」を選択すると、
  ダイアログが出てくるので、「Kanji(receive)」および「Kanji(transmit)」にて、
  【UTF-8】を選んでください。
  TeraTerm Proの再起動は必要ありません。
  コマンドラインの「/KT」および「/KR」オプションにおいて、"UTF8"を指定すると、
  送信および受信コードにUTF-8を設定することができます。
  
  日本語以外の言語を表示できるようにするためには、teraterm.iniファイルにロケール
  およびコードページの設定が必要となります。下記にサンプルを示します。
    
------------------------------------------------------------------------------------
; Locale for Unicode
Locale = japanese

; CodePage for Unicode
CodePage = 932
------------------------------------------------------------------------------------

  ロケールおよびコードページに設定できる内容については、下記のサイトを参照してください。
  
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore98/html/_crt_language_strings.asp
    
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_81rn.asp

  [Example of WindowsXP Simplified Chinese]
  ------------------------------------------------------------------------------------
    ; Locale for Unicode 
    Locale = chs 
    
    ; CodePage for Unicode 
    CodePage = 936   
  ------------------------------------------------------------------------------------

  ※注意：Mac OS X
    【UTF-8m】はMac OS X(HFS+)向けのエンコーディング指定です。受信のみのサポートです。
    「/KR」コマンドラインでは"UTF8m"を指定することができます。

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


●ANSIカラー色変更＆行連結パッチ
  本プログラムは「ANSIカラー色変更＆行連結パッチ」を取り込んでいます。
  teraterm.iniファイルの[Tera Term]セクションに、下記の記述を追加することにより、
  当該機能を有効にすることができます。

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

  ※ヒント
    TermTypeを"xterm"に設定することにより、ANSIカラーに対応しているvim6などで
    カラー表示をさせることができます。これはオリジナルからある機能です。


●ホイールマウス
  オリジナルのTeraTermではホイールマウスが効きませんが、本プログラムではホイールボタンを
  使ってスクロールバーを上下させることができます。


●マウスカーソル
  マウスカーソルの種別を設定することができます。
  teraterm.iniファイルの[Tera Term]セクションに、下記の記述を追加することにより、
  当該機能を有効にすることができます。

------------------------------------------------------------------------------------
; Mouse cursor type (arrow/ibeam/cross/hand)
MouseCursor = hand
------------------------------------------------------------------------------------

    arrow = 標準の矢印カーソル
    ibeam = アイビーム（縦線）カーソル（デフォルト）
    cross = 十字カーソル
    hand  = ハンドカーソル


●半透明ウィンドウ
  半透明ウィンドウの設定をすることができます。デフォルトは255（不透明）です。
  teraterm.iniファイルの[Tera Term]セクションに、下記の記述を追加することにより、
  当該機能を有効にすることができます（Windows2000以降）。

------------------------------------------------------------------------------------
; Translucent window (0 - 255)
AlphaBlend = 230
------------------------------------------------------------------------------------


●Cygwin接続
  ローカルのCygwinへ接続することができます。内部でCygtermを利用しています。
  Fileメニューから「Cygwin connection」を選択する、もしくはAlt+Gを押下すると、
  Cygwinへ接続します。
  あらかじめ、teraterm.iniファイルの[Tera Term]セクションに、Cygwinの
  インストールパスを設定しておく必要があります。
  フォーマットは「ドライブ名：￥ディレクトリ」です。

------------------------------------------------------------------------------------
; Cygwin install path
CygwinDirectory = c:\cygwin
------------------------------------------------------------------------------------

  ※注意：
    cygtermがうまく起動しない場合は、cygtermをビルドし直してください。



●バッファの全選択
  Editメニューから「Select all」を選択することにより、バッファの全選択を
  行うことができます。

  ※ヒント
    Altキーを押しながらマウスをドラッグすると「ブロック選択」ができるように
    なります。これはオリジナルからある機能です。


●ログファイルへのコメント追加
  Fileメニューから"Comment to Log..."を選択すると、ログファイルへコメントを追加
  することができます。追加できるコメントは最大255文字までです。



●設定ダイアログ
  Editメニューから"Additional settings"を選択すると、UTF-8化以降に追加した機能に関する
  オプション設定が行えます。


●ラインバッファ
  ラインバッファを 10000 行を超える設定をしたい場合、teraterm.ini ファイルの
  MaxBuffSize エントリも合わせて設定をする必要があります。以下に例を示します。

------------------------------------------------------------------------------------
; Max scroll buffer size
MaxBuffSize=500000
------------------------------------------------------------------------------------


●SSHの利用
  本プログラムでは SSH2対応TTSSH を利用することにより、SSH(Secure Shell)を利用することが
  できます。TTSSHオリジナルや日本語版TTSSHとの組み合わせでは動作しませんので、ご注意くだ
  さい。
  
  SSHに関するコマンドラインオプションは以下のものがあります。
  
    オプション    意味
    /1            SSH1で接続する
    /2            SSH2で接続する（サーバがSSH2をサポートしていない場合はSSH1接続になる）。

  以下にコマンドラインオプションの例を示します（IPv4 & SSH2 & UTF-8で接続）。

    ttermpro.exe /KR=utf8 /4 192.168.1.3:22 /ssh /2


●マクロリファレンス: connect
  TeraTermマクロから接続する方法には、3つの種類があります。
  
  - telnet接続 (port 23) 
  - SSH1 および SSH2 接続 (port 22) 
  - COM port 接続
  
  ・telnet接続
  
  connect 'myserver:23 /nossh'
  or
  connect 'myserver:23 /telnet'
  or
  connect 'myserver:23'
  
  /nossh を使う方を強く推奨します。このオプションがない場合、TeraTermは teraterm.ini が最後に保存されたときと同じメソッド（telnet もしくは SSH）を使って、接続しようとします。もし、SSHを使って接続しようとするならば、connectマクロは失敗するでしょう。
  
  
  ・SSH接続
  
  connect 'myserver /ssh'
  
  この方法は、SSHバージョンを指定せずにSSH接続をします。
  
  connect 'myserver /ssh /1'
  or
  connect 'myserver /ssh /2'
  
  /1 と /2 のオプションは、SSH1およびSSH2であることを指定します。
  
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
  
  この6つの方法は、マクロからユーザ名とパスワードを渡すことにより、SSH認証ダイアログをスキップさせることができます。/auth=passwordはパスワード認証、/auth=publickeyは公開鍵認証であることを表します。
  注意: パスワードにはスペースを含むことが可能です。パスワードの中でスペースを表すには、"@"で置き換えてください。もし"@"がパスワードの一部である場合は、連続した"@" つまり"@@"で置き換えてください。
  TeraTermマクロにユーザ名とパスワードを指定した場合、そのマクロファイルをプレーンテキストとして保存したのなら、セキュリティ的に安全な場所に置く必要があることを肝に銘じておいてください。
  
  connect 'myserver /ssh /auth=password /user=username /ask4passwd'
  or
  connect 'myserver /ssh /1 /auth=password /user=username /ask4passwd'
  or
  connect 'myserver /ssh /2 /auth=password /user=username /ask4passwd'
  or
  connect 'myserver /ssh /auth=publickey /user=username /ask4passwd /keyfile=private-key-file'
  or
  connect 'myserver /ssh /1 /auth=publickey /user=username /ask4passwd /keyfile=private-key-file'
  or
  connect 'myserver /ssh /2 /auth=publickey /user=username /ask4passwd /keyfile=private-key-file'
  
  この6つの方法は、マクロからユーザ名と認証方法を渡して、ポップアップでパスワードの入力を促します。
  
  
  ・COMポート接続
  
  connect '/C=x' 
  
  xはCOMポート番号を表します。たとえば、COM1に接続したいのなら、 connect '/C=1' とします。
  
  source URL: http://www.neocom.ca/forum/viewtopic.php?t=28



●マクロリファレンス: waitregex

形式：
       waitregex <string1 with regular expression> <string2 with regular expression> ...

       使用できる正規表現： RE-ja.txt

正規表現文字列 <string1 with regular expression>, <string2 with regular expression>, ...  のうち1つ以上を含む行（最大256文字）をホストから受信するか、タイムアウトが発生するまで MACRO を停止させる。正規表現文字列は最大10個まで指定できる。

システム変数 timeout が0より大きい場合、<timeout> 秒の時間がすぎるとタイムアウトが発生する。 timeout の値が0以下の場合は、タイムアウトは発生しない。

このコマンドの実行結果はシステム変数 result , inputstr, matchstr に格納される。

変数 result の値の意味は以下のとおり。
値		意味
0		タイムアウト。どの文字列も来なかった。
1		<string1 with regular expression> を受信した。
2		<string2 with regular expression> を受信した。
.		.
.		.

変数 inputstr は受信した行が格納される。
変数 matchstr は最初にマッチしたパターンが格納される。
変数 groupmatchstr1～groupmatchstr9 にはグループマッチしたパターンが格納される。


例:
timeout = 30
waitregex 'Longhorn' '.*@sai' 'Pentium\d+' 'TeraTermX{3}'
int2str s result
messagebox s 'result'
messagebox inputstr 'inputstr'
messagebox matchstr 'matchstr'



●マクロリファレンス: mpause

形式:
      mpause <time>

TTL の実行を <time> ミリ秒だけ休止する。

例:
mpause 100               100ミリ秒休止



●マクロリファレンス: random

形式:
      random <intvar> <value>

0から<value>までの整数の一様乱数を生成し、整数変数<intvar>へ代入する。

例:
random val 100           0から100までの乱数を変数valへ格納する



●マクロリファレンス: clipb2var

形式:
      clipb2var <strvar>

クリップボードのテキストデータを<strvar>へ代入する。

このコマンドの実行結果はシステム変数 result に格納される。

変数 result の値の意味は以下のとおり。
値		意味
0		クリップボードを開けなかった。またはテキストデータではなかった。
1		<strvar>への代入に成功した。



●マクロリファレンス: var2clipb

形式:
      var2clipb <string>

<strvar>をクリップボードにコピーする。

このコマンドの実行結果はシステム変数 result に格納される。

変数 result の値の意味は以下のとおり。
値		意味
0		クリップボードを開けなかった。
1		クリップボードへのコピーに成功した。



●マクロリファレンス: ifdefined

形式:
      ifdefined <var>

<var>の形式を表す値をシステム変数 result に格納する。

変数 result の値の意味は以下のとおり。
値		意味
0		定義されていない
1		整数型
3		文字列型
4		ラベル



■透過ウィンドウ
  TeraTermはウィンドウの半透明をサポートしています。現在、半透明ウィンドウは2種類のタイプがあり、ユーザは好きな方を選択することができます。
  
  (1) 半透明ウィンドウ
  この機能を使うと、TeraTermのウィンドウを半透明にすることができます。透明度は0～255の範囲で指定することができ、teraterm.iniのAlphaBlendエントリで設定します。値を小さくするほど、透明度は強くなります。以下に設定例を示します。
  
	; Translucent window (0 - 255)
	AlphaBlend =255
  
  (2) Eterm風半透明ウィンドウ
  この機能を使うと、Eterm風の半透明ウィンドウを使用することができます。この機能を有効にすると、TeraTermのウィンドウは半透明化され、壁紙と指定した画像ファイル(*.jpg)と混合されて描画されるようになります。この機能を有効にするには、teraterm.iniの[BG]セクションのBGEnableエントリで設定します。

	[BG]
	; Use Background extension (On / Off)
	BGEnable = Off

  そのほかの設定については以下に示します。

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



■キーボード＆マウス操作

  Ctrl + Tab, Ctrl + Shift + Tab
    TeraTermウィンドウの切り替え
  
  Alt + Enter
    ウィンドウの最大化（トグル）
  
  Alt + Space  Alt + X
    ウィンドウの最大化
  
  Alt + N
    新しい接続を開く
    
  Alt + D
    セッションの複製（SSH, telnetのみ）
    
  Alt + G
    Cygwin接続
  
  Ctrl + 上下カーソル
    上下方向へ一行スクロール

  Ctrl + PageUp/PageDown
    上下方向へページ単位スクロール

  Ctrl + Home
    バッファの先頭へ移動

  Ctrl + End
    バッファの末尾へ移動

  Alt + C, Ctrl + Insert, マウスドラッグ
    選択文字列をクリップボードへコピーする

  Alt + V, Shift + Insert, マウスホイールボタン・右クリック
    クリップボードからペーストする
  
  F1キー
    Num pad NumLockキー押下

  F2キー
    Num pad '/'キー押下
    
  F3キー
    Num pad '*' キー押下

  F4キー
    Num pad '-' キー押下

  Alt + マウスドラッグ
    ブロック選択
    
  左クリック  Shift+左クリック
    ページをまたぐテキスト選択、選択済みテキスト領域の伸縮
    
  マウスホイールボタンスクロール
    上下方向へ一行スクロール

  ファイルをウィンドウへドラッグ
    ファイルの内容を貼り付ける
    
  フォルダをウィンドウへドラッグ
    フォルダのフルパスを貼り付ける


■マクロエラーメッセージ

エラーメッセージ           意味
-------------------------+------------------------------------------------------
Too many labels.           ラベルの数が多すぎる。(最大512個)
Too many variables.        変数の数が多すぎる。(整数型、文字列型それぞれ最大256個ずつ)


■ライセンス
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


■著作権
  本プログラムはフリーソフトウェアです。BSDライセンスのもとに配布されています。
  
  TeraTerm Proは寺西高氏の著作物です。
  ANSIカラー色変更＆行連結パッチは多賀奈由太氏に著作権があります。
  UTF-8化およびSSH2化のコードについては、著作権は平田豊にあります。
  IPv6版TeraTerm Proは加藤淳也氏の著作物です。
  CygtermはNSym氏の著作物です。
  TTSSHはRobert O'Callahan氏の著作物です。
  TeraTerm Menuは早川信平氏の著作物です。
  LogMeTTはNeoCom Solutionsの著作物です。
  Eterm風透過ウィンドウ機能はAKASI氏に著作権があります。
  TTProxyは蛭子屋双六氏の著作物です。
  日本語版TeraTermは高橋靖英氏および橋井健人氏に著作権があります。
  クリッカブルURLパッチはmonkey magic氏に著作権があります。
  行末スクロール抑止パッチはいわもとこういち氏に著作権があります。
  マルチディスプレイ対応パッチは安藤弦彦氏に著作権があります。
  鬼車はK.Kosako氏の著作物です。
  TeraTermおよびTTSSHに関するパッチの一部は永田真也に著作権があります。
  TeraTermおよびTTSSHのアイコンは坂本龍彦氏に著作権があります
  HTMLヘルプの一部は市村光康氏、桑田政輝氏、橋井健人氏、平田豊に著作権があります
  本プログラムは商用利用問わず無償で利用できますが、作者は本プログラムの使用にあたり
  生じる障害や問題に対して一切の責任を負いません。

  URL: http://ttssh2.sourceforge.jp/
  Project: http://sourceforge.jp/projects/ttssh2/
  e-mail: yutakakn@gmail.com (平田豊)
  forum(英語): http://www.neocom.ca/forum/index.php
  ML: http://www.freeml.com/info/teraterm@freeml.com

  TeraTerm Pro(オリジナル): http://hp.vector.co.jp/authors/VA002416/
  ANSIカラー色変更＆行連結パッチ: http://www.sakurachan.org/ml/teraterm/msg00264.html
  IPv6版TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  Cygterm: http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index.html
  TeraTerm Menu: http://park11.wakwak.com/~shinpei/freesoft/index.html
  LogMeTT: http://www.neocom.ca/freeware/LogMeIn/
  Eterm風透過ウィンドウ機能パッチ：http://www.geocities.co.jp/SiliconValley-PaloAlto/4954/av.html
  TTProxy: http://yebisuya.dip.jp/Software/TTProxy/
  Oniguruma: http://www.geocities.jp/kosako3/oniguruma/

  Special thanks to ... (敬称略)
  ・寺西高 (T.Teranishi) - オリジナルの TeraTerm Pro の作者
    URL: http://hp.vector.co.jp/authors/VA002416/
  ・平田豊 (Yutaka Hirata) - UTF-8対応 & SSH2対応
    e-mail: yutakakn@gmail.com
    URL: http://hp.vector.co.jp/authors/VA013320/


■注意事項
  ・UTF-8からShift_JISへ変換できない文字は疑問符表示となります。
  ・UTF-8環境でのEUC出力が画面に反映されないことがあります。


■改版履歴

2006.10.13 (Ver 4.47)
  ・LogMeTT 2.8.6へ差し替えた
  ・Fileメニューのアクセラレータキーを重複しないようにした。
  ・同期モードでのマクロ実行において、DDEバッファがフルになった場合にCPU使用率が100%となる現象を回避するようにした。
  ・sendln後の無条件100ミリ秒のスリープを解除した。

2006.10.9 (Ver 4.46)
  ・Windows Vista で teraterm.ini の保存先のデフォルトがデスクトップになるのを修正した。
  ・マクロコマンド 'clipb2var' を追加した。クリップボードからデータのコピーが可能となる。
  ・マクロコマンド 'var2clipb' を追加した。クリップボードへのデータのコピーが可能となる。
  ・最初のウインドウで、接続ダイアログの /L= パラメータが使用されないバグを修正した。
  ・マクロコマンド 'ifdefined' を追加した。このコマンドは変数の型を返します。
  ・HTMLヘルプを更新した
  ・Oniguruma 4.4.4へ差し替えた
  ・CygTerm+ 1.06_11へ差し替えた

2006.9.16 (Ver 4.45)
  ・HTMLヘルプを更新した
  ・SSH2対応TTSSH(2.33)へ差し替えた
  ・CygTerm+ 1.06_8へ差し替えた
  ・Oniguruma 4.4.1へ差し替えた
  ・デフォルトのログファイル名を指定できるようにした。
        * エディットコントロールを "Additional settings" ダイアログに追加した。
        * teraterm.ini ファイルに LogDefaultName エントリを追加した。
        * ファイル名に strftime のフォーマットを使えるようにした。
          書式:
              %a  曜日の省略形。
              %A  曜日の正式名。
              %b  月の省略形。
              %B  月の正式名。
              %c  ロケールに対応する日付と時刻の表現。
              %d  10 進数で表す月の日付 (01 ～ 31)。
              %H  24 時間表記の時間 (00 ～ 23)。
              %I  12 時間表記の時間 (01 ～ 12)。
              %j  10 進数で表す年初からの日数 (001 ～ 366)。
              %m  10 進数で表す月 (01 ～ 12)。
              %M  10 進数で表す分 (00 ～ 59)。
              %p  現在のロケールの午前/午後。
              %S  10 進数で表す秒 (00 ～ 59)。
              %U  10 進数で表す週の通し番号。日曜日を週の最初の日とする (00 ～ 53)。
              %w  10 進数で表す曜日 (0 ～ 6、日曜日が 0)。
              %W  10 進数で表す週の通し番号。月曜日を週の最初の日とする (00 ～ 53)。
              %x  現在のロケールの日付表現。
              %X  現在のロケールの時刻表現。
              %y  10 進数で表す西暦の下 2 桁 (00 ～ 99)。
              %Y  10 進数で表す 4 桁の西暦。
              %z, %Z  レジストリの設定に応じて、タイム ゾーンの名前または省略形を指定します。タイム ゾーンが不明な場合は指定しません。
              %%  パーセント記号。
  ・/M コマンドラインパラメータが指定されている場合、TeraTerm 起動時に自動的にシリアルポートへ接続しないようにした。

2006.8.19 (Ver 4.44)
  ・Windows98/NT4.0 において ttpmacro.exe が起動直後に終了してしまう問題を修正した。
  ・ttermpro.exe, ttpmacro.exe, ttpmenu.exe, keycode.exe, cygterm.exeに mutex を追加した。
  ・SSH2対応TTSSH(2.32)へ差し替えた
  ・インストーラを InnoSetup へ変更した
  ・HTMLヘルプを更新した
  ・CygTerm+ 1.06_6へ差し替えた。
        * -sと-tオプションのパラメータの長さを制限した。
        * インストーラから起動チェックをするため、mutex を作成した。
        * シェルをログインシェルとして呼び出すオプション LOGIN_SHELL を追加した。
          LOGIN_SHELL: yes | no
        * ホームディレクトリで起動するのを選択するオプション HOME_CHDIR を追加した。
          HOME_CHDIR: yes | no

2006.8.4 (Ver 4.43)
  ・いくつかの変数がシステム変数に使われていたので、ユーザが使えるマクロ変数が256個になるようにした。
  ・マクロの inputbox のダイアログを、メッセージの長さに合わせて広げるようにした。
  ・マクロのダイアログボックスに渡される "\n" が改行とみなされるようにした。
  ・マクロの messagebox のダイアログサイズを変更できるようにした
  ・接続先ホストが 'telnet://' で始まってポート番号の指定がない場合、ポート番号 23 を設定するようにした。
  ・TTProxyを2006/8/3版へ差し替えた。蛭子屋氏に感謝します。
  ・SSH2対応TTSSH(2.31)へ差し替えた

2006.7.26 (Ver 4.42)
  ・TeraTermアイコン(teraterm.ico, vt.ico)を新しくした。坂本龍彦氏に感謝します。
  ・LogMeTT 2.8.5へ差し替えた
  ・Collectorのリソース英文漏れの修正を行った
  ・SSH2対応TTSSH(2.30)へ差し替えた

2006.7.24 (Ver 4.41)
  ・ログに時刻を記録する機能を追加した。また、teraterm.iniに LogTimestamp エントリを追加した。
  ・上書きインストール時に、前回のインストールフォルダがデフォルトで指定されるようにした。
  ・Collectorのリソース英文漏れの修正を行った
  ・Oniguruma 4.2.0へ差し替えた

2006.7.19 (Ver 4.40)
  ・LogMeTT 2.8.4へ差し替えた
  ・HTMLヘルプを更新した
  ・ttermpro.exeとttpmacro.exeのバージョン情報を更新した

2006.7.1 (Ver 4.39)
  ・UTF-8の2バイト文字の直後に1バイト文字(ASCII)が来る場合、文字化けとなっていたバグを修正した（例：´)）。
  ・Copyright表記を変更した

2006.6.23 (Ver 4.38)
  ・SSH2対応TTSSH(2.28)へ差し替えた
  ・HTMLヘルプを更新した

2006.6.18 (Ver 4.37)
  ・タイトルに日本語を入力すると、文字化けするバグを修正した。パッチ作成に感謝します＞永田氏
  ・SSH2対応TTSSH(2.27)へ差し替えた
  ・Collectorのリソース英文漏れの修正を行った

2006.6.11 (Ver 4.36)
  ・SSH2対応TTSSH(2.26)へ差し替えた

2006.5.25 (Ver 4.35b)
  ・タブ化ソフト「Collector 3.2.3」を英語化し、同梱した。収録許可をいただいた高橋弘氏に感謝します。

2006.5.7 (Ver 4.35a)
  ・SSH2対応TTSSH(2.25)へ差し替えた

2006.4.7 (Ver 4.35)
  ・HTMLヘルプファイルのオーナーをデスクトップへ変更した
  ・SSH2対応TTSSH(2.24)へ差し替えた

2006.4.4 (Ver 4.34)
  ・SSH2対応TTSSH(2.23)へ差し替えた

2006.4.1 (Ver 4.33)
  ・半透明化を行わない場合に画面がちらつかないようにした。

2006.3.30 (Ver 4.32)
  ・SSH2対応TTSSH(2.22)へ差し替えた

2006.3.28 (Ver 4.31)
  ・ヘルプファイルを .hlp から .chm へ変更した
  ・Additional settingsダイアログにおけるウィンドウの半透明変更を即座に反映させるようにした。
  ・文字の背景色をスクリーンの背景色と一致させるパッチのバグを修正した。パッチ作成に感謝します＞337氏
  ・VisualStyleが有効となるようにした
  ・VT windowでの1ミリ秒スリープを削除した
  ・SSH2対応TTSSH(2.21)へ差し替えた
  ・LogMeTT 2.8.3へ差し替えた

2006.3.6 (Ver 4.30)
  ・SSH2対応TTSSH(2.20)へ差し替えた
  ・ttpmacro.exeのバージョン情報を更新した

2006.3.3 (Ver 4.29)
  ・文字の背景色をスクリーンの背景色と一致させるようにした。それにともないWindow setupダイアログに"Always use Normal text's BG"チェックボックスを追加した。また、teraterm.iniにUseNormalBGColorエントリを追加した。パッチ作成に感謝します＞337氏
  ・Windows9x上においてインストーラから起動されるバッチファイルがうまく動かないバグを修正した。白井隆氏に感謝します。

2006.2.24 (Ver 4.28)
  ・コンパイラを Visual Studio 2005 Standard Edition に切り替えた。
  ・stricmp()を_stricmp()へ置換した
  ・strnicmp()を_strnicmp()へ置換した
  ・strdup()を_strdup()へ置換した
  ・chdir()を_chdir()へ置換した
  ・getcwd()を_getcwd()へ置換した
  ・strupr()を_strupr()へ置換した
  ・time_tの64ビット化にともなう警告メッセージを抑止した
  ・Window setupでのカラー変更がリアルタイムに行われるようにした。
  ・マクロ動作の高速化のため、マクロウィンドウへのコマンド表示をウェイト系に限定した。
  ・Oniguruma 4.0.1へ差し替えた
  ・SSH2対応TTSSH(2.19)へ差し替えた
  ・TeraTerm Menu 1.04へ差し替えた
     * VS2005でビルドエラーとなる問題を修正した
     * EXEファイルのバージョン情報を更新した

2006.2.11 (Ver 4.27)
  ・マクロで使用できる変数の個数を128から256へ、ラベルの個数を256から512へ拡張した。
  ・TEK windowをアクティブにした状態で pause マクロを使用すると、CPU使用率が100%となる現象への暫定対処を施した(workaround)。
  ・Fileメニューから TeraTerm Menu を削除した。
  ・Broadcast Command使用時にウィンドウがちらつく動作を改善した。
  ・Setupメニューのアクセラレータキーを変更した。
  ・マクロコマンド 'mpause' を追加した。ミリ秒単位での休止が可能となる。
  ・サンプルマクロ mpause.ttl を追加した。
  ・マクロコマンド 'random' を追加した。乱数の生成が可能となる。
  ・サンプルマクロ random.ttl を追加した。
  ・CygTerm 1.06_2へ差し替えた。BabyDaemon氏に感謝します。
        * CygTermのアイコンが作成されました。（"DotWork 2.50"にて作成）
          "DotWork 2.50" は右記のURLより入手可能です。 http://www5a.biglobe.ne.jp/~suuta
          すばらしいソフトウェアを公開していただき、
          "suuta at hamal dot freemal dot ne dot jp"に感謝致します。
        * Makefileが修正されました。"clean"ターゲットでcygterm.icoを削除しないように。
        * 以下の設定ファイルの読み込みをサポート:
            + /etc/cygterm.conf
            + ~/.cygtermrc
        * シェルがユーザのホームディレクトリで起動されるように。
        * アカウントの設定情報を/etc/passwdからgetpwnam(3)で読み込むように。
          アカウント名は環境変数USERNAMEからgetenv(3)で取得してます。
        * Makefile修正：ディフォルトでアイコンつきの.exeをビルドします。

2006.1.21 (Ver 4.26)
  ・ウィンドウへのドラッグ＆ドロップでファイル送信する前に、問い合わせダイアログを出すようにした。
  ・LogMeTT 2.8.1へ差し替えた
  ・Oniguruma 3.9.1へ差し替えた

2005.12.13 (Ver 4.25)
  ・設定内容がteraterm.ini以外のファイル名で保存できないバグを修正した。

2005.12.1 (Ver 4.24)
  ・シリアル接続のCOM最大ポートを99まで拡張した。
  ・シリアル接続のボーレートに230400, 460800, 921600を追加した。
  ・LogMeTT 2.7.4へ差し替えた

2005.11.3 (Ver 4.23)
  ・teraterm.iniを保存するときに書き込みできるかどうかの判別を追加した。
  ・TCP/IP setupダイアログの"Term type"を常に有効とするようにした。

2005.10.21 (Ver 4.22)
  ・システム変数 'groupmatchstr1'～'groupmatchstr9' を追加した。正規表現でグループマッチしたパターンが格納される。
  ・Cygwin接続の複製ができるようにした
  ・Mac OS X向けUTF-8において、濁点が正しく表示されないことがある問題を修正した。
  ・TeraTerm Menuをマルチモニタ環境で使用すると、ポップアップメニューが正しいモニタに表示されないバグを修正した（TeraTerm Menu 1.03）。
  ・LogMeTT 2.7.2へ差し替えた
  ・SSH2対応TTSSH(2.18)へ差し替えた

2005.10.7 (Ver 4.21)
  ・スクロールレンジを 16bit から 32bit へ拡張した
  ・マクロコマンド 'waitregex' を追加した。正規表現を使った文字列の wait が可能となる。
  ・システム変数 'matchstr' を追加した。正規表現でマッチしたパターンが格納される。
  ・サンプルマクロ wait_regex.ttl を追加した。
  ・正規表現ライブラリ Oniguruma のバージョンをバージョン情報に追加した

2005.9.5 (Ver 4.20)
  ・SSH2対応TTSSH(2.17)へ差し替え

2005.8.27 (Ver 4.19)
  ・macroの除算結果が不正となるバグを修正した。白井隆氏に感謝します。
  ・ttpmacro.exeのプロセス優先度は、ログ採取中も下げないようにした。
  ・LogMeTT 2.6.1へ差し替えた

2005.7.24 (Ver 4.18)
  ・マクロ実行中のダイアログに行番号と内容を表示するようにした。
  ・Mac OS X向けUTF-8において、濁点が正しく表示されないことがある問題を修正した。
  ・LogMeTT 2.5.6へ差し替えた

2005.5.29 (Ver 4.17)
  ・<ESC>[J(Clear screen from cursor-position up to end)処理時において、カーソルが左上隅を指している場合にのみ、カレントバッファをスクロールアウトさせるようにした。

2005.5.28 (Ver 4.16)
  ・ログ種別のplain textの情報をteraterm.iniに LogTypePlainText として残すようにした。
  ・CygTermの設定を Additional settings ダイアログに追加した。
  ・ブロック選択のキーバインドを Shift+MouseDrag から Alt+MouseDrag へ変更した。
  ・ページをまたぐテキスト選択（選択済みテキスト領域の伸縮）をサポートした。選択範囲の開始位置は左クリック、終了位置はShift + 左クリックで指定する。
  ・<ESC>[Jによる画面クリア時にカレントバッファをスクロールアウトさせるようにした。
  ・マクロ使用時にCPU使用率が100%となる現象への暫定処置を施した（sendln後に無条件に100ミリ秒のスリープを追加）。

2005.4.24 (Ver 4.15)
  ・ALT + Enterキー（トグル）押下でウィンドウを最大化できるようにした。
  ・Eterm lookfeel alphablendの設定内容を teraterm.ini へ保存するようにした。
  ・Eterm lookfeel alphablendのon/offを Additional settings ダイアログ上から設定できるようにした。
  ・Windows98においてフォントがインストールされない問題を修正した。Boris氏に感謝します。
  ・ライセンス文を追加した
  ・マクロヘルプファイル（英語版）を更新した。Boris氏に感謝します。
  ・LogMeTT 2.4.4へ差し替えた

2005.4.9 (Ver 4.14)
  ・URL文字列をダブルクリックするとブラウザが起動するしかけを追加した。monkey magic氏に感謝します。teraterm.iniにURLColor, EnableClickableUrlを追加。
  ・エスケープシーケンス（ESC[39m:foreground color reset, ESC[49m:background color reset）をサポートした。これによりscreen上でw3mを使用した場合、色が戻らない現象が改善される。岩本氏に感謝します。
  ・Additional settingsでのマウスカーソル種別を設定時に変更されるようにした。
  ・バージョン情報ダイアログのURLをダブルクリックすると、ブラウザが起動されるようにした。
  ・制限事項「SSHに対する"Duplicate session"では常にSSH2での接続を試みる」を解除した。
  ・"Duplicate session"においてSSH自動ログインを行うようにした。
  ・LogMeTT 2.4.3へ差し替えた

2005.3.31 (Ver 4.13)
  ・2つめ以降のTeraTermプロセスにおいて、バージョン情報が正しく取得されない問題へ対処した。
  ・マウスの右ボタン押下でのペーストを禁止する設定を teraterm.ini(DisablePasteMouseRButton) および Additional settings に追加した。
  ・ZMODEMの送信において、ファイルサイズが20MBを超えるとクラッシュする問題へ対処した。Thorsten Behrens氏に感謝します。
  ・LogMeTT 2.4.2へ差し替え。

2005.3.13 (Ver 4.12)
  ・ttermpro.exeのバージョン情報が古いままだったので修正した。
  ・全角文字（日本語）が行末をまたぐ場合、上方へスクロールしてしまう問題へ対処した（岩本氏パッチ）。
  ・タイトルバーに日本語を設定する場合、Shift_JIS(CP932)へ変換するようにした（EUC-JPのみに対応）。
  ・LogMeTT 2.1.2へ差し替え。

2005.2.22 (Ver 4.11)
  ・ログ採取中に File -> log がグレイ表示にならない問題へ対処した。
  ・Eterm風透過ウィンドウ機能を追加した。AKASI氏に感謝します。
  ・VisualStyle(WindowsXP)に対応させた
  ・TitleFormatにformat ID=13を新規追加した
  ・キャプション表示において、COM5以上の表示に対応した。
  ・ログファイルの種別に"plain text"を追加した。当該オプションを有効にすると、ログファイルにASCII非表示文字を採取しないようになる。ただし、例外キャラクタ(HT, CR, LF)は採取対象。
  ・TTProxy(2004/1/22版)の新規追加。蛭子屋双六氏に感謝します。
  ・LogMeTT 2.02βの新規追加。Boris氏に感謝します。
  ・LogMeInを削除した
  ・Additional settingsをtab control化した
  ・ログ採取時のデフォルト名を"teraterm.log"に変更した

2005.1.30 (Ver 4.10)
  ・最大化ボタンを有効にした
  ・TeraTerm Menu, LogMeInのアクセラレータキーを削除した
  ・ログ採取時のデフォルト名(log_YYYYMMDD_HHMMSS.txt)を設定するようにした
  ・Broadcast commandメニューをControl menu配下に追加した。
  ・バージョンダイアログにホームページのURLを追加した。
  ・"Save setup"ダイアログの初期ファイルディレクトリを、読み込まれたteraterm.iniがある箇所へ固定するよう変更した。
  ・"File"配下に"View Log"メニューを追加した。
  ・"Additional settings"に"View Log Editor"ボックスを追加した。
  ・teraterm.iniの[Tera Term]セクションに、"ViewlogEditor"エントリを追加した。
  ・SSH2対応TTSSH(1.08)へ差し替え
  ・TeraTerm Menu 1.02へ差し替え。
     - デフォルトオプションに漢字コード(/KT=UTF8 /KR=UTF8)を追加した。
     - SSH自動ログインにおいて、ユーザパラメータを指定できるようにした。
     - SSH自動ログインにおいて、マクロの指定を削除した。
     - SSH自動ログインにおいて、秘密鍵ファイルの指定をできるようにした。

2005.1.10 (Ver 2.09)
  ・マルチディスプレイ環境において、ウィンドウのリサイズを行うと、プライマリディスプレイへ戻ってしまう現象に対処。パッチ作成に感謝します＞安藤弦彦氏

2005.1.6 (Ver 2.08)
  ・ログ採取のダイアログをオープンダイアログからセーブダイアログへ変更した
  ・"Save setup"ダイアログの初期ファイルディレクトリをプログラム本体がある箇所へ固定した
  ・SSH2対応TTSSH(1.07)へ差し替え

2005.1.5 (Ver 2.07h)
  ・SSH2対応TTSSH(1.06)へ差し替え

2004.12.27 (Ver 2.07g)
  ・SSH2対応TTSSH(1.05)へ差し替え

2004.12.23 (Ver 2.07f)
  ・SSH2対応TTSSH(1.04)へ差し替え

2004.12.18 (Ver 2.07e)
  ・SSH2対応TTSSH(1.03)へ差し替え

2004.12.16 (Ver 2.07d)
  ・SSH2対応TTSSH(1.02)へ差し替え

2004.12.15 (Ver 2.07c)
  ・フォントのファイル属性の見直し
  ・インストール時にTeraTerm関連のプロセスを殺すようにした

2004.12.14 (Ver 2.07b)
  ・teraterm.iniのスクロールバッファの初期値を 20000 へ変更。
  ・フォントのインストール時に、ファイル属性を ReadOnly にするようにした。
  ・TeraTerm Menu 1.01へ差し替え。ttermpro.exeの初期フォルダパスをカレントディレクトリに変更。

2004.12.11 (Ver 2.07a)
  ・SSH2対応TTSSH(1.01)へ差し替え

2004.12.8 (Ver 2.07)
  ・"External setup"をsetup menu配下へ"Additional settings"として移動
  ・File menuに"Duplicate session"を追加（/DUPLICATEオプションを追加）
  ・ttpmacro.exeのプロセス優先度は、ログ採取中のみに下げるようにした。
  ・行が連結している場合は、ログファイルに改行コードを含めないようにした（EnableContinuedLineCopy機能が有効の場合のみ）。
  ・スクロールバッファの最大ライン数を400000までに拡張した。
  ・LogMeIn 1.21を同梱した。収録許可をBoris氏よりいただきました。感謝します。
  ・TeraTerm Menu 0.94r2(+SSH2 support 1.00)を同梱した。収録許可を早川様よりいただきました。感謝します。
  ・SSH2対応TTSSH(1.00)へ差し替え
  ・その他細かいバグ修正

2004.11.6 (Ver 2.06)
  ・DEC特殊フォントによる罫線描画時にゴミが残る問題を修正した。これにより、UTF-8環境では日本語以外の言語には未対応となる。また、BOLD styleを使用すると、文字列の右端が切れなくなる。
  ・2つめ以降のTeraTermの起動時にもディスクから teraterm.ini を読むようにした。
  ・エクスプローラからのディレクトリのドラッグ＆ドロップに対応した。
  ・READMEをBoris Maisuradze氏に英文化していただきました。感謝します。

2004.10.25 (Ver 2.05a)
  ・SSH2対応TTSSH(1.00alpha4)へ差し替え

2004.10.11 (Ver 2.05)
  ・SSH2対応TTSSH(1.00alpha2)を同梱した
  ・MFC7.1 runtimeを同梱した
  ・環境変数 TERATERM_EXTENSIONS の設定有無に関わらず、常に TTSSH をロードするようにした。
  ・半角カナのUTF-8化が漏れていたのを修正した
  ・INIファイルにおいてTTSSHが有効になっていると、Cygtermがうまく起動されない問題への対処（/nosshオプションを追加）。

2004.9.29 (Ver 2.04a)
  ・配布形式をパッチからバイナリへ変更した。
    TeraTerm原作者（寺西様）より許可をいただきました。感謝します。

2004.9.5 (Ver 2.04)
  ・ログ採取中のマクロ実行が停止することがある問題への対処
  ・UTF-8化以降に追加した機能をGUIから設定できるようにした（Edit - External setupを追加）
  ・ANSI Colorの設定内容をiniファイルへ保存するようにした

2004.8.6 (Ver 2.03)
  ・Cygwin接続のアクセラレータキーに"Alt + G"を割り当てた
  ・ログ採取ダイアログにファイル名のフルパス表示を追加した
  ・ログ採取中にファイルへコメントを追加できるようにした
  ・SetLayeredWindowAttributes() APIを静的リンクから動的ロードへ変更した（NT4.0対応）。
  ・差分ファイル作成ツールを WDiff から udm へ変更した

2004.4.12 (Ver 2.02)
  ・Mac OS X(HFS+)向けUTF-8をサポート
  ・日本語以外の言語表示をサポート（ロケールとコードページ設定の追加）
  ・日本語以外の言語入力をサポート（imm32.libをリンク）
  ・BOM(Byte Order Mark)を削除するようにした
  ・その他細かいバグフィックス

2004.3.19 (Ver 2.00a)
  ・READMEの記述を修正
  ・READMEの英文化

2004.3.18 (Ver 2.00)
  ・ベースソースをオリジナルからIPv6版へ変更した。
  ・半透明ウィンドウに対応した
  ・Cygwin接続に対応した
  ・バッファの全選択をできるようにした
  ・マウスカーソルの種別を設定できるようにした
  ・WinSock1.1から2.2へ切り替えた
  ・その他細かいバグフィックス

2004.3.13 (Ver 1.02)
  ・ホスト名の最大長を80へ戻した
  ・送信にもUTF-8対応した

2004.3.9 (Ver 1.01a)
  ・SSHの利用に関する記述をREADMEに記載。
  ・SSHはTTSSH 1.5.4 + IPv6 0.81にて動作確認を行った。

2004.3.6 (Ver 1.01)
  ・ホイールマウスに対応した
  ・/KR オプションに"UTF8"を追加した
  ・UTF-8変換機能の強化（テーブル引きを追加）
  ・ホスト名の最大長を80から1024へ拡張

2004.3.2 (Ver 1.00)
  ・初公開

以上

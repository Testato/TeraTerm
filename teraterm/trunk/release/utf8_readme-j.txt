==============================================================================
■UTF-8対応TeraTerm Pro             【利用手引き】
==============================================================================

●はじめに
  本プログラムはターミナルソフト「TeraTerm Pro」をUTF-8対応させたものです。


●収録ファイル
  アーカイブを解凍すると次に示すファイルがあります。

  ---------------------+-----------------------------------
  readme.txt              いまお読みのテキストファイル
  cygterm.exe             Cygterm本体
  cygterm.cfg             Cygterm用設定ファイル
  TERATERM_sample.INI     iniファイルサンプル
  ttp*.dll                TeraTerm DLL
  tt*.exe                 TeraTerm 実行ファイル
  keycode.exe             keycode
  ttxssh.dll              SSH2対応TTSSH
  msvcr71.dll             MFC7.1ランタイム
  ---------------------+-----------------------------------


●インストール方法
  本プログラムに含まれるバイナリファイルを、TeraTerm Pro2.3オリジナルがインストール
  されているディレクトリへ上書きコピーしてください。
  また、cygterm.exeおよびcygterm.cfgを同一のディレクトリへコピーしておきます。
  
  ※注意：動作モード
    UTF-8機能は日本語モードでのみ有効となります。オリジナルTeraTerm Proは日本語モードで
    インストールしておいてください。
  
  
●Unicode設定
  UTF-8化を行うには、TeraTerm Proの「Setup」メニューから「Terminal」を選択すると、
  ダイアログが出てくるので、「Kanji(receive)」および「Kanji(transmit)」にて、
  【UTF-8】を選んでください。
  TeraTerm Proの再起動は必要ありません。
  コマンドラインの「/KT」および「/KR」オプションにおいて、"UTF8"を指定すると、
  送信および受信コードにUTF-8を設定することができます。
  
  日本語以外の言語を表示できるようにするためには、teraterm.iniファイルにロケール
  およびコードページの設定が必要となります。下記にサンプルを示します。
  ロケールおよびコードページに設定できる内容については、下記のサイトを参照してください。
  
    http://www.microsoft.com/japan/msdn/library/default.asp?url=/japan/msdn/library/ja/vclib/html/_crt_language_strings.asp
    http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/unicode_81rn.asp
    
------------------------------------------------------------------------------------
; Locale for Unicode
Locale = japanese

; CodePage for Unicode
CodePage = 932
------------------------------------------------------------------------------------

  ※注意：Mac OS X
    【UTF-8m】はMac OS X(HFS+)向けのエンコーディング指定です。受信のみのサポートです。
    「/KR」コマンドラインでは"UTF8m"を指定することができます。


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
    SHIFTキーを押しながらマウスをドラッグすると「ブロック選択」ができるように
    なります。これはオリジナルからある機能です。


●ログファイルへのコメント追加
  Fileメニューから"Comment to Log..."を選択すると、ログファイルへコメントを追加
  することができます。追加できるコメントは最大255文字までです。



●設定ダイアログ
  Editメニューから"External setup"を選択すると、UTF-8化以降に追加した機能に関する
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
  connect 'myserver:23' 
  
  /nossh を使う方を強く推奨します。このオプションがない場合、TeraTermは teraterm.ini が最後に保存されたときと同じメソッド（telnet もしくは SSH）を使って、接続しようとします。もし、SSHを使って接続しようとするならば、connectマクロは失敗するでしょう。
  
  
  ・SSH接続
  
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
  
  最初の方法は、SSHバージョンを指定せずにSSH接続をします。/1 と /2 のオプションは、SSH1およびSSH2であることを指定します。最後の6つの方法は、マクロからユーザ名とパスワードを渡すことにより、SSH認証ダイアログをスキップさせることができます。/auth=passwordはパスワード認証、/auth=publickeyは公開鍵認証であることを表します。TeraTermマクロにユーザ名とパスワードを指定した場合、そのマクロファイルをプレーンテキストとして保存したのなら、セキュリティ的に安全な場所に置く必要があることを肝に銘じておいてください。
  
  
  ・COMポート接続
  
  connect '/C=x' 
  
  xはCOMポート番号を表します。たとえば、COM1に接続したいのなら、 connect '/C=1' とします。
  
  source URL: http://www.neocom.ca/forum/viewtopic.php?t=6


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


■開発環境
  OS: Windows XP Professional
  コンパイラ: Visual Studio .NET 2003
  
  ソフトウェア：
  TeraTerm Pro 2.3
  IPv6 0.81
  Cygterm 1.06
  TTSSH 1.5.4
  

■著作権
  本プログラムはフリーソフトウェアです。
  TeraTerm Proは寺西高氏の著作物です。
  ANSIカラー色変更＆行連結パッチは多賀奈由太氏に著作権があります。
  UTF-8化のコードについては、著作権は平田豊にあります。
  IPv6版TeraTerm Proは加藤淳也氏の著作物です。
  CygtermはNSym氏の著作物です。
  TTSSHはRobert O'Callahan氏の著作物です。
  TeraTerm Menuは早川信平氏の著作物です。
  LogMeTTはNeoCom Solutionsの著作物です。
  Eterm風透過ウィンドウ機能はAKASI氏に著作権があります。
  TTProxyは蛭子屋双六氏の著作物です。
  本プログラムは無償で利用できますが、作者は本プログラムの使用にあたり
  生じる障害や問題に対して一切の責任を負いません。

  e-mail : yutakakn@mail.goo.ne.jp
  URL    : http://hp.vector.co.jp/authors/VA013320/
  CVS    : http://sourceforge.jp/projects/ttssh2/
  TeraTerm Pro: http://hp.vector.co.jp/authors/VA002416/
  ANSIカラー色変更＆行連結パッチ: http://www.sakurachan.org/ml/teraterm/msg00264.html
  IPv6版TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  Cygterm: http://www.dd.iij4u.or.jp/~nsym/cygwin/cygterm/index.html
  TeraTerm Menu: http://park11.wakwak.com/~shinpei/freesoft/index.html
  LogMeTT: http://www.neocom.ca/freeware/LogMeIn/
  Eterm風透過ウィンドウ機能パッチ：http://www.geocities.co.jp/SiliconValley-PaloAlto/4954/av.html
  TTProxy: http://yebisuya.dip.jp/Software/TTProxy/
  TeraTerm forum: http://www.neocom.ca/forum/index.php
  TeraTerm ML: http://www.freeml.com/info/teraterm@freeml.com


■注意事項
  ・UTF-8からShift_JISへ変換できない文字は疑問符表示となります。
  ・UTF-8環境でのEUC出力が画面に反映されないことがあります。

  ※インストーラは日本語バージョンとなっています。日本以外のユーザは、インストーラのウィザードにおいて、すべてデフォルトのボタンを押下してください。インストーラが完了したあと、TeraTermを起動し、Settings->Generaメニュー配下のLanguage設定を変更してください。最後に、teraterm.iniファイルに保存してください。


■制限事項
  ・UTF-8対応は現在日本語のみです。
  ・SSHに対する"Duplicate session"では常に SSH2 での接続を試みます。


■改版履歴

2005.4.?? (Ver 4.13)
  ・2つめ以降のTeraTermプロセスにおいて、バージョン情報が正しく取得されない問題へ対処した。
  ・マウスの右ボタン押下でのペーストを禁止する設定を teraterm.ini(DisablePasteMouseRButton) および Additional settings に追加した。
  ・ZMODEMの送信において、ファイルサイズが20MBを超えるとクラッシュする問題へ対処した。Thorsten Behrens氏に感謝します。
  ・LogMeTT 2.2.4へ差し替え。

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
  ・ttmacro.exeのプロセス優先度は、ログ採取中のみに下げるようにした。
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

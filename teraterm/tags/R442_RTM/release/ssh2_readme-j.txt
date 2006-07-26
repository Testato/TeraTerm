==============================================================================
■SSH2対応TTSSH             【利用手引き】
==============================================================================

●はじめに
  本プログラムはTTSSH(An SSH Extension to Teraterm)をSSH2に対応させたモジュールであり、
  TeraTerm上でSSH2(Secure Shell version 2)を利用することが可能となります。


●収録ファイル
  アーカイブを解凍すると次に示すファイルがあります。

  ---------------------+-----------------------------------
  readme-j.txt            いまお読みのテキストファイル
  ttxssh.dll              DLL本体
  TERATERM_sample.INI     INIファイルサンプル
  ---------------------+-----------------------------------


●インストール方法
  TeraTermがインストールされているフォルダへDLL本体をコピーします。
  すでにオリジナル版TTSSHが格納されている場合は、事前にバックアップを取っておいてから
  上書きしてください。
  
  ※注意：
    SSH2対応TTSSHはUTF-8対応TeraTerm Pro上でしか動作しません。
    UTF-8対応TeraTerm Pro: http://hp.vector.co.jp/authors/VA013320/
  
  
●SSHの利用方法
  以下の手順によりSSH(Secure Shell)を使った通信を行うことができます。
  
  １．DLLのコピー
      UTF-8対応TeraTerm Proがインストールされているフォルダへ"ttxssh.dll" を
      コピーします。
  
  ２．TeraTermメニューの"New connection"を呼び出し、"Service"で'SSH'を
      選択する（ポート番号は22）。
      
  ３．"SSH Authentication"ダイアログが出てくるので、ユーザ名とパスワードを
      入力して、OKボタンを押下する。
      
  ４．ユーザ認証に成功すれば、サーバ側のプロンプトが出てくる。


●SSH自動ログイン
  以下のコマンドライン・オプションの使用により、SSH1およびSSH2を利用してユーザの
  自動ログインが可能となります。
  
  書式）パスワードおよびkeyboard-interactive認証
     ttermpro.exe ホスト名:22 /ssh [/1|/2] /auth=password /user=ユーザ名 /passwd=パスワード
  書式）公開鍵認証
     ttermpro.exe ホスト名:22 /ssh [/1|/2] /auth=publickey /user=ユーザ名 /passwd=パスワード /keyfile=秘密鍵ファイル

  例）SSH1によるパスワード認証ログイン（ユーザ名 nike パスワード kukuri）
     ttermpro.exe 192.168.1.3:22 /ssh /1 /auth=password /user=nike /passwd=kukuri

  例）SSH2によるパスワード認証ログイン（ユーザ名 nike パスワード kukuri）
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=password /user=nike /passwd=kukuri

  例）SSH2による公開鍵認証ログイン（ユーザ名 kitakita パスワード oyaji 28 秘密鍵ファイル d:\tmp\id_rsa）
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=publickey /user=kitakita /passwd=oyaji@28 /keyfile=d:\tmp\id_rsa

  ※空白の扱い
    パスワードおよびパスに空白を含める場合、スペース(' ')の代わりにアットマーク('@')を
    使用してください。また、アットマークという文字を使用する場合は、2つ続けて('@@')と
    記述してください。


●コマンドラインパラメータ
  TTSSHでは以下のコマンドラインパラメータが使用できます。
  
  パラメータ                説明
  ------------------------+-------------------------------------------------------
  /ssh-Lport1:host:port2    ローカルポート(port1)からリモートポート(port2)へポートフォワーディングする
  /ssh-Rport1:host:port2    リモートポート(port1)からローカルポート(port2)へポートフォワーディングする
  /ssh-X                    X11をポートフォワーディングする
  /ssh-f=file               teraterm.iniファイル(file)を読み込む
  /ssh-v                    ログレベルを VERBOSE モードにする
  /ssh-autologin            空のパスワードでの自動ログインを試みる
  /ssh-autologon            同上
  /ssh-consume=file         teraterm.iniファイル(file)の読み込み後、当該ファイルを削除する。
  /t=value                  TTSSHを有効にする(value=2)、無効にする(value=2以外)
  /f=file                   teraterm.iniファイル(file)を読み込む
  /1                        SSH1で接続する
  /2                        SSH2で接続する
  /nossh                    TTSSHを無効にする
  /auth=mode                自動ログインを試みる(mode=password, publickey)
  /user=username            ユーザ名を指定する
  /passwd=password          パスワードを指定する
  /keyfile=file             秘密鍵を指定する


  例）
  ttermpro.exe server1:22 /ssh /ssh-L30025:smtpsrv1:25 /ssh-L30110:pop3srv1:110
  ttermpro.exe server2:22 /ssh /ssh-X
  ttermpro.exe server3:22 /ssh /2 /auth=password /user=guest /passwd=guest


●known_hostsファイル
　known_hostsファイルの書式は以下のとおりです。
　
　書式：
　  ホスト名  キー種別  ホスト公開鍵
　  
　  # コメント
　  ! 否定

　例：
    192.168.1.2 ssh-rsa AAAAB3NzaC1...nXIDE=
    192.168.1.2 ssh-dss AAAAB3NzaC1...nXIDE=
    192.168.1.2 1024 35 13032138...794461 
    #192.168.1.3 ssh-rsa AAAAB3NzaC1...nXIDE=
    192.168.1.2 1024 35 13032138...794461 
    *.example.com,!mail.example.com ssh-rsa AAAAB3NzaC1...nXIDE=


■TTSSHのビルド方法
  TTSSHのビルド方法について以下に示します。ビルドにはVisual Studio 2005(VC++8.0)、ActivePerlが必要です。
  
  1. ソースコードをSourceForge(http://sourceforge.jp/projects/ttssh2/)からチェックアウトする。
  
  2. zlibのソースコード(http://www.zlib.net/)を ttssh2\zlib ディレクトリに展開する。
     Release では zlib\projects\visualc6\Win32_LIB_Release\zlib.lib がリンクされることになる（コンパイルオプションに /MT が指定されていること）。
     Debug では zlib\projects\visualc6\Win32_LIB_Release\zlibd.lib がリンクされることになる（コンパイルオプションに /MTd が指定されていること）。
  
  3. OpenSSLのソースコード(http://www.openssl.org/)を ttssh2\openssl ディレクトリに展開する。OpenSSLをビルドする（以下参照）。
     Release では openssl\out32\libeay32.lib が、Debug では openssl\out32.dbg\libeay32.lib がリンクされることになる。

      - Build OpenSSL.
            + cd openssl
            + perl Configure VC-WIN32
                (Yes, you need perl to build OpenSSL!)
            + ms\do_ms.bat をエディタで開いて、ms\nt.mak の次の行に下記の内容を追記する
              perl util\mk1mf.pl no-asm debug VC-WIN32 >ms\ntd.mak
            + ms\do_ms
            + ms\nt.mak をエディタで開いて、CFLAG行の /MD を /MT に変更する
            + ms\ntd.mak をエディタで開いて、CFLAG行の /MDd を /MTd に変更する
            + nmake -f ms\nt.mak
            + nmake -f ms\ntd.mak
            + cd ..
                (Now you are back in PortForwarder folder.)
        See the instruction in the OpenSSL documentation for details.

  4. ttssh2\ttssh.sln をVisual Studioで開く
  5. ソリューションをビルドする
  6. ビルドに成功するとttssh2\ttxssh ディレクトリにDLLが生成される



■開発環境
  OS: Windows XP Professional
  コンパイラ: Visual Studio 2005 Standard Edition
  
  ソフトウェア：
  TeraTerm Pro 2.3
  IPv6 0.81
  TTSSH 1.5.4
  

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
  IPv6版TeraTerm Proは加藤淳也氏の著作物です。
  TTSSHはRobert O'Callahan氏の著作物です。
  TTSSHのSSH2化のコードについては、著作権は平田豊にあります。
  OpenSSLはOpenSSL Projectの著作物です。
  zlibはGreg RoelofsとJean-loup Gailly氏の著作物です。
  本プログラムは無償で利用できますが、作者は本プログラムの使用にあたり
  生じる障害や問題に対して一切の責任を負いません。

  e-mail : yutakakn@mail.goo.ne.jp
  URL    : http://hp.vector.co.jp/authors/VA013320/
  CVS    : http://sourceforge.jp/projects/ttssh2/
  TeraTerm Pro: http://hp.vector.co.jp/authors/VA002416/
  IPv6版TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  TTSSH original:http://www.zip.com.au/~roca/ttssh.html


■改版履歴

2006.7.26 (Ver 2.30)
  ・TTSSHアイコン(ttsecure.ico)を新しくした。坂本龍彦氏に感謝します。

2006.6.29 (Ver 2.29)
  ・TTSSHのsetupダイアログの変更内容が次回接続時から反映されるようにした。パッチ作成に感謝します＞永田氏
  ・ssh_known_filesファイルを常にTeraTermインストールディレクトリへ保存するようにした。パッチ作成に感謝します＞永田氏
  ・Copyright表記を変更した

2006.6.23 (Ver 2.28)
  ・SSH2遅延パケット圧縮（OpenSSH 4.2以降）をサポートした。パッチ作成に感謝します＞永田氏

2006.6.14 (Ver 2.27)
  ・OpenSSH 4.3以降で遅延パケット圧縮が設定されている場合、従来のパケット圧縮を有効にした状態でのサーバへの接続ができないバグを修正した。

2006.6.11 (Ver 2.26)
  ・SSH Port Forward の編集画面で Alt + キーが効かないバグを修正した。パッチ作成に感謝します＞永田氏
  ・SSH Port Forward の編集画面で、TeraTermが未接続状態の場合、ポート番号不正を即座にメッセージボックスが表示されないバグを修正した。

2006.5.7 (Ver 2.25)
  ・OpenSSL 0.9.8bへ差し替えた

2006.4.7 (Ver 2.24)
  ・HP-UXにおいてX11 fowardingが失敗した場合に、SSH2セッションが切断されないようにした。

2006.4.4 (Ver 2.23)
  ・known_hostsファイルにおいてキー種別の異なる同一ホストのエントリがあった場合、古いキーを削除するユーザダイアログオプションを追加した。パッチ作成に感謝します＞永田氏

2006.3.29 (Ver 2.22)
  ・known_hostsファイルにキー種別の異なる同一ホストのエントリがあると、アプリケーションエラーとなるバグを修正した。

2006.3.27 (Ver 2.21)
  ・SSH2のknown_hosts対応を追加した。
  ・known_hostsダイアログにサーバの fingerprint を表示するようにした

2006.3.6 (Ver 2.20)
  ・SSH2ウィンドウ制御の見直しにより、スループットを向上させた。

2006.2.24 (Ver 2.19)
  ・コンパイラを Visual Studio 2005 Standard Edition に切り替えた。
  ・stricmp()を_stricmp()へ置換した
  ・strdup()を_strdup()へ置換した
  ・authorized_keysファイルの"command="をサポートした
  ・OpenSSL 0.9.8b snapshotへ差し替えた

2005.10.21 (Ver 2.18)
  ・接続ダイアログに History チェックボックスを追加した。
  ・OpenSSL 0.9.8aへ差し替えた

2005.9.5 (Ver 2.17)
  ・SSH自動ログイン時に認証ダイアログを /I オプション使用時にのみ最小化するようにした。

2005.8.27 (Ver 2.16)
  ・SSH自動ログイン時に認証ダイアログを最小化するようにした。高橋智雄氏に感謝します。
  ・zlib 1.2.3へ差し替えた

2005.7.24 (Ver 2.15)
  ・SSH2ポートフォワーディングをサポートした。
  ・SSH2パケット圧縮をサポートした。
  ・SSH2接続時にTTY情報(VERASE)を渡すようにした。
  ・SSH1接続で一度ユーザ認証に失敗すると、その後認証ができなくなるバグを修正。
  ・接続ダイアログ上で Alt+P や Alt+C などのアクセラレータキーが効かないバグを修正。
  ・OpenSSL 0.9.8へ差し替えた
  ・zlib 1.2.2にsecurity fix(CAN-2005-2096)を追加。

2005.5.28 (Ver 2.14)
  ・秘密鍵ファイルダイアログに3ファイルフィルタを追加した。
  ・zlibのバージョンをバージョン情報に追加した。

2005.4.24 (Ver 2.13)
  ・キー作成ダイアログを追加した（Setup - SSH KeyGenerator）。
  ・OpenSSL 0.9.7gへ差し替えた

2005.4.9 (Ver 2.12)
  ・SSH2チャネルルックアップ機構を追加
  ・キー再作成時にアプリケーションエラーとなるバグを修正（2.10でのデグレード）。
  ・"Duplicate session"においてSSH自動ログインを行うようにした。


2005.3.31 (Ver 2.11)
  ・SSH2認証ダイアログの Use challenge/response to log in にアクセラレータキーを割り当てた。
  ・シリアルポートを開いた状態からAlt-Nで新規接続を開こうとしたとき、フォーカスを当てるようにした。
  ・SSHログファイル(TTSSH.LOG)の採取データを増加した
  ・NetScreen(HITACHI) workaround対応。SSH2_MSG_CHANNEL_REQUEST送信時において、wantconfirmをfalseにした（サーバからのリプライを期待しない）。
  ・OpenSSL 0.9.7fへ差し替えた


2005.3.13 (Ver 2.10)
   ・ttxssh.dllのバージョン情報が古いままだったので修正した。
   ・SSH client ID(SSH-2.0-TTSSH/X.XX Win32)にバージョン情報を含めるようにした。
   ・Tru64 UNIX workaroundを追加。Miguel氏に感謝します。
   ・SSH2 log dump機構を追加した
   ・SSH server IDにCR+LFが含まれていた場合、CRの除去ができていなかったバグを修正。
   ・すでにログイン処理を行っている場合は、SSH2_MSG_SERVICE_REQUESTの送信は行わないようにした。
   ・パスワード認証の前に行うkeyboard-interactiveメソッドで、デフォルト設定値([TTSSH] KeyboardInteractive)を無効(0)にした。OpenSSH 4.0では無効なメソッドを使用するとコネクションが切られてしまうため。また、認証ダイアログのラベル名を設定の有無により変更するようにした。
   ・SSH2（keyboard-interactive認証）接続においてchallenge/response loginに対応した。

2005.2.22 (Ver 1.09)
  ・ドットで始まるディレクトリにあるSSH2秘密鍵ファイルが読み込めない問題へ対処した。
  ・HeartBeat(keep-alive), SSH2 keyboard-interactiveの設定をTTSSHのダイアログ上から行えるようにした。

2005.1.30 (Ver 1.08)
  ・keyboard-interactive認証をサポートした。それにともない、teraterm.ini ファイルのTTSSHセクションの"KeyboardInteractive"エントリを追加した。
  ・バージョンダイアログにホームページのURL・OpenSSLバージョンを追加した。
  ・SSH2認証ダイアログで、rhostsおよびTISがグレーになる前に、Enterキーを押下すると、アプリケーションエラーとなる現象に対処した。
  ・公開鍵認証自動ログインをサポート。/auth=publickey, /keyfileコマンドラインオプションを追加。

2005.1.6 (Ver 1.07)
  ・telnet接続時にターミナルサイズ変更を行うと、アプリケーションエラーとなるバグを修正（1.06でのデグレード）。

2005.1.5 (Ver 1.06)
  ・ターミナルサイズ変更に対応した。
  ・キー再作成時にアプリケーションエラーとなるバグを修正（1.05でのデグレード）。

2004.12.27 (Ver 1.05)
  ・'Auto window close'が無効の場合、切断後の接続ができない問題を修正した。
  ・SSH2 KEX処理のメモリリークを修正した。
  ・SSH2秘密鍵読み込み失敗時のエラーメッセージ出力を強化した。

2004.12.23 (Ver 1.04)
  ・公開鍵認証をサポート

2004.12.18 (Ver 1.03)
  ・パケット受信時におけるHMACチェックを追加
  ・メッセージ認証アルゴリズムに HMAC-MD5 を追加
  ・KEXにおけるRSAおよびDSSのkey verify処理を追加

2004.12.16 (Ver 1.02)
   ・SSH自動ログインにおいて、まれにアプリケーションエラーとなる現象へ対処した。
   ・SSH1接続時の"SECURITY WARNING"ダイアログで、キャンセルするとアプリケーションエラーとなる現象へ対処した。

2004.12.11 (Ver 1.01)
  ・SSHハートビート（キープアライブ）機能を追加した。ハートビート間隔は、 teraterm.ini ファイルのTTSSHセクションの"HeartBeat"エントリで設定可能（デフォルトは60秒）。

2004.12.8 (Ver 1.00)
  ・SSH1/SSH2自動ログインをサポート（password認証のみ）。/auth, /username, /passwdオプションを追加。
  ・SSHのデフォルトバージョンを SSH2 に変更した
  ・接続ダイアログのtab orderを修正した。
  ・接続ダイアログの起動時、ホスト名にフォーカスが当たるようにした。
  ・SSH2自動ログイン・サンプルマクロ(ssh2login.ttl)を同梱した

2004.11.7 (Ver 1.00 beta1)
  ・暗号アルゴリズム AES128 をサポートした
  ・KEXアルゴリズム diffie-hellman-group14-sha1 (OpenSSH 3.9以降)をサポートした
  ・KEXアルゴリズム diffie-hellman-group-exchange-sha1 をサポートした
  ・ホストキーアルゴリズム ssh-dss をサポートした
  ・バージョン情報ダイアログにSSH2情報を出力するようにした
  ・OpenSSL 0.9.7eへ差し替えた
  ・READMEをBoris Maisuradze氏に英文化していただきました。感謝します。

2004.10.25 (Ver 1.00 alpha4)
  ・ウィンドウサイズ制御の不具合を修正した。
  ・データ通信確立後のDH鍵再作成に対応した。
  ・その他細かいバグフィックス

2004.10.17 (Ver 1.00 alpha3)
  ・ウィンドウサイズ制御処理を追加した
  ・コネクションを切断する際、サーバ側へ SSH2_MSG_CHANNEL_CLOSE を送るようにした。
  ・terminal typeが環境変数TERMへ渡されるようにした

2004.10.11 (Ver 1.00 alpha2)
  ・SSH1をサポートした
  ・SSH sessionが切断されるバグ（"Oversized packet received from server"と表示されて、
    コネクションが切れる問題）を修正した
  ・/1, /2 オプションの追加
  ・TERATERM.INI ファイルにSSH protocol versionのエントリを追加した

2004.8.8 (Ver 1.00 alpha1a)
  ・DLLをrelease build版へ差し替えた

2004.8.5 (Ver 1.00 alpha1)
  ・初公開

以上

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
  msvcr71.dll             MFC7.1ランタイム
  TERATERM_sample.INI     INIファイルサンプル
  ---------------------+-----------------------------------


●インストール方法
  TeraTermがインストールされているフォルダへDLL本体とMFC7.1ランタイムをコピーします。
  すでにオリジナル版TTSSHが格納されている場合は、事前にバックアップを取っておいてから
  上書きしてください。
  
  ※注意：
    SSH2対応TTSSHはUTF-8対応TeraTerm Pro上でしか動作しません。
    UTF-8対応TeraTerm Pro: http://hp.vector.co.jp/authors/VA013320/
  
  
●SSHの利用方法
  以下の手順によりSSH(Secure Shell)を使った通信を行うことができます。
  
  １．DLLのコピー
      UTF-8対応TeraTerm Proがインストールされているフォルダへ"ttxssh.dll", "msvcr71.dll"を
      コピーします。
  
  ２．TeraTermメニューの"New connection"を呼び出し、"Service"で'SSH'を
      選択する（ポート番号は22）。
      
  ３．"SSH Authentication"ダイアログが出てくるので、ユーザ名とパスワードを
      入力して、OKボタンを押下する。
      
  ４．ユーザ認証に成功すれば、サーバ側のプロンプトが出てくる。


●SSH自動ログイン
  以下のコマンドライン・オプションの使用により、SSH1およびSSH2を利用してユーザの
  自動ログインが可能となります。ただし、パスワードに空白を含めることはできません。
  また、password認証のみのサポートとなっています。
  
  書式）
     ttermpro.exe ホスト名:22 /ssh [/1|/2] /auth=password /user=ユーザ名 /passwd=パスワード

  例）SSH1
     ttermpro.exe 192.168.1.3:22 /ssh /1 /auth=password /user=nike /passwd=kukuri

  例）SSH2
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=password /user=nike /passwd=kukuri


■開発環境
  OS: Windows XP Professional
  コンパイラ: Visual Studio .NET 2003
  
  ソフトウェア：
  TeraTerm Pro 2.3
  IPv6 0.81
  TTSSH 1.5.4
  

■著作権
  本プログラムはフリーソフトウェアです。
  TeraTerm Proは寺西高氏の著作物です。
  IPv6版TeraTerm Proは加藤淳也氏の著作物です。
  TTSSHはRobert O'Callahan氏の著作物です。
  TTSSHのSSH2化のコードについては、著作権は平田豊にあります。
  本プログラムは無償で利用できますが、作者は本プログラムの使用にあたり
  生じる障害や問題に対して一切の責任を負いません。

  e-mail : yutakakn@mail.goo.ne.jp
  URL    : http://hp.vector.co.jp/authors/VA013320/
  CVS    : http://sourceforge.jp/projects/ttssh2/
  TeraTerm Pro: http://hp.vector.co.jp/authors/VA002416/
  IPv6版TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  TTSSH original:http://www.zip.com.au/~roca/ttssh.html


■制限事項
  ・パケットの圧縮はサポートしていません
  ・ポートフォワーディングはサポートしていません


■改版履歴

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

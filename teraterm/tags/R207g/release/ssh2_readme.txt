==============================================================================
* SSH2 version TTSSH        [User's Manual]
==============================================================================

* Introduction
  "TTSSH" is an extension of the terminal emulator TeraTerm Pro that adds Secure Shell 
  version 2 (SSH2) protocol support to TeraTerm. This version of TTSSH replaces the
  original version created by Robert O'Callahan that supported only SSH1.

* Content of archive file
  Archive (zip) file contains:

  File name               Description  
  ---------------------+-----------------------------------
  readme.txt              this file
  ttxssh.dll              DLL
  msvcr71.dll             MFC7.1 runtime library
  TERATERM_sample.INI     sample ini file
  ---------------------+-----------------------------------

* Installation
  1. To add SSH2 support to TeraTerm you should already have installed the 
     original version of TeraTerm Pro with additional package supporting UTF-8.
     
     The original version of TeraTerm Pro can be downloaded from: 
         http://hp.vector.co.jp/authors/VA002416/ttermp23.zip
         
     Additional package for UTF-8 support can be downloaded from:
         http://sleep.mat-yan.jp/~yutaka/windows/index.html
  
  2. Backup all files from the TeraTerm directory.
  
  3. Extract ttxssh.dll and msvcr71.dll files from the archive into the TeraTerm directory.
  
  !! IMPORTANT !!
    SSH2 version of TTSSH is compatible only with UTF-8 version of TeraTerm Pro.
    

* How to use SSH
  Perform the following steps to establish network communication using SSH (Secure Shell):
  
  1. Select "New connection" from the TeraTerm menu and chose "SSH" option from "Service" (port 22).
  
  2. Enter username and password in "SSH Authentication" dialog and press OK button.
  
  3. If you successfully pass authentication, UNIX shell will appear on TeraTerm terminal. 

* SSH autologin
  Perform the following command line options to be able to autologin with SSH protocol.
  But now you can't include the blank in the passphrase. And also supporting method 
  is only password authentication method.
  
  SYNOPSIS:
     ttermpro.exe HOSTNAME:22 /ssh [/1|/2] /auth=password /user=USERNAME /passwd=PASSPHRASE

  EXAMPLE: SSH1
     ttermpro.exe 192.168.1.3:22 /ssh /1 /auth=password /user=nike /passwd=kukuri

  EXAMPLE: SSH2
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=password /user=nike /passwd=kukuri



* Development Environment
  OS: Windows XP Professional
  Compiler: Visual Studio .NET 2003
  
  Software:
  TeraTerm Pro 2.3
  IPv6 0.81
  TTSSH 1.5.4


* Copyright
  This program is freeware.
  
  Copyright of original TeraTerm Pro belongs to T.Teranishi.
  Copyright of IPv6 version TeraTerm Pro belongs to J.Kato.
  Copyright of original TTSSH belongs to Robert O'Callahan.
  Copyright of TTSSH supporing SSH2 belongs Yutaka Hirata.

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
  IPv6 version TeraTerm Pro: http://win6.jp/TeraTerm/index.html
  TTSSH: http://www.zip.com.au/~roca/ttssh.html


* Limitations of current release
  - does not support packet compression
  - does not support port forwarding


* History

2004.12.23 (Ver 1.04)
  - added support of public key authentication method

2004.12.18 (Ver 1.03)
  - added checking HMAC at received packet
  - added support of HMAC-MD5 message authentication code
  - added verifying the key of RSA and DSA at KEX procedure

2004.12.16 (Ver 1.02)
  - fixed application fault when using SSH autologin mode
  - fixed application fault when pressing ESC key at "SECURITY WARNING" dialog

2004.12.11 (Ver 1.01)
  - added support of SSH heartbeat(keep-alive). You can configurate the heartbeat interval "HeartBeat" entry of TTSSH section in the 'teraterm.ini' file. A measure of heartbeat is per second(default 60 seconds).

2004.12.8 (Ver 1.00)
  - added support of SSH1/SSH2 autologin (only password authentication). And also added /auth, /username, /passwd options.
  - changed SSH default version to SSH2
  - fixed tab order of connection dialog 
  - changed the focus of connection dialog to hostname
  - added SSH2 autologin sample macro(ssh2login.ttl)

2004.11.7 (Ver 1.00 beta1)
  - added support of encryption algorithm AES128
  - added support of KEX algorithm diffie-hellman-group14-sha(OpenSSH 3.9 later version)
  - added support of KEX algorithm diffie-hellman-group-exchange-sha1
  - added support of server host key algorithm ssh-dss
  - added show of SSH2 information in version dialog
  - upgraded OpenSSL to 0.9.7e
  - rewrote README(English). Special thanks to Boris Maisuradze for his cooperation.

2004.10.25 (Ver 1.00 alpha4)
  - fixed the bug of window size control flow
  - added support of DH key re-creation after data-communication establishment
  - in addition, fine bug fix

2004.10.17 (Ver 1.00 alpha3)
  - added support of window size control flow
  - added send SSH2_MSG_CHANNEL_CLOSE to server on disconnect.
  - terminal type is now passed to the environment variable TERM.

2004.10.11 (Ver 1.00 alpha2)
  - added support of SSH1
  - fixed the bug of suddenly disconnecting SSH session (the message 
    "Oversized packet received from server" was appearing in dialog box)
  - added /1, /2 command line options
  - added SSH protocol version related entry in TERATERM.INI file

2004.8.8 (Ver 1.00 alpha1a)
  - replaced DLL release build version

2004.8.5 (Ver 1.00 alpha1)
  - first release


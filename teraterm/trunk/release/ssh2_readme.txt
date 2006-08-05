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
  
  3. Extract ttxssh.dll file from the archive into the TeraTerm directory.
  
  !! IMPORTANT !!
    SSH2 version of TTSSH is compatible only with UTF-8 version of TeraTerm Pro.
    

* How to use SSH
  Perform the following steps to establish network communication using SSH (Secure Shell):
  
  1. Select "New connection" from the TeraTerm menu and chose "SSH" option from "Service" (port 22).
  
  2. Enter username and password in "SSH Authentication" dialog and press OK button.
  
  3. If you successfully pass authentication, UNIX shell will appear on TeraTerm terminal. 

* SSH autologin
  Perform the following command line options to be able to autologin with SSH protocol.
  
  SYNOPSIS:
     ttermpro.exe HOSTNAME:22 /ssh [/1|/2] /auth=password /user=USERNAME /passwd=PASSPHRASE

  SYNOPSIS:
     ttermpro.exe HOSTNAME:22 /ssh [/1|/2] /auth=publickey /user=USERNAME /passwd=PASSPHRASE /keyfile=PRIVATEKEYFILE

  EXAMPLE: SSH1 password authentication auto-login (username nike  password kukuri)
     ttermpro.exe 192.168.1.3:22 /ssh /1 /auth=password /user=nike /passwd=kukuri

  EXAMPLE: SSH2 password authentication auto-login (username nike  password kukuri)
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=password /user=nike /passwd=kukuri

  EXAMPLE: SSH2 public-key authentication auto-login (username kitakita  password oyaji 28  private-key-file d:\tmp\id_rsa)
     ttermpro.exe 192.168.1.3:22 /ssh /2 /auth=publickey /user=kitakita /passwd=oyaji@28 /keyfile=d:\tmp\id_rsa


  NOTICE: blank operation
    If you include the blank in the password or file path, replace the blank(' ') to '@'.
    And also if you use the atmark character in them, write two continuation '@@' instead of '@'.



* Command line parameters
  You can use the following command line parameters on TTSSH.
  
  parameter                 description
  ------------------------+-------------------------------------------------------
  /ssh-Lport1:host:port2    port-fowarding from local port(port1) to remote port(port2)
  /ssh-Rport1:host:port2    port-fowarding from remote port(port1) to local port(port2)
  /ssh-X                    X11 port-fowarding
  /ssh-f=file               read teraterm.ini(file)
  /ssh-v                    setup the log level to VERBOSE mode
  /ssh-autologin            try to autologin with empty password
  /ssh-autologon            the same as the above
  /ssh-consume=file         After teraterm.ini file is read, it is erased.
  /t=value                  TTSSH is enabled(value=2), disable(value=excluding 2)
  /f=file                   read teraterm.ini(file)
  /1                        connect to server with SSH1 protocol
  /2                        connect to server with SSH2 protocol
  /nossh                    TTSSH is disabled
  /auth=mode                try to autologin(mode=password, publickey)
  /user=username            your username is specified
  /passwd=password          your password is specified
  /keyfile=file             your private key file is specified


  Example:
  ttermpro.exe server1:22 /ssh /ssh-L30025:smtpsrv1:25 /ssh-L30110:pop3srv1:110
  ttermpro.exe server2:22 /ssh /ssh-X
  ttermpro.exe server3:22 /ssh /2 /auth=password /user=guest /passwd=guest



* known_hosts file
  The synopsis of 'known_hosts' file is as follows:
  
  SYNOPSIS:
    hostname key-type host-public-key
    
    #  comment
    !  negate

   EXAMPLE:
    192.168.1.2 ssh-rsa AAAAB3NzaC1...nXIDE=
    192.168.1.2 ssh-dss AAAAB3NzaC1...nXIDE=
    192.168.1.2 1024 35 13032138...794461 
    #192.168.1.3 ssh-rsa AAAAB3NzaC1...nXIDE=
    192.168.1.2 1024 35 13032138...794461 
    *.example.com,!mail.example.com ssh-rsa AAAAB3NzaC1...nXIDE=



* How to build TTSSH
  To build TTSSH source code is shown in the following step:
  And you need Visual Studio 2005(VC++8.0) and ActivePerl to build.
  
  1. Checkout TTSSH source code from SourceForge(http://sourceforge.jp/projects/ttssh2/).
  
  2. Extract zlib source code(http://www.zlib.net/) to ttssh2\zlib directory.
     The build target is 'Release' and TTSSH links to zlib\projects\visualc6\Win32_LIB_Release\zlib.lib (compile option /MT must be specified).
     The build target is 'Debug' and TTSSH links to  zlib\projects\visualc6\Win32_LIB_Release\zlibd.lib (compile option /MTd must be specified).
     
  3. Extract OpenSSL source code(http://www.openssl.org/) to ttssh2\openssl directory. Build OpenSSL in the following step(Release:TTSSH links to openssl\out32\libeay32.lib, Debug:openssl\out32.dbg\libeay32.lib):
  
      - Build OpenSSL.
            + cd openssl
            + perl Configure VC-WIN32
                (Yes, you need perl to build OpenSSL!)
            + Open ms\do_ms.bat and append the following line at the next line of 'ms\nt.mak' line.
              "perl util\mk1mf.pl no-asm debug VC-WIN32 >ms\ntd.mak"
            + ms\do_ms
            + Open ms\nt.mak and modify CFLAG line from /MD to /MT.
            + Open ms\ntd.mak and modify CFLAG line from /MDd to /MTd.
            + nmake -f ms\nt.mak
            + nmake -f ms\ntd.mak
            + cd ..
                (Now you are back in PortForwarder folder.)
        See the instruction in the OpenSSL documentation for details.
  
  4. Open ttssh2\ttssh.sln with Visual Studio.
  5. Build TTSSH solution.
  6. TTSSH DLL will be generated in ttssh2\ttxssh directory if the building is successful.

* Development Environment
  OS: Windows XP Professional
  Compiler: Visual Studio 2005 Standard Edition
  
  Software:
  TeraTerm Pro 2.3
  IPv6 0.81
  TTSSH 1.5.4


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
  This program is freeware. TTSSH is licensed under BSD style license.
  
  Copyright of original TeraTerm Pro belongs to T.Teranishi.
  Copyright of IPv6 version TeraTerm Pro belongs to J.Kato.
  Copyright of original TTSSH belongs to Robert O'Callahan.
  Copyright of TTSSH supporing SSH2 belongs to Yutaka Hirata.
  Copyright of OpenSSL belongs to OpenSSL project.
  Copyright of zlib belongs to Greg Roelofs and Jean-loup Gaill.

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


* History

2006.8.5 (Ver 2.31)
  - added the checkbox on SSH authentication dialog whether the user password string would be stored on heap memory. You can configurate the its function enabling flag by "RememberPassword" entry of TTSSH section in the 'teraterm.ini' file.

2006.7.26 (Ver 2.30)
  - updated to new TTSSH icon file(ttsecure.ico). Special thanks to Tatsu Sakamoto.

2006.6.29 (Ver 2.29)
  - Bug fix: Using SSH2 protocol, changes of the TTSSH setup dialog could not feedback on next ssh session. Special thanks to NAGATA Shinya.
  - Bug fix: The 'ssh_known_files' file might not be saved in TeraTerm installation directory. Special thanks to NAGATA Shinya.
  - changed Copyright string.

2006.6.23 (Ver 2.28)
  - added support for SSH2 delayed compression(OpenSSH 4.2 later).  Special thanks to NAGATA Shinya.
  
2006.6.14 (Ver 2.27)
  - Bug fix: TeraTerm enabled SSH compression could not connect to the server when OpenSSH 4.3 later was configured as delay SSH compression.

2006.6.11 (Ver 2.26)
  - Bug fix: Alt key was not available for SSH Port Forward dialog. Special thanks to NAGATA Shinya.
  - Bug fix: Message box dialog doesn't been quickly shown when TeraTerm does not connect to the server for SSH Port Forward dialog.

2006.5.7 (Ver 2.25)
  - upgraded OpenSSL to 0.9.8b

2006.4.7 (Ver 2.24)
  - Bug fix: disconnection of SSH2 protocol after X11 forwarding failure on HP-UX.

2006.4.4 (Ver 2.23)
  - added the user dialog option that delete the old entry in known_hosts file contains the multiple entrys including the different host key type. Special thanks to NAGATA Shinya.

2006.3.29 (Ver 2.22)
  - Bug fix: application error when known_hosts file contains the multiple entrys including the different host key type.

2006.3.27 (Ver 2.21)
  - added support for known_hosts on SSH2 connection.
  - added showing the server's fingerprint on known_hosts dialog.

2006.3.6 (Ver 2.20)
  - Improved SSH2 communication throughput by changing the window size adjustment.

2006.2.24 (Ver 2.19)
  - changed the compiler to Visual Studio 2005 Standard Edition.
  - replaced stricmp() to _stricmp().
  - replaced strdup() to _strdup().
  - added support for "command=" in authorized_keys file.
  - upgraded OpenSSL to 0.9.8b snapshot

2005.10.21 (Ver 2.18)
  - added the host history check button on connection dialog.
  - upgraded OpenSSL to 0.9.8a

2005.9.5 (Ver 2.17)
  - added minimizing the SSH authentication dialog with autologin mode only using '/I' option.

2005.8.27 (Ver 2.16)
  - added minimizing the SSH authentication dialog with autologin mode. Special thanks to Tomoo Takahashi.
  - upgraded zlib to 1.2.3

2005.7.24 (Ver 2.15)
  - added support of SSH2 port forwarding
  - added support of SSH2 packet compression
  - added sending TTY information(VERASE) to server with SSH2 connection.
  - Bug fix: The user can't login to the server since the second times when first user authentication is failed with SSH1 protocol.
  - Bug fix: The accelerator key(Alt + P or Alt + C) doesn't work on connection dialog.
  - upgraded OpenSSL to 0.9.8
  - added the security fix(CAN-2005-2096) to zlib 1.2.2

2005.5.28 (Ver 2.14)
  - added three file filter on private key saving dialog.
  - added zlib version information on TTSSH version dialog.

2005.4.24 (Ver 2.13)
  - added SSH key generator dialog(Setup - SSH KeyGenerator).
  - upgraded OpenSSL to 0.9.7g

2005.4.9 (Ver 2.12)
  - added SSH2 channel lookup mechanism for port forwading.
  - fixed the bug of application fault at DH key re-creation (degradation at TTSSH version supporting SSH2 2.10).
  - added SSH autologin with "Duplicate session".

2005.3.31 (Ver 2.11)
  - added the accelerator key at 'Use challenge/response to log in' on SSH2 authentication dialog.
  - added the focus COM input box when you open new connection using COM port.
  - added SSH log data(TTSSH.LOG).
  - added NetScreen(HITACHI) workaround. Sending SSH2_MSG_CHANNEL_REQUEST message to server is disabled wantconfirm entry.
  - upgraded OpenSSL to 0.9.7f

2005.3.13 (Ver 2.10)
  - upgraded ttxssh.dll version information from 1.5.4 to 2.10.
  - changed SSH client ID(SSH-2.0-TTSSH/X.XX Win32) including TTSSH version information.
  - added Tru64 UNIX workaround. Special thanks to Miguel.
  - added SSH2 dump logging
  - fixed the bug of being not able to get rid of CR when CR+LF code is included in SSH server ID.
  - changed TTSSH don't send SSH2_MSG_SERVICE_REQUEST to server once the client try to login.
  - changed the default value of [TTSSH] KeyboardInteractive to 0(disabled) because SSH connection is closed by server then the invalid method is used in OpenSSH 4.0. And also the passowrd authentication label of SSH authentication dialog is dynamically changed.
  - added support of challenge/response login on SSH2 connection.

2005.2.22 (Ver 1.09)
  - fixed the SSH2 public key problem that TTSSH can't read the private key file in the directory starting by the dot.
  - added the configuration of HeartBeat(keep-alive), SSH2 keyboard-interactive on TTSSH setup dialog

2005.1.30 (Ver 1.08)
  - added support of keyboard-interactive authentication. You can configurate the its authentication enabling flag by "KeyboardInteractive" entry of TTSSH section in the 'teraterm.ini' file.
  - added show of Project Homepage URL and OpenSSL version at version dialog.
  - fixed the application fault of pushing enter key before be grayed at rhosts and TIS in SSH2 authentication dailog.
  - added support of public-key authentication auto-login. And also added /auth=publickey and /keyfile command line option.

2005.1.6 (Ver 1.07)
  - fixed application fault when changing terminal size on telnet connection (degradation at TTSSH version supporting SSH2 1.06)

2005.1.5 (Ver 1.06)
  - added support of changing terminal size 
  - fixed application fault at DH key re-creation (degradation at TTSSH version supporting SSH2 1.05)

2004.12.27 (Ver 1.05)
  - fixed the bug of can't connect the server after disconnecting when 'Auto window close' function is disabled
  - fixed the memory leak of SSH2 KEX procedure
  - upgraded the error message string when reading the SSH2 private key is failure

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


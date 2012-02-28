#define AppName "Tera Term"
#define AppVer "4.73"
;#define snapshot GetDateTimeString('yyyymmdd_hhnnss', '', '');

[InnoIDE_PreCompile]
Name: makechm.bat
;Name: build.bat
Name: build.bat; Parameters: rebuild

[InnoIDE_PostCompile]
Name: makearchive.bat; Parameters: release

[PreCompile]
Name: makechm.bat
;Name: build.bat
Name: build.bat; Parameters: rebuild

[PostCompile]
Name: makearchive.bat; Parameters: release

[_ISToolPreCompile]
Name: makechm.bat
;Name: build.bat
Name: build.bat; Parameters: rebuild

[_ISToolPostCompile]
Name: makearchive.bat; Parameters: release

[Setup]
AppCopyright=TeraTerm Project
AppName={#AppName}
#ifndef snapshot
AppVerName={#AppName} {#AppVer}
#else
AppVerName={#AppName} {#AppVer}+ snapshot-{#snapshot}
#endif
LicenseFile=release\license.txt
DefaultDirName={pf}\teraterm
DefaultGroupName={#AppName}
ShowLanguageDialog=yes
AllowNoIcons=true
UninstallDisplayIcon={app}\ttermpro.exe
#ifndef snapshot
OutputBaseFilename=teraterm-{#AppVer}
#else
OutputBaseFilename=teraterm-{#snapshot}
#endif
PrivilegesRequired=none

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: ja; MessagesFile: compiler:Languages\Japanese.isl

[Dirs]
Name: {app}\Collector; Components: Collector
Name: {app}\theme; Components: TeraTerm
Name: {app}\theme\scale; Components: TeraTerm
Name: {app}\theme\tile; Components: TeraTerm
Name: {app}\plugin; Components: TeraTerm
Name: {app}\lang; Components: TeraTerm

[Files]
Source: ..\teraterm\release\ttermpro.exe; DestDir: {app}; Components: TeraTerm; Flags: ignoreversion
Source: ..\teraterm\release\ttpcmn.dll; DestDir: {app}; Components: TeraTerm
Source: ..\teraterm\release\ttpdlg.dll; DestDir: {app}; Components: TeraTerm
Source: ..\teraterm\release\ttpfile.dll; DestDir: {app}; Components: TeraTerm
Source: ..\teraterm\release\ttpset.dll; DestDir: {app}; Components: TeraTerm
Source: ..\teraterm\release\ttptek.dll; DestDir: {app}; Components: TeraTerm
Source: release\TERATERM.INI; DestDir: {app}; Components: TeraTerm; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: release\TSPECIAL1.TTF; DestDir: {fonts}; Components: TeraTerm; Attribs: readonly; Flags: overwritereadonly uninsneveruninstall; FontInstall: Tera Special; Check: isAbleToInstallFont
Source: ..\doc\en\teraterm.chm; DestDir: {app}; Components: TeraTerm
Source: ..\doc\ja\teratermj.chm; DestDir: {app}; Components: TeraTerm
Source: release\license.txt; DestDir: {app}; Components: TeraTerm
Source: release\FUNCTION.CNF; DestDir: {app}; Components: TeraTerm
Source: release\IBMKEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: release\EDITOR.CNF; DestDir: {app}; Components: TeraTerm; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify; DestName: KEYBOARD.CNF
Source: release\EDITOR.CNF; DestDir: {app}; Components: TeraTerm
Source: release\NT98KEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: release\PC98KEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\teraterm\release\keycode.exe; DestDir: {app}; Components: TeraTerm
Source: ..\teraterm\release\ttpmacro.exe; DestDir: {app}; Components: TeraTerm; Flags: ignoreversion
Source: release\delpassw.ttl; DestDir: {app}; Components: TeraTerm
Source: release\dialup.ttl; DestDir: {app}; Components: TeraTerm
Source: release\login.ttl; DestDir: {app}; Components: TeraTerm
Source: release\mpause.ttl; DestDir: {app}; Components: TeraTerm
Source: release\random.ttl; DestDir: {app}; Components: TeraTerm
Source: release\screencapture.ttl; DestDir: {app}; Components: TeraTerm
Source: release\ssh2login.ttl; DestDir: {app}; Components: TeraTerm
Source: release\wait_regex.ttl; DestDir: {app}; Components: TeraTerm
Source: release\lang\Default.lng; DestDir: {app}\lang; Components: TeraTerm; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: release\lang\Japanese.lng; DestDir: {app}\lang; Components: TeraTerm; Attribs: readonly; Flags: uninsremovereadonly overwritereadonly
Source: release\lang\German.lng; DestDir: {app}\lang; Components: TeraTerm; Attribs: readonly; Flags: uninsremovereadonly overwritereadonly
Source: release\lang\French.lng; DestDir: {app}\lang; Components: TeraTerm; Attribs: readonly; Flags: uninsremovereadonly overwritereadonly
Source: ..\ttssh2\ttxssh\Release\ttxssh.dll; DestDir: {app}; Components: TTSSH; Flags: ignoreversion
Source: release\ssh_known_hosts; DestDir: {app}; Components: TTSSH; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: ..\cygterm\cygterm.exe; DestDir: {app}; Components: cygterm
Source: ..\cygterm\cygterm.cfg; DestDir: {app}; Components: cygterm; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: ..\cygterm\cyglaunch.exe; DestDir: {app}; Components: cygterm
Source: ..\cygterm\cygterm+.tar.gz; DestDir: {app}; Components: cygterm
Source: ..\libs\logmett\Setup_LogMeTT_2_9_9_2.exe; DestDir: {tmp}; Components: LogMeTT; Flags: deleteafterinstall
Source: ..\libs\logmett\Setup_TTLEditor_1_4_41.exe; DestDir: {tmp}; Components: TTLEdit; Flags: deleteafterinstall
Source: ..\ttpmenu\Release\ttpmenu.exe; DestDir: {app}; Components: TeraTerm_Menu; Flags: ignoreversion
Source: release\ttmenu_readme-j.txt; DestDir: {app}; Components: TeraTerm_Menu
Source: ..\TTProxy\Release\TTXProxy.dll; DestDir: {app}; Components: TTProxy; Flags: ignoreversion
Source: release\theme\Advanced.sample; DestDir: {app}\theme\; Components: TeraTerm
Source: release\theme\Scale.INI; DestDir: {app}\theme\; Components: TeraTerm
Source: release\theme\Tile.INI; DestDir: {app}\theme\; Components: TeraTerm
Source: release\theme\scale\23.jpg; DestDir: {app}\theme\scale; Components: TeraTerm
Source: release\theme\scale\43.jpg; DestDir: {app}\theme\scale; Components: TeraTerm
Source: release\theme\tile\03.jpg; DestDir: {app}\theme\tile; Components: TeraTerm
Source: release\theme\tile\44.jpg; DestDir: {app}\theme\tile; Components: TeraTerm
Source: release\plugin\ttAKJpeg.dll; DestDir: {app}\plugin\; Components: TeraTerm
Source: release\plugin\ttAKJpeg.txt; DestDir: {app}\plugin\; Components: TeraTerm
Source: release\Collector\Collector.exe; DestDir: {app}\Collector\; Components: Collector
Source: release\Collector\collector.ini; DestDir: {app}\Collector\; Components: Collector
Source: release\Collector\Collector_org.exe; DestDir: {app}\Collector\; Components: Collector
Source: release\Collector\hthook.dll; DestDir: {app}\Collector\; Components: Collector
Source: release\Collector\mfc70.dll; DestDir: {app}\Collector\; Components: Collector
Source: release\Collector\msvcr70.dll; DestDir: {app}\Collector\; Components: Collector
Source: release\Collector\readme.txt; DestDir: {app}\Collector\; Components: Collector
Source: ..\TTXKanjiMenu\release\ttxkanjimenu.dll; DestDir: {app}\; Components: Additional_Plugins/TTXKanjiMenu; Flags: ignoreversion
Source: ..\TTXSamples\release\TTXResizeMenu.dll; DestDir: {app}\; Components: Additional_Plugins/TTXResizeMenu; Flags: ignoreversion
Source: ..\TTXSamples\release\TTXttyrec.dll; DestDir: {app}\; Components: Additional_Plugins/TTXttyrec; Flags: ignoreversion
Source: ..\TTXSamples\release\TTXttyplay.dll; DestDir: {app}\; Components: Additional_Plugins/TTXttyrec; Flags: ignoreversion
Source: ..\TTXSamples\release\TTXKcodeChange.dll; DestDir: {app}\; Components: Additional_Plugins/TTXKcodeChange; Flags: ignoreversion
Source: ..\TTXSamples\release\TTXViewMode.dll; DestDir: {app}\; Components: Additional_Plugins/TTXViewMode; Flags: ignoreversion
Source: ..\TTXSamples\release\TTXAlwaysOnTop.dll; DestDir: {app}\; Components: Additional_Plugins/TTXAlwaysOnTop; Flags: ignoreversion
Source: ..\TTXSamples\release\TTXRecurringCommand.dll; DestDir: {app}\; Components: Additional_Plugins/TTXRecurringCommand; Flags: ignoreversion

[Types]
Name: standard; Description: {cm:type_standard}
Name: full; Description: {cm:type_full}
Name: compact; Description: {cm:type_compact}
Name: custom; Description: {cm:type_custom}; Flags: iscustom

[Components]
Name: TeraTerm; Description: Tera Term & Macro; Flags: fixed; Types: custom compact full standard
Name: TTSSH; Description: TTSSH; Types: compact full standard
Name: cygterm; Description: CygTerm+; Types: full standard; Check: not isIA64
Name: LogMeTT; Description: LogMeTT ({cm:comp_installer}); Types: full; MinVersion: 4.1.1998,4.0.1381sp6
Name: TTLEdit; Description: TTLEdit ({cm:comp_installer}); Types: full; MinVersion: 4.1.1998,4.0.1381sp6
Name: TeraTerm_Menu; Description: TeraTerm Menu; Types: full
Name: TTProxy; Description: TTProxy; Types: full standard
Name: Collector; Description: Collector; Types: full
Name: Additional_Plugins; Description: {cm:comp_TTX}
Name: Additional_Plugins/TTXResizeMenu; Description: TTXResizeMenu ({cm:comp_TTXResizeMenu}); Types: full standard
Name: Additional_Plugins/TTXttyrec; Description: TTXttyrec ({cm:comp_TTXttyrec}); Types: full standard
Name: Additional_Plugins/TTXKanjiMenu; Description: TTXKanjiMenu ({cm:comp_TTXKanjiMenu}); Languages: en
Name: Additional_Plugins/TTXKanjiMenu; Description: TTXKanjiMenu ({cm:comp_TTXKanjiMenu}); Types: full; Languages: ja
Name: Additional_Plugins/TTXKcodeChange; Description: TTXKcodeChange ({cm:comp_TTXKcodeChange}); Languages: en
Name: Additional_Plugins/TTXKcodeChange; Description: TTXKcodeChange ({cm:comp_TTXKcodeChange}); Types: full; Languages: ja
Name: Additional_Plugins/TTXViewMode; Description: TTXViewMode ({cm:comp_TTXViewMode}); Types: full
Name: Additional_Plugins/TTXAlwaysOnTop; Description: TTXAlwaysOnTop ({cm:comp_TTXAlwaysOnTop}); Types: full
Name: Additional_Plugins/TTXRecurringCommand; Description: TTXRecurringCommand ({cm:comp_TTXRecurringCommand}); Types: full

[Icons]
Name: {group}\Tera Term; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; IconIndex: 0; Components: TeraTerm; Flags: createonlyiffileexists
Name: {group}\{cm:UninstallProgram,{#AppName}}; Filename: {uninstallexe}; Components: TeraTerm; Flags: createonlyiffileexists
Name: {group}\cyglaunch; Filename: {app}\cyglaunch.exe; WorkingDir: {app}; IconFilename: {app}\cyglaunch.exe; IconIndex: 0; Components: cygterm; Flags: createonlyiffileexists
Name: {group}\TeraTerm Menu; Filename: {app}\ttpmenu.exe; WorkingDir: {app}; IconFilename: {app}\ttpmenu.exe; IconIndex: 0; Components: TeraTerm_Menu; Flags: createonlyiffileexists
Name: {group}\Collector; Filename: {app}\Collector\Collector.exe; WorkingDir: {app}\Collector; IconFilename: {app}\Collector\Collector.exe; IconIndex: 0; Components: Collector; Flags: createonlyiffileexists
Name: {userdesktop}\Tera Term; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; Components: TeraTerm; Tasks: desktopicon; IconIndex: 0; Flags: createonlyiffileexists
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\Tera Term; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; Components: TeraTerm; Tasks: quicklaunchicon; IconIndex: 0; Flags: createonlyiffileexists
Name: {userstartup}\TeraTerm Menu; Filename: {app}\ttpmenu.exe; WorkingDir: {app}; IconFilename: {app}\ttpmenu.exe; Components: TeraTerm_Menu; IconIndex: 0; Tasks: startupttmenuicon; Flags: createonlyiffileexists
Name: {userstartup}\Collector; Filename: {app}\collector\collector.exe; WorkingDir: {app}\Collector; IconFilename: {app}\collector\collector.exe; Components: Collector; Tasks: startupcollectoricon; IconIndex: 0; Flags: createonlyiffileexists
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\cyglaunch; Filename: {app}\cyglaunch.exe; WorkingDir: {app}; IconFilename: {app}\cyglaunch.exe; Components: cygterm; Tasks: quickcyglaunch; IconIndex: 0; Flags: createonlyiffileexists

[Registry]
; Cygterm Here
Root: HKCU; Subkey: Software\Classes\Folder\shell\cygterm; ValueType: string; ValueData: Cy&gterm Here; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: cygterm; Tasks: cygtermhere
Root: HKCU; Subkey: Software\Classes\Folder\shell\cygterm\command; ValueType: string; ValueData: """{app}\cyglaunch.exe"" -nocd -v CHERE_INVOKING=y -d ""\""%L\"""""; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: cygterm; Tasks: cygtermhere
Root: HKCR; Subkey: Folder\shell\cygterm; ValueType: string; ValueData: Cy&gterm Here; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: cygterm; Tasks: cygtermhere
Root: HKCR; Subkey: Folder\shell\cygterm\command; ValueType: string; ValueData: """{app}\cyglaunch.exe"" -nocd -v CHERE_INVOKING=y -d ""\""%L\"""""; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: cygterm; Tasks: cygtermhere
; Cygterm Here from folder Background
; MinVersion 5.00.0000 = not Win9x, 6.0.6000 = Vista or later
Root: HKCU; Subkey: Software\Classes\Directory\Background\shell\cygterm; ValueType: string; ValueData: Cy&gterm Here; Flags: uninsdeletekey; Components: cygterm; Tasks: cygtermhere; MinVersion: 5.00.0000,6.0.6000
Root: HKCU; Subkey: Software\Classes\Directory\Background\shell\cygterm\command; ValueType: string; ValueData: """{app}\cyglaunch.exe"" -nocd -v CHERE_INVOKING=y -d ""\""%V\"""""; Flags: uninsdeletekey; Components: cygterm; Tasks: cygtermhere; MinVersion: 5.00.0000,6.0.6000
Root: HKCU; Subkey: Software\Classes\LibraryFolder\Background\shell\cygterm; ValueType: string; ValueData: Cy&gterm Here; Flags: uninsdeletekey; Components: cygterm; Tasks: cygtermhere; MinVersion: 5.00.0000,6.0.6000
Root: HKCU; Subkey: Software\Classes\LibraryFolder\Background\shell\cygterm\command; ValueType: string; ValueData: """{app}\cyglaunch.exe"" -nocd -v CHERE_INVOKING=y -d ""\""%V\"""""; Flags: uninsdeletekey; Components: cygterm; Tasks: cygtermhere; MinVersion: 5.00.0000,6.0.6000
; Associate with .TTL
Root: HKCU; Subkey: Software\Classes\.ttl; ValueType: string; ValueData: TeraTerm.MacroFile; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
Root: HKCU; Subkey: Software\Classes\TeraTerm.MacroFile; ValueType: string; ValueData: Tera Term Macro File; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
Root: HKCU; Subkey: Software\Classes\TeraTerm.MacroFile\DefaultIcon; ValueType: string; ValueData: {app}\ttpmacro.exe,0; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
Root: HKCU; Subkey: Software\Classes\TeraTerm.MacroFile\shell\open\command; ValueType: string; ValueData: """{app}\ttpmacro.exe"" ""%1"""; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
Root: HKCR; Subkey: .ttl; ValueType: string; ValueData: TeraTerm.MacroFile; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
Root: HKCR; Subkey: TeraTerm.MacroFile; ValueType: string; ValueData: Tera Term Macro File; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
Root: HKCR; Subkey: TeraTerm.MacroFile\DefaultIcon; ValueType: string; ValueData: {app}\ttpmacro.exe,0; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
Root: HKCR; Subkey: TeraTerm.MacroFile\shell\open\command; ValueType: string; ValueData: """{app}\ttpmacro.exe"" ""%1"""; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TeraTerm; Tasks: macroassoc
; Associate with telnet://
Root: HKCU; Subkey: Software\Classes\telnet\shell; ValueType: string; ValueData: Open with Tera Term; Flags: uninsclearvalue; Check: isWin2kOrLater; Components: TeraTerm; Tasks: telnetassoc
Root: HKCU; Subkey: Software\Classes\telnet\shell\Open with Tera Term\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" /T=1 /nossh %1"; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TeraTerm; Tasks: telnetassoc
Root: HKCR; Subkey: telnet\shell; ValueType: string; ValueData: Open with Tera Term; Flags: uninsclearvalue; Check: not isWin2kOrLater; Components: TeraTerm; Tasks: telnetassoc
Root: HKCR; Subkey: telnet\shell\Open with Tera Term\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" /T=1 /nossh %1"; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TeraTerm; Tasks: telnetassoc
; Associate with ssh://
Root: HKCU; Subkey: Software\Classes\ssh; ValueType: string; ValueData: URL: SSH Protocol; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\ssh; ValueName: URL Protocol; ValueType: string; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\ssh; ValueName: EditFlags; ValueType: dword; ValueData: 2; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\ssh\DefaultIcon; ValueType: string; ValueData: """{app}\ttxssh.dll"",0"; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\ssh\shell\open\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" %1"; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: ssh; ValueType: string; ValueData: URL: SSH Protocol; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: ssh; ValueName: URL Protocol; ValueType: string; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: ssh; ValueName: EditFlags; ValueType: dword; ValueData: 2; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: ssh\DefaultIcon; ValueType: string; ValueData: """{app}\ttxssh.dll"",0"; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: ssh\shell\open\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" %1"; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
; Associate with slogin://
Root: HKCU; Subkey: Software\Classes\slogin; ValueType: string; ValueData: URL: slogin Protocol; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\slogin; ValueName: URL Protocol; ValueType: string; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\slogin; ValueName: EditFlags; ValueType: dword; ValueData: 2; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\slogin\DefaultIcon; ValueType: string; ValueData: """{app}\ttxssh.dll"",0"; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCU; Subkey: Software\Classes\slogin\shell\open\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" %1"; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: slogin; ValueType: string; ValueData: URL: slogin Protocol; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: slogin; ValueName: URL Protocol; ValueType: string; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: slogin; ValueName: EditFlags; ValueType: dword; ValueData: 2; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: slogin\DefaultIcon; ValueType: string; ValueData: """{app}\ttxssh.dll"",0"; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
Root: HKCR; Subkey: slogin\shell\open\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" %1"; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: TTSSH; Tasks: sshassoc
; Associate with .TTY
Root: HKCU; Subkey: Software\Classes\.tty; ValueType: string; ValueData: TTYRecordFile; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc
Root: HKCU; Subkey: Software\Classes\TTYRecordFile; ValueType: string; ValueData: TTY Record File; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc
Root: HKCU; Subkey: Software\Classes\TTYRecordFile\DefaultIcon; ValueType: string; ValueData: {app}\ttermpro.exe,0; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc
Root: HKCU; Subkey: Software\Classes\TTYRecordFile\shell\open\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" /R=""%1"" /TTYPLAY"; Flags: uninsdeletekey; Check: isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc
Root: HKCR; Subkey: .tty; ValueType: string; ValueData: TTYRecordFile; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc
Root: HKCR; Subkey: TTYRecordFile; ValueType: string; ValueData: TTY Record File; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc
Root: HKCR; Subkey: TTYRecordFile\DefaultIcon; ValueType: string; ValueData: {app}\ttermpro.exe,0; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc
Root: HKCR; Subkey: TTYRecordFile\shell\open\command; ValueType: string; ValueData: """{app}\ttermpro.exe"" /R=""%1"" /TTYPLAY"; Flags: uninsdeletekey; Check: not isWin2kOrLater; Components: Additional_Plugins/TTXttyrec; Tasks: ttyplayassoc

[Tasks]
Name: desktopicon; Description: {cm:task_desktopicon}; Components: TeraTerm
Name: quicklaunchicon; Description: {cm:task_quicklaunchicon}; Components: TeraTerm
Name: startupttmenuicon; Description: {cm:task_startupttmenuicon}; Components: TeraTerm_Menu
Name: startupcollectoricon; Description: {cm:task_startupcollectoricon}; Components: Collector
Name: cygtermhere; Description: {cm:task_cygtermhere}; Components: cygterm; Flags: unchecked
Name: quickcyglaunch; Description: {cm:task_quickcyglaunch}; Components: cygterm; Flags: unchecked
Name: macroassoc; Description: {cm:task_macroassoc}; Components: TeraTerm; Flags: unchecked
Name: telnetassoc; Description: {cm:task_telnetassoc}; Components: TeraTerm; Flags: unchecked
Name: sshassoc; Description: {cm:task_sshassoc}; Components: TTSSH; Flags: unchecked
Name: ttyplayassoc; Description: {cm:task_ttyplayassoc}; Components: Additional_Plugins/TTXttyrec; Flags: unchecked

[Run]
Filename: "{app}\ttermpro.exe"; Flags: nowait postinstall skipifsilent unchecked; Description: "{cm:launch_teraterm}"; Components: TeraTerm
Filename: "{tmp}\Setup_LogMeTT_2_9_9_2.exe"; Flags: skipifsilent; Components: LogMeTT
Filename: "{tmp}\Setup_LogMeTT_2_9_9_2.exe"; Parameters: "/SILENT"; Flags: skipifnotsilent; Components: LogMeTT
Filename: "{tmp}\Setup_TTLEditor_1_4_41.exe"; Flags: skipifsilent; Components: TTLEdit
Filename: "{tmp}\Setup_TTLEditor_1_4_41.exe"; Parameters: "/SILENT"; Flags: skipifnotsilent; Components: TTLEdit
Filename: "{app}\ttpmenu.exe"; Flags: nowait postinstall skipifsilent unchecked; Description: "{cm:launch_ttmenu}"; Components: TeraTerm_Menu
Filename: "{app}\Collector\Collector.exe"; Flags: nowait postinstall skipifsilent unchecked; Description: "{cm:launch_collector}"; Components: Collector

[CustomMessages]
en.task_desktopicon=Create Tera Term shortcut to &Desktop
en.task_quicklaunchicon=Create Tera Term shortcut to &Quick Launch
en.task_startupttmenuicon=Create TeraTerm &Menu shortcut to Startup
en.task_startupcollectoricon=Create &Collector shortcut to Startup
en.task_cygtermhere=Add "Cy&gterm Here" to Context menu
en.task_quickcyglaunch=Create cyg&launch shortcut to Quick Launch
en.task_macroassoc=Associate .&ttl file to ttpmacro.exe
en.task_telnetassoc=Associate t&elnet protocol to ttermpro.exe
en.task_sshassoc=Associate &ssh protocol to ttermpro.exe
en.task_ttyplayassoc=Associate .tty file to tterm&pro.exe
ja.task_desktopicon=�f�X�N�g�b�v�� Tera Term �̃V���[�g�J�b�g�����(&D)
ja.task_quicklaunchicon=�N�C�b�N�N���� Tera Term �̃V���[�g�J�b�g�����(&Q)
ja.task_startupttmenuicon=�X�^�[�g�A�b�v�� TeraTerm &Menu �̃V���[�g�J�b�g�����
ja.task_startupcollectoricon=�X�^�[�g�A�b�v�� &Collector �̃V���[�g�J�b�g�����
ja.task_cygtermhere=�R���e�L�X�g���j���[�� "Cy&gterm Here" ��ǉ�����
ja.task_quickcyglaunch=�N�C�b�N�N���� cyg&launch �̃V���[�g�J�b�g�����
ja.task_macroassoc=.&ttl �t�@�C���� ttpmacro.exe �Ɋ֘A�t����
ja.task_telnetassoc=t&elnet �v���g�R���� ttermpro.exe �Ɋ֘A�t����
ja.task_sshassoc=&ssh �v���g�R���� ttermpro.exe �Ɋ֘A�t����
ja.task_ttyplayassoc=.tty �t�@�C���� tterm&pro.exe �Ɋ֘A�t����
en.type_standard=Standard installation
en.type_full=Full installation
en.type_compact=Compact installation
en.type_custom=Custom installation
ja.type_standard=�W���C���X�g�[��
ja.type_full=�t���C���X�g�[��
ja.type_compact=�R���p�N�g�C���X�g�[��
ja.type_custom=�J�X�^���C���X�g�[��
en.launch_teraterm=Launch &Tera Term
en.launch_ttmenu=Launch TeraTerm &Menu
en.launch_collector=Launch &Collector
ja.launch_teraterm=������ &Tera Term �����s����
ja.launch_ttmenu=������ TeraTerm &Menu �����s����
ja.launch_collector=������ &Collector �����s����
en.msg_language_caption=Select Language
en.msg_language_description=Which language shoud be used?
en.msg_language_subcaption=Select the language of application's menu and dialog, then click Next.
en.msg_language_none=&English
en.msg_language_japanese=&Japanese
en.msg_language_german=&German
en.msg_language_french=&French
ja.msg_language_caption=����̑I��
ja.msg_language_description=���[�U�[�C���^�[�t�F�[�X�̌����I�����Ă��������B
ja.msg_language_subcaption=�A�v���P�[�V�����̃��j���[��_�C�A���O���̕\�������I�����āA�u���ցv���N���b�N���Ă��������B
ja.msg_language_none=�p��(&E)
ja.msg_language_japanese=���{��(&J)
ja.msg_language_german=�h�C�c��(&G)
ja.msg_language_french=�t�����X��(&F)
en.msg_del_confirm=Are you sure that you want to delete %s ?
ja.msg_del_confirm=%s ���폜���܂����H
en.msg_uninstall_confirm=It seems a former version is installed. You are recommended to uninstall it previously. Do you uninstall former version ?
ja.msg_uninstall_confirm=�ȑO�̃o�[�W�������C���X�g�[������Ă���悤�ł��B��ɃA���C���X�g�[�����邱�Ƃ������߂��܂��B�A���C���X�g�[�����܂����H
en.comp_TTX=Additional Plugins
ja.comp_TTX=�ǉ��v���O�C��
en.comp_TTXResizeMenu=VT-Window size can be changed from preset
ja.comp_TTXResizeMenu=VT�E�B���h�E�̃T�C�Y���v���Z�b�g�l�̒�����ύX�ł���悤�ɂ���
en.comp_TTXttyrec=ttyrec format record data can be recorded or playback
ja.comp_TTXttyrec=ttyrec�`���̘^��f�[�^���L�^/�Đ��ł���悤�ɂ���
en.comp_TTXKanjiMenu=Changes Japanese Kanji Code from VT-Window menu
ja.comp_TTXKanjiMenu=���{��̊����R�[�h��VT�E�B���h�E�̃��j���[����ݒ�ł���悤�ɂ���
en.comp_TTXKcodeChange=Change Japanese Kanji code by remote sequence
ja.comp_TTXKcodeChange=�����[�g����̃V�[�P���X�œ��{��̊����R�[�h��ύX����
en.comp_TTXViewMode=View-only mode can be used
ja.comp_TTXViewMode=�\����p���[�h�ɂ��邱�Ƃ��ł���
en.comp_TTXAlwaysOnTop=Always On Top can be used
ja.comp_TTXAlwaysOnTop=��ɍőO�ʂɕ\���ł���悤�ɂ���
en.comp_TTXRecurringCommand=Recurring Command can be used
ja.comp_TTXRecurringCommand=����I�ɕ�����𑗐M����
en.comp_installer=Other installer is started
ja.comp_installer=�C���X�g�[�����N�����܂�
en.msg_AppRunningError=Setup has detected that %s is currently running.%n%nPlease close all instances of it now, then click Next to continue.
ja.msg_AppRunningError=�Z�b�g�A�b�v�͎��s���� %s �����o���܂����B%n%n�J���Ă���A�v���P�[�V���������ׂĕ��Ă���u���ցv���N���b�N���Ă��������B

[Code]
const
  SHCNF_IDLIST = $0000;
  SHCNE_ASSOCCHANGED = $08000000;

procedure SHChangeNotify(wEventId, uFlags, dwItem1, dwItem2: Integer);
external 'SHChangeNotify@shell32.dll stdcall';

var
  UILangFilePage: TInputOptionWizardPage;


// It is different from x64
function isIA64 : Boolean;
begin
  if ProcessorArchitecture = paIA64 then
    Result := True
  else
    Result := False;
end;

// Windows 2000 or later
function isWin2kOrLater : Boolean;
var
  Version: TWindowsVersion;
begin;
  GetWindowsVersionEx(Version);
  if Version.Major >= 5 then
    Result := True
  else
    Result := False;
end;

// Window 9x or NT 4.0 or (Admins or PowerUsers)
function isAbleToInstallFont : Boolean;
begin;
  if not UsingWinNT() then begin
    Result := True;
  end else begin
    if isWin2kOrLater() then begin
      if IsAdminLoggedOn() or IsPowerUserLoggedOn() then begin
        Result := True;
      end else begin
        Result := False
      end;
    end else begin
      Result := True;
    end;
  end;
end;

{
// If value is not found in INI, returns ""
function GetIniString2(Section:String; Key:String; Filename:String) : String;
var
  Default1: String;
  Default2: String;
  Value1:   String;
  Value2:   String;
begin
  Default1 := 'on';
  Default2 := 'off';
  Value1 := GetIniString(Section, Key, Default1, Filename);
  Value2 := GetIniString(Section, Key, Default2, Filename);

  if Value1 <> Value2 then
    // no value in INI
    Result := ''
  else
    // value in INI
    Result := Value1
end;
}

function CheckFileUsing(Filename:String) : integer;
var
  TmpFileName : String;
begin
  if FileExists(FileName) then
    begin
      TmpFileName := FileName + '.' + GetDateTimeString('yyyymmddhhnnss', #0, #0); // Tmp file ends with timestamp
      if FileCopy(FileName, TmpFileName, True) then
        if DeleteFile(FileName) then
          if RenameFile(TmpFileName, FileName) then
            Result := 0
          else
            Result := -1 // permission?
        else
          begin
            Result := 1; // failed to delete
            DeleteFile(TmpFileName);
          end
      else
        Result := -1 // permission?
    end
  else
    Result := 0;
end;

function CheckAppsUsing() : string;
var
  FileDir  : String;
  FileName : array[0..6] of String;
  FileDesc : array[0..6] of String;
  i        : integer;
begin
  FileDir := ExpandConstant('{app}');
  FileName[0] := FileDir + '\ttermpro.exe';
  FileName[1] := FileDir + '\ttpmacro.exe';
  FileName[2] := FileDir + '\keycode.exe';
  FileName[3] := FileDir + '\ttpmenu.exe';
  FileName[4] := FileDir + '\cygterm.exe';
  FileName[5] := FileDir + '\Collector.exe';
  FileName[6] := FileDir + '\Collector_org.exe';
  FileDesc[0] := 'Tera Term';
  FileDesc[1] := 'Tera Term Macro';
  FileDesc[2] := 'Keycode';
  FileDesc[3] := 'TeraTerm Menu';
  FileDesc[4] := 'CygTerm+';
  FileDesc[5] := 'Collector';
  FileDesc[6] := 'Collector';

  for i := 0 to 6 do
  begin
    case CheckFileUsing(FileName[i]) of
      1:
        // Failed to delete. In use.
        begin
          if Length(Result) > 0 then
            Result := Result + ', ' + FileDesc[i]
          else
            Result := FileDesc[i]
        end;
      else
        // -1: Failed to copy/rename
        //  0: OK
        // NOP
    end;
  end;

end;

function GetDefaultIniFilename : String;
begin
  Result := ExpandConstant('{app}') + '\TERATERM.INI';
end;

procedure SetIniFile(iniFile: String);
var
  Language      : String;
  Locale        : String;
  CodePage      : integer;
  VTFont        : String;
  TEKFont       : String;
  FileDir       : String;
  TCPPort       : integer;
  ViewlogEditor : String;

begin
  Language       := GetIniString('Tera Term', 'Language', '', iniFile);
  Locale         := GetIniString('Tera Term', 'Locale', '', iniFile);
  CodePage       := GetIniInt('Tera Term', 'CodePage', 0, 0, 0, iniFile);
  VTFont         := GetIniString('Tera Term', 'VTFont', '', iniFile);
  TEKFont        := GetIniString('Tera Term', 'TEKFont', '', iniFile);
  FileDir        := GetIniString('Tera Term', 'FileDir', '', iniFile);
  TCPPort        := GetIniInt('Tera Term', 'TCPPort', 0, 0, 65535, iniFile)
  ViewlogEditor  := GetIniString('Tera Term', 'ViewlogEditor', '', iniFile);

  case GetUILanguage and $3FF of
  $04: // Chinese
    begin
      if Length(Language) = 0 then
        SetIniString('Tera Term', 'Language', 'UTF-8', iniFile);
      if Length(Locale) = 0 then
        SetIniString('Tera Term', 'Locale', 'chs', iniFile);
      if CodePage = 0 then
        SetIniInt('Tera Term', 'CodePage', 936, iniFile);
      if Length(VTFont) = 0 then
        SetIniString('Tera Term', 'VTFont', 'Terminal,0,-12,255', iniFile);
      if Length(TEKFont) = 0 then
        SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,255', iniFile);
    end;
  $11: // Japanese
    begin
      if Length(Language) = 0 then
        SetIniString('Tera Term', 'Language', 'Japanese', iniFile);
      if Length(Locale) = 0 then
        SetIniString('Tera Term', 'Locale', 'japanese', iniFile);
      if CodePage = 0 then
        SetIniInt('Tera Term', 'CodePage', 932, iniFile);
      if Length(VTFont) = 0 then
        SetIniString('Tera Term', 'VTFont', 'Terminal,0,-19,128', iniFile);
      if Length(TEKFont) = 0 then
        SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,128', iniFile);
    end;
  $12: // Korean
    begin
      if Length(Language) = 0 then
        SetIniString('Tera Term', 'Language', 'Korean', iniFile);
      if Length(Locale) = 0 then
        SetIniString('Tera Term', 'Locale', 'korean', iniFile);
      if CodePage = 0 then
        SetIniInt('Tera Term', 'CodePage', 949, iniFile);
      if Length(VTFont) = 0 then
        SetIniString('Tera Term', 'VTFont', 'Terminal,0,-12,255', iniFile);
      if Length(TEKFont) = 0 then
        SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,255', iniFile);
    end;
  $19: // Russian
    begin
      if Length(Language) = 0 then
        SetIniString('Tera Term', 'Language', 'Russian', iniFile);
      if Length(Locale) = 0 then
        SetIniString('Tera Term', 'Locale', 'russian', iniFile);
      if CodePage = 0 then
        SetIniInt('Tera Term', 'CodePage', 1251, iniFile);
      if Length(VTFont) = 0 then
        SetIniString('Tera Term', 'VTFont', 'Terminal,0,-12,255', iniFile);
      if Length(TEKFont) = 0 then
        SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,255', iniFile);
    end;
  else // Other
    begin

      if GetUILanguage = $409 then begin // en-US

        if Length(Language) = 0 then
          SetIniString('Tera Term', 'Language', 'UTF-8', iniFile);
        if Length(Locale) = 0 then
          SetIniString('Tera Term', 'Locale', 'american', iniFile);
        if CodePage = 0 then
          SetIniInt('Tera Term', 'CodePage', 65001, iniFile);

      end else begin // Other

        if Length(Language) = 0 then
          SetIniString('Tera Term', 'Language', 'English', iniFile);
        if Length(Locale) = 0 then
          SetIniString('Tera Term', 'Locale', 'english', iniFile);
        if CodePage = 0 then
          SetIniInt('Tera Term', 'CodePage', 1252, iniFile);

      end;

      if Length(VTFont) = 0 then
        SetIniString('Tera Term', 'VTFont', 'Terminal,0,-12,255', iniFile);
      if Length(TEKFont) = 0 then
        SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,255', iniFile);
    end;
  end;

  case UILangFilePage.SelectedValueIndex of
    1:
      SetIniString('Tera Term', 'UILanguageFile', 'lang\Japanese.lng', iniFile);
    2:
      SetIniString('Tera Term', 'UILanguageFile', 'lang\German.lng', iniFile);
    3:
      SetIniString('Tera Term', 'UILanguageFile', 'lang\French.lng', iniFile);
    else
      SetIniString('Tera Term', 'UILanguageFile', 'lang\Default.lng', iniFile);
  end;

  if Length(FileDir) = 0 then begin
    FileDir := ExpandConstant('{app}');
    SetIniString('Tera Term', 'FileDir', FileDir, iniFile);
  end;

  if TCPPort = 0 then begin
    if IsComponentSelected('TTSSH') then
      SetIniInt('Tera Term', 'TCPPort', 22, iniFile)
    else
      SetIniInt('Tera Term', 'TCPPort', 23, iniFile);
  end;

  if ViewlogEditor = 'notepad.exe' then begin
    ViewlogEditor := ExpandConstant('{win}') + '\' + 'notepad.exe';
    SetIniString('Tera Term', 'ViewlogEditor', ViewlogEditor, iniFile);
  end;

end;

procedure InitializeWizard;
var
  UILangFilePageCaption     : String;
  UILangFilePageDescription : String;
  UILangFilePageSubCaption  : String;
  UILangFilePageNone        : String;
  UILangFilePageJapanese    : String;
  UILangFilePageGerman      : String;
  UILangFilePageFrench      : String;
begin
  UILangFilePageCaption     := CustomMessage('msg_language_caption');
  UILangFilePageDescription := CustomMessage('msg_language_description');
  UILangFilePageSubCaption  := CustomMessage('msg_language_subcaption');
  UILangFilePageNone        := CustomMessage('msg_language_none');
  UILangFilePageJapanese    := CustomMessage('msg_language_japanese');
  UILangFilePageGerman      := CustomMessage('msg_language_german');
  UILangFilePageFrench      := CustomMessage('msg_language_french');

  UILangFilePage := CreateInputOptionPage(wpSelectComponents,
    UILangFilePageCaption, UILangFilePageDescription,
    UILangFilePageSubCaption, True, False);
  UILangFilePage.Add(UILangFilePageNone);
  UILangFilePage.Add(UILangFilePageJapanese);
  UILangFilePage.Add(UILangFilePageGerman);
  UILangFilePage.Add(UILangFilePageFrench);
  case ActiveLanguage of
    'ja':
      UILangFilePage.SelectedValueIndex := 1;
    // ���̌���͍ŐV�łɒǏ]���Ă��Ȃ��̂ŁA���{�ꂾ�����ʈ���
    else
      UILangFilePage.SelectedValueIndex := 0;
  end;
end;

function NextButtonClick(CurPageID: Integer): Boolean;
var
  uninstaller  : String;
  uninstaller2 : String;
  ResultCode   : Integer;
  iniFile      : String;
  ErrMsg       : String;
begin
  Result := True;

  case CurPageID of

    wpWelcome:
      begin

        if RegQueryStringValue(HKEY_LOCAL_MACHINE,
                               'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\UTF-8 TeraTerm Pro with TTSSH2_is1',
                               'UninstallString', uninstaller) then
        begin
          // UTF-8 TeraTerm Pro with TTSSH2 �̃A���C���X�g�[��������𔭌�����
          if not RegKeyExists(HKEY_LOCAL_MACHINE,
                              'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Tera Term_is1') then
          begin
            // Tera Term �̃C���X�g�[���G���g����������Ȃ�(�������Ă��Ȃ�)
            if MsgBox(CustomMessage('msg_uninstall_confirm'), mbInformation, MB_YESNO) = IDYES then
            begin
              // ���[�U���A���C���X�g�[����I������

              // ���[�� " �����
              uninstaller2 := Copy(uninstaller, 2, Length(uninstaller) - 2);

              if not Exec(uninstaller2, '', '', SW_SHOW,
                          ewWaitUntilTerminated, ResultCode) then
              begin
                // ���s�Ɏ��s
                MsgBox(SysErrorMessage(ResultCode), mbError, MB_OK);
              end;
            end;
          end;
        end;

      end;

    wpSelectDir:
      begin

        ErrMsg := CheckAppsUsing();
        if Length(ErrMsg) > 0 then
          begin
            MsgBox(Format(CustomMessage('msg_AppRunningError'), [ErrMsg]), mbError, MB_OK);
            Result := False;
          end
        else
        // -1: goto next. Turn over to Inno Setup.
        //  0: goto next. No problem.
        // NOP
      end;

    wpSelectComponents:
      begin

        if FileExists(GetDefaultIniFileName()) then
        begin
          iniFile := Lowercase(GetIniString('Tera Term', 'UILanguageFile', '', GetDefaultIniFilename()));
          if iniFile = 'lang\japanese.lng' then
            UILangFilePage.SelectedValueIndex := 1
          else if iniFile = 'lang\german.lng' then
            UILangFilePage.SelectedValueIndex := 2
          else if iniFile = 'lang\french.lng' then
            UILangFilePage.SelectedValueIndex := 3
          else
            UILangFilePage.SelectedValueIndex := 0;
        end;

      end;
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
var
  iniFile : String;
begin
  case CurStep of
    ssDone:
      begin
        iniFile := GetDefaultIniFilename();
        SetIniFile(iniFile);

        if not IsTaskSelected('cygtermhere') then
        begin;
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Classes\Folder\shell\cygterm');
          RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT, 'Folder\shell\cygterm');
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Classes\Directory\Background\shell\cygterm');
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Classes\LibraryFolder\Background\shell\cygterm');
        end;

        if not IsTaskSelected('macroassoc') then
        begin;
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Classes\.ttl');
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Classes\TeraTerm.MacroFile');
          RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT, '.ttl');
          RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT, 'TeraTerm.MacroFile');
        end;

        if not IsTaskSelected('telnetassoc') then
        begin;
          RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\Classes\telnet\shell\Open with Tera Term');
          RegDeleteValue(HKEY_CURRENT_USER, 'Software\Classes\telnet\shell', '');
          RegDeleteKeyIncludingSubkeys(HKEY_CLASSES_ROOT, 'telnet\shell\Open with Tera Term');
          RegDeleteValue(HKEY_CLASSES_ROOT, 'telnet\shell', '');
        end;

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

      end; // ssDone
   end; // case CurStep of
end; // CurStepChanged

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ini     : array[0..4] of String;
  buf     : String;
  conf    : String;
  confmsg : String;
  app     : String;
  i, res  : integer;
begin
  case CurUninstallStep of
    usPostUninstall:
      begin
        ini[0] := '\TERATERM.INI';
        ini[1] := '\KEYBOARD.CNF';
        ini[2] := '\ssh_known_hosts';
        ini[3] := '\cygterm.cfg';
        ini[4] := '\broadcast.log';

        conf := CustomMessage('msg_del_confirm');
        app  := ExpandConstant('{app}');

        // delete config files
        for i := 0 to 4 do
        begin
          buf := app + ini[i];
          if FileExists(buf) then begin
            confmsg := Format(conf, [buf]);
            res := MsgBox(confmsg, mbInformation, MB_YESNO or MB_DEFBUTTON2);
            if res = IDYES then
              DeleteFile(buf);
          end;
        end;

        // delete registory
        if RegKeyExists(HKEY_CURRENT_USER, 'Software\ShinpeiTools\TTermMenu') then begin
          confmsg := Format(conf, ['HKEY_CURRENT_USER' + '\Software\ShinpeiTools\TTermMenu']);
          res := MsgBox(confmsg, mbInformation, MB_YESNO or MB_DEFBUTTON2);
          if res = IDYES then begin
            RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\ShinpeiTools\TTermMenu');
            RegDeleteKeyIfEmpty(HKEY_CURRENT_USER, 'Software\ShinpeiTools');
          end;
        end;

        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

        // directory is deleted only if empty
        RemoveDir(app);
      end;
  end;
end;

[InstallDelete]
Name: {app}\OpenSSH-LICENCE.txt; Type: files
Name: {app}\cygterm-README.txt; Type: files
Name: {app}\cygterm-README-j.txt; Type: files
Name: {app}\keycode.txt; Type: files
Name: {app}\keycodej.txt; Type: files
Name: {app}\RE.txt; Type: files
Name: {app}\RE-ja.txt; Type: files
Name: {app}\ssh2_readme.txt; Type: files
Name: {app}\ssh2_readme-j.txt; Type: files
Name: {app}\utf8_readme.txt; Type: files
Name: {app}\utf8_readme-j.txt; Type: files
Name: {app}\OpenSSH-LICENSE.txt; Type: files
Name: {app}\OpenSSL-LICENSE.txt; Type: files
Name: {group}\TeraTerm Document.lnk; Type: files
Name: {group}\TeraTerm Document(Japanese).lnk; Type: files
Name: {group}\TTSSH Document.lnk; Type: files
Name: {group}\TTSSH Document(Japanese).lnk; Type: files
Name: {app}\LogMeTT.hlp; Type: files
Name: {app}\macro.hlp; Type: files
Name: {app}\macroj.hlp; Type: files
Name: {app}\ttermp.hlp; Type: files
Name: {app}\ttermpj.hlp; Type: files
Name: {app}\copyfont.bat; Type: files
Name: {app}\copyfont.pif; Type: files
Name: {app}\libeay.txt; Type: files

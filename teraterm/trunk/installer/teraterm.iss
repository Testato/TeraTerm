#define AppName "UTF-8 TeraTerm Pro with TTSSH2"
#define AppVer "4.43"

[Setup]
AppCopyright=TeraTerm Project
AppName={#AppName}
AppVerName={#AppName} {#AppVer}
LicenseFile=..\release\license.txt
DefaultDirName={pf}\teraterm
DefaultGroupName={#AppName}
ShowLanguageDialog=yes
AllowNoIcons=true
SetupIconFile=
UninstallDisplayIcon={app}\ttermpro.exe
OutputBaseFilename=teraterm_utf8-{#AppVer}

[Languages]
Name: en; MessagesFile: compiler:Default.isl
Name: ja; MessagesFile: compiler:Languages\Japanese.isl

[Dirs]
Name: {app}\Collector; Components: Collector
Name: {app}\theme; Components: TeraTerm
Name: {app}\theme\scale; Components: TeraTerm
Name: {app}\theme\tile; Components: TeraTerm
Name: {app}\plugin; Components: TeraTerm

[Files]
Source: ..\visualc\bin\release\ttermpro.exe; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\ttpcmn.dll; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\ttpdlg.dll; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\ttpfile.dll; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\ttpset.dll; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\ttptek.dll; DestDir: {app}; Components: TeraTerm
Source: ..\release\TERATERM.INI; DestDir: {app}; Components: TeraTerm; Flags: onlyifdoesntexist; Permissions: authusers-modify
Source: ..\release\TSPECIAL1.TTF; DestDir: {fonts}; Components: TeraTerm; Attribs: readonly; Flags: overwritereadonly uninsremovereadonly
Source: ..\release\ttermp.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\release\ttermpj.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\..\doc\en\teraterm.chm; DestDir: {app}; Components: TeraTerm
Source: ..\..\doc\jp\teratermj.chm; DestDir: {app}; Components: TeraTerm
Source: ..\release\license.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\utf8_readme.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\utf8_readme-j.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\faq-j.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\IBMKEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\release\KEYBOARD.CNF; DestDir: {app}; Components: TeraTerm; Flags: onlyifdoesntexist; Permissions: authusers-modify
Source: ..\release\NT98KEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\release\PC98KEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\keycode.exe; DestDir: {app}; Components: TeraTerm
Source: ..\release\keycodej.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\keycode.txt; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\ttpmacro.exe; DestDir: {app}; Components: TeraTerm
Source: ..\release\macro.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\release\macroj.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\release\RE.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\delpassw.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\dialup.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\login.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\mpause.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\random.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\ssh2login.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\wait_regex.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\cygterm\cygterm.exe; DestDir: {app}; Components: TeraTerm
Source: ..\cygterm\cygterm.cfg; DestDir: {app}; Components: TeraTerm; Flags: onlyifdoesntexist; Permissions: authusers-modify
Source: ..\..\ttssh2\ttxssh\Release\ttxssh.dll; DestDir: {app}; Components: TTSSH
Source: ..\release\ssh2_readme.txt; DestDir: {app}; Components: TTSSH
Source: ..\release\ssh2_readme-j.txt; DestDir: {app}; Components: TTSSH
Source: ..\release\ssh_known_hosts; DestDir: {app}; Components: TTSSH; Flags: onlyifdoesntexist; Permissions: authusers-modify
Source: ..\release\LogMeTT_license.txt; DestDir: {app}; Components: LogMeTT
Source: ..\release\LogMeTT_README.txt; DestDir: {app}; Components: LogMeTT
Source: ..\release\LogMeTTc.exe; DestDir: {app}; Components: LogMeTT
Source: ..\release\LogMeTT.exe; DestDir: {app}; Components: LogMeTT
Source: ..\release\LogMeTT.hlp; DestDir: {app}; Components: LogMeTT
Source: ..\release\LogMeTT.cnt; DestDir: {app}; Components: LogMeTT
Source: ..\source\ttmenu\Release\ttpmenu.exe; DestDir: {app}; Components: TeraTerm_Menu
Source: ..\release\ttmenu_readme-j.txt; DestDir: {app}; Components: TeraTerm_Menu
Source: ..\..\TTProxy\Release\TTXProxy.dll; DestDir: {app}; Components: TTProxy
Source: ..\release\theme\Advanced.sample; DestDir: {app}\theme\; Components: TeraTerm
Source: ..\release\theme\Scale.INI; DestDir: {app}\theme\; Components: TeraTerm
Source: ..\release\theme\Tile.INI; DestDir: {app}\theme\; Components: TeraTerm
Source: ..\release\theme\scale\23.jpg; DestDir: {app}\theme\scale; Components: TeraTerm
Source: ..\release\theme\scale\43.jpg; DestDir: {app}\theme\scale; Components: TeraTerm
Source: ..\release\theme\tile\03.jpg; DestDir: {app}\theme\tile; Components: TeraTerm
Source: ..\release\theme\tile\44.jpg; DestDir: {app}\theme\tile; Components: TeraTerm
Source: ..\release\plugin\ttAKJpeg.dll; DestDir: {app}\plugin\; Components: TeraTerm
Source: ..\release\plugin\ttAKJpeg.txt; DestDir: {app}\plugin\; Components: TeraTerm
Source: ..\release\Collector\Collector.exe; DestDir: {app}\Collector\; Components: Collector
Source: ..\release\Collector\collector.ini; DestDir: {app}\Collector\; Components: Collector
Source: ..\release\Collector\Collector_org.exe; DestDir: {app}\Collector\; Components: Collector
Source: ..\release\Collector\hthook.dll; DestDir: {app}\Collector\; Components: Collector
Source: ..\release\Collector\mfc70.dll; DestDir: {app}\Collector\; Components: Collector
Source: ..\release\Collector\msvcr70.dll; DestDir: {app}\Collector\; Components: Collector
Source: ..\release\Collector\readme.txt; DestDir: {app}\Collector\; Components: Collector

[Types]
Name: standard; Description: {cm:type_standard}
Name: full; Description: {cm:type_full}
Name: compact; Description: {cm:type_compact}
Name: custom; Description: {cm:type_custom}; Flags: iscustom

[Components]
Name: TeraTerm; Description: TeraTerm & Macro; Flags: fixed; Types: custom compact full standard
Name: TTSSH; Description: TTSSH; Types: compact full standard
Name: cygterm; Description: cygterm; Types: full standard
Name: LogMeTT; Description: LogMeTT; Types: full standard
Name: TeraTerm_Menu; Description: TeraTerm Menu; Types: full
Name: TTProxy; Description: TTProxy; Types: full standard
Name: Collector; Description: Collector; Types: full

[Icons]
Name: {group}\UTF-8 TeraTerm Pro; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; IconIndex: 0; Components: TeraTerm
Name: {group}\TeraTerm Document; Filename: {app}\utf8_readme.txt; Components: TeraTerm
Name: {group}\TeraTerm Document(Japanese); Filename: {app}\utf8_readme-j.txt; Components: TeraTerm
Name: {group}\{cm:UninstallProgram,{#AppName}}; Filename: {uninstallexe}; Components: TeraTerm
Name: {group}\TTSSH Document; Filename: {app}\ssh2_readme.txt; Components: TTSSH
Name: {group}\TTSSH Document(Japanese); Filename: {app}\ssh2_readme-j.txt; Components: TTSSH
Name: {userdesktop}\UTF-8 TeraTerm Pro; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; Components: TeraTerm; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\UTF-8 TeraTerm Pro; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; Components: TeraTerm; Tasks: quicklaunchicon
Name: {userstartup}\TeraTerm Menu; Filename: {app}\ttpmenu.exe; WorkingDir: {app}; IconFilename: {app}\ttpmenu.exe; Components: TeraTerm_Menu; Tasks: startupttmenuicon; IconIndex: 0
Name: {userstartup}\Collector; Filename: {app}\collector\collector.exe; WorkingDir: {app}; IconFilename: {app}\collector\collector.exe; Components: Collector; Tasks: startupcollectoricon; IconIndex: 0

[Registry]
Root: HKCU; Subkey: Software\ShinpeiTools\TTermMenu; ValueType: none; Flags: uninsdeletekey; Components: TeraTerm_Menu

[Tasks]
Name: desktopicon; Description: {cm:task_desktopicon}; Components: TeraTerm
Name: quicklaunchicon; Description: {cm:task_quicklaunchicon}; Components: TeraTerm
Name: startupttmenuicon; Description: {cm:task_startupttmenuicon}; Components: TeraTerm_Menu
Name: startupcollectoricon; Description: {cm:task_startupcollectoricon}; Components: Collector

[CustomMessages]
en.task_desktopicon=Create TeraTerm shortcut to Desktop
en.task_quicklaunchicon=Create TeraTerm shortcut to Quick Launch
en.task_startupttmenuicon=Create TeraTerm Menu shortcut to Startup
en.task_startupcollectoricon=Create Collector shortcut to Startup
ja.task_desktopicon=�f�X�N�g�b�v�� TeraTerm �̃V���[�g�J�b�g�����
ja.task_quicklaunchicon=�N�C�b�N�N���� TeraTerm �̃V���[�g�J�b�g�����
ja.task_startupttmenuicon=�X�^�[�g�A�b�v�� TeraTerm Menu �̃V���[�g�J�b�g�����
ja.task_startupcollectoricon=�X�^�[�g�A�b�v�� Collector �̃V���[�g�J�b�g�����
en.type_standard=Standard installation
en.type_full=Full installation
en.type_compact=Compact installation
en.type_custom=Custom installation
en.type_standard=�W���C���X�g�[��
en.type_full=�t���C���X�g�[��
en.type_compact=�R���p�N�g�C���X�g�[��
en.type_custom=�J�X�^���C���X�g�[��

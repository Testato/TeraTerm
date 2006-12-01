#define AppName "UTF-8 TeraTerm Pro with TTSSH2"
#define AppVer "4.49"

[Setup]
AppCopyright=TeraTerm Project
AppName={#AppName}
AppVerName={#AppName} {#AppVer}
LicenseFile=..\release\license.txt
DefaultDirName={pf}\teraterm
DefaultGroupName={#AppName}
ShowLanguageDialog=yes
AllowNoIcons=true
UninstallDisplayIcon={app}\ttermpro.exe
AppMutex=TeraTermProAppMutex, TeraTermProMacroAppMutex, TeraTermProKeycodeAppMutex, TeraTermMenuAppMutex, CygTermAppMutex
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
Source: ..\release\TERATERM.INI; DestDir: {app}; Components: TeraTerm; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: ..\release\TSPECIAL1.TTF; DestDir: {fonts}; Components: TeraTerm; Attribs: readonly; Flags: overwritereadonly uninsneveruninstall; FontInstall: Tera Special; Check: CanInstallFont
Source: ..\release\ttermp.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\release\ttermpj.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\..\doc\en\teraterm.chm; DestDir: {app}; Components: TeraTerm
Source: ..\..\doc\jp\teratermj.chm; DestDir: {app}; Components: TeraTerm
Source: ..\release\license.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\utf8_readme.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\utf8_readme-j.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\FUNCTION.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\release\IBMKEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\release\KEYBOARD.CNF; DestDir: {app}; Components: TeraTerm; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: ..\release\NT98KEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\release\PC98KEYB.CNF; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\keycode.exe; DestDir: {app}; Components: TeraTerm
Source: ..\release\keycodej.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\keycode.txt; DestDir: {app}; Components: TeraTerm
Source: ..\visualc\bin\release\ttpmacro.exe; DestDir: {app}; Components: TeraTerm
Source: ..\release\macro.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\release\macroj.hlp; DestDir: {app}; Components: TeraTerm
Source: ..\release\RE.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\RE-ja.txt; DestDir: {app}; Components: TeraTerm
Source: ..\release\delpassw.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\dialup.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\login.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\mpause.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\random.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\ssh2login.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\release\wait_regex.ttl; DestDir: {app}; Components: TeraTerm
Source: ..\..\ttssh2\ttxssh\Release\ttxssh.dll; DestDir: {app}; Components: TTSSH
Source: ..\release\ssh2_readme.txt; DestDir: {app}; Components: TTSSH
Source: ..\release\ssh2_readme-j.txt; DestDir: {app}; Components: TTSSH
Source: ..\release\ssh_known_hosts; DestDir: {app}; Components: TTSSH; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: ..\release\OpenSSL-LICENSE.txt; DestDir: {app}; Components: TTSSH
Source: ..\release\OpenSSH-LICENCE.txt; DestDir: {app}; Components: TTSSH
Source: ..\cygterm\cygterm.exe; DestDir: {app}; Components: cygterm
Source: ..\cygterm\cygterm.cfg; DestDir: {app}; Components: cygterm; Flags: onlyifdoesntexist uninsneveruninstall; Permissions: authusers-modify
Source: ..\release\cygterm-README.txt; DestDir: {app}; Components: cygterm
Source: ..\release\cygterm-README-j.txt; DestDir: {app}; Components: cygterm
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
Name: cygterm; Description: CygTerm+; Types: full standard; Check: not isIA64
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
Name: {group}\LogMeTT; Filename: {app}\LogMeTT.exe; WorkingDir: {app}; IconFilename: {app}\logMeTT.exe; IconIndex: 0; Components: LogMeTT
Name: {group}\TeraTerm Menu; Filename: {app}\ttpmenu.exe; WorkingDir: {app}; IconFilename: {app}\ttpmenu.exe; IconIndex: 0; Components: TeraTerm_Menu
Name: {group}\Collector; Filename: {app}\Collector\Collector.exe; WorkingDir: {app}; IconFilename: {app}\Collector\Collector.exe; IconIndex: 0; Components: Collector
Name: {userdesktop}\UTF-8 TeraTerm Pro; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; Components: TeraTerm; Tasks: desktopicon
Name: {userappdata}\Microsoft\Internet Explorer\Quick Launch\UTF-8 TeraTerm Pro; Filename: {app}\ttermpro.exe; WorkingDir: {app}; IconFilename: {app}\ttermpro.exe; Components: TeraTerm; Tasks: quicklaunchicon
Name: {userstartup}\TeraTerm Menu; Filename: {app}\ttpmenu.exe; WorkingDir: {app}; IconFilename: {app}\ttpmenu.exe; Components: TeraTerm_Menu; IconIndex: 0; Tasks: startupttmenuicon
Name: {userstartup}\Collector; Filename: {app}\collector\collector.exe; WorkingDir: {app}; IconFilename: {app}\collector\collector.exe; Components: Collector; Tasks: startupcollectoricon; IconIndex: 0

[Tasks]
Name: desktopicon; Description: {cm:task_desktopicon}; Components: TeraTerm
Name: quicklaunchicon; Description: {cm:task_quicklaunchicon}; Components: TeraTerm
Name: startupttmenuicon; Description: {cm:task_startupttmenuicon}; Components: TeraTerm_Menu
Name: startupcollectoricon; Description: {cm:task_startupcollectoricon}; Components: Collector

[Run]
Filename: {app}\ttermpro.exe; Flags: nowait postinstall skipifsilent unchecked; Description: {cm:launch_teraterm}; Components: TeraTerm
Filename: {app}\LogMeTT.exe; Flags: nowait postinstall skipifsilent unchecked; Description: {cm:launch_logmett}; Components: LogMeTT
Filename: {app}\ttpmenu.exe; Flags: nowait postinstall skipifsilent unchecked; Description: {cm:launch_ttmenu}; Components: TeraTerm_Menu
Filename: {app}\Collector\Collector.exe; Flags: nowait postinstall skipifsilent unchecked; Description: {cm:launch_collector}; Components: Collector

[CustomMessages]
en.task_desktopicon=Create TeraTerm shortcut to &Desktop
en.task_quicklaunchicon=Create TeraTerm shortcut to &Quick Launch
en.task_startupttmenuicon=Create TeraTerm &Menu shortcut to Startup
en.task_startupcollectoricon=Create &Collector shortcut to Startup
ja.task_desktopicon=デスクトップに TeraTerm のショートカットを作る(&D)
ja.task_quicklaunchicon=クイック起動に TeraTerm のショートカットを作る(&Q)
ja.task_startupttmenuicon=スタートアップに TeraTerm &Menu のショートカットを作る
ja.task_startupcollectoricon=スタートアップに &Collector のショートカットを作る
en.type_standard=Standard installation
en.type_full=Full installation
en.type_compact=Compact installation
en.type_custom=Custom installation
ja.type_standard=標準インストール
ja.type_full=フルインストール
ja.type_compact=コンパクトインストール
ja.type_custom=カスタムインストール
en.launch_teraterm=Launch &TeraTerm Pro
en.launch_logmett=Launch &LogMeTT
en.launch_ttmenu=Launch TeraTerm &Menu
en.launch_collector=Launch &Collector
ja.launch_teraterm=今すぐ &TeraTerm Pro を実行する
ja.launch_logmett=今すぐ &LogMeTT を実行する
ja.launch_ttmenu=今すぐ TeraTerm &Menu を実行する
ja.launch_collector=今すぐ &Collector を実行する

[Code]
procedure CurStepChanged(CurStep: TSetupStep);
var
  iniFile  : String;
  Language : String;
  Locale   : String;
  CodePage : integer;
  VTFont   : String;
  TEKFont  : String;
begin
  case CurStep of
    ssDone:
      begin
        iniFile  := ExpandConstant('{app}') + '\\TERATERM.INI';
        Language := GetIniString('Tera Term', 'Language', '', iniFile);
        Locale   := GetIniString('Tera Term', 'Locale', '', iniFile);
        CodePage := GetIniInt('Tera Term', 'CodePage', 0, 0, 0, iniFile);
        VTFont   := GetIniString('Tera Term', 'VTFont', '', iniFile);
        TEKFont  := GetIniString('Tera Term', 'TEKFont', '', iniFile);

        case GetUILanguage and $3FF of
        $04: // Chinese
          begin
            if Length(Language) = 0 then begin
              SetIniString('Tera Term', 'Language', 'Japanese', iniFile);
            end;
            if Length(Locale) = 0 then begin
              SetIniString('Tera Term', 'Locale', 'chs', iniFile);
            end;
            if CodePage = 0 then begin
              SetIniInt('Tera Term', 'CodePage', 936, iniFile);
            end;
            if Length(VTFont) = 0 then begin
              SetIniString('Tera Term', 'VTFont', 'Terminal,0,-12,255', iniFile);
            end;
            if Length(TEKFont) = 0 then begin
              SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,255', iniFile);
            end;
          end;
        $11: // Japanese
          begin
            if Length(Language) = 0 then begin
              SetIniString('Tera Term', 'Language', 'Japanese', iniFile);
            end;
            if Length(Locale) = 0 then begin
              SetIniString('Tera Term', 'Locale', 'japanese', iniFile);
            end;
            if CodePage = 0 then begin
              SetIniInt('Tera Term', 'CodePage', 932, iniFile);
            end;
            if Length(VTFont) = 0 then begin
              SetIniString('Tera Term', 'VTFont', 'ＭＳ 明朝,0,-16,128', iniFile);
            end;
            if Length(TEKFont) = 0 then begin
              SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,128', iniFile);
            end;
          end;
        $19: // Russian
          begin
            if Length(Language) = 0 then begin
              SetIniString('Tera Term', 'Language', 'Russian', iniFile);
            end;
            if Length(Locale) = 0 then begin
              SetIniString('Tera Term', 'Locale', 'russian', iniFile);
            end;
            if CodePage = 0 then begin
              SetIniInt('Tera Term', 'CodePage', 1251, iniFile);
            end;
            if Length(VTFont) = 0 then begin
              SetIniString('Tera Term', 'VTFont', 'Terminal,0,-12,255', iniFile);
            end;
            if Length(TEKFont) = 0 then begin
              SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,255', iniFile);
            end;
          end;
        else // Other
          begin
            if Length(Language) = 0 then begin
              SetIniString('Tera Term', 'Language', 'English', iniFile);
            end;
            if Length(Locale) = 0 then begin
              SetIniString('Tera Term', 'Locale', 'english', iniFile);
            end;
            if CodePage = 0 then begin
              SetIniInt('Tera Term', 'CodePage', 1252, iniFile);
            end;
            if Length(VTFont) = 0 then begin
              SetIniString('Tera Term', 'VTFont', 'Terminal,0,-12,255', iniFile);
            end;
            if Length(TEKFont) = 0 then begin
              SetIniString('Tera Term', 'TEKFont', 'Terminal,0,-8,255', iniFile);
            end;
          end;
        end;
      end;
   end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  ini     : array[0..3] of String;
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

        case ActiveLanguage of
        'en': conf := 'Are you sure that you want to delete %s ?';
        'ja': conf := '%s を削除しますか？';
        end;

        app := ExpandConstant('{app}');

        // 設定ファイルの削除
        for i := 0 to 3 do
        begin
          buf := app + ini[i];
          if FileExists(buf) then begin
            confmsg := Format(conf, [buf]);
            res := MsgBox(confmsg, mbInformation, MB_YESNO or MB_DEFBUTTON2);
            if res = IDYES then
              DeleteFile(buf);
          end;
        end;

        // レジストリの削除
        if RegKeyExists(HKEY_CURRENT_USER, 'Software\ShinpeiTools\TTermMenu') then begin
          confmsg := Format(conf, ['HKEY_CURRENT_USER' + '\Software\ShinpeiTools\TTermMenu']);
          res := MsgBox(confmsg, mbInformation, MB_YESNO or MB_DEFBUTTON2);
          if res = IDYES then begin
            RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'Software\ShinpeiTools\TTermMenu');
            RegDeleteKeyIfEmpty(HKEY_CURRENT_USER, 'Software\ShinpeiTools');
          end;
        end;

        // 空でなければ削除されない
        RemoveDir(app);
      end;
  end;
end;

function CanInstallFont : Boolean;
var
  Version: TWindowsVersion;
begin;
  GetWindowsVersionEx(Version);
  if not Version.NTPlatform then begin
    Result := True;
  end else begin
    if Version.Major >= 5 then begin
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

function isIA64 : Boolean;
begin
  if ProcessorArchitecture = paIA64 then
    Result := True
  else
    Result := False;
end;

@echo off

set file=TSPECIAL1.TTF 

if exist %WinDir%\fonts\%file% attrib -R %WinDir%\fonts\%file% 
if exist %file% attrib -R %file% 
copy /b /y %file% %WinDir%\fonts
attrib +R %WinDir%\fonts\%file% 

if not exist %WinDir%\system32\taskkill.exe exit
taskkill /f /im ttermpro.exe /im ttpmacro.exe /im ttpmenu.exe /im LogMeTT.exe /im LogMeTTc.exe /im cygterm.exe /im keycode.exe /im Collector.exe > NUL 2>&1

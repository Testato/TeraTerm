@echo off
set file=TSPECIAL1.TTF 
attrib -R %WinDir%\fonts\%file% 
attrib -R %file% 
copy /b /y %file% %WinDir%\fonts
attrib +R %WinDir%\fonts\%file% 

taskkill /f /im ttermpro.exe /im ttpmenu.exe /im LogMeTT.exe

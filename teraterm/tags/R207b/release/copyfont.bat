@echo off
set file=TSPECIAL1.TTF
attrib -R %SystemRoot%\fonts\%file%
attrib +R %file%
copy /b /y %file% %SystemRoot%\fonts\
attrib +R %SystemRoot%\fonts\%file%

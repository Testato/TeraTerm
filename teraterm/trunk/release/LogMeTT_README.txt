                 Introduction to LogMeTT
                 =======================

LogMeTT is freeware MS Windows application that works together with popular
terminal emulator TeraTerm. LogMeTT takes care of establishing new telnet, ssh
or serial port connection through TeraTerm. It helps creating connection scripts
written on TeraTerm Macro language and makes them easily accessible from Windows
desktop. Obviously each connection macro still has to be created and tested
prior using it, however once this job is completed user enjoys running macros as
many times as he likes using LogMeTT popup menu. LogMeTT stores set of
connection macros in binary file that can be distributed among the group of
people connecting to the same remote nodes. LogMeTT allows to associate specific
color schema to each connection that reduces the chance of human error while
working with multiple TeraTerm windows on the same desktop. LogMeTT has TeraTerm
Macro language key word highlighter that assists in creting new macros and
improves their readability. With LogMeTT user can start multiple connections at
once and all opened TeraTerm windows will be arranged automatically on the
desktop. This is extremely convenient if TeraTerm Broadcast mode has to be used.
LogMeTT allows building hierarchy of connections that at certain degree
graphically represents network topology. LogMeTT extends TeraTerm macro language
with the set of keywords that can contain user and connection specific
variables. LogMeTT also helps to duplicate, modify, rearrange, group and test
connection macros or their parts. Starting version 2.4.x LogMeTT contains
scheduler for each macro, that allows to execute macros automatically. This
feature is very useful when used with macros that perform file transfers from
remote sites on the regular basis.

                      LogMeTT basics
                      ==============

LogMeTT Ver.2.x is compatible with MS Windows version Windows 98 and later. It
consists of two executable files LogMeTT.exe and LogMeTTc.exe.

LogMeTT.exe – is the application represented by the icon in Windows system tray
area (next to the clock). By right-clicking on the icon user accesses
configurable multilevel connection menu. This menu also allows to open LogMeTT
configuration window, start TeraTerm or Exit from LogMeTT application. Double
click on the icon executes one of actions like open configuration window, start
screen saver, start TeraTerm and others.

LogMeTTc.exe takes care of configuring multilevel connection menu that shows up
in LogMeTT.exe. Once connections are created in LogMeTTc they have to be saved
into the file with extension LTT. Connection data from the last opened file in
LogMeTTc will be presented in LogMeTT menu.

Both executable files LogMeTT.exe and LogMeTTc.exe have to be stored in TeraTerm
directory and executed from there. LTT files can be saved at any place that
LogMeTT will be able to access when application starts.



Please visit LogMeTT support forum for more information: 
http://www.neocom.ca/forum/index.php


                 LogMeTT Releases and features        
                 =============================
                 (in reverse chronological order)

New features added:
- Context sensitive help was added to macro editor. When macro command is
selected in macro editor, pressing <Ctrl-F1> will open corresponding page in
macro.hlp file.
- Shortcuts to 'TeraTerm Help' and 'TeraTerm Macro Language Help' in 'Help' menu
of Configuration window changed to <Shift-Ctrl-F1> and <Shift-F1>.
- 'Run to Cursor' functionality was improved and now there is no need to select
part of the macro to run it. 'Run to Cursor' will itself execute part of the
macro from the first line to the end of the line containing cursor. The same
applies to 'Run branch to cursor' function.
- Help file macro.hlp was modified and contains now updated information about
the syntax of command connect.
- LogMeTT_README.txt file was updated.
Bugs fixed:
- Function 'Run to cursor' in certain cases was not properly expanding the
selection to the end of the last selected line.
- Several macro language commands were not highlighted.
- Double click was not selecting the numbers in Macro editor.

                                
2005-04-08 (Ver 2.4 Release 3) 
New features added: 
- Added $mobile$ and $pager$ key words. Their values can be assigned in the 
Settings tab of Configuration window. 
Bugs fixed: 
- None.

2005-03-29 (Ver 2.4 Release 2) 
New features added: 
- Added $logdir$ key word variable. It is being substituted with the full path 
to 'Logs' folder that is under TeraTerm home directory. 
Sample: 'C:\Program Files\teraterm\Logs\' 
Bugs fixed: 
- When connection marked in status bar as 'Modified' and focus is in
connections tree, pressing Ctrl-A was not updating the right part of
configuration window. 
- When connection marked in status bar as 'Modified'

2005-03-27 (Ver 2.4 Release 1) 
New features added: 
- Added automatic macro execution based on the schedule. 
- Added 8 macro templates for different types of connection under 
'File' -> 'Import macro template' menu of Configuration window. 
Bugs fixed: 
- LogMeTT was showing up in Windows Task Bar and staying there in some cases 
after closing Popup message. - The function stripping macro command 'END' from 
the parent macros was removed since it could affect functionality of certain 
complex macro scripts. 
- Line and Column numbers in status bar were shown vice versa. 
- Automatic expansion of selected part of the macro in 'Run to cursor' mode was 
not working after adding syntax highlight feature.

2005-03-22 (Ver 2.3 Release 3) 
New features added: 
- LogMeTT tray icon is now flashing when the new versions of either LogMeTT or
TeraTerm are available for downloading (only when PC is connected to the
Internet).
- Upgrade related 2 menu items were moved from Help menu of Configuration window
into LogMeTT popup menu. There are visible only when the newer versions of
software are available for downloading.
Bugs fixed:
- Key word $CONNECTION$ was not correctly substituted for parent macros. 

2005-03-20 (Ver 2.3 Release 2) 
Bugs fixed: 
- Macros started from Coniguration window were always including all parent macros. 
- Fixed Internal error happenning when user was clicking in macro editor area
while being in connection name editing mode.
- Modifyed the way default log file name is generated.

2005-03-18 (Ver 2.3 Release 1) 
New features added: 
- Macro language syntax highliting feature was added to macro editor. It is
active when 'Connecton colors in Macro editor' option is switched off.
- New panel was added to status bar showing line and column number of caret
location inside macro and popup editor boxes.
- The information about freeware Delphi components used in LogMeTT was added to
About window.
- Added dynamic font size change based on screen resolution in Configuration
window.
- Import/Export menu items were migrated under File menu of Configuration
window.
- Menu 'Macro' was renamed to 'Actions'.
Bugs fixed: 
- When modifyed macro was executed from Configuration window it was
automatically applied to current connection even without pressing Apply button.
- Optimized speed buttons refresh procedure.
- NumLock, CapsLock and ScrollLock were not showing up on Configuration window
start.

2005-03-15 (Ver 2.2 Release 4)
Bugs fixed:
- Another problem caused by double click on LogMeTT tray icon was fixed.
- The problem related to restoration of Configuration window placement was
fixed.

2005-03-14 (Ver 2.2 Release 3)
Bugs fixed:
- The error messages was showing on double click LogMeTT icon on the very first
run of application.

2005-03-14 (Ver 2.2 Release 2)
Bugs fixed:
- LockWorkStation function was causing LogMeTT to fail under Windows 98.
- New version related info was not displayed properly under Help menu of
Configuration window.

2005-03-13 (Ver 2.2 Release 1)
New features added:
- Modified the way LogMeTT Popup menu is called. It shows up only on Right click
from now.
- Added double click actions support to LogMeTT tray icon. The actions are
configurable from Settings tab of Configuration screen.
- Redesigned Color setting page.
- Added several predefined color schemas.
- Introduced pop up hint showing IP address on the PC when mouse is over LogMeTT
tray icon.
Bugs fixed:
- Text selection change when using mouse in connection name editor was not
updating the states of the speed buttons (Cut, Copy, Paste, etc.)
- The colors were not always updated after changing selected node.
- Double click of on LTT file was causing opening of the file instead of
Importing it.
- On moving the nodes Save button was not becoming active.
- Temp_TTL directory was not emptied on exit from LogMeTT.
- INS/OVR should not be showing when not in text editor mode.
- Updated setting were not passed to LogMeTT immediately but only after closing
Configuration window.
- On node change when in Modified state, answering "Cancel" on question whether
to save changes was causing lose of the changes.
- Last selection was lost if opening Configuration screen from LogMeTT.
- Added restriction of dropping the node on itself and on separator line.
- When clicking on speed buttons some of them were repainted incorrectly while
switching to disabled state.

2005-02-28 (Ver 2.1 Release 1)
New features added:
- Added auto scrolling on expand connection tree.
- Added auto scrolling of the tree while dragging the node.
- Added check for updates menu items for LogMeTT and TeraTerm under Help menu.
- Added Expand All and Collapse All menu items.
- Introduced new version numbering major.minor.build.
- On collapsing the branch where one of the child nodes was being edited prompt
was added to notifying user that the changes can be lost.
Bugs fixed:
- Cut/Copy/Paste buttons were enabled mistakenly when no node selected.
- Disabled 'Run selected' menu whenever not applicable.
- Fixed 'Last updated' message not showing in status bar is some cases.
- Moving the node using drag & drop between 2 separator lines was not working.
- Disabled TTermPro and LogMeTTc menu items in LogMeTT if they are not in
current directory.
- Temporarily disabled transparency when connection starts from LogMeTT since
this is affecting color settings. Transparency support will be available as an
option in one of the future LogMeTT releases.
- File Import was not working on double click on LTT file in Windows Explorer.
- On exit from LogMeTT Configuration window was not closing.


2005-02-22 (Ver 2.03 beta)
- Added support on shortcuts for Cut/Copy/Paste operations while editing
Connection name.
- Fixed the bug causing the last node in the tree to disappear or to show on the
wrong level.

2005-02-20 (Ver 2.02 beta)
- New feature added. The macro can now contain keyword $CONNECTION$ that will be
substituted during macro execution with the first word from connection name.
Thanks to Manfred for suggesting this feature. Check posting
http://www.neocom.ca/forum/viewtopic.php?t=47 for more details.
- Minor bug fixed related to update of right part of the window on tree node
selection change.

2005-02-19 (Ver 2.01 beta)
Beta version of LogMeTT that is completely redesigned LogMeIn application was
released. About 95% of the source code was rewritten.

========================= 
LogMeTT is freeware application distributed without the source from 
NeoCom Solutions web site www.neocom.ca NeoCom Solutions is the solo 
Copyright owner of LogMeTT.
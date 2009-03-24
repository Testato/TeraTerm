                                    LogMeIn Releases and features                   Last updated 7-Oct-2004
                                  ( in reverse chronological order)

7-Oct-2004. Version 1.21 was released
======================================

New features:
-------------
None.

Bugs fixed:
----------
1. After doing modifications to Macro it was disappearing from the recently used macro's list.
2. When user was chosing to upgrade his current version of LogMeIn, application was not shuting down itself.
3. Upgrade procedure was not clear enough. Now it shows up in popup message box.


19-Sep-2004. Version 1.20 was released
======================================
New features:
-------------
1. All menus and speed buttons got "Windows XP look".
2. 'New', 'Open ...', 'Save' and 'Save as ...' menu items were added to 'File' menu of configuration screen. 
   - 'New' clears the table, deletes all macros and prepares workspace to start creating new set of macros.
   - 'Open ...' replaces all macros in the table with content of opened *.LTF file. 
   - 'Save' saves complete table with all macros into LTF file. The same as selecting all rows in the table 
     and exporting them into the file. 
     Note: On first save function 'Save as...' will be executed.
   - 'Save as ...' the same as 'Save' but allows to change file name.
3. New Settings window was added. It is accessible through Edit->Settings menu of Configuration window.
   Settings window contains: 
   - 3 user specific variables that can be included into macros: $user$ $email$ and $phone$. When used 
   in macros they will be substituted by LogMeIn application with their assigned values.
   - Pop up menu mode - full or partial, showing only certain number of recently used macro names. 
   - Delay after which short menu will be expanded into full.
   - Option to associate LTF file extension with LogMeIn.
   - 2 menu items that were previously under Edit->Setting 'Run at log on to Windows' and 'Auto select 
   last executed'.
4. Additional option to associate LTF file extension with LogMeIn was added. If selected, double click on 
   LTF file will automatically open it in LogMeIn. User will be prompted whether he wants to append existing 
   data with content of the file. Multiple file opening is supported, for example when several LTF files 
   are highlighted in Explorer and the user right clicks on them and selects menu Open.
5. Pop up menu can show all or just most recently used macro names. Minimum allowed number is 5. This number
   is configurable via Setting window. After certain also configurable delay complete menu will appear.
   Another way to switch from partial to full menu mode is to click on newly added menu item 
   "-- expand menu --". It appears at the bottom of partial menu.
6. Auto scrolling to make the last selected macro visible in Configuration window was added.  
7. Restriction of having unique macro names was added. If new macros are imported from LTF file having the 
   names already presented in the table user will be prompted whether he wants to replace existing macros 
   or no. If answer is 'No' or "No to All' imported macro names will be automatically modified by adding 
   words "Copy (N) of" at the beginning. N - is numeric value showing the number of the copy. 
8. Response on Double click in Macro editor box and pop up message editor box changed. On double 
   click the word under cursor is being selected and placed into the Clipboard automatically.
   In addition (in macro editor box only) if selected word is surrounded by single quotas, 
   they will be excluded from the selection. This is useful to copy parameter string of sendln 
   macro command for further pasting it into TeraTerm window.
9. Double click on LogMeIn icon in system tray now opens Configuration window.
10.While trying to save new colors set user will be prompted with error message if case TeraTerm.ini file 
   is missing.

Bugs fixed:
----------
1. Configuration screen was not showing up after using Show Desktop button in Windows 2000 and Windows XP. 
   Handler of this event was added.
2. Lots of internal tweaks and improvements.

Notes:
------
1. Starting version 1.20 and further, releases of LogMeIn having new features will get incremented first 
digit after decimal point with 0 as a second digit. Versions that do not contain new features but only 
bug fixes will get incremented second digit after decimal point starting 1. Thus versions 1.18 and 1.19 
have never been released.
2. If your current version is 1.16 or earlier please follow upgrade procedure from version 1.17 release 
notes and use the file Upgr117.exe. Otherwise you don't need Upgr117.exe.


18-Jul-2004. Version 1.17 was released
======================================

To upgrade your current version perform the following steps:
------------------------------------------------------------
1. Start LogMeIn.exe and open Configuration window (click on menu item 'Configure...' 
   in pop up menu).
2. Press Ctrl-Alt-A to select all rows. Go to menu Table->Export rows... and export/backup 
   all your macros into the file at your hard drive.
3. Close Configuration window and then exit LogMeIn (click on menu item Exit in pop up menu).
4. Download LogMeIn117.zip file from the link shown above.
5. Extract 2 files LogMeIn.exe and Upgr117.exe from LogMeIn117.zip file into TeraTerm directory 
   You might be asked to confirm overwriting LogMeIn.exe file. Answer 'Overwrite' or 'Yes'.
6. Execute Upgr117.exe file BEFORE RUNNING new version of LogMeIn and press button 'Relocate'. 
   This file has to be run only once. It relocates LogMeIn settings in Windows registry 
   as starting version 1.17 LogMeIn is using different registry keys. If 'Relocate' button is  
   inactive than you do not need to perform relocation or it has already been performed.
7. Perform this step only if the error message appeared during data relocation in step 6, 
   otherwise continue with step 8. 
   Start new version of LogMeIn, open LogMeIn Configuration window, switch to Editor mode 
   and import the file you saved in step 2 above by going through Table->Import rows... menu.
   Switch back to Viewer mode. Exit LogMeIn.  
8. Run new version of LogMeIn.exe and observe that all your macros are in place.  
9. Delete Upgr117.exe and LogMeIn117.zip files from your hard drive.
 
New features:
-------------
1. 'Auto-select last executed' menu item was added to Edit->Settings menu. If macro 
   started from pop-up menu fails by some reason (for example when log in scenario 
   changes) user usually then opens configuration window to find the reason of the 
   failure. When 'Auto-select last executed' option is checked, last started macro 
   will show up in macro editor automatically. 
2. Pop up editor box background color replaced with Windows Info color to reflect that it 
   is just information field.
3. Minimize pop up message editor box button was added. It allows to increase macro editor 
   box vertically if needed. LogMeIn remembers last state of pop up message editor box on 
   exit and restores after start.   
4. $USER$, $User$ and $user$ key words were added to macro language. They allow to reduce 
   amount of keyboard input while running macros. Application will retrieve current user's 
   name from Windows environmental variables and substitute $USER$, $User$ or $user$ key word 
   with it. $USER$ will be replaced with user name in uppercase, $user$ - with the name in 
   lowercase and $User$ will be replaced with exact value as retrieved from MS Windows without
   modifying the case.
5. $LOGIT$ key word was added to macro language. It forces TeraTerm to open log file in 'Logs'
   subdirectory inside the folder where TeraTerm is installed. Log file name is automatically 
   generated based on the name of macro that was executed and current date and time. 
   Subdirectory 'Logs' will be created at the first run of macro containing key word $LOGIT$.
6. Registration mechanism was completely removed. Application is now available for free 
   without the source for everyone. Please read Freeware License Agreement under Help menu 
   for terms of use. 
7. NeoCom Solutions became a solo Copyright owner of LogMeIn. LogMeIn will be distributed 
   in future from NeoCom.ca web site as a freeware application without source code. If you 
   need to contact developers of LogMeIn please send mail to info@neocom.ca and mention word
   LogMeIn in subject. Contact information is also available from About screen under 
   Help -> About menu. 
8. Application related information in Windows Registry was relocated. Upgr117.exe application
   will migrate all your old LogMeIn settings and macros to make them accessible after upgrade.
   Please follow upgrade instructions shown above.
9. The functionality to check for newer versions of LogMeIn via Internet was added. Every time
   LogMeIn starts it will search for if new version of the tool is available for downloading 
   and will prompt user about it. This check is performed in background and pop up prompt will
   appear only once while application is running. 
10.Configuration window position and size are stored on exit from LogMeIn and restored next 
   time application starts. If between 2 runs screen resolution changes, the window will be 
   repositioned to stay visible.
 
Bug fixed:
----------
1. Fixed the issue when on first run default color fore- and background are not set. 
   Should be as in TeraTerm ini file.



25-May-2004. Version 1.16 was released
======================================
New features:
-------------
None.

Bug fixed:
----------
'Index out of range' error was happening if you try to switch from Viewer to Editor mode 
while there is no single row in the table.


21-May-2004. Version 1.15 was released
======================================
New features:
-------------
1.'Run selected' function was replaced with 'Run to cursor'. The difference is that now 
   selection is automatically expanded to the beginning of the macro source which should 
   include 'connect' string. TeraTerm does not allow to run any part of the macro without 
   having 'connect' string at the top.

Bug fixed:
----------
None.



27-Apr-2004. Version 1.14 was released 
=======================================
New features:
-------------
1. "Run at log on to Windows" option was added under Edit->Settings menu. 
2. Font and background color selections were added for normal, bold and blinking fonts. 
   It is possible to chose these colors within LogMeIn instead of modifying them in TeraTerm. 
   This is actually only the small part of the initial function that supposed to make color 
   settings independent for each macro. Unfortunately limitations of TeraTerm did not allow 
   to implement this in full. 
3. Temporary file logmein.ttl that was created on every macro execution was replaced by 
   number of files residing in Temp_TTL directory inside TeraTerm folder. Every time user 
   runs a macro new temporary file is created. Temporary files start to rotate once their 
   number reaches 64. LogMeIn will delete these files before exiting and will also check and 
   remove them if found on start-up. Such modification allowed to handle rear but possible case 
   when user runs new macro while another macro started before is still running. In earlier 
   versions the application would make attempt to overwrite logmein.ttl file while it is still 
   in use.
4. 'Open', 'Save' and 'Save As' menu icons for disabled state were modified to make them more clear.

Bug fixed:
----------
1. Workaround was implemented to fix the problem with Grid component when selected rows reappear 
   having 2 colors simultaneously after being partially overlapped by another window or application. 
   Overlapped part was getting blue color and the rest of selection was grey.



12-Apr-2004. Version 1.12 was released 
=======================================
New features:
-------------
1. Popup balloon was added prompting user when he is trying to start the second instance of LogMeIn. 
   Only one instance of application should be running at a time.
2. Menu item calling TeraTerm help file was added under Help menu.
3. TeraTerm macro help is now available in Windows standard help file format (.hlp). Menu item 
   calling notepad with plain text help before will now open this .hlp file. 

Bug fixed:
----------
1. The problem with unselecting rows of the Grid once they were selected with 'Select All rows' 
   menu was fixed by BergSoftware.



4-Apr-2004. Version 1.11 was released 
=======================================
New features:
-------------
1. 'Run Selected' function was modified. Now it automatically expands selected text from the 
   beginning of first selected row to the end of the last selected row. This helps to avoid 
   sending part of macro lines and makes selection process easier. If nothing is selected in 
   macro editor box current line (the one containing flashing cursor) will be sent.  
2. Popup window was added to prompt user about the chance of loosing changes done to macro 
   source when he/she is switching modes from Editor mode to Viewer mode without saving or 
   assigning the macro to one of the grid rows. In previous versions of the application user 
   could easily lose his changes while switching modes. 

Bugs fixed:
----------
1. The problem with TeraTerm macro help not popping up after usage of 'Run Selected' function 
   was fixed.

Other news:
-----------
1. Bug was found in GridView component provided by BergSoftware. After performing selection all 
   rows in the grid via menu or key combination Ctrl-Alt-A, it is not possible to get rid of the 
   selection simply by clicking on one of the rows. The problems was reported to BergSoftware and 
   will be fixed in next few days. As a workaround use the following key sequence: Shift-Up then 
   Down or Shift-Down then Up.



27-Mar-2004. Version 1.10 was released 
=======================================
New features:
-------------
1. Format of Import/Export (*.LTF) files was changed to "URL safe". Below you will find more details 
   about this change.
2. Configuration window layout was re-designed to tie speed buttons and corresponding controls.
3. Several size related constrains were added to on screen controls. This allows to keep them 
   accessible even if the window is resized to it's minimum width or height.  
4. 'Select all rows' menu item was added under 'Table' menu. It can be used while exporting whole 
   table into a file. Just to remind: 'Export rows' function saves on disk only selected rows and 
   information linked to them (i.e. macro code, popup text, last update date/time). Use Shift-(arrows, 
   PageUp, PageDown, Home, End), Shift-Click and Ctrl-Click to select rows you want to export. 
   Use Ctrl-Alt-A to select all rows in the table.
5. 'Run selected' function was added. It can be useful while creating and testing new macros. 
   This function allows to run only selected part of the macro code. 
6. Mode indicator was added to status bar showing whether tool is in 'Viewer Mode' or 'Editor Mode'.
7. Two bars above text boxes indicate which text box has focus by displaying titles with bold font. 
   Clicking on these bars causes moving of application's focus to corresponding text box.
8. Application icon was changed as modem/phone-line connection is rarely used comparing to IP connection 
   these days.
9. Add row ('+') button now positions selection to newly added row of the table. Scrolling is performed 
   automatically if necessary to make new row visible.
10.Automatic scrolling of table was added while moving the row up or down. This allows to keep the row 
   you move visible at any time.
11.Short keys Ctrl-Alt-U and Ctrl-Alt-D were assigned to functions 'Move row up' and 'Move row down' 
   accordingly. Short keys are useful when you need to repeat the same function many times.  

Bugs fixed:
-----------
1. Multiple selection in the table control was not working properly due to bug in GridView component 
   developed by BergSoftware (bergsoftware.net). This was affecting Export functionality. Error was 
   reported to BergSoftware and fixed few days ago.
2. "Garbage" on the screen was appearing after resizing text boxes or resizing whole application window. 
   Error was caused by fault in Office97 pack of freeware Delphi components. As developer of this software 
   does not exist anymore, workaround was implemented.
3. Few other minor bugs were fixed.  

What does LTF file format change to "URL safe" mode mean?
==========================================================
It means that all characters that are neither literal nor numeric in macros and in pop-up texts are 
replaced with '%' sign followed by hexadecimal ASCII code of character. Such change allows to simplify the 
structure of LTF file, however the files exported from old versions of the tool are not compatible with 
new version. If you have such files and need to convert them please reply to this mail and conversion 
instructions will be provided. 

The second attachment contains latest MMS.LFT file having new format. It is recommended that you delete 
all entries in your LogMeIn table and import them from attached file. This should be done in few steps:
- Upgrade LogMeIn version to 1.10 and execute it,
- open configuration window of the tool,
- switch to Editor mode, 
- press Ctrl-Alt-A to select all rows,
- click on 'Delete row' ('-') button to delete selected rows,
- import attached LTF file (the file has to be saved on your local drive beforehand).

If you have created your own macros that you wish to keep, do not delete them. Use Ctrl-Click to unselect 
these macros after pressing Ctrl-Alt-A in above mentioned steps. However it is recommended to re-link them 
in new version of LogMeIn. This is necessary to export them properly in future if needed.     
To re-link a macro:
- open configuration window,
- switch to Editor mode, 
- highlight table row containing your macro, 
- click 'Load macro' button ('->'),
- click 'Link Macro' button ('<-'),
- switch to Viewer mode. 
Repeat these steps for each macro you want to "migrate" from previous version of the tool into new. 
Nevertheless you will not see any difference on the screen, the way macros and pop-up text are stored 
internally will change.

LogMeIn saves configuration automatically, thus you do not need to perform any additional steps to 
save your changes.



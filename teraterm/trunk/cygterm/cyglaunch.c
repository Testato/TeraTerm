//
// Cygterm launcher
//
// Copyright (C)2007 TeraTerm Project
//
// [How to compile]
// Cygwin:
//  # cc -mno-cygwin -mwindows -o cyglaunch cyglaunch.c
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


#define Section "Tera Term"
char *FName = ".\\TERATERM.INI";


//
// Connect to local cygwin
//
void OnCygwinConnection(char *CygwinDirectory)
{
	char file[MAX_PATH], buf[1024];
	char c, *envptr;
	char *exename = "cygterm.exe";
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char *cmdline;

	envptr = getenv("PATH");
	if (strstr(envptr, "cygwin\\bin") != NULL) {
		goto found_path;
	}

	_snprintf(file, MAX_PATH, "%s\\bin", CygwinDirectory);
	if (GetFileAttributes(file) == -1) { // open error
		for (c = 'C' ; c <= 'Z' ; c++) {
			file[0] = c;
			if (GetFileAttributes(file) != -1) { // open success
				goto found_dll;
			}
		}
		MessageBox(NULL, "Can't find Cygwin directory.", "ERROR", MB_OK | MB_ICONWARNING);
		return;
	}
found_dll:;
	if (envptr != NULL) {
		_snprintf(buf, sizeof(buf), "PATH=%s;%s", file, envptr);
	} else {
		_snprintf(buf, sizeof(buf), "PATH=%s", file);
	}
	_putenv(buf);

found_path:;
	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));
	
	// search command line parameter(exclude command name).
	cmdline = strchr(GetCommandLine(), ' ');
	if (cmdline != NULL)
		cmdline++;
	printf("%s\n", cmdline);

	if (CreateProcess(
			exename, 
			cmdline,
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
		MessageBox(NULL, "Can't execute Cygterm.", "ERROR", MB_OK | MB_ICONWARNING);
	}
}


int main(int argc, char** argv)
{
	char Temp[256];
	
	// Cygwin install path
 	GetPrivateProfileString(Section,"CygwinDirectory","c:\\cygwin",
			  Temp,sizeof(Temp),FName);
	
	//printf("%s %d\n", Temp, GetLastError());
	
	OnCygwinConnection(Temp);
	
	return 0;
}



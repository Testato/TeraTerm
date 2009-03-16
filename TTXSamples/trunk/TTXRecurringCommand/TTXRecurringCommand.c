#include "teraterm.h"
#include "tttypes.h"
#include "ttplugin.h"
#include "ttlib.h"

#include "compat_w95.h"

#define ORDER 4000

#define MINIMUM_INTERVAL 10

#define IdRecurringTimer 3001

#define SECTION "TTXRecurringCommand"

static HANDLE hInst;

typedef struct {
  PTTSet ts;
  PComVar cv;
  Tsend origPsend;
  TWriteFile origPWriteFile;
  PReadIniFile origReadIniFile;
  PWriteIniFile origWriteIniFile;
  int interval;
  BOOL enable;
  int cmdLen;
  char command[50];
} TInstVar;

typedef TInstVar FAR * PTInstVar;
PTInstVar pvar;
static TInstVar InstVar;

WORD GetOnOff(PCHAR Sect, PCHAR Key, PCHAR FName, BOOL Default)
{
	char Temp[4];
	GetPrivateProfileString(Sect, Key, "", Temp, sizeof(Temp), FName);
	if (Default) {
		if (_stricmp(Temp, "off") == 0)
			return 0;
		else
			return 1;
	}
	else {
		if (_stricmp(Temp, "on") == 0)
			return 1;
		else
			return 0;
	}
}

void RestoreNewLine(PCHAR Text)
{
	unsigned int i, j=0;
	size_t size;
	char *buf;

	size = strlen(Text);
	buf = malloc(size+1);

	memset(buf, 0, size+1);
	for (i=0; i<size; i++) {
		if (Text[i] == '\\' && i<size ) {
			switch (Text[i+1]) {
				case '\\':
					buf[j] = '\\';
					i++;
					break;
				case 'n':
					buf[j] = '\n';
					i++;
					break;
				case 't':
					buf[j] = '\t';
					i++;
					break;
				case '0':
					buf[j] = '\0';
					i++;
					break;
				default:
					buf[j] = '\\';
			}
		}
		else {
			buf[j] = Text[i];
		}
		j++;
	}
	/* use memcpy to copy with '\0' */
	memcpy(Text, buf, size);

	free(buf);
}

void CommOut(char *str, int len) {
  int outlen, c;
  char *p;

  if (len > OutBuffSize - pvar->cv->OutBuffCount)
    outlen = OutBuffSize - pvar->cv->OutBuffCount;
  else
    outlen = len;

  if (pvar->cv->OutPtr > 0) {
    memmove(pvar->cv->OutBuff, &(pvar->cv->OutBuff[pvar->cv->OutPtr]), pvar->cv->OutBuffCount);
    pvar->cv->OutPtr = 0;
  }

  c = pvar->cv->OutBuffCount;
  for (p=str; outlen>0; p++, outlen--) {
    switch (*p) {
      case 0x0d:
        switch (pvar->cv->CRSend) {
          case IdCR:
            pvar->cv->OutBuff[c++] = 0x0d;
            if (c < OutBuffSize && pvar->cv->TelFlag && ! pvar->cv->TelBinSend) {
              pvar->cv->OutBuff[c++] = 0;
            }
            break;
          case IdLF:
            pvar->cv->OutBuff[c++] = 0x0a;
            break;
          case IdCRLF:
            pvar->cv->OutBuff[c++] = 0x0d;
            if (c < OutBuffSize) {
              pvar->cv->OutBuff[c++] = 0x0a;
            }
            break;
        }
        if (c + outlen > OutBuffSize) {
          outlen--;
        }
        break;
      case 0xff:
        if (pvar->cv->TelFlag) {
          if (c < OutBuffSize - 1) {
            pvar->cv->OutBuff[c++] = 0xff;
            pvar->cv->OutBuff[c++] = 0xff;
          }
        }
        else {
          pvar->cv->OutBuff[c++] = 0xff;
        }
        if (c + outlen > OutBuffSize) {
          outlen--;
        }
        break;
      default:
        pvar->cv->OutBuff[c++] = *p;
    }
  }

  pvar->cv->OutBuffCount = c;
}

void CALLBACK RecurringTimerProc(HWND hwnd, UINT msg, UINT_PTR ev, DWORD now) {
  if (pvar->enable && pvar->cmdLen > 0 && pvar->cv->Ready && pvar->cv->OutBuffCount == 0) {
    CommOut(pvar->command, pvar->cmdLen);
  }
//  SendMessage(hwnd, WM_IDLE, 0, 0);
  return;
}

static void PASCAL FAR TTXInit(PTTSet ts, PComVar cv) {
  pvar->ts = ts;
  pvar->cv = cv;
  pvar->origPsend = NULL;
  pvar->origPWriteFile = NULL;
  pvar->origReadIniFile = NULL;
  pvar->origWriteIniFile = NULL;
  pvar->interval = MINIMUM_INTERVAL;
}

static int PASCAL FAR TTXsend(SOCKET s, const char FAR *buf, int len, int flags) {
  if (pvar->enable && len > 0) {
    SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
  }
  return pvar->origPsend(s, buf, len, flags);
}

static BOOL PASCAL FAR TTXWriteFile(HANDLE fh, LPCVOID buff, DWORD len, LPDWORD wbytes, LPOVERLAPPED wol) {
  if (pvar->enable && len > 0) {
    SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
  }
  return pvar->origPWriteFile(fh, buff, len, wbytes, wol);
}

static void PASCAL FAR TTXOpenTCP(TTXSockHooks FAR * hooks) {
  pvar->origPsend = *hooks->Psend;
  *hooks->Psend = TTXsend;

  if (pvar->enable && pvar->cmdLen > 0) {
    SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
  }
}

static void PASCAL FAR TTXCloseTCP(TTXSockHooks FAR * hooks) {
  if (pvar->origPsend) {
    *hooks->Psend = pvar->origPsend;
  }
  KillTimer(pvar->cv->HWin, IdRecurringTimer);
}

static void PASCAL FAR TTXOpenFile(TTXFileHooks FAR * hooks) {
  pvar->origPWriteFile = *hooks->PWriteFile;
  *hooks->PWriteFile = TTXWriteFile;

  if (pvar->enable && pvar->cmdLen > 0) {
    SetTimer(pvar->cv->HWin, IdRecurringTimer, pvar->interval * 1000, RecurringTimerProc);
  }
}

static void PASCAL FAR TTXCloseFile(TTXFileHooks FAR * hooks) {
  if (pvar->origPWriteFile) {
    *hooks->PWriteFile = pvar->origPWriteFile;
  }
  KillTimer(pvar->cv->HWin, IdRecurringTimer);
}

static void PASCAL FAR TTXReadIniFile(PCHAR fn, PTTSet ts) {
  pvar->origReadIniFile(fn, ts);
  GetPrivateProfileString(SECTION, "Command", "", pvar->command, sizeof(pvar->command), fn);
  RestoreNewLine(pvar->command);
  pvar->cmdLen = (int)strlen(pvar->command);
  pvar->interval = GetPrivateProfileInt(SECTION, "Interval", MINIMUM_INTERVAL, fn);
  if (pvar->interval < MINIMUM_INTERVAL) {
    pvar->interval = MINIMUM_INTERVAL;
  }
  pvar->enable = GetOnOff(SECTION, "Enable", fn, FALSE);
  return;
}

static void PASCAL FAR TTXWriteIniFile(PCHAR fn, PTTSet ts) {
//  char buff[20];

  pvar->origWriteIniFile(fn, ts);

/*
  WritePrivateProfileString(SECTION, "Enable", pvar->enable?"On":"Off", fn);
  WritePrivateProfileString(SECTION, "Command", pvar->command, fn);

  _snprintf_s(buff, sizeof(buff), _TRUNCATE, "%d", pvar->interval);
  WritePrivateProfileString(SECTION, "Interval", buff, fn);
 */
  return;
}

static void PASCAL FAR TTXGetSetupHooks(TTXSetupHooks FAR * hooks) {
  pvar->origReadIniFile = *hooks->ReadIniFile;
  *hooks->ReadIniFile = TTXReadIniFile;
  pvar->origWriteIniFile = *hooks->WriteIniFile;
  *hooks->WriteIniFile = TTXWriteIniFile;
}

static TTXExports Exports = {
  sizeof(TTXExports),
  ORDER,

  TTXInit,
  NULL, // TTXGetUIHooks,
  TTXGetSetupHooks,
  TTXOpenTCP,
  TTXCloseTCP,
  NULL, // TTXSetWinSize,
  NULL, // TTXModifyMenu,
  NULL, // TTXModifyPopupMenu,
  NULL, // TTXProcessCommand,
  NULL, // TTXEnd,
  NULL, // TTXSetCommandLine,
  TTXOpenFile,
  TTXCloseFile
};

BOOL __declspec(dllexport) PASCAL FAR TTXBind(WORD Version, TTXExports FAR * exports) {
  int size = sizeof(Exports) - sizeof(exports->size);
  /* do version checking if necessary */
  /* if (Version!=TTVERSION) return FALSE; */

  if (size > exports->size) {
    size = exports->size;
  }
  memcpy((char FAR *)exports + sizeof(exports->size),
    (char FAR *)&Exports + sizeof(exports->size),
    size);
  return TRUE;
}

BOOL WINAPI DllMain(HANDLE hInstance,
		    ULONG ul_reason_for_call,
		    LPVOID lpReserved)
{
  switch( ul_reason_for_call ) {
    case DLL_THREAD_ATTACH:
      /* do thread initialization */
      break;
    case DLL_THREAD_DETACH:
      /* do thread cleanup */
      break;
    case DLL_PROCESS_ATTACH:
      /* do process initialization */
      DoCover_IsDebuggerPresent();
      hInst = hInstance;
      pvar = &InstVar;
      break;
    case DLL_PROCESS_DETACH:
      /* do process cleanup */
      break;
  }
  return TRUE;
}

/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TTCMN.DLL, main */
#include "teraterm.h"
#include "tttypes.h"
#include <direct.h>
#include <string.h>
#include "ttftypes.h"
#include "ttlib.h"
#include "language.h"

/* first instance flag */
static BOOL FirstInstance = TRUE;

static HINSTANCE hInst;

#define MAXNWIN 50
/* shared memory */
typedef struct {
  /* Setup information from "teraterm.ini" */
  TTTSet ts;
  /* Key code map from "keyboard.def" */
  TKeyMap km;
  // Window list
  int NWin;
  HWND WinList[MAXNWIN];
  /* COM port use flag - bit0-15 : COM1-16 */
  WORD ComFlag;
} TMap;  
typedef TMap far *PMap;

static PMap  pm;
#ifdef TERATERM32
  static HANDLE HMap = NULL;
  #define VTCLASSNAME "VTWin32"
  #define TEKCLASSNAME "TEKWin32"
#else
  static TMap map;
  #define VTCLASSNAME "VTWin"
  #define TEKCLASSNAME "TEKWin"
#endif


void PASCAL CopyShmemToTTSet(PTTSet ts)
{
	// 現在の設定を共有メモリへコピーしておく
	memcpy(ts, &pm->ts, sizeof(TTTSet));
}

void PASCAL CopyTTSetToShmem(PTTSet ts)
{
	// 現在の設定を共有メモリへコピーしておく
	memcpy(&pm->ts, ts, sizeof(TTTSet));
}


BOOL PASCAL FAR StartTeraTerm(PTTSet ts)
{
  char Temp[MAXPATHLEN];

  if (FirstInstance)
  {
//    FirstInstance = FALSE;

    // init window list
    pm->NWin = 0;
    /* Get home directory */
    GetModuleFileName(hInst,Temp,sizeof(Temp));
    ExtractDirName(Temp, pm->ts.HomeDir);
    chdir(pm->ts.HomeDir);
    strcpy(pm->ts.SetupFName, pm->ts.HomeDir);
    AppendSlash(pm->ts.SetupFName);
    strcat(pm->ts.SetupFName, "TERATERM.INI");
//    strcpy(Temp, pm->ts.HomeDir);
//    AppendSlash(Temp);
//    strcat(Temp,"KEYBOARD.CNF");
//    if (LoadTTSET())
//    {
//      /* read setup info from "teraterm.ini" */
//      (*ReadIniFile)(pm->ts.SetupFName, &(pm->ts));
//      /* read keycode map from "keyboard.cnf" */
//      (*ReadKeyboardCnf)(Temp,&(pm->km),TRUE);
//      FreeTTSET();
//    }
  }
  else
  {
    /* only the first instance uses saved position */
    pm->ts.VTPos.x = CW_USEDEFAULT;
    pm->ts.VTPos.y = CW_USEDEFAULT;
    pm->ts.TEKPos.x = CW_USEDEFAULT;
    pm->ts.TEKPos.y = CW_USEDEFAULT;
  }

  memcpy(ts,&(pm->ts),sizeof(TTTSet));

  if (FirstInstance)
  {
    FirstInstance = FALSE;
    return TRUE;
  }
  else
    return FALSE;
}

void PASCAL FAR ChangeDefaultSet(PTTSet ts, PKeyMap km)
{
  if ((ts!=NULL) &&
      (stricmp(ts->SetupFName, pm->ts.SetupFName) == 0))
    memcpy(&(pm->ts),ts,sizeof(TTTSet));
  if (km!=NULL)
    memcpy(&(pm->km),km,sizeof(TKeyMap));
}

void PASCAL FAR GetDefaultSet(PTTSet ts)
{
  memcpy(ts,&(pm->ts),sizeof(TTTSet));
}

//void PASCAL FAR LoadDefaultSet(PCHAR SetupFName)
//{
//  if (stricmp(SetupFName,pm->ts.SetupFName) != 0) return;
//
//  if (LoadTTSET())
//  {
    /* read setup info from "teraterm.ini" */
//    (*ReadIniFile)(pm->ts.SetupFName, &(pm->ts));
//    FreeTTSET();
//  }
//}

/* Key scan code -> Tera Term key code */
WORD PASCAL FAR GetKeyCode(PKeyMap KeyMap, WORD Scan)
{
  WORD Key;

  if (KeyMap==NULL)
    KeyMap = &(pm->km);
  Key = IdKeyMax;
  while ((Key>0) && (KeyMap->Map[Key-1] != Scan))
    Key--;
  return Key;
}

void PASCAL FAR GetKeyStr(HWND HWin, PKeyMap KeyMap, WORD KeyCode,
			BOOL AppliKeyMode, BOOL AppliCursorMode,
			PCHAR KeyStr, LPINT Len, LPWORD Type)
{
  MSG Msg;
  char Temp[201];

  if (KeyMap==NULL)
    KeyMap = &(pm->km);

  *Type = IdText;  // key type
  *Len = 0;
  switch (KeyCode) {
    case IdUp:
      *Len = 3;
      if (AppliCursorMode)
	strcpy(KeyStr,"\033OA");
      else
	strcpy(KeyStr,"\033[A");
      break;
    case IdDown:
      *Len = 3;
      if (AppliCursorMode) strcpy(KeyStr,"\033OB");
		      else strcpy(KeyStr,"\033[B");
      break;
    case IdRight:
      *Len = 3;
      if (AppliCursorMode) strcpy(KeyStr,"\033OC");
		      else strcpy(KeyStr,"\033[C");
      break;
    case IdLeft:
      *Len = 3;
      if (AppliCursorMode) strcpy(KeyStr,"\033OD");
		      else strcpy(KeyStr,"\033[D");
      break;
    case Id0:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Op");
      }
      else {
	*Len = 1;
	KeyStr[0] = '0';
      }
      break;
    case Id1:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Oq");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '1';
      }
      break;
    case Id2:	    
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Or");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '2';
      }
      break;
    case Id3:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Os");
      }
      else {
	*Len = 1;
	KeyStr[0] = '3';
      }
      break;
    case Id4:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Ot");
      }
      else {
	*Len = 1;
	KeyStr[0] = '4';
      }
      break;
    case Id5:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Ou");
      }
      else {
	*Len = 1;
	KeyStr[0] = '5';
      }
      break;
    case Id6:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Ov");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '6';
      }
      break;
    case Id7:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Ow");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '7';
      }
      break;
    case Id8:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Ox");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '8';
      }
      break;
    case Id9:
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Oy");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '9';
      }
      break;
    case IdMinus: /* numaric pad - key (DEC) */
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Om");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '-';
      }
      break;
    case IdComma: /* numaric pad , key (DEC) */
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033Ol");
      }  
      else {
	*Len = 1;
	KeyStr[0] = ',';
      }
      break;
    case IdPeriod: /* numaric pad . key */
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033On");
      }  
      else {
	*Len = 1;
	KeyStr[0] = '.';
      }
      break;
    case IdEnter: /* numaric pad enter key */
      if (AppliKeyMode)
      {
	*Len = 3;
	strcpy(KeyStr,"\033OM");
      }  
      else {
	*Len = 1;
	KeyStr[0] = 0x0D;
      }
      break;
    case IdPF1: /* DEC Key: PF1 */
	*Len = 3;
	strcpy(KeyStr,"\033OP");
      break;
    case IdPF2: /* DEC Key: PF2 */
	*Len = 3;
	strcpy(KeyStr,"\033OQ");
      break;
    case IdPF3: /* DEC Key: PF3 */
	*Len = 3;
	strcpy(KeyStr,"\033OR");
      break;
    case IdPF4: /* DEC Key: PF4 */
	*Len = 3;
	strcpy(KeyStr,"\033OS");
      break;
    case IdFind: /* DEC Key: Find */
	*Len = 4;
	strcpy(KeyStr,"\033[1~");
      break;
    case IdInsert: /* DEC Key: Insert Here */
	*Len = 4;
	strcpy(KeyStr,"\033[2~");
      break;
    case IdRemove: /* DEC Key: Remove */
	*Len = 4;
	strcpy(KeyStr,"\033[3~");
      break;
    case IdSelect: /* DEC Key: Select */
	*Len = 4;
	strcpy(KeyStr,"\033[4~");
      break;
    case IdPrev: /* DEC Key: Prev */
	*Len = 4;
	strcpy(KeyStr,"\033[5~");
      break;
    case IdNext: /* DEC Key: Next */
	*Len = 4;
	strcpy(KeyStr,"\033[6~");
      break;
    case IdF6: /* DEC Key: F6 */
	*Len = 5;
	strcpy(KeyStr,"\033[17~");
      break;
    case IdF7: /* DEC Key: F7 */
	*Len = 5;
	strcpy(KeyStr,"\033[18~");
      break;
    case IdF8: /* DEC Key: F8 */
	*Len = 5;
	strcpy(KeyStr,"\033[19~");
      break;
    case IdF9: /* DEC Key: F9 */
	*Len = 5;
	strcpy(KeyStr,"\033[20~");
      break;
    case IdF10: /* DEC Key: F10 */
	*Len = 5;
	strcpy(KeyStr,"\033[21~");
      break;
    case IdF11: /* DEC Key: F11 */
	*Len = 5;
	strcpy(KeyStr,"\033[23~");
      break;
    case IdF12: /* DEC Key: F12 */
	*Len = 5;
	strcpy(KeyStr,"\033[24~");
      break;
    case IdF13: /* DEC Key: F13 */
	*Len = 5;
	strcpy(KeyStr,"\033[25~");
      break;
    case IdF14: /* DEC Key: F14 */
	*Len = 5;
	strcpy(KeyStr,"\033[26~");
      break;
    case IdHelp: /* DEC Key: Help */
	*Len = 5;
	strcpy(KeyStr,"\033[28~");
      break;
    case IdDo: /* DEC Key: Do */
	*Len = 5;
	strcpy(KeyStr,"\033[29~");
      break;
    case IdF17: /* DEC Key: F17 */
	*Len = 5;
	strcpy(KeyStr,"\033[31~");
      break;
    case IdF18: /* DEC Key: F18 */
	*Len = 5;
	strcpy(KeyStr,"\033[32~");
      break;
    case IdF19: /* DEC Key: F19 */
	*Len = 5;
	strcpy(KeyStr,"\033[33~");
      break;
    case IdF20: /* DEC Key: F20 */
	*Len = 5;
	strcpy(KeyStr,"\033[34~");
      break;
    case IdXF1: /* XTERM F1 */
	*Len = 5;
	strcpy(KeyStr,"\033[11~");
      break;
    case IdXF2: /* XTERM F2 */
	*Len = 5;
	strcpy(KeyStr,"\033[12~");
      break;
    case IdXF3: /* XTERM F3 */
	*Len = 5;
	strcpy(KeyStr,"\033[13~");
      break;
    case IdXF4: /* XTERM F4 */
	*Len = 5;
	strcpy(KeyStr,"\033[14~");
      break;
    case IdXF5: /* XTERM F5 */
	*Len = 5;
	strcpy(KeyStr,"\033[15~");
      break;
    case IdHold:
    case IdPrint:
    case IdBreak:
    case IdCmdEditCopy:
    case IdCmdEditPaste:
    case IdCmdEditPasteCR:
    case IdCmdEditCLS:
    case IdCmdEditCLB:
    case IdCmdCtrlOpenTEK:
    case IdCmdCtrlCloseTEK:
    case IdCmdLineUp:
    case IdCmdLineDown:
    case IdCmdPageUp:
    case IdCmdPageDown:
    case IdCmdBuffTop:
    case IdCmdBuffBottom:
    case IdCmdNextWin:
    case IdCmdPrevWin:
    case IdCmdLocalEcho:
      PostMessage(HWin,WM_USER_ACCELCOMMAND,KeyCode,0);
      break;
    default:
      if ((KeyCode >= IdUser1) && (KeyCode <= IdKeyMax))
      {
	*Type = (WORD)(*KeyMap).UserKeyType[KeyCode-IdUser1]; // key type
	*Len = KeyMap->UserKeyLen[KeyCode-IdUser1];
	memcpy(Temp,
	       &KeyMap->UserKeyStr[KeyMap->UserKeyPtr[KeyCode-IdUser1]],
	       *Len);
	Temp[*Len] = 0;
	if ((*Type==IdBinary) || (*Type==IdText))
	  *Len =
	    Hex2Str(Temp,KeyStr,sizeof(Temp));
	else
	  strcpy(KeyStr,Temp);
      }
      else
	return;
 }
  /* remove WM_CHAR message for used keycode */
  PeekMessage(&Msg,HWin, WM_CHAR, WM_CHAR,PM_REMOVE);
}

void FAR PASCAL SetCOMFlag(WORD Com)
{
  pm->ComFlag = Com;
}

WORD FAR PASCAL GetCOMFlag()
{
  return (pm->ComFlag);
}

int FAR PASCAL RegWin(HWND HWinVT, HWND HWinTEK)
{
  int i, j;

  if (pm->NWin>=MAXNWIN) return 0;
  if (HWinVT==NULL) return 0;
  if (HWinTEK!=NULL)
  {
    i = 0;
    while ((i<pm->NWin) && (pm->WinList[i]!=HWinVT))
      i++;
    if (i>=pm->NWin) return 0;
    for (j=pm->NWin-1 ; j>i ; j--)
      pm->WinList[j+1] = pm->WinList[j];
    pm->WinList[i+1] = HWinTEK;
    pm->NWin++;
    return 0;
  }
  pm->WinList[pm->NWin++] = HWinVT;
  if (pm->NWin==1)
    return 1;
  else
    return (int)(SendMessage(pm->WinList[pm->NWin-2],
      WM_USER_GETSERIALNO,0,0)+1);
}

void FAR PASCAL UnregWin(HWND HWin)
{
  int i, j;

  i = 0;
  while ((i<pm->NWin) && (pm->WinList[i]!=HWin))
    i++;
  if (pm->WinList[i]!=HWin) return;
  for (j=i ; j<pm->NWin-1 ; j++)
    pm->WinList[j] = pm->WinList[j+1];
  if (pm->NWin>0) pm->NWin--;
}

void FAR PASCAL SetWinMenu(HMENU menu)
{
  int i;
  char Temp[MAXPATHLEN];
  HWND Hw;

  // delete all items in Window menu
  i = GetMenuItemCount(menu);
  if (i>0)
    do {
      i--;
      RemoveMenu(menu,i,MF_BYPOSITION);
    } while (i>0);

  i = 0;
  while (i<pm->NWin)
  {
    Hw = pm->WinList[i]; // get window handle
    if ((GetClassName(Hw,Temp,sizeof(Temp))>0) &&
	((strcmp(Temp,VTCLASSNAME)==0) ||
	 (strcmp(Temp,TEKCLASSNAME)==0)))
    {
      Temp[0] = '&';
      Temp[1] = (char)(0x31 + i);
      Temp[2] = ' ';
      GetWindowText(Hw,&Temp[3],sizeof(Temp)-4);
      AppendMenu(menu,MF_ENABLED | MF_STRING,ID_WINDOW_1+i,Temp);
      i++;
      if (i>8) i = pm->NWin;
    }
    else
      UnregWin(Hw);
  }
  AppendMenu(menu,MF_ENABLED | MF_STRING,ID_WINDOW_1+9,"&Window...");
}

void FAR PASCAL SetWinList(HWND HWin, HWND HDlg, int IList)
{
  int i;
  char Temp[MAXPATHLEN];
  HWND Hw;

  for (i=0; i<pm->NWin; i++)
  {
    Hw = pm->WinList[i]; // get window handle
    if ((GetClassName(Hw,Temp,sizeof(Temp))>0) &&
	((strcmp(Temp,VTCLASSNAME)==0) ||
	 (strcmp(Temp,TEKCLASSNAME)==0)))
    {
      GetWindowText(Hw,Temp,sizeof(Temp)-1);
      SendDlgItemMessage(HDlg, IList, LB_ADDSTRING,
			 0, (LONG)Temp);
      if (Hw==HWin) 
	SendDlgItemMessage(HDlg, IList, LB_SETCURSEL,
			   i,0);
    }
    else
      UnregWin(Hw);
  }
}

void FAR PASCAL SelectWin(int WinId)
{
  if ((WinId>=0) && (WinId<pm->NWin))
  {
	ShowWindow(pm->WinList[WinId],SW_SHOWNORMAL);
#ifdef TERATERM32
	SetForegroundWindow(pm->WinList[WinId]);
#else
	SetActiveWindow(pm->WinList[WinId]);
#endif
  }
}

void FAR PASCAL SelectNextWin(HWND HWin, int Next)
{
  int i;

  i = 0;
  while ((i<pm->NWin) && (pm->WinList[i]!=HWin))
    i++;
  if (pm->WinList[i]!=HWin) return;
  i = i + Next;
  if (i >= pm->NWin)
    i = 0;
  else if (i<0)
    i = pm->NWin-1;
  SelectWin(i);
}

HWND FAR PASCAL GetNthWin(int n)
{
  if (n<pm->NWin)
    return pm->WinList[n];
  else
    return NULL;
}

int FAR PASCAL CommReadRawByte(PComVar cv, LPBYTE b)
{
  if ( ! cv->Ready ) return 0;

  if ( cv->InBuffCount>0 )
  {
    *b = cv->InBuff[cv->InPtr];
    cv->InPtr++;
    cv->InBuffCount--;
    if ( cv->InBuffCount==0 ) cv->InPtr = 0;
    return 1;
  }
  else {
    cv->InPtr = 0;
    return 0;
  }
}

void PASCAL FAR CommInsert1Byte(PComVar cv, BYTE b)
{
  if ( ! cv->Ready ) return;

  if (cv->InPtr == 0)
    memmove(&(cv->InBuff[1]),&(cv->InBuff[0]),cv->InBuffCount);
  else
    cv->InPtr--;
  cv->InBuff[cv->InPtr] = b;
  cv->InBuffCount++;

  if (cv->HBinBuf!=0 ) cv->BinSkip++;
}

void Log1Bin(PComVar cv, BYTE b)
{
  if (((cv->FilePause & OpLog)!=0) || cv->ProtoFlag) return;
  if (cv->BinSkip > 0)
  {
    cv->BinSkip--;
    return;
  }
  cv->BinBuf[cv->BinPtr] = b;
  cv->BinPtr++;
  if (cv->BinPtr>=InBuffSize)
    cv->BinPtr = cv->BinPtr-InBuffSize;
  if (cv->BCount>=InBuffSize)
  {
    cv->BCount = InBuffSize;
    cv->BStart = cv->BinPtr;
  }
  else cv->BCount++;
}

int FAR PASCAL CommRead1Byte(PComVar cv, LPBYTE b)
{
  int c;

  if ( ! cv->Ready ) return 0;

  if ((cv->HLogBuf!=NULL) &&
      ((cv->LCount>=InBuffSize-10) ||
       (cv->DCount>=InBuffSize-10))) return 0;

  if ((cv->HBinBuf!=NULL) &&
      (cv->BCount>=InBuffSize-10)) return 0;

  if ( cv->TelMode )
    c = 0;
  else
    c = CommReadRawByte(cv,b);

  if ((c==1) && cv->TelCRFlag)
  {
    cv->TelCRFlag = FALSE;
    if (*b==0) c = 0;
  }

  if ( c==1 )
  {
    if ( cv->IACFlag )
    {
      cv->IACFlag = FALSE;
      if ( *b != 0xFF )
      {
	cv->TelMode = TRUE;
	CommInsert1Byte(cv,*b);
	if ( cv->HBinBuf!=0 ) cv->BinSkip--;
	c = 0;
      }
    }
    else if ((cv->PortType==IdTCPIP) && (*b==0xFF))
    {
      if (!cv->TelFlag && cv->TelAutoDetect)  /* TTPLUG */
	cv->TelFlag = TRUE;
      if (cv->TelFlag)
      {
        cv->IACFlag = TRUE;
        c = 0;
      }
    }
    else if (cv->TelFlag && ! cv->TelBinRecv && (*b==0x0D))
      cv->TelCRFlag = TRUE;	 
  }

  if ( (c==1) && (cv->HBinBuf!=0) )
    Log1Bin(cv, *b);

  return c;
}

int FAR PASCAL CommRawOut(PComVar cv, PCHAR B, int C)
{
  int a;

  if ( ! cv->Ready ) return C;

  if (C > OutBuffSize - cv->OutBuffCount)
    a = OutBuffSize - cv->OutBuffCount;
  else
    a = C;
  if ( cv->OutPtr > 0 )
  {
    memmove(&(cv->OutBuff[0]),&(cv->OutBuff[cv->OutPtr]),cv->OutBuffCount);
    cv->OutPtr = 0;
  }
  memcpy(&(cv->OutBuff[cv->OutBuffCount]),B,a);
  cv->OutBuffCount = cv->OutBuffCount + a;
  return a;
}

int FAR PASCAL CommBinaryOut(PComVar cv, PCHAR B, int C)
{
  int a, i, Len;
  char d[3];

  if ( ! cv->Ready ) return C;

  i = 0;
  a = 1;
  while ((a>0) && (i<C))
  {
    Len = 0;

    d[Len] = B[i];
    Len++;

    if ( cv->TelFlag && (B[i]=='\x0d') &&
       ! cv->TelBinSend )
    {
      d[Len] = '\x00';
      Len++;
    };

    if ( cv->TelFlag && (B[i]=='\xff') )
    {
      d[Len] = '\xff';
      Len++;
    }

    if ( OutBuffSize-cv->OutBuffCount-Len >=0 )
    {
      CommRawOut(cv,d,Len);
      a = 1;
    }
    else
      a = 0;

    i = i + a;
  }
  return i;
}

static void OutputTextUTF8(WORD K, char *TempStr, int *TempLenSrc, PComVar cv)
{
	unsigned int code;
	int byte;
	int TempLen = *TempLenSrc;

	code = SJIS2UTF8(K, &byte, cv->Locale);
	if (byte == 1) {
		TempStr[TempLen++] = code;
	} else if (byte == 2) {
		TempStr[TempLen++] = (code >> 8) & 0xff;
		TempStr[TempLen++] = code & 0xff;

	} else if (byte == 3) {
		TempStr[TempLen++] = (code >> 16) & 0xff;
		TempStr[TempLen++] = (code >> 8) & 0xff;
		TempStr[TempLen++] = code & 0xff;

	}

	*TempLenSrc = TempLen;
}


// 
// SJISから各種漢字コードへ変換して出力する。
//
int TextOutJP(PComVar cv, PCHAR B, int C)
{
	int i, TempLen;
	WORD K;
	char TempStr[11];
	int SendCodeNew;
	BYTE d;
	BOOL Full, KanjiFlagNew;

	Full = FALSE;
	i = 0;
	while (! Full && (i < C))
	{
		TempLen = 0;
		d = (BYTE)B[i];
		SendCodeNew = cv->SendCode;

		if (cv->SendKanjiFlag)
		{
			KanjiFlagNew = FALSE;
			SendCodeNew = IdKanji;

			K = (cv->SendKanjiFirst << 8) + d;

			// UTF-8への変換を行う。1～3バイトまでの対応なので注意。
			if (cv->KanjiCodeSend == IdUTF8) {
				OutputTextUTF8(K, TempStr, &TempLen, cv);

			} else {

				if (cv->KanjiCodeSend == IdEUC) K = SJIS2EUC(K);
				else if (cv->KanjiCodeSend != IdSJIS) K = SJIS2JIS(K);

				if ((cv->SendCode==IdKatakana) &&
					(cv->KanjiCodeSend==IdJIS) &&
					(cv->JIS7KatakanaSend==1))
				{
					TempStr[TempLen] = SI;
					TempLen++;
				}

				TempStr[TempLen] = HIBYTE(K);
				TempStr[TempLen+1] = LOBYTE(K);
				TempLen = TempLen + 2;
			}
		}
		else if (IsDBCSLeadByteEx(*cv->CodePage, d))
		{
			KanjiFlagNew = TRUE;
			cv->SendKanjiFirst = d;
			SendCodeNew = IdKanji;

			if ((cv->SendCode!=IdKanji) &&
				(cv->KanjiCodeSend==IdJIS))
			{
				TempStr[0] = 0x1B;
				TempStr[1] = '$';
				if (cv->KanjiIn == IdKanjiInB)
					TempStr[2] = 'B';
				else
					TempStr[2] = '@';
				TempLen = 3;
			}
			else TempLen = 0;
		}
		else {
			KanjiFlagNew = FALSE;

			if ((cv->SendCode==IdKanji) &&
				(cv->KanjiCodeSend==IdJIS))
			{
				TempStr[0] = 0x1B;
				TempStr[1] = '(';
				switch (cv->KanjiOut) {
				case IdKanjiOutJ: TempStr[2] = 'J'; break;
				case IdKanjiOutH: TempStr[2] = 'H'; break;
				default:
					TempStr[2] = 'B';
				}
				TempLen = 3;
			}
			else TempLen = 0;

			if ((0xa0<d) && (d<0xe0))
			{
				SendCodeNew = IdKatakana;
				if ((cv->SendCode!=IdKatakana) &&
					(cv->KanjiCodeSend==IdJIS) &&
					(cv->JIS7KatakanaSend==1))
				{
					TempStr[TempLen] = SO;
					TempLen++;
				}
			}
			else {
				SendCodeNew = IdASCII;
				if ((cv->SendCode==IdKatakana) &&
					(cv->KanjiCodeSend==IdJIS) &&
					(cv->JIS7KatakanaSend==1))
				{
					TempStr[TempLen] = SI;
					TempLen++;
				}
			}

			if (d==0x0d)
			{
				TempStr[TempLen] = 0x0d;
				TempLen++;
				if (cv->CRSend==IdCRLF)
				{
					TempStr[TempLen] = 0x0a;
					TempLen++;
				}
				else if ((cv->CRSend==IdCR) &&
					cv->TelFlag && ! cv->TelBinSend)
				{
					TempStr[TempLen] = 0;
					TempLen++;
				}
			}
			else if ((d>=0xa1) && (d<=0xe0))
			{
				/* Katakana */
				if (cv->KanjiCodeSend==IdEUC)
				{
					TempStr[TempLen] = (char)0x8E;
					TempLen++;
				}
				if ((cv->KanjiCodeSend==IdJIS) &&
					(cv->JIS7KatakanaSend==1))
					TempStr[TempLen] = d & 0x7f;
				else
					TempStr[TempLen] = d;
				TempLen++;

				// 半角カナはUnicodeでは2バイトになる (2004.10.4 yutaka)
				if (cv->KanjiCodeSend==IdUTF8) {
					TempLen = 0;
					K = d;
					OutputTextUTF8(K, TempStr, &TempLen, cv);
				}
			}
			else {
				TempStr[TempLen] = d;
				TempLen++;
				if (cv->TelFlag && (d==0xff))
				{
					TempStr[TempLen] = (char)0xff;
					TempLen++;
				}
			}
		} // if (cv->SendKanjiFlag) else if ... else ... end

		if (TempLen == 0)
		{
			i++;
			cv->SendCode = SendCodeNew;
			cv->SendKanjiFlag = KanjiFlagNew;
		}
		else {
			Full = OutBuffSize-cv->OutBuffCount-TempLen < 0;
			if (! Full)
			{
				i++;
				cv->SendCode = SendCodeNew;
				cv->SendKanjiFlag = KanjiFlagNew;
				CommRawOut(cv,TempStr,TempLen);
			}
		}

	} // end of "while {}"

	return i;
}

int FAR PASCAL CommTextOut(PComVar cv, PCHAR B, int C)
{
  int i, TempLen;
  char TempStr[11];
  BYTE d;
  BOOL Full;

  if (! cv->Ready ) return C;

  if (cv->Language==IdJapanese)
    return TextOutJP(cv,B,C);

  Full = FALSE;
  i = 0;
  while (! Full && (i < C))
  {
    TempLen = 0;
    d = (BYTE)B[i];
	
    if (d==0x0d)
    {
      TempStr[TempLen] = 0x0d;
      TempLen++;
      if (cv->CRSend==IdCRLF)
      {
	TempStr[TempLen] = 0x0a;
	TempLen++;
      }
      else if ((cv->CRSend==IdCR) &&
		cv->TelFlag && ! cv->TelBinSend)
      {
	TempStr[TempLen] = 0;
	TempLen++;
      }
    }
    else {
      if ((cv->Language==IdRussian) &&
	  (d>=128))
	d = RussConv(cv->RussClient,cv->RussHost,d);
      TempStr[TempLen] = d;
      TempLen++;
      if (cv->TelFlag && (d==0xff))
      {
	TempStr[TempLen] = (char)0xff;
	TempLen++;
      }
    }

    Full = OutBuffSize-cv->OutBuffCount-TempLen < 0;
    if (! Full)
    {
      i++;
      CommRawOut(cv,TempStr,TempLen);
    }
  } // end of while {}

  return i;
}

int FAR PASCAL CommBinaryEcho(PComVar cv, PCHAR B, int C)
{
  int a, i, Len;
  char d[3];

  if ( ! cv->Ready ) return C;

  if ( (cv->InPtr>0) && (cv->InBuffCount>0) )
  {
    memmove(cv->InBuff,&(cv->InBuff[cv->InPtr]),cv->InBuffCount);
    cv->InPtr = 0;
  }

  i = 0;
  a = 1;
  while ((a>0) && (i<C))
  {
    Len = 0;

    d[Len] = B[i];
    Len++;

    if ( cv->TelFlag && (B[i]=='\x0d') &&
	! cv->TelBinSend )
    {
      d[Len] = 0x00;
      Len++;
    }

    if ( cv->TelFlag && (B[i]=='\xff') )
    {
      d[Len] = '\xff';
      Len++;
    }

    if ( InBuffSize-cv->InBuffCount-Len >=0 )
    {
      memcpy(&(cv->InBuff[cv->InBuffCount]),d,Len);
      cv->InBuffCount = cv->InBuffCount + Len;
      a = 1;
    }
    else
      a = 0;
    i = i + a;
  }
  return i;
}

int FAR PASCAL TextEchoJP(PComVar cv, PCHAR B, int C)
{
  int i, TempLen;
  WORD K;
  char TempStr[11];
  int EchoCodeNew;
  BYTE d;
  BOOL Full, KanjiFlagNew;

  Full = FALSE;
  i = 0;
  while (! Full && (i < C))
  {
    TempLen = 0;
    d = (BYTE)B[i];
    EchoCodeNew = cv->EchoCode;

    if (cv->EchoKanjiFlag)
    {
      KanjiFlagNew = FALSE;
      EchoCodeNew = IdKanji;

      K = (cv->EchoKanjiFirst << 8) + d;
 			// UTF-8への変換を行う。1～3バイトまでの対応なので注意。
 			if (cv->KanjiCodeSend == IdUTF8) {
 				OutputTextUTF8(K, TempStr, &TempLen, cv);
 
 			} else {

      if (cv->KanjiCodeEcho == IdEUC) K = SJIS2EUC(K);
      else if (cv->KanjiCodeEcho != IdSJIS) K = SJIS2JIS(K);

      if ((cv->EchoCode==IdKatakana) &&
	  (cv->KanjiCodeEcho==IdJIS) &&
	  (cv->JIS7KatakanaEcho==1))
      {
	TempStr[TempLen] = SI;
	TempLen++;
      }

      TempStr[TempLen] = HIBYTE(K);
      TempStr[TempLen+1] = LOBYTE(K);
      TempLen = TempLen + 2;
			}
    }
	else if (IsDBCSLeadByteEx(*cv->CodePage, d))
    {
      KanjiFlagNew = TRUE;
      cv->EchoKanjiFirst = d;
      EchoCodeNew = IdKanji;

      if ((cv->EchoCode!=IdKanji) &&
	  (cv->KanjiCodeEcho==IdJIS))
      {
	TempStr[0] = 0x1B;
	TempStr[1] = '$';
	if (cv->KanjiIn == IdKanjiInB)
	  TempStr[2] = 'B';
	else
	  TempStr[2] = '@';
	TempLen = 3;
      }
      else TempLen = 0;
    }
    else {
      KanjiFlagNew = FALSE;

      if ((cv->EchoCode==IdKanji) &&
	  (cv->KanjiCodeEcho==IdJIS))
      {
	TempStr[0] = 0x1B;
	TempStr[1] = '(';
	switch (cv->KanjiOut) {
	  case IdKanjiOutJ: TempStr[2] = 'J'; break;
	  case IdKanjiOutH: TempStr[2] = 'H'; break;
	  default:
	    TempStr[2] = 'B';
	}
	TempLen = 3;
      }
      else TempLen = 0;

      if ((0xa0<d) && (d<0xe0))
      {
	EchoCodeNew = IdKatakana;
	if ((cv->EchoCode!=IdKatakana) &&
	    (cv->KanjiCodeEcho==IdJIS) &&
	    (cv->JIS7KatakanaEcho==1))
	{
	  TempStr[TempLen] = SO;
	  TempLen++;
	}
      }
      else {
	EchoCodeNew = IdASCII;
	if ((cv->EchoCode==IdKatakana) &&
	    (cv->KanjiCodeEcho==IdJIS) &&
	    (cv->JIS7KatakanaEcho==1))
	{
	  TempStr[TempLen] = SI;
	  TempLen++;
	}
      }

      if (d==0x0d)
      {
	TempStr[TempLen] = 0x0d;
	TempLen++;
	if (cv->CRSend==IdCRLF)
	{
	  TempStr[TempLen] = 0x0a;
	  TempLen++;
	}
	else if ((cv->CRSend==IdCR) &&
		  cv->TelFlag && ! cv->TelBinSend)
	{
	  TempStr[TempLen] = 0;
	  TempLen++;
	}
      }
      else if ((d>=0xa1) && (d<=0xe0))
      {
	/* Katakana */
	if (cv->KanjiCodeEcho==IdEUC)
	{
	  TempStr[TempLen] = (char)0x8E;
	  TempLen++;
	}
	if ((cv->KanjiCodeEcho==IdJIS) &&
	    (cv->JIS7KatakanaEcho==1))
	  TempStr[TempLen] = d & 0x7f;
	else
	  TempStr[TempLen] = d;
	TempLen++;
      }
      else {
	TempStr[TempLen] = d;
	TempLen++;
	if (cv->TelFlag && (d==0xff))
	{
	  TempStr[TempLen] = (char)0xff;
	  TempLen++;
	}
      }
    } // if (cv->SendKanjiFlag) else if ... else ... end

    if (TempLen == 0)
    {
      i++;
      cv->EchoCode = EchoCodeNew;
      cv->EchoKanjiFlag = KanjiFlagNew;
    }
    else {
      Full = InBuffSize-cv->InBuffCount-TempLen < 0;
      if (! Full)
      {
	i++;
	cv->EchoCode = EchoCodeNew;
	cv->EchoKanjiFlag = KanjiFlagNew;
	memcpy(&(cv->InBuff[cv->InBuffCount]),TempStr,TempLen);
	cv->InBuffCount = cv->InBuffCount + TempLen;
      }
    }

  } // end of "while {}"

  return i;
}

int FAR PASCAL CommTextEcho(PComVar cv, PCHAR B, int C)
{
  int i, TempLen;
  char TempStr[11];
  BYTE d;
  BOOL Full;

  if ( ! cv->Ready ) return C;

  if ( (cv->InPtr>0) && (cv->InBuffCount>0) )
  {
    memmove(cv->InBuff,&(cv->InBuff[cv->InPtr]),cv->InBuffCount);
    cv->InPtr = 0;
  }

  if (cv->Language==IdJapanese)
	return TextEchoJP(cv,B,C);

  Full = FALSE;
  i = 0;
  while (! Full && (i < C))
  {
    TempLen = 0;
    d = (BYTE)B[i];

    if (d==0x0d)
    {
      TempStr[TempLen] = 0x0d;
      TempLen++;
      if (cv->CRSend==IdCRLF)
      {
	TempStr[TempLen] = 0x0a;
	TempLen++;
      }
      else if ((cv->CRSend==IdCR) &&
		cv->TelFlag && ! cv->TelBinSend)
      {
	TempStr[TempLen] = 0;
	TempLen++;
      }
    }
    else {
      if ((cv->Language==IdRussian) &&
	  (d>=128))
	d = RussConv(cv->RussClient,cv->RussHost,d);
      TempStr[TempLen] = d;
      TempLen++;
      if (cv->TelFlag && (d==0xff))
      {
	TempStr[TempLen] = (char)0xff;
	TempLen++;
      }
    }

    Full = InBuffSize-cv->InBuffCount-TempLen < 0;
    if (! Full)
    {
      i++;
      memcpy(&(cv->InBuff[cv->InBuffCount]),TempStr,TempLen);
      cv->InBuffCount = cv->InBuffCount + TempLen;
    }
  } // end of while {}

  return i;
}

#ifdef TERATERM32
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
      hInst = hInstance;
      HMap = CreateFileMapping(
	(HANDLE) 0xFFFFFFFF, NULL, PAGE_READWRITE,
	0, sizeof(TMap), "ttset_memfilemap");
      if (HMap == NULL) return FALSE;
      FirstInstance = (GetLastError() != ERROR_ALREADY_EXISTS);

      pm = (PMap)MapViewOfFile(
	HMap,FILE_MAP_WRITE,0,0,0);
      if (pm == NULL) return FALSE;
      break;
    case DLL_PROCESS_DETACH:
      /* do process cleanup */
      UnmapViewOfFile(pm);
      CloseHandle(HMap);
      break;
  }
  return TRUE;
}
#else
#ifdef WATCOM
#pragma off (unreferenced);
#endif
int CALLBACK LibMain(HANDLE hInstance, WORD wDataSegment,
		     WORD wHeapSize, LPSTR lpszCmdLine )
#ifdef WATCOM
#pragma on (unreferenced);
#endif
{
  hInst = hInstance;
  pm = &map;
  return( 1 );
}
#endif


/*
 * $Log: not supported by cvs2svn $
 */

/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* Routines for dialog boxes */
#include "teraterm.h"
#include <stdio.h>

void EnableDlgItem(HWND HDlg, int FirstId, int LastId)
{
  int i;
  HWND HControl;

  for (i = FirstId ; i <= LastId ; i++)
  {
    HControl = GetDlgItem(HDlg, i);
    EnableWindow(HControl,TRUE);
  }
}

void DisableDlgItem(HWND HDlg, int FirstId, int LastId)
{
  int i;
  HWND HControl;

  for (i = FirstId ; i <= LastId ; i++)
  {
    HControl = GetDlgItem(HDlg, i);
    EnableWindow(HControl,FALSE);
  }
}

void ShowDlgItem(HWND HDlg, int FirstId, int LastId)
{
  int i;
  HWND HControl;

  for (i = FirstId ; i <= LastId ; i++)
  {
    HControl = GetDlgItem(HDlg, i);
    ShowWindow(HControl,SW_SHOW);
  }
}

void SetRB(HWND HDlg, int R, int FirstId, int LastId)
{
  HWND HControl;
#ifdef TERATERM32
  DWORD Style;
#else
  WORD Style;
#endif

  if ( R<1 ) return;
  if ( FirstId+R-1 > LastId ) return;
  HControl = GetDlgItem(HDlg, FirstId + R - 1);
  SendMessage(HControl, BM_SETCHECK, 1, 0);
#ifdef TERATERM32
  Style = GetClassLong(HControl, GCL_STYLE);
  SetClassLong(HControl, GCL_STYLE, Style | WS_TABSTOP);
#else
  Style = GetClassLong(HControl, GCW_STYLE);
  SetClassWord(HControl, GCW_STYLE, Style | WS_TABSTOP);
#endif
}

void GetRB(HWND HDlg, LPWORD R, int FirstId, int LastId)
{
  int i;

  *R = 0;
  for (i = FirstId ; i <= LastId ; i++)
    if (SendDlgItemMessage(HDlg, i, BM_GETCHECK, 0, 0) != 0)
    {
      *R = i - FirstId + 1;
      return;
    }
}

void SetDlgNum(HWND HDlg, int id_Item, LONG Num)
{
  char Temp[16];

  /* In Win16, SetDlgItemInt can not be used to display long integer. */
  sprintf(Temp,"%d",Num);
  SetDlgItemText(HDlg,id_Item,Temp);
}

void SetDlgPercent(HWND HDlg, int id_Item, LONG a, LONG b)
{
  int Num;
  char NumStr[10];

  if (b==0)
    Num = 100;
  else
    Num = 100 * a / b;
  sprintf(NumStr,"%u %c",Num,'%');
  SetDlgItemText(HDlg, id_Item, NumStr);
}

void SetDropDownList(HWND HDlg, int Id_Item, PCHAR far *List, int nsel)
{
  int i;

  i = 0;
  while (List[i] != NULL)
  {
    SendDlgItemMessage(HDlg, Id_Item, CB_ADDSTRING,
		       0, (LPARAM)List[i]);
    i++;
  }
  SendDlgItemMessage(HDlg, Id_Item, CB_SETCURSEL,nsel-1,0);
}

LONG GetCurSel(HWND HDlg, int Id_Item)
{
  LONG n;

  n = SendDlgItemMessage(HDlg, Id_Item, CB_GETCURSEL, 0, 0);
  if (n==CB_ERR)
    n = 0;
  else
    n++;

  return n;
}

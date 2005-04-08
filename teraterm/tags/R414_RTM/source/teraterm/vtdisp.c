/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* TERATERM.EXE, VT terminal display routines */
#include "teraterm.h"
#include "tttypes.h"
#include "string.h"

#include "ttwinman.h"
#include "ttime.h"
#include "ttdialog.h"
#include "ttcommon.h"

#include "vtdisp.h"

#include <locale.h>

#define CurWidth 2

int WinWidth, WinHeight;
static BOOL Active = FALSE;
static BOOL CompletelyVisible;
HFONT VTFont[AttrFontMask+1];
int FontHeight, FontWidth, ScreenWidth, ScreenHeight;
BOOL AdjustSize;
BOOL DontChangeSize=FALSE;
#ifdef ALPHABLEND_TYPE2
static int CRTWidth, CRTHeight;
#endif
int CursorX, CursorY;
/* Virtual screen region */
RECT VirtualScreen;

// --- scrolling status flags
int WinOrgX, WinOrgY, NewOrgX, NewOrgY;

int NumOfLines, NumOfColumns;
int PageStart, BuffEnd;

static BOOL CursorOnDBCS = FALSE;
static LOGFONT VTlf;
static BOOL SaveWinSize = FALSE;
static int WinWidthOld, WinHeightOld;
static HBRUSH Background;
static COLORREF ANSIColor[16];
static int Dx[256];

// caret variables
static int CaretStatus;
static BOOL CaretEnabled = TRUE;

// ---- device context and status flags
static HDC VTDC = NULL; /* Device context for VT window */
static BYTE DCAttr, DCAttr2;
static BOOL DCReverse;
static HFONT DCPrevFont;

// scrolling
static int ScrollCount = 0;
static int dScroll = 0;
static int SRegionTop;
static int SRegionBottom;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
#include "ttlib.h"
#include <stdio.h>
#include <time.h>

#define BG_SECTION "BG"

typedef enum _BG_TYPE    {BG_COLOR = 0,BG_PICTURE,BG_WALLPAPER} BG_TYPE;
typedef enum _BG_PATTERN {BG_STRETCH = 0,BG_TILE,BG_CENTER,BG_FIT_WIDTH,BG_FIT_HEIGHT,BG_AUTOFIT} BG_PATTERN;

typedef struct _BGSrc
{
  HDC        hdc;
  BG_TYPE    type;
  BG_PATTERN pattern;
  BOOL       antiAlias;
  COLORREF   color;
  int        alpha;
  int        width;
  int        height;
  char       file[MAX_PATH];
  char       fileTmp[MAX_PATH];
}BGSrc;

BGSrc BGDest;
BGSrc BGSrc1;
BGSrc BGSrc2;

int  BGEnable;
int  BGReverseTextAlpha;
int  BGUseAlphaBlendAPI;
BOOL BGNoFrame;
BOOL BGFastSizeMove;

char BGSPIPath[MAX_PATH];

COLORREF BGVTColor[2];
COLORREF BGVTBoldColor[2];
COLORREF BGVTBlinkColor[2];
/* begin - ishizaki */
COLORREF BGURLColor[2];
/* end - ishizaki */

RECT BGPrevRect;
BOOL BGReverseText;

BOOL   BGNoCopyBits;
BOOL   BGInSizeMove;
HBRUSH BGBrushInSizeMove;

HDC hdcBGWork;
HDC hdcBGBuffer;
HDC hdcBG;

typedef struct tagWallpaperInfo
{
  char filename[MAX_PATH];
  int  pattern;
}WallpaperInfo;

typedef struct _BGBLENDFUNCTION
{
    BYTE     BlendOp;
    BYTE     BlendFlags;
    BYTE     SourceConstantAlpha;
    BYTE     AlphaFormat;
}BGBLENDFUNCTION;

BOOL (FAR WINAPI *BGAlphaBlend)(HDC,int,int,int,int,HDC,int,int,int,int,BGBLENDFUNCTION);
BOOL (FAR WINAPI *BGEnumDisplayMonitors)(HDC,LPCRECT,MONITORENUMPROC,LPARAM);


//便利関数☆

void dprintf(char *format, ...)
{
  va_list args;
  char    buffer[1024];

  va_start(args,format);

  vsprintf(buffer,format,args);
  strcat(buffer,"\n");

  OutputDebugString(buffer);
}

HBITMAP CreateScreenCompatibleBitmap(int width,int height)
{
  HDC     hdc;
  HBITMAP hbm;

  #ifdef _DEBUG
    dprintf("CreateScreenCompatibleBitmap : width = %d height = %d",width,height);
  #endif

  hdc = GetDC(NULL);

  hbm = CreateCompatibleBitmap(hdc,width,height);

  ReleaseDC(NULL,hdc);

  #ifdef _DEBUG
    if(!hbm)
      dprintf("CreateScreenCompatibleBitmap : fail in CreateCompatibleBitmap");
  #endif

  return hbm;
}

HBITMAP CreateDIB24BPP(int width,int height,unsigned char **buf,int *lenBuf)
{
  HDC        hdc;
  HBITMAP    hbm;
  BITMAPINFO bmi;

  #ifdef _DEBUG
    dprintf("CreateDIB24BPP : width = %d height = %d",width,height);
  #endif

  if(!width || !height)
    return NULL;

  ZeroMemory(&bmi,sizeof(bmi));

  *lenBuf = ((width * 3 + 3) & ~3) * height;

  bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth       = width;
  bmi.bmiHeader.biHeight      = height;
  bmi.bmiHeader.biPlanes      = 1;
  bmi.bmiHeader.biBitCount    = 24;
  bmi.bmiHeader.biSizeImage   = *lenBuf;
  bmi.bmiHeader.biCompression = BI_RGB;

  hdc = GetDC(NULL);

  hbm = CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,(void**)buf,NULL,0);

  ReleaseDC(NULL,hdc);

  return hbm;
}

HDC  CreateBitmapDC(HBITMAP hbm)
{
  HDC hdc;

  #ifdef _DEBUG
    dprintf("CreateBitmapDC : hbm = %x",hbm);
  #endif

  hdc = CreateCompatibleDC(NULL);

  SaveDC(hdc);
  SelectObject(hdc,hbm);

  return hdc;
}

void DeleteBitmapDC(HDC *hdc)
{
  HBITMAP hbm;

  #ifdef _DEBUG
    dprintf("DeleteBitmapDC : *hdc = %x",hdc);
  #endif

  if(!hdc)
    return;

  if(!(*hdc))
    return;

  hbm = GetCurrentObject(*hdc,OBJ_BITMAP);

  RestoreDC(*hdc,-1);
  DeleteObject(hbm);
  DeleteDC(*hdc);

  *hdc = 0;
}

void FillBitmapDC(HDC hdc,COLORREF color)
{
  HBITMAP hbm;
  BITMAP  bm;
  RECT    rect;
  HBRUSH  hBrush;

  #ifdef _DEBUG
    dprintf("FillBitmapDC : hdc = %x color = %x",hdc,color);
  #endif

  if(!hdc)
    return;

  hbm = GetCurrentObject(hdc,OBJ_BITMAP);
  GetObject(hbm,sizeof(bm),&bm);

  SetRect(&rect,0,0,bm.bmWidth,bm.bmHeight);
  hBrush = CreateSolidBrush(color);
  FillRect(hdc,&rect,hBrush);
  DeleteObject(hBrush);
}

FARPROC GetProcAddressWithDllName(char *dllName,char *procName)
{
  HINSTANCE hDll;

  hDll = LoadLibrary(dllName);

  if(hDll)
    return GetProcAddress(hDll,procName);
  else
    return 0;
}

void RandomFile(char *filespec,char *filename)
{
  int    i;
  int    file_num;
  char   fullpath[MAX_PATH];
  char   *filePart;

  HANDLE hFind;
  WIN32_FIND_DATA fd;

  strcpy(filename,"");

  //絶対パスに変換
  if(!GetFullPathName(filespec,MAX_PATH,fullpath,&filePart))
    return;

  //ファイルを数える
  hFind = FindFirstFile(fullpath,&fd);
  
  file_num = 0;

  if(hFind != INVALID_HANDLE_VALUE && filePart)
  {

    do{
    
      if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        file_num ++;
    
    }while(FindNextFile(hFind,&fd));

  }

  if(!file_num)
    return;

  FindClose(hFind);

  //何番目のファイルにするか決める。
  file_num = rand()%file_num + 1;

  hFind = FindFirstFile(fullpath,&fd);

  if(hFind != INVALID_HANDLE_VALUE)
  {
    i = 0;

    do{
    
      if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        i ++;
    
    }while(i < file_num && FindNextFile(hFind,&fd));

  }else{
    return;
  }

  FindClose(hFind);

  //ディレクトリ取得
  ZeroMemory(filename,MAX_PATH);
  strncpy(filename,fullpath,filePart - fullpath);
  strcat(filename,fd.cFileName);
}

BOOL LoadPictureWithSPI(char *nameSPI,char *nameFile,unsigned char *bufFile,long sizeFile,HLOCAL *hbuf,HLOCAL *hbmi)
{
  HINSTANCE hSPI;
  char spiVersion[8];
  int (FAR PASCAL *SPI_IsSupported)(LPSTR,DWORD);
  int (FAR PASCAL *SPI_GetPicture)(LPSTR,long,unsigned int,HANDLE *,HANDLE *,FARPROC,long);
  int (FAR PASCAL *SPI_GetPluginInfo)(int,LPSTR,int);
  int ret;

  ret  = FALSE;
  hSPI = NULL;

  //SPI をロード
  hSPI = LoadLibrary(nameSPI);

  if(!hSPI)
    goto error;

  (FARPROC)SPI_GetPluginInfo = GetProcAddress(hSPI,"GetPluginInfo");
  (FARPROC)SPI_IsSupported   = GetProcAddress(hSPI,"IsSupported");
  (FARPROC)SPI_GetPicture    = GetProcAddress(hSPI,"GetPicture");

  if(!SPI_GetPluginInfo || !SPI_IsSupported || !SPI_GetPicture)
    goto error;

  //バージョンチェック
  SPI_GetPluginInfo(0,spiVersion,8);

  if(spiVersion[2] != 'I' || spiVersion[3] != 'N')
    goto error;

  if(!(SPI_IsSupported)(nameFile,(unsigned long)bufFile))
    goto error;

  if((SPI_GetPicture)(bufFile,sizeFile,1,hbmi,hbuf,NULL,0))
    goto error;

  ret = TRUE;

  error :

  if(hSPI)
    FreeLibrary(hSPI);

  return ret;
}

BOOL SaveBitmapFile(char *nameFile,unsigned char *pbuf,BITMAPINFO *pbmi)
{
  int    bmiSize;
  DWORD  writtenByte;
  HANDLE hFile;
  BITMAPFILEHEADER bfh;

  hFile = CreateFile(nameFile,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

  if(hFile == INVALID_HANDLE_VALUE)
    return FALSE;

  bmiSize = pbmi->bmiHeader.biSize;
  
  switch(pbmi->bmiHeader.biBitCount)
  {
    case 1:
      bmiSize += pbmi->bmiHeader.biClrUsed ? sizeof(RGBQUAD) * 2 : 0;
      break;
    
    case 2 :
      bmiSize += sizeof(RGBQUAD) * 4;
      break;

    case 4 :
      bmiSize += sizeof(RGBQUAD) * 16;
      break;

    case 8 : 
      bmiSize += sizeof(RGBQUAD) * 256;
      break;
  }

  ZeroMemory(&bfh,sizeof(bfh));
  bfh.bfType    = MAKEWORD('B','M');
  bfh.bfOffBits = sizeof(bfh) + bmiSize;
  bfh.bfSize    = bfh.bfOffBits + pbmi->bmiHeader.biSizeImage;

  WriteFile(hFile,&bfh,sizeof(bfh)                ,&writtenByte,0);
  WriteFile(hFile,pbmi,bmiSize                    ,&writtenByte,0);
  WriteFile(hFile,pbuf,pbmi->bmiHeader.biSizeImage,&writtenByte,0);

  CloseHandle(hFile);

  return TRUE;
}

BOOL FAR WINAPI AlphaBlendWithoutAPI(HDC hdcDest,int dx,int dy,int width,int height,HDC hdcSrc,int sx,int sy,int sw,int sh,BGBLENDFUNCTION bf)
{
  HDC hdcDestWork,hdcSrcWork;
  int i,invAlpha,alpha;
  int lenBuf;
  unsigned char *bufDest;
  unsigned char *bufSrc;

  if(dx != 0 || dy != 0 || sx != 0 || sy != 0 || width != sw || height != sh)
    return FALSE;

  hdcDestWork = CreateBitmapDC(CreateDIB24BPP(width,height,&bufDest,&lenBuf));
  hdcSrcWork  = CreateBitmapDC(CreateDIB24BPP(width,height,&bufSrc ,&lenBuf));

  if(!bufDest || !bufSrc)
    return FALSE;

  BitBlt(hdcDestWork,0,0,width,height,hdcDest,0,0,SRCCOPY);
  BitBlt(hdcSrcWork ,0,0,width,height,hdcSrc ,0,0,SRCCOPY);

  alpha = bf.SourceConstantAlpha;
  invAlpha = 255 - alpha;

  for(i = 0;i < lenBuf;i++,bufDest++,bufSrc++)
    *bufDest = (*bufDest * invAlpha + *bufSrc * alpha)>>8;

  BitBlt(hdcDest,0,0,width,height,hdcDestWork,0,0,SRCCOPY);

  DeleteBitmapDC(&hdcDestWork);
  DeleteBitmapDC(&hdcSrcWork);

  return TRUE;
}

// 画像読み込み関係

void BGPreloadPicture(BGSrc *src)
{
  char  spiPath[MAX_PATH];
  char  filespec[MAX_PATH];
  char *filePart;
  int   fileSize;
  int   readByte;
  unsigned char *fileBuf;

  HBITMAP hbm;
  HANDLE  hPictureFile;
  HANDLE  hFind;
  WIN32_FIND_DATA fd;

  #ifdef _DEBUG
    dprintf("Preload Picture : %s",src->file);
  #endif

  //ファイルを読み込む
  hPictureFile = CreateFile(src->file,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

  if(hPictureFile == INVALID_HANDLE_VALUE)
    return;
  
  fileSize = GetFileSize(hPictureFile,0);

  //最低 2kb は確保 (Susie plugin の仕様より)
  fileBuf  = GlobalAlloc(GPTR,fileSize + 2048);

  //頭の 2kb は０で初期化
  ZeroMemory(fileBuf,2048);

  ReadFile(hPictureFile,fileBuf,fileSize,&readByte,0);

  CloseHandle(hPictureFile);

  // SPIPath を絶対パスに変換
  if(!GetFullPathName(BGSPIPath,MAX_PATH,filespec,&filePart))
    return;

  //プラグインを当たっていく
  hFind = FindFirstFile(filespec,&fd);

  if(hFind != INVALID_HANDLE_VALUE && filePart)
  {
    //ディレクトリ取得
    ZeroMemory(spiPath,MAX_PATH);
    strncpy(spiPath,filespec,filePart - filespec);

    do{
      HLOCAL hbuf,hbmi;
      BITMAPINFO *pbmi;
      char       *pbuf;
      char spiFileName[MAX_PATH];

      if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        continue;

      strcpy(spiFileName,spiPath);
      strcat(spiFileName,fd.cFileName);

      if(LoadPictureWithSPI(spiFileName,src->file,fileBuf,fileSize,&hbuf,&hbmi))
      {
        pbuf = LocalLock(hbuf);
        pbmi = LocalLock(hbmi);

        SaveBitmapFile(src->fileTmp,pbuf,pbmi);

        LocalUnlock(hbmi);
        LocalUnlock(hbuf);

        LocalFree(hbmi);
        LocalFree(hbuf);
    
        strcpy(src->file,src->fileTmp);

        break;
      }
    }while(FindNextFile(hFind,&fd));

    FindClose(hFind);
  }

  GlobalFree(fileBuf);

  //画像をビットマップとして読み込み

  hbm = LoadImage(0,src->file,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);

  if(hbm)
  {
    BITMAP bm;

    GetObject(hbm,sizeof(bm),&bm);

    src->hdc    = CreateBitmapDC(hbm);
    src->width  = bm.bmWidth;
    src->height = bm.bmHeight;
  }else{
    src->type = BG_COLOR;
  }
}

void BGGetWallpaperInfo(WallpaperInfo *wi)
{
  int  length;
  int  tile;
  char str[256];
  HKEY hKey;

  wi->pattern = BG_CENTER;
  strcpy(wi->filename,"");

  //レジストリキーのオープン
  if(RegOpenKeyEx(HKEY_CURRENT_USER, "Control Panel\\Desktop", 0, KEY_READ, &hKey) != ERROR_SUCCESS)
    return;

  //壁紙名ゲット
  length = MAX_PATH;
  RegQueryValueEx(hKey,"Wallpaper"     ,NULL,NULL,(BYTE*)(wi->filename),&length);

  //壁紙スタイルゲット
  length = 256;
  RegQueryValueEx(hKey,"WallpaperStyle",NULL,NULL,(BYTE*)str,&length);
  wi->pattern = atoi(str);

  //壁紙スタイルゲット
  length = 256;
  RegQueryValueEx(hKey,"TileWallpaper" ,NULL,NULL,(BYTE*)str,&length);
  tile = atoi(str);

  //これでいいの？
  if(tile)
    wi->pattern = BG_TILE;
  else
  if(wi->pattern == 0)
    wi->pattern = BG_CENTER;
  else
  if(wi->pattern == 2)
    wi->pattern = BG_STRETCH;

  //レジストリキーのクローズ
  RegCloseKey(hKey);
}

void BGPreloadWallpaper(BGSrc *src)
{
  HBITMAP       hbm;
  WallpaperInfo wi;

  BGGetWallpaperInfo(&wi);

  //壁紙を読み込み
  //LR_CREATEDIBSECTION を指定するのがコツ
  if(wi.pattern == BG_STRETCH)
    hbm = LoadImage(0,wi.filename,IMAGE_BITMAP,CRTWidth,CRTHeight,LR_LOADFROMFILE | LR_CREATEDIBSECTION);
  else
    hbm = LoadImage(0,wi.filename,IMAGE_BITMAP,        0,       0,LR_LOADFROMFILE);

  //壁紙DCを作る
  if(hbm)
  {
    BITMAP bm;

    GetObject(hbm,sizeof(bm),&bm);

    src->hdc     = CreateBitmapDC(hbm);
    src->width   = bm.bmWidth;
    src->height  = bm.bmHeight;
    src->pattern = wi.pattern;
  }else{
    src->hdc = NULL;
  }

  src->color = GetSysColor(COLOR_DESKTOP);
}

void BGPreloadSrc(BGSrc *src)
{
  DeleteBitmapDC(&(src->hdc));

  switch(src->type)
  {
    case BG_COLOR :
      break;

    case BG_WALLPAPER :
      BGPreloadWallpaper(src);
      break;

    case BG_PICTURE :
      BGPreloadPicture(src);
      break;
  }
}

void BGStretchPicture(HDC hdcDest,BGSrc *src,int x,int y,int width,int height,BOOL bAntiAlias)
{
  if(!hdcDest || !src)
    return;

  if(bAntiAlias)
  {
    if(src->width != width || src->height != height)
    {
      HBITMAP hbm;

      hbm = LoadImage(0,src->file,IMAGE_BITMAP,width,height,LR_LOADFROMFILE);

      if(!hbm)
        return;
      
      DeleteBitmapDC(&(src->hdc));
      src->hdc = CreateBitmapDC(hbm);
      src->width  = width;
      src->height = height;
    }
  
    BitBlt(hdcDest,x,y,width,height,src->hdc,0,0,SRCCOPY);
  }else{
    SetStretchBltMode(src->hdc,COLORONCOLOR);
    StretchBlt(hdcDest,x,y,width,height,src->hdc,0,0,src->width,src->height,SRCCOPY);
  }
}

void BGLoadPicture(HDC hdcDest,BGSrc *src)
{
  int x,y,width,height,pattern;
  HDC hdc = NULL;

  FillBitmapDC(hdcDest,src->color);

  if(!src->height || !src->width)
    return;

  if(src->pattern == BG_AUTOFIT){
    if((src->height * ScreenWidth) > (ScreenHeight * src->width))
      pattern = BG_FIT_WIDTH;
    else
      pattern = BG_FIT_HEIGHT;
  }else{
    pattern = src->pattern;
  }

  switch(pattern)
  {
    case BG_STRETCH :
      BGStretchPicture(hdcDest,src,0,0,ScreenWidth,ScreenHeight,src->antiAlias);
      break;

    case BG_FIT_WIDTH :

      height = (src->height * ScreenWidth) / src->width;
      y      = (ScreenHeight - height) / 2;
      
      BGStretchPicture(hdcDest,src,0,y,ScreenWidth,height,src->antiAlias);
      break;

    case BG_FIT_HEIGHT :

      width = (src->width * ScreenHeight) / src->height;
      x     = (ScreenWidth - width) / 2;

      BGStretchPicture(hdcDest,src,x,0,width,ScreenHeight,src->antiAlias);
      break;

    case BG_TILE :
      for(x = 0;x < ScreenWidth ;x += src->width )
      for(y = 0;y < ScreenHeight;y += src->height)
        BitBlt(hdcDest,x,y,src->width,src->height,src->hdc,0,0,SRCCOPY);
      break;

    case BG_CENTER :
      x = (ScreenWidth  -  src->width) / 2;
      y = (ScreenHeight - src->height) / 2;

      BitBlt(hdcDest,x,y,src->width,src->height,src->hdc,0,0,SRCCOPY);
      break;
  }
}

typedef struct tagLoadWallpaperStruct
{
  RECT *rectClient;
  HDC hdcDest;
  BGSrc *src;
}LoadWallpaperStruct;

BOOL CALLBACK BGLoadWallpaperEnumFunc(HMONITOR hMonitor,HDC hdcMonitor,LPRECT lprcMonitor,LPARAM dwData)
{
  RECT rectDest;
  RECT rectRgn;
  int  monitorWidth;
  int  monitorHeight;
  int  destWidth;
  int  destHeight;
  HRGN hRgn;
  int  x;
  int  y;

  LoadWallpaperStruct *lws = (LoadWallpaperStruct*)dwData;

  if(!IntersectRect(&rectDest,lprcMonitor,lws->rectClient))
    return TRUE;

  //モニターにかかってる部分をマスク
  SaveDC(lws->hdcDest);
  CopyRect(&rectRgn,&rectDest);
  OffsetRect(&rectRgn,- lws->rectClient->left,- lws->rectClient->top);
  hRgn = CreateRectRgnIndirect(&rectRgn);
  SelectObject(lws->hdcDest,hRgn);

  //モニターの大きさ
  monitorWidth  = lprcMonitor->right  - lprcMonitor->left;
  monitorHeight = lprcMonitor->bottom - lprcMonitor->top;

  destWidth  = rectDest.right  - rectDest.left;
  destHeight = rectDest.bottom - rectDest.top;

  switch(lws->src->pattern)
  {
    case BG_CENTER  :
    case BG_STRETCH :

      SetWindowOrgEx(lws->src->hdc,
                     lprcMonitor->left + (monitorWidth  - lws->src->width )/2,
                     lprcMonitor->top  + (monitorHeight - lws->src->height)/2,NULL);
      BitBlt(lws->hdcDest ,rectDest.left,rectDest.top,destWidth,destHeight,
             lws->src->hdc,rectDest.left,rectDest.top,SRCCOPY);

      break;
    case BG_TILE :
      
      SetWindowOrgEx(lws->src->hdc,0,0,NULL);

      for(x = rectDest.left - (rectDest.left % lws->src->width ) - lws->src->width ;
          x < rectDest.right ;x += lws->src->width )
      for(y = rectDest.top  - (rectDest.top  % lws->src->height) - lws->src->height;
          y < rectDest.bottom;y += lws->src->height)
        BitBlt(lws->hdcDest,x,y,lws->src->width,lws->src->height,lws->src->hdc,0,0,SRCCOPY);
      break;
  }

  //リージョンを破棄
  RestoreDC(lws->hdcDest,-1);
  DeleteObject(hRgn);

  return TRUE;
}

void BGLoadWallpaper(HDC hdcDest,BGSrc *src)
{
  RECT  rectClient;
  POINT point;
  LoadWallpaperStruct lws;

  //取りあえずデスクトップ色で塗りつぶす
  FillBitmapDC(hdcDest,src->color);

  //壁紙が設定されていない
  if(!src->hdc)
    return;

  //hdcDestの座標系を仮想スクリーンに合わせる
  point.x = 0;
  point.y = 0;
  ClientToScreen(HVTWin,&point);

  SetWindowOrgEx(hdcDest,point.x,point.y,NULL);

  //仮想スクリーンでのクライアント領域
  GetClientRect(HVTWin,&rectClient);
  OffsetRect(&rectClient,point.x,point.y);

  //モニターを列挙
  lws.rectClient = &rectClient;
  lws.src        = src;
  lws.hdcDest    = hdcDest;

  if(BGEnumDisplayMonitors)
  {
    (*BGEnumDisplayMonitors)(NULL,NULL,BGLoadWallpaperEnumFunc,(LPARAM)&lws);
  }else{
    RECT rectMonitor;
  
    SetRect(&rectMonitor,0,0,CRTWidth,CRTHeight);
    BGLoadWallpaperEnumFunc(NULL,NULL,&rectMonitor,(LPARAM)&lws);
  }

  //座標系を戻す
  SetWindowOrgEx(hdcDest,0,0,NULL);
}

void BGLoadSrc(HDC hdcDest,BGSrc *src)
{
  switch(src->type)
  {
    case BG_COLOR :
      FillBitmapDC(hdcDest,src->color);
      break;

    case BG_WALLPAPER :
      BGLoadWallpaper(hdcDest,src);
      break;

    case BG_PICTURE :
      BGLoadPicture(hdcDest,src);
      break;
  }
}

void BGSetupPrimary(BOOL forceSetup)
{
  POINT point;
  RECT rect;

  if(!BGEnable)
    return;

  //窓の位置、大きさが変わったかチェック
  point.x = 0;
  point.y = 0;
  ClientToScreen(HVTWin,&point);
  
  GetClientRect(HVTWin,&rect);
  OffsetRect(&rect,point.x,point.y);

  if(!forceSetup && EqualRect(&rect,&BGPrevRect))
    return;

  CopyRect(&BGPrevRect,&rect);

  #ifdef _DEBUG
    dprintf("BGSetupPrimary : BGInSizeMove = %d",BGInSizeMove);
  #endif

  //作業用 DC 作成
  if(hdcBGWork)   DeleteBitmapDC(&hdcBGWork);
  if(hdcBGBuffer) DeleteBitmapDC(&hdcBGBuffer);

  hdcBGWork   = CreateBitmapDC(CreateScreenCompatibleBitmap(ScreenWidth,FontHeight));
  hdcBGBuffer = CreateBitmapDC(CreateScreenCompatibleBitmap(ScreenWidth,FontHeight));

  //hdcBGBuffer の属性設定
  SetBkMode(hdcBGBuffer,TRANSPARENT);

  if(!BGInSizeMove)
  {
    BGBLENDFUNCTION bf;
    HDC hdcSrc = NULL;

    //背景 HDC
    if(hdcBG) DeleteBitmapDC(&hdcBG);
      hdcBG = CreateBitmapDC(CreateScreenCompatibleBitmap(ScreenWidth,ScreenHeight));

    //作業用DC
    hdcSrc = CreateBitmapDC(CreateScreenCompatibleBitmap(ScreenWidth,ScreenHeight));

    //背景生成
    BGLoadSrc(hdcBG,&BGDest);

    ZeroMemory(&bf,sizeof(bf));
    bf.BlendOp = AC_SRC_OVER;

    if(bf.SourceConstantAlpha = BGSrc1.alpha)
    {
      BGLoadSrc(hdcSrc,&BGSrc1);
      (BGAlphaBlend)(hdcBG,0,0,ScreenWidth,ScreenHeight,hdcSrc,0,0,ScreenWidth,ScreenHeight,bf);
    }

    if(bf.SourceConstantAlpha = BGSrc2.alpha)
    {
      BGLoadSrc(hdcSrc,&BGSrc2);
      (BGAlphaBlend)(hdcBG,0,0,ScreenWidth,ScreenHeight,hdcSrc,0,0,ScreenWidth,ScreenHeight,bf);
    }

    DeleteBitmapDC(&hdcSrc);
  }
}

COLORREF BGGetColor(char *name,COLORREF defcolor,char *file)
{
  unsigned int r,g,b;
  char colorstr[256],defstr[256];

  sprintf(defstr,"%d,%d,%d",GetRValue(defcolor),GetGValue(defcolor),GetBValue(defcolor));

  GetPrivateProfileString(BG_SECTION,name,defstr,colorstr,255,file);

  r = g = b = 0;

  sscanf(colorstr,"%d , %d , %d",&r,&g,&b);

  return RGB(r,g,b);
}

BG_PATTERN BGGetStrIndex(char *name,BG_PATTERN def,char *file,char **strList,int nList)
{
  char defstr[64],str[64];
  int  i;

  def %= nList;

  strcpy(defstr,strList[def]);
  GetPrivateProfileString(BG_SECTION,name,defstr,str,64,file);

  for(i = 0;i < nList;i++)
    if(!stricmp(str,strList[i]))
      return i;

  return 0;
}

BOOL BGGetOnOff(char *name,BOOL def,char *file)
{
  char *strList[2] = {"Off","On"};

  return BGGetStrIndex(name,def,file,strList,2);
}

BG_PATTERN BGGetPattern(char *name,BG_PATTERN def,char *file)
{
  char *strList[6]={"stretch","tile","center","fitwidth","fitheight","autofit"};

  return BGGetStrIndex(name,def,file,strList,6);
}

BG_PATTERN BGGetType(char *name,BG_TYPE def,char *file)
{
  char *strList[3]={"color","picture","wallpaper"};

  return BGGetStrIndex(name,def,file,strList,3);
}

void BGReadTextColorConfig(char *file)
{
  ANSIColor[IdFore   ] = BGGetColor("Fore"   ,ANSIColor[IdFore],file);
  ANSIColor[IdBack   ] = BGGetColor("Back"   ,ANSIColor[IdBack],file);
  ANSIColor[IdRed    ] = BGGetColor("Red"    ,ANSIColor[IdRed    ],file);
  ANSIColor[IdGreen  ] = BGGetColor("Green"  ,ANSIColor[IdGreen  ],file);
  ANSIColor[IdYellow ] = BGGetColor("Yellow" ,ANSIColor[IdYellow ],file);
  ANSIColor[IdBlue   ] = BGGetColor("Blue"   ,ANSIColor[IdBlue   ],file);
  ANSIColor[IdMagenta] = BGGetColor("Magenta",ANSIColor[IdMagenta],file);
  ANSIColor[IdCyan   ] = BGGetColor("Cyan"   ,ANSIColor[IdCyan   ],file);

  ANSIColor[IdFore   + 8] = BGGetColor("DarkFore"   ,ANSIColor[IdFore   + 8],file);
  ANSIColor[IdBack   + 8] = BGGetColor("DarkBack"   ,ANSIColor[IdBack   + 8],file);
  ANSIColor[IdRed    + 8] = BGGetColor("DarkRed"    ,ANSIColor[IdRed    + 8],file);
  ANSIColor[IdGreen  + 8] = BGGetColor("DarkGreen"  ,ANSIColor[IdGreen  + 8],file);
  ANSIColor[IdYellow + 8] = BGGetColor("DarkYellow" ,ANSIColor[IdYellow + 8],file);
  ANSIColor[IdBlue   + 8] = BGGetColor("DarkBlue"   ,ANSIColor[IdBlue   + 8],file);
  ANSIColor[IdMagenta+ 8] = BGGetColor("DarkMagenta",ANSIColor[IdMagenta+ 8],file);
  ANSIColor[IdCyan   + 8] = BGGetColor("DarkCyan"   ,ANSIColor[IdCyan   + 8],file);

  BGVTColor[0]      = BGGetColor("VTFore",BGVTColor[0],file);
  BGVTColor[1]      = BGGetColor("VTBack",BGVTColor[1],file);

  BGVTBlinkColor[0] = BGGetColor("VTBlinkFore",BGVTBlinkColor[0],file);
  BGVTBlinkColor[1] = BGGetColor("VTBlinkBack",BGVTBlinkColor[1],file);

  BGVTBoldColor[0]  = BGGetColor("VTBoldFore" ,BGVTBoldColor[0],file);
  BGVTBoldColor[1]  = BGGetColor("VTBoldBack" ,BGVTBoldColor[1],file);

  /* begin - ishizaki */
  BGURLColor[0]     = BGGetColor("URLFore" ,BGURLColor[0],file);
  BGURLColor[1]     = BGGetColor("URLBack" ,BGURLColor[1],file);
  /* end - ishizaki */
}

void BGReadIniFile(char *file)
{
  char path[MAX_PATH];

  // Easy Setting
  BGDest.pattern = BGGetPattern("BGPicturePattern",BGSrc1.pattern,file);
  BGDest.color   = BGGetColor("BGPictureBaseColor",BGSrc1.color,file);

  GetPrivateProfileString(BG_SECTION,"BGPictureFile",BGSrc1.file,path,MAX_PATH,file);
  RandomFile(path,BGDest.file);

  BGSrc1.alpha   = 255 - GetPrivateProfileInt(BG_SECTION,"BGPictureTone",255 - BGSrc1.alpha,file);

  if(!strcmp(BGDest.file,""))
    BGSrc1.alpha = 255;

  BGSrc2.alpha   = 255 - GetPrivateProfileInt(BG_SECTION,"BGFadeTone",255 - BGSrc2.alpha,file);
  BGSrc2.color   = BGGetColor("BGFadeColor",BGSrc2.color,file);

  BGReverseTextAlpha = GetPrivateProfileInt(BG_SECTION,"BGReverseTextTone",BGReverseTextAlpha,file);

  //Src1 の読み出し
  BGSrc1.type      = BGGetType("BGSrc1Type",BGSrc1.type,file);
  BGSrc1.pattern   = BGGetPattern("BGSrc1Pattern",BGSrc1.pattern,file);
  BGSrc1.antiAlias = BGGetOnOff("BGSrc1AntiAlias",BGSrc1.antiAlias,file);
  BGSrc1.alpha     = GetPrivateProfileInt(BG_SECTION,"BGSrc1Alpha"  ,BGSrc1.alpha  ,file);
  BGSrc1.color     = BGGetColor("BGSrc1Color",BGSrc1.color,file);

  GetPrivateProfileString(BG_SECTION,"BGSrc1File",BGSrc1.file,path,MAX_PATH,file);
  RandomFile(path,BGSrc1.file);
  
  //Src2 の読み出し
  BGSrc2.type      = BGGetType("BGSrc2Type",BGSrc2.type,file);
  BGSrc2.pattern   = BGGetPattern("BGSrc2Pattern",BGSrc2.pattern,file);
  BGSrc2.antiAlias = BGGetOnOff("BGSrc2AntiAlias",BGSrc2.antiAlias,file);
  BGSrc2.alpha     = GetPrivateProfileInt(BG_SECTION,"BGSrc2Alpha"  ,BGSrc2.alpha  ,file);
  BGSrc2.color     = BGGetColor("BGSrc2Color",BGSrc2.color,file);

  GetPrivateProfileString(BG_SECTION,"BGSrc2File",BGSrc2.file,path,MAX_PATH,file);
  RandomFile(path,BGSrc2.file);

  //Dest の読み出し
  BGDest.type      = BGGetType("BGDestType",BGDest.type,file);
  BGDest.pattern   = BGGetPattern("BGDestPattern",BGDest.pattern,file);
  BGDest.antiAlias = BGGetOnOff("BGDestAntiAlias",BGDest.antiAlias,file);
  BGDest.color     = BGGetColor("BGDestColor",BGDest.color,file);

  GetPrivateProfileString(BG_SECTION,"BGDestFile",BGDest.file,path,MAX_PATH,file);
  RandomFile(path,BGDest.file);

  //その他読み出し
  BGReverseTextAlpha = GetPrivateProfileInt(BG_SECTION,"BGReverseTextAlpha",BGReverseTextAlpha,file);
  BGReadTextColorConfig(file);
}

void BGDestruct(void)
{
  if(!BGEnable)
    return;

  DeleteBitmapDC(&hdcBGBuffer);
  DeleteBitmapDC(&hdcBGWork);
  DeleteBitmapDC(&hdcBG);
  DeleteBitmapDC(&(BGDest.hdc));
  DeleteBitmapDC(&(BGSrc1.hdc));
  DeleteBitmapDC(&(BGSrc2.hdc));

  //テンポラリーファイル削除
  DeleteFile(BGDest.fileTmp);
  DeleteFile(BGSrc1.fileTmp);
  DeleteFile(BGSrc2.fileTmp);
}

void BGInitialize(void)
{
  char path[MAX_PATH],config_file[MAX_PATH],tempPath[MAX_PATH];
  int i;

  // VTColor を読み込み
  BGVTColor[0] = ts.VTColor[0];
  BGVTColor[1] = ts.VTColor[1];

  BGVTBoldColor[0] = ts.VTBoldColor[0];
  BGVTBoldColor[1] = ts.VTBoldColor[1];

  BGVTBlinkColor[0] = ts.VTBlinkColor[0];
  BGVTBlinkColor[1] = ts.VTBlinkColor[1];

#if 0
  /* begin - ishizaki */
  BGURLColor[0] = ts.URLColor[0];
  BGURLColor[1] = ts.URLColor[1];
  /* end - ishizaki */
#else
  // TODO: ハイパーリンクの描画がリアルタイムに行われないことがあるので、
  // 色属性変更はいったん取りやめることにする。将来、対応する。(2005.4.3 yutaka)
  BGURLColor[0] = ts.VTColor[0];
  BGURLColor[1] = ts.VTColor[1];
#endif

  // ANSI color設定のほうを優先させる (2005.2.3 yutaka)
#ifndef NO_ANSI_COLOR_EXTENSION
  for (i = IdBack ; i <= IdFore+8 ; i++)
    ANSIColor[i] = ts.ANSIColor[i];
  if ((ts.ColorFlag & CF_USETEXTCOLOR)!=0) // use background color for "Black"
    ANSIColor[IdBack ]   = ts.VTColor[1];
  if ((ts.ColorFlag & CF_USETEXTCOLOR)!=0) // use text color for "white"
    ANSIColor[IdFore ]   = ts.VTColor[0];
#else /* NO_ANSI_COLOR_EXTENSION */
  ANSIColor[IdBack ]     = RGB(  0,  0,  0);
  ANSIColor[IdRed  ]     = RGB(255,  0,  0);
  ANSIColor[IdGreen]     = RGB(  0,255,  0);
  ANSIColor[IdYellow]    = RGB(255,255,  0);
  ANSIColor[IdBlue]      = RGB(  0,  0,255);
  ANSIColor[IdMagenta]   = RGB(255,  0,255);
  ANSIColor[IdCyan]      = RGB(  0,255,255);
  ANSIColor[IdFore ]     = RGB(255,255,255);
  ANSIColor[IdBack+8]    = RGB(128,128,128);
  ANSIColor[IdRed+8]     = RGB(128,  0,  0);
  ANSIColor[IdGreen+8]   = RGB(  0,128,  0);
  ANSIColor[IdYellow+8]	 = RGB(128,128,  0);
  ANSIColor[IdBlue+8]    = RGB(  0,  0,128);
  ANSIColor[IdMagenta+8] = RGB(128,  0,128);
  ANSIColor[IdCyan+8]    = RGB(  0,128,128);
  ANSIColor[IdFore+8]    = RGB(192,192,192);
#endif

  //リソース解放
  BGDestruct();

  //BG が有効かチェック
  BGEnable = BGGetOnOff("BGEnable",FALSE,ts.SetupFName);

  if(!BGEnable)
    return;

  //乱数初期化
  srand(time(NULL));

  //BGシステム設定読み出し
  BGUseAlphaBlendAPI = BGGetOnOff("BGUseAlphaBlendAPI",TRUE ,ts.SetupFName);
  BGNoFrame          = BGGetOnOff("BGNoFrame"         ,FALSE,ts.SetupFName);
  BGFastSizeMove     = BGGetOnOff("BGFastSizeMove"    ,TRUE ,ts.SetupFName);
  BGNoCopyBits       = BGGetOnOff("BGFlickerlessMove" ,TRUE ,ts.SetupFName);
  
  GetPrivateProfileString(BG_SECTION,"BGSPIPath","plugin",BGSPIPath,MAX_PATH,ts.SetupFName);

  //テンポラリーファイル名を生成
  GetTempPath(MAX_PATH,tempPath);
  GetTempFileName(tempPath,"ttAK",0,BGDest.fileTmp);
  GetTempFileName(tempPath,"ttAK",0,BGSrc1.fileTmp);
  GetTempFileName(tempPath,"ttAK",0,BGSrc2.fileTmp);
  
  //デフォルト値
  BGDest.type      = BG_PICTURE;
  BGDest.pattern   = BG_STRETCH;
  BGDest.color     = RGB(0,0,0);
  BGDest.antiAlias = TRUE;
  strcpy(BGDest.file,"");

  BGSrc1.type      = BG_WALLPAPER;
  BGSrc1.pattern   = BG_STRETCH;
  BGSrc1.color     = RGB(255,255,255);
  BGSrc1.antiAlias = TRUE;
  BGSrc1.alpha     = 255;
  strcpy(BGSrc1.file,"");

  BGSrc2.type      = BG_COLOR;
  BGSrc2.pattern   = BG_STRETCH;
  BGSrc2.color     = RGB(0,0,0);
  BGSrc2.antiAlias = TRUE;
  BGSrc2.alpha     = 128;
  strcpy(BGSrc2.file,"");

  BGReverseTextAlpha = 255;

  //設定の読み出し
  BGReadIniFile(ts.SetupFName);

  //コンフィグファイルの決定
  GetPrivateProfileString(BG_SECTION,"BGThemeFile","",path,MAX_PATH,ts.SetupFName);
  RandomFile(path,config_file);

  //設定のオーバーライド
  if(strcmp(config_file,""))
  {
    char dir[MAX_PATH],prevDir[MAX_PATH];
    
    //INIファイルのあるディレクトリに一時的に移動
    GetCurrentDirectory(MAX_PATH,prevDir);

    ExtractDirName(config_file,dir);
    SetCurrentDirectory(dir);

    BGReadIniFile(config_file);

    SetCurrentDirectory(prevDir);
  }

  //SPI のパスを整形
  AppendSlash(BGSPIPath);
  strcat(BGSPIPath,"*");

  //壁紙 or 背景をプリロード
  BGPreloadSrc(&BGDest);
  BGPreloadSrc(&BGSrc1);
  BGPreloadSrc(&BGSrc2);

  // AlphaBlend のアドレスを読み込み
  if(BGUseAlphaBlendAPI)
    (FARPROC)BGAlphaBlend = GetProcAddressWithDllName("msimg32.dll","AlphaBlend");
  else
    BGAlphaBlend = NULL;

  if(!BGAlphaBlend)
    BGAlphaBlend = AlphaBlendWithoutAPI;

  //EnumDisplayMonitors を探す
  (FARPROC)BGEnumDisplayMonitors = GetProcAddressWithDllName("user32.dll","EnumDisplayMonitors");
}

void BGFillRect(HDC hdc,RECT *R,HBRUSH brush)
{
  if(!BGEnable)
    FillRect(hdc,R,brush);
  else
    BitBlt(VTDC,R->left,R->top,R->right - R->left,R->bottom - R->top,hdcBG,R->left,R->top,SRCCOPY);
}

void BGScrollWindow(HWND hwnd,int xa,int ya,RECT *Rect,RECT *ClipRect)
{
  if(!BGEnable)
    ScrollWindow(hwnd,xa,ya,Rect,ClipRect);
  else
    InvalidateRect(HVTWin,ClipRect,FALSE);
}

void BGOnEnterSizeMove(void)
{
  int  r,g,b;

  if(!BGEnable || !BGFastSizeMove)
    return;

  BGInSizeMove = TRUE;

  //背景色生成
  r = GetRValue(BGDest.color);
  g = GetGValue(BGDest.color);
  b = GetBValue(BGDest.color);

  r = (r * (255 - BGSrc1.alpha) + GetRValue(BGSrc1.color) * BGSrc1.alpha) >> 8;
  g = (g * (255 - BGSrc1.alpha) + GetGValue(BGSrc1.color) * BGSrc1.alpha) >> 8;
  b = (b * (255 - BGSrc1.alpha) + GetBValue(BGSrc1.color) * BGSrc1.alpha) >> 8;

  r = (r * (255 - BGSrc2.alpha) + GetRValue(BGSrc2.color) * BGSrc2.alpha) >> 8;
  g = (g * (255 - BGSrc2.alpha) + GetGValue(BGSrc2.color) * BGSrc2.alpha) >> 8;
  b = (b * (255 - BGSrc2.alpha) + GetBValue(BGSrc2.color) * BGSrc2.alpha) >> 8;

  BGBrushInSizeMove = CreateSolidBrush(RGB(r,g,b));
}

void BGOnExitSizeMove(void)
{
  if(!BGEnable || !BGFastSizeMove)
    return;

  BGInSizeMove = FALSE;

  BGSetupPrimary(TRUE);
  InvalidateRect(HVTWin,NULL,FALSE);

  //ブラシを削除
  if(BGBrushInSizeMove)
  {
    DeleteObject(BGBrushInSizeMove);
    BGBrushInSizeMove = NULL;
  }
}

void BGOnSettingChange(void)
{
  if(!BGEnable)
    return;

  CRTWidth  = GetSystemMetrics(SM_CXSCREEN);
  CRTHeight = GetSystemMetrics(SM_CYSCREEN);

  //壁紙 or 背景をプリロード
  BGPreloadSrc(&BGDest);
  BGPreloadSrc(&BGSrc1);
  BGPreloadSrc(&BGSrc2);

  BGSetupPrimary(TRUE);
  InvalidateRect(HVTWin,0,FALSE);
}

//-->
#endif  // ALPHABLEND_TYPE2

void InitDisp()
{
  HDC TmpDC;
  int i;
  BOOL bMultiDisplaySupport = FALSE;

  TmpDC = GetDC(NULL);

#ifndef NO_ANSI_COLOR_EXTENSION
  for (i = IdBack ; i <= IdFore+8 ; i++)
    ANSIColor[i] = ts.ANSIColor[i];
  if ((ts.ColorFlag & CF_USETEXTCOLOR)!=0) // use background color for "Black"
    ANSIColor[IdBack ]   = ts.VTColor[1];
  if ((ts.ColorFlag & CF_USETEXTCOLOR)!=0) // use text color for "white"
    ANSIColor[IdFore ]   = ts.VTColor[0];
#else /* NO_ANSI_COLOR_EXTENSION */
  if ((ts.ColorFlag & CF_USETEXTCOLOR)==0)
    ANSIColor[IdBack ]   = RGB(  0,  0,  0);
  else // use background color for "Black"
    ANSIColor[IdBack ]   = ts.VTColor[1];
  ANSIColor[IdRed  ]     = RGB(255,  0,  0);
  ANSIColor[IdGreen]     = RGB(  0,255,  0);
  ANSIColor[IdYellow]    = RGB(255,255,  0);
  ANSIColor[IdBlue]      = RGB(  0,  0,255);
  ANSIColor[IdMagenta]   = RGB(255,  0,255);
  ANSIColor[IdCyan]      = RGB(  0,255,255);
  if ((ts.ColorFlag & CF_USETEXTCOLOR)==0)
    ANSIColor[IdFore ]   = RGB(255,255,255);
  else // use text color for "white"
    ANSIColor[IdFore ]   = ts.VTColor[0];

  ANSIColor[IdBack+8]    = RGB(128,128,128);
  ANSIColor[IdRed+8]     = RGB(128,  0,  0);
  ANSIColor[IdGreen+8]   = RGB(  0,128,  0);
  ANSIColor[IdYellow+8]	 = RGB(128,128,  0);
  ANSIColor[IdBlue+8]    = RGB(  0,  0,128);
  ANSIColor[IdMagenta+8] = RGB(128,  0,128);
  ANSIColor[IdCyan+8]    = RGB(  0,128,128);
  ANSIColor[IdFore+8]    = RGB(192,192,192);
#endif /* NO_ANSI_COLOR_EXTENSION */

#ifdef ALPHABLEND_TYPE2
  CRTWidth  = GetSystemMetrics(SM_CXSCREEN);
  CRTHeight = GetSystemMetrics(SM_CYSCREEN);

  BGInitialize();

  if ((ts.ColorFlag & CF_USETEXTCOLOR)!=0)
  {
    ANSIColor[IdBack ]   = BGVTColor[1];
    ANSIColor[IdFore ]   = BGVTColor[0];
  }
#endif  // ALPHABLEND_TYPE2

  for (i = IdBack ; i <= IdFore+8 ; i++)
    ANSIColor[i] = GetNearestColor(TmpDC, ANSIColor[i]);

  /* background paintbrush */
  Background = CreateSolidBrush(ts.VTColor[1]);
  /* CRT width & height */
  {
	OSVERSIONINFO ver;
	ZeroMemory( &ver, sizeof(ver) );
	ver.dwOSVersionInfoSize = sizeof(ver);
	GetVersionEx( &ver );
	switch( ver.dwPlatformId ) {
	// Windows 9x か NT かの判定
	case VER_PLATFORM_WIN32_WINDOWS:
		if( ver.dwMajorVersion > 4 ||
			(ver.dwMajorVersion == 4 && ver.dwMinorVersion >= 10) ) // Windows 98 or later
			bMultiDisplaySupport = TRUE;
		break;
	case VER_PLATFORM_WIN32_NT:
		if( ver.dwMajorVersion >= 5 ) // Windows 2000 or later
			bMultiDisplaySupport = TRUE;
		break;
	default:
		break;
	}
  }
  if( bMultiDisplaySupport ) {
	  VirtualScreen.left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	  VirtualScreen.top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	  VirtualScreen.right = VirtualScreen.left +  GetSystemMetrics(SM_CXVIRTUALSCREEN);
	  VirtualScreen.bottom = VirtualScreen.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
  } else {
	  VirtualScreen.left = 0;
	  VirtualScreen.top = 0;
	  VirtualScreen.right = GetDeviceCaps(TmpDC,HORZRES);
	  VirtualScreen.bottom = GetDeviceCaps(TmpDC,VERTRES);
  }

  ReleaseDC(NULL, TmpDC);

  if ( (ts.VTPos.x > VirtualScreen.right) || (ts.VTPos.y > VirtualScreen.bottom) )
  {
    ts.VTPos.x = CW_USEDEFAULT;
    ts.VTPos.y = CW_USEDEFAULT;
  }
  else if ( (ts.VTPos.x < VirtualScreen.left-20) || (ts.VTPos.y < VirtualScreen.top-20) )
  {
    ts.VTPos.x = CW_USEDEFAULT;
    ts.VTPos.y = CW_USEDEFAULT;
  }
  else {
    if ( ts.VTPos.x < VirtualScreen.left ) ts.VTPos.x = VirtualScreen.left;
    if ( ts.VTPos.y < VirtualScreen.top ) ts.VTPos.y = VirtualScreen.top;
  }

  if ( (ts.TEKPos.x >  VirtualScreen.right) || (ts.TEKPos.y > VirtualScreen.bottom) )
  {
    ts.TEKPos.x = CW_USEDEFAULT;
    ts.TEKPos.y = CW_USEDEFAULT;
  }
  else if ( (ts.TEKPos.x < VirtualScreen.left-20) || (ts.TEKPos.y < VirtualScreen.top-20) )
  {
    ts.TEKPos.x = CW_USEDEFAULT;
    ts.TEKPos.y = CW_USEDEFAULT;
  }
  else {
    if ( ts.TEKPos.x < VirtualScreen.left ) ts.TEKPos.x = VirtualScreen.left;
    if ( ts.TEKPos.y < VirtualScreen.top ) ts.TEKPos.y = VirtualScreen.top;
  }
}

void EndDisp()
{
  int i, j;

  if (VTDC!=NULL) DispReleaseDC();

  /* Delete fonts */
  for (i = 0 ; i <= AttrFontMask; i++)
  {
    for (j = i+1 ; j <= AttrFontMask ; j++)
      if (VTFont[j]==VTFont[i])
        VTFont[j] = 0;
    if (VTFont[i]!=0) DeleteObject(VTFont[i]);
  }

  if (Background!=0)
  {
	DeleteObject(Background);
	Background = 0;
  }

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
  BGDestruct();
//-->
#endif  // ALPHABLEND_TYPE2

}

void DispReset()
{
  /* Cursor */
  CursorX = 0;
  CursorY = 0;

  /* Scroll status */
  ScrollCount = 0;
  dScroll = 0;

  if (IsCaretOn()) CaretOn();
  DispEnableCaret(TRUE); // enable caret
}

void DispConvWinToScreen
  (int Xw, int Yw, int *Xs, int *Ys, PBOOL Right)
// Converts window coordinate to screen cordinate
//   Xs: horizontal position in window coordinate (pixels)
//   Ys: vertical
//  Output
//	 Xs, Ys: screen coordinate
//   Right: TRUE if the (Xs,Ys) is on the right half of
//			 a character cell.
{
  if (Xs!=NULL)
	*Xs = Xw / FontWidth + WinOrgX;
  *Ys = Yw / FontHeight + WinOrgY;
  if ((Xs!=NULL) && (Right!=NULL))
    *Right = (Xw - (*Xs-WinOrgX)*FontWidth) >= FontWidth/2;
}

void SetLogFont()
{
  memset(&VTlf, 0, sizeof(LOGFONT));
  VTlf.lfWeight = FW_NORMAL;
  VTlf.lfItalic = 0;
  VTlf.lfUnderline = 0;
  VTlf.lfStrikeOut = 0;
  VTlf.lfWidth = ts.VTFontSize.x;
  VTlf.lfHeight = ts.VTFontSize.y;
  VTlf.lfCharSet = ts.VTFontCharSet;
  VTlf.lfOutPrecision  = OUT_CHARACTER_PRECIS;
  VTlf.lfClipPrecision = CLIP_CHARACTER_PRECIS;
  VTlf.lfQuality       = DEFAULT_QUALITY;
  VTlf.lfPitchAndFamily = FIXED_PITCH | FF_DONTCARE;
  strcpy(VTlf.lfFaceName,ts.VTFont);
}

void ChangeFont()
{
  int i, j;
  TEXTMETRIC Metrics;
  HDC TmpDC;

  /* Delete Old Fonts */
  for (i = 0 ; i <= AttrFontMask ; i++)
  {
    for (j = i+1 ; j <= AttrFontMask ; j++)
      if (VTFont[j]==VTFont[i])
        VTFont[j] = 0;
    if (VTFont[i]!=0)
      DeleteObject(VTFont[i]);
  }

  /* Normal Font */
  SetLogFont();
  VTFont[0] = CreateFontIndirect(&VTlf);

  /* set IME font */
  SetConversionLogFont(&VTlf);

  TmpDC = GetDC(HVTWin);

  SelectObject(TmpDC, VTFont[0]);
  GetTextMetrics(TmpDC, &Metrics); 
  FontWidth = Metrics.tmAveCharWidth + ts.FontDW;
  FontHeight = Metrics.tmHeight + ts.FontDH;

  ReleaseDC(HVTWin,TmpDC);

  /* Underline */
  VTlf.lfUnderline = 1;
  VTFont[AttrUnder] = CreateFontIndirect(&VTlf);

  if (ts.EnableBold>0)
  {
    /* Bold */
    VTlf.lfUnderline = 0;
    VTlf.lfWeight = FW_BOLD;
    VTFont[AttrBold] = CreateFontIndirect(&VTlf);
    /* Bold + Underline */
    VTlf.lfUnderline = 1;
    VTFont[AttrBold | AttrUnder] = CreateFontIndirect(&VTlf);
  }
  else {
    VTFont[AttrBold] = VTFont[AttrDefault];
    VTFont[AttrBold | AttrUnder] = VTFont[AttrUnder];
  }

  /* Special font */
  VTlf.lfWeight = FW_NORMAL;
  VTlf.lfUnderline = 0;
  VTlf.lfWidth = FontWidth + 1; /* adjust width */
  VTlf.lfHeight = FontHeight;
  VTlf.lfCharSet = SYMBOL_CHARSET;

  strcpy(VTlf.lfFaceName,"Tera Special");
  VTFont[AttrSpecial] = CreateFontIndirect(&VTlf);
  VTFont[AttrSpecial | AttrBold] = VTFont[AttrSpecial];
  VTFont[AttrSpecial | AttrUnder] = VTFont[AttrSpecial];
  VTFont[AttrSpecial | AttrBold | AttrUnder] = VTFont[AttrSpecial];

  SetLogFont();

  for (i = 0 ; i <= 255; i++)
    Dx[i] = FontWidth;
}

void ResetIME()
{
  /* reset language for communication */
  cv.Language = ts.Language;

  /* reset IME */
  if (ts.Language==IdJapanese)
  {
    if (ts.UseIME==0)
      FreeIME();
    else if (! LoadIME())
      ts.UseIME = 0;

	if (ts.UseIME>0)
	{
		if (ts.IMEInline>0)
			SetConversionLogFont(&VTlf);
		else
			SetConversionWindow(HVTWin,-1,0);
	}
  }
  else
    FreeIME();

  if (IsCaretOn()) CaretOn();
}

void ChangeCaret()
{
  UINT T;

  if (! Active) return;
  if (CaretEnabled)
  {
	DestroyCaret();
	switch (ts.CursorShape) {
	  case IdVCur:
		CreateCaret(HVTWin, 0, CurWidth, FontHeight);
		break;
	  case IdHCur:
		CreateCaret(HVTWin, 0, FontWidth, CurWidth);
		break;
	}
	CaretStatus = 1;
  }
  CaretOn();
  if (CaretEnabled &&
	  (ts.NonblinkingCursor!=0))
  {
    T = GetCaretBlinkTime() * 2 / 3;
    SetTimer(HVTWin,IdCaretTimer,T,NULL);
  }
}

void CaretOn()
// Turn on the cursor
{
  int CaretX, CaretY, H;

  if (! Active) return;

  CaretX = (CursorX-WinOrgX)*FontWidth;
  CaretY = (CursorY-WinOrgY)*FontHeight;

  if ((ts.Language==IdJapanese) &&
      CanUseIME() && (ts.IMEInline>0))
    /* set IME conversion window pos. & font */
    SetConversionWindow(HVTWin,CaretX,CaretY);

  if (! CaretEnabled) return;

  if (ts.CursorShape!=IdVCur)
  {
    if (ts.CursorShape==IdHCur)
    {
     CaretY = CaretY+FontHeight-CurWidth;
     H = CurWidth;
    }
    else H = FontHeight;

    DestroyCaret();
    if (CursorOnDBCS)
      CreateCaret(HVTWin, 0, FontWidth*2, H); /* double width caret */
    else
      CreateCaret(HVTWin, 0, FontWidth, H); /* single width caret */
    CaretStatus = 1;
  }

  SetCaretPos(CaretX,CaretY);

  while (CaretStatus > 0)
  {
    ShowCaret(HVTWin);
    CaretStatus--;
  }

}

void CaretOff()
{
  if (! Active) return;
  if (CaretStatus == 0)
  {
    HideCaret(HVTWin);
    CaretStatus++;
  }
}

void DispDestroyCaret()
{
  DestroyCaret();
  if (ts.NonblinkingCursor!=0)
	KillTimer(HVTWin,IdCaretTimer);
}

BOOL IsCaretOn()
// check if caret is on
{
  return (Active && (CaretStatus==0));
}

void DispEnableCaret(BOOL On)
{
  if (! On) CaretOff();
  CaretEnabled = On;
}

BOOL IsCaretEnabled()
{
  return CaretEnabled;
}

void DispSetCaretWidth(BOOL DW)
{
  /* TRUE if cursor is on a DBCS character */
  CursorOnDBCS = DW;
}

void DispChangeWinSize(int Nx, int Ny)
{
  LONG W,H,dW,dH;
  RECT R;

  if (SaveWinSize)
  {
    WinWidthOld = WinWidth;
    WinHeightOld = WinHeight;
    SaveWinSize = FALSE;
  }
  else {
    WinWidthOld = NumOfColumns;
    WinHeightOld = NumOfLines;
  }

  WinWidth = Nx;
  WinHeight = Ny;

  ScreenWidth = WinWidth*FontWidth;
  ScreenHeight = WinHeight*FontHeight;

  AdjustScrollBar();

  GetWindowRect(HVTWin,&R);
  W = R.right-R.left;
  H = R.bottom-R.top;
  GetClientRect(HVTWin,&R);
  dW = ScreenWidth - R.right + R.left;
  dH = ScreenHeight - R.bottom + R.top;
  
  if ((dW!=0) || (dH!=0))
  {
    AdjustSize = TRUE;
    SetWindowPos(HVTWin,HWND_TOP,0,0,W+dW,H+dH,SWP_NOMOVE);
  }
  else
    InvalidateRect(HVTWin,NULL,FALSE);
}

void ResizeWindow(int x, int y, int w, int h, int cw, int ch)
{
  int dw,dh, NewX, NewY;
  POINT Point;

  if (! AdjustSize) return;
  dw = ScreenWidth - cw;
  dh = ScreenHeight - ch;
  if ((dw!=0) || (dh!=0))
    SetWindowPos(HVTWin,HWND_TOP,x,y,w+dw,h+dh,SWP_NOMOVE);
  else {
    AdjustSize = FALSE;

    NewX = x;
    NewY = y;
    if (x+w > VirtualScreen.right)
    {
      NewX = VirtualScreen.right-w;
      if (NewX < 0) NewX = 0;
    }
    if (y+h > VirtualScreen.bottom)
    {
      NewY =  VirtualScreen.bottom-h;
      if (NewY < 0) NewY = 0;
    }
    if ((NewX!=x) || (NewY!=y))
      SetWindowPos(HVTWin,HWND_TOP,NewX,NewY,w,h,SWP_NOSIZE);

    Point.x = 0;
    Point.y = ScreenHeight;
    ClientToScreen(HVTWin,&Point);
    CompletelyVisible = (Point.y <= VirtualScreen.bottom);
    if (IsCaretOn()) CaretOn();
  }
}

void PaintWindow(HDC PaintDC, RECT PaintRect, BOOL fBkGnd,
		 int* Xs, int* Ys, int* Xe, int* Ye)
//  Paint window with background color &
//  convert paint region from window coord. to screen coord.
//  Called from WM_PAINT handler
//    PaintRect: Paint region in window coordinate
//    Return:
//	*Xs, *Ys: upper left corner of the region
//		    in screen coord.
//	*Xe, *Ye: lower right
{
  if (VTDC!=NULL)
	DispReleaseDC();
  VTDC = PaintDC;
  DCPrevFont = SelectObject(VTDC, VTFont[0]);
  DispInitDC();

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//if (fBkGnd)
  if(!BGEnable && fBkGnd)
//-->
#else
  if (fBkGnd)
#endif  // ALPHABLEND_TYPE2

    FillRect(VTDC, &PaintRect,Background);

  *Xs = PaintRect.left / FontWidth + WinOrgX;
  *Ys = PaintRect.top / FontHeight + WinOrgY;
  *Xe = (PaintRect.right-1) / FontWidth + WinOrgX;
  *Ye = (PaintRect.bottom-1) / FontHeight + WinOrgY;
}

void DispEndPaint()
{
  if (VTDC==NULL) return;
  SelectObject(VTDC,DCPrevFont);
  VTDC = NULL;
}

void DispClearWin()
{
  InvalidateRect(HVTWin,NULL,FALSE);

  ScrollCount = 0;
  dScroll = 0;
  if (WinHeight > NumOfLines)
    DispChangeWinSize(NumOfColumns,NumOfLines);
  else {
    if ((NumOfLines==WinHeight) && (ts.EnableScrollBuff>0))
    {
      SetScrollRange(HVTWin,SB_VERT,0,1,FALSE);
    }
    else 
      SetScrollRange(HVTWin,SB_VERT,0,NumOfLines-WinHeight,FALSE);

    SetScrollPos(HVTWin,SB_HORZ,0,TRUE);
    SetScrollPos(HVTWin,SB_VERT,0,TRUE);
  }
  if (IsCaretOn()) CaretOn();
}

void DispChangeBackground()
{
  DispReleaseDC();
  if (Background != NULL) DeleteObject(Background);
#ifdef ALPHABLEND_TYPE2
  Background = CreateSolidBrush(BGVTColor[1]);
#else
  Background = CreateSolidBrush(ts.VTColor[1]);
#endif  // ALPHABLEND_TYPE2


  InvalidateRect(HVTWin,NULL,TRUE);
}

void DispChangeWin()
{
  /* Change window caption */
  ChangeTitle();

  /* Menu bar / Popup menu */
  SwitchMenu();

  SwitchTitleBar();

  /* Change caret shape */
  ChangeCaret();

  if ((ts.ColorFlag & CF_USETEXTCOLOR)==0)
  {
#ifndef NO_ANSI_COLOR_EXTENSION
    ANSIColor[IdFore ]   = ts.ANSIColor[IdFore ];
    ANSIColor[IdBack ]   = ts.ANSIColor[IdBack ];
#else /* NO_ANSI_COLOR_EXTENSION */
    ANSIColor[IdFore ]   = RGB(255,255,255);
    ANSIColor[IdBack ]   = RGB(  0,  0,  0);
#endif /* NO_ANSI_COLOR_EXTENSION */
  }
  else { // use text (background) color for "white (black)"
    ANSIColor[IdFore ]   = ts.VTColor[0];
    ANSIColor[IdBack ]   = ts.VTColor[1];

#ifdef ALPHABLEND_TYPE2
      ANSIColor[IdFore ]   = BGVTColor[0];
     ANSIColor[IdBack ]   = BGVTColor[1];
#endif  // ALPHABLEND_TYPE2

  }

  /* change background color */
  DispChangeBackground();
}

void DispInitDC()
{
  if (VTDC==NULL)
  {
    VTDC = GetDC(HVTWin);
    DCPrevFont = SelectObject(VTDC, VTFont[0]);
  }
  else
    SelectObject(VTDC, VTFont[0]);

#ifdef ALPHABLEND_TYPE2
  SetTextColor(VTDC, BGVTColor[0]);
  SetBkColor(VTDC, BGVTColor[1]);
#else
  SetTextColor(VTDC, ts.VTColor[0]);
  SetBkColor(VTDC, ts.VTColor[1]);
#endif  // ALPHABLEND_TYPE2

  SetBkMode(VTDC,OPAQUE);
  DCAttr = AttrDefault;
  DCAttr2 = AttrDefault2;
  DCReverse = FALSE;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
  BGReverseText = FALSE;
//-->
#endif  // ALPHABLEND_TYPE2
}

void DispReleaseDC()
{
  if (VTDC==NULL) return;
  SelectObject(VTDC, DCPrevFont);
  ReleaseDC(HVTWin,VTDC);
  VTDC = NULL;
}

void DispSetupDC(BYTE Attr, BYTE Attr2, BOOL Reverse)
// Setup device context
//   Attr, Attr2: character attribute 1 & 2
//   Reverse: true if text is selected (reversed) by mouse
{
  COLORREF TextColor, BackColor;
  int i, j;

  if (VTDC==NULL)  DispInitDC();

  if ((DCAttr==Attr) && (DCAttr2==Attr2) &&
      (DCReverse==Reverse)) return;
  DCAttr = Attr;
  DCAttr2 = Attr2;
  DCReverse = Reverse;
     
  SelectObject(VTDC, VTFont[Attr & AttrFontMask]);

  if ((ts.ColorFlag & CF_FULLCOLOR) == 0)
  {
	if ((Attr & AttrBlink) != 0)
	{

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  TextColor = ts.VTBlinkColor[0];
//  BackColor = ts.VTBlinkColor[1];
	  TextColor = BGVTBlinkColor[0];
	  BackColor = BGVTBlinkColor[1];
//-->
#else
  TextColor = ts.VTBlinkColor[0];
  BackColor = ts.VTBlinkColor[1];
#endif
	}
	else if ((Attr & AttrBold) != 0)
	{
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  TextColor = ts.VTBoldColor[0];
//  BackColor = ts.VTBoldColor[1];
	  TextColor = BGVTBoldColor[0];
	  BackColor = BGVTBoldColor[1];
//-->
#else
  TextColor = ts.VTBoldColor[0];
  BackColor = ts.VTBoldColor[1];
#endif
	}
    /* begin - ishizaki */
	else if ((Attr & AttrURL) != 0)
	{
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  TextColor = ts.VTBoldColor[0];
//  BackColor = ts.VTBoldColor[1];
	  TextColor = BGURLColor[0];
	  BackColor = BGURLColor[1];
//-->
#else
  TextColor = ts.URLColor[0];
  BackColor = ts.URLColor[1];
#endif
	}
    /* end - ishizaki */
	else {
	  if ((Attr2 & Attr2Fore) != 0)
	  {
		j = Attr2 & Attr2ForeMask;
		TextColor = ANSIColor[j];
	  }
	  else
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  TextColor = ts.VTColor[0];
    TextColor = BGVTColor[0];
//-->
#else
  TextColor = ts.VTColor[0];
#endif

	  if ((Attr2 & Attr2Back) != 0)
	  {
		j = (Attr2 & Attr2BackMask) >> SftAttrBack;
		BackColor = ANSIColor[j];
	  }
	  else
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  BackColor = ts.VTColor[1];
    BackColor = BGVTColor[1];
//-->
#else
  BackColor = ts.VTColor[1];
#endif
	}
  }
  else { // full color
	if ((Attr2 & Attr2Fore) != 0)
	{
	  if ((Attr & AttrBold) != 0)
		i = 0;
	  else
		i = 8;
	  j = Attr2 & Attr2ForeMask;
	  if (j==0)
		j = 8 - i + j;
	  else
		j = i + j;
	  TextColor = ANSIColor[j];
	}
	else if ((Attr & AttrBlink) != 0)
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  TextColor = ts.VTBlinkColor[0];
    TextColor = BGVTBlinkColor[0];
//-->
	else if ((Attr & AttrBold) != 0)
//<!--by AKASI
//  TextColor = ts.VTBoldColor[0];
    TextColor = BGVTBoldColor[0];
//-->
    /* begin - ishizaki */
	else if ((Attr & AttrURL) != 0)
    TextColor = BGURLColor[0];
    /* end - ishizaki */
  else
//<!--by AKASI
//  TextColor = ts.VTColor[0];
    TextColor = BGVTColor[0];
//-->
#else
  TextColor = ts.VTBlinkColor[0];
	else if ((Attr & AttrBold) != 0)
  TextColor = ts.VTBoldColor[0];
    /* begin - ishizaki */
	else if ((Attr & AttrURL) != 0)
  TextColor = ts.URLColor[0];
    /* end - ishizaki */
  else
  TextColor = ts.VTColor[0];
#endif

	if ((Attr2 & Attr2Back) != 0)
	{
	  if ((Attr & AttrBlink) != 0)
		i = 0;
	  else
		i = 8;
	  j = (Attr2 & Attr2BackMask) >> SftAttrBack;
	  if (j==0)
		j = 8 - i + j;
	  else
		j = i + j;
	  BackColor = ANSIColor[j];
	}
	else if ((Attr & AttrBlink) != 0)
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  BackColor = ts.VTBlinkColor[1];
    BackColor = BGVTBlinkColor[1];
//-->
	else if ((Attr & AttrBold) != 0)
//<!--by AKASI
//  BackColor = ts.VTBoldColor[1];
    BackColor = BGVTBoldColor[1];
//-->
    /* begin - ishizaki */
	else if ((Attr & AttrURL) != 0)
    BackColor = BGURLColor[1];
    /* end - ishizaki */
  else
  BackColor = ts.VTColor[1];
#else
  BackColor = ts.VTBlinkColor[1];
	else if ((Attr & AttrBold) != 0)
  BackColor = ts.VTBoldColor[1];
    /* begin - ishizaki */
	else if ((Attr & AttrURL) != 0)
  BackColor = ts.URLColor[1];
    /* end - ishizaki */
  else
  BackColor = ts.VTColor[1];
#endif
  }

  if (Reverse != ((Attr & AttrReverse) != 0))
  {
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
    BGReverseText = TRUE;
//-->
#endif
    SetTextColor(VTDC,BackColor);
    SetBkColor(  VTDC,TextColor);
  }
  else {
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
    BGReverseText = FALSE;
//-->
#endif
    SetTextColor(VTDC,TextColor);
    SetBkColor(  VTDC,BackColor);
  }
}

#if 1
// 当面はこちらの関数を使う。(2004.11.4 yutaka)
void DispStr(PCHAR Buff, int Count, int Y, int* X)
// Display a string
//   Buff: points the string
//   Y: vertical position in window cordinate
//  *X: horizontal position
// Return:
//  *X: horizontal position shifted by the width of the string
{
  RECT RText;

  if ((ts.Language==IdRussian) &&
      (ts.RussClient!=ts.RussFont))
    RussConvStr(ts.RussClient,ts.RussFont,Buff,Count);

  RText.top = Y;
  RText.bottom = Y+FontHeight;
  RText.left = *X;
  RText.right = *X + Count*FontWidth;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
  if(!BGEnable)
  {
    ExtTextOut(VTDC,*X+ts.FontDX,Y+ts.FontDY,
               ETO_CLIPPED | ETO_OPAQUE,
               &RText,Buff,Count,&Dx[0]);
  }else{

    int   width;
    int   height;
    int   eto_options = ETO_CLIPPED;
    RECT  rect;
    HFONT hPrevFont;

    width  = Count*FontWidth;
    height = FontHeight;
    SetRect(&rect,0,0,width,height);

    //hdcBGBuffer の属性を設定
    hPrevFont = SelectObject(hdcBGBuffer,GetCurrentObject(VTDC,OBJ_FONT));
    SetTextColor(hdcBGBuffer,GetTextColor(VTDC));
    SetBkColor(hdcBGBuffer,GetBkColor(VTDC));

    //窓の移動、リサイズ中は背景を BGBrushInSizeMove で塗りつぶす
    if(BGInSizeMove)
      FillRect(hdcBGBuffer,&rect,BGBrushInSizeMove);

    BitBlt(hdcBGBuffer,0,0,width,height,hdcBG,*X,Y,SRCCOPY);

    if(BGReverseText == TRUE)
    {
      if(BGReverseTextAlpha < 255)
      {
        BGBLENDFUNCTION bf;
        HBRUSH hbr;

        hbr = CreateSolidBrush(GetBkColor(hdcBGBuffer));
        FillRect(hdcBGWork,&rect,hbr);
        DeleteObject(hbr);

        ZeroMemory(&bf,sizeof(bf));
        bf.BlendOp             = AC_SRC_OVER;
        bf.SourceConstantAlpha = BGReverseTextAlpha;

        BGAlphaBlend(hdcBGBuffer,0,0,width,height,hdcBGWork,0,0,width,height,bf);
      }else{
        eto_options |= ETO_OPAQUE;
      }
    }
    
    ExtTextOut(hdcBGBuffer,ts.FontDX,ts.FontDY,eto_options,&rect,Buff,Count,&Dx[0]);
    BitBlt(VTDC,*X,Y,width,height,hdcBGBuffer,0,0,SRCCOPY);

    SelectObject(hdcBGBuffer,hPrevFont);
  }
//-->
#else
  ExtTextOut(VTDC,*X+ts.FontDX,Y+ts.FontDY,
             ETO_CLIPPED | ETO_OPAQUE,
             &RText,Buff,Count,&Dx[0]);
#endif
  *X = RText.right;

  if ((ts.Language==IdRussian) &&
      (ts.RussClient!=ts.RussFont))
    RussConvStr(ts.RussFont,ts.RussClient,Buff,Count);
}

#else
void DispStr(PCHAR Buff, int Count, int Y, int* X)
// Display a string
//   Buff: points the string
//   Y: vertical position in window cordinate
//  *X: horizontal position
// Return:
//  *X: horizontal position shifted by the width of the string
{
	RECT RText;
	wchar_t *wc;
	int len, wclen;
	CHAR ch;

#if 0
#include <crtdbg.h>
	_CrtSetBreakAlloc(52);
	Buff[0] = 0x82;
	Buff[1] = 0xe4;
	Buff[2] = 0x82;
	Buff[3] = 0xbd;
	Buff[4] = 0x82;
	Buff[5] = 0xa9;
	Count = 6;
#endif

	setlocale(LC_ALL, ts.Locale);

	ch = Buff[Count];
	Buff[Count] = 0;
	len = mbstowcs(NULL, Buff, 0);

	wc = malloc(sizeof(wchar_t) * (len + 1));
	if (wc == NULL)
		return;
	wclen = mbstowcs(wc, Buff, len + 1);
	Buff[Count] = ch;

	if ((ts.Language==IdRussian) &&
		(ts.RussClient!=ts.RussFont))
		RussConvStr(ts.RussClient,ts.RussFont,Buff,Count);

	RText.top = Y;
	RText.bottom = Y+FontHeight;
	RText.left = *X;
	RText.right = *X + Count*FontWidth; // 

	// Unicodeで出力する。
#if 1
	// UTF-8環境において、tcshがEUC出力した場合、画面に何も表示されないことがある。
	// マウスでドラッグしたり、ログファイルへ保存してみると、文字化けした文字列を
	// 確認することができる。(2004.8.6 yutaka)
	ExtTextOutW(VTDC,*X+ts.FontDX,Y+ts.FontDY,
		ETO_CLIPPED | ETO_OPAQUE,
		&RText, wc, wclen, NULL);
//		&RText, wc, wclen, &Dx[0]);
#else
	TextOutW(VTDC, *X+ts.FontDX, Y+ts.FontDY, wc, wclen);

#endif

	*X = RText.right;

	if ((ts.Language==IdRussian) &&
		(ts.RussClient!=ts.RussFont))
		RussConvStr(ts.RussFont,ts.RussClient,Buff,Count);

	free(wc);
}
#endif


void DispEraseCurToEnd(int YEnd)
{
  RECT R;

  if (VTDC==NULL) DispInitDC();
  R.left = 0;
  R.right = ScreenWidth;
  R.top = (CursorY+1-WinOrgY)*FontHeight;
  R.bottom = (YEnd+1-WinOrgY)*FontHeight;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  FillRect(VTDC,&R,Background);
  BGFillRect(VTDC,&R,Background);
//-->
#else
  FillRect(VTDC,&R,Background);
#endif

  R.left = (CursorX-WinOrgX)*FontWidth;
  R.bottom = R.top;
  R.top = R.bottom-FontHeight;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  FillRect(VTDC,&R,Background);
  BGFillRect(VTDC,&R,Background);
//-->
#else
  FillRect(VTDC,&R,Background);
#endif
}

void DispEraseHomeToCur(int YHome)
{
  RECT R;

  if (VTDC==NULL) DispInitDC();
  R.left = 0;
  R.right = ScreenWidth;
  R.top = (YHome-WinOrgY)*FontHeight;
  R.bottom = (CursorY-WinOrgY)*FontHeight;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  FillRect(VTDC,&R,Background);
  BGFillRect(VTDC,&R,Background);
//-->
#else
  FillRect(VTDC,&R,Background);
#endif

  R.top = R.bottom;
  R.bottom = R.top + FontHeight;
  R.right = (CursorX+1-WinOrgX)*FontWidth;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  FillRect(VTDC,&R,Background);
  BGFillRect(VTDC,&R,Background);
//-->
#else
  FillRect(VTDC,&R,Background);
#endif
}

void DispEraseCharsInLine(int XStart, int Count)
{
  RECT R;

  if (VTDC==NULL) DispInitDC();
  R.top = (CursorY-WinOrgY)*FontHeight;
  R.bottom = R.top+FontHeight;
  R.left = (XStart-WinOrgX)*FontWidth;
  R.right = R.left + Count * FontWidth;

#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  FillRect(VTDC,&R,Background);
  BGFillRect(VTDC,&R,Background);
//-->
#else
  FillRect(VTDC,&R,Background);
#endif
}

BOOL DispDeleteLines(int Count, int YEnd)
// return value:
//	 TRUE  - screen is successfully updated
//   FALSE - screen is not updated
{
  RECT R;

  if (Active && CompletelyVisible &&
      (YEnd+1-WinOrgY <= WinHeight))
  {
	R.left = 0;
	R.right = ScreenWidth;
	R.top = (CursorY-WinOrgY)*FontHeight;
	R.bottom = (YEnd+1-WinOrgY)*FontHeight;
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  ScrollWindow(HVTWin,0,-FontHeight*Count,&R,&R);
  BGScrollWindow(HVTWin,0,-FontHeight*Count,&R,&R);
//-->
#else
  ScrollWindow(HVTWin,0,-FontHeight*Count,&R,&R);
#endif
	UpdateWindow(HVTWin);
	return TRUE;
  }
  else
	return FALSE;
}

BOOL DispInsertLines(int Count, int YEnd)
// return value:
//	 TRUE  - screen is successfully updated
//   FALSE - screen is not updated
{
  RECT R;

  if (Active && CompletelyVisible &&
      (CursorY >= WinOrgY))
  {
    R.left = 0;
    R.right = ScreenWidth;
    R.top = (CursorY-WinOrgY)*FontHeight;
    R.bottom = (YEnd+1-WinOrgY)*FontHeight;
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  ScrollWindow(HVTWin,0,FontHeight*Count,&R,&R);
    BGScrollWindow(HVTWin,0,FontHeight*Count,&R,&R);
//-->
#else
  ScrollWindow(HVTWin,0,FontHeight*Count,&R,&R);
#endif
	UpdateWindow(HVTWin);
    return TRUE;
  }
  else
	return FALSE;
}

BOOL IsLineVisible(int* X, int* Y)
//  Check the visibility of a line
//	called from UpdateStr()
//    *X, *Y: position of a character in the line. screen coord.
//    Return: TRUE if the line is visible.
//	*X, *Y:
//	  If the line is visible
//	    position of the character in window coord.
//	  Otherwise
//	    no change. same as input value.
{
  if ((dScroll != 0) &&
      (*Y>=SRegionTop) &&
      (*Y<=SRegionBottom))
  {
    *Y = *Y + dScroll;
    if ((*Y<SRegionTop) || (*Y>SRegionBottom))
      return FALSE;
  }

  if ((*Y<WinOrgY) ||
      (*Y>=WinOrgY+WinHeight))
    return FALSE;

  /* screen coordinate -> window coordinate */
  *X = (*X-WinOrgX)*FontWidth;
  *Y = (*Y-WinOrgY)*FontHeight;
  return TRUE;
}

//-------------- scrolling functions --------------------

void AdjustScrollBar() /* called by ChangeWindowSize() */
{
  LONG XRange, YRange;
  int ScrollPosX, ScrollPosY;

  if (NumOfColumns-WinWidth>0)
    XRange = NumOfColumns-WinWidth;
  else
    XRange = 0;

  if (BuffEnd-WinHeight>0)
    YRange = BuffEnd-WinHeight;
  else
    YRange = 0;

  ScrollPosX = GetScrollPos(HVTWin,SB_HORZ);
  ScrollPosY = GetScrollPos(HVTWin,SB_VERT);
  if (ScrollPosX > XRange)
    ScrollPosX = XRange;
  if (ScrollPosY > YRange)
    ScrollPosY = YRange;

  WinOrgX = ScrollPosX;
  WinOrgY = ScrollPosY-PageStart;
  NewOrgX = WinOrgX;
  NewOrgY = WinOrgY;

  DontChangeSize = TRUE;

  SetScrollRange(HVTWin,SB_HORZ,0,XRange,FALSE);

  if ((YRange == 0) && (ts.EnableScrollBuff>0))
  {
    SetScrollRange(HVTWin,SB_VERT,0,1,FALSE);
  }
  else {
    SetScrollRange(HVTWin,SB_VERT,0,YRange,FALSE);
  }

  SetScrollPos(HVTWin,SB_HORZ,ScrollPosX,TRUE);
  SetScrollPos(HVTWin,SB_VERT,ScrollPosY,TRUE);

  DontChangeSize = FALSE;  
}

void DispScrollToCursor(int CurX, int CurY)
{
  if (CurX < NewOrgX)
    NewOrgX = CurX;
  else if (CurX >= NewOrgX+WinWidth)
    NewOrgX = CurX + 1 - WinWidth;

  if (CurY < NewOrgY)
    NewOrgY = CurY;
  else if (CurY >= NewOrgY+WinHeight)
    NewOrgY = CurY + 1 - WinHeight;
}

void DispScrollNLines(int Top, int Bottom, int Direction)
//  Scroll a region of the window by Direction lines
//    updates window if necessary
//  Top: top line of scroll region
//  Bottom: bottom line
//  Direction: +: forward, -: backward
{
  if ((dScroll*Direction <0) ||
      (dScroll*Direction >0) &&
      ((SRegionTop!=Top) ||
       (SRegionBottom!=Bottom)))
    DispUpdateScroll();
  SRegionTop = Top;
  SRegionBottom = Bottom;
  dScroll = dScroll + Direction;
  if (Direction>0)
    DispCountScroll(Direction);
  else
    DispCountScroll(-Direction);
}

void DispCountScroll(int n)
{
  ScrollCount = ScrollCount + n;
  if (ScrollCount>=ts.ScrollThreshold) DispUpdateScroll();
}

void DispUpdateScroll()
{
  int d;
  RECT R;

  ScrollCount = 0;

  /* Update partial scroll */
  if (dScroll != 0)
  {
    d = dScroll * FontHeight;
    R.left = 0;
    R.right = ScreenWidth;
    R.top = (SRegionTop-WinOrgY)*FontHeight;
    R.bottom = (SRegionBottom+1-WinOrgY)*FontHeight;
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  ScrollWindow(HVTWin,0,-d,&R,&R);
    BGScrollWindow(HVTWin,0,-d,&R,&R);
//-->
#else
  ScrollWindow(HVTWin,0,-d,&R,&R);
#endif

    if ((SRegionTop==0) && (dScroll>0))
	{ // update scroll bar if BuffEnd is changed
	  if ((BuffEnd==WinHeight) &&
          (ts.EnableScrollBuff>0))
        SetScrollRange(HVTWin,SB_VERT,0,1,TRUE);
      else
        SetScrollRange(HVTWin,SB_VERT,0,BuffEnd-WinHeight,FALSE);
      SetScrollPos(HVTWin,SB_VERT,WinOrgY+PageStart,TRUE);
	}
    dScroll = 0;
  }

  /* Update normal scroll */
  if (NewOrgX < 0) NewOrgX = 0;
  if (NewOrgX>NumOfColumns-WinWidth)
    NewOrgX = NumOfColumns-WinWidth;
  if (NewOrgY < -PageStart) NewOrgY = -PageStart;
  if (NewOrgY>BuffEnd-WinHeight-PageStart)
    NewOrgY = BuffEnd-WinHeight-PageStart;

  if ((NewOrgX==WinOrgX) &&
      (NewOrgY==WinOrgY)) return;

  if (NewOrgX==WinOrgX)
  {
    d = (NewOrgY-WinOrgY) * FontHeight;
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  ScrollWindow(HVTWin,0,-d,NULL,NULL);
    BGScrollWindow(HVTWin,0,-d,NULL,NULL);
//-->
#else
  ScrollWindow(HVTWin,0,-d,NULL,NULL);
#endif
  }
  else if (NewOrgY==WinOrgY)
  {
    d = (NewOrgX-WinOrgX) * FontWidth;
#ifdef ALPHABLEND_TYPE2
//<!--by AKASI
//  ScrollWindow(HVTWin,-d,0,NULL,NULL);
    BGScrollWindow(HVTWin,-d,0,NULL,NULL);
//-->
#else
  ScrollWindow(HVTWin,-d,0,NULL,NULL);
#endif
  }
  else
    InvalidateRect(HVTWin,NULL,TRUE);

  /* Update scroll bars */
  if (NewOrgX!=WinOrgX)
    SetScrollPos(HVTWin,SB_HORZ,NewOrgX,TRUE);

  if (NewOrgY!=WinOrgY)
  {
    if ((BuffEnd==WinHeight) &&
        (ts.EnableScrollBuff>0))
      SetScrollRange(HVTWin,SB_VERT,0,1,TRUE);
    else
      SetScrollRange(HVTWin,SB_VERT,0,BuffEnd-WinHeight,FALSE);
    SetScrollPos(HVTWin,SB_VERT,NewOrgY+PageStart,TRUE);
  }

  WinOrgX = NewOrgX;
  WinOrgY = NewOrgY;

  if (IsCaretOn()) CaretOn();
}

void DispScrollHomePos()
{
  NewOrgX = 0;
  NewOrgY = 0;
  DispUpdateScroll();
}

void DispAutoScroll(POINT p)
{
  int X, Y;

  X = (p.x + FontWidth / 2) / FontWidth;
  Y = p.y / FontHeight;
  if (X<0)
    NewOrgX = WinOrgX + X;
  else if (X>=WinWidth)
    NewOrgX = NewOrgX + X - WinWidth + 1;
  if (Y<0)
    NewOrgY = WinOrgY + Y;
  else if (Y>=WinHeight)
    NewOrgY = NewOrgY + Y - WinHeight + 1;

  DispUpdateScroll();
}

void DispHScroll(int Func, int Pos)
{
  switch (Func) {
	case SCROLL_BOTTOM:
      NewOrgX = NumOfColumns-WinWidth;
      break;
	case SCROLL_LINEDOWN: NewOrgX = WinOrgX + 1; break;
	case SCROLL_LINEUP: NewOrgX = WinOrgX - 1; break;
	case SCROLL_PAGEDOWN:
      NewOrgX = WinOrgX + WinWidth - 1;
      break;
	case SCROLL_PAGEUP:
      NewOrgX = WinOrgX - WinWidth + 1;
      break;
	case SCROLL_POS: NewOrgX = Pos; break;
	case SCROLL_TOP: NewOrgX = 0; break;
  }
  DispUpdateScroll();
}

void DispVScroll(int Func, int Pos)
{
  switch (Func) {
	case SCROLL_BOTTOM:
      NewOrgY = BuffEnd-WinHeight-PageStart;
      break;
	case SCROLL_LINEDOWN: NewOrgY = WinOrgY + 1; break;
	case SCROLL_LINEUP: NewOrgY = WinOrgY - 1; break;
	case SCROLL_PAGEDOWN:
      NewOrgY = WinOrgY + WinHeight - 1;
      break;
	case SCROLL_PAGEUP:
      NewOrgY = WinOrgY - WinHeight + 1;
      break;
	case SCROLL_POS: NewOrgY = Pos-PageStart; break;
	case SCROLL_TOP: NewOrgY = -PageStart; break;
  }
  DispUpdateScroll();
}

//-------------- end of scrolling functions --------

void DispSetupFontDlg()
//  Popup the Setup Font dialogbox and
//  reset window
{
  BOOL Ok;

  if (! LoadTTDLG()) return;
  SetLogFont();
  Ok = ChooseFontDlg(HVTWin,&VTlf,&ts);
  FreeTTDLG();
  if (! Ok) return;

  strcpy(ts.VTFont,VTlf.lfFaceName);
  ts.VTFontSize.x = VTlf.lfWidth;
  ts.VTFontSize.y = VTlf.lfHeight;
  ts.VTFontCharSet = VTlf.lfCharSet;

  ChangeFont();

  DispChangeWinSize(WinWidth,WinHeight);

  ChangeCaret();
}

void DispRestoreWinSize()
//  Restore window size by double clik on caption bar
{
  if (ts.TermIsWin>0) return;

  if ((WinWidth==NumOfColumns) && (WinHeight==NumOfLines))
  {
    if (WinWidthOld > NumOfColumns)
      WinWidthOld = NumOfColumns;
    if (WinHeightOld > BuffEnd)
      WinHeightOld = BuffEnd;
    DispChangeWinSize(WinWidthOld,WinHeightOld);
  }
  else {
    SaveWinSize = TRUE;
    DispChangeWinSize(NumOfColumns,NumOfLines);
  }
}

void DispSetWinPos()
{
  int CaretX, CaretY;
  POINT Point;
  RECT R;

  GetWindowRect(HVTWin,&R);
  ts.VTPos.x = R.left;
  ts.VTPos.y = R.top;

  if (CanUseIME() && (ts.IMEInline > 0))
  {
    CaretX = (CursorX-WinOrgX)*FontWidth;
    CaretY = (CursorY-WinOrgY)*FontHeight;
    /* set IME conversion window pos. */
    SetConversionWindow(HVTWin,CaretX,CaretY);
  }

  Point.x = 0;
  Point.y = ScreenHeight;
  ClientToScreen(HVTWin,&Point);
  CompletelyVisible = (Point.y <= VirtualScreen.bottom);

#ifdef ALPHABLEND_TYPE2
   if(BGEnable)
   	InvalidateRect(HVTWin,0,FALSE);
#endif
}

void DispSetActive(BOOL ActiveFlag)
{
  Active = ActiveFlag;
  if (Active)
  {
    SetFocus(HVTWin);
    ActiveWin = IdVT;
  }
  else {
    if ((ts.Language==IdJapanese) &&
        CanUseIME())
      /* position & font of conv. window -> default */
      SetConversionWindow(HVTWin,-1,0);
  }
}

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.4  2005/02/03 14:46:23  yutakakn
 * CVSログIDの追加。
 *
 *
 * AKASI氏によるEterm風透過ウィンドウ機能を追加。
 * VTColorの初期値は、teraterm.iniのANSI Colorを優先させた。
 *
 * マルチディスプレイ環境において、ウィンドウのリサイズを行うとプライマリディスプレイへ
 * 戻ってしまう現象への対処。
 * ＃パッチを送っていただいた安藤氏に感謝
 */

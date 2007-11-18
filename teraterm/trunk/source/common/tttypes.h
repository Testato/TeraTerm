/* Tera Term
 Copyright(C) 1994-1998 T. Teranishi
 All rights reserved. */

/* Constants and types for Tera Term */

#define IdBreakTimer         1
#define IdDelayTimer         2
#define IdProtoTimer         3
#define IdDblClkTimer        4
#define IdScrollTimer        5
#define IdComEndTimer        6
#define IdCaretTimer         7
#define IdPrnStartTimer      8
#define IdPrnProcTimer       9
#define IdCancelConnectTimer 10  // add (2007.1.10 yutaka)

  /* Window Id */
#define IdVT  1
#define IdTEK 2

  /* Talker mode */
#define IdTalkKeyb  0
#define IdTalkCB    1
#define IdTalkFile  2
#define IdTalkQuiet 3

  /* Character sets */
#define IdASCII    0
#define IdKatakana 1
#define IdKanji    2
#define IdSpecial  3

  /* Character attribute bit masks */
#define AttrDefault       0x00
#define AttrDefault2      0x00
#define AttrBold          0x01
#define AttrUnder         0x02
#define AttrSpecial       0x04
#define AttrFontMask      0x07
#define AttrBlink         0x08
#define AttrReverse       0x10
#ifndef NO_COPYLINE_FIX
#define AttrLineContinued 0x20 /* valid only at the beggining or end of a line */
#endif /* NO_COPYLINE_FIX */
/* begin - ishizaki */
#define AttrURL           0x40
/* end - ishizaki */
#define AttrKanji         0x80
  /* Color attribute bit masks */
#define Attr2Fore         0x08
#define Attr2ForeMask     0x07
#define Attr2Back         0x80
#define Attr2BackMask     0x70
#define SftAttrBack       4

  /* Color codes */
#define IdBack    0
#define IdRed     1
#define IdGreen   2
#define IdYellow  3
#define IdBlue    4
#define IdMagenta 5
#define IdCyan    6
#define IdFore    7

  /* Kermit function id */
#define IdKmtReceive 1
#define IdKmtGet     2
#define IdKmtSend    3
#define IdKmtFinish  4

  /* XMODEM function id */
#define IdXReceive 1
#define IdXSend    2

  /* ZMODEM function id */
#define IdZReceive 1
#define IdZSend    2
#define IdZAuto    3

  /* B-Plus function id */
#define IdBPReceive 1
#define IdBPSend    2
#define IdBPAuto    3

  /* Quick-VAN function id */
#define IdQVReceive 1
#define IdQVSend    2

#define HostNameMaxLength 1024
//#define HostNameMaxLength 80
#ifndef NO_INET6
#define ProtocolFamilyMaxLength 80
#endif /* NO_INET6 */

  /* internal WM_USER messages */
#define WM_USER_ACCELCOMMAND WM_USER+1
#define WM_USER_CHANGEMENU   WM_USER+2
#define WM_USER_CLOSEIME     WM_USER+3
#define WM_USER_COMMNOTIFY   WM_USER+4
#define WM_USER_COMMOPEN     WM_USER+5
#define WM_USER_COMMSTART    WM_USER+6
#define WM_USER_DLGHELP2     WM_USER+7
#define WM_USER_GETHOST      WM_USER+8
#define WM_USER_FTCANCEL     WM_USER+9
#define WM_USER_PROTOCANCEL  WM_USER+10
#define WM_USER_CHANGETBAR   WM_USER+11
#define WM_USER_KEYCODE      WM_USER+12
#define WM_USER_GETSERIALNO  WM_USER+13
#define WM_USER_CHANGETITLE  WM_USER+14

#define WM_USER_DDEREADY     WM_USER+21
#define WM_USER_DDECMNDEND   WM_USER+22
#define WM_USER_DDECOMREADY  WM_USER+23
#define WM_USER_DDEEND       WM_USER+24

  /* port type ID */
#define IdTCPIP  1
#define IdSerial 2
#define IdFile   3

  /* XMODEM option */
#define XoptCheck 1
#define XoptCRC   2
#define Xopt1K    3

  /* Language */
#define IdEnglish  1
#define IdJapanese 2
#define IdRussian  3

// log flags (used in ts.LogFlag) 
#define LOG_TEL 1
#define LOG_KMT 2
#define LOG_X   4
#define LOG_Z   8
#define LOG_BP  16
#define LOG_QV  32

// file transfer flags (used in ts.FTFlag)
#define FT_ZESCCTL  1
#define FT_ZAUTO    2
#define FT_BPESCCTL 4
#define FT_BPAUTO   8
#define FT_RENAME   16

// menu flags (used in ts.MenuFlag)
#define MF_NOSHOWMENU   1
#define MF_NOPOPUP      2
#define MF_NOLANGUAGE   4
#define MF_SHOWWINMENU  8

// Terminal flags (used in ts.TermFlag)
#define TF_FIXEDJIS           1
#define TF_AUTOINVOKE         2
#define TF_CTRLINKANJI        8
#define TF_ALLOWWRONGSEQUENCE 16
#define TF_ACCEPT8BITCTRL     32
#define TF_ENABLESLINE        64
#define TF_BACKWRAP           128

// ANSI color flags (used in ts.ColorFlag)
#define CF_FULLCOLOR    1
#define CF_USETEXTCOLOR 2

// port flags (used in ts.PortFlag)
#define PF_CONFIRMDISCONN 1
#define PF_BEEPONCONNECT  2

// Eterm lookfeel alphablend structure
typedef struct {
	int BGEnable;
	int BGUseAlphaBlendAPI;
	char BGSPIPath[MAX_PATH];
	int BGFastSizeMove;
	int BGNoCopyBits;
	int BGNoFrame;
	char BGThemeFile[MAX_PATH];
} eterm_lookfeel_t;

/* TTTSet */
//
// NOTE: â∫ãLÇÃÉGÉâÅ[Ç™Ç≈ÇÈÇ±Ç∆Ç™Ç†ÇÈ
//   fatal error C1001: INTERNAL COMPILER ERROR (compiler file 'msc1.cpp', line 2701)
// 
struct tttset {
/*------ VTSet --------*/
	/* Tera Term home directory */
	char HomeDir[MAXPATHLEN];

	/* Setup file name */
	char SetupFName[MAXPATHLEN];
	char KeyCnfFN[MAXPATHLEN];
	char LogFN[MAXPATHLEN];
	char MacroFN[MAXPATHLEN];
	char HostName[1024];

	POINT VTPos;
	char VTFont[LF_FACESIZE];
	POINT VTFontSize;
	int VTFontCharSet;
	int FontDW, FontDH, FontDX, FontDY;
	char PrnFont[LF_FACESIZE];
	POINT PrnFontSize;
	int PrnFontCharSet;
	POINT VTPPI, TEKPPI;
	int PrnMargin[4];
	char PrnDev[80];
	WORD PassThruDelay;
	WORD PrnConvFF;
	WORD EnableBold;
	WORD RussFont;
	int ScrollThreshold;
	WORD Debug;
	WORD LogFlag;
	WORD FTFlag;
	WORD TransBin, Append;
	WORD XmodemOpt, XmodemBin;
	int ZmodemDataLen, ZmodemWinSize;
	int QVWinSize;
	char FileDir[MAXPATHLEN];
	char FileSendFilter[128];
	WORD Language;
	char DelimList[52];
	WORD DelimDBCS;
	WORD Minimize;
	WORD HideWindow;
	WORD MenuFlag;
	WORD SelOnActive;
	WORD AutoTextCopy;
/*------ TEKSet --------*/
	POINT TEKPos;
	char TEKFont[LF_FACESIZE];
	POINT TEKFontSize;
	int TEKFontCharSet;
	int GINMouseCode;
/*------ TermSet --------*/
	int TerminalWidth;
	int TerminalHeight;
	WORD TermIsWin;
	WORD AutoWinResize;
	WORD CRSend;
	WORD CRReceive;
	WORD LocalEcho;
	char Answerback[32];
	int AnswerbackLen;
	WORD KanjiCode;
	WORD KanjiCodeSend;
	WORD JIS7Katakana;
	WORD JIS7KatakanaSend;
	WORD KanjiIn;
	WORD KanjiOut;
	WORD RussHost;
	WORD RussClient;
	WORD RussPrint;
	WORD AutoWinSwitch;
	WORD TerminalID;
	WORD TermFlag;
/*------ WinSet --------*/
	WORD VTFlag;
	HFONT SampleFont;
	/* begin - ishizaki */
	/* WORD TmpColor[3][6]; */
	WORD TmpColor[4][6];
	/* end - ishizaki */
	/* Tera Term window setup variables */
	char Title[50];
	WORD TitleFormat;
	WORD CursorShape;
	WORD NonblinkingCursor;
	WORD EnableScrollBuff;
	LONG ScrollBuffSize;
	LONG ScrollBuffMax;
	WORD HideTitle;
	WORD PopupMenu;
	WORD ColorFlag;
	WORD TEKColorEmu;
	COLORREF VTColor[2];
	COLORREF TEKColor[2];
	/* begin - ishizaki */
	COLORREF URLColor[2];
	/* end   - ishizaki */
	COLORREF VTBoldColor[2];
	COLORREF VTBlinkColor[2];
	WORD Beep;
/*------ KeybSet --------*/
	WORD BSKey;
	WORD DelKey;
	WORD UseIME;
	WORD IMEInline;
	WORD MetaKey;
	WORD RussKeyb;
/*------ PortSet --------*/
	WORD PortType;
	/* TCP/IP */
	WORD TCPPort;
	WORD Telnet;
	WORD TelPort;
	WORD TelBin;
	WORD TelEcho;
	char TermType[40];
	WORD AutoWinClose;
	WORD PortFlag;
	WORD TCPCRSend;
	WORD TCPLocalEcho;
	WORD HistoryList;
	/* Serial */
	WORD ComPort;
	WORD Baud;
	WORD Parity;
	WORD DataBit;
	WORD StopBit;
	WORD Flow;
	WORD DelayPerChar;
	WORD DelayPerLine;
	WORD MaxComPort;
	WORD ComAutoConnect;
#ifndef NO_COPYLINE_FIX
	WORD EnableContinuedLineCopy;
#endif /* NO_COPYLINE_FIX */
#ifndef NO_ANSI_COLOR_EXTENSION
	COLORREF ANSIColor[16];
#endif /* NO_ANSI_COLOR_EXTENSION */
#ifndef NO_INET6
	/* protocol used in connect() */
	int ProtocolFamily;
#endif /* NO_INET6 */
  char MouseCursorName[16];
	int AlphaBlend;
	char CygwinDirectory[MAX_PATH];
#define DEFAULT_LOCALE "japanese"
	char Locale[80];
#define DEFAULT_CODEPAGE 932
	int CodePage;
	int DuplicateSession;
	char ViewlogEditor[MAX_PATH];
	WORD LogTypePlainText;
	WORD LogTimestamp;
	char LogDefaultName[80];
	char LogDefaultPath[MAX_PATH];
	WORD LogAutoStart;
	int DisablePasteMouseRButton;
	WORD ConfirmPasteMouseRButton;
	WORD DisableAcceleratorSendBreak;
	int EnableClickableUrl;
	eterm_lookfeel_t EtermLookfeel;
#ifdef USE_NORMAL_BGCOLOR
	WORD UseNormalBGColor;
#endif
	char UILanguageFile[MAX_PATH];
	char UIMsg[MAX_UIMSG];
	WORD BroadcastCommandHistory;
	WORD AcceptBroadcast;		// 337: 2007/03/20
	WORD DisableTCPEchoCR;  // TCPLocalEcho/TCPCRSend Çñ≥å¯Ç…Ç∑ÇÈ (maya 2007.4.25)
	int ConnectingTimeout;
	WORD VTCompatTab;
	WORD TelKeepAliveInterval;
	WORD MaxBroadcatHistory;
	WORD DisableAppKeypad;
	WORD DisableAppCursor;
	WORD ClearComBuffOnOpen;
	WORD Send8BitCtrl;
	char UILanguageFile_ini[MAX_PATH];
};

typedef struct tttset TTTSet, *PTTSet;
//typedef TTTSet far *PTTSet;

  /* New Line modes */
#define IdCR   1
#define IdCRLF 2
#define IdLF   3

  /* Terminal ID */
#define IdVT100  1
#define IdVT100J 2
#define IdVT101  3
#define IdVT102  4
#define IdVT102J 5
#define IdVT220J 6
#define IdVT282  7
#define IdVT320  8
#define IdVT382  9

  /* Kanji Code ID */
#define IdSJIS  1
#define IdEUC   2
#define IdJIS   3
#define IdUTF8  4
#define IdUTF8m 5

// Russian code sets
#define IdWindows 1
#define IdKOI8    2
#define Id866     3
#define IdISO     4

  /* KanjiIn modes */
#define IdKanjiInA 1
#define IdKanjiInB 2
  /* KanjiOut modes */
#define IdKanjiOutB 1
#define IdKanjiOutJ 2
#define IdKanjiOutH 3

#define TermWidthMax  300
#define TermHeightMax 200

  /* Cursor shapes */
#define IdBlkCur 1
#define IdVCur   2
#define IdHCur   3

#define IdBS  1
#define IdDEL 2

  /* Serial port ID */
#define IdCOM1 1
#define IdCOM2 2
#define IdCOM3 3
#define IdCOM4 4
  /* Baud rate ID */
#define IdBaudNone   0
#define IdBaud110    1
#define IdBaud300    2
#define IdBaud600    3
#define IdBaud1200   4
#define IdBaud2400   5
#define IdBaud4800   6
#define IdBaud9600   7
#define IdBaud14400  8
#define IdBaud19200  9
#define IdBaud38400  10
#define IdBaud57600  11
#define IdBaud115200 12
// expansion (2005.11.30 yutaka)
#define IdBaud230400 13
#define IdBaud460800 14
#define IdBaud921600 15

  /* Parity ID */
#define IdParityEven 1
#define IdParityOdd  2
#define IdParityNone 3
  /* Data bit ID */
#define IdDataBit7 1
#define IdDataBit8 2
  /* Stop bit ID */
#define IdStopBit1 1
#define IdStopBit2 2
  /* Flow control ID */
#define IdFlowX 1
#define IdFlowHard 2
#define IdFlowNone 3

/* GetHostName dialog record */
typedef struct {
	PCHAR SetupFN; // setup file name
	WORD PortType; // TCPIP/Serial
	PCHAR HostName; // host name 
	WORD Telnet; // non-zero: enable telnet
	WORD TelPort; // default TCP port# for telnet
	WORD TCPPort; // TCP port #
#ifndef NO_INET6
	WORD ProtocolFamily; // Protocol Family (AF_INET/AF_INET6/AF_UNSPEC)
#endif /* NO_INET6 */
	WORD ComPort; // serial port #
	WORD MaxComPort; // max serial port #
} TGetHNRec;
typedef TGetHNRec far *PGetHNRec;

/* Tera Term internal key codes */
#define IdUp               1
#define IdDown             2
#define IdRight            3
#define IdLeft             4
#define Id0                5
#define Id1                6
#define Id2                7
#define Id3                8
#define Id4                9
#define Id5               10
#define Id6               11
#define Id7               12
#define Id8               13
#define Id9               14
#define IdMinus           15
#define IdComma           16
#define IdPeriod          17
#define IdSlash           18
#define IdAsterisk        19
#define IdPlus            20
#define IdEnter           21
#define IdPF1             22
#define IdPF2             23
#define IdPF3             24
#define IdPF4             25
#define IdFind            26
#define IdInsert          27
#define IdRemove          28
#define IdSelect          29
#define IdPrev            30
#define IdNext            31
#define IdF6              32
#define IdF7              33
#define IdF8              34
#define IdF9              35
#define IdF10             36
#define IdF11             37
#define IdF12             38
#define IdF13             39
#define IdF14             40
#define IdHelp            41
#define IdDo              42
#define IdF17             43
#define IdF18             44
#define IdF19             45
#define IdF20             46
#define IdXF1             47
#define IdXF2             48
#define IdXF3             49
#define IdXF4             50
#define IdXF5             51
#define IdUDK6            52
#define IdUDK7            53
#define IdUDK8            54
#define IdUDK9            55
#define IdUDK10           56
#define IdUDK11           57
#define IdUDK12           58
#define IdUDK13           59
#define IdUDK14           60
#define IdUDK15           61
#define IdUDK16           62
#define IdUDK17           63
#define IdUDK18           64
#define IdUDK19           65
#define IdUDK20           66
#define IdHold            67
#define IdPrint           68
#define IdBreak           69
#define IdCmdEditCopy     70
#define IdCmdEditPaste    71
#define IdCmdEditPasteCR  72
#define IdCmdEditCLS      73
#define IdCmdEditCLB      74
#define IdCmdCtrlOpenTEK  75
#define IdCmdCtrlCloseTEK 76
#define IdCmdLineUp       77
#define IdCmdLineDown     78
#define IdCmdPageUp       79
#define IdCmdPageDown     80
#define IdCmdBuffTop      81
#define IdCmdBuffBottom   82
#define IdCmdNextWin      83
#define IdCmdPrevWin      84
#define IdCmdLocalEcho    85
#define IdUser1           86
#define NumOfUDK          IdUDK20-IdUDK6+1
#define NumOfUserKey      99
#define IdKeyMax          IdUser1+NumOfUserKey-1

// key code for macro commands
#define IdCmdDisconnect   1000
#define IdCmdLoadKeyMap   1001
#define IdCmdRestoreSetup 1002

#define KeyStrMax 1023

// (user) key type IDs
#define IdBinary  0  // transmit text without any modification
#define IdText    1  // transmit text with new-line & DBCS conversions
#define IdMacro   2  // activate macro
#define IdCommand 3  // post a WM_COMMAND message

typedef struct {
	WORD Map[IdKeyMax];
	/* user key str position/length in buffer */
	int UserKeyPtr[NumOfUserKey], UserKeyLen[NumOfUserKey];
	BYTE UserKeyStr[KeyStrMax+1];
	/* user key type */
	BYTE UserKeyType[NumOfUserKey];
} TKeyMap;
typedef TKeyMap far *PKeyMap;

/* Control Characters */

#define NUL  0x00
#define SOH  0x01
#define STX  0x02
#define ETX  0x03
#define EOT  0x04
#define ENQ  0x05
#define ACK  0x06
#define BEL  0x07
#define BS   0x08
#define HT   0x09
#define LF   0x0A
#define VT   0x0B
#define FF   0x0C
#define CR   0x0D
#define SO   0x0E
#define SI   0x0F
#define DLE  0x10
#define DC1  0x11
#define XON  0x11
#define DC2  0x12
#define DC3  0x13
#define XOFF 0x13
#define DC4  0x14
#define NAK  0x15
#define SYN  0x16
#define ETB  0x17
#define CAN  0x18
#define EM   0x19
#define SUB  0x1A
#define ESC  0x1B
#define FS   0x1C
#define GS   0x1D
#define RS   0x1E
#define US   0x1F

#define SP   0x20

#define DEL  0x7F

#define IND  0x84
#define NEL  0x85
#define SSA  0x86
#define ESA  0x87
#define HTS  0x88
#define HTJ  0x89
#define VTS  0x8A
#define PLD  0x8B
#define PLU  0x8C
#define RI   0x8D
#define SS2  0x8E
#define SS3  0x8F
#define DCS  0x90
#define PU1  0x91
#define PU2  0x92
#define STS  0x93
#define CCH  0x94
#define MW   0x95
#define SPA  0x96
#define EPA  0x97
#define SOS  0x98


#define CSI  0x9B
#define ST   0x9C
#define OSC  0x9D
#define PM   0x9E
#define APC  0x9F

#define InBuffSize  1024
#define OutBuffSize 1024

typedef struct {
	BYTE InBuff[InBuffSize];
	int InBuffCount, InPtr;
	BYTE OutBuff[OutBuffSize];
	int OutBuffCount, OutPtr;

	HWND HWin;
	BOOL Ready;
	BOOL Open;
	WORD PortType;
	WORD ComPort;
	unsigned int s; /* SOCKET */
	WORD RetryCount;
	HANDLE ComID;
	BOOL CanSend, RRQ;

	BOOL SendKanjiFlag;
	BOOL EchoKanjiFlag;
	int SendCode;
	int EchoCode;
	BYTE SendKanjiFirst;
	BYTE EchoKanjiFirst;

	/* from VTSet */
	WORD Language;
	/* from TermSet */
	WORD CRSend;
	WORD KanjiCodeEcho;
	WORD JIS7KatakanaEcho;
	WORD KanjiCodeSend;
	WORD JIS7KatakanaSend;
	WORD KanjiIn;
	WORD KanjiOut;
	WORD RussHost;
	WORD RussClient;
	/* from PortSet */
	WORD DelayPerChar;
	WORD DelayPerLine;
	BOOL TelBinRecv, TelBinSend;

	BOOL DelayFlag;
	BOOL TelFlag, TelMode;
	BOOL IACFlag, TelCRFlag;
	BOOL TelCRSend, TelCRSendEcho;
	BOOL TelAutoDetect; /* TTPLUG */

	/* Text log */
	HANDLE HLogBuf;
	PCHAR LogBuf;
	int LogPtr, LStart, LCount;
	/* Binary log & DDE */
	HANDLE HBinBuf;
	PCHAR BinBuf;
	int BinPtr, BStart, BCount, DStart, DCount;
	int BinSkip;
	WORD FilePause;
	BOOL ProtoFlag;
	/* message flag */
	WORD NoMsg;
#ifndef NO_INET6
	/* if TRUE, teraterm trys to connect other protocol family */
	BOOL RetryWithOtherProtocol;
	struct addrinfo FAR * res0;
	struct addrinfo FAR * res;
#endif /* NO_INET6 */
	char *Locale;
	int *CodePage;
	int *ConnetingTimeout;

	time_t LastSendTime;
} TComVar;
typedef TComVar far *PComVar;

#define ID_FILE          0
#define ID_EDIT          1
#define ID_SETUP         2
#define ID_CONTROL       3
#define ID_HELPMENU      4

#define ID_WINDOW_1          50801
#define ID_WINDOW_WINDOW     50810
#define ID_TEKWINDOW_WINDOW  51810

#define ID_TRANSFER      4
#define ID_SHOWMENUBAR   995

#define MAXNWIN 50
#define MAXCOMPORT 256
#define MAXHOSTLIST 99

/* shared memory */
typedef struct {
	/* Setup information from "teraterm.ini" */
	TTTSet ts;
	/* Key code map from "keyboard.def" */
	TKeyMap km;
	// Window list
	int NWin;
	HWND WinList[MAXNWIN];
	/* COM port use flag
	 *           bit 8  7  6  5  4  3  2  1
	 * char[0] : COM 8  7  6  5  4  3  2  1
	 * char[1] : COM16 15 14 13 12 11 10  9 ...
	 */
	unsigned char ComFlag[(MAXCOMPORT-1)/CHAR_BIT+1];
} TMap;
typedef TMap far *PMap;


/*
 * Increment the number of this macro value
 * when you change TMap or member of TMap.
 *
 * - At version 4.56, ttset_memfilemap was replaced with ttset_memfilemap_4.
 *   added tttset.DisableAppKeypad.
 *   added tttset.DisableAppCursor.
 *   added tttset.ClearComBuffOnOpen.
 *   added tttset.Send8BitCtrl.
 *   added tttset.UILanguageFile_ini.
 *
 * - At version 4.54, ttset_memfilemap was replaced with ttset_memfilemap_3.
 *   added tttset.TelKeepAliveInterval.
 *   added tttset.MaxBroadcatHistory.
 *   changed pm.ComFlag type.
 *
 * - At version 4.53, ttset_memfilemap was replaced with ttset_memfilemap_2.
 *   added tttset.VTCompatTab.
 */

#define TT_FILEMAPNAME "ttset_memfilemap_4"

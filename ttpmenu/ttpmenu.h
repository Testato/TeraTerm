#ifndef TTPMENU_H
#define TTPMENU_H
/* ========================================================================
	Project  Name		: TeraTerm Menu
	Outline				: TeraTerm Menu Header
	Version				: 0.94
	Create				: 1998-11-22(Sun)
	Update				: 2001-11-01(Thu)
    Reference			: Copyright (C) S.Hayakawa 1997-2001
	======================================================================== */

#include	<windows.h>

// �e��萔
#define		WM_TMENU_NOTIFY			(WM_USER + 101)
#define		WM_MENUOPEN				(WM_USER + 102)
#define		ID_NOENTRY				49999
#define		ID_MENU_MIN				50000
#define		MAXJOBNUM				1024
#define		ICONSIZE_LARGE			32
#define		ICONSIZE_SMALL			16
#define		ICONSPACE_LARGE			40
#define		ICONSPACE_SMALL			24
#define		LISTBOX_HEIGHT			18
#define		LISTBOX_WIDTH			20
#define		TERATERM				"ttermpro.exe"
//#define		TTSSH					"ttssh.exe"
// ttssh.exe��UTF-8 TeraTerm�ł͕s�v�BTeraTerm�{�̂Ɠ����ł悢�B(2004.12.2 yutaka)
#define		TTSSH					TERATERM
#define		TTERM_KEY				"Software\\ShinpeiTools\\TTermMenu"
#define		LOGIN_PROMPT			"login:"
#define		PASSWORD_PROMPT			"Password:"
#define		TTPMENU_CLASS			"TMenuClass"

#define		DATA_NOENTRY			0xffffffff

#define		MODE_SMALLICON			0x0000
#define		MODE_LARGEICON			0x0001

#define		MODE_AUTOLOGIN			0x0000
#define		MODE_MACRO				0x0001
#define		MODE_DIRECT				0x0002

// ���W�X�g���̒l���i��ʐݒ�j
#define		KEY_ICONMODE			"IconMode"
#define		KEY_LEFTBUTTONPOPUP		"LeftButtonPopup"
#define		KEY_MENUTEXTCOLOR		"MenuTextColor"
#define		KEY_LF_HEIGHT			"lfHeight"
#define		KEY_LF_WIDTH			"lfWidth"
#define		KEY_LF_ESCAPEMENT		"lfEscapement"
#define		KEY_LF_ORIENTATION		"lfOrientation"
#define		KEY_LF_WEIGHT			"lfWeight"
#define		KEY_LF_ITALIC			"lfItalic"
#define		KEY_LF_UNDERLINE		"lfUnderline"
#define		KEY_LF_STRIKEOUT		"lfStrikeOut"
#define		KEY_LF_CHARSET			"lfCharSet"
#define		KEY_LF_OUTPRECISION		"lfOutPrecision"
#define		KEY_LF_CLIPPRECISION	"lfClipPrecision"
#define		KEY_LF_QUALITY			"lfQuality"
#define		KEY_LF_PITCHANDFAMILY	"lfPitchAndFamily"
#define		KEY_LF_FACENAME			"lfFaceName"
#define		KEY_HOTKEY				"Hotkey"

// ���W�X�g���̒l���i�W���u�ݒ�j
#define		KEY_MODE				"Mode"
#define		KEY_HOSTNAME			"HostName"
#define		KEY_USERFLAG			"UserFlag"
#define		KEY_USERNAME			"UserName"
#define		KEY_PASSWDFLAG			"PasswdFlag"
#define		KEY_PASSWORD			"Password"
#define		KEY_INITFILE			"INI_File"
#define		KEY_TERATERM			"TeraTerm"
#define		KEY_OPTION				"Option"
#define		KEY_LOGIN_PROMPT		"LoginPrompt"
#define		KEY_PASSWORD_PROMPT		"PasswdPrompt"
#define		KEY_MACROFILE			"MacroFile"
#define		KEY_TTSSH				"TeraTerm Mode"
#define		KEY_LOG					"Log"
#define		KEY_STARTUP				"Startup"
#define		KEY_KEYFILE				"PrivateKeyFile"  // add (2005.1.27 yutaka)
#define		KEY_CHALLENGE			"Challenge"       // add (2007.11.14 yutaka)
#define		KEY_PAGEANT				"Pageant"         // add (2008.5.26 maya)

#define		STR_ICONMODE			"showing large icon"
#define		STR_LEFTBUTTONPOPUP		"showing list by left-click"
#define		STR_HOTKEY				"showing list hotkey(Ctl+Alt+M)"
#define		STR_NOENTRY				"(none)"

// �ݒ���\����
struct JobInfo {
	char	szName[MAX_PATH];			// �W���u��
	BOOL	bStartup;					// �N�����ɃW���u�����s���邩�ǂ����̃t���O
	BOOL	bTtssh;						// ttssh���g�p���邩�ǂ����̃t���O
	DWORD	dwMode;						// �W���u�̎��

	// �������O�C���p�ݒ�
	char	szHostName[MAX_PATH];		// �z�X�g��
	BOOL	bUsername;					// ���[�U������͂��邩�ǂ����̃t���O
	char	szUsername[MAX_PATH];		// ���[�U��
	BOOL	bPassword;					// �p�X���[�h����͂��邩�ǂ����̃t���O
	char	szPassword[MAX_PATH];		// �p�X���[�h

	// �}�N�����s�p�ݒ�
	char	szMacroFile[MAX_PATH];		// ���s����}�N���t�@�C���̃t�@�C����

	// �ڍאݒ�
	char	szTeraTerm[MAX_PATH];		// �N���A�v���iTeraTerm�j�̃t�@�C����
	char	szInitFile[MAX_PATH];		// TeraTerm�̐ݒ�t�@�C���i�N���݈̂ӊO�j
	char	szOption[MAX_PATH];			// �A�v���P�[�V�����̃I�v�V����/����
	char	szLog[MAX_PATH];			// ���O�t�@�C�����i�������O�C���̂݁j
	char	szLoginPrompt[MAX_PATH];	// ���O�C���v�����v�g�i�������O�C���̂݁j
	char	szPasswdPrompt[MAX_PATH];	// �p�X���[�h�v�����v�g�i�������O�C���̂݁j

	char    PrivateKeyFile[MAX_PATH];   // �閧���t�@�C�� (2005.1.27 yutaka)
	BOOL    bChallenge;                 // keyboard-interative method(/challenge)
	BOOL    bPageant;                   // use Pageant(/pageant)
};

// �\���ݒ�\����
struct MenuData {
	TCHAR		szName[MAXJOBNUM][MAX_PATH];
	HICON		hLargeIcon[MAXJOBNUM];
	HICON		hSmallIcon[MAXJOBNUM];
	DWORD		dwMenuHeight;
	DWORD		dwIconMode;
	BOOL		bLeftButtonPopup;
	BOOL		bHotkey;
	HFONT		hFont;
	LOGFONT		lfFont;
	COLORREF	crMenuBg;
	COLORREF	crMenuTxt;
	COLORREF	crSelMenuBg;
	COLORREF	crSelMenuTxt;
};

// �֐��ꗗ
void	PopupMenu(HWND hWnd);
void	PopupListMenu(HWND hWnd);
BOOL	AddTooltip(int idControl);
BOOL	ConnectHost(HWND hWnd, UINT idItem, char *szJobName = NULL);
BOOL	CreateTooltip(void);
BOOL	DeleteLoginHostInformation(HWND hWnd);
BOOL	ErrorMessage(HWND hWnd, LPTSTR msg,...);
BOOL	ExtractAssociatedIconEx(char *szPath, HICON *hLargeIcon, HICON *hSmallIcon);
BOOL	ExecStartup(HWND hWnd);
BOOL	GetApplicationFilename(char *szName, char *szPath);
BOOL	InitConfigDlg(HWND hWnd);
BOOL	InitEtcDlg(HWND hWnd);
BOOL	InitListMenu(HWND hWnd);
BOOL	InitMenu(void);
BOOL	InitVersionDlg(HWND hWnd);
BOOL	LoadConfig(void);
BOOL	LoadLoginHostInformation(HWND hWnd);
BOOL	MakeTTL(char *TTLName, JobInfo *jobInfo);
BOOL	ManageWMCommand_Config(HWND hWnd, WPARAM wParam);
BOOL	ManageWMCommand_Etc(HWND hWnd, WPARAM wParam);
BOOL	ManageWMCommand_Menu(HWND hWnd, WPARAM wParam);
BOOL	ManageWMCommand_Version(HWND hWnd, WPARAM wParam);
BOOL	ManageWMNotify_Config(LPARAM lParam);
BOOL	RedrawMenu(HWND hWnd);
BOOL	RegLoadLoginHostInformation(char *szName, JobInfo *jobInfo);
BOOL	RegSaveLoginHostInformation(JobInfo *jobInfo);
BOOL	SaveConfig(void);
BOOL	SaveEtcInformation(HWND hWnd);
BOOL	SaveLoginHostInformation(HWND hWnd);
BOOL	SetDefaultEtcDlg(HWND hWnd);
BOOL	SetMenuFont(HWND hWnd);
BOOL	SetTaskTray(HWND hWnd, DWORD dwMessage);
BOOL	CALLBACK DlgCallBack_Config(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL	CALLBACK DlgCallBack_Etc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL	CALLBACK DlgCallBack_Version(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT	CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT	CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif

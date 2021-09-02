#pragma once

#include <my.h>
#include <windowsx.h>
#include <ShlObj.h>
#include <Stubs.h>
#include <vector>
#include <string>
#include <Shobjidl.h>
#include <uxtheme.h>
#include "resource.h"
#include "FolderDialog.h"


#pragma comment(lib, "version.lib")
#pragma comment(lib, "UxTheme.lib")

class UIViewer;
class UIViewerServer final : public ServerStub
{
public:
	UIViewerServer() = delete;
	UIViewerServer(const UIViewerServer&) = delete;
	UIViewerServer(UIViewer* Viewer) : m_Viewer(Viewer) {};

	BOOL NotifyServerProgressBar(PCWSTR TaskName, ULONGLONG Current, ULONGLONG Total);
	BOOL NotifyServerLogOutput(LogLevel Level, PCWSTR Command);
	BOOL NotifyServerCommandResultOutput(CommandStatus Status, PCWSTR Reply);
	BOOL NotifyServerUIReady();
	BOOL NotifyServerMessageBox(PCWSTR Description, ULONG Flags, BOOL Locked);
	BOOL NotifyServerTaskStartAndDisableUI();
	BOOL NotifyServerTaskEndAndEnableUI(BOOL TaskCompleteStatus, PCWSTR Description);
	BOOL NotifyServerUIHeartbeatPackage();
	BOOL NotifyServerHandShakeFromRemote(ULONG Secret);
	BOOL NotifyServerExitFromRemoteProcess();

private:
	UIViewer* m_Viewer;
};


enum IMMERSIVE_HC_CACHE_MODE
{
	IHCM_USE_CACHED_VALUE,
	IHCM_REFRESH
};


enum PreferredAppMode
{
	Default,
	AllowDark,
	ForceDark,
	ForceLight,
	Max
};

enum WINDOWCOMPOSITIONATTRIB
{
	WCA_UNDEFINED = 0,
	WCA_NCRENDERING_ENABLED = 1,
	WCA_NCRENDERING_POLICY = 2,
	WCA_TRANSITIONS_FORCEDISABLED = 3,
	WCA_ALLOW_NCPAINT = 4,
	WCA_CAPTION_BUTTON_BOUNDS = 5,
	WCA_NONCLIENT_RTL_LAYOUT = 6,
	WCA_FORCE_ICONIC_REPRESENTATION = 7,
	WCA_EXTENDED_FRAME_BOUNDS = 8,
	WCA_HAS_ICONIC_BITMAP = 9,
	WCA_THEME_ATTRIBUTES = 10,
	WCA_NCRENDERING_EXILED = 11,
	WCA_NCADORNMENTINFO = 12,
	WCA_EXCLUDED_FROM_LIVEPREVIEW = 13,
	WCA_VIDEO_OVERLAY_ACTIVE = 14,
	WCA_FORCE_ACTIVEWINDOW_APPEARANCE = 15,
	WCA_DISALLOW_PEEK = 16,
	WCA_CLOAK = 17,
	WCA_CLOAKED = 18,
	WCA_ACCENT_POLICY = 19,
	WCA_FREEZE_REPRESENTATION = 20,
	WCA_EVER_UNCLOAKED = 21,
	WCA_VISUAL_OWNER = 22,
	WCA_HOLOGRAPHIC = 23,
	WCA_EXCLUDED_FROM_DDA = 24,
	WCA_PASSIVEUPDATEMODE = 25,
	WCA_USEDARKMODECOLORS = 26,
	WCA_LAST = 27
};

struct WINDOWCOMPOSITIONATTRIBDATA
{
	WINDOWCOMPOSITIONATTRIB Attribute;
	PVOID pvData;
	SIZE_T cbData;
};

class UIViewer
{
public:
	UIViewer();
	UIViewer(const UIViewer&) = delete;
	BOOL ActiveUIViewer(PVOID DllModule, ClientStub* Client, ServerStub** Server);
	BOOL NotifyDllLoad(PVOID Module);
	
private:

	//
	// https://github.com/ysc3839/win32-darkmode/blob/master/win32-darkmode/DarkMode.h
	//

	using RefreshImmersiveColorPolicyStateApi = void (WINAPI *)(); // ordinal 104
	using ShouldAppsUseDarkModeApi  = bool (WINAPI *)(); // ordinal 132
	using AllowDarkModeForWindowApi = bool (WINAPI *)(HWND hWnd, bool allow); // ordinal 133
	using AllowDarkModeForAppApi    = bool (WINAPI *)(bool allow); // ordinal 135, in 1809
	using IsDarkModeAllowedForWindowApi           = bool (WINAPI *)(HWND hWnd); // ordinal 137
	using GetIsImmersiveColorUsingHighContrastApi = bool (WINAPI *)(IMMERSIVE_HC_CACHE_MODE mode); // ordinal 106
	using ShouldSystemUseDarkModeApi = bool (WINAPI *)(); // ordinal 138
	using SetPreferredAppModeApi     = PreferredAppMode(WINAPI *)(PreferredAppMode appMode); // ordinal 135, in 1903
	using IsDarkModeAllowedForAppApi = bool (WINAPI *)(); // ordinal 139
	using SetWindowCompositionAttributeApi = BOOL(WINAPI *)(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA*);


	ShouldAppsUseDarkModeApi                m_ShouldAppsUseDarkMode                = nullptr;
	AllowDarkModeForWindowApi               m_AllowDarkModeForWindow               = nullptr;
	AllowDarkModeForAppApi                  m_AllowDarkModeForApp                  = nullptr;
	RefreshImmersiveColorPolicyStateApi     m_RefreshImmersiveColorPolicyState     = nullptr;
	IsDarkModeAllowedForWindowApi           m_IsDarkModeAllowedForWindow           = nullptr;
	GetIsImmersiveColorUsingHighContrastApi m_GetIsImmersiveColorUsingHighContrast = nullptr;
	SetWindowCompositionAttributeApi        m_SetWindowCompositionAttribute        = nullptr;

	//
	// 1903 18362
	//
	ShouldSystemUseDarkModeApi m_ShouldSystemUseDarkMode = nullptr;
	SetPreferredAppModeApi     m_SetPreferredAppMode     = nullptr;


	BOOL  m_DarkModeEnabled = FALSE;
	BOOL  m_DarkModeSupported = FALSE;
	DWORD m_Major = 0;
	DWORD m_Minor = 0;
	DWORD m_BuildNumber = 0;

	const COLORREF DarkBkColor   = 0x383838;
	const COLORREF DarkTextColor = 0xFFFFFF;

	HBRUSH m_NewBgBrush = nullptr;

	inline constexpr BOOL IsDarkModeReady()
	{
		return m_DarkModeEnabled && m_DarkModeSupported;
	}

	VOID RefreshImmersiveColorPolicyState()
	{
		if (!IsDarkModeReady()) {
			return;
		}

		if (!m_RefreshImmersiveColorPolicyState) {
			return;
		}

		m_RefreshImmersiveColorPolicyState();
	}

	BOOL ShouldAppsUseDarkMode()
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_ShouldAppsUseDarkMode) {
			return FALSE;
		}

		return m_ShouldAppsUseDarkMode();
	}

	BOOL AllowDarkModeForWindow(HWND hWnd, BOOL Allow)
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_AllowDarkModeForWindow) {
			return FALSE;
		}

		return m_AllowDarkModeForWindow(hWnd, Allow);
	}

	BOOL AllowDarkModeForApp(BOOL Allow)
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_AllowDarkModeForApp) {
			return FALSE;
		}

		return m_AllowDarkModeForApp(Allow);
	}

	BOOL IsDarkModeAllowedForWindow(HWND hWnd)
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_IsDarkModeAllowedForWindow) {
			return FALSE;
		}

		return m_IsDarkModeAllowedForWindow(hWnd);
	}

	BOOL GetIsImmersiveColorUsingHighContrast(IMMERSIVE_HC_CACHE_MODE Mode)
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_GetIsImmersiveColorUsingHighContrast) {
			return FALSE;
		}

		return m_GetIsImmersiveColorUsingHighContrast(Mode);
	}

	BOOL ShouldSystemUseDarkMode()
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_ShouldSystemUseDarkMode) {
			return FALSE;
		}

		return m_ShouldSystemUseDarkMode();
	}

	BOOL SetPreferredAppMode(PreferredAppMode AppMode)
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_SetPreferredAppMode) {
			return FALSE;
		}

		return m_SetPreferredAppMode(AppMode);
	}

	BOOL SetWindowCompositionAttribute(HWND hWnd, WINDOWCOMPOSITIONATTRIBDATA* Data)
	{
		if (!IsDarkModeReady()) {
			return FALSE;
		}

		if (!m_SetWindowCompositionAttribute) {
			return FALSE;
		}

		return m_SetWindowCompositionAttribute(hWnd, Data);
	}

	constexpr BOOL IsWindows10And1809Higher(DWORD Major, DWORD Minor, DWORD BuildNumber)
	{
		if (Major != 10 || Minor != 0) {
			return FALSE;
		}

		//
		// 1809
		//

		return BuildNumber >= 17763;
	}

	bool IsHighContrast()
	{
		HIGHCONTRASTW highContrast = { sizeof(highContrast) };

		if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(highContrast), &highContrast, FALSE))
			return highContrast.dwFlags & HCF_HIGHCONTRASTON;

		return false;
	}

	VOID InitDarkMode()
	{
		HMODULE Uxtheme;
		HMODULE User32;

		RtlGetNtVersionNumbers(&m_Major, &m_Minor, &m_BuildNumber);
		if (!IsWindows10And1809Higher(m_Major, m_Minor, m_BuildNumber))
			return;

		Uxtheme = LoadLibraryExW(L"uxtheme.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (!Uxtheme)
			return;

		User32 = LoadLibraryExW(L"user32.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
		if (!User32)
			return;

		m_SetWindowCompositionAttribute        = reinterpret_cast<SetWindowCompositionAttributeApi>(Nt_GetProcAddress(User32, "SetWindowCompositionAttribute"));
		m_RefreshImmersiveColorPolicyState     = reinterpret_cast<RefreshImmersiveColorPolicyStateApi>(Nt_GetProcAddress(Uxtheme, MAKEINTRESOURCEA(104)));
		m_GetIsImmersiveColorUsingHighContrast = reinterpret_cast<GetIsImmersiveColorUsingHighContrastApi>(Nt_GetProcAddress(Uxtheme, MAKEINTRESOURCEA(106)));
		m_ShouldAppsUseDarkMode                = reinterpret_cast<ShouldAppsUseDarkModeApi>(Nt_GetProcAddress(Uxtheme, MAKEINTRESOURCEA(132)));
		m_AllowDarkModeForWindow               = reinterpret_cast<AllowDarkModeForWindowApi>(Nt_GetProcAddress(Uxtheme, MAKEINTRESOURCEA(133)));
		
		if (m_BuildNumber < 18362) {
			m_AllowDarkModeForApp = reinterpret_cast<AllowDarkModeForAppApi>(Nt_GetProcAddress(Uxtheme, MAKEINTRESOURCEA(135)));
		}
		else {
			m_SetPreferredAppMode = reinterpret_cast<SetPreferredAppModeApi>(Nt_GetProcAddress(Uxtheme, MAKEINTRESOURCEA(135)));
		}

		m_IsDarkModeAllowedForWindow = reinterpret_cast<IsDarkModeAllowedForWindowApi>(Nt_GetProcAddress(Uxtheme, MAKEINTRESOURCEA(137)));


		if (m_RefreshImmersiveColorPolicyState &&
			m_ShouldAppsUseDarkMode &&
			m_AllowDarkModeForWindow &&
			(m_AllowDarkModeForApp || m_SetPreferredAppMode) &&
			m_IsDarkModeAllowedForWindow)
		{
			m_DarkModeSupported = TRUE;

			if (m_BuildNumber < 18362) {
				m_AllowDarkModeForApp(TRUE);
			}
			else {
				m_SetPreferredAppMode(PreferredAppMode::AllowDark);
			}

			m_RefreshImmersiveColorPolicyState();

			m_DarkModeEnabled = m_ShouldAppsUseDarkMode() && !IsHighContrast();
		}
	}

	BOOL IsColorSchemeChangeMessage(LPARAM lParam)
	{
		BOOL Status;

		Status = FALSE;
		if (lParam && CompareStringOrdinal(reinterpret_cast<LPCWCH>(lParam), -1, L"ImmersiveColorSet", -1, TRUE) == CSTR_EQUAL)
		{
			RefreshImmersiveColorPolicyState();
			Status = TRUE;
		}

		GetIsImmersiveColorUsingHighContrast(IHCM_REFRESH);

		return Status;
	}

	BOOL IsColorSchemeChangeMessage(UINT Message, LPARAM lParam)
	{
		if (Message == WM_SETTINGCHANGE)
			return IsColorSchemeChangeMessage(lParam);

		return FALSE;
	}

	void RefreshTitleBarThemeColor(HWND hWnd)
	{
		BOOL dark = FALSE;

		if (IsDarkModeAllowedForWindow(hWnd) &&
			ShouldAppsUseDarkMode() &&
			!IsHighContrast())
		{
			dark = TRUE;
		}

		if (m_BuildNumber < 18362) {
			SetPropW(hWnd, L"UseImmersiveDarkModeColors", reinterpret_cast<HANDLE>(static_cast<INT_PTR>(dark)));
		}
		else if (m_SetWindowCompositionAttribute)
		{
			WINDOWCOMPOSITIONATTRIBDATA data = { WCA_USEDARKMODECOLORS, &dark, sizeof(dark) };
			SetWindowCompositionAttribute(hWnd, &data);
		}
	}


	VOID InitSubWindows(HWND hWnd)
	{
		m_hWndPsbRaw       = GetDlgItem(hWnd, IDC_PSB_RAW);
		m_hWndPsbText      = GetDlgItem(hWnd, IDC_PSB_TEXT);
		m_hWndPsbImage     = GetDlgItem(hWnd, IDC_PSB_IMAGE);
		m_hWndPsbAnm       = GetDlgItem(hWnd, IDC_PSB_ANIM);
		m_hWndPsbDecompile = GetDlgItem(hWnd, IDC_PSB_SCRIPT);
		m_hWndPsbAll       = GetDlgItem(hWnd, IDC_PSB_ALL);
		m_hWndPsbJson      = GetDlgItem(hWnd, IDC_PSB_JSON);

		m_hWndTextRaw   = GetDlgItem(hWnd, IDC_TEXT_RAW);
		m_hWndTextPlain = GetDlgItem(hWnd, IDC_TEXT_TEXT);

		m_hWndIcon           = GetDlgItem(hWnd, IDC_PACK_SAME_ICON);
		m_hWndProtect        = GetDlgItem(hWnd, IDC_PACK_ADD_PROTECT);
		m_hWndUniversalPatch = GetDlgItem(hWnd, IDC_PACK_BUTTON_UMAKE);

		m_hWndTlgRaw     = GetDlgItem(hWnd, IDC_TLG_RAW);
		m_hWndTlgBuildin = GetDlgItem(hWnd, IDC_TLG_BUILDIN);
		m_hWndTlgSys     = GetDlgItem(hWnd, IDC_TLG_SYSTEM);
		m_hWndTlgPng     = GetDlgItem(hWnd, IDC_TLG_PNG);
		m_hWndTlgJpg     = GetDlgItem(hWnd, IDC_TLG_JPG);

		m_hWndTjsRaw       = GetDlgItem(hWnd, IDC_TJS2_RAW);
		m_hWndTjsDisasm    = GetDlgItem(hWnd, IDC_TJS2_DISASM);
		m_hWndTjsDecompile = GetDlgItem(hWnd, IDC_TJS2_Decomp);

		m_hWndAmvRaw = GetDlgItem(hWnd, IDC_AMV_RAW);
		m_hWndAmvPNG = GetDlgItem(hWnd, IDC_AMV_PNG);
		m_hWndAmvJPG = GetDlgItem(hWnd, IDC_AMV_JPG);
		m_hWndAmvGIF = GetDlgItem(hWnd, IDC_AMV_GIF);

		m_hWndPngRaw = GetDlgItem(hWnd, IDC_PNG_RAW);
		m_hWndPngSys = GetDlgItem(hWnd, IDC_PNG_SYSTEM);

		m_hWndBaseDirEdit         = GetDlgItem(hWnd, IDC_PACK_EDIT_FOLDER);
		m_hWndOriginalArchiveEdit = GetDlgItem(hWnd, IDC_PACK_EDIT_ORIPACK);
		m_hWndOutputArchiveEdit   = GetDlgItem(hWnd, IDC_PACK_EDIT_OUTPACK);

		m_hWndBaseDirSelect         = GetDlgItem(hWnd, IDC_PACK_BUTTON_FOLDER);
		m_hWndOriginalArchiveSelect = GetDlgItem(hWnd, IDC_PACK_BUTTON_ORIPACK);
		m_hWndOutputArchiveSelect   = GetDlgItem(hWnd, IDC_PACK_BUTTON_OUTPACK);

		m_hWndMakePack = GetDlgItem(hWnd, IDC_PACK_BUTTON_MAKE);

		m_hWndProgressBar = GetDlgItem(hWnd, IDC_PROGRESS1);
		m_hWndConsole     = GetDlgItem(hWnd, IDC_BUTTON_DEBUGGER);
		m_hWndCancelTask  = GetDlgItem(hWnd, IDC_BUTTON_CANCEL);

		m_hWndPbdRaw  = GetDlgItem(hWnd, IDC_RADIO_PBD_RAW);
		m_hWndPbdJson = GetDlgItem(hWnd, IDC_RADIO_PBD_JSON);

		m_hWndUniversalDump  = GetDlgItem(hWnd, IDC_BUTTON_UDUMP);
		m_hWndVirtualConsole = GetDlgItem(hWnd, IDC_VCONSOLE);
	}


	std::vector<HWND> GetChildWindows()
	{
		return {
			m_hWndPsbRaw,
			m_hWndPsbText,
			m_hWndPsbImage,
			m_hWndPsbAnm,
			m_hWndPsbDecompile,
			m_hWndPsbAll,
			m_hWndPsbJson,
			m_hWndTextRaw,
			m_hWndTextPlain,
			m_hWndIcon,
			m_hWndProtect,
			m_hWndUniversalPatch,
			m_hWndTlgRaw,
			m_hWndTlgBuildin,
			m_hWndTlgSys,
			m_hWndTlgPng,
			m_hWndTlgJpg,
			m_hWndTjsRaw,
			m_hWndTjsDisasm,
			m_hWndTjsDecompile,
			m_hWndAmvRaw,
			m_hWndAmvPNG,
			m_hWndAmvJPG,
			m_hWndAmvGIF,
			m_hWndPngRaw,
			m_hWndPngSys,
			m_hWndBaseDirEdit,
			m_hWndOriginalArchiveEdit,
			m_hWndOutputArchiveEdit,
			m_hWndBaseDirSelect,
			m_hWndOriginalArchiveSelect,
			m_hWndOutputArchiveSelect,
			m_hWndMakePack,
			m_hWndProgressBar,
			m_hWndConsole,
			m_hWndCancelTask,
			m_hWndPbdRaw,
			m_hWndPbdJson,
			m_hWndUniversalDump,
			m_hWndVirtualConsole
		};
	}

	VOID InitProgressBar()
	{
		SendMessageW(m_hWndProgressBar, PBM_SETRANGE, (WPARAM)0, (LPARAM)(MAKELPARAM(0, 100)));
		SendMessageW(m_hWndProgressBar, PBM_SETSTEP, 1, 0);
	}

public:
	VOID EnterTask()
	{
		EnableWindow(m_hWndPsbRaw,       FALSE);
		EnableWindow(m_hWndPsbText,      FALSE);
		EnableWindow(m_hWndPsbImage,     FALSE);
		EnableWindow(m_hWndPsbAnm,       FALSE);
		EnableWindow(m_hWndPsbDecompile, FALSE);
		EnableWindow(m_hWndPsbAll,       FALSE);
		EnableWindow(m_hWndPsbJson,      FALSE);

		EnableWindow(m_hWndTextRaw,   FALSE);
		EnableWindow(m_hWndTextPlain, FALSE);

		EnableWindow(m_hWndIcon,           FALSE);
		EnableWindow(m_hWndProtect,        FALSE);
		EnableWindow(m_hWndUniversalPatch, FALSE);

		EnableWindow(m_hWndTlgRaw,     FALSE);
		EnableWindow(m_hWndTlgBuildin, FALSE);
		EnableWindow(m_hWndTlgSys,     FALSE);
		EnableWindow(m_hWndTlgPng,     FALSE);
		EnableWindow(m_hWndTlgJpg,     FALSE);

		EnableWindow(m_hWndTjsRaw,       FALSE);
		EnableWindow(m_hWndTjsDisasm,    FALSE);
		EnableWindow(m_hWndTjsDecompile, FALSE);

		EnableWindow(m_hWndAmvRaw, FALSE);
		EnableWindow(m_hWndAmvPNG, FALSE);
		EnableWindow(m_hWndAmvJPG, FALSE);
		EnableWindow(m_hWndAmvGIF, FALSE);

		EnableWindow(m_hWndPngRaw, FALSE);
		EnableWindow(m_hWndPngSys, FALSE);

		EnableWindow(m_hWndBaseDirEdit, FALSE);
		EnableWindow(m_hWndOriginalArchiveEdit, FALSE);
		EnableWindow(m_hWndOutputArchiveEdit, FALSE);
		
		EnableWindow(m_hWndBaseDirEdit,         FALSE);
		EnableWindow(m_hWndOriginalArchiveEdit, FALSE);
		EnableWindow(m_hWndOutputArchiveEdit,   FALSE);

		EnableWindow(m_hWndBaseDirSelect, FALSE);
		EnableWindow(m_hWndOriginalArchiveSelect, FALSE);
		EnableWindow(m_hWndOutputArchiveSelect, FALSE);

		EnableWindow(m_hWndMakePack, FALSE);

		EnableWindow(m_hWndProgressBar, FALSE);
		EnableWindow(m_hWndConsole,     FALSE);
		EnableWindow(m_hWndCancelTask,  TRUE);

		EnableWindow(m_hWndPbdRaw,  FALSE);
		EnableWindow(m_hWndPbdJson, FALSE);

		EnableWindow(m_hWndUniversalDump, FALSE);
	}

	VOID LeaveTask()
	{
		EnableWindow(m_hWndPsbRaw,       TRUE);
		EnableWindow(m_hWndPsbText,      TRUE);
		EnableWindow(m_hWndPsbImage,     TRUE);
		EnableWindow(m_hWndPsbAnm,       TRUE);
		EnableWindow(m_hWndPsbDecompile, TRUE);
		EnableWindow(m_hWndPsbAll,       TRUE);
		EnableWindow(m_hWndPsbJson,      TRUE);

		EnableWindow(m_hWndTextRaw,   TRUE);
		EnableWindow(m_hWndTextPlain, TRUE);

		EnableWindow(m_hWndIcon,           TRUE);
		EnableWindow(m_hWndProtect,        TRUE);
		EnableWindow(m_hWndUniversalPatch, TRUE);

		EnableWindow(m_hWndTlgRaw,     TRUE);
		EnableWindow(m_hWndTlgBuildin, TRUE);
		EnableWindow(m_hWndTlgSys,     TRUE);
		EnableWindow(m_hWndTlgPng,     TRUE);
		EnableWindow(m_hWndTlgJpg,     TRUE);

		EnableWindow(m_hWndTjsRaw,       TRUE);
		EnableWindow(m_hWndTjsDisasm,    TRUE);
		EnableWindow(m_hWndTjsDecompile, TRUE);

		EnableWindow(m_hWndAmvRaw, TRUE);
		EnableWindow(m_hWndAmvPNG, TRUE);
		EnableWindow(m_hWndAmvJPG, TRUE);
		EnableWindow(m_hWndAmvGIF, TRUE);

		EnableWindow(m_hWndPngRaw, TRUE);
		EnableWindow(m_hWndPngSys, TRUE);

		EnableWindow(m_hWndBaseDirEdit, TRUE);
		EnableWindow(m_hWndOriginalArchiveEdit, TRUE);
		EnableWindow(m_hWndOutputArchiveEdit, TRUE);
		
		EnableWindow(m_hWndBaseDirEdit,         TRUE);
		EnableWindow(m_hWndOriginalArchiveEdit, TRUE);
		EnableWindow(m_hWndOutputArchiveEdit,   TRUE);

		EnableWindow(m_hWndBaseDirSelect, TRUE);
		EnableWindow(m_hWndOriginalArchiveSelect, TRUE);
		EnableWindow(m_hWndOutputArchiveSelect, TRUE);

		EnableWindow(m_hWndMakePack, TRUE);

		EnableWindow(m_hWndProgressBar, TRUE);
		EnableWindow(m_hWndConsole,     TRUE);
		EnableWindow(m_hWndCancelTask,  FALSE);

		EnableWindow(m_hWndPbdRaw,  TRUE);
		EnableWindow(m_hWndPbdJson, TRUE);

		EnableWindow(m_hWndUniversalDump, TRUE);

		ResetTitle();
	}

	VOID ResetTitle()
	{
		WCHAR Buffer[0x200];

		FormatStringW(Buffer, L"KrkrExtract(local %s) - built on " MAKE_WSTRING(__DATE__) L" " MAKE_WSTRING(__TIME__), GetVersion());
		SetWindowTextW(m_hWndMain, Buffer);
	}

private:
	VOID SwitchToInitialStatus()
	{
		EnableWindow(m_hWndCancelTask, FALSE);

		Button_SetCheck(m_hWndPngRaw, BST_CHECKED);
		Button_SetCheck(m_hWndPngSys, BST_UNCHECKED);

		Button_SetCheck(m_hWndPsbRaw,       BST_CHECKED);
		Button_SetCheck(m_hWndPsbDecompile, BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbText,      BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbImage,     BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbAnm,       BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbAll,       BST_UNCHECKED);

		Button_SetCheck(m_hWndTextRaw,   BST_CHECKED);
		Button_SetCheck(m_hWndTextPlain, BST_UNCHECKED);

		Button_SetCheck(m_hWndTlgRaw,     BST_CHECKED);
		Button_SetCheck(m_hWndTlgBuildin, BST_UNCHECKED);
		Button_SetCheck(m_hWndTlgSys,     BST_UNCHECKED);
		Button_SetCheck(m_hWndTlgPng,     BST_UNCHECKED);
		Button_SetCheck(m_hWndTlgJpg,     BST_UNCHECKED);

		Button_SetCheck(m_hWndTjsRaw,       BST_CHECKED);
		Button_SetCheck(m_hWndTjsDisasm,    BST_CHECKED);
		Button_SetCheck(m_hWndTjsDecompile, BST_UNCHECKED);

		Button_SetCheck(m_hWndAmvRaw, BST_CHECKED);
		Button_SetCheck(m_hWndAmvPNG, BST_UNCHECKED);
		Button_SetCheck(m_hWndAmvJPG, BST_UNCHECKED);
		Button_SetCheck(m_hWndAmvGIF, BST_UNCHECKED);

		Button_SetCheck(m_hWndPbdRaw,  BST_CHECKED);
		Button_SetCheck(m_hWndPbdJson, BST_UNCHECKED);
	}


	PCWSTR GetVersion()
	{
		return m_Version.c_str();
	}

	BOOL OnInitDialog(HWND hWnd, HWND /* hWndFocus */, LPARAM /* lParam */)
	{
		RECT        ClientRect;
		NTSTATUS    Status;


		if (m_DarkModeSupported)
		{
			AllowDarkModeForWindow(hWnd, TRUE);
			RefreshTitleBarThemeColor(hWnd);
		}

		if (m_DarkModeSupported)
		{
			auto&& ChildWindows = GetChildWindows();
			
			for (auto& Window : ChildWindows)
			{
				SetWindowTheme(Window, L"Explorer", nullptr);
				SendMessageW(Window, WM_THEMECHANGED, 0, 0);
			}
		}

		ResetTitle();

		DragAcceptFiles(hWnd, TRUE);
		InitSubWindows(hWnd);
		SwitchToInitialStatus();

		GetClientRect(hWnd, &ClientRect);
		m_LastWindowSize.cx = ClientRect.right - ClientRect.left;
		m_LastWindowSize.cy = ClientRect.bottom - ClientRect.top;

		GetWindowRect(hWnd, &ClientRect);
		m_MinSize.cx = ClientRect.right  - ClientRect.left;
		m_MinSize.cy = ClientRect.bottom - ClientRect.top;

		return TRUE;
	}


	VOID OnClose(HWND hWnd)
	{
		DestroyWindow(hWnd);
	}

	VOID OnDestroy(HWND hWnd)
	{
		UNREFERENCED_PARAMETER(hWnd);

		if (m_NewBgBrush) 
		{
			DeleteObject(m_NewBgBrush);
			m_NewBgBrush = nullptr;
		}

		Ps::ExitProcess(0);
	}

	VOID OnCommandBaseDirSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		BOOL         Success;
		WCHAR        PathName[MAX_PATH];
		BROWSEINFOW  FolderInfo;
		LPITEMIDLIST List;
		
		RtlZeroMemory(&FolderInfo, sizeof(BROWSEINFOW));
		RtlZeroMemory(PathName,    sizeof(PathName));

		List = nullptr;
		FolderInfo.hwndOwner = hWnd;
		FolderInfo.lpszTitle = L"Select the base directory";
		FolderInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_UAHINT | BIF_NONEWFOLDERBUTTON;

		LOOP_ONCE
		{
			List = SHBrowseForFolderW(&FolderInfo);
			if (!List)
				break;

			Success = SHGetPathFromIDListW(List, PathName);
			if (!Success)
				break;

			Static_SetText(m_hWndBaseDirEdit, PathName);
		}

		if (List) {
			CoTaskMemFree(List);
		}
	}

	VOID OnCommandOriginalArchiveSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		BOOL          Success;
		OPENFILENAMEW OpenInfo;
		WCHAR         PathName[MAX_PATH];
		WCHAR         CurrentDir[MAX_PATH];

		RtlZeroMemory(PathName, sizeof(PathName));
		RtlZeroMemory(CurrentDir, sizeof(CurrentDir));
		RtlZeroMemory(&OpenInfo, sizeof(OPENFILENAMEW));

		OpenInfo.lStructSize = sizeof(OPENFILENAMEW);
		OpenInfo.hwndOwner   = hWnd;
		OpenInfo.lpstrFilter = L"XP3 File\0*.*";
		OpenInfo.lpstrFile   = PathName;
		OpenInfo.nMaxFile    = countof(PathName);
		OpenInfo.lpstrTitle  = L"Select an original xp3 file";

		GetCurrentDirectoryW(MAX_PATH, CurrentDir);
		OpenInfo.lpstrInitialDir = CurrentDir;
		OpenInfo.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

		Success = GetOpenFileNameW(&OpenInfo);
		if (Success) {
			Static_SetText(m_hWndOriginalArchiveEdit, PathName);
		}
	}


	VOID OnCommandOutputArchiveSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		BOOL          Success;
		OPENFILENAMEW OpenInfo;
		WCHAR         PathName[MAX_PATH];
		WCHAR         CurrentDir[MAX_PATH];

		RtlZeroMemory(PathName, sizeof(PathName));
		RtlZeroMemory(CurrentDir, sizeof(CurrentDir));
		RtlZeroMemory(&OpenInfo, sizeof(OPENFILENAMEW));

		OpenInfo.lStructSize = sizeof(OPENFILENAMEW);
		OpenInfo.hwndOwner   = hWnd;
		OpenInfo.lpstrFilter = L"XP3 File\0*.xp3";
		OpenInfo.lpstrFile   = PathName;
		OpenInfo.nMaxFile    = countof(PathName);
		OpenInfo.lpstrTitle  = L"Output xp3 file";

		GetCurrentDirectoryW(MAX_PATH, CurrentDir);
		OpenInfo.lpstrInitialDir = CurrentDir;
		OpenInfo.Flags = OFN_EXPLORER;
		
		Success= GetOpenFileNameW(&OpenInfo);
		if (Success) {
			Static_SetText(m_hWndOutputArchiveEdit, PathName);
		}
	}

	VOID OnCommandPngRawSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndPngRaw, BM_GETCHECK, 0, 0)) {
			m_PngFlag = KrkrPngMode::PNG_RAW;
		}
	}

	VOID OnCommandPngSystemSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndPngRaw, BM_GETCHECK, 0, 0)) {
			m_PngFlag = KrkrPngMode::PNG_SYS;
		}
	}

	BOOL PsbSelectStatus(ULONG& Status)
	{
		BOOL Checked;

		Checked = FALSE;
		Status  = 0;

		if (SendMessageW(m_hWndPsbAll, BM_GETCHECK, 0, 0))
		{
			SET_FLAG(Status, (ULONG)KrkrPsbMode::PSB_ALL);
			Checked = TRUE;
		}

		if (SendMessageW(m_hWndPsbRaw, BM_GETCHECK, 0, 0))
		{
			SET_FLAG(Status, (ULONG)KrkrPsbMode::PSB_RAW);
			Checked = TRUE;
		}

		if (SendMessageW(m_hWndPsbText, BM_GETCHECK, 0, 0))
		{
			SET_FLAG(Status, (ULONG)KrkrPsbMode::PSB_TEXT);
			Checked = TRUE;
		}

		if (SendMessageW(m_hWndPsbImage, BM_GETCHECK, 0, 0))
		{
			SET_FLAG(Status, (ULONG)KrkrPsbMode::PSB_IMAGE);
			Checked = TRUE;
		}

		if (SendMessageW(m_hWndPsbDecompile, BM_GETCHECK, 0, 0))
		{
			SET_FLAG(Status, (ULONG)KrkrPsbMode::PSB_DECOM);
			Checked = TRUE;
		}

		if (SendMessageW(m_hWndPsbAnm, BM_GETCHECK, 0, 0))
		{
			SET_FLAG(Status, (ULONG)KrkrPsbMode::PSB_ANM);
			Checked = TRUE;
		}

		if (SendMessageW(m_hWndPsbJson, BM_GETCHECK, 0, 0))
		{
			SET_FLAG(Status, (ULONG)KrkrPsbMode::PSB_JSON);
			Checked = TRUE;
		}

		return Checked;
	}

	VOID PsbSelectRawOnly()
	{
		Button_SetCheck(m_hWndPsbRaw,       BST_CHECKED);
		Button_SetCheck(m_hWndPsbText,      BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbImage,     BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbDecompile, BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbAnm,       BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbAll,       BST_UNCHECKED);
		Button_SetCheck(m_hWndPsbJson,      BST_UNCHECKED);
	}

	VOID PsbSelectAll()
	{
		Button_SetCheck(m_hWndPsbRaw,       BST_CHECKED);
		Button_SetCheck(m_hWndPsbText,      BST_CHECKED);
		Button_SetCheck(m_hWndPsbImage,     BST_CHECKED);
		Button_SetCheck(m_hWndPsbDecompile, BST_CHECKED);
		Button_SetCheck(m_hWndPsbAnm,       BST_CHECKED);
		Button_SetCheck(m_hWndPsbAll,       BST_CHECKED);
		Button_SetCheck(m_hWndPsbJson,      BST_CHECKED);
	}

	VOID OnCommandPsbSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		ULONG Value;

		// (1) Get the flags
		if (PsbSelectStatus(Value)) {
			m_PsbFlag = (KrkrPsbMode)Value;
		}
		else
		{
			PsbSelectRawOnly();
			m_PsbFlag = KrkrPsbMode::PSB_RAW;
		}

		// (2) Apply the button status
		if (m_PsbFlag == KrkrPsbMode::PSB_ALL) {
			PsbSelectAll();
		}
		else if (m_PsbFlag == KrkrPsbMode::PSB_NONE) {
			PsbSelectRawOnly();
		}
		else {
			// ignore, normal status
		}
	}

	VOID OnCommandTextRawSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTextRaw, BM_GETCHECK, 0, 0)) {
			m_TextFlag = KrkrTextMode::TEXT_RAW;
		}
	}

	VOID OnCommandTextPlainSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTextPlain, BM_GETCHECK, 0, 0)) {
			m_TextFlag = KrkrTextMode::TEXT_DECODE;
		}
	}

	VOID OnCommandTjs2RawSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTjsRaw, BM_GETCHECK, 0, 0)) {
			m_TjsFlag = KrkrTjs2Mode::TJS2_RAW;
		}
	}

	VOID OnCommandTjs2DisasmSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTjsDisasm, BM_GETCHECK, 0, 0)) {
			m_TjsFlag = KrkrTjs2Mode::TJS2_DEASM;
		}
	}

	VOID OnCommandTjs2DecompileSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTjsDecompile, BM_GETCHECK, 0, 0)) {
			m_TjsFlag = KrkrTjs2Mode::TJS2_DECOM;
		}
	}


	VOID OnCommandPbdRawSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndPbdRaw, BM_GETCHECK, 0, 0)) {
			m_PbdFlag = KrkrPbdMode::PBD_RAW;
		}
	}

	VOID OnCommandPbdJsonSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndPbdJson, BM_GETCHECK, 0, 0)) {
			m_PbdFlag = KrkrPbdMode::PBD_JSON;
		}
	}

	VOID OnCommandInheritIconSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		m_Icon = (BOOL)SendMessageW(m_hWndIcon, BM_GETCHECK, 0, 0);
	}

	VOID OnCommandProtectionSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		m_Protect = (BOOL)SendMessageW(m_hWndProtect, BM_GETCHECK, 0, 0);
	}

	VOID OnCommandTlgRawSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTlgRaw, BM_GETCHECK, 0, 0)) {
			m_TlgFlag = KrkrTlgMode::TLG_RAW;
		}
	}

	VOID OnCommandTlgJpgSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTlgJpg, BM_GETCHECK, 0, 0)) {
			m_TlgFlag = KrkrTlgMode::TLG_JPG;
		}
	}

	VOID OnCommandTlgPngSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTlgPng, BM_GETCHECK, 0, 0)) {
			m_TlgFlag = KrkrTlgMode::TLG_PNG;
		}
	}

	VOID OnCommandTlgSysSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTlgSys, BM_GETCHECK, 0, 0)) {
			m_TlgFlag = KrkrTlgMode::TLG_SYS;
		}
	}

	VOID OnCommandTlgBuildinSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (SendMessageW(m_hWndTlgBuildin, BM_GETCHECK, 0, 0)) {
			m_TlgFlag = KrkrTlgMode::TLG_BUILDIN;
		}
	}

	VOID OnCommandTaskCancelSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		m_ClientCallback->NotifyClientCancelTask();
	}

	VOID OnCommandUniversalDumperSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		m_ClientCallback->NotifyClientUniversalDumperModeChecked(
			m_PsbFlag,
			m_TextFlag,
			m_PngFlag,
			m_TjsFlag,
			m_TlgFlag,
			m_AmvFlag,
			m_PbdFlag
		);
	}

	VOID OnCommandMakeUniversalPatchSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		m_ClientCallback->NotifyClientUniversalPatchMakeChecked(m_Protect, m_Icon);
	}

	VOID OnCommandMakeArchiveSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		WCHAR OutputArchive  [MAX_NTPATH];
		WCHAR BaseDir        [MAX_NTPATH];
		WCHAR OriginalArchive[MAX_NTPATH];
		
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		Static_GetText(m_hWndOutputArchiveEdit,   OutputArchive,   countof(OutputArchive));
		Static_GetText(m_hWndBaseDirEdit,         BaseDir,         countof(BaseDir));
		Static_GetText(m_hWndOriginalArchiveEdit, OriginalArchive, countof(OriginalArchive));
		
		m_ClientCallback->NotifyClientPackerChecked(BaseDir, OriginalArchive, OutputArchive);
	}

	VOID OnCommandOpenOrCloseConsoleSelected(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		UNREFERENCED_PARAMETER(hWnd);
		UNREFERENCED_PARAMETER(ID);
		UNREFERENCED_PARAMETER(hWndCtl);
		UNREFERENCED_PARAMETER(codeNotify);

		if (m_ConsoleAttached)
		{
			FreeConsole();
			Static_SetText(m_hWndConsole, L"Open Console");
			m_ConsoleAttached = FALSE;
		}
		else
		{
			AllocConsole();
			Static_SetText(m_hWndConsole, L"Close Console");
			m_ConsoleAttached = TRUE;
		}
	}

	VOID OnStartDumper(PCWSTR PathName)
	{
		m_ClientCallback->NotifyClientTaskDumpStart(
			m_PsbFlag,
			m_TextFlag,
			m_PngFlag,
			m_TjsFlag,
			m_TlgFlag,
			m_AmvFlag,
			m_PbdFlag,
			PathName
		);
	}


	VOID OnCommand(HWND hWnd, INT ID, HWND hWndCtl, UINT codeNotify)
	{
		BOOL Checked;
		UNREFERENCED_PARAMETER(codeNotify);
		UNREFERENCED_PARAMETER(hWndCtl);

		switch (ID)
		{
		case IDCANCEL:
			OnClose(hWnd);
			break;

		case IDC_PACK_BUTTON_FOLDER:
			OnCommandBaseDirSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PACK_BUTTON_ORIPACK:
			OnCommandOriginalArchiveSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PACK_BUTTON_OUTPACK:
			OnCommandOutputArchiveSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PNG_RAW:
			OnCommandPngRawSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PNG_SYSTEM:
			OnCommandPngSystemSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PACK_BUTTON_UMAKE:
			OnCommandMakeUniversalPatchSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PACK_BUTTON_MAKE:
			break;

		case IDC_TLG_RAW:
			OnCommandTlgRawSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TLG_PNG:
			OnCommandTlgPngSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TLG_JPG:
			OnCommandTlgJpgSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TLG_SYSTEM:
			OnCommandTlgSysSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TLG_BUILDIN:
			OnCommandTlgBuildinSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_BUTTON_CANCEL:
			OnCommandTaskCancelSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_BUTTON_DEBUGGER:
			OnCommandOpenOrCloseConsoleSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_BUTTON_UDUMP:
			OnCommandUniversalDumperSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PSB_RAW:
		case IDC_PSB_SCRIPT:
		case IDC_PSB_TEXT:
		case IDC_PSB_ANIM:
		case IDC_PSB_IMAGE:
		case IDC_PSB_JSON:
		case IDC_PSB_ALL:
			OnCommandPsbSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TEXT_RAW:
			OnCommandTextRawSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TEXT_TEXT:
			OnCommandTextPlainSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TJS2_RAW:
			OnCommandTjs2RawSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TJS2_DISASM:
			OnCommandTjs2DisasmSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_TJS2_Decomp:
			OnCommandTjs2DecompileSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_RADIO_PBD_RAW:
			OnCommandPbdRawSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_RADIO_PBD_JSON:
			OnCommandPbdJsonSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PACK_SAME_ICON:
			OnCommandInheritIconSelected(hWnd, ID, hWndCtl, codeNotify);
			break;

		case IDC_PACK_ADD_PROTECT:
			OnCommandProtectionSelected(hWnd, ID, hWndCtl, codeNotify);
			break;
		}
	}


	VOID OnDropFiles(HWND hWnd, HDROP hDrop)
	{
		WCHAR Xp3Path[MAX_NTPATH];
		WCHAR ArchivePath[MAX_NTPATH];
		ULONG Index;
		ULONG Attribute;

		for (Index = 0; DragQueryFileW(hDrop, Index, Xp3Path, countof(Xp3Path)); ++Index) 
		{
			m_ViewerServer->NotifyServerLogOutput(LogLevel::LOG_INFO, Xp3Path);
		}

		LOOP_ONCE
		{
			if (Index != 1) {
				MessageBoxW(m_hWndMain, L"Please Drop one file on this window", L"XP3Extract", MB_OK);
				break;
			}

			Attribute =	Nt_GetFileAttributes(Xp3Path);
			if (Attribute == INVALID_FILE_ATTRIBUTES)
				break;

			if (Attribute & FILE_ATTRIBUTE_DIRECTORY)
			{
				RtlZeroMemory(ArchivePath, countof(ArchivePath));
				FormatStringW(ArchivePath, L"%s.xp3", Xp3Path);
				Static_SetText(m_hWndOutputArchiveEdit, ArchivePath);
				Static_SetText(m_hWndBaseDirEdit,       Xp3Path);
			}
			else {
				OnStartDumper(Xp3Path);
			}
		}
	}


	VOID OnSize(HWND hWnd, UINT State, INT cx, INT cy)
	{
		RECT ControlRect;
		INT  x, y, w, h;
		HDWP hDWP;

		if (m_LastWindowSize.cx == 0)
			return;

		if (State == SIZE_MINIMIZED)
			return;

		hDWP = BeginDeferWindowPos(1);
		if (hDWP == nullptr)
			return;

		auto&& ChildWindows = GetChildWindows();
		for (auto& Window : ChildWindows)
		{
			DeferWindowPos(hDWP, Window, 0, 0, 0, cx, cy, SWP_NOZORDER);
			EndDeferWindowPos(hDWP);
		}
	}

	VOID OnGetMinMaxInfo(HWND hWnd, LPMINMAXINFO MinMaxInfo)
	{
		MinMaxInfo->ptMinTrackSize = *(LPPOINT)&m_MinSize;
		MinMaxInfo->ptMaxTrackSize.x = INT_MAX;
		MinMaxInfo->ptMaxTrackSize.y = INT_MAX;
	}

	VOID OnLButtonUp(HWND hWnd, INT, INT, UINT)
	{
		SendMessageW(hWnd, WM_NCLBUTTONUP, HTCAPTION, 0);
	}

	VOID OnLButtonDown(HWND hWnd, BOOL, INT, INT, UINT)
	{
		SendMessageW(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}

	INT_PTR OnCtlColorDlg(HWND hWnd, HDC hdc, HWND, INT)
	{
		if (m_DarkModeSupported && m_DarkModeEnabled)
		{
			SetTextColor(hdc, DarkTextColor);
			SetBkColor(hdc, DarkBkColor);

			if (!m_NewBgBrush) {
				m_NewBgBrush = CreateSolidBrush(DarkBkColor);
			}

			return reinterpret_cast<INT_PTR>(m_NewBgBrush);
		}
		
		return reinterpret_cast<INT_PTR>(GetStockObject(NULL_BRUSH));
	}

	VOID OnSettingChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{
		if (m_DarkModeSupported && IsColorSchemeChangeMessage(lParam)) {
			SendMessageW(hWnd, WM_THEMECHANGED, 0, 0);
		}
	}

	VOID OnThemeChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
	{
		if (m_DarkModeSupported)
		{
			AllowDarkModeForWindow(hWnd, m_DarkModeEnabled);
			RefreshTitleBarThemeColor(hWnd);

			auto&& ChildWindows = GetChildWindows();
			for (auto& Window : ChildWindows)
			{
				AllowDarkModeForWindow(Window, m_DarkModeEnabled);
				SendMessageW(Window, WM_THEMECHANGED, 0, 0);
			}

			UpdateWindow(hWnd);
		}
	}

	INT_PTR DialogProcWorker(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
	{
		switch (Message)
		{
			HANDLE_MSG(hWnd, WM_COMMAND,         OnCommand);
			HANDLE_MSG(hWnd, WM_DROPFILES,       OnDropFiles);
			HANDLE_MSG(hWnd, WM_LBUTTONDOWN,     OnLButtonDown);
			HANDLE_MSG(hWnd, WM_LBUTTONUP,       OnLButtonUp);
			HANDLE_MSG(hWnd, WM_SIZE,            OnSize);
			HANDLE_MSG(hWnd, WM_GETMINMAXINFO,   OnGetMinMaxInfo);
			HANDLE_MSG(hWnd, WM_CLOSE,           OnClose);
			HANDLE_MSG(hWnd, WM_DESTROY,         OnDestroy);
			HANDLE_MSG(hWnd, WM_INITDIALOG,      OnInitDialog);
			HANDLE_MSG(hWnd, WM_CTLCOLORDLG,     OnCtlColorDlg);
			HANDLE_MSG(hWnd, WM_CTLCOLORSTATIC,  OnCtlColorDlg);
			HANDLE_MSG(hWnd, WM_CTLCOLORLISTBOX, OnCtlColorDlg);
			HANDLE_MSG(hWnd, WM_CTLCOLORMSGBOX,  OnCtlColorDlg);
			HANDLE_MSG(hWnd, WM_CTLCOLOREDIT,    OnCtlColorDlg);
			HANDLE_MSG(hWnd, WM_CTLCOLORBTN,     OnCtlColorDlg);
			HANDLE_MSG(hWnd, WM_CTLCOLORSCROLLBAR, OnCtlColorDlg);


		case WM_SETTINGCHANGE:
			OnSettingChange(hWnd, wParam, lParam);
			break;

		case WM_THEMECHANGED:
			OnSettingChange(hWnd, wParam, lParam);
			break;
		}

		return FALSE;
	}

	static INT_PTR CALLBACK DialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
	{
		UIViewer *This = (UIViewer *)GetWindowLongPtrW(hWnd, DWLP_USER);
		return This->DialogProcWorker(hWnd, Message, wParam, lParam);
	}

	static INT_PTR CALLBACK StartDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
	{
		if (Message != WM_INITDIALOG)
			return FALSE;

		SetWindowLongPtrW(hWnd, DWLP_USER, (LONG_PTR)lParam);
		SetWindowLongPtrW(hWnd, DWLP_DLGPROC, (LONG_PTR)DialogProc);

		UIViewer *This = (UIViewer *)lParam;
		This->m_hWndMain = hWnd;
		return This->DialogProcWorker(hWnd, Message, wParam, lParam);
	}

	static DWORD NTAPI DoModelInternal(PVOID Param)
	{
		UIViewer *This = (UIViewer *)Param;

		return DialogBoxParamW(
			This->GetInstance(),
			This->GetTemplateID(),
			NULL,
			StartDialogProc,
			(LPARAM)Param
		);
	}

	INT_PTR DoModel(HINSTANCE hInstance, LPWSTR lpTemplateID)
	{
		NTSTATUS Status;
		HRESULT  Success;

		if (m_Thread) {
			PrintConsoleW(L"UIViewer::DoModel : DoModel is already called\n");
			return -1;
		}

		Success = CoInitialize(nullptr);
		if (FAILED(Success)) {
			PrintConsoleW(L"UIViewer::DoModel : CoInitialize failed, %08x\n", Success);
			return (INT_PTR)-1;
		}

		InitDarkMode();

		m_Instance   = hInstance;
		m_TemplateID = lpTemplateID;

		Status = Nt_CreateThread(
			DoModelInternal,
			this,
			FALSE,
			NtCurrentProcess(),
			&m_Thread
		);

		if (NT_FAILED(Status)) 
		{
			PrintConsoleW(L"UIViewer::DoModel : Nt_CreateThread failed, %08x\n", Status);
			m_Thread = nullptr;
			return -1;
		}

		return 0;
	}

	

public:
	//
	// Local
	//
	
	BOOL NotifyUIProgressBar(PCWSTR TaskName, ULONGLONG Current, ULONGLONG Total);
	BOOL NotifyUILogOutput(LogLevel Level, PCWSTR Command, BOOL IsCmd);
	BOOL NotifyUIReady();
	BOOL NotifyUITaskError();
	BOOL NotifyUIMessageBox(PCWSTR Description, ULONG Flags, BOOL Locked);
	BOOL NotifyUIExitFromRemoteProcess();

	//
	// Helper
	//

	HWND GetMainWindow() { return m_hWndMain; };

	inline HINSTANCE GetInstance()   { return m_Instance; }
	inline LPWSTR    GetTemplateID() { return m_TemplateID; }

private:

	HINSTANCE m_Instance   = nullptr;
	LPWSTR    m_TemplateID = nullptr;
	HANDLE    m_Thread     = nullptr;

	//
	// Psb
	//

	HWND m_hWndMain         = nullptr;
	HWND m_hWndPsbRaw       = nullptr;
	HWND m_hWndPsbText      = nullptr;
	HWND m_hWndPsbImage     = nullptr;
	HWND m_hWndPsbAnm       = nullptr;
	HWND m_hWndPsbDecompile = nullptr;
	HWND m_hWndPsbJson      = nullptr;
	HWND m_hWndPsbAll       = nullptr;
	
	//
	// Text
	//

	HWND m_hWndTextRaw   = nullptr;
	HWND m_hWndTextPlain = nullptr;

	//
	// Universal patch
	//

	HWND m_hWndIcon           = nullptr;
	HWND m_hWndProtect        = nullptr;
	HWND m_hWndUniversalPatch = nullptr;

	//
	// Tlg
	//

	HWND m_hWndTlgRaw     = nullptr;
	HWND m_hWndTlgBuildin = nullptr;
	HWND m_hWndTlgSys     = nullptr;
	HWND m_hWndTlgPng     = nullptr;
	HWND m_hWndTlgJpg     = nullptr;

	//
	// Tjs
	//

	HWND m_hWndTjsRaw       = nullptr;
	HWND m_hWndTjsDisasm    = nullptr;
	HWND m_hWndTjsDecompile = nullptr;

	//
	// Alpha Movie
	//

	HWND m_hWndAmvRaw = nullptr;
	HWND m_hWndAmvPNG = nullptr;
	HWND m_hWndAmvJPG = nullptr;
	HWND m_hWndAmvGIF = nullptr;

	//
	// PNG
	//

	HWND m_hWndPngRaw = nullptr;
	HWND m_hWndPngSys = nullptr;
	
	//
	// Packer
	//

	HWND m_hWndBaseDirEdit         = nullptr;
	HWND m_hWndOriginalArchiveEdit = nullptr;
	HWND m_hWndOutputArchiveEdit   = nullptr;

	HWND m_hWndBaseDirSelect         = nullptr;
	HWND m_hWndOriginalArchiveSelect = nullptr;
	HWND m_hWndOutputArchiveSelect   = nullptr;

	HWND m_hWndMakePack = nullptr;

	//
	// Misc
	//
	
	HWND m_hWndProgressBar = nullptr;
	HWND m_hWndConsole     = nullptr;
	HWND m_hWndCancelTask  = nullptr;

	//
	// Pbd
	//

	HWND m_hWndPbdRaw  = nullptr;
	HWND m_hWndPbdJson = nullptr;

	//
	// Universal dumper
	//

	HWND m_hWndUniversalDump  = nullptr;
	HWND m_hWndVirtualConsole = nullptr;

	//
	// Flags
	//

	KrkrPngMode         m_PngFlag = KrkrPngMode::PNG_RAW;
	KrkrTlgMode         m_TlgFlag = KrkrTlgMode::TLG_RAW;
	KrkrTextMode        m_TextFlag = KrkrTextMode::TEXT_RAW;
	KrkrPsbMode         m_PsbFlag = KrkrPsbMode::PSB_RAW;
	KrkrTjs2Mode        m_TjsFlag = KrkrTjs2Mode::TJS2_RAW;
	KrkrAmvMode         m_AmvFlag = KrkrAmvMode::AMV_RAW;
	KrkrPbdMode         m_PbdFlag = KrkrPbdMode::PBD_RAW;

	BOOL                m_Icon    = FALSE;
	BOOL                m_Protect = FALSE;

	UIViewerServer*     m_ViewerServer     = nullptr;
	ClientStub*         m_ClientCallback   = nullptr;
	PVOID               m_KrkrModule       = nullptr;
	PVOID               m_CurrentModule    = nullptr;
	ITaskbarList3*      m_Taskbar          = nullptr;
	BOOL                m_ConsoleAttached  = FALSE;
	SIZE                m_LastWindowSize;
	SIZE                m_MinSize;

	std::wstring        m_Version;
};


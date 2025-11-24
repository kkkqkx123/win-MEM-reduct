#include "app_init.h"
#include "icon_manager.h"

VOID _app_hotkeyinit (
	_In_ HWND hwnd
)
{
	LONG hk;

	UnregisterHotKey (hwnd, UID);

	if (!_r_config_getboolean (L"HotkeyCleanEnable", FALSE, NULL))
		return;

	hk = _r_config_getlong (L"HotkeyClean", MAKEWORD (VK_F1, HOTKEYF_CONTROL), NULL);

	if (!hk)
		return;

	if (!RegisterHotKey (hwnd, UID, (HIBYTE (hk) & 2) | ((HIBYTE (hk) & 4) >> 2) | ((HIBYTE (hk) & 1) << 2), LOBYTE (hk)))
		_r_show_errormessage (hwnd, NULL, NtLastError (), NULL, ET_WINDOWS);
}

VOID _app_setfontcontrol (
	_In_ HWND hwnd,
	_In_ INT ctrl_id,
	_In_ PLOGFONT logfont,
	_In_ LONG dpi_value
)
{
	_r_ctrl_setstringformat (
		hwnd,
		IDC_FONT,
		L"%s, %" TEXT (PR_LONG) L"px, %" TEXT (PR_LONG),
		logfont->lfFaceName,
		_r_dc_fontheighttosize (logfont->lfHeight, dpi_value),
		logfont->lfWeight
	);
}

VOID _app_resizecolumns (
	_In_ HWND hwnd
)
{
	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 0, NULL, -50);
	_r_listview_setcolumn (hwnd, IDC_LISTVIEW, 1, NULL, -50);
}

VOID _app_initialize (
	_In_opt_ HWND hwnd
)
{
	ULONG privileges[] = {
		SE_PROF_SINGLE_PROCESS_PRIVILEGE,
		SE_INCREASE_QUOTA_PRIVILEGE,
	};

	LONG dpi_value;

	if (_r_sys_iselevated ())
	{
		_r_sys_setprocessprivilege (NtCurrentProcess (), privileges, RTL_NUMBER_OF (privileges), TRUE);
	}
	else
	{
		if (hwnd)
			_r_ctrl_setbuttonshield (hwnd, IDC_CLEAN, TRUE);
	}

	if (!hwnd)
		return;

	dpi_value = _r_dc_getwindowdpi (hwnd);

	_r_ctrl_setbuttonmargins (hwnd, IDC_CLEAN, dpi_value);

	// configure listview
	_r_listview_setstyle (
		hwnd,
		IDC_LISTVIEW,
		LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP,
		TRUE
	);

	_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 1, NULL, 10, LVCFMT_RIGHT);
	_r_listview_addcolumn (hwnd, IDC_LISTVIEW, 2, NULL, 10, LVCFMT_LEFT);

	// configure listview
	for (INT i = 0, k = 0; i < 3; i++)
	{
		_r_listview_addgroup (hwnd, IDC_LISTVIEW, i, _r_locale_getstring (IDS_GROUP_1 + i), 0, LVGS_COLLAPSIBLE, LVGS_COLLAPSIBLE);

		for (INT j = 0; j < 3; j++)
		{
			_r_listview_additem (hwnd, IDC_LISTVIEW, k++, _r_locale_getstring (IDS_ITEM_1 + j), I_DEFAULT, i, 0);
		}
	}

	// settings
	_r_settings_addpage (IDD_SETTINGS_GENERAL, IDS_SETTINGS_GENERAL);
	_r_settings_addpage (IDD_SETTINGS_MEMORY, IDS_SETTINGS_MEMORY);
	_r_settings_addpage (IDD_SETTINGS_APPEARANCE, IDS_SETTINGS_APPEARANCE);
	_r_settings_addpage (IDD_SETTINGS_TRAY, IDS_SETTINGS_TRAY);
	_r_settings_addpage (IDD_SETTINGS_ADVANCED, IDS_TITLE_ADVANCED);

	_app_resizecolumns (hwnd);
}
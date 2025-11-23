// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#include "icon_manager.h"
#include "config_utils.h"
#include "ui_utils.h"

// External global config reference (required for icon management)
extern STATIC_DATA config;

HICON _app_iconcreate (
	_In_opt_ ULONG percent
)
{
	static HICON hicon = NULL;

	R_MEMORY_INFO mem_info;
	R_STRINGREF sr;
	ICONINFO ii = {0};
	WCHAR icon_text[8];
	HGDIOBJ prev_font;
	HGDIOBJ prev_bmp;
	HICON hicon_new;
	COLORREF text_color;
	COLORREF bg_color;
	LONG prev_mode;
	BOOLEAN is_transparent;
	BOOLEAN is_border;
	BOOLEAN is_round;
	BOOLEAN has_warning;
	BOOLEAN has_danger;

	text_color = _r_config_getulong (L"TrayColorText", TRAY_COLOR_TEXT, NULL);
	bg_color = _r_config_getulong (L"TrayColorBg", TRAY_COLOR_BG, NULL);

	is_transparent = _r_config_getboolean (L"TrayUseTransparency", FALSE, NULL);
	is_border = _r_config_getboolean (L"TrayShowBorder", FALSE, NULL);
	is_round = _r_config_getboolean (L"TrayRoundCorners", FALSE, NULL);

	if (!percent)
	{
		_app_getmemoryinfo (&mem_info);

		percent = mem_info.physical_memory.percent;
	}

	has_danger = percent >= _app_getdangervalue ();
	has_warning = !has_danger && percent >= _app_getwarningvalue ();

	if (has_danger || has_warning)
	{
		if (_r_config_getboolean (L"TrayChangeBg", TRUE, NULL))
		{
			if (has_danger)
			{
				bg_color = _r_config_getulong (L"TrayColorDanger", TRAY_COLOR_DANGER, NULL);
			}
			else
			{
				bg_color = _r_config_getulong (L"TrayColorWarning", TRAY_COLOR_WARNING, NULL);
			}

			is_transparent = FALSE;
		}
		else
		{
			if (has_danger)
			{
				text_color = _r_config_getulong (L"TrayColorDanger", TRAY_COLOR_DANGER, NULL);
			}
			else
			{
				text_color = _r_config_getulong (L"TrayColorWarning", TRAY_COLOR_WARNING, NULL);
			}
		}
	}

	// set tray text
	_r_str_fromulong (icon_text, RTL_NUMBER_OF (icon_text), percent);

	_r_obj_initializestringref (&sr, icon_text);

	// draw main device context
	prev_bmp = SelectObject (config.hdc, config.hbitmap);
	prev_font = SelectObject (config.hdc, config.hfont);
	prev_mode = SetBkMode (config.hdc, TRANSPARENT);

	_app_drawbackground (config.hdc, bg_color, is_border ? text_color : bg_color, is_transparent ? text_color : bg_color, &config.icon_size, is_round);

	_r_dc_drawtext (NULL, config.hdc, &sr, &config.icon_size, 0, 0, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX, text_color);

	SetBkMode (config.hdc, prev_mode);

	SelectObject (config.hdc, prev_font);
	SelectObject (config.hdc, prev_bmp);

	// draw mask device context
	prev_bmp = SelectObject (config.hdc_mask, config.hbitmap_mask);
	prev_font = SelectObject (config.hdc_mask, config.hfont);
	prev_mode = SetBkMode (config.hdc_mask, TRANSPARENT);

	_app_drawbackground (
		config.hdc_mask,
		TRAY_COLOR_WHITE,
		is_border ? TRAY_COLOR_BLACK : TRAY_COLOR_WHITE,
		is_transparent ? TRAY_COLOR_WHITE : TRAY_COLOR_BLACK,
		&config.icon_size,
		is_round
	);

	_r_dc_drawtext (NULL, config.hdc_mask, &sr, &config.icon_size, 0, 0, DT_VCENTER | DT_CENTER | DT_SINGLELINE | DT_NOCLIP | DT_NOPREFIX, TRAY_COLOR_BLACK);

	SetBkMode (config.hdc, prev_mode);

	SelectObject (config.hdc_mask, prev_bmp);
	SelectObject (config.hdc_mask, prev_font);

	// create icon
	ii.fIcon = TRUE;
	ii.hbmColor = config.hbitmap;
	ii.hbmMask = config.hbitmap_mask;

	hicon_new = CreateIconIndirect (&ii);

	if (hicon)
		DestroyIcon (hicon);

	hicon = hicon_new;

	return hicon;
}

VOID _app_iconredraw (
	_In_opt_ HWND hwnd
)
{
	config.ms_prev = 0;

	if (hwnd)
		_app_timercallback (hwnd, 0, UID, 0);
}

VOID _app_iconinit (
	_In_ LONG dpi_value
)
{
	LOGFONT logfont;
	PVOID bits;
	HDC hdc;

	SAFE_DELETE_OBJECT (config.hbitmap_mask);
	SAFE_DELETE_OBJECT (config.hbitmap);
	SAFE_DELETE_OBJECT (config.hfont);

	SAFE_DELETE_DC (config.hdc_mask);
	SAFE_DELETE_DC (config.hdc);

	// init font
	_app_fontinit (&logfont, dpi_value);

	config.hfont = CreateFontIndirectW (&logfont);

	// init rect
	SetRect (&config.icon_size, 0, 0, _r_dc_getsystemmetrics (SM_CXSMICON, dpi_value), _r_dc_getsystemmetrics (SM_CYSMICON, dpi_value));

	// init dc
	hdc = GetDC (NULL);

	if (!hdc)
		return;

	config.hdc = CreateCompatibleDC (hdc);
	config.hdc_mask = CreateCompatibleDC (hdc);

	// init bitmap
	config.hbitmap = _r_dc_createbitmap (hdc, config.icon_size.right, config.icon_size.bottom, &bits);
	config.hbitmap_mask = CreateBitmap (config.icon_size.right, config.icon_size.bottom, 1, 1, NULL);

	ReleaseDC (NULL, hdc);
}
// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#include "routine.h"
#include "main.h"
#include "rapp.h"
#include "resource.h"
#include "constants.h"
#include "utils.h"
#include "settings_dialog.h"
#include "memory_cleaner.h"
#include "icon_manager.h"
#include "app_init.h"
#include "config_utils.h"
#include "ui_utils.h"
#include "cmdline_parser.h"
#include "tray_manager.h"

STATIC_DATA config = {0};

ULONG_PTR limits_arr[13] = {0};
ULONG_PTR intervals_arr[13] = {0};

// Configuration utility functions are now in config_utils.c

VOID _app_memoryclean (
	_In_ HWND hwnd,
	_In_ CLEANUP_SOURCE_ENUM src,
	_In_ ULONG mask
)
{
	MEMORY_COMBINE_INFORMATION_EX combine_info_ex = {0};
	SYSTEM_MEMORY_LIST_COMMAND command;
	SYSTEM_FILECACHE_INFORMATION sfci = {0};
	ULONG reduct_before;
	ULONG reduct_after;
	WCHAR buffer1[128];
	WCHAR buffer2[256];
	NTSTATUS status;
	ULONG flags;
	MEMORYSTATUSEX mem_info;

	if (!mask)
		mask = _app_getcleanupmask ();

	flags = _r_config_getboolean (L"BalloonCleanResults", TRUE, NULL) ? NIIF_INFO : 0;

	mem_info.dwLength = sizeof (mem_info);

	SetCursor (LoadCursorW (NULL, IDC_WAIT));

	// difference (before)
	reduct_before = _app_getmemoryinfo (&mem_info);

	// Working set (vista+)
	if ((mask & REDUCT_WORKING_SET) == REDUCT_WORKING_SET)
	{
		command = MemoryEmptyWorkingSets;

		status = NtSetSystemInformation (SystemMemoryListInformation, &command, sizeof (SYSTEM_MEMORY_LIST_COMMAND));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"MemoryEmptyWorkingSets");
	}

	// System file cache
	if ((mask & REDUCT_SYSTEM_FILE_CACHE) == REDUCT_SYSTEM_FILE_CACHE)
	{
		sfci.MinimumWorkingSet = MAXSIZE_T;
		sfci.MaximumWorkingSet = MAXSIZE_T;

		status = NtSetSystemInformation (SystemFileCacheInformationEx, &sfci, sizeof (SYSTEM_FILECACHE_INFORMATION));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"SystemFileCacheInformation");
	}

	// Modified page list (vista+)
	if ((mask & REDUCT_MODIFIED_LIST) == REDUCT_MODIFIED_LIST)
	{
		command = MemoryFlushModifiedList;

		status = NtSetSystemInformation (SystemMemoryListInformation, &command, sizeof (SYSTEM_MEMORY_LIST_COMMAND));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"MemoryFlushModifiedList");
	}

	// Standby list (vista+)
	if ((mask & REDUCT_STANDBY_LIST) == REDUCT_STANDBY_LIST)
	{
		command = MemoryPurgeStandbyList;

		status = NtSetSystemInformation (SystemMemoryListInformation, &command, sizeof (SYSTEM_MEMORY_LIST_COMMAND));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"MemoryPurgeStandbyList");
	}

	// Standby priority-0 list (vista+)
	if ((mask & REDUCT_STANDBY_PRIORITY0_LIST) == REDUCT_STANDBY_PRIORITY0_LIST)
	{
		command = MemoryPurgeLowPriorityStandbyList;

		status = NtSetSystemInformation (SystemMemoryListInformation, &command, sizeof (SYSTEM_MEMORY_LIST_COMMAND));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"MemoryPurgeLowPriorityStandbyList");
	}

	// Flush volume cache
	if ((mask & REDUCT_MODIFIED_FILE_CACHE) == REDUCT_MODIFIED_FILE_CACHE)
		_app_flushvolumecache ();

	// Flush registry cache (win8.1+)
	if (_r_sys_isosversiongreaterorequal (WINDOWS_8_1))
	{
		if ((mask & REDUCT_REGISTRY_CACHE) == REDUCT_REGISTRY_CACHE)
		{
			status = NtSetSystemInformation (SystemRegistryReconciliationInformation, NULL, 0);

			if (!NT_SUCCESS (status))
				_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"SystemRegistryReconciliationInformation");
		}
	}

	// Combine memory lists (win10+)
	if (_r_sys_isosversiongreaterorequal (WINDOWS_10))
	{
		if ((mask & REDUCT_COMBINE_MEMORY_LISTS) == REDUCT_COMBINE_MEMORY_LISTS)
		{
			status = NtSetSystemInformation (SystemCombinePhysicalMemoryInformation, &combine_info_ex, sizeof (MEMORY_COMBINE_INFORMATION_EX));

			if (!NT_SUCCESS (status))
				_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"SystemCombinePhysicalMemoryInformation");
		}
	}

	// WSL cache clean
	if ((mask & REDUCT_WSL_CACHE_CLEAN) == REDUCT_WSL_CACHE_CLEAN)
	{
		WSL_CLEANUP_RESULT wsl_result = _app_wsl_cleanup_cache ();
		
		if (wsl_result != WSL_CLEANUP_SUCCESS)
		{
			LPCWSTR error_text = _app_get_wsl_error_text (wsl_result);
			_r_log (LOG_LEVEL_ERROR, NULL, L"_app_wsl_cleanup_cache", wsl_result, error_text);
		}
	}

	// WSL memory reclaim
	if ((mask & REDUCT_WSL_MEMORY_RECLAIM) == REDUCT_WSL_MEMORY_RECLAIM)
	{
		WSL_CLEANUP_RESULT wsl_result = _app_wsl_reclaim_memory ();
		
		if (wsl_result != WSL_CLEANUP_SUCCESS)
		{
			LPCWSTR error_text = _app_get_wsl_error_text (wsl_result);
			_r_log (LOG_LEVEL_ERROR, NULL, L"_app_wsl_reclaim_memory", wsl_result, error_text);
		}
	}

	SetCursor (LoadCursorW (NULL, IDC_ARROW));

	// difference (after)
	reduct_after = _app_getmemoryinfo (&mem_info);

	if (reduct_after < reduct_before)
	{
		reduct_after = (reduct_before - reduct_after);
	}
	else
	{
		reduct_after = 0;
	}

	// time of last cleaning
	_r_config_setlong64 (L"StatisticLastReduct", _r_unixtime_now (), NULL);

	_r_format_bytesize64 (buffer1, RTL_NUMBER_OF (buffer1), reduct_after);

	_r_str_printf (buffer2, RTL_NUMBER_OF (buffer2), _r_locale_getstring (IDS_STATUS_CLEANED), buffer1);

	if (src == SOURCE_CMDLINE)
	{
		_r_show_message (hwnd, MB_OK | MB_ICONINFORMATION, NULL, buffer2);
	}
	else
	{
		if (hwnd && _r_config_getboolean (L"BalloonCleanResults", TRUE, NULL))
			_r_tray_popup (hwnd, &GUID_TrayIcon, flags, _r_app_getname (), buffer2);
	}

	if (_r_config_getboolean (L"LogCleanResults", FALSE, NULL))
		_r_log_v (LOG_LEVEL_INFO, 0, _app_getcleanupreason (src), 0, buffer1);
}

// Font initialization function is now in ui_utils.c

// Background drawing function is now in ui_utils.c

// Icon creation function is now in icon_manager.c

// Timer callback function is now in icon_manager.c

// Icon redraw function is now in icon_manager.c

// Icon initialization function is now in icon_manager.c

INT_PTR CALLBACK SettingsProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
);

INT_PTR CALLBACK DlgProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			_r_app_sethwnd (hwnd); // HACK!!!

			_app_initialize (hwnd);

			SetTimer (hwnd, UID, TIMER, &_app_timercallback);

			break;
		}

		case WM_DESTROY:
		{
			KillTimer (hwnd, UID);

			_r_tray_destroy (hwnd, &GUID_TrayIcon);

			PostQuitMessage (0);

			break;
		}

		case RM_INITIALIZE:
		{
			HMENU hmenu;
			LONG dpi_value;

			hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_checkitem (hmenu, IDM_ALWAYSONTOP_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"AlwaysOnTop", FALSE, NULL));
				_r_menu_checkitem (hmenu, IDM_USEDARKTHEME, 0, MF_BYCOMMAND, _r_theme_isenabled ());
				_r_menu_checkitem (hmenu, IDM_LOADONSTARTUP_CHK, 0, MF_BYCOMMAND, _r_autorun_isenabled ());
				_r_menu_checkitem (hmenu, IDM_STARTMINIMIZED_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"IsStartMinimized", FALSE, NULL));
				_r_menu_checkitem (hmenu, IDM_REDUCTCONFIRMATION_CHK, 0, MF_BYCOMMAND, _r_config_getboolean (L"IsShowReductConfirmation", TRUE, NULL));
				_r_menu_checkitem (hmenu, IDM_SKIPUACWARNING_CHK, 0, MF_BYCOMMAND, _r_skipuac_isenabled ());
				_r_menu_checkitem (hmenu, IDM_CHECKUPDATES_CHK, 0, MF_BYCOMMAND, _r_update_isenabled (FALSE));

				if (!_r_sys_iselevated ())
					_r_menu_enableitem (hmenu, IDM_SKIPUACWARNING_CHK, FALSE, FALSE);
			}

			dpi_value = _r_dc_gettaskbardpi ();

			_app_iconinit (dpi_value);

			_r_tray_create (hwnd, &GUID_TrayIcon, RM_TRAYICON, _app_iconcreate (0), _r_app_getname (), FALSE);

			_app_iconredraw (hwnd);

			break;
		}

		case RM_INITIALIZE_POST:
		{
			if (_r_sys_iselevated ())
				_app_hotkeyinit (hwnd);

			break;
		}

		case RM_TASKBARCREATED:
		{
			LONG dpi_value;

			dpi_value = _r_dc_gettaskbardpi ();

			_app_iconinit (dpi_value);

			_r_tray_create (hwnd, &GUID_TrayIcon, RM_TRAYICON, _app_iconcreate (0), _r_app_getname (), FALSE);

			_app_iconredraw (hwnd);

			break;
		}

		case RM_LOCALIZE:
		{
			// localize menu
			HMENU hmenu;

			hmenu = GetMenu (hwnd);

			if (hmenu)
			{
				_r_menu_setitemtext (hmenu, 0, TRUE, _r_locale_getstring (IDS_FILE));
				_r_menu_setitemtext (hmenu, 1, TRUE, _r_locale_getstring (IDS_VIEW));
				_r_menu_setitemtext (hmenu, 2, TRUE, _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtext (hmenu, 3, TRUE, _r_locale_getstring (IDS_HELP));

				_r_menu_setitemtextformat (hmenu, IDM_SETTINGS, FALSE, L"%s...\tF2", _r_locale_getstring (IDS_SETTINGS));
				_r_menu_setitemtext (hmenu, IDM_EXIT, FALSE, _r_locale_getstring (IDS_EXIT));
				_r_menu_setitemtext (hmenu, IDM_ALWAYSONTOP_CHK, FALSE, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
				_r_menu_setitemtext (hmenu, IDM_LOADONSTARTUP_CHK, FALSE, _r_locale_getstring (IDS_LOADONSTARTUP_CHK));
				_r_menu_setitemtext (hmenu, IDM_USEDARKTHEME, FALSE, _r_locale_getstring (IDS_USEDARKTHEME));
				_r_menu_setitemtext (hmenu, IDM_STARTMINIMIZED_CHK, FALSE, _r_locale_getstring (IDS_STARTMINIMIZED_CHK));
				_r_menu_setitemtext (hmenu, IDM_REDUCTCONFIRMATION_CHK, FALSE, _r_locale_getstring (IDS_REDUCTCONFIRMATION_CHK));
				_r_menu_setitemtext (hmenu, IDM_SKIPUACWARNING_CHK, FALSE, _r_locale_getstring (IDS_SKIPUACWARNING_CHK));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES_CHK, FALSE, _r_locale_getstring (IDS_CHECKUPDATES_CHK));
				_r_menu_setitemtextformat (GetSubMenu (hmenu, LANG_SUBMENU), LANG_MENU, TRUE, L"%s (Language)", _r_locale_getstring (IDS_LANGUAGE));
				_r_menu_setitemtext (hmenu, IDM_WEBSITE, FALSE, _r_locale_getstring (IDS_WEBSITE));
				_r_menu_setitemtext (hmenu, IDM_CHECKUPDATES, FALSE, _r_locale_getstring (IDS_CHECKUPDATES));
				_r_menu_setitemtextformat (hmenu, IDM_ABOUT, FALSE, L"%s\tF1", _r_locale_getstring (IDS_ABOUT));
			}

			// configure listview
			for (INT i = 0, k = 0; i < 3; i++)
			{
				_r_listview_setgroup (hwnd, IDC_LISTVIEW, i, _r_locale_getstring (IDS_GROUP_1 + i), 0, 0);

				for (INT j = 0; j < 3; j++)
				{
					_r_listview_setitem (hwnd, IDC_LISTVIEW, k++, 0, _r_locale_getstring (IDS_ITEM_1 + j), I_DEFAULT, I_DEFAULT, I_DEFAULT);
				}
			}

			// configure button
			_r_ctrl_setstring (hwnd, IDC_CLEAN, _r_locale_getstring (IDS_CLEAN));

			// enum localizations
			if (hmenu)
				_r_locale_enum (GetSubMenu (hmenu, LANG_SUBMENU), LANG_MENU, IDX_LANGUAGE);

			break;
		}

		case WM_DPICHANGED:
		{
			LONG dpi_value;

			dpi_value = _r_dc_gettaskbardpi ();

			_app_iconinit (dpi_value);
			_app_iconredraw (hwnd);

			_app_resizecolumns (hwnd);

			if (!_r_sys_iselevated ())
			{
				dpi_value = LOWORD (wparam);

				_r_ctrl_setbuttonmargins (hwnd, IDC_CLEAN, dpi_value);
			}

			break;
		}

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;

			hdc = BeginPaint (hwnd, &ps);

			if (!hdc)
				break;

			_r_dc_drawwindow (hdc, hwnd, TRUE);

			EndPaint (hwnd, &ps);

			break;
		}

		case WM_HOTKEY:
		{
			if (wparam == UID)
				_app_memoryclean (hwnd, SOURCE_HOTKEY, 0);

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR nmlp;

			nmlp = (LPNMHDR)lparam;

			switch (nmlp->code)
			{
				case BCN_DROPDOWN:
				{
					R_RECTANGLE rectangle;
					RECT rect;
					HMENU hsubmenu;

					hsubmenu = CreatePopupMenu ();

					if (!hsubmenu)
						break;

					_r_menu_additem (hsubmenu, IDM_CLEAN_WORKINGSET, _r_locale_getstring (IDS_WORKINGSET_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_SYSTEMFILECACHE, _r_locale_getstring (IDS_SYSTEMFILECACHE_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_MODIFIEDFILECACHE, _r_locale_getstring (IDS_MODIFIEDFILECACHE_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_MODIFIEDLIST, _r_locale_getstring (IDS_MODIFIEDLIST_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_STANDBYLIST, _r_locale_getstring (IDS_STANDBYLIST_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_STANDBYLISTPRIORITY0, _r_locale_getstring (IDS_STANDBYLISTPRIORITY0_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_REGISTRYCACHE, _r_locale_getstring (IDS_REGISTRYCACHE_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_COMBINEMEMORYLISTS, _r_locale_getstring (IDS_COMBINEMEMORYLISTS_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_WSL_CACHE, _r_locale_getstring (IDS_WSL_CACHE_CLEAN_CHK));
					_r_menu_additem (hsubmenu, IDM_CLEAN_WSL_MEMORY, _r_locale_getstring (IDS_WSL_MEMORY_RECLAIM_CHK));

					if (_r_sys_isosversionlower (WINDOWS_8_1))
						_r_menu_enableitem (hsubmenu, IDM_CLEAN_REGISTRYCACHE, FALSE, FALSE);

					if (_r_sys_isosversionlower (WINDOWS_10))
						_r_menu_enableitem (hsubmenu, IDM_CLEAN_COMBINEMEMORYLISTS, FALSE, FALSE);

					if (GetClientRect (nmlp->hwndFrom, &rect))
					{
						POINT pt;
						
						ClientToScreen (nmlp->hwndFrom, (PPOINT)&rect);

						_r_wnd_recttorectangle (&rectangle, &rect);
						_r_wnd_adjustrectangletoworkingarea (nmlp->hwndFrom, &rectangle);
						_r_wnd_rectangletorect (&rect, &rectangle);

						pt.x = rect.left;
						pt.y = rect.top;
						_r_menu_popup (hsubmenu, hwnd, &pt, TRUE);
					}

					DestroyMenu (hsubmenu);

					break;
				}

				case NM_CUSTOMDRAW:
				{
					LPNMLVCUSTOMDRAW lpnmlv;
					LONG_PTR result;
					ULONG value;

					lpnmlv = (LPNMLVCUSTOMDRAW)lparam;
					result = CDRF_DODEFAULT;

					if (nmlp->idFrom != IDC_LISTVIEW)
						break;

					switch (lpnmlv->nmcd.dwDrawStage)
					{
						case CDDS_PREPAINT:
						{
							result = (CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW);
							break;
						}

						case CDDS_ITEMPREPAINT:
						{
							value = (ULONG)lpnmlv->nmcd.lItemlParam;

							if (value >= _app_getdangervalue ())
							{
								lpnmlv->clrText = _r_config_getulong (L"TrayColorDanger", TRAY_COLOR_DANGER, NULL);

								result = (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
							}
							else if (value >= _app_getwarningvalue ())
							{
								lpnmlv->clrText = _r_config_getulong (L"TrayColorWarning", TRAY_COLOR_WARNING, NULL);

								result = (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
							}

							break;
						}
					}

					SetWindowLongPtrW (hwnd, DWLP_MSGRESULT, result);

					return result;
				}
			}

			break;
		}

		case RM_TRAYICON:
		{
			switch (LOWORD (lparam))
			{
				case NIN_KEYSELECT:
				{
					if (GetForegroundWindow () != hwnd)
						_r_wnd_toggle (hwnd, TRUE);

					break;
				}

				case WM_LBUTTONDOWN:
				case WM_MBUTTONDOWN:
				{
					LONG action;

					if (LOWORD (lparam) == WM_MBUTTONDOWN)
					{
						action = _r_config_getlong (L"TrayActionMc", 1, NULL);
					}
					else
					{
						action = _r_config_getlong (L"TrayActionDc", 0, NULL);
					}

					switch (action)
					{
						case 1:
						{
							_app_memoryclean (hwnd, SOURCE_MANUAL, 0);
							break;
						}

						case 2:
						{
							_r_sys_createprocess (L"taskmgr.exe", NULL, NULL, FALSE);
							break;
						}

						default:
						{
							_r_wnd_toggle (hwnd, FALSE);
							break;
						}
					}

					SetForegroundWindow (hwnd);

					break;
				}

				case WM_CONTEXTMENU:
				{
					_app_tray_menu_create (hwnd);
					break;
				}
			}

			break;
		}

		case WM_COMMAND:
		{
			INT ctrl_id = LOWORD (wparam);
			INT notify_code = HIWORD (wparam);

			if (notify_code == 0 && ctrl_id >= IDX_LANGUAGE && ctrl_id <= IDX_LANGUAGE + (INT)(INT_PTR)_r_locale_getcount () + 1)
			{
				HMENU hmenu;
				HMENU hsubmenu;

				hmenu = GetMenu (hwnd);

				if (hmenu)
				{
					hsubmenu = GetSubMenu (GetSubMenu (hmenu, LANG_SUBMENU), LANG_MENU);

					if (hsubmenu)
						_r_locale_apply (hsubmenu, ctrl_id, IDX_LANGUAGE);
				}

				return FALSE;
			}
			else if ((ctrl_id >= IDX_TRAY_POPUP_1 && ctrl_id <= IDX_TRAY_POPUP_1 + (INT)RTL_NUMBER_OF (limits_arr) - 1))
			{
				ULONG_PTR idx;

				idx = (ULONG_PTR)ctrl_id - IDX_TRAY_POPUP_1;

				_r_config_setboolean (L"AutoreductEnable", TRUE, NULL);
			_r_config_setlong (L"AutoreductValue", (LONG)limits_arr[idx], NULL);

				return FALSE;
			}
			else if ((ctrl_id >= IDX_TRAY_POPUP_2 && ctrl_id <= IDX_TRAY_POPUP_2 + (INT)RTL_NUMBER_OF (intervals_arr) - 1))
			{
				ULONG_PTR idx;

				idx = (ULONG_PTR)ctrl_id - IDX_TRAY_POPUP_2;

				_r_config_setboolean (L"AutoreductIntervalEnable", TRUE, NULL);
			_r_config_setlong (L"AutoreductIntervalValue", (LONG)intervals_arr[idx], NULL);

				return FALSE;
			}

			switch (ctrl_id)
			{
				case IDM_ALWAYSONTOP_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_config_getboolean (L"AlwaysOnTop", FALSE, NULL);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"AlwaysOnTop", new_val, NULL);

					_r_wnd_top (hwnd, new_val);

					break;
				}

				case IDM_STARTMINIMIZED_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_config_getboolean (L"IsStartMinimized", FALSE, NULL);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"IsStartMinimized", new_val, NULL);

					break;
				}

				case IDM_REDUCTCONFIRMATION_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_config_getboolean (L"IsShowReductConfirmation", TRUE, NULL);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_config_setboolean (L"IsShowReductConfirmation", new_val, NULL);

					break;
				}

				case IDM_LOADONSTARTUP_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_autorun_isenabled ();

					_r_autorun_enable (hwnd, new_val);
					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, _r_autorun_isenabled ());

					break;
				}

				case IDM_USEDARKTHEME:
				{
					BOOLEAN is_enabled = !_r_theme_isenabled ();

					_r_theme_enable (hwnd, is_enabled);
					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, is_enabled);

					break;
				}

				case IDM_SKIPUACWARNING_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_skipuac_isenabled ();

					_r_skipuac_enable (hwnd, new_val);
					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, _r_skipuac_isenabled ());

					break;
				}

				case IDM_CHECKUPDATES_CHK:
				{
					BOOLEAN new_val;

					new_val = !_r_update_isenabled (FALSE);

					_r_menu_checkitem (GetMenu (hwnd), ctrl_id, 0, MF_BYCOMMAND, new_val);
					_r_update_enable (new_val);

					break;
				}

				case IDM_WORKINGSET_CHK:
				case IDM_SYSTEMFILECACHE_CHK:
				case IDM_MODIFIEDFILECACHE_CHK:
				case IDM_MODIFIEDLIST_CHK:
				case IDM_STANDBYLIST_CHK:
				case IDM_STANDBYLISTPRIORITY0_CHK:
				case IDM_REGISTRYCACHE_CHK:
				case IDM_COMBINEMEMORYLISTS_CHK:
				{
					ULONG new_mask = 0;
					ULONG mask;

					mask = _r_config_getulong (L"ReductMask2", REDUCT_MASK_DEFAULT, NULL);

					switch (ctrl_id)
					{
						case IDM_WORKINGSET_CHK:
						{
							new_mask = REDUCT_WORKING_SET;
							break;
						}

						case IDM_SYSTEMFILECACHE_CHK:
						{
							new_mask = REDUCT_SYSTEM_FILE_CACHE;
							break;
						}

						case IDM_MODIFIEDFILECACHE_CHK:
						{
							new_mask = REDUCT_MODIFIED_FILE_CACHE;
							break;
						}

						case IDM_MODIFIEDLIST_CHK:
						{
							new_mask = REDUCT_MODIFIED_LIST;
							break;
						}

						case IDM_STANDBYLIST_CHK:
						{
							new_mask = REDUCT_STANDBY_LIST;
							break;
						}

						case IDM_STANDBYLISTPRIORITY0_CHK:
						{
							new_mask = REDUCT_STANDBY_PRIORITY0_LIST;
							break;
						}

						case IDM_REGISTRYCACHE_CHK:
						{
							new_mask = REDUCT_REGISTRY_CACHE;
							break;
						}

						case IDM_COMBINEMEMORYLISTS_CHK:
					{
						new_mask = REDUCT_COMBINE_MEMORY_LISTS;
						break;
					}

					case IDM_WSL_CACHE_CLEAN_CHK:
					{
						new_mask = REDUCT_WSL_CACHE_CLEAN;
						break;
					}

					case IDM_WSL_MEMORY_RECLAIM_CHK:
					{
						new_mask = REDUCT_WSL_MEMORY_RECLAIM;
						break;
					}

					default:
					{
						return FALSE;
					}
					}

					if ((ctrl_id == IDM_STANDBYLIST_CHK && !(mask & REDUCT_STANDBY_LIST)) || (ctrl_id == IDM_MODIFIEDLIST_CHK && !(mask & REDUCT_MODIFIED_LIST)))
					{
						if (!_r_show_confirmmessage (hwnd, _r_locale_getstring (IDS_QUESTION_WARNING), NULL, L"IsShowWarningConfirmation", FALSE))
							return FALSE;
					}

					_r_config_setulong (L"ReductMask2", (mask & new_mask) != 0 ? (mask & ~new_mask) : (mask | new_mask), NULL);

					break;
				}

				case IDM_CLEAN_WORKINGSET:
				case IDM_CLEAN_SYSTEMFILECACHE:
				case IDM_CLEAN_MODIFIEDFILECACHE:
				case IDM_CLEAN_MODIFIEDLIST:
				case IDM_CLEAN_STANDBYLIST:
				case IDM_CLEAN_STANDBYLISTPRIORITY0:
				case IDM_CLEAN_REGISTRYCACHE:
				case IDM_CLEAN_COMBINEMEMORYLISTS:
				{
					ULONG mask;

					switch (ctrl_id)
					{
						case IDM_CLEAN_WORKINGSET:
						{
							mask = REDUCT_WORKING_SET;
							break;
						}

						case IDM_CLEAN_SYSTEMFILECACHE:
						{
							mask = REDUCT_SYSTEM_FILE_CACHE;
							break;
						}

						case IDM_CLEAN_MODIFIEDFILECACHE:
						{
							mask = REDUCT_MODIFIED_FILE_CACHE;
							break;
						}

						case IDM_CLEAN_MODIFIEDLIST:
						{
							mask = REDUCT_MODIFIED_LIST;
							break;
						}

						case IDM_CLEAN_STANDBYLIST:
						{
							mask = REDUCT_STANDBY_LIST;
							break;
						}

						case IDM_CLEAN_STANDBYLISTPRIORITY0:
						{
							mask = REDUCT_STANDBY_PRIORITY0_LIST;
							break;
						}

						case IDM_CLEAN_REGISTRYCACHE:
						{
							mask = REDUCT_REGISTRY_CACHE;
							break;
						}

						case IDM_CLEAN_COMBINEMEMORYLISTS:
			{
				mask = REDUCT_COMBINE_MEMORY_LISTS;
				break;
			}

			case IDM_CLEAN_WSL_CACHE:
			{
				mask = REDUCT_WSL_CACHE_CLEAN;
				break;
			}

			case IDM_CLEAN_WSL_MEMORY:
			{
				mask = REDUCT_WSL_MEMORY_RECLAIM;
				break;
			}

			default:
			{
				return FALSE;
			}
		}

		_app_memoryclean (hwnd, SOURCE_CMDLINE, mask);

					break;
				}

				case IDM_TRAY_DISABLE_1:
				{
					BOOLEAN new_val;

					new_val = !_r_config_getboolean (L"AutoreductEnable", FALSE, NULL);

					_r_config_setboolean (L"AutoreductEnable", new_val, NULL);

					break;
				}

				case IDM_TRAY_DISABLE_2:
				{
					BOOLEAN new_val;

					new_val = !_r_config_getboolean (L"AutoreductIntervalEnable", FALSE, NULL);

					_r_config_setboolean (L"AutoreductIntervalEnable", new_val, NULL);

					break;
				}

				case IDM_SETTINGS:
				case IDM_TRAY_SETTINGS:
				{
					_r_settings_createwindow (hwnd, &SettingsProc, 0);
					break;
				}

				case IDM_EXIT:
				case IDM_TRAY_EXIT:
				{
					DestroyWindow (hwnd);
					break;
				}

				case IDCANCEL: // process Esc key
				case IDM_TRAY_SHOW:
				{
					_r_wnd_toggle (hwnd, FALSE);
					break;
				}

				case IDOK: // process Enter key
				case IDC_CLEAN:
				case IDM_TRAY_CLEAN:
				{
					if (_r_sys_iselevated ())
					{
						_app_memoryclean (hwnd, SOURCE_MANUAL, 0);
					}
					else
					{
						if (_r_app_runasadmin ())
						{
							DestroyWindow (hwnd);
						}
						else
						{
							_r_show_message (hwnd, MB_OK | MB_ICONSTOP, NULL, (LPCWSTR)_r_locale_getstring (IDS_STATUS_NOPRIVILEGES));
						}
					}

					break;
				}

				case IDM_WEBSITE:
				case IDM_TRAY_WEBSITE:
				{
					_r_shell_opendefault (_r_app_getwebsite_url ());
					break;
				}

				case IDM_CHECKUPDATES:
				{
					_r_update_check (hwnd);
					break;
				}

				case IDM_ABOUT:
				case IDM_TRAY_ABOUT:
				{
					_r_show_aboutmessage (hwnd);
					break;
				}
			}

			break;
		}
	}

	return FALSE;
}

// Timer callback function (implementation)
VOID CALLBACK _app_timercallback (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ UINT_PTR id_event,
_In_ ULONG time
)
{
	// Timer callback implementation
	_app_iconredraw (hwnd);
}

// Command line parsing function is now in cmdline_parser.c

ULONG _app_getcleanupmask ()
{
	ULONG mask = 0;

	if (_r_config_getboolean (L"ReductWorkingSet", TRUE, NULL))
		mask |= REDUCT_WORKING_SET;

	if (_r_config_getboolean (L"ReductSystemFileCache", TRUE, NULL))
		mask |= REDUCT_SYSTEM_FILE_CACHE;

	if (_r_config_getboolean (L"ReductModifiedList", FALSE, NULL))
		mask |= REDUCT_MODIFIED_LIST;

	if (_r_config_getboolean (L"ReductStandbyList", FALSE, NULL))
		mask |= REDUCT_STANDBY_LIST;

	if (_r_config_getboolean (L"ReductStandbyPriority0List", TRUE, NULL))
		mask |= REDUCT_STANDBY_PRIORITY0_LIST;

	if (_r_config_getboolean (L"ReductRegistryCache", TRUE, NULL))
		mask |= REDUCT_REGISTRY_CACHE;

	if (_r_config_getboolean (L"ReductCombineMemoryLists", TRUE, NULL))
		mask |= REDUCT_COMBINE_MEMORY_LISTS;

	if (_r_config_getboolean (L"ReductModifiedFileCache", TRUE, NULL))
		mask |= REDUCT_MODIFIED_FILE_CACHE;

	if (_r_config_getboolean (L"ReductWslCacheClean", FALSE, NULL))
		mask |= REDUCT_WSL_CACHE_CLEAN;

	if (_r_config_getboolean (L"ReductWslMemoryReclaim", FALSE, NULL))
		mask |= REDUCT_WSL_MEMORY_RECLAIM;

	return mask;
}

INT APIENTRY wWinMain (
	_In_ HINSTANCE hinst,
	_In_opt_ HINSTANCE prev_hinst,
	_In_ LPWSTR cmdline,
	_In_ INT show_cmd
)
{
	HWND hwnd;

	if (!_r_app_initialize (&_app_parseargs))
		return ERROR_APP_INIT_FAILURE;

	hwnd = _r_app_createwindow (hinst, MAKEINTRESOURCEW (IDD_MAIN), MAKEINTRESOURCEW (IDI_MAIN), &DlgProc);

	if (!hwnd)
		return ERROR_APP_INIT_FAILURE;

	return _r_wnd_message_callback (hwnd, MAKEINTRESOURCEW (IDA_MAIN));
}

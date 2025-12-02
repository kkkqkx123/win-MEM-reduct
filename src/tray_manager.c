#include "tray_manager.h"
#include "settings_dialog.h"

VOID _app_tray_create (
	_In_ HWND hwnd
)
{
	_r_tray_create (hwnd, &GUID_TrayIcon, RM_TRAYICON, _app_iconcreate (0), _r_app_getname (), FALSE);
}

VOID _app_tray_destroy (
	_In_ HWND hwnd
)
{
	_r_tray_destroy (hwnd, &GUID_TrayIcon);
}

VOID _app_tray_popup (
	_In_ HWND hwnd,
	_In_ LPCWSTR title,
	_In_ LPCWSTR text
)
{
	_r_tray_popup (hwnd, &GUID_TrayIcon, NIIF_INFO | NIIF_NOSOUND, title, text);
}

VOID _app_tray_menu_create (
	_In_ HWND hwnd
)
{
	HMENU hmenu;
	HMENU htraymenu;
	HMENU hsubmenu;
	POINT pt;

	// Load the tray menu resource
	hmenu = LoadMenu (_r_sys_getimagebase (), MAKEINTRESOURCE (IDM_TRAY));
	if (!hmenu)
		return;

	// Get the submenu (first popup)
	htraymenu = GetSubMenu (hmenu, 0);
	if (!htraymenu)
	{
		DestroyMenu (hmenu);
		return;
	}

	// Localize tray menu items
	_r_menu_setitemtext (htraymenu, IDM_TRAY_SHOW, FALSE, _r_locale_getstring (IDS_TRAY_SHOW));
	_r_menu_setitemtext (htraymenu, IDM_TRAY_CLEAN, FALSE, _r_locale_getstring (IDS_TRAY_CLEAN));
	_r_menu_setitemtext (htraymenu, IDM_TRAY_SETTINGS, FALSE, _r_locale_getstring (IDS_TRAY_SETTINGS));
	_r_menu_setitemtext (htraymenu, IDM_TRAY_WEBSITE, FALSE, _r_locale_getstring (IDS_TRAY_WEBSITE));
	_r_menu_setitemtext (htraymenu, IDM_TRAY_ABOUT, FALSE, _r_locale_getstring (IDS_TRAY_ABOUT));
	_r_menu_setitemtext (htraymenu, IDM_TRAY_EXIT, FALSE, _r_locale_getstring (IDS_TRAY_EXIT));

	// Localize and generate memory cleaning submenus
	hsubmenu = GetSubMenu (htraymenu, 0); // First popup submenu (memory cleaning)
	if (hsubmenu)
	{
		// 修复：通过父菜单设置第一个POPUP子菜单的标题（索引0：第一个POPUP子菜单在父菜单中的位置）
		_r_menu_setitemtext (htraymenu, 0, TRUE, _r_locale_getstring (IDS_TRAY_POPUP_1)); // 清理区域
		
		// Set memory cleaning items
		_r_menu_setitemtext (hsubmenu, IDM_WORKINGSET_CHK, FALSE, _r_locale_getstring (IDS_WORKINGSET_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_SYSTEMFILECACHE_CHK, FALSE, _r_locale_getstring (IDS_SYSTEMFILECACHE_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_MODIFIEDFILECACHE_CHK, FALSE, _r_locale_getstring (IDS_MODIFIEDFILECACHE_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_STANDBYLISTPRIORITY0_CHK, FALSE, _r_locale_getstring (IDS_STANDBYLISTPRIORITY0_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_STANDBYLIST_CHK, FALSE, _r_locale_getstring (IDS_STANDBYLIST_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_MODIFIEDLIST_CHK, FALSE, _r_locale_getstring (IDS_MODIFIEDLIST_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_REGISTRYCACHE_CHK, FALSE, _r_locale_getstring (IDS_REGISTRYCACHE_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_COMBINEMEMORYLISTS_CHK, FALSE, _r_locale_getstring (IDS_COMBINEMEMORYLISTS_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_WSL_CACHE_CLEAN_CHK, FALSE, _r_locale_getstring (IDS_WSL_CACHE_CLEAN_CHK));
		_r_menu_setitemtext (hsubmenu, IDM_WSL_MEMORY_RECLAIM_CHK, FALSE, _r_locale_getstring (IDS_WSL_MEMORY_RECLAIM_CHK));
	}

	// Localize and generate autoreduct submenus
	hsubmenu = GetSubMenu (htraymenu, 1); // Second popup submenu (autoreduct disable)
	if (hsubmenu)
	{
		// 修复：通过父菜单设置第二个POPUP子菜单的标题（索引1：第二个POPUP子菜单在父菜单中的位置）
		_r_menu_setitemtext (htraymenu, 1, TRUE, _r_locale_getstring (IDS_TRAY_POPUP_2)); // 清理界限
		// 生成自动清理间隔时间菜单项
		ULONG_PTR local_limits_arr[LIMITS_ARRAY_SIZE] = {0};
		_app_generate_array (local_limits_arr, RTL_NUMBER_OF (local_limits_arr), _r_config_getlong (L"AutoreductValue", DEFAULT_AUTOREDUCT_VAL, NULL));
		// 使用局部数组并生成菜单项
		_app_generate_menu (hsubmenu, IDX_TRAY_POPUP_1, local_limits_arr, RTL_NUMBER_OF (local_limits_arr), L"%lu%%", 
			_r_config_getlong (L"AutoreductValue", DEFAULT_AUTOREDUCT_VAL, NULL), _r_config_getboolean (L"AutoreductEnable", FALSE, NULL));
	}

	hsubmenu = GetSubMenu (htraymenu, 2); // Third popup submenu (interval disable)
	if (hsubmenu)
	{
		// 修复：通过父菜单设置第三个POPUP子菜单的标题（索引2：第三个POPUP子菜单在父菜单中的位置）
		_r_menu_setitemtext (htraymenu, 2, TRUE, _r_locale_getstring (IDS_TRAY_POPUP_3)); // 清理间隔
		// 生成自动清理时间间隔菜单项
		ULONG_PTR local_intervals_arr[INTERVALS_ARRAY_SIZE] = {0};
		_app_generate_array (local_intervals_arr, RTL_NUMBER_OF (local_intervals_arr), _r_config_getlong (L"AutoreductIntervalValue", DEFAULT_AUTOREDUCTINTERVAL_VAL, NULL));
		// 使用局部数组并生成菜单项
		_app_generate_menu (hsubmenu, IDX_TRAY_POPUP_2, local_intervals_arr, RTL_NUMBER_OF (local_intervals_arr), L"%lu min", 
			_r_config_getlong (L"AutoreductIntervalValue", DEFAULT_AUTOREDUCTINTERVAL_VAL, NULL), _r_config_getboolean (L"AutoreductIntervalEnable", FALSE, NULL));
	}

	// 修复：在正确的子菜单中设置IDM_TRAY_DISABLE_1和IDM_TRAY_DISABLE_2菜单项
	// 获取清理界限子菜单
	hsubmenu = GetSubMenu (htraymenu, 1);
	if (hsubmenu)
	{
		_r_menu_setitemtext (hsubmenu, IDM_TRAY_DISABLE_1, FALSE, _r_locale_getstring (IDS_TRAY_DISABLE));
	}
	
	// 获取清理间隔子菜单
	hsubmenu = GetSubMenu (htraymenu, 2);
	if (hsubmenu)
	{
		_r_menu_setitemtext (hsubmenu, IDM_TRAY_DISABLE_2, FALSE, _r_locale_getstring (IDS_TRAY_DISABLE));
	}

	// Get current cursor position for menu display
	GetCursorPos (&pt);

	// Set foreground window to ensure proper menu behavior
	SetForegroundWindow (hwnd);

	// Display the tray menu
	TrackPopupMenu (htraymenu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);

	// Destroy the menu
	DestroyMenu (hmenu);
}

VOID _app_tray_menu_handle (
	_In_ HWND hwnd,
	_In_ INT ctrl_id,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
)
{
	ULONG mask;
	BOOLEAN new_val;

	switch (ctrl_id)
	{
		case IDM_CLEAN_WORKINGSET:
		case IDM_CLEAN_SYSTEMFILECACHE:
		case IDM_CLEAN_MODIFIEDFILECACHE:
		case IDM_CLEAN_MODIFIEDLIST:
		case IDM_CLEAN_STANDBYLIST:
		case IDM_CLEAN_STANDBYLISTPRIORITY0:
		case IDM_CLEAN_REGISTRYCACHE:
		case IDM_CLEAN_COMBINEMEMORYLISTS:
		case IDM_CLEAN_WSL_CACHE:
		case IDM_CLEAN_WSL_MEMORY:
		{
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
					return;
				}
			}

			_app_memoryclean (hwnd, SOURCE_CMDLINE, mask);
			break;
		}

		case IDM_TRAY_DISABLE_1:
		{
			new_val = !_r_config_getboolean (L"AutoreductEnable", FALSE, NULL);
			_r_config_setboolean (L"AutoreductEnable", new_val, NULL);
			break;
		}

		case IDM_TRAY_DISABLE_2:
		{
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
}
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
	// This function is called from WM_CONTEXTMENU handler
	// The actual menu creation logic is in the WM_CONTEXTMENU case
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
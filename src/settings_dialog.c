#include "settings_dialog.h"

INT_PTR CALLBACK SettingsProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
)
{
	switch (msg)
	{
		case RM_INITIALIZE:
		{
			INT dialog_id;

			dialog_id = (INT)wparam;

			switch (dialog_id)
			{
				case IDD_SETTINGS_GENERAL:
				{
					_r_ctrl_checkbutton (hwnd, IDC_ALWAYSONTOP_CHK, _r_config_getboolean (L"AlwaysOnTop", FALSE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_LOADONSTARTUP_CHK, _r_autorun_isenabled ());
					_r_ctrl_checkbutton (hwnd, IDC_STARTMINIMIZED_CHK, _r_config_getboolean (L"IsStartMinimized", FALSE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_REDUCTCONFIRMATION_CHK, _r_config_getboolean (L"IsShowReductConfirmation", TRUE, NULL));

					if (!_r_sys_iselevated ())
						_r_ctrl_enable (hwnd, IDC_SKIPUACWARNING_CHK, FALSE);

					_r_ctrl_checkbutton (hwnd, IDC_SKIPUACWARNING_CHK, _r_skipuac_isenabled ());
					_r_ctrl_checkbutton (hwnd, IDC_CHECKUPDATES_CHK, _r_update_isenabled (FALSE));

					_r_locale_enum (hwnd, IDC_LANGUAGE, 0);

					break;
				}

				case IDD_SETTINGS_MEMORY:
				{
					ULONG mask;

					_r_listview_setstyle (hwnd, IDC_REGIONS, LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP, FALSE);

					_r_wnd_setcontext (hwnd, IDC_REGIONS, INVALID_HANDLE_VALUE);

					_r_listview_addcolumn (hwnd, IDC_REGIONS, 0, L"", 10, LVCFMT_LEFT);

					_r_listview_additem (hwnd, IDC_REGIONS, 0, _r_locale_getstring (IDS_WORKINGSET_CHK), I_DEFAULT, I_DEFAULT, REDUCT_WORKING_SET);
					_r_listview_additem (hwnd, IDC_REGIONS, 1, _r_locale_getstring (IDS_SYSTEMFILECACHE_CHK), I_DEFAULT, I_DEFAULT, REDUCT_SYSTEM_FILE_CACHE);
					_r_listview_additem (hwnd, IDC_REGIONS, 2, _r_locale_getstring (IDS_MODIFIEDLIST_CHK), I_DEFAULT, I_DEFAULT, REDUCT_MODIFIED_LIST);
					_r_listview_additem (hwnd, IDC_REGIONS, 3, _r_locale_getstring (IDS_STANDBYLIST_CHK), I_DEFAULT, I_DEFAULT, REDUCT_STANDBY_LIST);
					_r_listview_additem (hwnd, IDC_REGIONS, 4, _r_locale_getstring (IDS_STANDBYLISTPRIORITY0_CHK), I_DEFAULT, I_DEFAULT, REDUCT_STANDBY_PRIORITY0_LIST);
					_r_listview_additem (hwnd, IDC_REGIONS, 5, _r_locale_getstring (IDS_MODIFIEDFILECACHE_CHK), I_DEFAULT, I_DEFAULT, REDUCT_MODIFIED_FILE_CACHE);
					_r_listview_additem (hwnd, IDC_REGIONS, 6, _r_locale_getstring (IDS_REGISTRYCACHE_CHK), I_DEFAULT, I_DEFAULT, REDUCT_REGISTRY_CACHE);
					_r_listview_additem (hwnd, IDC_REGIONS, 7, _r_locale_getstring (IDS_COMBINEMEMORYLISTS_CHK), I_DEFAULT, I_DEFAULT, REDUCT_COMBINE_MEMORY_LISTS);

					_r_listview_setcolumn (hwnd, IDC_REGIONS, 0, NULL, -100);

					mask = _r_config_getulong (L"ReductMask2", REDUCT_MASK_DEFAULT, NULL);

					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 0, (mask & REDUCT_WORKING_SET) == REDUCT_WORKING_SET);
					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 1, (mask & REDUCT_SYSTEM_FILE_CACHE) == REDUCT_SYSTEM_FILE_CACHE);
					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 2, (mask & REDUCT_MODIFIED_LIST) == REDUCT_MODIFIED_LIST);
					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 3, (mask & REDUCT_STANDBY_LIST) == REDUCT_STANDBY_LIST);
					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 4, (mask & REDUCT_STANDBY_PRIORITY0_LIST) == REDUCT_STANDBY_PRIORITY0_LIST);
					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 5, (mask & REDUCT_MODIFIED_FILE_CACHE) == REDUCT_MODIFIED_FILE_CACHE);
					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 6, (mask & REDUCT_REGISTRY_CACHE) == REDUCT_REGISTRY_CACHE);
					_r_listview_setitemcheck (hwnd, IDC_REGIONS, 7, (mask & REDUCT_COMBINE_MEMORY_LISTS) == REDUCT_COMBINE_MEMORY_LISTS);

					_r_wnd_removecontext (hwnd, IDC_REGIONS);

					if (!_r_sys_iselevated ())
					{
						_r_ctrl_enable (hwnd, IDC_AUTOREDUCTENABLE_CHK, FALSE);
						_r_ctrl_enable (hwnd, IDC_AUTOREDUCTINTERVALENABLE_CHK, FALSE);
						_r_ctrl_enable (hwnd, IDC_HOTKEY_CLEAN_CHK, FALSE);
						_r_ctrl_enable (hwnd, IDC_HOTKEY_CLEAN, FALSE);
					}

					_r_ctrl_checkbutton (hwnd, IDC_AUTOREDUCTENABLE_CHK, _r_config_getboolean (L"AutoreductEnable", FALSE, NULL));

					_r_updown_setrange (hwnd, IDC_AUTOREDUCTVALUE, 1, 99);

					_r_updown_setvalue (hwnd, IDC_AUTOREDUCTVALUE, _app_getlimitvalue ());

					_r_ctrl_checkbutton (hwnd, IDC_AUTOREDUCTINTERVALENABLE_CHK, _r_config_getboolean (L"AutoreductIntervalEnable", FALSE, NULL));

					_r_updown_setrange (hwnd, IDC_AUTOREDUCTINTERVALVALUE, 1, 1440);

					_r_updown_setvalue (hwnd, IDC_AUTOREDUCTINTERVALVALUE, _app_getintervalvalue ());

					_r_ctrl_checkbutton (hwnd, IDC_HOTKEY_CLEAN_CHK, _r_config_getboolean (L"HotkeyCleanEnable", FALSE, NULL));

					if (!_r_ctrl_isbuttonchecked (hwnd, IDC_HOTKEY_CLEAN_CHK))
						_r_ctrl_enable (hwnd, IDC_HOTKEY_CLEAN, FALSE);

					_r_hotkey_set (hwnd, IDC_HOTKEY_CLEAN, _r_config_getlong (L"HotkeyClean", MAKEWORD (VK_F1, HOTKEYF_CONTROL), NULL));

					_r_ctrl_sendcommand (hwnd, IDC_AUTOREDUCTENABLE_CHK, 0);
					_r_ctrl_sendcommand (hwnd, IDC_AUTOREDUCTINTERVALENABLE_CHK, 0);
					_r_ctrl_sendcommand (hwnd, IDC_HOTKEY_CLEAN_CHK, 0);

					break;
				}

				case IDD_SETTINGS_APPEARANCE:
				{
					LOGFONT logfont;
					LONG dpi_value;

					_r_ctrl_checkbutton (hwnd, IDC_TRAYUSETRANSPARENCY_CHK, _r_config_getboolean (L"TrayUseTransparency", FALSE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_TRAYSHOWBORDER_CHK, _r_config_getboolean (L"TrayShowBorder", FALSE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_TRAYROUNDCORNERS_CHK, _r_config_getboolean (L"TrayRoundCorners", FALSE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_TRAYCHANGEBG_CHK, _r_config_getboolean (L"TrayChangeBg", TRUE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_TRAYUSEANTIALIASING_CHK, _r_config_getboolean (L"TrayUseAntialiasing", FALSE, NULL));

					dpi_value = _r_dc_gettaskbardpi ();

					_app_fontinit (&logfont, dpi_value);
					_app_setfontcontrol (hwnd, IDC_FONT, &logfont, dpi_value);

					_r_listview_setstyle (hwnd, IDC_COLORS, LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP | LVS_EX_LABELTIP, FALSE);

					_r_listview_addcolumn (hwnd, IDC_COLORS, 0, L"", -100, LVCFMT_LEFT);

					_r_listview_additem (
						hwnd,
						IDC_COLORS,
						0,
						_r_locale_getstring (IDS_COLOR_TEXT_HINT),
						I_DEFAULT,
						I_DEFAULT,
						_r_config_getulong (L"TrayColorText", TRAY_COLOR_TEXT, NULL)
					);

					_r_listview_additem (
						hwnd,
						IDC_COLORS,
						1,
						_r_locale_getstring (IDS_COLOR_BACKGROUND_HINT),
						I_DEFAULT,
						I_DEFAULT,
						_r_config_getulong (L"TrayColorBg", TRAY_COLOR_BG, NULL)
					);

					_r_listview_additem (
						hwnd,
						IDC_COLORS,
						2,
						_r_locale_getstring (IDS_COLOR_WARNING_HINT),
						I_DEFAULT,
						I_DEFAULT,
						_r_config_getulong (L"TrayColorWarning", TRAY_COLOR_WARNING, NULL)
					);

					_r_listview_additem (
						hwnd,
						IDC_COLORS,
						3,
						_r_locale_getstring (IDS_COLOR_DANGER_HINT),
						I_DEFAULT,
						I_DEFAULT,
						_r_config_getulong (L"TrayColorDanger", TRAY_COLOR_DANGER, NULL)
					);

					break;
				}

				case IDD_SETTINGS_TRAY:
				{
					_r_updown_setrange (hwnd, IDC_TRAYLEVELWARNING, 1, 99);
					_r_updown_setvalue (hwnd, IDC_TRAYLEVELWARNING, _app_getwarningvalue ());

					_r_updown_setrange (hwnd, IDC_TRAYLEVELDANGER, 1, 99);
					_r_updown_setvalue (hwnd, IDC_TRAYLEVELDANGER, _app_getdangervalue ());

					_r_combobox_setcurrentitem (hwnd, IDC_TRAYACTIONSC, _r_config_getlong (L"TrayActionDc", 0, NULL));
					_r_combobox_setcurrentitem (hwnd, IDC_TRAYACTIONMC, _r_config_getlong (L"TrayActionMc", 1, NULL));

					_r_ctrl_checkbutton (hwnd, IDC_SHOW_CLEAN_RESULT_CHK, _r_config_getboolean (L"BalloonCleanResults", TRUE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_NOTIFICATIONSOUND_CHK, _r_config_getboolean (L"IsNotificationsSound", TRUE, NULL));

					break;
				}

				case IDD_SETTINGS_ADVANCED:
				{
					_r_ctrl_checkbutton (hwnd, IDC_ALLOWSTANDBYLISTCLEANUP_CHK, _r_config_getboolean (L"IsAllowStandbyListCleanup", FALSE, NULL));
					_r_ctrl_checkbutton (hwnd, IDC_LOGRESULTS_CHK, _r_config_getboolean (L"LogCleanResults", FALSE, NULL));

					break;
				}
			}

			break;
		}

		case RM_LOCALIZE:
		{
			INT dialog_id;

			dialog_id = (INT)wparam;

			// localize titles
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_1, L"%s:", _r_locale_getstring (IDS_TITLE_1));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_2, L"%s: (Language)", _r_locale_getstring (IDS_TITLE_2));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_3, L"%s:", _r_locale_getstring (IDS_TITLE_3));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_4, L"%s:", _r_locale_getstring (IDS_TITLE_4));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_5, L"%s:", _r_locale_getstring (IDS_TITLE_5));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_6, L"%s:", _r_locale_getstring (IDS_TITLE_6));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_7, L"%s:", _r_locale_getstring (IDS_TITLE_7));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_8, L"%s:", _r_locale_getstring (IDS_TITLE_8));
			_r_ctrl_setstringformat (hwnd, IDC_TITLE_9, L"%s:", _r_locale_getstring (IDS_TITLE_9));

			switch (dialog_id)
			{
				case IDD_SETTINGS_GENERAL:
				{
					_r_ctrl_setstring (hwnd, IDC_ALWAYSONTOP_CHK, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
					_r_ctrl_setstring (hwnd, IDC_LOADONSTARTUP_CHK, _r_locale_getstring (IDS_LOADONSTARTUP_CHK));
					_r_ctrl_setstring (hwnd, IDC_STARTMINIMIZED_CHK, _r_locale_getstring (IDS_STARTMINIMIZED_CHK));
					_r_ctrl_setstring (hwnd, IDC_REDUCTCONFIRMATION_CHK, _r_locale_getstring (IDS_REDUCTCONFIRMATION_CHK));
					_r_ctrl_setstring (hwnd, IDC_SKIPUACWARNING_CHK, _r_locale_getstring (IDS_SKIPUACWARNING_CHK));
					_r_ctrl_setstring (hwnd, IDC_CHECKUPDATES_CHK, _r_locale_getstring (IDS_CHECKUPDATES_CHK));
					_r_ctrl_setstring (hwnd, IDC_LANGUAGE_HINT, _r_locale_getstring (IDS_LANGUAGE_HINT));

					break;
				}

				case IDD_SETTINGS_MEMORY:
				{
					_r_ctrl_setstring (hwnd, IDC_AUTOREDUCTENABLE_CHK, _r_locale_getstring (IDS_AUTOREDUCTENABLE_CHK));
					_r_ctrl_setstring (hwnd, IDC_AUTOREDUCTINTERVALENABLE_CHK, _r_locale_getstring (IDS_AUTOREDUCTINTERVALENABLE_CHK));

					_r_ctrl_setstring (hwnd, IDC_HOTKEY_CLEAN_CHK, _r_locale_getstring (IDS_HOTKEY_CLEAN_CHK));

					break;
				}

				case IDD_SETTINGS_APPEARANCE:
				{
					_r_ctrl_setstring (hwnd, IDC_TRAYUSETRANSPARENCY_CHK, _r_locale_getstring (IDS_TRAYUSETRANSPARENCY_CHK));
					_r_ctrl_setstring (hwnd, IDC_TRAYSHOWBORDER_CHK, _r_locale_getstring (IDS_TRAYSHOWBORDER_CHK));
					_r_ctrl_setstring (hwnd, IDC_TRAYROUNDCORNERS_CHK, _r_locale_getstring (IDS_TRAYROUNDCORNERS_CHK));
					_r_ctrl_setstring (hwnd, IDC_TRAYCHANGEBG_CHK, _r_locale_getstring (IDS_TRAYCHANGEBG_CHK));
					_r_ctrl_setstring (hwnd, IDC_TRAYUSEANTIALIASING_CHK, _r_locale_getstring (IDS_TRAYUSEANTIALIASING_CHK));

					_r_ctrl_setstring (hwnd, IDC_FONT_HINT, _r_locale_getstring (IDS_FONT_HINT));

					_r_listview_setitem (hwnd, IDC_COLORS, 0, 0, _r_locale_getstring (IDS_COLOR_TEXT_HINT), I_DEFAULT, I_DEFAULT, I_DEFAULT);
					_r_listview_setitem (hwnd, IDC_COLORS, 1, 0, _r_locale_getstring (IDS_COLOR_BACKGROUND_HINT), I_DEFAULT, I_DEFAULT, I_DEFAULT);
					_r_listview_setitem (hwnd, IDC_COLORS, 2, 0, _r_locale_getstring (IDS_COLOR_WARNING_HINT), I_DEFAULT, I_DEFAULT, I_DEFAULT);
					_r_listview_setitem (hwnd, IDC_COLORS, 3, 0, _r_locale_getstring (IDS_COLOR_DANGER_HINT), I_DEFAULT, I_DEFAULT, I_DEFAULT);

					break;
				}

				case IDD_SETTINGS_TRAY:
				{
					LPCWSTR string;

					_r_ctrl_setstring (hwnd, IDC_TRAYLEVELWARNING_HINT, _r_locale_getstring (IDS_TRAYLEVELWARNING_HINT));
					_r_ctrl_setstring (hwnd, IDC_TRAYLEVELDANGER_HINT, _r_locale_getstring (IDS_TRAYLEVELDANGER_HINT));

					_r_ctrl_setstring (hwnd, IDC_TRAYACTIONSC_HINT, _r_locale_getstring (IDS_TRAYACTIONSC_HINT));
					_r_ctrl_setstring (hwnd, IDC_TRAYACTIONMC_HINT, _r_locale_getstring (IDS_TRAYACTIONMC_HINT));

					_r_combobox_clear (hwnd, IDC_TRAYACTIONSC);
					_r_combobox_clear (hwnd, IDC_TRAYACTIONMC);

					for (INT i = 0; i < 3; i++)
					{
						string = _r_locale_getstring (IDS_TRAY_ACTION_1 + i);

						_r_combobox_insertitem (hwnd, IDC_TRAYACTIONSC, i, string, i);
						_r_combobox_insertitem (hwnd, IDC_TRAYACTIONMC, i, string, i);
					}

					_r_combobox_setcurrentitembylparam (hwnd, IDC_TRAYACTIONSC, _r_config_getlong (L"TrayActionDc", 0, NULL));
					_r_combobox_setcurrentitembylparam (hwnd, IDC_TRAYACTIONMC, _r_config_getlong (L"TrayActionMc", 1, NULL));

					_r_ctrl_setstring (hwnd, IDC_SHOW_CLEAN_RESULT_CHK, _r_locale_getstring (IDS_SHOW_CLEAN_RESULT_CHK));
					_r_ctrl_setstring (hwnd, IDC_NOTIFICATIONSOUND_CHK, _r_locale_getstring (IDS_NOTIFICATIONSOUND_CHK));

					break;
				}

				case IDD_SETTINGS_ADVANCED:
				{
					_r_ctrl_setstring (hwnd, IDC_ALLOWSTANDBYLISTCLEANUP_CHK, L"Allow \"Standby lists\" and \"Modified page list\" cleanup on autoreduct");
					_r_ctrl_setstring (hwnd, IDC_LOGRESULTS_CHK, L"Log cleaning results into a debug log");

					break;
				}
			}

			break;
		}

		case WM_NOTIFY:
		{
			LPNMHDR nmlp;

			nmlp = (LPNMHDR)lparam;

			switch (nmlp->code)
			{
				case NM_CUSTOMDRAW:
				{
					LPNMLVCUSTOMDRAW lpnmlv;
					COLORREF clr;

					lpnmlv = (LPNMLVCUSTOMDRAW)lparam;

					switch (lpnmlv->nmcd.dwDrawStage)
					{
						case CDDS_PREPAINT:
						{
							SetWindowLongPtrW (hwnd, DWLP_MSGRESULT, CDRF_NOTIFYITEMDRAW);

							return CDRF_NOTIFYITEMDRAW;
						}

						case CDDS_ITEMPREPAINT:
						{
							if (lpnmlv->nmcd.hdr.idFrom == IDC_REGIONS)
							{
								if (_r_sys_isosversionlower (WINDOWS_8_1) && (lpnmlv->nmcd.lItemlParam & REDUCT_REGISTRY_CACHE) == REDUCT_REGISTRY_CACHE)
								{
									clr = _r_theme_isenabled () ? WND_GRAYTEXT_CLR : GetSysColor (COLOR_GRAYTEXT);

									lpnmlv->clrText = _r_dc_getcolorbrightness (clr);
									lpnmlv->clrTextBk = clr;

									return CDRF_NEWFONT;
								}

								if (_r_sys_isosversionlower (WINDOWS_10) && (lpnmlv->nmcd.lItemlParam & REDUCT_COMBINE_MEMORY_LISTS) == REDUCT_COMBINE_MEMORY_LISTS)
								{
									clr = _r_theme_isenabled () ? WND_GRAYTEXT_CLR : GetSysColor (COLOR_GRAYTEXT);

									lpnmlv->clrText = _r_dc_getcolorbrightness (clr);
									lpnmlv->clrTextBk = clr;

									return CDRF_NEWFONT;
								}
							}
							else if (lpnmlv->nmcd.hdr.idFrom == IDC_COLORS)
							{
								clr = (COLORREF)lpnmlv->nmcd.lItemlParam;

								lpnmlv->clrText = _r_dc_getcolorbrightness (clr);
								lpnmlv->clrTextBk = clr;

								_r_dc_fillrect (lpnmlv->nmcd.hdc, &lpnmlv->nmcd.rc, clr);

								SetWindowLongPtrW (hwnd, DWLP_MSGRESULT, CDRF_NEWFONT);

								return CDRF_NEWFONT;
							}
						}
					}

					break;
				}

				case NM_DBLCLK:
				{
					LPNMITEMACTIVATE lpnmlv;
					CHOOSECOLOR cc = {0};
					COLORREF cust[16] = {TRAY_COLOR_DANGER, TRAY_COLOR_WARNING, TRAY_COLOR_BG, TRAY_COLOR_TEXT};
					COLORREF clr;
					LONG dpi_value;
					INT listview_id;

					lpnmlv = (LPNMITEMACTIVATE)lparam;
					listview_id = (INT)(INT_PTR)lpnmlv->hdr.idFrom;

					if (lpnmlv->iItem == INT_ERROR || listview_id != IDC_COLORS)
						break;

					clr = (COLORREF)_r_listview_getitemlparam (hwnd, listview_id, lpnmlv->iItem);

					if (!clr)
						break;

					cc.lStructSize = sizeof (CHOOSECOLOR);
					cc.Flags = CC_RGBINIT | CC_FULLOPEN;
					cc.hwndOwner = hwnd;
					cc.lpCustColors = cust;
					cc.rgbResult = clr;

					if (ChooseColorW (&cc))
					{
						if (lpnmlv->iItem == 0)
						{
							_r_config_setulong (L"TrayColorText", cc.rgbResult, NULL);
						}
						else if (lpnmlv->iItem == 1)
						{
							_r_config_setulong (L"TrayColorBg", cc.rgbResult, NULL);
						}
						else if (lpnmlv->iItem == 2)
						{
							_r_config_setulong (L"TrayColorWarning", cc.rgbResult, NULL);
						}
						else if (lpnmlv->iItem == 3)
						{
							_r_config_setulong (L"TrayColorDanger", cc.rgbResult, NULL);
						}

						_r_listview_setitem (hwnd, IDC_COLORS, lpnmlv->iItem, lpnmlv->iSubItem, NULL, I_DEFAULT, I_DEFAULT, cc.rgbResult);

						_r_listview_redraw (hwnd, IDC_COLORS);

						dpi_value = _r_dc_gettaskbardpi ();

						_app_iconinit (dpi_value);
						_app_iconredraw (_r_app_gethwnd ());
					}

					break;
				}

				case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW lpnmlv;
					INT listview_id;
					ULONG value;
					ULONG mask;

					lpnmlv = (LPNMLISTVIEW)lparam;
					listview_id = (INT)(INT_PTR)lpnmlv->hdr.idFrom;

					if (listview_id != IDC_REGIONS)
						break;

					if (_r_wnd_getcontext (hwnd, listview_id))
						break;

					if ((lpnmlv->uChanged & LVIF_STATE) == 0)
						break;

					if ((lpnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK (1) || ((lpnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK (2)))
					{
						value = (ULONG)lpnmlv->lParam;

						mask = _r_config_getulong (L"ReductMask2", REDUCT_MASK_DEFAULT, NULL);

						if ((lpnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK (2))
						{
							if ((value & REDUCT_MASK_FREEZES) != 0)
							{
								if (!_r_show_confirmmessage (hwnd, NULL, _r_locale_getstring (IDS_QUESTION_WARNING), L"IsShowWarningConfirmation", FALSE))
								{
									_r_listview_setitemcheck (hwnd, listview_id, lpnmlv->iItem, FALSE);

									return FALSE;
								}
							}
						}

						if (_r_sys_isosversionlower (WINDOWS_8_1) && (value & REDUCT_REGISTRY_CACHE) == REDUCT_REGISTRY_CACHE)
						{
							_r_listview_setitemcheck (hwnd, listview_id, lpnmlv->iItem, (mask & REDUCT_REGISTRY_CACHE) == REDUCT_REGISTRY_CACHE);

							return FALSE;
						}

						if (_r_sys_isosversionlower (WINDOWS_10) && (value & REDUCT_COMBINE_MEMORY_LISTS) == REDUCT_COMBINE_MEMORY_LISTS)
						{
							_r_listview_setitemcheck (hwnd, listview_id, lpnmlv->iItem, (mask & REDUCT_COMBINE_MEMORY_LISTS) == REDUCT_COMBINE_MEMORY_LISTS);

							return FALSE;
						}

						if ((lpnmlv->uNewState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK (2))
						{
							mask |= value;
						}
						else
						{
							mask &= ~value;
						}

						_r_config_setulong (L"ReductMask2", mask, NULL);
					}

					break;
				}
			}

			break;
		}

		case WM_VSCROLL:
		case WM_HSCROLL:
		{
			LONG value;
			INT ctrl_id;
			BOOLEAN is_stylechanged = FALSE;

			ctrl_id = GetDlgCtrlID ((HWND)lparam);

			if (ctrl_id == IDC_AUTOREDUCTVALUE)
			{
				value = _r_updown_getvalue (hwnd, ctrl_id);

				_r_config_setlong (L"AutoreductValue", value, NULL);
			}
			else if (ctrl_id == IDC_AUTOREDUCTINTERVALVALUE)
			{
				value = _r_updown_getvalue (hwnd, ctrl_id);

				_r_config_setlong (L"AutoreductIntervalValue", value, NULL);
			}
			else if (ctrl_id == IDC_TRAYLEVELWARNING)
			{
				value = _r_updown_getvalue (hwnd, ctrl_id);

				_r_config_setlong (L"TrayLevelWarning", value, NULL);

				is_stylechanged = TRUE;
			}
			else if (ctrl_id == IDC_TRAYLEVELDANGER)
			{
				value = _r_updown_getvalue (hwnd, ctrl_id);

				_r_config_setlong (L"TrayLevelDanger", value, NULL);

				is_stylechanged = TRUE;
			}

			if (is_stylechanged)
			{
				_app_iconredraw (_r_app_gethwnd ());

				_r_listview_redraw (_r_app_gethwnd (), IDC_LISTVIEW);
			}

			break;
		}

		case WM_COMMAND:
		{
			INT ctrl_id = LOWORD (wparam);
			INT notify_code = HIWORD (wparam);

			switch (ctrl_id)
			{
				case IDC_AUTOREDUCTVALUE_CTRL:
				{
					LONG value;

					if (notify_code == EN_CHANGE)
					{
						value = _r_updown_getvalue (hwnd, IDC_AUTOREDUCTVALUE);

						_r_config_setlong (L"AutoreductValue", value, NULL);
					}

					break;
				}

				case IDC_AUTOREDUCTINTERVALVALUE_CTRL:
				{
					LONG value;

					if (notify_code == EN_CHANGE)
					{
						value = _r_updown_getvalue (hwnd, IDC_AUTOREDUCTINTERVALVALUE);

						_r_config_setlong (L"AutoreductIntervalValue", value, NULL);
					}

					break;
				}

				case IDC_TRAYLEVELWARNING_CTRL:
				case IDC_TRAYLEVELDANGER_CTRL:
				{
					LONG value;

					if (notify_code == EN_CHANGE)
					{
						if (ctrl_id == IDC_TRAYLEVELWARNING_CTRL)
						{
							value = _r_updown_getvalue (hwnd, IDC_TRAYLEVELWARNING);

							_r_config_setlong (L"TrayLevelWarning", value, NULL);
						}
						else if (ctrl_id == IDC_TRAYLEVELDANGER_CTRL)
						{
							value = _r_updown_getvalue (hwnd, IDC_TRAYLEVELDANGER);

							_r_config_setlong (L"TrayLevelDanger", value, NULL);
						}

						_app_iconredraw (_r_app_gethwnd ());

						_r_listview_redraw (_r_app_gethwnd (), IDC_LISTVIEW);
					}

					break;
				}

				case IDC_ALWAYSONTOP_CHK:
				{
					BOOLEAN is_enable;

					is_enable = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_config_setboolean (L"AlwaysOnTop", is_enable, NULL);

					_r_menu_checkitem (GetMenu (_r_app_gethwnd ()), IDM_ALWAYSONTOP_CHK, 0, MF_BYCOMMAND, is_enable);

					break;
				}

				case IDC_LOADONSTARTUP_CHK:
				{
					BOOLEAN is_enable;

					is_enable = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_autorun_enable (hwnd, is_enable);

					is_enable = _r_autorun_isenabled ();

					_r_menu_checkitem (GetMenu (_r_app_gethwnd ()), IDM_LOADONSTARTUP_CHK, 0, MF_BYCOMMAND, is_enable);

					_r_ctrl_checkbutton (hwnd, ctrl_id, is_enable);

					break;
				}

				case IDC_STARTMINIMIZED_CHK:
				{
					BOOLEAN is_enable;

					is_enable = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_config_setboolean (L"IsStartMinimized", is_enable, NULL);

					_r_menu_checkitem (GetMenu (_r_app_gethwnd ()), IDM_STARTMINIMIZED_CHK, 0, MF_BYCOMMAND, is_enable);

					break;
				}

				case IDC_REDUCTCONFIRMATION_CHK:
				{
					BOOLEAN is_enable;

					is_enable = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_config_setboolean (L"IsShowReductConfirmation", is_enable, NULL);

					_r_menu_checkitem (GetMenu (_r_app_gethwnd ()), IDM_REDUCTCONFIRMATION_CHK, 0, MF_BYCOMMAND, is_enable);

					break;
				}

				case IDC_SKIPUACWARNING_CHK:
				{
					BOOLEAN is_enable;

					is_enable = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_skipuac_enable (hwnd, is_enable);

					is_enable = _r_skipuac_isenabled ();

					_r_menu_checkitem (GetMenu (_r_app_gethwnd ()), IDM_SKIPUACWARNING_CHK, 0, MF_BYCOMMAND, is_enable);

					_r_ctrl_checkbutton (hwnd, ctrl_id, is_enable);

					break;
				}

				case IDC_CHECKUPDATES_CHK:
				{
					BOOLEAN is_enable;

					is_enable = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_update_enable (is_enable);

					is_enable = _r_update_isenabled (FALSE);

					_r_menu_checkitem (GetMenu (_r_app_gethwnd ()), IDM_CHECKUPDATES_CHK, 0, MF_BYCOMMAND, is_enable);

					break;
				}

				case IDC_LANGUAGE:
				{
					if (notify_code == CBN_SELCHANGE)
						_r_locale_apply (hwnd, ctrl_id, 0);

					break;
				}

				case IDC_AUTOREDUCTENABLE_CHK:
				{
					HWND hbuddy;
					BOOLEAN is_enabled;

					is_enabled = _r_ctrl_isenabled (hwnd, ctrl_id);

					hbuddy = _r_updown_getbuddy (hwnd, IDC_AUTOREDUCTVALUE);

					if (hbuddy)
						_r_ctrl_enable (hbuddy, 0, is_enabled);

					if (is_enabled)
					{
						is_enabled = (_r_ctrl_isbuttonchecked (hwnd, ctrl_id));

						_r_config_setboolean (L"AutoreductEnable", is_enabled, NULL);
					}

					break;
				}

				case IDC_AUTOREDUCTINTERVALENABLE_CHK:
				{
					HWND hbuddy;
					BOOLEAN is_enabled;

					is_enabled = _r_ctrl_isenabled (hwnd, ctrl_id);

					hbuddy = _r_updown_getbuddy (hwnd, IDC_AUTOREDUCTINTERVALVALUE);

					if (hbuddy)
						_r_ctrl_enable (hbuddy, 0, is_enabled);

					if (is_enabled)
					{
						is_enabled = (_r_ctrl_isbuttonchecked (hwnd, ctrl_id));

						_r_config_setboolean (L"AutoreductIntervalEnable", is_enabled, NULL);
					}

					break;
				}

				case IDC_HOTKEY_CLEAN_CHK:
				{
					BOOLEAN is_checked;

					is_checked = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_ctrl_enable (hwnd, IDC_HOTKEY_CLEAN, is_checked);

					_r_config_setboolean (L"HotkeyCleanEnable", is_checked, NULL);

					_app_hotkeyinit (_r_app_gethwnd ());

					break;
				}

				case IDC_HOTKEY_CLEAN:
				{
					if (!_r_ctrl_isbuttonchecked (hwnd, IDC_HOTKEY_CLEAN_CHK))
						break;

					if (notify_code == EN_CHANGE)
					{
						_r_config_setlong (L"HotkeyClean", _r_hotkey_get (hwnd, ctrl_id), NULL);

						_app_hotkeyinit (_r_app_gethwnd ());
					}

					break;
				}

				case IDC_TRAYUSETRANSPARENCY_CHK:
				case IDC_TRAYSHOWBORDER_CHK:
				case IDC_TRAYROUNDCORNERS_CHK:
				case IDC_TRAYCHANGEBG_CHK:
				case IDC_TRAYUSEANTIALIASING_CHK:
				{
					BOOLEAN is_enabled;
					LONG dpi_value;

					is_enabled = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					switch (ctrl_id)
					{
						case IDC_TRAYUSETRANSPARENCY_CHK:
						{
							_r_config_setboolean (L"TrayUseTransparency", is_enabled, NULL);
							break;
						}

						case IDC_TRAYSHOWBORDER_CHK:
						{
							_r_config_setboolean (L"TrayShowBorder", is_enabled, NULL);
							break;
						}

						case IDC_TRAYROUNDCORNERS_CHK:
						{
							_r_config_setboolean (L"TrayRoundCorners", is_enabled, NULL);
							break;
						}

						case IDC_TRAYCHANGEBG_CHK:
						{
							_r_config_setboolean (L"TrayChangeBg", is_enabled, NULL);
							break;
						}

						case IDC_TRAYUSEANTIALIASING_CHK:
						{
							_r_config_setboolean (L"TrayUseAntialiasing", is_enabled, NULL);
							break;
						}
					}

					dpi_value = _r_dc_gettaskbardpi ();

					_app_iconinit (dpi_value);
					_app_iconredraw (_r_app_gethwnd ());

					break;
				}

				case IDC_TRAYACTIONSC:
				{
					if (notify_code == CBN_SELCHANGE)
						_r_config_setlong (L"TrayActionDc", _r_combobox_getcurrentitem (hwnd, ctrl_id), NULL);

					break;
				}

				case IDC_TRAYACTIONMC:
				{
					if (notify_code == CBN_SELCHANGE)
						_r_config_setlong (L"TrayActionMc", _r_combobox_getcurrentitem (hwnd, ctrl_id), NULL);

					break;
				}

				case IDC_SHOW_CLEAN_RESULT_CHK:
				{
					BOOLEAN is_enabled;

					is_enabled = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_config_setboolean (L"BalloonCleanResults", is_enabled, NULL);

					break;
				}

				case IDC_NOTIFICATIONSOUND_CHK:
				{
					BOOLEAN is_enabled;

					is_enabled = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_config_setboolean (L"IsNotificationsSound", is_enabled, NULL);

					break;
				}

				case IDC_FONT:
				{
					CHOOSEFONT cf = {0};
					LOGFONT logfont;
					LONG dpi_value;

					cf.lStructSize = sizeof (CHOOSEFONT);
					cf.hwndOwner = hwnd;
					cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST | CF_SCREENFONTS;

					dpi_value = _r_dc_gettaskbardpi ();

					_app_fontinit (&logfont, dpi_value);

					cf.lpLogFont = &logfont;

					if (ChooseFontW (&cf))
					{
						_r_config_setfont (L"TrayFont", &logfont, dpi_value, NULL);

						_app_setfontcontrol (hwnd, IDC_FONT, &logfont, dpi_value);

						_app_iconinit (dpi_value);
						_app_iconredraw (_r_app_gethwnd ());
					}

					break;
				}

				case IDC_ALLOWSTANDBYLISTCLEANUP_CHK:
				{
					BOOLEAN is_enabled;

					is_enabled = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_config_setboolean (L"IsAllowStandbyListCleanup", is_enabled, NULL);

					break;
				}

				case IDC_LOGRESULTS_CHK:
				{
					BOOLEAN is_enabled;

					is_enabled = _r_ctrl_isbuttonchecked (hwnd, ctrl_id);

					_r_config_setboolean (L"LogCleanResults", is_enabled, NULL);

					break;
				}
			}

			break;
		}
	}

	return FALSE;
}
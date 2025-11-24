#pragma once

#include "routine.h"
#include "main.h"
#include "rapp.h"
#include "resource.h"
#include "constants.h"
#include "config_utils.h"
#include "ui_utils.h"
#include "memory_cleaner.h"
#include "app_init.h"
#include "icon_manager.h"

VOID _app_tray_create (
	_In_ HWND hwnd
);

VOID _app_tray_destroy (
	_In_ HWND hwnd
);

VOID _app_tray_popup (
	_In_ HWND hwnd,
	_In_ LPCWSTR title,
	_In_ LPCWSTR text
);

VOID _app_tray_menu_create (
	_In_ HWND hwnd
);

VOID _app_tray_menu_handle (
	_In_ HWND hwnd,
	_In_ INT ctrl_id,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
);
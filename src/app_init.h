#pragma once

#include "routine.h"
#include "main.h"
#include "rapp.h"
#include "resource.h"

VOID _app_hotkeyinit (
	_In_ HWND hwnd
);

VOID _app_setfontcontrol (
	_In_ HWND hwnd,
	_In_ INT ctrl_id,
	_In_ PLOGFONT logfont,
	_In_ LONG dpi_value
);

VOID _app_resizecolumns (
	_In_ HWND hwnd
);

VOID _app_initialize (
	_In_opt_ HWND hwnd
);
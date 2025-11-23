// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

#include <windows.h>

// Icon management functions
HICON _app_iconcreate (
	_In_opt_ ULONG percent
);

VOID _app_iconredraw (
	_In_opt_ HWND hwnd
);

VOID _app_iconinit (
	_In_ LONG dpi_value
);

// Timer callback function (declared in main.c)
VOID CALLBACK _app_timercallback (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ UINT_PTR id_event,
	_In_ ULONG time
);

// Icon creation helper functions (internal)
VOID _app_drawbackground (
	_In_ HDC hdc,
	_In_ COLORREF bg_clr,
	_In_ COLORREF pen_clr,
	_In_ COLORREF brush_clr,
	_In_ LPCRECT rect,
	_In_ BOOLEAN is_round
);
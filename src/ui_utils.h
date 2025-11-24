// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"
#include "main.h"

// Array generation and sorting
INT CALLBACK compare_numbers (
	_In_opt_ PVOID context,
	_In_ LPCVOID ptr1,
	_In_ LPCVOID ptr2
);

VOID _app_generate_array (
	_Out_ _Writable_elements_ (count) PULONG_PTR integers,
	_In_ ULONG_PTR count,
	_In_ ULONG_PTR value
);

// Menu generation
VOID _app_generate_menu (
	_In_ HMENU hsubmenu,
	_In_ INT start_id,
	_In_ ULONG_PTR* ptr_arr,
	_In_ ULONG items_count,
	_In_ LPCWSTR format,
	_In_ ULONG_PTR selected_value,
	_In_ BOOLEAN is_enabled
);

// Font and drawing utilities
VOID _app_fontinit (
	_Out_ PLOGFONT logfont,
	_In_ LONG dpi_value
);

VOID _app_drawbackground (
	_In_ HDC hdc,
	_In_ COLORREF bg_clr,
	_In_ COLORREF pen_clr,
	_In_ COLORREF brush_clr,
	_In_ LPCRECT rect,
	_In_ BOOLEAN is_round
);
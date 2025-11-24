// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"
#include "main.h"
#include "constants.h"

// Menu generation function
VOID _app_generate_menu (
	_In_ HMENU hsubmenu,
	_In_ INT start_id,
	_In_ ULONG_PTR* ptr_arr,
	_In_ ULONG items_count,
	_In_ LPCWSTR format,
	_In_ ULONG_PTR selected_value,
	_In_ BOOLEAN is_enabled
);

// Alias for _r_menu_clearitems
#define _r_menu_clear(hmenu) _r_menu_clearitems(hmenu)
// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#include "utils.h"

VOID _app_generate_menu (
	_In_ HMENU hsubmenu,
	_In_ INT start_id,
	_In_ ULONG_PTR* ptr_arr,
	_In_ ULONG items_count,
	_In_ LPCWSTR format,
	_In_ ULONG_PTR selected_value,
	_In_ BOOLEAN is_enabled
)
{
	WCHAR buffer[128];
	ULONG_PTR item_value;
	INT check_state;
	BOOLEAN is_checked;

	if (!hsubmenu || !ptr_arr || !items_count)
		return;

	// Clear existing menu items
	_r_menu_clear (hsubmenu);

	// Add menu items
	for (ULONG i = 0; i < items_count; i++)
	{
		item_value = ptr_arr[i];
		is_checked = (selected_value == item_value);
		check_state = is_checked ? MF_CHECKED : MF_UNCHECKED;

		if (format)
		{
			_r_str_printf (buffer, RTL_NUMBER_OF (buffer), format, item_value);
		}
		else
		{
			_r_str_printf (buffer, RTL_NUMBER_OF (buffer), L"%" TEXT (PR_ULONG), item_value);
		}

		AppendMenu (hsubmenu, MF_BYPOSITION | check_state, start_id + i, buffer);
	}

	// Add separator and disable option
	if (is_enabled)
	{
		AppendMenu (hsubmenu, MF_SEPARATOR, 0, NULL);
		AppendMenu (hsubmenu, MF_BYPOSITION, start_id + items_count, _r_locale_getstring (IDS_TRAY_DISABLE));
	}
}
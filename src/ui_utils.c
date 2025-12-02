// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#include "ui_utils.h"

INT CALLBACK compare_numbers (
	_In_opt_ PVOID context,
	_In_ LPCVOID ptr1,
	_In_ LPCVOID ptr2
)
{
	ULONG_PTR val1;
	ULONG_PTR val2;

	val1 = *(PULONG_PTR)ptr1;
	val2 = *(PULONG_PTR)ptr2;

	if (val1 < val2)
		return -1;

	if (val1 > val2)
		return 1;

	return 0;
}

VOID _app_generate_array (
	_Out_ _Writable_elements_ (count) PULONG_PTR integers,
	_In_ ULONG_PTR count,
	_In_ ULONG_PTR value
)
{
	PR_HASHTABLE hashtable;
	ULONG_PTR enum_key = 0;
	ULONG hash_code;
	ULONG_PTR index;

	RtlSecureZeroMemory (integers, sizeof (ULONG_PTR) * (SIZE_T)count);

	hashtable = _r_obj_createhashtable (sizeof (BOOLEAN), 16, NULL);

	// 添加10的倍数（10, 20, 30, ..., 80）
	for (index = 1; index < 9; index++)
	{
		_r_obj_addhashtableitem (hashtable, (ULONG)(index * 10), NULL);
	}

	// 添加selected_value附近的值（如果>=0）
	for (index = value - 2; index <= (value + 2); index++)
	{
		if (index >= 0) // 修复：允许添加0值
			_r_obj_addhashtableitem (hashtable, (ULONG)index, NULL);
	}

	index = 0;

	// 从哈希表中获取值填充数组
	while (_r_obj_enumhashtable (hashtable, NULL, &hash_code, (PULONG_PTR)&enum_key))
	{
		if (hash_code <= 99)
			*(PULONG_PTR)PTR_ADD_OFFSET (integers, index * sizeof (ULONG_PTR)) = (ULONG_PTR)hash_code;

		if (++index >= count)
			break;
	}

	// 确保数组中没有空值，如果有则填充合理的值
	if (index < count)
	{
		for (ULONG_PTR i = index; i < count; i++)
		{
			// 填充递增的值，确保不会有空值
			integers[i] = (i + 1) * 5;
		}
	}

	qsort_s ((void*)integers, (SIZE_T)count, sizeof (ULONG_PTR), &compare_numbers, NULL);

	_r_obj_dereference (hashtable);
}

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
	WCHAR buffer[64];
	LONG_PTR menu_value;
	ULONG menu_items = 0;
	ULONG menu_id;
	BOOLEAN is_checked = FALSE;

	_app_generate_array (ptr_arr, items_count, selected_value);

	for (UINT i = 0; i < items_count; i++)
	{
		menu_value = ptr_arr[i];

		// 修复：允许0值显示，因为清理界限和间隔可能设置为0
		// 但仍然跳过无效的负值
		if (menu_value < 0)
			continue;

		menu_id = start_id + i;

		// 修复字符格式化问题：使用宽字符格式字符串和正确的类型转换
		// 直接使用menu_value参数，确保格式字符串与参数类型匹配
		_r_str_printf (buffer, RTL_NUMBER_OF (buffer), format, (ULONG_PTR)menu_value);

		_r_menu_additem (hsubmenu, menu_id, buffer);

		if (!_r_sys_iselevated ())
			_r_menu_enableitem (hsubmenu, menu_id, FALSE, FALSE);

		if (selected_value == menu_value)
		{
			_r_menu_checkitem (hsubmenu, menu_id, menu_id, MF_BYCOMMAND, menu_id);

			is_checked = TRUE;
		}

		menu_items += 1;
	}

	if (!is_enabled || !is_checked)
		_r_menu_checkitem (hsubmenu, 0, menu_items + 2, MF_BYPOSITION, 0);
}

VOID _app_fontinit (
	_Out_ PLOGFONT logfont,
	_In_ LONG dpi_value
)
{
	RtlZeroMemory (logfont, sizeof (LOGFONT));

	_r_str_copy (logfont->lfFaceName, LF_FACESIZE, L"Lucida Console");

	logfont->lfHeight = _r_dc_fontsizetoheight (8, dpi_value);
	logfont->lfWeight = FW_NORMAL;

	_r_config_getfont (L"TrayFont", logfont, dpi_value, NULL);

	logfont->lfCharSet = DEFAULT_CHARSET;
	logfont->lfQuality = CLEARTYPE_QUALITY;
}

VOID _app_drawbackground (
	_In_ HDC hdc,
	_In_ COLORREF bg_clr,
	_In_ COLORREF pen_clr,
	_In_ COLORREF brush_clr,
	_In_ LPCRECT rect,
	_In_ BOOLEAN is_round
)
{
	HGDIOBJ prev_brush;
	HGDIOBJ prev_pen;
	COLORREF prev_clr;

	prev_brush = SelectObject (hdc, GetStockObject (DC_BRUSH));
	prev_pen = SelectObject (hdc, GetStockObject (DC_PEN));

	prev_clr = SetBkColor (hdc, bg_clr);

	SetDCPenColor (hdc, pen_clr);
	SetDCBrushColor (hdc, brush_clr);

	_r_dc_fillrect (hdc, rect, bg_clr);

	if (is_round)
	{
		RoundRect (hdc, rect->left, rect->top, rect->right, rect->bottom, rect->right - 2, rect->right / 2);
	}
	else
	{
		Rectangle (hdc, rect->left, rect->top, rect->right, rect->bottom);
	}

	SelectObject (hdc, prev_brush);
	SelectObject (hdc, prev_pen);

	SetBkColor (hdc, prev_clr);
}
// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#include "config_utils.h"

ULONG _app_getlimitvalue ()
{
	ULONG value;

	value = _r_config_getulong (L"AutoreductValue", DEFAULT_AUTOREDUCT_VAL, NULL);

	return _r_calc_clamp (value, 1, 99);
}

LONG_PTR _app_getintervalvalue ()
{
	LONG_PTR value;

#if defined(_WIN64)
	value = _r_config_getlong64 (L"AutoreductIntervalValue", DEFAULT_AUTOREDUCTINTERVAL_VAL, NULL);

	return _r_calc_clamp64 (value, 1, 1440);
#else
	value = _r_config_getlong (L"AutoreductIntervalValue", DEFAULT_AUTOREDUCTINTERVAL_VAL, NULL);

	return _r_calc_clamp (value, 1, 1440);
#endif // _WIN64
}

ULONG _app_getdangervalue ()
{
	ULONG value;

	value = _r_config_getulong (L"TrayLevelDanger", DEFAULT_DANGER_LEVEL, NULL);

	return _r_calc_clamp (value, 1, 99);
}

ULONG _app_getwarningvalue ()
{
	ULONG value;

	value = _r_config_getulong (L"TrayLevelWarning", DEFAULT_WARNING_LEVEL, NULL);

	return _r_calc_clamp (value, 1, 99);
}

ULONG64 _app_getmemoryinfo (
	_Out_ PR_MEMORY_INFO mem_info
)
{
	_r_sys_getmemoryinfo (mem_info);

	return mem_info->physical_memory.used_bytes;
}

LPCWSTR _app_getcleanupreason (
	_In_ CLEANUP_SOURCE_ENUM src
)
{
	switch (src)
	{
		case SOURCE_AUTO:
		{
			return L"Cleanup (Auto)";
		}

		case SOURCE_MANUAL:
		{
			return L"Cleanup (Manual)";
		}

		case SOURCE_HOTKEY:
		{
			return L"Cleanup (Hotkey)";
		}

		case SOURCE_CMDLINE:
		{
			return L"Cleanup (Command-line)";
		}
	}

	return NULL;
}
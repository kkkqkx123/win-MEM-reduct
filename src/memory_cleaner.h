// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"
#include "main.h"
#include "constants.h"

// Memory cleanup functions
NTSTATUS _app_flushvolumecache ();

VOID _app_memoryclean (
	_In_opt_ HWND hwnd,
	_In_ CLEANUP_SOURCE_ENUM src,
	_In_opt_ ULONG mask
);

LPCWSTR _app_getcleanupreason (
	_In_ CLEANUP_SOURCE_ENUM src
);

// WSL cleanup functions
WSL_CLEANUP_RESULT _app_wsl_cleanup_cache ();
WSL_CLEANUP_RESULT _app_wsl_reclaim_memory ();
LPCWSTR _app_get_wsl_error_text (
	_In_ WSL_CLEANUP_RESULT result
);
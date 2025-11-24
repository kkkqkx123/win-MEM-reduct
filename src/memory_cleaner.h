// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"
#include "main.h"
#include "constants.h"

// Memory cleanup result codes
typedef enum _CLEANUP_SOURCE_ENUM
{
	SOURCE_AUTO = 0,
	SOURCE_MANUAL = 1,
	SOURCE_HOTKEY = 2,
	SOURCE_CMDLINE = 3
} CLEANUP_SOURCE_ENUM;

// WSL cleanup result codes
typedef enum _WSL_CLEANUP_RESULT
{
	WSL_CLEANUP_SUCCESS = 0,
	WSL_CLEANUP_NOT_INSTALLED = 0x80000001,
	WSL_CLEANUP_NOT_RUNNING = 0x80000002,
	WSL_CLEANUP_ACCESS_DENIED = 0x80000003,
	WSL_CLEANUP_COMMAND_FAILED = 0x80000004,
	WSL_CLEANUP_SERVICE_ERROR = 0x80000005,
	WSL_CLEANUP_TIMEOUT = 0x80000006,
	WSL_CLEANUP_MEMORY_ERROR = 0x80000007,
	WSL_CLEANUP_INVALID_PARAMETER = 0x80000008
} WSL_CLEANUP_RESULT;

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
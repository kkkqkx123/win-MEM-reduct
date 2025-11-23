// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"
#include "main.h"

// Configuration management functions
ULONG _app_getlimitvalue ();

LONG_PTR _app_getintervalvalue ();

ULONG _app_getdangervalue ();

ULONG _app_getwarningvalue ();

ULONG64 _app_getmemoryinfo (
	_Out_ PR_MEMORY_INFO mem_info
);

LPCWSTR _app_getcleanupreason (
	_In_ CLEANUP_SOURCE_ENUM src
);
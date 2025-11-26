// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

#include "routine.h"
#include "main.h"
#include "constants.h"
#include "config_utils.h"
#include "ui_utils.h"
#include "icon_manager.h"
#include "app_init.h"

// Settings dialog procedure
INT_PTR CALLBACK SettingsProc (
	_In_ HWND hwnd,
	_In_ UINT msg,
	_In_ WPARAM wparam,
	_In_ LPARAM lparam
);
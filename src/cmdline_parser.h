#pragma once

#include "routine.h"
#include "main.h"
#include "rapp.h"
#include "resource.h"
#include "constants.h"
#include "memory_cleaner.h"
#include "app_init.h"

BOOLEAN NTAPI _app_parseargs (
	_In_ R_CMDLINE_INFO_CLASS type
);
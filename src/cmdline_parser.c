#include "cmdline_parser.h"

BOOLEAN NTAPI _app_parseargs (
	_In_ R_CMDLINE_INFO_CLASS type
)
{
	PR_STRING clean_args;
	ULONG mask = 0;

	switch (type)
	{
		case CmdlineClean:
		{
			_r_sys_getopt ((LPCWSTR)_r_sys_getcommandline ()->buffer, L"clean", &clean_args);

			if (clean_args)
			{
				if (_r_str_isequal2 (&clean_args->sr, (LPWSTR)L"full", TRUE))
					mask = REDUCT_MASK_ALL;

				_r_obj_dereference (clean_args);
			}

			if (!mask)
				mask = REDUCT_MASK_DEFAULT;

			_app_initialize (NULL);

			_app_memoryclean (NULL, SOURCE_CMDLINE, mask);

			return TRUE;
		}

		case CmdlineHelp:
		{
			_r_show_message (
				NULL,
				MB_OK | MB_ICONINFORMATION | MB_TOPMOST,
				(LPCWSTR)L"Available options for memreduct.exe:",
				(LPCWSTR)L"-clean - clear default memory regions\r\n" \
				L"-clean:full - clear all memory regions"
			);

			return TRUE;
		}
	}

	return FALSE;
}
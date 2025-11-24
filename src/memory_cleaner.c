// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#include "memory_cleaner.h"

// WSL cleanup functions implementation
WSL_CLEANUP_RESULT _app_wsl_cleanup_cache ()
{
	// Check if WSL is installed
	if (!_r_sys_isosversiongreaterorequal (WINDOWS_10))
		return WSL_CLEANUP_NOT_INSTALLED;

	// Check if WSL service is running
	SC_HANDLE hscmanager = OpenSCManagerW (NULL, NULL, SC_MANAGER_CONNECT);
	if (!hscmanager)
		return WSL_CLEANUP_ACCESS_DENIED;

	SC_HANDLE hservice = OpenServiceW (hscmanager, L"LxssManager", SERVICE_QUERY_STATUS);
	if (!hservice)
	{
		CloseServiceHandle (hscmanager);
		return WSL_CLEANUP_NOT_INSTALLED;
	}

	SERVICE_STATUS status;
	if (!QueryServiceStatus (hservice, &status))
	{
		CloseServiceHandle (hservice);
		CloseServiceHandle (hscmanager);
		return WSL_CLEANUP_SERVICE_ERROR;
	}

	if (status.dwCurrentState != SERVICE_RUNNING)
	{
		CloseServiceHandle (hservice);
		CloseServiceHandle (hscmanager);
		return WSL_CLEANUP_NOT_RUNNING;
	}

	CloseServiceHandle (hservice);
	CloseServiceHandle (hscmanager);

	// Execute WSL cache cleanup command
	WCHAR command[] = L"wsl.exe --shutdown";
	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);

	if (!CreateProcessW (NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		return WSL_CLEANUP_COMMAND_FAILED;

	// Wait for completion with timeout
	DWORD wait_result = WaitForSingleObject (pi.hProcess, 30000); // 30 second timeout
	
	CloseHandle (pi.hThread);
	CloseHandle (pi.hProcess);

	if (wait_result == WAIT_TIMEOUT)
		return WSL_CLEANUP_TIMEOUT;

	return WSL_CLEANUP_SUCCESS;
}

WSL_CLEANUP_RESULT _app_wsl_reclaim_memory ()
{
	// Check if WSL is installed
	if (!_r_sys_isosversiongreaterorequal (WINDOWS_10))
		return WSL_CLEANUP_NOT_INSTALLED;

	// Execute WSL memory reclaim command
	WCHAR command[] = L"wsl.exe -d Ubuntu -- sh -c \"echo 3 > /proc/sys/vm/drop_caches\"";
	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};
	si.cb = sizeof(si);

	if (!CreateProcessW (NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		return WSL_CLEANUP_COMMAND_FAILED;

	// Wait for completion with timeout
	DWORD wait_result = WaitForSingleObject (pi.hProcess, 30000); // 30 second timeout
	
	CloseHandle (pi.hThread);
	CloseHandle (pi.hProcess);

	if (wait_result == WAIT_TIMEOUT)
		return WSL_CLEANUP_TIMEOUT;

	return WSL_CLEANUP_SUCCESS;
}

LPCWSTR _app_get_wsl_error_text (
	_In_ WSL_CLEANUP_RESULT result
)
{
	switch (result)
	{
		case WSL_CLEANUP_SUCCESS:
			return L"WSL cleanup completed successfully";
		case WSL_CLEANUP_NOT_INSTALLED:
			return L"WSL is not installed or not supported on this system";
		case WSL_CLEANUP_NOT_RUNNING:
			return L"WSL service is not running";
		case WSL_CLEANUP_ACCESS_DENIED:
			return L"Access denied to WSL service";
		case WSL_CLEANUP_COMMAND_FAILED:
			return L"Failed to execute WSL cleanup command";
		case WSL_CLEANUP_SERVICE_ERROR:
			return L"WSL service error";
		case WSL_CLEANUP_TIMEOUT:
			return L"WSL cleanup operation timed out";
		case WSL_CLEANUP_MEMORY_ERROR:
			return L"WSL memory operation failed";
		case WSL_CLEANUP_INVALID_PARAMETER:
			return L"Invalid parameter for WSL cleanup";
		default:
			return L"Unknown WSL cleanup error";
	}
}

NTSTATUS _app_flushvolumecache ()
{
	PMOUNTMGR_MOUNT_POINTS object_mountpoints;
	PMOUNTMGR_MOUNT_POINT mountpoint;
	OBJECT_ATTRIBUTES oa = {0};
	IO_STATUS_BLOCK isb;
	UNICODE_STRING us;
	HANDLE hdevice;
	HANDLE hvolume;
	NTSTATUS status;

	RtlInitUnicodeString (&us, MOUNTMGR_DEVICE_NAME);

	InitializeObjectAttributes (&oa, &us, OBJ_CASE_INSENSITIVE, NULL, NULL);

	status = NtCreateFile (
		&hdevice,
		FILE_READ_ATTRIBUTES | SYNCHRONIZE,
		&oa,
		&isb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_OPEN,
		FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0
	);

	if (!NT_SUCCESS (status))
		return status;

	status = _r_fs_getvolumemountpoints (hdevice, &object_mountpoints);

	if (!NT_SUCCESS (status))
		goto CleanupExit;

	for (ULONG i = 0; i < object_mountpoints->NumberOfMountPoints; i++)
	{
		mountpoint = &object_mountpoints->MountPoints[i];

		us.Length = mountpoint->SymbolicLinkNameLength;
		us.MaximumLength = mountpoint->SymbolicLinkNameLength + sizeof (UNICODE_NULL);
		us.Buffer = PTR_ADD_OFFSET (object_mountpoints, mountpoint->SymbolicLinkNameOffset);

		if (MOUNTMGR_IS_VOLUME_NAME (&us)) // \\??\\Volume{1111-2222}
		{
			InitializeObjectAttributes (&oa, &us, OBJ_CASE_INSENSITIVE, NULL, NULL);

			status = NtCreateFile (
				&hvolume,
				FILE_WRITE_DATA | SYNCHRONIZE,
				&oa,
				&isb,
				NULL,
				FILE_ATTRIBUTE_NORMAL,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				FILE_OPEN,
				FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
				NULL,
				0
			);

			if (NT_SUCCESS (status))
			{
				status = _r_fs_flushfile (hvolume);

				NtClose (hvolume);
			}
		}
	}

	_r_mem_free (object_mountpoints);

CleanupExit:

	NtClose (hdevice);

	return status;
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

VOID _app_memoryclean (
	_In_opt_ HWND hwnd,
	_In_ CLEANUP_SOURCE_ENUM src,
	_In_opt_ ULONG mask
)
{
	MEMORY_COMBINE_INFORMATION_EX combine_info_ex = {0};
	SYSTEM_FILECACHE_INFORMATION sfci = {0};
	SYSTEM_MEMORY_LIST_COMMAND command;
	R_MEMORY_INFO mem_info;
	WCHAR buffer1[256] = {0};
	WCHAR buffer2[256] = {0};
	LPCWSTR error_text;
	ULONG64 reduct_before;
	ULONG64 reduct_after;
	ULONG flags = NIIF_WARNING;
	NTSTATUS status;

	if (!_r_config_getboolean (L"IsNotificationsSound", TRUE, NULL))
		flags |= NIIF_NOSOUND;

	if (!_r_sys_iselevated ())
	{
		error_text = _r_locale_getstring (IDS_STATUS_NOPRIVILEGES);

		if (_r_app_runasadmin ())
		{
			if (hwnd)
				DestroyWindow (hwnd);
		}
		else
		{
			if (src == SOURCE_CMDLINE)
			{
				if (hwnd)
					_r_show_message (hwnd, MB_OK | MB_ICONSTOP, NULL, error_text);
			}
			else
			{
				if (hwnd)
					_r_tray_popup (hwnd, &GUID_TrayIcon, flags, _r_app_getname (), error_text);
			}
		}

		return;
	}

	if (!mask)
		mask = _r_config_getulong (L"ReductMask2", REDUCT_MASK_DEFAULT, NULL);

	if (src == SOURCE_AUTO)
	{
		if (!_r_config_getboolean (L"IsAllowStandbyListCleanup", FALSE, NULL))
			mask &= ~REDUCT_MASK_FREEZES; // exclude freezes from autoclean feature ;)
	}
	else if (src == SOURCE_MANUAL)
	{
		if ((mask & REDUCT_WORKING_SET) == REDUCT_WORKING_SET)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_WORKINGSET_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_SYSTEM_FILE_CACHE) == REDUCT_SYSTEM_FILE_CACHE)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_SYSTEMFILECACHE_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_MODIFIED_LIST) == REDUCT_MODIFIED_LIST)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_MODIFIEDLIST_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_STANDBY_LIST) == REDUCT_STANDBY_LIST)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_STANDBYLIST_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_STANDBY_PRIORITY0_LIST) == REDUCT_STANDBY_PRIORITY0_LIST)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_STANDBYLISTPRIORITY0_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_MODIFIED_FILE_CACHE) == REDUCT_MODIFIED_FILE_CACHE)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_MODIFIEDFILECACHE_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_REGISTRY_CACHE) == REDUCT_REGISTRY_CACHE)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_REGISTRYCACHE_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_COMBINE_MEMORY_LISTS) == REDUCT_COMBINE_MEMORY_LISTS)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), _r_locale_getstring (IDS_COMBINEMEMORYLISTS_CHK));
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_WSL_CACHE_CLEAN) == REDUCT_WSL_CACHE_CLEAN)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"WSL cache cleanup");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if ((mask & REDUCT_WSL_MEMORY_RECLAIM) == REDUCT_WSL_MEMORY_RECLAIM)
		{
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"- ");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"WSL memory reclaim");
			_r_str_append (buffer1, RTL_NUMBER_OF (buffer1), L"\r\n");
		}

		if (_r_show_confirmmessage (hwnd, _r_app_getname (), buffer1, L"IsShowReductConfirmation", TRUE))
		{
			// continue ;)
		}
		else
		{
			return;
		}
	}

	SetCursor (LoadCursorW (NULL, IDC_WAIT));

	// difference (before)
	reduct_before = _app_getmemoryinfo (&mem_info);

	// Working set cleanup
	if ((mask & REDUCT_WORKING_SET) == REDUCT_WORKING_SET)
	{
		PROCESSENTRY32W pe32;
		HANDLE hsnapshot;
		HANDLE hprocess;
		NTSTATUS status;

		hsnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);

		if (hsnapshot != INVALID_HANDLE_VALUE)
		{
			pe32.dwSize = sizeof (PROCESSENTRY32W);

			if (Process32FirstW (hsnapshot, &pe32))
			{
				do
				{
					if (pe32.th32ProcessID == 0 || pe32.th32ProcessID == 4)
						continue;

					hprocess = OpenProcess (PROCESS_SET_QUOTA, FALSE, pe32.th32ProcessID);

					if (hprocess)
					{
						status = NtSetInformationProcess (hprocess, ProcessWorkingSetWatch, NULL, 0);

						if (!NT_SUCCESS (status))
						{
							if (status != STATUS_ACCESS_DENIED)
							{
								_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetInformationProcess", status, L"ProcessWorkingSetWatch (pid: %" TEXT (PR_LONG) L")", pe32.th32ProcessID);
							}
						}

						CloseHandle (hprocess);
					}

				} while (Process32NextW (hsnapshot, &pe32));
			}

			CloseHandle (hsnapshot);
		}
	}

	// System file cache cleanup
	if ((mask & REDUCT_SYSTEM_FILE_CACHE) == REDUCT_SYSTEM_FILE_CACHE)
	{
		sfci.MinimumWorkingSet = (SIZE_T)-1;
		sfci.MaximumWorkingSet = (SIZE_T)-1;

		status = NtSetSystemInformation (SystemFileCacheInformation, &sfci, sizeof (SYSTEM_FILECACHE_INFORMATION));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"SystemFileCacheInformation");
	}

	// Modified page list cleanup
	if ((mask & REDUCT_MODIFIED_LIST) == REDUCT_MODIFIED_LIST)
	{
		command.ListCommand = MemoryPurgeModifiedList;

		status = NtSetSystemInformation (SystemMemoryListInformation, &command, sizeof (SYSTEM_MEMORY_LIST_COMMAND));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"MemoryPurgeModifiedList");
	}

	// Standby list cleanup
	if ((mask & REDUCT_STANDBY_LIST) == REDUCT_STANDBY_LIST)
	{
		command.ListCommand = MemoryPurgeStandbyList;

		status = NtSetSystemInformation (SystemMemoryListInformation, &command, sizeof (SYSTEM_MEMORY_LIST_COMMAND));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"MemoryPurgeStandbyList");
	}

	// Standby priority 0 list cleanup
	if ((mask & REDUCT_STANDBY_PRIORITY0_LIST) == REDUCT_STANDBY_PRIORITY0_LIST)
	{
		command.ListCommand = MemoryPurgeLowPriorityStandbyList;

		status = NtSetSystemInformation (SystemMemoryListInformation, &command, sizeof (SYSTEM_MEMORY_LIST_COMMAND));

		if (!NT_SUCCESS (status))
			_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"MemoryPurgeLowPriorityStandbyList");
	}

	// Flush volume cache
	if ((mask & REDUCT_MODIFIED_FILE_CACHE) == REDUCT_MODIFIED_FILE_CACHE)
		_app_flushvolumecache ();

	// Flush registry cache (win8.1+)
	if (_r_sys_isosversiongreaterorequal (WINDOWS_8_1))
	{
		if ((mask & REDUCT_REGISTRY_CACHE) == REDUCT_REGISTRY_CACHE)
		{
			status = NtSetSystemInformation (SystemRegistryReconciliationInformation, NULL, 0);

			if (!NT_SUCCESS (status))
				_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"SystemRegistryReconciliationInformation");
		}
	}

	// Combine memory lists (win10+)
	if (_r_sys_isosversiongreaterorequal (WINDOWS_10))
	{
		if ((mask & REDUCT_COMBINE_MEMORY_LISTS) == REDUCT_COMBINE_MEMORY_LISTS)
		{
			status = NtSetSystemInformation (SystemCombinePhysicalMemoryInformation, &combine_info_ex, sizeof (MEMORY_COMBINE_INFORMATION_EX));

			if (!NT_SUCCESS (status))
				_r_log (LOG_LEVEL_ERROR, NULL, L"NtSetSystemInformation", status, L"SystemCombinePhysicalMemoryInformation");
		}
	}

	// WSL cache clean
	if ((mask & REDUCT_WSL_CACHE_CLEAN) == REDUCT_WSL_CACHE_CLEAN)
	{
		WSL_CLEANUP_RESULT wsl_result = _app_wsl_cleanup_cache ();
		
		if (wsl_result != WSL_CLEANUP_SUCCESS)
		{
			LPCWSTR error_text = _app_get_wsl_error_text (wsl_result);
			_r_log (LOG_LEVEL_ERROR, NULL, L"_app_wsl_cleanup_cache", wsl_result, error_text);
		}
	}

	// WSL memory reclaim
	if ((mask & REDUCT_WSL_MEMORY_RECLAIM) == REDUCT_WSL_MEMORY_RECLAIM)
	{
		WSL_CLEANUP_RESULT wsl_result = _app_wsl_reclaim_memory ();
		
		if (wsl_result != WSL_CLEANUP_SUCCESS)
		{
			LPCWSTR error_text = _app_get_wsl_error_text (wsl_result);
			_r_log (LOG_LEVEL_ERROR, NULL, L"_app_wsl_reclaim_memory", wsl_result, error_text);
		}
	}

	SetCursor (LoadCursorW (NULL, IDC_ARROW));

	// difference (after)
	reduct_after = _app_getmemoryinfo (&mem_info);

	if (reduct_after < reduct_before)
	{
		reduct_after = (reduct_before - reduct_after);
	}
	else
	{
		reduct_after = 0;
	}

	// time of last cleaning
	_r_config_setlong64 (L"StatisticLastReduct", _r_unixtime_now (), NULL);

	_r_format_bytesize64 (buffer1, RTL_NUMBER_OF (buffer1), reduct_after);

	_r_str_printf (buffer2, RTL_NUMBER_OF (buffer2), _r_locale_getstring (IDS_STATUS_CLEANED), buffer1);

	if (src == SOURCE_CMDLINE)
	{
		_r_show_message (hwnd, MB_OK | MB_ICONINFORMATION, NULL, buffer2);
	}
	else
	{
		if (hwnd && _r_config_getboolean (L"BalloonCleanResults", TRUE, NULL))
			_r_tray_popup (hwnd, &GUID_TrayIcon, flags, _r_app_getname (), buffer2);
	}

	if (_r_config_getboolean (L"LogCleanResults", FALSE, NULL))
		_r_log_v (LOG_LEVEL_INFO, 0, _app_getcleanupreason (src), 0, buffer1);
}
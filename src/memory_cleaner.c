// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#include "memory_cleaner.h"
#include "config_utils.h"
#include "settings_dialog.h"
#include <tlhelp32.h>

// WSL functions are implemented in wsl_helper.c

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

// _app_getcleanupreason is implemented in main.c

// _app_memoryclean is implemented in main.c
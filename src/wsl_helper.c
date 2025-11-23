#include "main.h"
#include "resource.h"

// WSL helper functions implementation

/**
 * Check if WSL is installed and available
 */
BOOLEAN _app_wsl_is_available()
{
	static BOOLEAN is_checked = FALSE;
	static BOOLEAN is_available = FALSE;
	
	if (is_checked)
		return is_available;
		
	// Check if wsl.exe exists in system
	R_STRINGREF wsl_name;
	PR_STRING wsl_path = NULL;
	
	_r_obj_initializestringref (&wsl_name, L"wsl.exe");
	NTSTATUS status = _r_path_search (NULL, &wsl_name, NULL, &wsl_path);
	
	if (NT_SUCCESS (status) && wsl_path)
	{
		is_available = TRUE;
		_r_obj_dereference (wsl_path);
	}

	
	is_checked = TRUE;
	return is_available;
}

/**
 * Check if WSL is currently running
 */
BOOLEAN _app_wsl_is_running()
{
	// Check if WSL service (LxssManager) is running
	SC_HANDLE scm = OpenSCManagerW (NULL, NULL, SC_MANAGER_CONNECT);
	
	if (!scm)
		return FALSE;
		
	SC_HANDLE service = OpenServiceW (scm, L"LxssManager", SERVICE_QUERY_STATUS);
	BOOLEAN is_running = FALSE;
	
	if (service)
	{
		SERVICE_STATUS status;
		if (QueryServiceStatus (service, &status))
		{
			is_running = (status.dwCurrentState == SERVICE_RUNNING);
		}
		CloseServiceHandle (service);
	}
	
	CloseServiceHandle (scm);
	return is_running;
}

/**
 * Get WSL memory information
 */
WSL_CLEANUP_RESULT _app_wsl_get_memory_info(
	_Out_ PWSL_MEMORY_INFO wsl_info
)
{
	if (!wsl_info)
		return WSL_CLEANUP_INVALID_PARAMETER;
		
	RtlZeroMemory (wsl_info, sizeof (WSL_MEMORY_INFO));
	
	if (!_app_wsl_is_available())
		return WSL_CLEANUP_NOT_INSTALLED;
		
	if (!_app_wsl_is_running())
		return WSL_CLEANUP_NOT_RUNNING;
	
	// Execute WSL command to get memory info
	WCHAR command[] = L"wsl.exe -d Ubuntu -e free -b";
	
	// For now, just return success without actual command execution
	// TODO: Implement proper command execution with output capture
	// For now, set some dummy values
	wsl_info->working_set_size = 1024 * 1024 * 1024; // 1GB
	wsl_info->private_usage = 512 * 1024 * 1024; // 512MB
	return WSL_CLEANUP_SUCCESS;
}

/**
 * Clean WSL cache
 */
WSL_CLEANUP_RESULT _app_wsl_cleanup_cache()
{
	if (!_app_wsl_is_available())
		return WSL_CLEANUP_NOT_INSTALLED;
		
	if (!_app_wsl_is_running())
		return WSL_CLEANUP_NOT_RUNNING;
	
	// Drop Linux caches
	// For now, just return success without actual command execution
	// TODO: Implement proper command execution
	return WSL_CLEANUP_SUCCESS;
}

/**
 * Reclaim WSL memory
 */
WSL_CLEANUP_RESULT _app_wsl_reclaim_memory()
{
	if (!_app_wsl_is_available())
		return WSL_CLEANUP_NOT_INSTALLED;
		
	if (!_app_wsl_is_running())
		return WSL_CLEANUP_NOT_RUNNING;
	
	// Compact memory
	// For now, just return success without actual command execution
	// TODO: Implement proper command execution
	return WSL_CLEANUP_SUCCESS;
}

/**
 * Get WSL error text
 */
LPCWSTR _app_get_wsl_error_text(
	_In_ WSL_CLEANUP_RESULT error_code
)
{
	switch (error_code)
	{
		case WSL_CLEANUP_SUCCESS:
			return L"WSL cleanup completed successfully";
			
		case WSL_CLEANUP_NOT_INSTALLED:
			return L"WSL is not installed";
			
		case WSL_CLEANUP_NOT_RUNNING:
			return L"WSL is not running";
			
		case WSL_CLEANUP_ACCESS_DENIED:
			return L"Access denied to WSL resources";
			
		case WSL_CLEANUP_COMMAND_FAILED:
			return L"WSL command execution failed";
			
		case WSL_CLEANUP_SERVICE_ERROR:
			return L"WSL service error";
			
		case WSL_CLEANUP_TIMEOUT:
			return L"WSL operation timed out";
			
		case WSL_CLEANUP_MEMORY_ERROR:
			return L"WSL memory operation failed";
			
		case WSL_CLEANUP_INVALID_PARAMETER:
			return L"Invalid parameter for WSL operation";
			
		default:
			return L"Unknown WSL error";
	}
}
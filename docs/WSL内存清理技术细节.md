# WSL内存清理功能技术细节

## 概述

本文档详细说明WSL内存清理功能的技术实现细节，包括API调用、错误处理、性能优化等关键技术点。

## 核心技术实现

### 1. WSL状态检测技术

#### 1.1 WSL安装检测
```c
static BOOL _app_is_wsl_installed(VOID)
{
    // 方法1：检查wsl.exe文件存在性
    WCHAR wsl_path[MAX_PATH];
    if (GetEnvironmentVariableW(L"SystemRoot", wsl_path, MAX_PATH))
    {
        _r_str_cat(wsl_path, MAX_PATH, L"\\System32\\wsl.exe");
        return _r_fs_exists(wsl_path);
    }
    
    // 方法2：检查WSL服务注册表项
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Appx", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        // 检查WSL相关包
        RegCloseKey(hKey);
        return TRUE;
    }
    
    return FALSE;
}
```

#### 1.2 WSL运行状态检测
```c
static BOOL _app_is_wsl_running(VOID)
{
    // 方法1：检查vmmem进程
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32W pe32 = { sizeof(PROCESSENTRY32W) };
        if (Process32FirstW(hSnapshot, &pe32))
        {
            do
            {
                if (_r_str_compare(pe32.szExeFile, L"vmmem.exe", -1) == 0)
                {
                    CloseHandle(hSnapshot);
                    return TRUE;
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    
    // 方法2：检查LxssManager服务状态
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager)
    {
        SC_HANDLE hService = OpenServiceW(hSCManager, L"LxssManager", SERVICE_QUERY_STATUS);
        if (hService)
        {
            SERVICE_STATUS_PROCESS ssp;
            DWORD dwBytesNeeded;
            
            if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &dwBytesNeeded))
            {
                CloseServiceHandle(hService);
                CloseServiceHandle(hSCManager);
                return ssp.dwCurrentState == SERVICE_RUNNING;
            }
            CloseServiceHandle(hService);
        }
        CloseServiceHandle(hSCManager);
    }
    
    return FALSE;
}
```

### 2. WSL内存清理技术

#### 2.1 Linux缓存清理机制
```c
typedef enum _WSL_CACHE_CLEAN_TYPE
{
    WSL_CACHE_CLEAN_PAGE = 1,      // 清理页面缓存
    WSL_CACHE_CLEAN_DENTRIES = 2,  // 清理目录项缓存
    WSL_CACHE_CLEAN_INODES = 4,    // 清理inode缓存
    WSL_CACHE_CLEAN_ALL = 7        // 清理所有缓存
} WSL_CACHE_CLEAN_TYPE;

static WSL_CLEANUP_RESULT _app_clean_wsl_cache_advanced(DWORD clean_type)
{
    WCHAR command[512];
    
    // 构建清理命令
    _r_str_printf(command, RTL_NUMBER_OF(command), 
        L"wsl -e sh -c 'echo %lu | sudo tee /proc/sys/vm/drop_caches'", 
        clean_type);
    
    // 执行命令并获取结果
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi = { 0 };
    
    if (CreateProcessW(NULL, command, NULL, NULL, FALSE, 
        CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi))
    {
        // 等待命令完成（设置超时）
        DWORD wait_result = WaitForSingleObject(pi.hProcess, 30000); // 30秒超时
        
        if (wait_result == WAIT_OBJECT_0)
        {
            DWORD exit_code;
            if (GetExitCodeProcess(pi.hProcess, &exit_code))
            {
                CloseHandle(pi.hThread);
                CloseHandle(pi.hProcess);
                return exit_code == 0 ? WSL_CLEANUP_SUCCESS : WSL_CLEANUP_COMMAND_FAILED;
            }
        }
        else
        {
            // 超时处理
            TerminateProcess(pi.hProcess, 1);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            return WSL_CLEANUP_COMMAND_FAILED;
        }
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    
    return WSL_CLEANUP_COMMAND_FAILED;
}
```

#### 2.2 WSL服务重启技术
```c
static WSL_CLEANUP_RESULT _app_restart_wsl_service_advanced(VOID)
{
    // 步骤1：优雅关闭WSL
    WCHAR shutdown_cmd[] = L"wsl --shutdown";
    
    STARTUPINFOW si = { sizeof(si) };
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION pi = { 0 };
    
    if (CreateProcessW(NULL, shutdown_cmd, NULL, NULL, FALSE, 
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, 60000); // 等待60秒
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        
        // 步骤2：重启LxssManager服务（如果需要）
        SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (hSCManager)
        {
            SC_HANDLE hService = OpenServiceW(hSCManager, L"LxssManager", 
                SERVICE_START | SERVICE_QUERY_STATUS);
            
            if (hService)
            {
                SERVICE_STATUS_PROCESS ssp;
                DWORD dwBytesNeeded;
                
                if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, 
                    (LPBYTE)&ssp, sizeof(ssp), &dwBytesNeeded))
                {
                    if (ssp.dwCurrentState != SERVICE_RUNNING)
                    {
                        // 启动服务
                        if (StartServiceW(hService, 0, NULL))
                        {
                            // 等待服务启动
                            for (int i = 0; i < 30; i++) // 最多等待30秒
                            {
                                Sleep(1000);
                                if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, 
                                    (LPBYTE)&ssp, sizeof(ssp), &dwBytesNeeded))
                                {
                                    if (ssp.dwCurrentState == SERVICE_RUNNING)
                                    {
                                        CloseServiceHandle(hService);
                                        CloseServiceHandle(hSCManager);
                                        return WSL_CLEANUP_SUCCESS;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        CloseServiceHandle(hService);
                        CloseServiceHandle(hSCManager);
                        return WSL_CLEANUP_SUCCESS;
                    }
                }
                CloseServiceHandle(hService);
            }
            CloseServiceHandle(hSCManager);
        }
    }
    
    return WSL_CLEANUP_SERVICE_ERROR;
}
```

### 3. 内存使用量监控技术

#### 3.1 vmmem进程内存获取
```c
typedef struct _WSL_MEMORY_INFO
{
    DWORD64 working_set_size;
    DWORD64 private_usage;
    DWORD64 peak_working_set;
    DWORD page_fault_count;
    DWORD process_count;
} WSL_MEMORY_INFO, *PWSL_MEMORY_INFO;

static BOOL _app_get_wsl_memory_info(PWSL_MEMORY_INFO memory_info)
{
    if (!memory_info)
        return FALSE;
        
    RtlZeroMemory(memory_info, sizeof(WSL_MEMORY_INFO));
    
    // 获取所有vmmem进程的内存信息
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE)
        return FALSE;
        
    PROCESSENTRY32W pe32 = { sizeof(PROCESSENTRY32W) };
    
    if (Process32FirstW(hSnapshot, &pe32))
    {
        do
        {
            // 检查是否为vmmem进程
            if (_r_str_compare(pe32.szExeFile, L"vmmem.exe", -1) == 0)
            {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                    FALSE, pe32.th32ProcessID);
                    
                if (hProcess)
                {
                    PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(pmc) };
                    
                    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
                    {
                        memory_info->working_set_size += pmc.WorkingSetSize;
                        memory_info->private_usage += pmc.PrivateUsage;
                        memory_info->peak_working_set += pmc.PeakWorkingSetSize;
                        memory_info->page_fault_count += pmc.PageFaultCount;
                        memory_info->process_count++;
                    }
                    
                    CloseHandle(hProcess);
                }
            }
        } while (Process32NextW(hSnapshot, &pe32));
    }
    
    CloseHandle(hSnapshot);
    return memory_info->process_count > 0;
}
```

#### 3.2 内存使用量趋势分析
```c
typedef struct _WSL_MEMORY_HISTORY
{
    DWORD64 timestamp[60];      // 60个时间点的历史数据
    DWORD64 memory_usage[60];   // 对应的内存使用量
    INT current_index;          // 当前索引
    INT sample_count;           // 采样数量
} WSL_MEMORY_HISTORY, *PWSL_MEMORY_HISTORY;

static VOID _app_update_memory_history(PWSL_MEMORY_HISTORY history, DWORD64 current_usage)
{
    if (!history)
        return;
        
    history->timestamp[history->current_index] = GetTickCount64();
    history->memory_usage[history->current_index] = current_usage;
    
    history->current_index = (history->current_index + 1) % 60;
    if (history->sample_count < 60)
        history->sample_count++;
}

static DOUBLE _app_calculate_memory_trend(PWSL_MEMORY_HISTORY history)
{
    if (!history || history->sample_count < 2)
        return 0.0;
        
    // 简单线性回归计算趋势
    DOUBLE sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
    INT n = history->sample_count;
    
    for (INT i = 0; i < n; i++)
    {
        DOUBLE x = (DOUBLE)i;
        DOUBLE y = (DOUBLE)history->memory_usage[i];
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }
    
    DOUBLE slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    return slope;
}
```

## 错误处理和日志记录

### 错误码定义
```c
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
```

### 错误处理函数
```c
static LPCWSTR _app_get_wsl_error_text(WSL_CLEANUP_RESULT result)
{
    switch (result)
    {
        case WSL_CLEANUP_SUCCESS:
            return L"WSL cleanup completed successfully";
            
        case WSL_CLEANUP_NOT_INSTALLED:
            return L"WSL is not installed on this system";
            
        case WSL_CLEANUP_NOT_RUNNING:
            return L"WSL is not currently running";
            
        case WSL_CLEANUP_ACCESS_DENIED:
            return L"Access denied. Administrator privileges required";
            
        case WSL_CLEANUP_COMMAND_FAILED:
            return L"WSL command execution failed";
            
        case WSL_CLEANUP_SERVICE_ERROR:
            return L"WSL service operation failed";
            
        case WSL_CLEANUP_TIMEOUT:
            return L"WSL operation timed out";
            
        case WSL_CLEANUP_MEMORY_ERROR:
            return L"Memory allocation failed";
            
        case WSL_CLEANUP_INVALID_PARAMETER:
            return L"Invalid parameter provided";
            
        default:
            return L"Unknown WSL cleanup error";
    }
}
```

### 日志记录系统
```c
static VOID _app_log_wsl_operation(LPCWSTR operation, WSL_CLEANUP_RESULT result, DWORD memory_before, DWORD memory_after)
{
    WCHAR log_buffer[512];
    
    if (result == WSL_CLEANUP_SUCCESS)
    {
        DWORD saved_memory = memory_before - memory_after;
        
        _r_str_printf(log_buffer, RTL_NUMBER_OF(log_buffer),
            L"WSL %s: Success, Memory saved: %s (Before: %s, After: %s)",
            operation,
            _r_format_bytesize64(saved_memory),
            _r_format_bytesize64(memory_before),
            _r_format_bytesize64(memory_after));
    }
    else
    {
        _r_str_printf(log_buffer, RTL_NUMBER_OF(log_buffer),
            L"WSL %s: Failed (0x%08X - %s)",
            operation,
            result,
            _app_get_wsl_error_text(result));
    }
    
    _r_log(LOG_LEVEL_INFO, NULL, L"WSL Operation", result, log_buffer);
}
```

## 性能优化

### 异步操作实现
```c
typedef struct _WSL_CLEANUP_CONTEXT
{
    HWND hwnd;
    ULONG cleanup_mask;
    WSL_CLEANUP_RESULT result;
    HANDLE hThread;
    DWORD thread_id;
} WSL_CLEANUP_CONTEXT, *PWSL_CLEANUP_CONTEXT;

static DWORD WINAPI _app_wsl_cleanup_thread(LPVOID lpParam)
{
    PWSL_CLEANUP_CONTEXT context = (PWSL_CLEANUP_CONTEXT)lpParam;
    
    if (!context)
        return ERROR_INVALID_PARAMETER;
        
    // 执行清理操作
    context->result = _app_perform_wsl_cleanup(context->cleanup_mask);
    
    // 发送完成消息
    if (context->hwnd)
    {
        PostMessage(context->hwnd, WM_WSL_CLEANUP_COMPLETE, 0, (LPARAM)context);
    }
    
    return ERROR_SUCCESS;
}

static BOOL _app_start_wsl_cleanup_async(HWND hwnd, ULONG cleanup_mask)
{
    PWSL_CLEANUP_CONTEXT context = (PWSL_CLEANUP_CONTEXT)_r_mem_allocate(sizeof(WSL_CLEANUP_CONTEXT));
    
    if (!context)
        return FALSE;
        
    context->hwnd = hwnd;
    context->cleanup_mask = cleanup_mask;
    context->result = WSL_CLEANUP_SUCCESS;
    
    context->hThread = CreateThread(NULL, 0, _app_wsl_cleanup_thread, context, 0, &context->thread_id);
    
    if (!context->hThread)
    {
        _r_mem_free(context);
        return FALSE;
    }
    
    return TRUE;
}
```

### 缓存机制
```c
static WSL_CACHE_STATUS _app_get_cached_wsl_status(VOID)
{
    static WSL_CACHE_STATUS cached_status = { 0 };
    static DWORD64 last_update = 0;
    DWORD64 current_time = GetTickCount64();
    
    // 缓存有效期：5秒
    if (current_time - last_update > 5000)
    {
        cached_status.is_installed = _app_is_wsl_installed();
        cached_status.is_running = _app_is_wsl_running();
        cached_status.memory_usage = _app_get_wsl_memory_usage();
        cached_status.last_check = current_time;
        
        last_update = current_time;
    }
    
    return cached_status;
}
```

## 安全考虑

### 权限管理
```c
static BOOL _app_check_wsl_permissions(VOID)
{
    // 检查管理员权限
    if (!_r_sys_iselevated())
    {
        return FALSE;
    }
    
    // 检查WSL命令执行权限
    HANDLE hToken;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
    {
        // 检查Token权限
        DWORD dwLength = 0;
        GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwLength);
        
        if (dwLength > 0)
        {
            PTOKEN_PRIVILEGES pPrivs = (PTOKEN_PRIVILEGES)_r_mem_allocate(dwLength);
            if (pPrivs)
            {
                if (GetTokenInformation(hToken, TokenPrivileges, pPrivs, dwLength, &dwLength))
                {
                    // 检查需要的权限
                    BOOL has_required_privileges = TRUE;
                    // ... 权限检查逻辑
                    
                    _r_mem_free(pPrivs);
                    CloseHandle(hToken);
                    return has_required_privileges;
                }
                _r_mem_free(pPrivs);
            }
        }
        CloseHandle(hToken);
    }
    
    return FALSE;
}
```

### 输入验证
```c
static BOOL _app_validate_wsl_command(LPCWSTR command)
{
    if (!command || _r_str_isempty(command))
        return FALSE;
        
    // 检查命令长度
    if (_r_str_length(command) > 1024)
        return FALSE;
        
    // 检查危险字符
    LPCWSTR dangerous_chars[] = { L"&", L"|", L">", L"<", L";", NULL };
    
    for (INT i = 0; dangerous_chars[i]; i++)
    {
        if (_r_str_find(command, -1, dangerous_chars[i]) != -1)
            return FALSE;
    }
    
    // 检查命令白名单
    LPCWSTR allowed_commands[] = { 
        L"wsl -e sh -c 'echo 1 | sudo tee /proc/sys/vm/drop_caches'",
        L"wsl -e sh -c 'echo 2 | sudo tee /proc/sys/vm/drop_caches'",
        L"wsl -e sh -c 'echo 3 | sudo tee /proc/sys/vm/drop_caches'",
        L"wsl --shutdown",
        NULL 
    };
    
    for (INT i = 0; allowed_commands[i]; i++)
    {
        if (_r_str_compare(command, allowed_commands[i], -1) == 0)
            return TRUE;
    }
    
    return FALSE;
}
```

## 兼容性处理

### WSL版本检测
```c
typedef enum _WSL_VERSION
{
    WSL_VERSION_UNKNOWN = 0,
    WSL_VERSION_1 = 1,
    WSL_VERSION_2 = 2
} WSL_VERSION;

static WSL_VERSION _app_detect_wsl_version(VOID)
{
    WCHAR command[] = L"wsl -l -v";
    WCHAR output[4096] = { 0 };
    
    // 执行命令并获取输出
    if (_app_execute_command_with_output(command, output, RTL_NUMBER_OF(output)))
    {
        // 分析输出内容
        if (_r_str_find(output, -1, L"VERSION") != -1)
        {
            // 查找版本信息
            LPCWSTR version_str = _r_str_find(output, -1, L"2");
            if (version_str)
                return WSL_VERSION_2;
                
            version_str = _r_str_find(output, -1, L"1");
            if (version_str)
                return WSL_VERSION_1;
        }
    }
    
    return WSL_VERSION_UNKNOWN;
}
```

### 发行版检测
```c
typedef struct _WSL_DISTRO_INFO
{
    WCHAR name[64];
    WCHAR version[32];
    BOOL is_running;
    WSL_VERSION wsl_version;
} WSL_DISTRO_INFO, *PWSL_DISTRO_INFO;

static BOOL _app_enum_wsl_distros(PWSL_DISTRO_INFO distro_array, DWORD array_size, PDWORD distro_count)
{
    if (!distro_array || !distro_count)
        return FALSE;
        
    *distro_count = 0;
    
    WCHAR command[] = L"wsl -l -v";
    WCHAR output[8192] = { 0 };
    
    if (!_app_execute_command_with_output(command, output, RTL_NUMBER_OF(output)))
        return FALSE;
        
    // 解析输出内容
    LPCWSTR line = output;
    DWORD count = 0;
    
    // 跳过标题行
    while (*line && *line != L'\n') line++;
    if (*line == L'\n') line++;
    
    // 解析每一行
    while (*line && count < array_size)
    {
        // 解析发行版信息
        if (_r_str_parse_wsl_line(line, &distro_array[count]))
        {
            count++;
        }
        
        // 移动到下一行
        while (*line && *line != L'\n') line++;
        if (*line == L'\n') line++;
    }
    
    *distro_count = count;
    return TRUE;
}
```

## 性能监控和优化

### 性能计数器
```c
static VOID _app_initialize_wsl_performance_counters(VOID)
{
    // 创建性能计数器
    PDH_STATUS status;
    
    // WSL内存使用计数器
    status = PdhAddCounterW(g_wsl_query, L"\\Process(vmmem)\\Working Set", 0, &g_wsl_memory_counter);
    if (status != ERROR_SUCCESS)
    {
        _r_log(LOG_LEVEL_WARNING, NULL, L"WSL Performance Counter", status, L"Failed to add memory counter");
    }
    
    // WSL CPU使用计数器
    status = PdhAddCounterW(g_wsl_query, L"\\Process(vmmem)\\% Processor Time", 0, &g_wsl_cpu_counter);
    if (status != ERROR_SUCCESS)
    {
        _r_log(LOG_LEVEL_WARNING, NULL, L"WSL Performance Counter", status, L"Failed to add CPU counter");
    }
}

static BOOL _app_collect_wsl_performance_data(PWSL_PERFORMANCE_DATA perf_data)
{
    if (!perf_data)
        return FALSE;
        
    PDH_FMT_COUNTERVALUE counter_value;
    PDH_STATUS status;
    
    // 收集内存数据
    status = PdhGetFormattedCounterValue(g_wsl_memory_counter, PDH_FMT_LARGE, NULL, &counter_value);
    if (status == ERROR_SUCCESS)
    {
        perf_data->memory_usage = counter_value.largeValue;
    }
    else
    {
        return FALSE;
    }
    
    // 收集CPU数据
    status = PdhGetFormattedCounterValue(g_wsl_cpu_counter, PDH_FMT_DOUBLE, NULL, &counter_value);
    if (status == ERROR_SUCCESS)
    {
        perf_data->cpu_usage = counter_value.doubleValue;
    }
    else
    {
        perf_data->cpu_usage = 0.0;
    }
    
    return TRUE;
}
```

### 内存优化
```c
static VOID _app_optimize_wsl_memory_usage(VOID)
{
    // 获取当前内存使用状态
    WSL_MEMORY_INFO memory_info;
    if (!_app_get_wsl_memory_info(&memory_info))
        return;
        
    // 根据内存使用情况决定优化策略
    DWORD64 total_memory = memory_info.working_set_size;
    DWORD64 threshold_high = 2ULL * 1024 * 1024 * 1024; // 2GB
    DWORD64 threshold_low = 512ULL * 1024 * 1024; // 512MB
    
    if (total_memory > threshold_high)
    {
        // 高内存使用：执行全面清理
        _app_perform_wsl_cleanup(REDUCT_WSL_CACHE_CLEAN | REDUCT_WSL_MEMORY_RECLAIM);
    }
    else if (total_memory > threshold_low)
    {
        // 中等内存使用：执行轻度清理
        _app_perform_wsl_cleanup(REDUCT_WSL_CACHE_CLEAN);
    }
    else
    {
        // 低内存使用：跳过清理
        _r_log(LOG_LEVEL_INFO, NULL, L"WSL Memory", 0, L"Memory usage is normal, skipping cleanup");
    }
}
```

## 故障排除指南

### 常见问题诊断
```c
static VOID _app_diagnose_wsl_issues(VOID)
{
    // 检查1：WSL安装状态
    if (!_app_is_wsl_installed())
    {
        _r_log(LOG_LEVEL_ERROR, NULL, L"WSL Diagnosis", 0, L"WSL is not installed");
        return;
    }
    
    // 检查2：WSL服务状态
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager)
    {
        SC_HANDLE hService = OpenServiceW(hSCManager, L"LxssManager", SERVICE_QUERY_STATUS);
        if (hService)
        {
            SERVICE_STATUS_PROCESS ssp;
            DWORD dwBytesNeeded;
            
            if (QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, 
                (LPBYTE)&ssp, sizeof(ssp), &dwBytesNeeded))
            {
                if (ssp.dwCurrentState != SERVICE_RUNNING)
                {
                    _r_log(LOG_LEVEL_WARNING, NULL, L"WSL Diagnosis", 0, 
                        L"LxssManager service is not running (State: %lu)", ssp.dwCurrentState);
                }
            }
            CloseServiceHandle(hService);
        }
        else
        {
            _r_log(LOG_LEVEL_ERROR, NULL, L"WSL Diagnosis", GetLastError(), 
                L"Failed to open LxssManager service");
        }
        CloseServiceHandle(hSCManager);
    }
    
    // 检查3：WSL命令可用性
    WCHAR test_output[256] = { 0 };
    if (_app_execute_command_with_output(L"wsl -e echo test", test_output, RTL_NUMBER_OF(test_output)))
    {
        if (_r_str_find(test_output, -1, L"test") == -1)
        {
            _r_log(LOG_LEVEL_ERROR, NULL, L"WSL Diagnosis", 0, L"WSL command execution failed");
        }
    }
    else
    {
        _r_log(LOG_LEVEL_ERROR, NULL, L"WSL Diagnosis", 0, L"WSL command is not available");
    }
}
```

## 总结

本文档详细阐述了WSL内存清理功能的技术实现细节，包括状态检测、内存清理、性能监控、错误处理等关键技术点。通过合理的设计和实现，可以确保功能的稳定性、安全性和高效性。在实际开发过程中，需要根据具体情况进行调整和优化，确保功能在各种环境下都能正常工作。
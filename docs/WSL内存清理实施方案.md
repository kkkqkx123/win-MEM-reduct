# WSL内存清理功能实施方案

## 概述

基于《WSL内存清理实现方案》文档，制定详细的代码实施计划。本方案将在Mem Reduct现有框架基础上，扩展支持WSL内存清理功能。

## 现有代码架构分析

### 核心文件结构
- **main.h**: 定义内存清理标志位和枚举类型
- **main.c**: 包含主逻辑和内存清理函数 `_app_memoryclean`
- **resource.h**: 定义资源ID和字符串ID
- **resource.rc**: 定义UI资源和对话框布局
- **routine/**: 包含辅助函数库

### 现有内存清理机制
- 使用位掩码标志位（REDUCT_*）控制清理类型
- `_app_memoryclean` 函数处理所有内存清理操作
- 设置对话框通过 `SettingsProc` 函数管理配置
- 支持自动清理、手动清理、热键清理等多种触发方式

## 需要修改的文件列表

### 1. 头文件扩展（main.h）
**新增WSL相关定义：**
```c
// WSL内存清理标志位
#define REDUCT_WSL_CACHE_CLEAN      0x100
#define REDUCT_WSL_SERVICE_RESTART  0x200
#define REDUCT_WSL_MEMORY_RECLAIM   0x400

#define REDUCT_WSL_MASK_ALL (REDUCT_WSL_CACHE_CLEAN | REDUCT_WSL_SERVICE_RESTART | REDUCT_WSL_MEMORY_RECLAIM)

// WSL清理结果枚举
typedef enum _WSL_CLEANUP_RESULT
{
    WSL_CLEANUP_SUCCESS = 0,
    WSL_CLEANUP_NOT_INSTALLED,
    WSL_CLEANUP_NOT_RUNNING,
    WSL_CLEANUP_ACCESS_DENIED,
    WSL_CLEANUP_COMMAND_FAILED,
    WSL_CLEANUP_SERVICE_ERROR
} WSL_CLEANUP_RESULT;
```

### 2. 主程序文件（main.c）
**新增WSL相关函数：**
```c
// WSL功能检测函数
static BOOL _app_is_wsL_installed(VOID);
static BOOL _app_is_wsl_running(VOID);
static DWORD _app_get_wsl_memory_usage(VOID);

// WSL清理函数
static WSL_CLEANUP_RESULT _app_clean_wsl_cache(VOID);
static WSL_CLEANUP_RESULT _app_restart_wsl_service(VOID);
static WSL_CLEANUP_RESULT _app_reclaim_wsl_memory(VOID);

// 扩展内存清理函数
static VOID _app_memoryclean_wsl(ULONG mask);
```

**修改现有 `_app_memoryclean` 函数：**
```c
VOID _app_memoryclean(
    _In_opt_ HWND hwnd,
    _In_ CLEANUP_SOURCE_ENUM src,
    _In_opt_ ULONG mask
)
{
    // 现有内存清理代码...
    
    // 新增WSL内存清理
    if (mask & REDUCT_WSL_MASK_ALL)
    {
        _app_memoryclean_wsl(mask);
    }
}
```

### 3. 资源头文件（resource.h）
**新增资源ID定义：**
```c
// WSL设置页面
#define IDD_SETTINGS_WSL        106

// WSL控件ID
#define IDC_WSL_ENABLE_CHK      191
#define IDC_WSL_CACHE_CLEAN_CHK 192
#define IDC_WSL_RESTART_CHK     193
#define IDC_WSL_MEMORY_RECLAIM_CHK 194
#define IDC_WSL_MEMORY_USAGE    195
#define IDC_WSL_STATUS          196

// 菜单项ID
#define IDM_WSL_CLEAN           197
#define IDM_WSL_SETTINGS        198

// 字符串资源ID
#define IDS_SETTINGS_WSL        55
#define IDS_WSL_ENABLE_CHK      56
#define IDS_WSL_CACHE_CLEAN_CHK 57
#define IDS_WSL_RESTART_CHK     58
#define IDS_WSL_MEMORY_RECLAIM_CHK 59
#define IDS_WSL_STATUS_RUNNING  60
#define IDS_WSL_STATUS_STOPPED  61
#define IDS_WSL_STATUS_NOT_INSTALLED 62
#define IDS_WSL_CLEANUP_SUCCESS 63
#define IDS_WSL_CLEANUP_FAILED  64
```

### 4. 资源文件（resource.rc）
**新增WSL设置页面：**
```rc
IDD_SETTINGS_WSL DIALOGEX 0, 0, 260, 168
STYLE WS_CHILD | WS_SYSMENU | WS_TABSTOP | WS_GROUP | DS_SHELLFONT | DS_CONTROL
EXSTYLE WS_EX_CONTROLPARENT
FONT 8, "Ms Shell Dlg"
{
    GROUPBOX        "", IDC_TITLE_WSL, 0, 0, 260, 60
    
    AUTOCHECKBOX    "", IDC_WSL_ENABLE_CHK, 8, 12, 244, 10
    AUTOCHECKBOX    "", IDC_WSL_CACHE_CLEAN_CHK, 8, 24, 244, 10
    AUTOCHECKBOX    "", IDC_WSL_RESTART_CHK, 8, 36, 244, 10
    AUTOCHECKBOX    "", IDC_WSL_MEMORY_RECLAIM_CHK, 8, 48, 244, 10
    
    GROUPBOX        "", IDC_TITLE_WSL_STATUS, 0, 64, 260, 50
    
    LTEXT           "", IDC_WSL_STATUS, 8, 76, 244, 20, SS_LEFT
    LTEXT           "", IDC_WSL_MEMORY_USAGE, 8, 92, 244, 12, SS_LEFT
}
```

**扩展主菜单和托盘菜单：**
```rc
// 主菜单添加WSL选项
POPUP ""
{
    MENUITEM "", IDM_WSL_SETTINGS
    MENUITEM SEPARATOR
}

// 托盘菜单扩展
MENUITEM "", IDM_TRAY_WSL_CLEAN
```

### 5. 配置文件处理
**新增配置项：**
```c
// WSL功能启用状态
#define CONFIG_WSL_ENABLE L"WslEnable"
#define CONFIG_WSL_CLEAN_CACHE L"WslCleanCache"
#define CONFIG_WSL_RESTART_SERVICE L"WslRestartService"
#define CONFIG_WSL_MEMORY_RECLAIM L"WslMemoryReclaim"
#define CONFIG_WSL_AUTO_CLEAN L"WslAutoClean"
#define CONFIG_WSL_CLEAN_THRESHOLD L"WslCleanThreshold"
```

## 具体实施步骤

### 第一阶段：基础功能实现（1-2周）

#### 步骤1：添加WSL检测功能
```c
// 检测WSL是否安装
static BOOL _app_is_wsl_installed(VOID)
{
    // 检查wsl.exe命令是否存在
    WCHAR wsl_path[MAX_PATH];
    if (GetEnvironmentVariableW(L"SystemRoot", wsl_path, MAX_PATH))
    {
        _r_str_cat(wsl_path, MAX_PATH, L"\\System32\\wsl.exe");
        return _r_fs_exists(wsl_path);
    }
    return FALSE;
}

// 检测WSL是否运行
static BOOL _app_is_wsl_running(VOID)
{
    // 检查vmmem进程是否存在
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
    return FALSE;
}
```

#### 步骤2：实现WSL清理功能
```c
// 清理WSL文件缓存
static WSL_CLEANUP_RESULT _app_clean_wsl_cache(VOID)
{
    // 执行Linux缓存清理命令
    WCHAR command[256] = L"wsl -e sh -c 'echo 3 | sudo tee /proc/sys/vm/drop_caches'";
    
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    if (CreateProcessW(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        
        return exit_code == 0 ? WSL_CLEANUP_SUCCESS : WSL_CLEANUP_COMMAND_FAILED;
    }
    
    return WSL_CLEANUP_COMMAND_FAILED;
}

// 重启WSL服务
static WSL_CLEANUP_RESULT _app_restart_wsl_service(VOID)
{
    // 执行wsl --shutdown命令
    WCHAR command[] = L"wsl --shutdown";
    
    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    
    if (CreateProcessW(NULL, command, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        WaitForSingleObject(pi.hProcess, INFINITE);
        
        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        
        return exit_code == 0 ? WSL_CLEANUP_SUCCESS : WSL_CLEANUP_SERVICE_ERROR;
    }
    
    return WSL_CLEANUP_SERVICE_ERROR;
}
```

#### 步骤3：集成到现有清理框架
```c
// WSL内存清理主函数
static VOID _app_memoryclean_wsl(ULONG mask)
{
    if (!_app_is_wsl_installed())
    {
        _r_log(LOG_LEVEL_WARNING, NULL, L"WSL", 0, L"WSL is not installed");
        return;
    }
    
    if (!_app_is_wsl_running())
    {
        _r_log(LOG_LEVEL_INFO, NULL, L"WSL", 0, L"WSL is not running");
        return;
    }
    
    WSL_CLEANUP_RESULT result = WSL_CLEANUP_SUCCESS;
    
    if (mask & REDUCT_WSL_CACHE_CLEAN)
    {
        result = _app_clean_wsl_cache();
        if (result != WSL_CLEANUP_SUCCESS)
        {
            _r_log(LOG_LEVEL_ERROR, NULL, L"WSL Cache Clean", result, L"Failed to clean WSL cache");
        }
    }
    
    if (mask & REDUCT_WSL_SERVICE_RESTART)
    {
        result = _app_restart_wsl_service();
        if (result != WSL_CLEANUP_SUCCESS)
        {
            _r_log(LOG_LEVEL_ERROR, NULL, L"WSL Service Restart", result, L"Failed to restart WSL service");
        }
    }
}
```

### 第二阶段：用户界面扩展（1-2周）

#### 步骤4：扩展设置对话框
```c
// 在SettingsProc函数中添加WSL页面处理
case IDD_SETTINGS_WSL:
{
    // 初始化WSL设置控件
    _r_ctrl_checkbutton(hwnd, IDC_WSL_ENABLE_CHK, _r_config_getboolean(CONFIG_WSL_ENABLE, FALSE, NULL));
    _r_ctrl_checkbutton(hwnd, IDC_WSL_CACHE_CLEAN_CHK, _r_config_getboolean(CONFIG_WSL_CLEAN_CACHE, TRUE, NULL));
    _r_ctrl_checkbutton(hwnd, IDC_WSL_RESTART_CHK, _r_config_getboolean(CONFIG_WSL_RESTART_SERVICE, FALSE, NULL));
    _r_ctrl_checkbutton(hwnd, IDC_WSL_MEMORY_RECLAIM_CHK, _r_config_getboolean(CONFIG_WSL_MEMORY_RECLAIM, TRUE, NULL));
    
    // 更新WSL状态显示
    _app_update_wsl_status(hwnd);
    break;
}
```

#### 步骤5：添加WSL状态显示
```c
// 更新WSL状态信息
static VOID _app_update_wsl_status(HWND hwnd)
{
    WCHAR status_text[256] = { 0 };
    WCHAR memory_text[128] = { 0 };
    
    if (_app_is_wsl_installed())
    {
        if (_app_is_wsl_running())
        {
            _r_str_printf(status_text, RTL_NUMBER_OF(status_text), L"WSL Status: Running");
            
            DWORD memory_usage = _app_get_wsl_memory_usage();
            if (memory_usage > 0)
            {
                _r_format_bytesize64(memory_text, RTL_NUMBER_OF(memory_text), memory_usage);
                _r_str_cat(status_text, RTL_NUMBER_OF(status_text), L"\r\nMemory Usage: ");
                _r_str_cat(status_text, RTL_NUMBER_OF(status_text), memory_text);
            }
        }
        else
        {
            _r_str_printf(status_text, RTL_NUMBER_OF(status_text), L"WSL Status: Stopped");
        }
    }
    else
    {
        _r_str_printf(status_text, RTL_NUMBER_OF(status_text), L"WSL Status: Not Installed");
    }
    
    _r_ctrl_setstring(hwnd, IDC_WSL_STATUS, status_text);
}
```

### 第三阶段：高级功能实现（1-2周）

#### 步骤6：实现内存监控功能
```c
// 获取WSL内存使用量
static DWORD _app_get_wsl_memory_usage(VOID)
{
    // 通过vmmem进程获取内存使用量
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
                    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
                    if (hProcess)
                    {
                        PROCESS_MEMORY_COUNTERS_EX pmc = { sizeof(pmc) };
                        if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
                        {
                            CloseHandle(hProcess);
                            CloseHandle(hSnapshot);
                            return pmc.WorkingSetSize;
                        }
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    return 0;
}
```

#### 步骤7：实现自动清理功能
```c
// 检查WSL内存使用情况并自动清理
static VOID _app_check_wsl_auto_cleanup(VOID)
{
    if (!_r_config_getboolean(CONFIG_WSL_ENABLE, FALSE, NULL))
        return;
    
    if (!_r_config_getboolean(CONFIG_WSL_AUTO_CLEAN, FALSE, NULL))
        return;
    
    DWORD threshold = _r_config_getulong(CONFIG_WSL_CLEAN_THRESHOLD, 2048, NULL) * 1024 * 1024; // MB to bytes
    DWORD current_usage = _app_get_wsl_memory_usage();
    
    if (current_usage > threshold)
    {
        ULONG mask = 0;
        
        if (_r_config_getboolean(CONFIG_WSL_CLEAN_CACHE, TRUE, NULL))
            mask |= REDUCT_WSL_CACHE_CLEAN;
        
        if (_r_config_getboolean(CONFIG_WSL_MEMORY_RECLAIM, TRUE, NULL))
            mask |= REDUCT_WSL_MEMORY_RECLAIM;
        
        _app_memoryclean_wsl(mask);
    }
}
```

## 安全考虑

### 权限管理
- WSL缓存清理需要Linux系统内的sudo权限
- WSL服务重启需要Windows管理员权限
- 提供权限检查和用户提示

### 错误处理
- 完善的错误码定义和处理机制
- 操作日志记录，便于故障排查
- 用户友好的错误提示信息

### 安全策略
- 清理操作前显示确认对话框
- 支持选择性清理，避免误操作
- 提供操作撤销和配置恢复功能

## 兼容性要求

### 系统要求
- Windows 10版本1903或更高版本
- WSL2功能已启用
- 适用于所有支持的Linux发行版

### 功能可用性
- 基础功能支持所有WSL版本
- 高级功能需要WSL 0.67.6+版本
- 某些功能需要Windows 11系统

## 测试计划

### 功能测试
1. WSL安装状态检测准确性
2. WSL运行状态检测准确性
3. 缓存清理功能有效性
4. 服务重启功能可靠性
5. 内存使用量获取准确性

### 兼容性测试
1. 不同Windows版本兼容性
2. 不同WSL发行版兼容性
3. 权限不足情况下的处理
4. 异常情况下的错误处理

### 性能测试
1. 清理操作执行时间
2. 内存监控功能资源消耗
3. 自动清理触发机制准确性

## 部署计划

### 第一阶段（第1-2周）
- 完成基础WSL检测和清理功能
- 集成到现有内存清理框架
- 基础UI界面实现

### 第二阶段（第3-4周）
- 完善用户界面和配置管理
- 实现高级功能（自动清理、内存监控）
- 错误处理和日志记录

### 第三阶段（第5-6周）
- 全面测试和bug修复
- 性能优化和代码重构
- 文档编写和用户指南

## 风险评估与缓解

### 技术风险
- **权限问题**: 某些操作需要管理员权限
  - 缓解: 提供权限检查和用户提示
- **兼容性问题**: 不同WSL版本功能差异
  - 缓解: 功能检测和版本适配
- **稳定性问题**: 清理操作可能影响系统稳定性
  - 缓解: 保守的默认配置和确认机制

### 项目风险
- **开发时间超期**: 功能复杂度超出预期
  - 缓解: 分阶段实施，优先核心功能
- **测试不充分**: 兼容性问题
  - 缓解: 制定详细测试计划，多环境验证

## 结论

本实施方案基于Mem Reduct现有架构，通过分阶段的方式逐步集成WSL内存清理功能。方案考虑了安全性、兼容性和用户体验，提供了完整的功能实现路径和风险缓解措施。通过合理的开发计划和测试策略，可以确保功能的稳定性和可靠性。
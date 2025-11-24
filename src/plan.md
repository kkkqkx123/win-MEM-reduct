基于对main.c文件的完整分析，以下是可以拆分的主要部分：

## 立即可拆分的无状态函数（适合放入头文件）

**工具函数类：**
- `_app_getlimitvalue()` - 简单返回数组值
- `_app_getintervalvalue()` - 简单返回数组值
- `_app_generate_menu()` - 纯UI生成逻辑（已标注移至ui_utils.c）

## 常量定义分离

**创建constants.h包含：**
- 所有`REDUCT_*`掩码定义
- 限制数组：`limits_arr[]`
- 间隔数组：`intervals_arr[]`
- 各种配置键名字符串常量（如`L"AutoreductEnable"`等）

## 大型组件独立成模块

**1. 设置对话框（约500行）**
分离到`settings_dialog.c`，包含：
- 所有`IDD_SETTINGS_*`对话框的处理逻辑
- `SettingsProc`函数完整实现
- 设置页面的初始化和事件处理

**2. 内存清理核心逻辑**
分离到`memory_cleaner.c`，包含：
- `_app_memoryclean()`函数实现
- `_app_flushvolumecache()`等相关函数
- 各种内存区域清理的具体实现

**3. 托盘菜单管理**
分离到`tray_manager.c`，包含：
- 所有`IDM_TRAY_*`相关处理逻辑
- 托盘图标更新逻辑
- 右键菜单生成和管理

**4. 命令行处理**
分离到`cmdline_parser.c`，包含：
- `_app_parseargs()`函数
- 命令行参数解析逻辑

## 主文件重构后结构

重构后的`main.c`将专注于：
- 应用程序初始化（`_app_initialize`）
- 主对话框生命周期管理（`DlgProc`的核心框架）
- 消息分发和主要流程控制
- 热键管理（`_app_hotkeyinit`）

## 具体拆分优先级

**高优先级（立即执行）：**
1. 创建`constants.h` - 常量定义
2. 创建`utils.h/utils.c` - 无状态工具函数
3. 分离`settings_dialog.c` - 设置对话框（最独立的代码块）

**中优先级：**
4. 分离`memory_cleaner.c` - 内存清理逻辑
5. 分离`tray_manager.c` - 托盘管理
6. 分离`cmdline_parser.c` - 命令行处理

这样的重构可以将2500行的main.c减少到500-800行，每个文件专注特定功能，大大提高代码的可维护性和可读性。

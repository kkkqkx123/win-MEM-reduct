# UI字符显示配置指南

本文档说明如何在MemReduct项目中正确配置设置项和右键菜单的字符显示。

## 字符显示配置位置

### 1. 资源文件定义

#### 字符串ID定义 (`resource.h`)
所有UI字符都需要在`resource.h`中定义唯一的字符串ID：

```c
// 设置项字符串ID
#define IDS_WSL_CACHE_CLEAN_CHK 191
#define IDS_WSL_MEMORY_RECLAIM_CHK 192

// 托盘菜单字符串ID  
#define IDS_TRAY_SHOW 12
#define IDS_TRAY_CLEAN 203
#define IDS_TRAY_SETTINGS 204
#define IDS_TRAY_WEBSITE 205
#define IDS_TRAY_ABOUT 206
#define IDS_TRAY_EXIT 207
#define IDS_TRAY_POPUP_1 14    // "Clean areas"
#define IDS_TRAY_DISABLE 13    // "Disable"
```

#### 字符串内容定义 (`resource.rc`)
在`resource.rc`的STRINGTABLE部分定义实际的字符串内容：

```rc
STRINGTABLE
{
    // WSL设置项
    IDS_WSL_CACHE_CLEAN_CHK "WSL cache clean"
    IDS_WSL_MEMORY_RECLAIM_CHK "WSL memory reclaim"
    
    // 托盘菜单项
    IDS_TRAY_CLEAN "Clean memory"
    IDS_TRAY_SETTINGS "Settings..."
    IDS_TRAY_WEBSITE "Website" 
    IDS_TRAY_ABOUT "About"
    IDS_TRAY_EXIT "Exit"
    IDS_TRAY_POPUP_1 "Clean areas"
    IDS_TRAY_DISABLE "Disable"
}
```

### 2. 菜单资源定义 (`resource.rc`)

#### 托盘菜单资源
托盘菜单(IDM_TRAY)中的菜单项文本必须留空(`" "`)，因为文本将通过代码动态设置：

```rc
IDM_TRAY MENU
{
    POPUP ""
    {
        POPUP " "  // 内存清理子菜单 - 文本留空，通过代码设置
        {
            MENUITEM " ", IDM_WORKINGSET_CHK
            MENUITEM " ", IDM_SYSTEMFILECACHE_CHK
            MENUITEM " ", IDM_WSL_CACHE_CLEAN_CHK    // WSL选项
            MENUITEM " ", IDM_WSL_MEMORY_RECLAIM_CHK  // WSL选项
        }
        MENUITEM " ", IDM_TRAY_SETTINGS  // 设置菜单项
        MENUITEM " ", IDM_TRAY_WEBSITE   // 网站菜单项
        MENUITEM " ", IDM_TRAY_ABOUT     // 关于菜单项
        MENUITEM " ", IDM_TRAY_EXIT     // 退出菜单项
    }
}
```

#### 主菜单资源
主菜单(IDM_MAIN)可以直接在资源中定义文本，因为会通过RM_LOCALIZE消息处理：

```rc
IDM_MAIN MENU
{
    POPUP "&File"
    {
        MENUITEM "E&xit", IDM_EXIT
    }
    POPUP "&View" 
    {
        MENUITEM "&Always on top", IDM_ALWAYSONTOP_CHK
    }
}
```

### 3. 代码中的本地化实现

#### 设置对话框本地化 (`settings_dialog.c`)
在`RM_LOCALIZE`消息处理中设置设置项文本：

```c
case RM_LOCALIZE:
{
    // 设置WSL选项文本
    _r_listview_setitem (hwnd, IDC_LISTVIEW, 8, 0, _r_locale_getstring (IDS_WSL_CACHE_CLEAN_CHK));
    _r_listview_setitem (hwnd, IDC_LISTVIEW, 9, 0, _r_locale_getstring (IDS_WSL_MEMORY_RECLAIM_CHK));
    break;
}
```

#### 托盘菜单本地化 (`tray_manager.c`)
在`_app_tray_menu_create`函数中设置所有菜单项文本：

```c
VOID _app_tray_menu_create (_In_ HWND hwnd)
{
    // ... 菜单创建代码 ...
    
    // 本地化主托盘菜单项
    _r_menu_setitemtext (htraymenu, IDM_TRAY_SHOW, FALSE, _r_locale_getstring (IDS_TRAY_SHOW));
    _r_menu_setitemtext (htraymenu, IDM_TRAY_CLEAN, FALSE, _r_locale_getstring (IDS_TRAY_CLEAN));
    _r_menu_setitemtext (htraymenu, IDM_TRAY_SETTINGS, FALSE, _r_locale_getstring (IDS_TRAY_SETTINGS));
    
    // 本地化内存清理子菜单
    HMENU hsubmenu = GetSubMenu (htraymenu, 0);
    if (hsubmenu)
    {
        // 设置子菜单标题
        _r_menu_setitemtext (hsubmenu, 0, TRUE, _r_locale_getstring (IDS_TRAY_POPUP_1));
        
        // 设置所有内存清理选项
        _r_menu_setitemtext (hsubmenu, IDM_WORKINGSET_CHK, FALSE, _r_locale_getstring (IDS_WORKINGSET_CHK));
        _r_menu_setitemtext (hsubmenu, IDM_WSL_CACHE_CLEAN_CHK, FALSE, _r_locale_getstring (IDS_WSL_CACHE_CLEAN_CHK));
        _r_menu_setitemtext (hsubmenu, IDM_WSL_MEMORY_RECLAIM_CHK, FALSE, _r_locale_getstring (IDS_WSL_MEMORY_RECLAIM_CHK));
    }
}
```

#### 主菜单本地化 (`main.c`)
在`RM_LOCALIZE`消息处理中设置主菜单文本：

```c
case RM_LOCALIZE:
{
    HMENU hmenu = GetMenu (hwnd);
    if (hmenu)
    {
        // 本地化文件菜单
        _r_menu_setitemtext (hmenu, 0, TRUE, _r_locale_getstring (IDS_FILE));
        _r_menu_setitemtext (hmenu, IDM_EXIT, FALSE, _r_locale_getstring (IDS_EXIT));
        
        // 本地化视图菜单
        _r_menu_setitemtext (hmenu, 1, TRUE, _r_locale_getstring (IDS_VIEW));
        _r_menu_setitemtext (hmenu, IDM_ALWAYSONTOP_CHK, FALSE, _r_locale_getstring (IDS_ALWAYSONTOP_CHK));
    }
    break;
}
```

## 配置 checklist

添加新的UI字符时，确保完成以下步骤：

1. **定义字符串ID** - 在`resource.h`中添加唯一的ID定义
2. **定义字符串内容** - 在`resource.rc`的STRINGTABLE中添加实际文本
3. **更新菜单资源** - 在`resource.rc`中添加相应的菜单项（文本留空）
4. **添加本地化代码** - 在相应的.c文件中添加`_r_locale_getstring`调用
5. **测试显示** - 编译并测试所有字符是否正确显示

## 常见问题

### 右键菜单显示空白
**原因**: 托盘菜单资源中的文本留空，但没有在代码中设置本地化文本
**解决**: 确保在`tray_manager.c`的`_app_tray_menu_create`函数中添加所有菜单项的本地化代码

### 设置项不显示文本
**原因**: 没有在`settings_dialog.c`的RM_LOCALIZE处理中设置列表项文本
**解决**: 确保在`RM_LOCALIZE`消息处理中添加`_r_listview_setitem`调用

### 编译错误：字符串ID重复定义
**原因**: 在`resource.h`或`resource.rc`中重复定义了相同的字符串ID
**解决**: 检查现有定义，避免重复添加相同的ID
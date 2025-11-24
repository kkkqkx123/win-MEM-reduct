// Mem Reduct
// Copyright (c) 2011-2025 Henry++

#pragma once

// Memory reduction masks
#define REDUCT_WORKING_SET                0x01
#define REDUCT_SYSTEM_FILE_CACHE          0x02
#define REDUCT_STANDBY_PRIORITY0_LIST     0x04
#define REDUCT_STANDBY_LIST               0x08
#define REDUCT_MODIFIED_LIST              0x10
#define REDUCT_COMBINE_MEMORY_LISTS       0x20
#define REDUCT_REGISTRY_CACHE             0x40
#define REDUCT_MODIFIED_FILE_CACHE        0x80
#define REDUCT_WSL_CACHE_CLEAN            0x100
#define REDUCT_WSL_MEMORY_RECLAIM          0x200

// Combined masks
#define REDUCT_MASK_ALL                   (REDUCT_WORKING_SET | \
                                          REDUCT_SYSTEM_FILE_CACHE | \
                                          REDUCT_STANDBY_PRIORITY0_LIST | \
                                          REDUCT_STANDBY_LIST | \
                                          REDUCT_MODIFIED_LIST | \
                                          REDUCT_COMBINE_MEMORY_LISTS | \
                                          REDUCT_REGISTRY_CACHE | \
                                          REDUCT_MODIFIED_FILE_CACHE | \
                                          REDUCT_WSL_CACHE_CLEAN | \
                                          REDUCT_WSL_MEMORY_RECLAIM)

#define REDUCT_MASK_DEFAULT               (REDUCT_WORKING_SET | REDUCT_SYSTEM_FILE_CACHE | REDUCT_STANDBY_PRIORITY0_LIST | REDUCT_REGISTRY_CACHE | REDUCT_COMBINE_MEMORY_LISTS | REDUCT_MODIFIED_FILE_CACHE)

#define REDUCT_MASK_FREEZES               (REDUCT_STANDBY_LIST | REDUCT_MODIFIED_LIST)

// Configuration key names
#define CONFIG_KEY_ALWAYS_ON_TOP                L"AlwaysOnTop"
#define CONFIG_KEY_AUTOREDUCT_ENABLE            L"AutoreductEnable"
#define CONFIG_KEY_AUTOREDUCT_VALUE             L"AutoreductValue"
#define CONFIG_KEY_AUTOREDUCT_INTERVAL_ENABLE   L"AutoreductIntervalEnable"
#define CONFIG_KEY_AUTOREDUCT_INTERVAL_VALUE    L"AutoreductIntervalValue"
#define CONFIG_KEY_BALLOON_CLEAN_RESULTS        L"BalloonCleanResults"
#define CONFIG_KEY_HOTKEY_CLEAN_ENABLE          L"HotkeyCleanEnable"
#define CONFIG_KEY_HOTKEY_CLEAN                 L"HotkeyClean"
#define CONFIG_KEY_IS_ALLOW_STANDBY_LIST_CLEANUP L"IsAllowStandbyListCleanup"
#define CONFIG_KEY_IS_NOTIFICATIONS_SOUND        L"IsNotificationsSound"
#define CONFIG_KEY_IS_SHOW_REDUCTION_CONFIRMATION L"IsShowReductConfirmation"
#define CONFIG_KEY_IS_START_MINIMIZED           L"IsStartMinimized"
#define CONFIG_KEY_LOG_CLEAN_RESULTS            L"LogCleanResults"
#define CONFIG_KEY_STATISTIC_LAST_REDUCTION     L"StatisticLastReduct"
#define CONFIG_KEY_TRAY_ACTION_DC               L"TrayActionDc"
#define CONFIG_KEY_TRAY_ACTION_MC               L"TrayActionMc"
#define CONFIG_KEY_TRAY_CHANGE_BG               L"TrayChangeBg"
#define CONFIG_KEY_TRAY_COLOR_BG                L"TrayColorBg"
#define CONFIG_KEY_TRAY_COLOR_DANGER            L"TrayColorDanger"
#define CONFIG_KEY_TRAY_COLOR_TEXT              L"TrayColorText"
#define CONFIG_KEY_TRAY_COLOR_WARNING           L"TrayColorWarning"
#define CONFIG_KEY_TRAY_FONT                    L"TrayFont"
#define CONFIG_KEY_TRAY_LEVEL_DANGER            L"TrayLevelDanger"
#define CONFIG_KEY_TRAY_LEVEL_WARNING           L"TrayLevelWarning"
#define CONFIG_KEY_TRAY_ROUND_CORNERS           L"TrayRoundCorners"
#define CONFIG_KEY_TRAY_SHOW_BORDER             L"TrayShowBorder"
#define CONFIG_KEY_TRAY_USE_ANTIALIASING        L"TrayUseAntialiasing"
#define CONFIG_KEY_TRAY_USE_TRANSPARENCY        L"TrayUseTransparency"
#define CONFIG_KEY_REDUCTION_MASK               L"ReductMask2"

// Default values
#define DEFAULT_AUTOREDUCT_VAL                  90
#define DEFAULT_AUTOREDUCTINTERVAL_VAL          30
#define DEFAULT_DANGER_LEVEL                    90
#define DEFAULT_WARNING_LEVEL                   60

// Array sizes
#define LIMITS_ARRAY_SIZE                       13
#define INTERVALS_ARRAY_SIZE                  13

// Global arrays (defined in main.c)
extern ULONG_PTR limits_arr[LIMITS_ARRAY_SIZE];
extern ULONG_PTR intervals_arr[INTERVALS_ARRAY_SIZE];
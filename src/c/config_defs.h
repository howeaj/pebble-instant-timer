// Copyright (c) 2026 Andrew Howe. All rights reserved. See LICENSE (GPLv3.0).

// App-specific definitions of config options, for inclusion by config.h

#pragma once

#include <pebble.h>

#include "macros.h"


/** The persist data version for dealing with backwards-incompatible changes.g
    This doesn't need to change as long as you only append new fields
    and don't change the messageKeys or meaning of existing data.
    Note lowest allowed value is 1.
*/
#define PERSIST_CONFIG_VERSION (1)

/** Macro table defining all config data.

    - Conftype must match one of the RECEIVE_CONFIG_ options in config.c
    - CType is the C-side storage type.
    - Message key must match config.json and package.json, and be a valid C identifier.
    - Default value must match config.json.
*/
#define X_CONFIG_OPTIONS(MACRO) \
    /* conftype, C type,              message key,                  default */ \
    MACRO(COLOR, GColor,              textColor,                    GColorWhite) \
    MACRO(COLOR, GColor,              bgColor,                      GColorBlack) \
    MACRO(COLOR, GColor,              bgColorImage,                 GColorBulgarianRose) \
    MACRO(COLOR, GColor,              ringColorEmpty,               GColorDarkGray) \
    MACRO(COLOR, GColor,              ringColorRemaining,           PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite)) \
    MACRO(COLOR, GColor,              ringColorOvertime,            PBL_IF_COLOR_ELSE(GColorRed, GColorWhite)) \
    MACRO(COLOR, GColor,              statusBarBgColor,             GColorBlack) \
    MACRO(COLOR, GColor,              statusBarTextColor,           GColorWhite) \
    MACRO(COLOR, GColor,              actionBarBgColor,             GColorBlack) \
    MACRO(COLOR, GColor,              actionBarIconColor,           GColorWhite) \
    MACRO(BOOL,  bool,                enableTouch,                  true) \
    MACRO(INT,   int32_t,             touchInputTimeoutDeciseconds, 20) /*how long you have to start the second touch before it cancels*/ \
    MACRO(INT,   int32_t,             touchMinDurationMs,           150) /*minimum duration for touches on the outer ring to register*/ \
    MACRO(ENUM,  TouchZoneAssignment, touchZoneAssignment,          TouchZoneAssignment_Default) \
    MACRO(ENUM,  TouchTimerMode,      touchTimerMode,               TouchTimerMode_Duration) \
    MACRO(ENUM,  AlarmVibePattern,    alarmVibePattern,             AlarmVibePattern_Double) \
/* end of X_CONFIG_OPTIONS */


// The meaning of the inner/outer ring for the initial touch
typedef enum TouchZoneAssignment {
    TouchZoneAssignment_Default = 0,  // inner=duration, outer=alarm
    TouchZoneAssignment_Invert = 1,  // inner=alarm, outer=duration
} TouchZoneAssignment;

// What values the touch timer affects
typedef enum TouchTimerMode {
    TouchTimerMode_Clear = 0,
    TouchTimerMode_Duration = 1,
    TouchTimerMode_Remaining = 2,
} TouchTimerMode;

// Alarm vibe pattern
typedef enum AlarmVibePattern {
    AlarmVibePattern_Double = 0,
    AlarmVibePattern_Short = 1,
    AlarmVibePattern_Long = 2,
} AlarmVibePattern;

// Copyright (c) 2026 Andrew Howe. All rights reserved. See LICENSE (GPLv3.0).

// App-specific definitions of config options, for inclusion by config.h

#pragma once

#include <pebble.h>

#include "macros.h"


/** The persist data version for dealing with backwards-incompatible changes.
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
    MACRO(ENUM,  TouchTimerEffect,    touchTimerMode,               TouchTimerEffect_Duration) \
    MACRO(ENUM,  AlarmVibePattern,    alarmVibePattern,             AlarmVibePattern_Double) \
    MACRO(ENUM,  TouchTimerSetMethod, touchTimerSetMethod,          TouchTimerSetMethod_MinuteWindup) \
    MACRO(BOOL,  bool,                touchDisableWhileInactive,    false) \
    MACRO(INT,   int32_t,             touchLiftMinDurationMs,       100) /*minimum duration for lifts to register setting hours*/\
    MACRO(INT,   int8_t,              backlightDurationS,           3) \
    MACRO(INT,   int8_t,              shortAlarmMinutes,            3) \
    MACRO(INT,   int8_t,              shortStopwatchMinutes,        1) \
/* end of X_CONFIG_OPTIONS */


// The maximum value of shortAlarmMinutes and shortStopwatchMinutes, in config.json
#define MAX_POWER_SAVING_THRESHOLD (16)

// The meaning of the inner/outer ring for the initial touch
typedef enum TouchZoneAssignment {
    TouchZoneAssignment_Default = 0,  // inner=duration, outer=alarm
    TouchZoneAssignment_Invert = 1,  // inner=alarm, outer=duration
} TouchZoneAssignment;

// What values the touch timer affects
typedef enum TouchTimerEffect {
    TouchTimerEffect_Clear = 0,
    TouchTimerEffect_Duration = 1,
    TouchTimerEffect_Remaining = 2,
} TouchTimerEffect;

// Alarm vibe pattern
typedef enum AlarmVibePattern {
    AlarmVibePattern_Double = 0,
    AlarmVibePattern_Short = 1,
    AlarmVibePattern_Long = 2,
} AlarmVibePattern;

// How many touches are required when setting the timer duration
typedef enum TouchTimerSetMethod {
    TouchTimerSetMethod_MinuteWindup = 0,
    TouchTimerSetMethod_TwoTouch = 1,
} TouchTimerSetMethod;

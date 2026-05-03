// Copyright (c) 2026 Andrew Howe. All rights reserved. See LICENSE (GPLv3.0).

#pragma once

#include <pebble.h>

#include "macros.h"


#pragma pack(push, 1)  // prevent unpredictable format changes

// The names of these fields should match the messageKeys in config.json and package.json.
typedef struct Config {
    GColor textColor;
    GColor bgColor;
    GColor bgColorImage;
    GColor ringColorEmpty;
    GColor ringColorRemaining;
    GColor ringColorOvertime;
    GColor statusBarBgColor;
    GColor statusBarTextColor;
    GColor actionBarBgColor;
    GColor actionBarIconColor;

    bool enableTouch;
    int32_t touchInputTimeoutDeciseconds;  // how long you have to start the second touch before it cancels
    int32_t touchMinDurationMs;  // minimum duration for touches on the outer ring to register
    bool invertTouchZones;  // swap the meaning of the inner/outer ring for the initial touch
} Config;
// This doesn't need to change as long as you only append new fields
// and don't change the meaning of existing data.
#define PERSIST_CONFIG_VERSION (1)

#pragma pack(pop)


typedef void (*NewConfigCallback)(const Config* config);

void config_init(NewConfigCallback callback);
void config_deinit(void);
const Config* config_get(void);

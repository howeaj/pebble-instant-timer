// Copyright (c) 2026 Andrew Howe. All rights reserved. See LICENSE (GPLv3.0).

// App configuration via clay
// pebble package install @rebble/clay

#include "config.h"

#include <pebble.h>

#include "macros.h"
#include "persist_keys.h"

// This should match the defaults in config.json and Dark theme in index.js
static Config s_config = {
    .textColor = GColorWhite,
    .bgColor = GColorBlack,
    .bgColorImage = GColorBulgarianRose,
    .ringColorEmpty = GColorDarkGray,
    .ringColorRemaining = PBL_IF_COLOR_ELSE(GColorGreen, GColorWhite),
    .ringColorOvertime = PBL_IF_COLOR_ELSE(GColorRed, GColorWhite),
    .statusBarBgColor = GColorBlack,
    .statusBarTextColor = GColorWhite,
    .actionBarBgColor = GColorBlack,
    .actionBarIconColor = GColorWhite
};
STATIC_ASSERT(sizeof(Config) == 10);

static NewConfigCallback s_new_config_callback = NULL;


/******************************************************************************
 Local watch persistence
******************************************************************************/

static bool is_local_persist_written_and_current_version(void) {
    return persist_read_int(PERSIST_KEY_CONFIG_VERSION) == PERSIST_CONFIG_VERSION;
}

static void local_persist_load(void) {
    StatusCode status = E_DOES_NOT_EXIST;
    if (is_local_persist_written_and_current_version()){
        status = persist_read_data(PERSIST_KEY_CONFIG, &s_config, sizeof(s_config));
    }
    if (status <= 0) {
        LOG("Config not loaded from persistent storage (%d)", status);
    }
}

static void local_persist_save(void) {
    StatusCode status = persist_write_data(PERSIST_KEY_CONFIG, &s_config, sizeof(s_config));
    ASSERT(status == sizeof(s_config));

    if (status == sizeof(s_config)) {
        status = persist_write_int(PERSIST_KEY_CONFIG_VERSION, PERSIST_CONFIG_VERSION);
        ASSERT(status == sizeof(int32_t));
    }
}


/******************************************************************************
 Receive config from phone
******************************************************************************/

#define RECEIVE_CONFIG(message_key, convert) MACRO_START \
    const Tuple *tuple = dict_find(iter, MESSAGE_KEY_##message_key); \
    if (tuple) { \
        s_config.message_key = convert; \
    } else { \
        LOG("Missing config: " #message_key); \
    } \
MACRO_END
#define RECEIVE_CONFIG_BOOL(message_key) RECEIVE_CONFIG(message_key, (tuple->value->int32 == 1))
#define RECEIVE_CONFIG_COLOR(message_key) RECEIVE_CONFIG(message_key, GColorFromHEX(tuple->value->int32))

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
    Config saved_config = s_config;

    RECEIVE_CONFIG_COLOR(textColor);
    RECEIVE_CONFIG_COLOR(bgColor);
    RECEIVE_CONFIG_COLOR(bgColorImage);
    RECEIVE_CONFIG_COLOR(ringColorEmpty);
    RECEIVE_CONFIG_COLOR(ringColorRemaining);
    RECEIVE_CONFIG_COLOR(ringColorOvertime);
    RECEIVE_CONFIG_COLOR(statusBarBgColor);
    RECEIVE_CONFIG_COLOR(statusBarTextColor);
    RECEIVE_CONFIG_COLOR(actionBarBgColor);
    RECEIVE_CONFIG_COLOR(actionBarIconColor);
    STATIC_ASSERT(sizeof(Config) == 10);

    if (memcmp(&saved_config, &s_config, sizeof(saved_config)) != 0) {
        LOG("New app config received");
        local_persist_save();
        if (s_new_config_callback != NULL) {
            s_new_config_callback(&s_config);
        }
    }
}


/******************************************************************************
 Public methods
******************************************************************************/

// `callback` is an optional function to call whenever a new config is received from the phone;
// it should be used to mark affected layers dirty or otherwise live-update config changes
void config_init(NewConfigCallback callback) {
    s_new_config_callback = callback;
    local_persist_load();
    app_message_register_inbox_received(&inbox_received_handler);
    app_message_open(128, 128);  // TODO how big?
}

void config_deinit(void) {
    app_message_deregister_callbacks();
}

const Config* config_get(void) {
    return &s_config;
}

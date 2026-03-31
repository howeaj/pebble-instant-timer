/*
    TODO
        - app icon
        - better icons
        - hide seconds when backlight off (not when <2mins remaining)
            light_enable_interaction() or prv_change_state(LIGHT_STATE_ON_TIMED)
            DEFAULT_BACKLIGHT_TIMEOUT_MS
            - any button *released*
            - shake (AKA "tap") accel_tap_service_subscribe
                - alerts_preferences_dnd_get_motion_backlight
            - battery state connected

        - bell icon
            - re-enable rotation animation?
            - convert to pdc?
        - timeline pin
        - support other pebbles
        - touchscreen control

    "targetPlatforms": [
      "aplite",
      "basalt",
      "chalk",
      "diorite",
      "emery",
      "flint",
      "gabbro"
    ],
*/

#include <pebble.h>
#include <stdio.h>
#include <time.h>

#define MACRO_START do {
#define MACRO_END } while (0)
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(a) (((a) >= 0) ? (a) : ((a) * -1))
#define LOG(...) APP_LOG(APP_LOG_LEVEL_INFO, __VA_ARGS__)
#define TRACE(...) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)
#define ASSERT(condition) if (!(condition)) APP_LOG(APP_LOG_LEVEL_ERROR, "ASSERTION FAILED ON LINE %d", __LINE__)
#define TIME_MAX INT32_MAX  // maximum storeable value of time_t

#define LONG_CLICK_DURATION (500)  // duration for long click events

#define DEFAULT_BACKLIGHT_TIMEOUT_MS 3000 // TODO

static Window *s_main_window;
static StatusBarLayer *s_status_bar;

static Layer* s_bg_layer;
static Layer* s_duration_layer;
static PropertyAnimation* s_edit_indicator_animation;

static RotBitmapLayer* s_bell_layer;
static GBitmap* s_icon_bell;

static BitmapLayer* s_alarm_icon_layer;
static GBitmap* s_icon_alarm;

// text
static TextLayer *s_text_layer_edit_indicator;
static TextLayer *s_text_layer_alarm_duration;
static TextLayer *s_text_layer_alarm_time;
static TextLayer *s_text_layer_primary;
static TextLayer *s_text_layer_secondary;
#define MAX_TEXT_SIZE (50)
static char s_edit_indicator_text[MAX_TEXT_SIZE] = "^";
static char s_alarm_duration_text[MAX_TEXT_SIZE] = "00h00m00s";
static char s_alarm_time_text[MAX_TEXT_SIZE] = "00:00pm";
static char s_elapsed_text[MAX_TEXT_SIZE] = "00h00m00s";
static char s_remaining_text[MAX_TEXT_SIZE] = "00h00m00s";

// action bar
static ActionBarLayer *s_action_bar;
static GBitmap* s_icon_up;
static GBitmap* s_icon_right;
static GBitmap* s_icon_down;
static GBitmap* s_icon_pause;
static GBitmap* s_icon_start;
static GBitmap* s_icon_delete;
static GBitmap* s_icon_refresh;

typedef enum IncrementMode_e {
    INCR_HOURS = 0,
    INCR_MINS  = 1,
    INCR_SECS  = 2
} IncrementMode;

// mode state
#define MODE_EXIT  (-1)
#define MODE_HOURS INCR_HOURS
#define MODE_MINS  INCR_MINS
#define MODE_SECS  INCR_SECS
#define MODE_CTRL  (3)
#define MODE_MAX MODE_CTRL
static int32_t s_mode = MODE_HOURS;

// app state
// affects PERSIST_VERSION
typedef struct State_s {
    time_t alarm_duration;
    time_t start_time;
    time_t elapsed_time;
    bool is_counting;
    bool is_alarm_done;
    WakeupId alarm_wakeup_id;
} State_t;
State_t s_state = {
    .alarm_duration = 0,
    .start_time = 0,
    .elapsed_time = 0,
    .is_counting = false,
    .is_alarm_done = false,
    .alarm_wakeup_id = E_DOES_NOT_EXIST
};

static bool s_initialising = true;


/// Format `seconds` into a `buffer` of `size` as hours, minutes, seconds
static void snprintf_hms(char* buffer, size_t size, time_t seconds, bool truncate) {
    const char* neg = seconds < 0 ? "-" : "";
    const int abs_seconds = ABS(seconds);
    const int h = abs_seconds / SECONDS_PER_HOUR ;
    const int m = (abs_seconds % SECONDS_PER_HOUR ) / SECONDS_PER_MINUTE;
    const int s = abs_seconds % SECONDS_PER_MINUTE;
    if (h || !truncate) {
        snprintf(buffer, size, "%s%dh%02dm%02ds", neg, h, m, s);
    } else if (m) {
        snprintf(buffer, size, "%s%dm%02ds", neg, m, s);
    } else {
        snprintf(buffer, size, "%s%ds", neg, s);
    }
}

// format a time_t into a string
static void snprintf_time(char* target, size_t size, const char* fmt, time_t time) {
    char time_str[MAX_TEXT_SIZE] = {0};
    const char* time_fmt = clock_is_24h_style() ? "%H:%M" : "%I:%M%P";
    strftime(time_str, sizeof(time_str), time_fmt, localtime(&time));
    snprintf(target, size, fmt, time_str);
}


/******************************************************************************
 Persistence
******************************************************************************/

#define PERSIST_VERSION (4)  // The current persistent storage version. Increment when making changes to stored data.

// persistent storage keys
#define PERSIST_KEY_VERSION (0)  // key for the version of the remaining storage layout
#define PERSIST_KEY_STATE   (1)  // key for s_state


static bool is_persist_written_and_current_version(void) {
    return persist_read_int(PERSIST_KEY_VERSION) == PERSIST_VERSION;
}

/// Return true if data was loaded
static bool stopwatch_load(void){
    StatusCode status = E_DOES_NOT_EXIST;
    if (is_persist_written_and_current_version()){
        status = persist_read_data(PERSIST_KEY_STATE, &s_state, sizeof(s_state));
        ASSERT(status == sizeof(s_state));
    }
    return status == sizeof(s_state);
}

static void stopwatch_save(void){
    StatusCode status = persist_write_data(PERSIST_KEY_STATE, &s_state, sizeof(s_state));
    ASSERT(status == sizeof(s_state));

    if (status == sizeof(s_state)) {
        status = persist_write_int(PERSIST_KEY_VERSION, PERSIST_VERSION);
        ASSERT(status == sizeof(int32_t));
    }
}

static void stopwatch_delete(void){
    StatusCode status = persist_delete(PERSIST_KEY_STATE);
    ASSERT((status == S_TRUE) || (status == E_DOES_NOT_EXIST));

    status = persist_delete(PERSIST_KEY_VERSION);
    ASSERT((status == S_TRUE) || (status == E_DOES_NOT_EXIST));
}


/******************************************************************************
 Business logic
******************************************************************************/

/// Return the time at which the alarm should go off, or 0 if there is no future alarm.
static time_t stopwatch_get_alarm_time(void) {
    const time_t end_time = s_state.start_time + s_state.alarm_duration;
    if (s_state.is_counting && (end_time > time(NULL))) {
        return end_time;
    } else {
        return 0;
    }
}

static void stopwatch_tick(void){
    if (s_state.is_counting) {
        s_state.elapsed_time = time(NULL) - s_state.start_time;
    }
}

static void stopwatch_toggle(void){
    if (s_state.is_counting) {
        // pause; ensure elapsed time is up-to-date
        stopwatch_tick();
        s_state.is_counting = false;
    } else {
        // resume; reload a new start time from the elapsed time
        s_state.start_time = time(NULL) - s_state.elapsed_time;
        s_state.is_counting = true;
    }
}

static void stopwatch_restart(void){
    s_state.start_time = time(NULL);
    s_state.elapsed_time = 0;
}

static void stopwatch_clear(void){
    s_state.alarm_duration = 0;
    s_state.is_counting = true;
    stopwatch_restart();
}

/** Return the value to be added (add=True) or subtracted (add=False) by the next increment_alarm().

    This doesn't count handling of decrements below 0.
*/
static time_t get_alarm_increment_diff(const IncrementMode incr, const bool add) {
    // Each increment adds 15s until 2 minutes, then by 30s after 5m etc
    const time_t diffs[4]      = {5,   30,   60,    60*5};  // TODO 15 not 5
    const time_t thresholds[4] = {2*60, 5*60, 30*60, TIME_MAX};
    int bucket = 0;
    for (; bucket < 4; bucket++){
        if ((s_state.alarm_duration - ((add || !s_state.alarm_duration) ? 0 : 1)) < thresholds[bucket]){
            break;
        }
    }

    time_t change = diffs[bucket];
    switch (incr) {
        case INCR_HOURS:
            change = MAX(SECONDS_PER_HOUR, change);
            break;
        case INCR_MINS:
            change = MAX(SECONDS_PER_MINUTE, change);
            break;
        case INCR_SECS:
            break;
        default:
            ASSERT(false);
            break;
    }
    return change;
}

/// Increment (add=True) or decrement (add=False) the alarm duration.
static void increment_alarm(const IncrementMode incr, const bool add, const bool allow_wrap) {
    const time_t change = get_alarm_increment_diff(incr, add);

    if (!add && (change > s_state.alarm_duration)){
        if (allow_wrap && (s_state.alarm_duration == 0)){  // when decrementing from 0, jump to 8h or 55m or 45s
            switch (incr) {
                case INCR_HOURS:
                    s_state.alarm_duration = 8 * SECONDS_PER_HOUR;
                    break;
                case INCR_MINS:
                    s_state.alarm_duration = 55 * SECONDS_PER_MINUTE;
                    break;
                case INCR_SECS:
                    s_state.alarm_duration = 45;
                    break;
                default:
                    ASSERT(false);
                    break;
            }
        } else {
            s_state.alarm_duration = 0;
        }
    } else {
        s_state.alarm_duration += change * (add ? 1 : -1);
    }
}

/// Return the next mode that would be reached via increment (add=True) or decrement (add=False)
static int get_next_mode(const bool add) {
    int next_mode = s_mode + (add ? 1 : -1);

    // Skip secs selection if the alarm duration is beyond the secs modification threshold
    if ((next_mode == MODE_SECS) && (get_alarm_increment_diff((IncrementMode)next_mode, false) >= SECONDS_PER_MINUTE)){
        next_mode += (add ? 1 : -1);
    }

    return MIN(next_mode, MODE_MAX);
}

/// Increment (add=True) or decrement (add=False) the mode.
static void increment_mode(const bool add) {
    s_mode = get_next_mode(add);
}


/******************************************************************************
 Alarm
******************************************************************************/

#define ALARM_PULSE_DURATION SECONDS_PER_MINUTE  // how long the alarm will ring before automatically stopping
static AppTimer* s_alarm_pulse_timer = NULL;

/// Return true if an alarm was active
static bool alarm_clear(void) {
    const bool was_active = s_alarm_pulse_timer != NULL;
    if (was_active) {
        LOG("Clearing alarm");
        app_timer_cancel(s_alarm_pulse_timer);
        s_alarm_pulse_timer = NULL;
        vibes_cancel();
        s_state.is_alarm_done = true;
    }
    return was_active;
}

static void alarm_pulse(void) {
    LOG("ALARM PULSE!");
    vibes_double_pulse();
    // TODO copy the pebble's builtin alarm pattern
    // static const uint32_t const segments[] = { 200, 100, 400 };
    // VibePattern pat = {
    //   .durations = segments,
    //   .num_segments = ARRAY_LENGTH(segments),
    // };
    // vibes_enqueue_custom_pattern(pat);
}

/// Repeat alarm_pulse() until ALARM_PULSE_DURATION is up
static void alarm_pulse_timer_handler(void* data) {
    alarm_pulse();
    const time_t end_time = s_state.start_time + s_state.alarm_duration;
    if ((time(NULL) - end_time) < ALARM_PULSE_DURATION) {
        s_alarm_pulse_timer = app_timer_register(2000, alarm_pulse_timer_handler, NULL);
    } else {
        (void) alarm_clear();
    }
}

static bool alarm_should_start(void) {
    return (
        (s_state.alarm_duration > 0)
        && !s_alarm_pulse_timer  // already started
        && !s_state.is_alarm_done  // already started and finished
        && (s_state.elapsed_time >= s_state.alarm_duration)
    );
}

/// Trigger the alarm
static void alarm_start(void) {
    ASSERT(s_alarm_pulse_timer == NULL);
    ASSERT(!s_state.is_alarm_done);
    alarm_pulse_timer_handler(NULL);
}

static bool alarm_is_pulsing(void) {
    return s_alarm_pulse_timer;
}

static void alarm_cancel_any_wakeup(void) {
    if (s_state.alarm_wakeup_id >= 0) {
        wakeup_cancel(s_state.alarm_wakeup_id);
        s_state.alarm_wakeup_id = E_DOES_NOT_EXIST;
    }
}

/// Schedule (or cancel) the wakeup timer
static void alarm_schedule_any_wakeup(void) {
    TRACE("alarm_schedule_wakeup_timer");
    ASSERT(s_state.alarm_wakeup_id == E_DOES_NOT_EXIST);

    time_t alarm_time = stopwatch_get_alarm_time();
    if ((alarm_time != 0) && !s_state.is_alarm_done) {
        do {
            s_state.alarm_wakeup_id = wakeup_schedule(alarm_time, 0, true);
            alarm_time -= 1;
        } while (s_state.alarm_wakeup_id == E_RANGE);  // other app already scheduled wakeup within 1 minute
        ASSERT(s_state.alarm_wakeup_id >= 0);

        alarm_time += 1;
        const struct tm* alarm_time_local = localtime(&alarm_time);
        char time_str[MAX_TEXT_SIZE] = {0};
        const size_t num_bytes = strftime(time_str, sizeof(time_str), "%H:%M:%S", alarm_time_local);
        ASSERT(num_bytes);
        LOG("alarm_wakeup_id = %d at %s", s_state.alarm_wakeup_id, time_str);
    }
}

/// Reset the alarm if necessary
static void alarm_reset(void) {
    s_state.is_alarm_done = stopwatch_get_alarm_time() == 0;
}


/******************************************************************************
 UI updates
******************************************************************************/

static void update_action_bar(void) {
    TRACE("update_action_bar");
    const GBitmap* icon_toggle = s_state.is_counting ? s_icon_pause : s_icon_start;
    if (s_mode == MODE_CTRL) {
        action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_UP, s_icon_refresh, true);
        action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_SELECT, icon_toggle, true);
        action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_DOWN, s_icon_delete, true);
        action_bar_layer_set_icon_press_animation(s_action_bar, BUTTON_ID_UP, ActionBarLayerIconPressAnimationMoveLeft);
        action_bar_layer_set_icon_press_animation(s_action_bar, BUTTON_ID_DOWN, ActionBarLayerIconPressAnimationMoveLeft);
    } else {
        const GBitmap* icon_mode = (get_next_mode(true) == MODE_CTRL ? icon_toggle : s_icon_right);
        action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_UP, s_icon_up, true);
        action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_SELECT, icon_mode, true);
        action_bar_layer_set_icon_animated(s_action_bar, BUTTON_ID_DOWN, s_icon_down, true);
        action_bar_layer_set_icon_press_animation(s_action_bar, BUTTON_ID_UP, ActionBarLayerIconPressAnimationMoveUp);
        action_bar_layer_set_icon_press_animation(s_action_bar, BUTTON_ID_DOWN, ActionBarLayerIconPressAnimationMoveDown);
    }
}

static void update_primary_and_secondary_text(void) {
    if (s_state.alarm_duration > 0) {
        text_layer_set_text(s_text_layer_primary, s_remaining_text);
        text_layer_set_text(s_text_layer_secondary, s_elapsed_text);
    } else {
        text_layer_set_text(s_text_layer_primary, s_elapsed_text);
        text_layer_set_text(s_text_layer_secondary, "");
    }
}

static void update_alarm_time(void) {
    // TRACE("update_alarm_time");
    if (s_state.alarm_duration > 0) {
        const time_t alarm_time = time(NULL) + s_state.alarm_duration - s_state.elapsed_time;
        const struct tm* alarm_time_local = localtime(&alarm_time);
        const char* fmt = clock_is_24h_style() ? "%H:%M" : "%I:%M%P";
        const size_t num_bytes = strftime(s_alarm_time_text, sizeof(s_alarm_time_text), fmt, alarm_time_local);
        ASSERT(num_bytes);
        text_layer_set_text(s_text_layer_alarm_time, s_alarm_time_text);
    } else {
        text_layer_set_text(s_text_layer_alarm_time, "");
    }
    update_primary_and_secondary_text();
    alarm_reset();
}

static void update_remaining(void) {
    // TRACE("update_remaining");
    bool show_alarm_icon = false;
    if (s_state.alarm_duration > 0) {
        const time_t remaining = s_state.alarm_duration - s_state.elapsed_time;
        snprintf_hms(s_remaining_text, sizeof(s_remaining_text), remaining, true);
        show_alarm_icon = remaining > 0;
    }
    layer_set_hidden(bitmap_layer_get_layer(s_alarm_icon_layer), !show_alarm_icon);
    update_primary_and_secondary_text();
}

static void update_mode(void) {
    const char* text = "^^";
    GRect frame = layer_get_frame((Layer*)s_text_layer_edit_indicator);
    const bool large = s_state.alarm_duration >= 10 * 60 * 60;
    switch (s_mode) {
        case MODE_HOURS:
            text = large ? "^^" : "^";
            frame.origin.x = -26;
            break;
        case MODE_MINS:
            frame.origin.x = large ? -5 : -9;
            break;
        case MODE_SECS:
            frame.origin.x = 16;
            break;
        case MODE_CTRL:
            frame.origin.x = 150;
            break;
        default:
            ASSERT(false);
            break;
    }
    snprintf(s_edit_indicator_text, sizeof(s_edit_indicator_text), text);
    text_layer_set_text(s_text_layer_edit_indicator, s_edit_indicator_text);
    // This is how to set it without animation: layer_set_frame((Layer*)s_text_layer_edit_indicator, frame);
    if (s_edit_indicator_animation != NULL){
        animation_unschedule(property_animation_get_animation(s_edit_indicator_animation));
        property_animation_destroy(s_edit_indicator_animation);
        s_edit_indicator_animation = NULL;
    }
    s_edit_indicator_animation = property_animation_create_layer_frame((Layer*)s_text_layer_edit_indicator, NULL, &frame);
    animation_schedule(property_animation_get_animation(s_edit_indicator_animation));
}

static void update_alarm_duration(void) {
    TRACE("update_alarm_duration");
    snprintf_hms(s_alarm_duration_text, sizeof(s_alarm_duration_text), s_state.alarm_duration, false);
    text_layer_set_text(s_text_layer_alarm_duration, s_alarm_duration_text);

    update_mode();
    update_alarm_time();
}

static void update_elapsed(void) {
    // TRACE("update_elapsed");
    snprintf_hms(s_elapsed_text, sizeof(s_elapsed_text), s_state.elapsed_time, true);
    update_remaining();
}


/******************************************************************************
 Handlers
******************************************************************************/

void glance_reload_callback(AppGlanceReloadSession *session, size_t limit, void *context) {
    char subtitle[150] = {0};
    AppGlanceSlice slice = {
        .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION,
        .layout.icon = APP_GLANCE_SLICE_DEFAULT_ICON, // TODO "publishedResource"
        .layout.subtitle_template_string = subtitle,
    };

    #define ADD_SLICE() \
        MACRO_START \
            AppGlanceResult result = app_glance_add_slice(session, slice); \
            ASSERT(result == APP_GLANCE_RESULT_SUCCESS); \
            LOG("appglanceresult = %d", result);  \
        MACRO_END

    if (s_state.is_counting) {
        if (s_state.alarm_duration > 0) {
            const time_t alarm_time = s_state.start_time + s_state.alarm_duration;
            snprintf_time(subtitle, sizeof(subtitle), "Alarm expired at %s", alarm_time);
            ADD_SLICE();
            if (s_state.elapsed_time < s_state.alarm_duration) {
                snprintf_time(subtitle, sizeof(subtitle), "Alarm set for %s", alarm_time);
                slice.expiration_time = alarm_time;
                ADD_SLICE();
            }
        } else {
            snprintf_time(subtitle, sizeof(subtitle), "Timer running since %s", s_state.start_time);
            ADD_SLICE();
        }
    }

    #undef ADD_SLICE
}

// clear the entire app back to nothing
static void do_clear(void){
    if (s_mode == MODE_CTRL) {
        s_mode = MODE_HOURS;
    }
    stopwatch_clear();
    update_alarm_duration();
    update_elapsed();
    update_mode();
    update_action_bar();
}

// restart the timer, keeping the alarm duration
static void do_restart(void){
    stopwatch_restart();
    update_elapsed();
    update_alarm_time();
}

// modify the alarm duration
static void do_increment(bool add, ClickRecognizerRef recognizer) {
    increment_alarm((IncrementMode)s_mode, add, !click_recognizer_is_repeating(recognizer));
    stopwatch_tick();
    update_alarm_duration();
    update_remaining();
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    // TRACE("tick_handler");
    if (s_state.is_counting) {
        stopwatch_tick();
        update_elapsed();
        if (alarm_should_start()) {
            alarm_start();
        }
    } else {
        update_alarm_time();
    }
    layer_set_hidden((Layer*)s_action_bar, alarm_is_pulsing());

    // TODO chugs the emulator to hell
    layer_set_hidden((Layer*)s_bell_layer, true);// TODO !alarm_is_pulsing());
    // if (alarm_is_pulsing()) {
    //     static int16_t angle = 30;
    //     angle *= -1;
    //     rot_bitmap_layer_set_angle(s_bell_layer, DEG_TO_TRIGANGLE(angle));
    // }
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    TRACE("up_click_handler");
    if (!alarm_clear()) {
        if (s_mode == MODE_CTRL) {
            do_restart();
        } else {
            do_increment(true, recognizer);
        }
    }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    TRACE("down_click_handler");
    if (!alarm_clear()) {
        if (s_mode == MODE_CTRL) {
            do_clear();
        } else {
            do_increment(false, recognizer);
        }
    }
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    TRACE("select_click_handler");
    if (!alarm_clear()) {
        if (s_mode != MODE_CTRL) {
            increment_mode(true);
            update_mode();
        }
        // toggle both on entry to and during MODE_CTRL
        if (s_mode == MODE_CTRL) {
            stopwatch_toggle();
        }
        update_action_bar();
    }
}

static void select_long_click_handler(ClickRecognizerRef recognizer, void *context) {
    TRACE("select_long_click_handler");
    if (!alarm_clear()) {
        do_clear();
    }
}

static void back_click_handler(ClickRecognizerRef recognizer, void *context) {
    TRACE("back_click_handler");
    if (!alarm_clear()) {
        increment_mode(false);
        if (s_mode == MODE_EXIT) {
            LOG("Back click - exit");
            window_stack_pop(true);
        } else {
            update_mode();
            update_action_bar();
        }
    }
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_long_click_subscribe(BUTTON_ID_SELECT, LONG_CLICK_DURATION, select_long_click_handler, NULL);
    window_single_click_subscribe(BUTTON_ID_BACK, back_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_UP, 100, up_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 100, down_click_handler);
}

#define TEXT_COLOR GColorWhite
#define BG_COLOR GColorBlack
#define REMAINING_COLOR GColorGreen
#define ELAPSED_COLOR GColorBlack
#define OVERTIME_COLOR GColorRed
#define BG_COLOR_PAUSED GColorBulgarianRose

static void render_background(Layer *layer, GContext *ctx) {
    const GRect frame = layer_get_frame(layer);
    const GPoint centre = grect_center_point(&frame);
    const int central_panel_radius = 100;

    const bool is_overtime = s_state.alarm_duration && (s_state.elapsed_time >= s_state.alarm_duration);

    // remaining time
    graphics_context_set_fill_color(ctx, is_overtime ? ELAPSED_COLOR : REMAINING_COLOR);
    graphics_fill_rect(ctx, frame, 0, GCornerNone);

    // expired time
    if (s_state.alarm_duration) {
        graphics_context_set_fill_color(ctx, is_overtime ? OVERTIME_COLOR : ELAPSED_COLOR);
        graphics_fill_radial(ctx, frame, GOvalScaleModeFillCircle,
            (centre.x / 2) - central_panel_radius + 1,
            DEG_TO_TRIGANGLE(0),
            (TRIG_MAX_ANGLE * s_state.elapsed_time / s_state.alarm_duration) % (TRIG_MAX_ANGLE + 1)
        );
    }

    // central panel
    graphics_context_set_fill_color(ctx, BG_COLOR);
    graphics_fill_circle(ctx, centre, central_panel_radius);

    // pause symbol
    if (!s_state.is_counting) {
        const GSize pause_size = {central_panel_radius / 2.5, central_panel_radius * 1.5};
        graphics_context_set_fill_color(ctx, BG_COLOR_PAUSED);
        GPoint pause_origin = {centre.x - (pause_size.w * 1.5), centre.y - (pause_size.h / 2)};
        graphics_fill_rect(ctx, (GRect){.origin=pause_origin, .size=pause_size}, 2, GCornersAll);
        pause_origin.x += pause_size.w * 2;
        graphics_fill_rect(ctx, (GRect){.origin=pause_origin, .size=pause_size}, 2, GCornersAll);
    }
}

static void create_alarm_icon(Layer* parent) {
    const GRect bounds = layer_get_bounds(parent);

    s_icon_alarm = gbitmap_create_with_resource(RESOURCE_ID_ALARM);
    const int16_t size = 15;  // assuming its a square

    // TODO put this in a layer together with the text it is meant to be with
    // or use a font with an alarm emoji
    GRect alarm_icon_frame = {
        .origin = {
            (bounds.size.h / 2) - (size / 2) - 30,
            (bounds.size.w / 2) - (size / 2) - 39
        },
        .size = {size, size}
    };

    s_alarm_icon_layer = bitmap_layer_create(alarm_icon_frame);
    bitmap_layer_set_bitmap(s_alarm_icon_layer, s_icon_alarm);
    bitmap_layer_set_compositing_mode(s_alarm_icon_layer, GCompOpSet);  // enable transparency
    bitmap_layer_set_background_color(s_alarm_icon_layer, GColorClear);
    layer_add_child(parent, bitmap_layer_get_layer(s_alarm_icon_layer));
}

static void create_bell_icon(Layer* parent) {
    s_icon_bell = gbitmap_create_with_resource(RESOURCE_ID_BELL);
    s_bell_layer = rot_bitmap_layer_create(s_icon_bell);

    // move the layer to the middle of its parent
    const GRect parent_bounds = layer_get_bounds(parent);
    const GSize size = layer_get_bounds((Layer*)s_bell_layer).size;
    GRect new_bell_frame = {
        .origin = {
            (parent_bounds.size.h / 2) - (size.h / 2),
            (parent_bounds.size.w / 2) - (size.w / 2)
        },
        .size = size
    };
    layer_set_frame((Layer*)s_bell_layer, new_bell_frame);

    rot_bitmap_set_compositing_mode(s_bell_layer, GCompOpSet);  // enable transparency
    layer_add_child(parent, (Layer*)s_bell_layer);
}

static void create_text_layout(Layer* parent) {
    const GRect bounds = layer_get_bounds(parent);

    const GFont main_text_font = fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD);
    const int16_t main_text_h = 28;
    const int16_t main_text_pad = 12; // total additional pixels above&below the maint ext's bounding box
    const int16_t main_text_y = (bounds.size.h / 2) - ((main_text_h + main_text_pad) / 2);
    const GFont small_text_font = fonts_get_system_font(FONT_KEY_GOTHIC_18);
    const int16_t small_text_h = 18;
    const int16_t small_text_spacing = 5;

    // primary text (elapsed or remaining)
    s_text_layer_primary = text_layer_create(GRect(0, main_text_y, bounds.size.w, main_text_h));
    text_layer_set_text(s_text_layer_primary, s_elapsed_text);
    text_layer_set_text_alignment(s_text_layer_primary, GTextAlignmentCenter);
    text_layer_set_font(s_text_layer_primary, main_text_font);
    text_layer_set_background_color(s_text_layer_primary, GColorClear);
    text_layer_set_text_color(s_text_layer_primary, TEXT_COLOR);
    layer_add_child(parent, text_layer_get_layer(s_text_layer_primary));

    #define SMALL_TEXT(name, y_loc) \
    MACRO_START \
        s_text_layer_##name = text_layer_create(GRect(0, y_loc, bounds.size.w, small_text_h)); \
        text_layer_set_text(s_text_layer_##name, s_##name##_text); \
        text_layer_set_text_alignment(s_text_layer_##name, GTextAlignmentCenter); \
        text_layer_set_font(s_text_layer_##name, small_text_font); \
        text_layer_set_background_color(s_text_layer_##name, GColorClear); \
        text_layer_set_text_color(s_text_layer_##name, TEXT_COLOR); \
    MACRO_END
    const int16_t first_text_y = (bounds.size.h / 4) - (small_text_h / 2);

    // duration
    s_duration_layer = layer_create(GRect(0, first_text_y, bounds.size.w, small_text_h * 2));
    layer_add_child(parent, s_duration_layer);

    SMALL_TEXT(alarm_duration, 0);
    layer_add_child(s_duration_layer, text_layer_get_layer(s_text_layer_alarm_duration));

    SMALL_TEXT(edit_indicator, small_text_h - 3);
    layer_add_child(s_duration_layer, text_layer_get_layer(s_text_layer_edit_indicator));

    // alarm time
    SMALL_TEXT(alarm_time, first_text_y + small_text_h + small_text_spacing);
    layer_add_child(parent, text_layer_get_layer(s_text_layer_alarm_time));

    // secondary (elapsed or remaining)
    #define s_secondary_text s_remaining_text
    SMALL_TEXT(secondary, main_text_y + main_text_h + small_text_spacing);
    layer_add_child(parent, text_layer_get_layer(s_text_layer_secondary));
    #undef s_secondary_text

    #undef SMALL_TEXT
}

static void main_window_load(Window *window) {
    TRACE("main_window_load");
    Layer *window_layer = window_get_root_layer(window);

    // background
    s_bg_layer = layer_create(layer_get_frame(window_layer));
    layer_set_update_proc(s_bg_layer, render_background);
    layer_add_child(window_layer, s_bg_layer);

    // bell icon
    create_bell_icon(window_layer);

    // alarm icon
    create_alarm_icon(window_layer);

    // text
    create_text_layout(window_layer);

    // action bar
    s_action_bar = action_bar_layer_create();
    action_bar_layer_set_background_color(s_action_bar, GColorClear);
    action_bar_layer_add_to_window(s_action_bar, window);
    action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);
    s_icon_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_UP);
    s_icon_right = gbitmap_create_with_resource(RESOURCE_ID_ICON_RIGHT);
    s_icon_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_DOWN);
    s_icon_refresh = gbitmap_create_with_resource(RESOURCE_ID_ICON_REFRESH);
    s_icon_start = gbitmap_create_with_resource(RESOURCE_ID_ICON_START);
    s_icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
    s_icon_delete = gbitmap_create_with_resource(RESOURCE_ID_ICON_DELETE);

    // status bar
    s_status_bar = status_bar_layer_create();
    layer_add_child(window_layer, status_bar_layer_get_layer(s_status_bar));
    // status_bar_layer_set_colors(s_status_bar, GColorBlack, GColorWhite);
    // status_bar_layer_set_separator_mode(s_status_bar, StatusBarLayerSeparatorModeDotted);

    // services
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    // business logic
    s_initialising = true;
    if (stopwatch_load()) {
        stopwatch_tick();
        stopwatch_delete();
    } else {
        stopwatch_clear();
    }
    update_mode();
    update_action_bar();
    update_alarm_duration();
    update_elapsed();
    if (launch_reason() == APP_LAUNCH_WAKEUP) {
        alarm_start();
    }
    tick_handler(NULL, 0);
    alarm_cancel_any_wakeup();
    s_initialising = false;
}

static void main_window_unload(Window *window) {
    TRACE("main_window_unload");

    app_glance_reload(glance_reload_callback, NULL);

    // background
    layer_destroy(s_bg_layer);

    // bell icon
    gbitmap_destroy(s_icon_bell);
    rot_bitmap_layer_destroy(s_bell_layer);

    // alarm icon
    gbitmap_destroy(s_icon_alarm);
    bitmap_layer_destroy(s_alarm_icon_layer);

    // text
    layer_destroy(s_duration_layer);
    text_layer_destroy(s_text_layer_edit_indicator);
    text_layer_destroy(s_text_layer_alarm_duration);
    text_layer_destroy(s_text_layer_alarm_time);
    text_layer_destroy(s_text_layer_primary);
    text_layer_destroy(s_text_layer_secondary);
    if (s_edit_indicator_animation != NULL) {
        animation_unschedule(property_animation_get_animation(s_edit_indicator_animation));
        property_animation_destroy(s_edit_indicator_animation);
    }

    // action bar
    action_bar_layer_destroy(s_action_bar);
    gbitmap_destroy(s_icon_up);
    gbitmap_destroy(s_icon_right);
    gbitmap_destroy(s_icon_down);
    gbitmap_destroy(s_icon_refresh);
    gbitmap_destroy(s_icon_start);
    gbitmap_destroy(s_icon_pause);
    gbitmap_destroy(s_icon_delete);

    // status bar
    status_bar_layer_destroy(s_status_bar);

    // services
    tick_timer_service_unsubscribe();

    // business logic
    if (!alarm_clear()){
        alarm_schedule_any_wakeup();
    }
    if (s_state.is_counting) {
        stopwatch_save();
    }
}

static void init(void) {
    LOG("Init");
    s_main_window = window_create();
    window_set_click_config_provider(s_main_window, click_config_provider);
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload,
    });
    window_stack_push(s_main_window, true);
}

static void deinit(void) {
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}

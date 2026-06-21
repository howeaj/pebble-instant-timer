# Changelog

## 2.6.0

- Pressing Up or Down while the alarm is ringing now Restarts or Delete&Exits the app, rather than just clearing the alarm.
- New config options for power saving (defaults to the previous hardcoded values). Can force always-seconds-updates.

## 2.5.0

- Touch timer seconds precision; enable by "winding down" anticlockwise from 0.

## 2.4.0

- Touch timer duration now set with a single touch; just select minutes and "wind up" (circle clockwise) to increase hours.
- Ignore very short finger lifts during touch hours selection
- New config options:
  - Revert to the two-touch hours-then-minutes method.
  - Turn off the touch screen while inactive; turns on for 3s after a button press or accel shake/tap, like the system backlight
  - Minimum finger lift duration

## 2.3.1

- Bugfix minutes touch input timeout after a touch below the minimum threshold
- Bugfix slow elapsed update for touch "clear elapsed" mode

## 2.3.0

- Config alarm vibes: double, long or short
- Add missing start/stop vibe for exit-and-delete

## 2.2.1

- Change the default "Affected Values" touch config to "Set Duration, Keep Elapsed" to match button controls

## 2.2.0

- Increase size of the central touch zone
- Touches reveal seconds if hidden (synced with backlight)
- New user-requested config options:
  - Swap duration vs alarms on the outer vs inner touch zone
  - Retain elapsed time when setting duration with touch

## 2.1.0

- Ignore very short touches (configurable)
- Bugfix incorrect scheduling of wakeup timer on exit-and-delete

## 2.0.1

- Bugfix; angle selection indicator now updates while touching in the middle

## 2.0.0

- Touch control!
  - Touch and drag to select hours then minutes; this clears all state and starts a new timer/alarm.
  - Start touching from the middle to set a timer duration, or from the edge to set an alarm time
  - To cancel, finish touching in the middle or don't select any minutes
- Configuration options:
  - Colours / themes
  - Touch enable/disable and timeout

## 1.7

- Fix crash on Pebble Time 2!

## 1.6

- Fix save icon on black&white platforms.
- Fix app launch icon on OG Pebble.

## 1.5

- Clarify app exit status with trash/save screenwipe

## 1.4

- Return to per-second updates whenever the green or red ring is within 3 minutes of completion

## 1.3

- Always allow 1min increments, rather than forcing 5 beyond 30.
- Shorten glance text to fit without scrolling on smaller screens.

## 1.2

- Clarify display when update rate is reduced from seconds to minutes ("--s" instead of "......")

## 1.1

- Improve display on Time2; inset ring to avoid bezel

## 1.0

First release!

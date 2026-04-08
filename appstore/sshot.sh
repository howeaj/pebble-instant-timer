#!/bin/bash
#
# Script to take a screenshot for the appstore banner.
#
# Run:    ./appstore/demo.sh gabbro
# Record: ShareX; Capture Screen recording (GIF). Keybind CTRL-SHIFT-PRINTSCREEN to repeat recording the same area.
# Crop:   ffmpeg -i input.gif -vf "trim=start=4000ms:end=24000ms" output.gif -y

if [ $# -eq 0 ]
  then
    echo "Must supply the emulator target e.g. './appstore/demo.sh emery'"
    exit 1
fi

set -x

pebble kill
pebble wipe
pebble install --emulator $1
sleep 1
pebble emu-time-format --format 12h
pebble emu-button click select --emulator $1
pebble emu-button click up --emulator $1 --repeat 6
pebble emu-set-time --emulator $1 --utc 19:54:00
sleep 1
pebble screenshot --emulator $1
sleep 1
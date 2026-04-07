#!/bin/bash
#
# Script to help record gifs for the appstore.
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

# setup for start of recording
pebble emu-button click back --emulator $1
pebble emu-button click down --emulator $1
pebble emu-button click select --emulator $1
pebble emu-time-format --format 12h

set +x
read -n 1 -s -r -p "Start recording, then press any key to continue"
printf "\n"
set -x

# start app
pebble emu-button click select --emulator $1
sleep 2

# set 5min timer
pebble emu-button click select --emulator $1
pebble emu-button click up --emulator $1
sleep 2

# reduce to 15s
pebble emu-button click select --emulator $1
pebble emu-button click down --emulator $1 --repeat 4
pebble emu-button click select --emulator $1
pebble emu-button click down --emulator $1 --repeat 3
sleep 1

# pause
pebble emu-button click select --emulator $1
sleep 2

# unpause
pebble emu-button click select --emulator $1

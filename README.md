# Keymo Crone

A handwired, Corne-style (3x6+3) split mechanical keyboard, built as a custom
[QMK](https://qmk.fm/) keyboard (`handwired/keymo_crone`).

Only the right half exists so far and is fully wired and verified (21/21
switches on the first sweep). It runs standalone as the USB master; a left
half is planned.

## Hardware

- **MCU:** SparkFun Pro Micro (`atmega32u4`)
- **Layout:** split 3x6+3 (18 alpha keys + 3 thumbs per half)
- **Matrix:** 4 rows x 6 cols per half, diode direction `COL2ROW`
- **Display:** SSD1306 OLED, 128x32, I2C address `0x3C`
- **Split link:** serial on pin `D2`
- **Bootmagic:** hold the bottom-right key and re-plug USB to enter the
  bootloader

## Firmware layout

```
config.h            # handedness (MASTER_RIGHT) and OLED display defaults
keyboard.json       # QMK keyboard definition: pins, matrix, split config, layout
rules.mk            # build flags (I2C driver required for the diag OLED scan)
flash.sh            # compile + flash helper (see below)
keymaps/
  default/          # the daily-driver 3-layer QWERTY keymap + OLED sprite/keylog feed
  diag/             # right-half matrix bring-up keymap (types a unique char per key)
  diag_left/        # left-half matrix bring-up keymap, tested standalone
  pinscan/          # all-KC_NO probe used to identify pins before wiring
tools/
  frames2qmk.py     # converts ASCII art / images / GIFs into a QMK OLED sprite header
```

### Keymaps

- **default** – the real layout: a 3-layer QWERTY (Base / Lower / Raise)
  transcribed from an Oryx-style layout image, plus a live OLED keystroke
  feed and animated sprite on the right half's screen.
- **diag** – bring-up keymap for the right half's matrix. Each physical key
  types a distinct character so wiring can be verified just by typing into a
  text editor.
- **diag_left** – same idea for the left half, built and tested standalone
  (split disabled) before it's wired into the full split setup.
- **pinscan** – every key is `KC_NO`. Used to safely probe pins while
  identifying wiring, without typing stray characters into whatever window
  has focus.

## Building and flashing

This repo is meant to be dropped into `qmk_firmware/keyboards/handwired/keymo_crone`
(or symlinked in) so `qmk compile` can find it.

```bash
./flash.sh            # compile + flash the `default` keymap
./flash.sh pinscan     # compile + flash the pin-discovery probe
```

`flash.sh` exists instead of `qmk flash` because QMK's AVR flash target
points `avrdude` at `/dev/tty.*`, which hangs on macOS. It instead watches
for the Caterina bootloader by USB vendor ID and flashes via `/dev/cu.*`,
which works reliably. See the comments in `flash.sh` for the full rationale.

## Tools

`tools/frames2qmk.py` turns animation frames into a QMK-ready OLED sprite
header (`sprite.h`), used by the `default` keymap's animation:

```bash
./tools/frames2qmk.py art.txt            > keymaps/default/sprite.h
./tools/frames2qmk.py f1.png f2.png ...  > keymaps/default/sprite.h
./tools/frames2qmk.py walk.gif           > keymaps/default/sprite.h
```

ASCII input needs no dependencies; image/GIF input needs Pillow.

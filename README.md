# Keymo Crone

A handwired, Corne-style (3x6+3) split mechanical keyboard, built as a custom
[QMK](https://qmk.fm/) keyboard (`handwired/keymo_crone`).

Both halves exist and are wired and verified. There are two hardware
revisions with different physical column wiring (see below) -- always
flash the matching revision to each board, since the column pin order is
swapped between them.

## Hardware revisions

The column pin order differs between two physically distinct builds. Each
lives in its own leaf directory since QMK only allows `keyboard.json` at the
deepest part of a build target (no parent/child merging):

- **`rev_soldered/`** -- the original hand-soldered board.
- **`rev_pcb/`** -- a later PCB-based board, whose column wiring order comes
  out mirrored relative to the hand-soldered one (both `matrix_pins.cols`
  and `split.matrix_pins.right.cols` are reversed).

Flashing the wrong revision's firmware to a board doesn't fail to compile or
flash -- it just silently reads every column in the wrong order, which looks
like scrambled/reversed key output. If a board's keys come out mirrored or
scrambled, re-check which revision was flashed before assuming a wiring
fault.

## Hardware

- **MCU:** SparkFun Pro Micro (`atmega32u4`) or a compatible clone
- **Layout:** split 3x6+3 (18 alpha keys + 3 thumbs per half)
- **Matrix:** 4 rows x 6 cols per half, diode direction `COL2ROW`
- **Display:** SSD1306 OLED, 128x32, I2C address `0x3C`
- **Split link:** serial on pin `D2`
- **Bootmagic:** hold the bottom-right key and re-plug USB to enter the
  bootloader (requires QMK already flashed at least once -- a virgin board
  needs a hardware double RST-to-GND reset instead)

## Firmware layout

```
config.h              # handedness (MASTER_RIGHT) and OLED display defaults
rules.mk              # build flags (I2C driver required for the diag OLED scan)
flash.sh              # compile + flash helper (see below)
rev_soldered/
  keyboard.json        # full keyboard definition for the hand-soldered board
rev_pcb/
  keyboard.json         # full keyboard definition for the PCB-based board
keymaps/
  default/            # the daily-driver 3-layer QWERTY keymap + OLED sprite/keylog feed
  diag/               # right-half matrix bring-up keymap (types a unique char per key)
  diag_left/          # left-half matrix bring-up keymap, tested standalone
  pinscan/            # raw-superset pin probe, console-reports row/col bridges
tools/
  frames2qmk.py       # converts ASCII art / images / GIFs into a QMK OLED sprite header
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
- **pinscan** – scans a superset of every usable Pro Micro GPIO (not just the
  wired 4x6 matrix) and reports each key's row/col bridge over the QMK
  console, safely, without typing into whatever window has focus. Use this
  to verify a fresh board's matrix before trusting any other keymap on it.

## Building and flashing

This repo is meant to be dropped into `qmk_firmware/keyboards/handwired/keymo_crone`
(or symlinked in) so `qmk compile` can find it.

```bash
./flash.sh                     # rev_pcb, default keymap
./flash.sh pinscan             # rev_pcb, pin-discovery probe
./flash.sh default soldered    # rev_soldered, default keymap
./flash.sh pinscan soldered    # rev_soldered, pin-discovery probe
```

`flash.sh` takes `[keymap] [revision]`, defaulting to `default` and `pcb`.
Always pass the revision matching the physical board being flashed -- see
[Hardware revisions](#hardware-revisions) above.

`flash.sh` exists instead of `qmk flash` because QMK's AVR flash target
points `avrdude` at `/dev/tty.*`, which hangs on macOS. It instead watches
for the Caterina bootloader by USB vendor ID and flashes via `/dev/cu.*`,
which works reliably. See the comments in `flash.sh` for the full rationale.

Note this VID-based detection assumes a genuine SparkFun Pro Micro
bootloader. Some clones enumerate under Arduino's VID (`0x2341`) instead of
SparkFun's (`0x1B4F`) -- if `flash.sh` times out waiting for the bootloader,
check the board's actual VID/PID first (e.g. via `ioreg -p IOUSB -l -w 0` on
macOS) rather than assuming a bad connection.

## Tools

`tools/frames2qmk.py` turns animation frames into a QMK-ready OLED sprite
header (`sprite.h`), used by the `default` keymap's animation:

```bash
./tools/frames2qmk.py art.txt            > keymaps/default/sprite.h
./tools/frames2qmk.py f1.png f2.png ...  > keymaps/default/sprite.h
./tools/frames2qmk.py walk.gif           > keymaps/default/sprite.h
```

ASCII input needs no dependencies; image/GIF input needs Pillow.

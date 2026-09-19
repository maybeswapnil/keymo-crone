// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// OLED defaults (SSD1306, 128x32, addr 0x3C) match the same module type used
// on corne_hw. Uncomment only what your silkscreen/scan actually indicates.
// #define OLED_DISPLAY_128X64
// #define OLED_IC OLED_IC_SH1106
// #define OLED_COLUMN_OFFSET 2
// #define OLED_DISPLAY_ADDRESS 0x3D
#define OLED_TIMEOUT 0

// ---------------------------------------------------------------------------
// Handedness: only this half exists so far, and it is the one plugged into
// USB (so it is always "master" -- see quantum/split_common/split_util.c,
// is_keyboard_master_impl() == usb_bus_detected()). Without any of
// SPLIT_HAND_PIN / SPLIT_HAND_MATRIX_GRID / EE_HANDS defined, QMK's fallback
// is `is_keyboard_left() == is_keyboard_master()` -- i.e. "whichever half has
// USB is treated as LEFT" by default. MASTER_RIGHT flips that one comparison
// (`return !is_keyboard_master();`) so the master half is RIGHT instead,
// matching this actual board. Verified by reading split_util.c directly, not
// assumed. Once a real left half exists, switch to EE_HANDS (or a handedness
// pin) so either half can be master.
// ---------------------------------------------------------------------------
#define MASTER_RIGHT

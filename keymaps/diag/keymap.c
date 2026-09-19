// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "i2c_master.h"

// What we EXPECT the OLED to answer on. Defined locally because
// OLED_DISPLAY_ADDRESS lives in oled_driver.h, which is not compiled in while
// the OLED feature is disabled.
#define EXPECTED_OLED_ADDR 0x3C

// ---------------------------------------------------------------------------
// Matrix-test / bring-up keymap -- RIGHT half of a split, only half wired so far.
//
// Global matrix rows 0-3 are the (not yet built) LEFT half -- left all KC_NO.
// Global rows 4-7 are THIS board's physical rows, one unique character each,
// so a hand-soldered junction can be verified just by typing into an editor:
//
//   row4 -> 7 8 9 0 - =      row5 -> y u i o p [
//   row6 -> h j k l ; '      row7 ->       n m ,   (thumbs, cols TENTATIVE)
//
// RIGHT-HAND characters on purpose: this is confirmed as the RIGHT half
// (global rows 4-7). The corne_hw project used 1-6/qwerty/asdfgh/zxc for ITS
// diagnostic build -- reusing those same LEFT-looking labels here was a
// mistake that caused real confusion (pressing this board's keys and seeing
// "wasd" reasonably looked like a wiring bug). It was not one: the matrix
// coordinates were correct the whole time, only the arbitrary test labels
// were misleading. These are NOT a final layout, just recognizable markers.
//
// These are declared as a RAW 2D array rather than via LAYOUT_split_3x6_3(...):
// the row/col reported by the console and by record->event.key come from the
// physical matrix scan, not from keymap argument order, so a raw array sidesteps
// any risk of miscounting the macro's 42-argument position order while wiring
// is still unverified. No modifiers or layer keys on purpose: a dead modifier
// looks identical to a dead solder joint, which is what we are ruling out.
// ---------------------------------------------------------------------------

#define _N KC_NO
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = {
        { _N,    _N,    _N,    _N,    _N,    _N    },  // row0 (left, absent)
        { _N,    _N,    _N,    _N,    _N,    _N    },  // row1 (left, absent)
        { _N,    _N,    _N,    _N,    _N,    _N    },  // row2 (left, absent)
        { _N,    _N,    _N,    _N,    _N,    _N    },  // row3 (left, absent)
        { KC_7,  KC_8,  KC_9,  KC_0,  KC_MINS, KC_EQL },  // row4 (this board)
        { KC_Y,  KC_U,  KC_I,  KC_O,  KC_P,  KC_LBRC },  // row5 (this board)
        { KC_H,  KC_J,  KC_K,  KC_L,  KC_SCLN, KC_QUOT },  // row6 (this board)
        { KC_N,  KC_M,  KC_COMM, _N,  _N,    _N    },  // row7 (this board, thumbs)
    }
};

// ---------------------------------------------------------------------------
// I2C bus scan.
//
// The point is to separate "the OLED is not wired / not responding" from
// "the OLED is fine but QMK is configured for the wrong address or driver".
// A silent bus means wiring or power; an ACK at an unexpected address means
// configuration. QMK's i2c API takes the SHIFTED 8-bit address (the OLED
// driver itself calls i2c_transmit(OLED_DISPLAY_ADDRESS << 1, ...)), so we
// shift our 7-bit candidates going in and report them unshifted.
// ---------------------------------------------------------------------------
static void i2c_bus_scan(void) {
    i2c_init();
    uprintf("\n=== I2C SCAN (7-bit addresses) ===\n");

    uint8_t found = 0;
    for (uint8_t addr = 0x08; addr < 0x78; addr++) {
        if (i2c_ping_address(addr << 1, 5) == I2C_STATUS_SUCCESS) {
            uprintf("  FOUND 0x%02X\n", addr);
            found++;
        }
    }

    uprintf("=== I2C SCAN: %u device(s) ===\n", found);
    if (found == 0) {
        uprintf("  bus silent -> check VCC, GND, and SDA/SCL orientation\n");
        uprintf("  SDA must be Pro Micro pin 2 (D1), SCL must be pin 3 (D0)\n");
    }
    uprintf("expected OLED address = 0x%02X\n", EXPECTED_OLED_ADDR);
}

static uint16_t boot_timer    = 0;
static bool     rescan_pending = true;

void keyboard_post_init_user(void) {
    debug_enable = true;
    // debug_matrix stays OFF: an unmapped matrix cell is currently chattering
    // and its per-change dumps drown out everything else on the console.
    debug_matrix = false;

    boot_timer = timer_read();
    i2c_bus_scan();   // usually LOST -- see housekeeping_task_user() below
}

void housekeeping_task_user(void) {
    // QMK's HID console DROPS output when no host is attached: sendchar()
    // bails out unless USB_DeviceState == DEVICE_STATE_Configured, and there
    // is no ring buffer. The boot-time scan above therefore almost always goes
    // nowhere, because `qmk console` needs ~100ms+ after enumeration to attach.
    // Re-run once, well after the host has certainly connected.
    if (rescan_pending && timer_elapsed(boot_timer) > 6000) {
        rescan_pending = false;
        uprintf("\n(delayed re-scan -- the boot-time one is dropped by design)\n");
        i2c_bus_scan();
    }
}

// Compact, rate-limited RAW matrix dump.
//
// debug_matrix prints a 6-line dump on every single change, which a chattering
// cell turns into an unreadable firehose. This caps reports at 10/second, which
// is far above human sweep speed (so no keypress is ever hidden) but low enough
// that electrical chatter cannot flood the console. Crucially it still covers
// matrix cells the layout does not map to any key, which process_record_user()
// can never see.
static void report_matrix_if_changed(void) {
    static matrix_row_t last[MATRIX_ROWS];
    static uint16_t     last_report = 0;
    static bool         pending     = false;

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        const matrix_row_t now = matrix_get_row(row);
        if (now != last[row]) {
            last[row] = now;
            pending   = true;
        }
    }

    if (!pending || timer_elapsed(last_report) < 100) return;

    char    line[MATRIX_ROWS * (MATRIX_COLS + 1) + 1];
    uint8_t n = 0;
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            line[n++] = (last[row] & (MATRIX_ROW_SHIFTER << col)) ? '#' : '.';
        }
        if (row + 1 < MATRIX_ROWS) line[n++] = '|';
    }
    line[n] = '\0';

    uprintf("MTX R1..R4: %s\n", line);
    last_report = timer_read();
    pending     = false;
}

void matrix_scan_user(void) {
    report_matrix_if_changed();
}

// Report the matrix coordinate of every real key event. This is what the
// per-key sweep should be read against -- it names the row/col directly, so a
// key landing on the wrong coordinate is immediately obvious.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uprintf("KEY %s  row=%u col=%u  kc=0x%04X\n",
            record->event.pressed ? "DOWN" : "UP  ",
            record->event.key.row, record->event.key.col, keycode);
    return true;
}

#ifdef OLED_ENABLE

// The module is lying flat on the bench while we wire, so render unrotated.
// Switch to OLED_ROTATION_270 once it is mounted upright on the keyboard.
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

// Live matrix map: '#' = contact closed, '.' = open.
//
// A 128x32 panel has exactly 4 text lines, so this shows only THIS BOARD'S
// local 4 rows -- not all 8 global rows (the other half doesn't exist yet, and
// would just be 4 lines of unchanging dots). quantum/matrix.c writes a split
// half's local scan into global rows [row_offset .. row_offset+3], where
// row_offset is 0 for the left half and MATRIX_ROWS_PER_HAND for the right
// (see `thisHand` in quantum/matrix.c) -- is_keyboard_left() is the same public
// call QMK itself uses to pick that offset, so this mirrors it exactly rather
// than guessing which global rows belong to "here".
bool oled_task_user(void) {
    const uint8_t row_offset = is_keyboard_left() ? 0 : MATRIX_ROWS_PER_HAND;

    for (uint8_t local_row = 0; local_row < MATRIX_ROWS_PER_HAND; local_row++) {
        const matrix_row_t state = matrix_get_row(row_offset + local_row);

        oled_set_cursor(0, local_row);
        oled_write_char('R', false);
        oled_write_char('1' + local_row, false);
        oled_write_char(' ', false);

        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            oled_write_char((state & (MATRIX_ROW_SHIFTER << col)) ? '#' : '.', false);
        }
    }
    return false;
}

#endif

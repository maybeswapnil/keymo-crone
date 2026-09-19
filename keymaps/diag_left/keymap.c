// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "i2c_master.h"

// What we EXPECT the OLED to answer on.
#define EXPECTED_OLED_ADDR 0x3C

// ---------------------------------------------------------------------------
// Matrix-test / bring-up keymap for the LEFT board, tested STANDALONE (split
// disabled via rules.mk -- see the comment there for why). Rows are a plain
// local 0-3, no handedness ambiguity.
//
// LEFT-hand-appropriate labels this time (this bit us on the right board: the
// SAME character set read as "left-hand" keys there and caused confusion --
// here it is actually correct):
//
//   row0 -> 1 2 3 4 5 6      row1 -> q w e r t y
//   row2 -> a s d f g h      row3 ->       z x c   (thumbs, cols TENTATIVE)
//
// Column order UNKNOWN and NOT ASSUMED -- the right board turned out to sweep
// in the un-reversed order, but corne_hw's left board needed a full reversal.
// Different physical boards, verify each one. No modifiers/layers on purpose:
// a dead modifier looks identical to a dead solder joint.
// ---------------------------------------------------------------------------

// Raw 2D array, not the LAYOUT(...) macro: keyboard.json only defines
// LAYOUT_split_3x6_3 now (42 args across both hands), which does not exist as
// a plain 21-key "LAYOUT" macro. A raw array sidesteps needing to know any
// macro's argument order at all.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = {
        { KC_1, KC_2, KC_3, KC_4, KC_5, KC_6 },
        { KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y },
        { KC_A, KC_S, KC_D, KC_F, KC_G, KC_H },
        { KC_Z, KC_X, KC_C, KC_NO, KC_NO, KC_NO },
    }
};

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

static uint16_t boot_timer     = 0;
static bool     rescan_pending = true;

void keyboard_post_init_user(void) {
    debug_enable  = true;
    debug_matrix  = false;
    boot_timer    = timer_read();
    i2c_bus_scan();   // usually LOST -- see housekeeping_task_user() below
}

void housekeeping_task_user(void) {
    // QMK's HID console drops output with no host attached and there is no
    // ring buffer, so the boot-time scan above is almost always lost. Re-run
    // once, well after `qmk console` has certainly attached.
    if (rescan_pending && timer_elapsed(boot_timer) > 6000) {
        rescan_pending = false;
        uprintf("\n(delayed re-scan -- the boot-time one is dropped by design)\n");
        i2c_bus_scan();
    }
}

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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uprintf("KEY %s  row=%u col=%u  kc=0x%04X\n",
            record->event.pressed ? "DOWN" : "UP  ",
            record->event.key.row, record->event.key.col, keycode);
    return true;
}

#ifdef OLED_ENABLE

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

bool oled_task_user(void) {
    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        const matrix_row_t state = matrix_get_row(row);
        oled_set_cursor(0, row);
        oled_write_char('R', false);
        oled_write_char('1' + row, false);
        oled_write_char(' ', false);
        for (uint8_t col = 0; col < MATRIX_COLS; col++) {
            oled_write_char((state & (MATRIX_ROW_SHIFTER << col)) ? '#' : '.', false);
        }
    }
    return false;
}

#endif

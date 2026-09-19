// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

// Every key is KC_NO on purpose: this build is a probe, not a keyboard. It must
// not type stray characters into whatever window has focus while you press keys.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = {
        {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
        {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
        {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
        {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
        {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
        {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
        {KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO, KC_NO},
    }
};

// Labels are "ProMicroLabel/AVRPin" so the report is directly actionable
// against the silkscreen, without translating port names in your head.
static const char *const ROW_LABEL[MATRIX_ROWS] = {
    "4/D4", "5/C6", "6/D7", "7/E6", "8/B4", "9/B5", "10/B6"
};
static const char *const COL_LABEL[MATRIX_COLS] = {
    "A3/F4", "A2/F5", "A1/F6", "A0/F7", "15/B1", "14/B3", "16/B2", "1/D3"
};

void keyboard_post_init_user(void) {
    debug_enable = true;
    debug_matrix = false;
    uprintf("\n=== PIN DISCOVERY (%ux%u superset scan) ===\n",
            (unsigned)MATRIX_ROWS, (unsigned)MATRIX_COLS);
    uprintf("rows: 4 5 6 7 8 9 10\n");
    uprintf("cols: A3 A2 A1 A0 15 14 16 1\n");
    uprintf("excluded: 2/D1 3/D0 (I2C), 0/D2 (TRRS)\n");
    uprintf("Press each key. Nothing types; each press names its two pins.\n\n");
}

// Report only edges (newly closed contacts), so holding a key prints once.
void matrix_scan_user(void) {
    static matrix_row_t last[MATRIX_ROWS];

    for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
        const matrix_row_t now   = matrix_get_row(row);
        const matrix_row_t newly = now & ~last[row];

        for (uint8_t col = 0; col < MATRIX_COLS && newly; col++) {
            if (newly & (MATRIX_ROW_SHIFTER << col)) {
                uprintf("BRIDGE  row %s  <->  col %s   [%u,%u]\n",
                        ROW_LABEL[row], COL_LABEL[col],
                        (unsigned)row, (unsigned)col);
            }
        }
        last[row] = now;
    }
}

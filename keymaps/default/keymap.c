// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "sprite.h"
#include "transactions.h"
#include <string.h>

// ---------------------------------------------------------------------------
// RIGHT half of Keymo Crone -- confirmed 21/21 on the very first sweep, zero
// wiring faults. Column 0 is OUTER (pinky-side) on this board -- verified
// empirically, not assumed from corne_hw (that board needed a reversal; this
// one did not. Physically different boards can wire either way).
//
// Declared as a RAW 8x6 array rather than LAYOUT_split_3x6_3(...): our
// keyboard.json's "layout" array interleaves left/right positions for visual
// x/y placement, so the macro's argument order does not read left-to-right,
// top-to-bottom the way a raw array does. Getting that 42-argument order
// wrong by hand would silently mis-map keys with no compile error to catch it
// -- not worth the risk versus a plain, unambiguous 2D array.
//
//   row0 -> 1 2 3 4 5 6      row1 -> q w e r t y
//   row2 -> a s d f g h      row3 ->       z x c
//
//   row4 -> 7 8 9 0 - =      row5 -> y u i o p [
//   row6 -> h j k l ; '      row7 ->       n m ,
//
// LEFT half (rows 0-3): column order confirmed REVERSED relative to the right
// board by direct test -- pressing the physically outermost top-row key
// reported col=5 before the fix. keyboard.json's base matrix_pins.cols is now
// the reversed order for this reason; split.matrix_pins.right restores the
// RIGHT board's already-verified original order so it needs no changes.
// Left-hand thumbs land at cols 3/4/5 (same physical F4/F5/F6 pins as the
// right board's thumbs), which happens to match crkbd's own convention.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// 3-layer QWERTY layout (Base / Lower / Raise), transcribed from the
// requested Oryx-style layout image. Thumb column mapping is derived from
// this project's already-verified physical order: on each half, LOWER column
// index = more OUTER (pinky-ward) -- proven for the main 3x6 grid and assumed
// to hold for the thumb row too, since thumbs share the same column wiring.
//   left thumbs  (outer->inner): col3=Ctrl,  col4=MO(_LOWER), col5=Space
//   right thumbs (outer->inner): col0=Alt,   col1=MO(_RAISE), col2=Enter
//
// UNCERTAIN reads worth double-checking against the source image (flagged in
// chat): the two rotated "Alt+arrow" thumb keys on Raise, the Raise-layer
// right-side window-management block, and BT1/BT2/BTclr (Bluetooth pairing --
// meaningless on this wired-only board; left as KC_NO for now).
// ---------------------------------------------------------------------------

enum layers { _BASE, _LOWER, _RAISE };

#define _N    KC_NO
#define _T    KC_TRNS
#define LOWER MO(_LOWER)
#define RAISE MO(_RAISE)

// Row order per hand is (0) top alpha, (1) home alpha, (2) bottom alpha,
// (3) thumbs -- left hand occupies global rows 0-3, right hand rows 4-7.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = {
        { KC_ESC,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T    },  // left top
        { KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G    },  // left home
        { KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B    },  // left bottom
        { _N,      _N,      _N,      KC_LCTL, LOWER,   KC_SPC  },  // left thumbs
        { KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSPC },  // right top
        { KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT },  // right home
        { KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT },  // right bottom
        { KC_LALT, RAISE,   KC_ENT,  _N,      _N,      _N      },  // right thumbs
    },
    [_LOWER] = {
        { KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5    },
        { KC_TILD, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC },
        { KC_PIPE, KC_UNDS, KC_MINS, KC_EQL,  KC_PLUS, KC_LGUI },
        { _N,      _N,      _N,      _T,      _T,      _T      },
        { KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_BSPC },
        { KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_BSLS },
        { KC_PSCR, KC_LBRC, KC_RBRC, KC_LCBR, KC_RCBR, KC_RSFT },
        { KC_ENT,  KC_LALT, KC_END,  _N,      _N,      _N      },
    },
    [_RAISE] = {
        { KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5   },
        { KC_TAB,  KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10  },
        { KC_LSFT, _N,      _N,      _N,      KC_F11,  KC_F12  },
        { _N,      _N,      _N,      KC_HOME, LALT(KC_LEFT), KC_SPC },
        { LCTL(LSFT(KC_F3)), LCTL(KC_MINS), KC_UP,   LCTL(LSFT(KC_MINS)), LCTL(KC_F3), KC_DEL },
        { LCTL(LSFT(KC_LEFT)), KC_LEFT, KC_DOWN, KC_RIGHT, LCTL(LSFT(KC_RIGHT)), KC_PGUP },
        { LCTL(KC_W), LCTL(KC_P), LCTL(KC_K), LCTL(KC_N), LALT(KC_F4), KC_PGDN },
        { KC_HOME, LALT(KC_RIGHT), KC_ENT,  _N, _N,    _N      },
    },
};

#ifdef OLED_ENABLE

// Mounted flat on the bench for now; switch to OLED_ROTATION_270 once upright.
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_0;
}

// Track TAB specifically: it's not a modifier bit, so get_mods() can't see it.
// Read on both halves regardless of which physical Tab key is pressed --
// process_record_user runs identically wherever the keypress originates.
static bool tab_held = false;

// ---------------------------------------------------------------------------
// STEP 1 of the keystroke-feed rebuild: bare RPC plumbing only. Sends a fixed
// 1-byte heartbeat from master to slave every 250ms. No rendering changes yet
// -- the point is to isolate whether the custom-RPC mechanism itself is what
// crashed the board when TRRS was connected, before adding back any of the
// actual keylog data or left-screen UI changes.
// ---------------------------------------------------------------------------
static uint8_t heartbeat = 0;

void keylog_sync_handler(uint8_t in_size, const void *in, uint8_t out_size, void *out) {
    if (in_size == sizeof(heartbeat)) {
        memcpy(&heartbeat, in, sizeof(heartbeat));
    }
}

void keyboard_post_init_user(void) {
    transaction_register_rpc(RPC_ID_KEYLOG_SYNC, keylog_sync_handler);
}

void housekeeping_task_user(void) {
    if (!is_keyboard_master()) return;
    static uint32_t last_sync = 0;
    if (timer_elapsed32(last_sync) > 250) {
        heartbeat++;
        transaction_rpc_send(RPC_ID_KEYLOG_SYNC, sizeof(heartbeat), &heartbeat);
        last_sync = timer_read32();
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (keycode == KC_TAB) {
        tab_held = record->event.pressed;
    }
    return true;
}

// ---------------------------------------------------------------------------
// RIGHT half: animated sprite + WPM (unchanged from before).
// ---------------------------------------------------------------------------
#define SPRITE_X        0
#define TEXT_COL        6
#define FRAME_MS_IDLE   420
#define FRAME_MS_FAST   110
#define WPM_FAST        45

// `x0` is an explicit parameter (not the old hardcoded SPRITE_X) because the
// right half puts the sprite on the LEFT of its screen and the left half puts
// it on the RIGHT of its own -- same frames, same function, different spot.
static void draw_sprite(uint8_t frame, uint8_t x0) {
    for (uint8_t page = 0; page < SPRITE_PAGES; page++) {
        const uint16_t dst = page * OLED_DISPLAY_WIDTH + x0;
        for (uint8_t x = 0; x < SPRITE_W; x++) {
            oled_write_raw_byte(pgm_read_byte(&sprite[frame][page * SPRITE_W + x]), dst + x);
        }
    }
}

// Raw pixels, not a font glyph: QMK's bundled glcdfont.c has no dependable
// solid-block character. 0x3C lights the middle 4 rows of one page. `page` is
// explicit because the two halves put the bar on different rows, and `width`
// is explicit (not just "rest of the panel") because on the left half the bar
// must stop before the sprite region, not run into it.
static void draw_wpm_bar(uint8_t wpm, uint8_t page, uint8_t x0, uint8_t width) {
    const uint8_t len = (wpm >= 120) ? width : (uint8_t)((uint16_t)wpm * width / 120);
    for (uint8_t i = 0; i < width; i++) {
        oled_write_raw_byte(i < len ? 0x3C : 0x00, page * OLED_DISPLAY_WIDTH + x0 + i);
    }
}

static void render_right_oled(void) {
    static uint16_t frame_timer = 0;
    static uint8_t  frame       = 0;

    const uint8_t  wpm      = get_current_wpm();
    const uint16_t interval = (wpm >= WPM_FAST) ? FRAME_MS_FAST
                            : (wpm > 0)         ? FRAME_MS_IDLE - (wpm * 4)
                                                : FRAME_MS_IDLE;

    if (timer_elapsed(frame_timer) > interval) {
        frame       = (frame + 1) % SPRITE_FRAMES;
        frame_timer = timer_read();
        draw_sprite(frame, SPRITE_X);
    }

    // Full board name -- fits the 15-char budget left of the sprite (TEXT_COL*6
    // = px36, panel is 128px wide -> 92px = ~15 chars free).
    oled_set_cursor(TEXT_COL, 0);
    oled_write_P(PSTR("Keymo Crone"), false);

    oled_set_cursor(TEXT_COL, 1);
    oled_write_P(PSTR("wpm "), false);
    char buf[4];
    buf[0] = wpm >= 100 ? '0' + (wpm / 100)     : ' ';
    buf[1] = wpm >= 10  ? '0' + (wpm / 10) % 10 : ' ';
    buf[2] = '0' + wpm % 10;
    buf[3] = '\0';
    oled_write(buf, false);

    draw_wpm_bar(wpm, 2, TEXT_COL * OLED_FONT_WIDTH, OLED_DISPLAY_WIDTH - TEXT_COL * OLED_FONT_WIDTH);

    oled_set_cursor(TEXT_COL, 3);
    const led_t led = host_keyboard_led_state();
    oled_write_P(led.caps_lock ? PSTR("CAPS ") : PSTR("     "), false);
    oled_write_P(tab_held      ? PSTR("TAB")   : PSTR("   "), false);
}

// ---------------------------------------------------------------------------
// LEFT half: this is the board NOT plugged into USB in normal use. Same
// critter sprite as the right half, mirrored to the RIGHT side of its screen
// (x=96..127) instead of the left, so the two boards read as a matched pair
// rather than identical layouts. Text is confined to columns 0-15 (0-95px) so
// it never overwrites the sprite -- same non-overlap discipline as the right
// half's SPRITE_X/TEXT_COL split, just mirrored.
//
// Modifier state (get_mods/get_weak_mods) and WPM both require the
// split.transport.sync entries in keyboard.json to be accurate here --
// confirmed against quantum/split_common/transactions.c: layer_state, LED
// state and WPM sync unconditionally, but modifiers need "sync":
// {"modifiers": true} explicitly, which is why that flag was added rather
// than assumed to already work.
// ---------------------------------------------------------------------------
#define LEFT_SPRITE_X     (OLED_DISPLAY_WIDTH - SPRITE_W)   // 96: right-aligned
#define LEFT_TEXT_CHARS   16                                 // columns 0-15, 0-95px

static void render_left_oled(void) {
    static uint16_t frame_timer = 0;
    static uint8_t  frame       = 0;

    const uint8_t wpm = get_current_wpm();
    const uint16_t interval = (wpm >= WPM_FAST) ? FRAME_MS_FAST
                            : (wpm > 0)         ? FRAME_MS_IDLE - (wpm * 4)
                                                : FRAME_MS_IDLE;
    if (timer_elapsed(frame_timer) > interval) {
        frame       = (frame + 1) % SPRITE_FRAMES;
        frame_timer = timer_read();
        draw_sprite(frame, LEFT_SPRITE_X);
    }

    // Board name -- 11 chars fits the 16-char text budget with room to spare.
    oled_set_cursor(0, 0);
    oled_write_P(PSTR("Keymo Crone     "), false);

    oled_set_cursor(0, 1);
    switch (get_highest_layer(layer_state)) {
        case _LOWER: oled_write_P(PSTR("LOWER "), false); break;
        case _RAISE: oled_write_P(PSTR("RAISE "), false); break;
        default:     oled_write_P(PSTR("BASE  "), false); break;
    }

    const uint8_t mods = get_mods() | get_weak_mods();
    oled_write_char(mods & MOD_MASK_CTRL  ? 'C' : '_', false);
    oled_write_char(mods & MOD_MASK_SHIFT ? 'S' : '_', false);
    oled_write_char(mods & MOD_MASK_ALT   ? 'A' : '_', false);
    oled_write_char(mods & MOD_MASK_GUI   ? 'G' : '_', false);
    oled_write_P(PSTR("      "), false);   // pad to LEFT_TEXT_CHARS (6+4+6=16)

    oled_set_cursor(0, 2);
    const led_t led = host_keyboard_led_state();
    oled_write_P(led.caps_lock ? PSTR("CAPS") : PSTR("    "), false);
    oled_write_P(tab_held      ? PSTR(" TAB")   : PSTR("    "), false);
    oled_write_P(PSTR("        "), false);   // pad to 16 (4+4+8)

    oled_set_cursor(0, 3);
    oled_write_P(PSTR("wpm "), false);
    char buf[4];
    buf[0] = wpm >= 100 ? '0' + (wpm / 100)     : ' ';
    buf[1] = wpm >= 10  ? '0' + (wpm / 10) % 10 : ' ';
    buf[2] = '0' + wpm % 10;
    buf[3] = '\0';
    oled_write(buf, false);
    // Bar fills the rest of the text budget only (8..15 -> px48..95), stopping
    // well short of the sprite at px96 rather than running the full panel width.
    draw_wpm_bar(wpm, 3, 8 * OLED_FONT_WIDTH, (LEFT_TEXT_CHARS - 8) * OLED_FONT_WIDTH);
}

bool oled_task_user(void) {
    if (is_keyboard_left()) {
        render_left_oled();
    } else {
        render_right_oled();
    }
    return false;
}

#endif

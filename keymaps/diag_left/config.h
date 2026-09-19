// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// keyboard.json's "split.enabled" makes info_config.h auto-double MATRIX_ROWS
// to 8, REGARDLESS of rules.mk's SPLIT_KEYBOARD flag -- that generation step
// runs from the JSON directly, before rules.mk is even read (confirmed by
// checking: disabling SPLIT_KEYBOARD alone left MATRIX_ROWS at 8). Override it
// back to 4 here, the same technique already verified working for `pinscan`.
#undef  MATRIX_ROWS
#define MATRIX_ROWS 4

// Confirmed by direct test: pressing the physically outermost (pinky-side)
// top-row key reported col=5 ('6'), meaning this board's column order is the
// mirror of the base keyboard.json order -- same pattern corne_hw's left half
// needed. Reversing just for this diagnostic build to verify before touching
// the shared keyboard.json (which the already-flashed right board depends on).
#undef  MATRIX_COL_PINS
#define MATRIX_COL_PINS { B3, B1, F7, F6, F5, F4 }

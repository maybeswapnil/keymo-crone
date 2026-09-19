// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// ---------------------------------------------------------------------------
// PIN DISCOVERY build.
//
// Instead of the six columns and four rows we *assume* are wired, scan a
// SUPERSET of every usable Pro Micro GPIO. Then pressing any key reports which
// two physical pins it actually bridges -- so misplaced bus wires identify
// themselves, with no continuity probing at all.
//
// These override keyboard.json because the generated info_config.h wraps every
// value in `#ifndef`, and the keymap's config.h is processed first.
//
// Deliberately EXCLUDED:
//   D1 (pin 2) / D0 (pin 3) -- I2C, reserved for the OLED
//   D2 (pin 0)              -- reserved for TRRS split data
// Scanning those would fight the OLED driver and mislead the split bring-up.
// ---------------------------------------------------------------------------

#undef MATRIX_ROWS
#undef MATRIX_COLS
#define MATRIX_ROWS 7
#define MATRIX_COLS 8

// Pro Micro:  4    5    6    7    8    9    10
#undef  MATRIX_ROW_PINS
#define MATRIX_ROW_PINS { D4,  C6,  D7,  E6,  B4,  B5,  B6 }

// Pro Micro:  A3   A2   A1   A0   15   14   16   1
#undef  MATRIX_COL_PINS
#define MATRIX_COL_PINS { F4,  F5,  F6,  F7,  B1,  B3,  B2,  D3 }

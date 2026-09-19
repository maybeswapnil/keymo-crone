// Copyright 2026 swapnil
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

// Custom split RPC channel -- see keymap.c for why this is needed (the slave
// half never resolves real keycodes, so "what was just pressed" isn't covered
// by QMK's automatic sync). Pattern verified against keyboards/spleeb/spleeb.c
// and quantum/split_common source before writing any of this.
#define SPLIT_TRANSACTION_IDS_USER RPC_ID_KEYLOG_SYNC

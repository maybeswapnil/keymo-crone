# Standalone bring-up for the LEFT board, tested with no TRRS connection yet.
# Split-disabled on purpose: with only MASTER_RIGHT configured (no handedness
# pin / EE_HANDS), whichever half has USB is "master" and MASTER_RIGHT then
# makes IT identify as right -- so a lone left board tested under the normal
# split-aware `diag` build would mislabel itself and report rows 4-7, exactly
# the kind of mismatch that just caused real confusion on the right board.
# Disabling split for this bring-up keymap keeps rows a clean, unambiguous 0-3
# regardless of master/USB status, deferring left/right logic until wiring is
# verified. (Same technique already verified working for `pinscan`: confirmed
# via `serial.o` absence in the build output.)
SPLIT_KEYBOARD = no

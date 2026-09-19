# Pin discovery is a standalone diagnostic: it scans a superset of raw GPIOs,
# unrelated to the split transport/handedness logic. Force split off for this
# keymap only so MATRIX_ROWS_PER_HAND math (MATRIX_ROWS/2) doesn't fight our
# custom 7-row override in config.h.
SPLIT_KEYBOARD = no

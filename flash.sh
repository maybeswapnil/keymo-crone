#!/bin/bash
# Compile and flash a keymap to the Pro Micro.
#
#   ./flash.sh            # the real keyboard (default keymap)
#   ./flash.sh pinscan    # the pin-discovery probe
#
# Why this exists instead of `qmk flash`:
#   QMK's platforms/avr/flash.mk points avrdude at /dev/tty.*, which on macOS
#   blocks on DCD and dies with "Device not configured" / butterfly_send errors.
#   /dev/cu.* works first time.
#
# Bootloader detection is by USB VID: QMK runs as 0xFEED, so any appearance of
# SparkFun's 0x1B4F means Caterina. (Note 0x9205 is the BOOTLOADER pid on this
# board and 0x9206 is a running Arduino sketch -- the reverse of the common
# assumption.)
set -u
export PATH="/opt/homebrew/opt/avr-gcc@8/bin:$PATH"

KM="${1:-default}"
KB="handwired/keymo_crone"
HEX="$HOME/qmk_firmware/handwired_keymo_crone_${KM}.hex"
SPARKFUN_VID=6991   # 0x1B4F

echo ">>> compiling $KB:$KM"
qmk compile -kb "$KB" -km "$KM" || exit 1
[ -f "$HEX" ] || { echo "FATAL: $HEX missing"; exit 1; }

vids()     { ioreg -p IOUSB -l -w 0 2>/dev/null | grep '"idVendor"' | tr -dc '0-9\n'; }
in_boot()  { vids | grep -q "^${SPARKFUN_VID}$"; }
node()     { ls -t /dev/cu.usbmodem* 2>/dev/null | head -1; }

echo
echo ">>> enter the bootloader: hold the Bootmagic key (see keyboard.json's"
echo "    bootmagic/split.bootmagic matrix) and re-plug USB, KEEP HOLDING ~2s"
echo "    (Bootmagic sets Caterina's RAM magic key -> full 8s window;"
echo "     a plain reset tap gives only ~750ms, too short for a ~1.4s write)"

end=$((SECONDS + 300))
while [ $SECONDS -lt $end ]; do
  in_boot || { sleep 0.02; continue; }
  echo ">>> bootloader detected"

  # Re-resolve the node on EVERY attempt: macOS re-creates it during
  # enumeration, and a cached name goes stale mid-window.
  for i in $(seq 1 160); do
    p="$(node)"
    if [ -n "$p" ] && [ -e "$p" ]; then
      avrdude -p atmega32u4 -c avr109 -P "$p" -U flash:w:"$HEX":i && {
        echo ">>> FLASH OK ($KM)"; exit 0; }
    fi
    in_boot || { echo ">>> window closed after $i tries"; break; }
    sleep 0.05
  done
done
echo ">>> TIMEOUT: bootloader never appeared"
exit 1

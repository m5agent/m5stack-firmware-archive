#!/usr/bin/env bash
# Run the prebuilt CoreS3 documentation sketches on the native simulator.
# Usage: ESP32SIM_ROM_DIR=... run_all_esp32sim.sh BINARY NEW_OUTDIR
set -euo pipefail

binary=${1:?simulator binary required}
out=${2:?new output directory required}
archive=$(cd "$(dirname "$0")/.." && pwd)
firmware=${CORES3_FIRMWARE_DIR:-"$archive/devices/cores3/arduino-docs"}
inputs=${CORES3_FIXTURES:-"$archive/fixtures"}
wifi=${CORES3_RTC_WIFI:-'ssid=***********,psk=***********'}
: "${ESP32SIM_ROM_DIR:?set ESP32SIM_ROM_DIR to the directory containing esp32s3_rev0_rom.elf}"
export ESP32SIM_ROM_DIR
seed=${CORES3_SD_SEED:-"$inputs/seed.img.gz"}
mkdir -p "$(dirname "$out")"
mkdir "$out"
# Cargo can relink the original binary while the suite is running.
cp "$binary" "$out/esp32sim"
binary="$out/esp32sim"

case "$seed" in
    *.gz) gzip -dc "$seed" > "$out/sd-seed.img" ;;
    *) cp "$seed" "$out/sd-seed.img" ;;
esac
for label in sdcard sdcard-before pic pic-before wav; do
    cp "$out/sd-seed.img" "$out/sd-$label.img"
done
cp "$inputs/pic-a.bmp" "$out/camera-input.bmp"
cp "$inputs/pic-b.bmp" "$out/camera-before-input.bmp"
# The sketch clears the marker on state changes. Two moves leave a point
# visible after the firmware enters the stable flick state.
printf '%s\n' '3.0 touch 30 30 1' '3.12 touch 70 70 1' '3.17 touch 100 120 1' '3.4 touch 100 120 0' > "$out/touch.txt"
printf '%s\n' '3.0 pwrkey 250' > "$out/button.txt"
printf '%s\n' '3.0 battery 4123 87 1 1' '7.0 battery 3678 42 0 0' > "$out/power.txt"
printf '%s\n' '6.0 touch 160 120 1' '6.2 touch 160 120 0' > "$out/wakeup.txt"
cp "$out/wakeup.txt" "$out/mic.txt"
cp "$inputs/imu.txt" "$out/imu.txt"
cp "$inputs/ltr553.txt" "$out/ltr553.txt"
cp "$inputs/mic-733hz.wav" "$out/mic-input.wav"

shasum -a 256 "$binary" "$ESP32SIM_ROM_DIR/esp32s3_rev0_rom.elf" \
    "$firmware"/*/*.ino.merged.bin "$out"/*.txt "$out"/*-input.* "$out/sd-seed.img" > "$out/inputs.sha256"

run() {
    local label=$1 case=$2 seconds=$3
    shift 3
    local sketch=${case//-/_}
    local status=0
    "$binary" --board m5stack-cores3 --boot rom --flash-image "$firmware/$case/$sketch.ino.merged.bin" \
        --console both --max-seconds "$seconds" --tft-png "$out/$label.png" --no-dump "$@" \
        > "$out/$label.log" 2> "$out/$label.err" || status=$?
    printf '%s\n' "$status" > "$out/$label.status"
    return "$status"
}

# Keep running after a guest failure so the checker can report every case.
pids=()
start() { run "$@" & pids+=("$!"); }
failed=0
join() {
    for pid in "${pids[@]}"; do wait "$pid" || failed=1; done
    pids=()
}

start display display 6
start display-before display 4
start quickstart-bargraph quickstart-bargraph 6
start quickstart-bargraph-before quickstart-bargraph 4
join
start touch touch 3.6 --script "$out/touch.txt"
start touch-before touch 3.2 --script "$out/touch.txt"
start button button 5 --script "$out/button.txt"
start button-before button 2.5 --script "$out/button.txt"
join
start power power 10 --script "$out/power.txt"
start power-before power 6 --script "$out/power.txt"
start imu imu 5 --script "$out/imu.txt"
start imu-before imu 9 --script "$out/imu.txt"
start ltr553 ltr553 10 --script "$out/ltr553.txt"
start ltr553-before ltr553 5 --script "$out/ltr553.txt"
join
start rtc rtc 10 --wifi "$wifi,epoch=1764000000" --net none
start rtc-before rtc 8 --wifi "$wifi,epoch=1764000000" --net none
start speaker speaker 6 --wav "$out/speaker.wav"
start mic mic 9 --mic-wav "$out/mic-input.wav" --script "$out/mic.txt" --wav "$out/mic.wav"
start mic-before mic 4 --mic-wav "$out/mic-input.wav" --script "$out/mic.txt"
join
start camera camera 6 --cam-image "$out/camera-input.bmp"
start camera-before camera 6 --cam-image "$out/camera-before-input.bmp"
start wakeup wakeup 12 --script "$out/wakeup.txt"
start wakeup-before wakeup 8 --script "$out/wakeup.txt"
join
start sdcard sdcard 4.5 --sd-rw "$out/sd-sdcard.img"
start sdcard-before sdcard 4 --sd-rw "$out/sd-sdcard-before.img"
start pic pic 4.4 --sd-rw "$out/sd-pic.img"
start pic-before pic 3.4 --sd-rw "$out/sd-pic-before.img"
start wav wav 14 --sd-rw "$out/sd-wav.img" --wav "$out/wav.wav" --console usb
join
python3 "$archive/tools/check_cores3.py" "$out" >"$out/verdicts.md" || failed=1
cat "$out/verdicts.md"
exit "$failed"

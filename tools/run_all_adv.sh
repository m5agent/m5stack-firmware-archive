#!/usr/bin/env bash
# Run and check the pinned Cardputer-ADV docs firmware matrix.
# Usage: ADV_FIRMWARE_DIR=... ADV_SD_SEED=... run_all_adv.sh BINARY NEW_OUTDIR
set -euo pipefail

binary=${1:?simulator binary required}
out=${2:?new output directory required}
archive=$(cd "$(dirname "$0")/.." && pwd)
firmware=${ADV_FIRMWARE_DIR:-"$archive/devices/cardputer-adv/arduino-docs"}
seed=${ADV_SD_SEED:-"$archive/fixtures/seed.img.gz"}
: "${ESP32SIM_ROM_DIR:?set ESP32SIM_ROM_DIR to the directory containing esp32s3_rev0_rom.elf}"
export ESP32SIM_ROM_DIR
checker="$archive/tools/check_adv.py"
mkdir -p "$(dirname "$out")"
mkdir "$out"
out=$(cd "$out" && pwd)
cp "$binary" "$out/esp32sim"
case "$seed" in
    *.gz) gzip -dc "$seed" > "$out/sd-seed.img" ;;
    *) cp "$seed" "$out/sd-seed.img" ;;
esac
cp "$out/sd-seed.img" "$out/sd-files.img"
cp "$out/sd-seed.img" "$out/sd-wav.img"

cat >"$out/keyboard.txt" <<'SCRIPT'
2.0 key a
2.3 key space
2.6 key b
2.9 key backspace
3.2 key c
3.5 key enter
4.0 key fn
SCRIPT
sed 's/^2.9 key backspace$/2.82 key fn 250\
2.9 key del 75/' "$out/keyboard.txt" >"$out/keyboard-chord.txt"
printf '%s\n' '2.0 press 0 150' '3.0 press btn 150' '4.0 press g0 150' >"$out/button.txt"
printf '%s\n' '4.0 adc 10 2.0' >"$out/battery.txt"
printf '%s\n' '2.0 imu 0.5 -0.25 1.0 125 -50 25' '4.0 imu -0.75 0.3 0.25 -100 80 -30' >"$out/imu.txt"
printf '%s\n' '4.5 press g0 150' >"$out/mic.txt"
python3 - "$out" <<'PY'
from array import array
from math import sin, tau
from pathlib import Path
import sys
import wave

out = Path(sys.argv[1])
rate = 17000
for filename, amplitude in [('mic-input.wav', 192), ('mic-silence.wav', 0)]:
    samples = array('h', (round(amplitude * sin(tau * 733 * i / rate)) for i in range(rate * 12)))
    if sys.byteorder == 'big':
        samples.byteswap()
    with wave.open(str(out / filename), 'wb') as wav:
        wav.setparams((1, 2, rate, 0, 'NONE', 'not compressed'))
        wav.writeframes(samples.tobytes())
PY
shasum -a 256 "$out/esp32sim" "$ESP32SIM_ROM_DIR/esp32s3_rev0_rom.elf" \
    "$firmware"/*/*.ino.merged.bin "$out"/*.txt "$out"/mic-*.wav "$out/sd-seed.img" >"$out/inputs.sha256"

run() {
    local label=$1 case=$2 seconds=$3 status=0
    shift 3
    "$out/esp32sim" --board m5stack-cardputer-adv --boot rom \
        --flash-image "$firmware/$case/${case//-/_}.ino.merged.bin" --console both \
        --max-seconds "$seconds" --tft-png "$out/$label.png" --no-dump "$@" \
        >"$out/$label.log" 2>"$out/$label.err" || status=$?
    printf '%s\n' "$status" >"$out/$label.status"
    return "$status"
}

pids=()
failed=0
start() { run "$@" & pids+=("$!"); }
join() {
    for pid in "${pids[@]}"; do wait "$pid" || failed=1; done
    pids=()
}

for case in display quickstart-display; do
    start "$case-before" "$case" 3
    start "$case" "$case" 8
done
join
start keyboard-before keyboard 1.8 --script "$out/keyboard.txt"
start keyboard keyboard 8 --script "$out/keyboard.txt"
start keyboard-chord keyboard 5 --script "$out/keyboard-chord.txt"
start button button 8 --script "$out/button.txt"
start ir_nec-before ir_nec 3
start ir_nec ir_nec 8
join
start battery-before battery 3 --script "$out/battery.txt"
start battery battery 6 --script "$out/battery.txt"
start imu-before imu 3 --script "$out/imu.txt"
start imu imu 6 --script "$out/imu.txt"
start speaker speaker 8 --wav "$out/speaker.wav"
start sdcard-first sdcard 12 --sd-rw "$out/sd-files.img"
start speaker-sd-wav speaker-sd-wav 13.5 --sd "$out/sd-wav.img" --wav "$out/speaker-sd-wav.wav"
join
# Record the writable card after the first boot and before the persistence replay.
(cd "$out" && shasum -a 256 sd-files.img) >"$out/sd-files-before-replay.sha256"
start mic-rec mic 3.9 --mic-wav "0:$out/mic-input.wav"
start mic-silence mic 3.9 --mic-wav "0:$out/mic-silence.wav"
start mic-play mic 5.5 --mic-wav "0:$out/mic-input.wav" --script "$out/mic.txt"
start mic mic 10 --mic-wav "0:$out/mic-input.wav" --script "$out/mic.txt" --wav "$out/mic.wav"
start sdcard sdcard 8 --sd-rw "$out/sd-files.img"
join
python3 "$checker" "$out" >"$out/verdicts.md" || failed=1
cat "$out/verdicts.md"
exit "$failed"

# Rebuild the board documentation matrix

The English Arduino examples are pinned to docs revision
`7219cc9286c39b463da84053deee40bc234625db`. Each case manifest links to
its docs page and the location within that revision. The original compiler
receipts and defect evidence are from the [integration record](https://github.com/m5agent/agent-work-logs/tree/6806ff46edbe0a5d3c7a28059ca1e6b0adbd6d2e/records/esp32sim/2026-11-27-lx6b-integration).

Use Arduino CLI 1.3.1, M5Stack board package 3.2.5 and the macOS arm64
toolchain from the [original rebuild recipe](rebuild-2026-11-26.md).
The exact FQBN and any library override appear in the case manifest.
Compile errors are recorded alongside the original sketch and compiler
diagnostic. A case with a null firmware hash has no archived image.

The [library lock](arduino-matrix-library-lock.json) records the exact
library versions, source commits or checksummed ZIPs in the build
environment. Install the listed versions in `ARDUINO_DIRECTORIES_USER/libraries`.
Use the `directory` name for Git sources and the archive's top-level
directory for ZIP sources. Keep override libraries in a separate directory
and pass its path as `--library-overrides`. A manifest with overrides names
the cases that select them. StamPLC Modbus uses the pinned M5Stack forks.
Nesso N1 and PaperMono LoRa use RadioLib 7.3.0.

Rebuild a published case from the archive root after setting the
installation variables from the original recipe.

```bash
python3 tools/rebuild_arduino.py \
    --cli "$cli" --config "$config" \
    --data "$ARDUINO_DIRECTORIES_DATA" --user "$ARDUINO_DIRECTORIES_USER" \
    --esptool-dir "$install/esptool/bin" --library-overrides "$install/overrides" \
    --out "$install/rebuild" --jobs 1 --compiler-jobs 2 \
    atoms3r-cam/quickstart-camera
python3 tools/scan_firmware_paths.py "$install/rebuild/firmware"
```

The recipe uses `SOURCE_DATE_EPOCH=1795651200`, `TZ=UTC` and the three
GCC prefix maps from the original recipe, including any alternate library
root under `/build-env/library-overrides`. The manifest records the sketch
basename passed to the CLI. Two builds from independent roots must
produce the same app, merged image, bootloader, partition table and ELF.
The strings scan checks all five outputs.

The replay uses native esp32sim source
[`4e257b6b`](https://github.com/m5agent/esp32sim/tree/4e257b6be8ee0ed706c2e509d8f31ef7472a424b).
Compile with `cargo build --locked --release -p esp32sim --bin esp32sim --bin esp32sim-p4`.
The case manifest records the exercised board, runtime SHA-256, runner
command, checker verdict and capture hashes. The runner's `$FIRMWARE_DIR`
contains `DEVICE/CASE/sketch.merged.bin` and, where used, `sketch.elf`.
Copy or symlink these from the corresponding verified archive files.
`$ROM_DIR` contains the matching ROM ELFs from the simulator's ROM fetch helper.
`$OUT` names a new capture directory. Use the matching fixtures and stock
firmware listed in the integration record for the relevant board.

Classic ESP32 ELFs include a path from debug sections in a precompiled
SDK library. Those cases use the pinned `xtensa-esp-elf-objcopy --strip-debug`
on each ELF after the Arduino build. The defined-symbol names and addresses
matched before and after stripping in the Atom Lite control. The merged
flash image is still the original Arduino build output, and each stripped
ELF and merged image must pass the path scan and double-build comparison.

## Tab5 and C6

Build the P4 examples with `m5stack:esp32:m5stack_tab5` and the manifest's
`sketch` basename. The ESP32-P4 and ESP32-C6 builds use the macOS arm64
`esp-rv32@2411` archive with SHA-256
`a7276042a7eb2d33c2dff7167539e445c32c07d43a2c6827e86d035642503e0b`.
The original Tab5 speaker compile failure and the corrected speaker are
separate cases. The SoftAP variant enables the AP block from the pinned
station/AP example.

Use the [Tab5 capture runner and checker](https://github.com/m5agent/agent-work-logs/tree/b2c5b12a4d45e63069a6d15e6393b0c60f72e1b6/records/esp32sim/2026-11-26-tab5-milestone-3/code).
Set `ESP32SIM_P4` to the pinned `esp32sim-p4` executable and
`ESP32SIM_ROM_DIR` to the ROM directory. For Wi-Fi and SNTP, build the
ESP-Hosted 2.0.13 SDIO fixture using the recipe there and use its matched
C6 segments as `$C6_COPROCESSOR`. Each manifest gives the runner commands
and the outcome for its rebuilt image.

# Rebuild the Cardputer K132 Arduino firmware

The display, keyboard and microSD sketches are pinned to documentation
revision `7219cc9286c39b463da84053deee40bc234625db`. Their case manifests record the
original hashes, the rebuilt images and the precise source locations.

Install Arduino CLI 1.3.1, the M5Stack board package 3.2.5 and its locked SDK and
macOS arm64 toolchain using the [original rebuild recipe](rebuild-2026-11-26.md).
The Cardputer K132 uses `m5stack:esp32:m5stack_cardputer` with the package defaults.
Use the M5Unified, M5GFX and M5Cardputer commits in the case manifests.
Both builds set `SOURCE_DATE_EPOCH=1795651200` and `TZ=UTC` and apply the
file, debug and macro prefix maps from the same recipe. The three builds in
each build root used two compiler jobs. The compiled assets matched byte for byte
across roots. Staged firmware had zero private path or identity matches.

Run `python3 tools/rebuild_arduino.py` with the options in the original recipe
to rebuild the indexed cases. Run `python3 tools/verify.py` to verify the
archived flash segments, file sizes and identities.

For native virtual-device validation, use Device Service commit
`16faae71c3ee67f5158431dbe78c27e726e67d3d` and its pinned runtime. Create a
`cardputer` session with `runtime=esp32sim` for each merged image. Sample changing
panel frames for display. Send `a`, `b` and `enter` through the keyboard for the
keyboard case. Supply a blank FAT32 image for microSD. Verify the readback and
rename markers and export the changed SD image after stopping the guest.
[Exact runtime and output hashes](../evidence/cardputer/2026-11-27-path-clean-summary.json)

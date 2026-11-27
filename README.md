# M5Stack 固件索引与模拟器复测

记录 M5Stack 文档例程版本、固件 SHA-256、Flash 布局和复测脚本。2026-11-26，CoreS3 15 个例程和 Cardputer-Adv 11 个例程的去除私有编译路径版镜像在固定版本的 esp32sim 上全部通过。官方 CoreS3 UIFlow2 v2.5.3 通过启动、USB CDC REPL 和 LCD 绘制检查。两项 CoreS3 Unit 用例使用 2026-11-25 的 QEMU-WASM 候选配置。所有通过项均为 **virtual-device validation**。真机验证为 **NOT RUN**。

[机器可读索引](index.json)收录完整 SHA-256。各用例清单包含文档小节、源码版本、工具链、实际输入、镜像分段和输出摘要。[来源与许可](NOTICE)记录依赖和固件出处。

## 获取与校验

Git LFS 提供已发布 Arduino 例程的合并镜像、应用 BIN、bootloader、分区表、ELF、共享的 boot_app0 及压缩 SD 镜像。各例程的 `original_build` 字段保留原始构建的 SHA-256 和验证记录。官方 UIFlow2 固件的下载地址和 SHA-256 记录在索引或对应机型清单中。

```bash
GIT_LFS_SKIP_SMUDGE=1 git clone https://github.com/m5agent/m5stack-firmware-archive.git
cd m5stack-firmware-archive
git lfs install --local
git lfs pull
python3 tools/verify.py
```

需要 Git LFS 和 Python 3.11+。可选的 `python3 tools/fetch_uiflow2.py` 下载官方 UIFlow2 固件。`python3 tools/import_arduino.py "$PREBUILT"` 可导入历史原件，`python3 tools/verify.py --with-local` 也校验本地原件与 UIFlow2。

[重建命令和工具链](build-records/rebuild-2026-11-26.md)记录了路径映射和版本。[本次验证摘要](evidence/arduino-path-clean-summary.json)包含大小、路径扫描、两次重建的字节比较和行为对比。

## 扩展机型

[机器可读索引](index.json)逐项记录固件下载路径、文档出处、源码版本、
验证结果和模拟器命令。编译失败的条目保留诊断和原始源码位置。
[扩展矩阵重建命令](build-records/rebuild-2026-11-27-matrix.md)包含依赖锁、路径映射和对应的复测脚本。

[Cardputer-ADV UIFlow2 源码例程](devices/cardputer-adv/uiflow2/README.md)
保留实际执行的 MicroPython 源码、CAP 测试结果和官方固件下载散列。

## esp32sim 复测

从 [esp32sim 固定提交](https://github.com/m5agent/esp32sim/tree/76e8b35710d1879e073f73c4a03bd11b216fb95f)构建。以下命令在本仓库根目录执行。两个批量脚本包含每个用例的完整命令和输入，输出目录必须是新目录。Cardputer-Adv 的 SD 文件用例连续使用同一张可写卡来检查数据持久化。

```bash
mkdir -p .local
sim_src="$PWD/.local/esp32sim"
git clone --branch m5-milestone-cores3-adv-20261126 https://github.com/m5agent/esp32sim.git "$sim_src"
test "$(git -C "$sim_src" rev-parse HEAD)" = 76e8b35710d1879e073f73c4a03bd11b216fb95f
cargo build --manifest-path "$sim_src/Cargo.toml" --release --locked -p esp32sim --bin esp32sim
export ESP32SIM_ROM_DIR="$PWD/.local/roms"
sh "$sim_src/tools/fetch-rom-elfs.sh" "$ESP32SIM_ROM_DIR"
sim="$sim_src/target/release/esp32sim"

bash tools/run_all_esp32sim.sh "$sim" .local/replay-cores3
bash tools/run_all_adv.sh "$sim" .local/replay-adv
python3 tools/check_cores3.py .local/replay-cores3
python3 tools/check_adv.py .local/replay-adv

run=$(mktemp -d "$PWD/.local/uiflow2-replay.XXXXXX")
"$sim" --board m5stack-cores3 --boot rom \
    --flash-image .local/firmware/cores3/uiflow2/v2.5.3/uiflow2-v2.5.3.bin \
    --usb-host fixtures/uiflow2-usb.txt --max-seconds 18 --tft-png "$run/uiflow-red.png" \
    --no-dump >"$run/uiflow.log" 2>"$run/uiflow.err"
python3 tools/check_uiflow2.py "$run"
```

路径可通过 `CORES3_FIRMWARE_DIR`、`ADV_FIRMWARE_DIR`、`CORES3_FIXTURES`、`CORES3_SD_SEED`、`ADV_SD_SEED` 和 `ESP32SIM_ROM_DIR` 配置。RTC 用例使用模拟 AP 与 NTP，SSID 和密码都是例程中的 `***********` 占位符。

QEMU-WASM Unit 证据见[候选配置摘要](evidence/cores3/accessory-qemu-wasm-summary.json)。该配置使用了局部 profile 修改。默认配置下 Relay 为 PARTIAL，Ultrasonic I2C 为 FAIL。esp32sim 上两项均为 NOT RUN。

## CoreS3

| 用例 | 文档 | 合并镜像 SHA-256 | 验证结果 |
| --- | --- | --- | --- |
| [button](devices/cores3/arduino-docs/button/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/button) | `58ea9e954d59f110737b02c776bfd290fe03f0e3d6dd51fc6a07607fcc16eb98` | PASS |
| [camera](devices/cores3/arduino-docs/camera/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/camera) | `9ed07790339a940c71e7b312404340d05777e7c0b2a99d7430d2eb8626565ec8` | PASS |
| [display](devices/cores3/arduino-docs/display/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/display) | `44c7912db911b32efb994e43bbb4ff7164265111e6c5efcfa8867bd86e2ff4c8` | PASS |
| [imu](devices/cores3/arduino-docs/imu/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/imu) | `c56bf2ca6e37edfb2e1c1e02002a5a07d9ff0a301b8fbf9264f1819053bfefba` | PASS |
| [ltr553](devices/cores3/arduino-docs/ltr553/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/ltr553) | `1a80525f57bc3d5f5b0d89476b8e5e5cb7e1b756af0387a2f19337aa0b72fa47` | PASS |
| [mic](devices/cores3/arduino-docs/mic/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/mic) | `5de68729e19eb8d0b241bb68792e38f06cc8265fcc719f079da9c9e95e07e4e0` | PASS |
| [pic](devices/cores3/arduino-docs/pic/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/pic) | `481694418ae5015b154de8d0f9641e6660a5965935fb9b9ec82baaa0975e1f12` | PASS |
| [power](devices/cores3/arduino-docs/power/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/power) | `ac19c6fb345ee441bd9c6b25f313bf190928af5b9e74649a721cc9945ac33a2b` | PASS |
| [quickstart-bargraph](devices/cores3/arduino-docs/quickstart-bargraph/firmware.json) | [4. Program Compile & Upload](https://docs.m5stack.com/en/arduino/m5cores3/program) | `5812186674963e0ba558e0ee8f65941cce49a7ab1b05a23c2b12046a5c7a1e58` | PASS |
| [rtc](devices/cores3/arduino-docs/rtc/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/rtc) | `70e500981e57fef904e8c941865fffee9d2dcf0045fa4a9255acaa516f0d68c0` | PASS |
| [sdcard](devices/cores3/arduino-docs/sdcard/firmware.json) | [Build Requirements](https://docs.m5stack.com/en/arduino/m5cores3/sdcard) | `2a73d631e383ab932e38d2db43d100ca05fbb5023944cb184194fe1046d56f1b` | PASS |
| [speaker](devices/cores3/arduino-docs/speaker/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/speaker) | `980f87ff390738e8b50c8819bf4050b8dc18c7d8a6edbe6936ea9ec70f245c78` | PASS |
| [touch](devices/cores3/arduino-docs/touch/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/touch) | `a3b1a46231ade128d44c0f1394febcadcb236d86d68ab67008fc19ab4be48c27` | PASS |
| [unit_relay](devices/cores3/arduino-docs/unit_relay/firmware.json) | [2. Example](https://docs.m5stack.com/en/arduino/projects/unit/unit_relay) | `a645e2a79947a01227c69ee565545838722ef0ebd7805d15705039bddcef84ca` | PASS · QEMU-WASM 候选 |
| [unit_ultrasonic_i2c](devices/cores3/arduino-docs/unit_ultrasonic_i2c/firmware.json) | [2. Example](https://docs.m5stack.com/en/arduino/projects/unit/unit_ultrasonic_i2c) | `0bb888aeb6fdde9200b7f81ab15ea314d75303ed6917aea58ff2a507c195e221` | PASS · QEMU-WASM 候选 |
| [wakeup](devices/cores3/arduino-docs/wakeup/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/wakeup) | `758e03ff538f8b09780995ba1e5bbfde19d8ba2f82d4fc37c13a3ff9c3734999` | PASS |
| [wav](devices/cores3/arduino-docs/wav/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/wav) | `e8f36a06ed036feec277119ecebb10e2443772a9c910323457ee2ea63c2a8d95` | PASS |
| [v2.5.3](devices/cores3/uiflow2/v2.5.3/firmware.json) | [1. UiFlow2 Firmware Flashing](https://docs.m5stack.com/en/uiflow2/m5cores3/program) | `1324f12d3ddd5392f47a9eff537f9ce0b01a8e6e36d829b6bd22cdf82d2c1264` | PASS |

## Cardputer-Adv

| 用例 | 文档 | 合并镜像 SHA-256 | 验证结果 |
| --- | --- | --- | --- |
| [battery](devices/cardputer-adv/arduino-docs/battery/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/battery) | `ddcc555063a83c065b64abfcc720966d4f2117e44eb8a5403ec6a255530494e9` | PASS |
| [button](devices/cardputer-adv/arduino-docs/button/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/button) | `00bd60f455a211d68b397e7008ff3c212acb420a0869e2deea5ee6d8ee601e2e` | PASS |
| [display](devices/cardputer-adv/arduino-docs/display/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/display) | `67bbe34e0e965f6d0491668d45b8007f153ba80be6ab3f8ec2e31e0223ee7972` | PASS |
| [imu](devices/cardputer-adv/arduino-docs/imu/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/imu) | `33639787c387413681e3b2837e115c727d53b7d58fe7fd3796f2e61b8afdc1a4` | PASS |
| [ir_nec](devices/cardputer-adv/arduino-docs/ir_nec/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/ir_nec) | `303e9c9c65c38332db862737b4ba78d8e0c35176c1dfd0f8e447c8dc40c39ae4` | PASS |
| [keyboard](devices/cardputer-adv/arduino-docs/keyboard/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/keyboard) | `ff5b8b152ba201852ded8684ca517bdf63ea155961ff99e9ff19fad11f4fcf78` | PASS |
| [mic](devices/cardputer-adv/arduino-docs/mic/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/mic) | `9a1e1a36dd4499d992254824e85a956229874c0a279146c43e599657d829bcd5` | PASS |
| [quickstart-display](devices/cardputer-adv/arduino-docs/quickstart-display/firmware.json) | [4. Program Compile & Upload](https://docs.m5stack.com/en/arduino/m5cardputer/program) | `db48605950abbd5423b62d36712dff9ccbe7db9f4997b9a88a6cf8c87111899e` | PASS |
| [sdcard](devices/cardputer-adv/arduino-docs/sdcard/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/sdcard) | `33ee9fc3513bc4e46cc5231b460ddca3e0966fbaa5874ceb8ec506a67fab8aaf` | PASS |
| [speaker](devices/cardputer-adv/arduino-docs/speaker/firmware.json) | [Beep Example](https://docs.m5stack.com/en/arduino/m5cardputer/speaker) | `1a40ab54d783e5850d09bcfa10e182f8ac43662bf4fa059b75810815f4bf93c1` | PASS |
| [speaker-sd-wav](devices/cardputer-adv/arduino-docs/speaker-sd-wav/firmware.json) | [Play WAV File Example](https://docs.m5stack.com/en/arduino/m5cardputer/speaker) | `b0b39a6f07f6c03027642c5876877d2a2e5546a86db92c8607ba93f75e8e1241` | PASS |

## English

This archive maps 28 Arduino cases and one official UIFlow2 release to their documentation, exact firmware hashes, flash layouts and validation inputs. The pinned esp32sim replay passed 15 CoreS3 and 11 Cardputer-Adv Arduino cases. UIFlow2 v2.5.3 passed boot, USB CDC REPL and framebuffer checks. Two CoreS3 Unit cases passed on QEMU-WASM candidate profiles. All results are virtual-device validation. Hardware validation was NOT RUN.

The public LFS checkout contains the rebuilt BIN and ELF files for all 26 native esp32sim cases. The original hashes and validation remain in each case manifest. See the [rebuild recipe](build-records/rebuild-2026-11-26.md) and [virtual-device validation summary](evidence/arduino-path-clean-summary.json) for reproduction, path scan results, and the original versus rebuilt capture comparison. The UIFlow2 fetch tool verifies the official image from M5Burner.

## Cardputer K132 · 2026-11-27

Three path-clean rebuilds passed native Device Service virtual-device validation. [Rebuild and validation](build-records/rebuild-2026-11-27-cardputer.md)

| Case | Docs | Merged SHA-256 |
| --- | --- | --- |
| [display](devices/cardputer/arduino-docs/display/firmware.json) | [Arduino docs](https://docs.m5stack.com/en/arduino/m5cardputer/display) | `bace5348fc88939903b37b2327a7b0bf539d4788e720646a000503dad910e8a5` |
| [keyboard](devices/cardputer/arduino-docs/keyboard/firmware.json) | [Arduino docs](https://docs.m5stack.com/en/arduino/m5cardputer/keyboard) | `0744303cf5e6ee282f8864e1560b1667abc938f4616d08a5c8e224bda17148ed` |
| [sdcard](devices/cardputer/arduino-docs/sdcard/firmware.json) | [Arduino docs](https://docs.m5stack.com/en/arduino/m5cardputer/sdcard) | `3cc2ad3fb85130e82f0f759f688b2c7aadd2a64e1c189eef556006d32c41b63e` |

# M5Stack 固件索引与模拟器复测

记录 CoreS3（K128）和 Cardputer-Adv（K132-Adv）的文档例程版本、原始镜像 SHA-256、Flash 布局和复测脚本。2026-11-26，CoreS3 15 个例程和 Cardputer-Adv 11 个例程在固定版本的 esp32sim 上全部通过。官方 CoreS3 UIFlow2 v2.5.3 通过启动、USB CDC REPL 和 LCD 绘制检查。两项 CoreS3 Unit 用例使用 2026-11-25 的 QEMU-WASM 候选配置。所有通过项均为 **virtual-device validation**。真机验证为 **NOT RUN**。

[机器可读索引](index.json)收录完整 SHA-256。各用例清单包含文档小节、源码版本、工具链、实际输入、镜像分段和输出摘要。[来源与许可](NOTICE)记录依赖和固件出处。

## 获取与校验

Git LFS 用于压缩 SD 镜像。Arduino 原始 merged BIN、应用 BIN 和 ELF 内嵌本机构建路径。Arduino 整套原始产物保存在本地忽略目录 `.local/firmware/`，公开索引记录每个文件的原始 SHA-256。UIFlow2 由脚本从 M5Burner 下载并校验。公开下载者需要另行取得原始 Arduino 构建导出才能逐字节复测。

```bash
GIT_LFS_SKIP_SMUDGE=1 git clone https://github.com/m5agent/m5stack-firmware-archive.git
cd m5stack-firmware-archive
git lfs install --local
git lfs pull
python3 tools/verify.py
python3 tools/import_arduino.py "$PREBUILT"
python3 tools/fetch_uiflow2.py
python3 tools/verify.py --with-local
```

需要 Git LFS 和 Python 3.11+。将 `PREBUILT` 设为已有的原始固件导出目录，其中 `cores3/` 和 `cardputer-adv/` 下各有用例子目录。导入脚本先校验所有原件，再复制到本地。LTR553 的依赖可从其清单中列出的原始链接下载。自行重编译产生的镜像应以新 SHA-256 记录和复测。

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

| 用例 | 文档 | 原始合并镜像 SHA-256 | 验证结果 |
| --- | --- | --- | --- |
| [button](devices/cores3/arduino-docs/button/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/button) | `3dfbaaaa1ea168074d284b40b14fc489821162fb211f568a6a52181e54dab69c` | PASS |
| [camera](devices/cores3/arduino-docs/camera/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/camera) | `5d61a3dbefec7bde7a8b7d29040f7e96e6374b5efd4fb702be9d3c5bbdfedb91` | PASS |
| [display](devices/cores3/arduino-docs/display/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/display) | `a88182dc789c1dfe7525ad7ff9a284fd201b6c55375502c895b689cd56af13d6` | PASS |
| [imu](devices/cores3/arduino-docs/imu/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/imu) | `3835dc5c8211736869c0f49762855875b09038d4aba8000516bfad9b014655ed` | PASS |
| [ltr553](devices/cores3/arduino-docs/ltr553/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/ltr553) | `9584d3339e863ee71b8e46ea43048bdc041028be3a0b0fb9e624185fdca40001` | PASS |
| [mic](devices/cores3/arduino-docs/mic/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/mic) | `ced692208d6b9b778aba1624404f461cbfad1709cee82238d2bcd7abc368a1ae` | PASS |
| [pic](devices/cores3/arduino-docs/pic/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/pic) | `f19a304c51f4f88c14c6b8157c06012357548cfd4b691ccef0d7babe7ccbbbe2` | PASS |
| [power](devices/cores3/arduino-docs/power/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/power) | `7beb59232d1b21d7fb38d4c994582609df364ba918a4b443bf8fdcbd0982112c` | PASS |
| [quickstart-bargraph](devices/cores3/arduino-docs/quickstart-bargraph/firmware.json) | [4. Program Compile & Upload](https://docs.m5stack.com/en/arduino/m5cores3/program) | `7635ebdc5bbddd6d33dc9bbeeb4d1fd9270c4971b0692595096c24b72f648d8a` | PASS |
| [rtc](devices/cores3/arduino-docs/rtc/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/rtc) | `cf450835c9b8d88f815272158040afee79f0d88cf32b1f46add9bd783b8ab00d` | PASS |
| [sdcard](devices/cores3/arduino-docs/sdcard/firmware.json) | [Build Requirements](https://docs.m5stack.com/en/arduino/m5cores3/sdcard) | `c97c3efde1aab234c4041d118b13eb9d4b506fd2158181e495ab4e8c8f9190b0` | PASS |
| [speaker](devices/cores3/arduino-docs/speaker/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/speaker) | `035d6b0e085b03024f8a10daa46595874113e5e7aa65bb1977526e03bbff1d5c` | PASS |
| [touch](devices/cores3/arduino-docs/touch/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/touch) | `3cec5642f681dc0525c7e6cdbada7189f0ee52f9a95b68b19055571664222606` | PASS |
| [unit_relay](devices/cores3/arduino-docs/unit_relay/firmware.json) | [2. Example](https://docs.m5stack.com/en/arduino/projects/unit/unit_relay) | `a645e2a79947a01227c69ee565545838722ef0ebd7805d15705039bddcef84ca` | PASS · QEMU-WASM 候选 |
| [unit_ultrasonic_i2c](devices/cores3/arduino-docs/unit_ultrasonic_i2c/firmware.json) | [2. Example](https://docs.m5stack.com/en/arduino/projects/unit/unit_ultrasonic_i2c) | `0bb888aeb6fdde9200b7f81ab15ea314d75303ed6917aea58ff2a507c195e221` | PASS · QEMU-WASM 候选 |
| [wakeup](devices/cores3/arduino-docs/wakeup/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/wakeup) | `2ddff71f4ceca5e79b709ba823b8ecf0828f2016a85e512df779e526e7a9f295` | PASS |
| [wav](devices/cores3/arduino-docs/wav/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cores3/wav) | `22eb0ffce846a6175de97e2c2f82c02bbef092724874b317297cf5c95751bff5` | PASS |
| [v2.5.3](devices/cores3/uiflow2/v2.5.3/firmware.json) | [1. UiFlow2 Firmware Flashing](https://docs.m5stack.com/en/uiflow2/m5cores3/program) | `1324f12d3ddd5392f47a9eff537f9ce0b01a8e6e36d829b6bd22cdf82d2c1264` | PASS |

## Cardputer-Adv

| 用例 | 文档 | 原始合并镜像 SHA-256 | 验证结果 |
| --- | --- | --- | --- |
| [battery](devices/cardputer-adv/arduino-docs/battery/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/battery) | `ac93be27c88cb4ae19d08056a537b06469a1e2b7f6ef8af82c6739d2ff38f34b` | PASS |
| [button](devices/cardputer-adv/arduino-docs/button/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/button) | `07ebcecd82c9a64aad457f5e30eac140bfd24db8006506ea6f04489a860dc0e7` | PASS |
| [display](devices/cardputer-adv/arduino-docs/display/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/display) | `cc1b3e5868feb075e2ec8a5e7211897e8ae1c9635f5d072442ef007273c2a968` | PASS |
| [imu](devices/cardputer-adv/arduino-docs/imu/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/imu) | `f933c50fcc3dca52d9fa8e256ed5ab5e7e1f5d68571f47d9cab976d16ec41b10` | PASS |
| [ir_nec](devices/cardputer-adv/arduino-docs/ir_nec/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/ir_nec) | `0969326c036e7a88168dcf9dae65de20b71ee501e2ecc1074c86819d10c240fb` | PASS |
| [keyboard](devices/cardputer-adv/arduino-docs/keyboard/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/keyboard) | `f7edb36bb3662506853773399467841a55bef3d3e16cc3469bc821b4ad7638bf` | PASS |
| [mic](devices/cardputer-adv/arduino-docs/mic/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/mic) | `1076a8fbac11c2382585c77f7226648dc3753d29e8135b98502ac8462265a8a9` | PASS |
| [quickstart-display](devices/cardputer-adv/arduino-docs/quickstart-display/firmware.json) | [4. Program Compile & Upload](https://docs.m5stack.com/en/arduino/m5cardputer/program) | `0a93553358ee75cf9125b2fdf9e7671ea096be2c70bc3e4ff4945bc034835717` | PASS |
| [sdcard](devices/cardputer-adv/arduino-docs/sdcard/firmware.json) | [Compilation Requirements](https://docs.m5stack.com/en/arduino/m5cardputer/sdcard) | `0bd2b3e56bf693661e8e04b5cbe17365d7346c2e84d03ac7abd6e36b95765132` | PASS |
| [speaker](devices/cardputer-adv/arduino-docs/speaker/firmware.json) | [Beep Example](https://docs.m5stack.com/en/arduino/m5cardputer/speaker) | `1be720c9f62e086081f63ff6adf7f907e71f80dcc4816af71050cddd937300b5` | PASS |
| [speaker-sd-wav](devices/cardputer-adv/arduino-docs/speaker-sd-wav/firmware.json) | [Play WAV File Example](https://docs.m5stack.com/en/arduino/m5cardputer/speaker) | `9c2993624b739090d852d7165f49b78300827fe916011c434bc939d63e4e7f3b` | PASS |

## English

This archive maps 28 Arduino cases and one official UIFlow2 release to their documentation, exact firmware hashes, flash layouts and validation inputs. The pinned esp32sim replay passed 15 CoreS3 and 11 Cardputer-Adv Arduino cases. UIFlow2 v2.5.3 passed boot, USB CDC REPL and framebuffer checks. Two CoreS3 Unit cases passed on QEMU-WASM candidate profiles. All results are virtual-device validation. Hardware validation was NOT RUN.

Run the commands above from the archive root. Git LFS supplies the compressed SD fixture. The import tool requires the original Arduino build export. It verifies and imports the exact firmware into the ignored local directory. The original merged images, app images and ELFs embed private build paths. The public index identifies those originals by their full SHA-256. The UIFlow2 fetch tool obtains the official image from M5Burner and verifies it before saving. The two Arduino runners contain the individual commands, capture times and injected inputs. See the case manifests for output hashes and validation scope.

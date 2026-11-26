#!/usr/bin/env python3
"""Check run-cores3-docs.sh artifacts for the pinned prebuilt CoreS3 sketches."""

import argparse
from array import array
from collections import Counter
from hashlib import sha256
import math
from pathlib import Path
import re
import struct
import sys
import wave
import zlib


def require(condition, reason):
    if not condition:
        raise ValueError(reason)


def png_rgb(blob, scale=1):
    # The simulator and these media fixtures write filter-0 RGB8 images.
    require(blob.startswith(b"\x89PNG\r\n\x1a\n"), "invalid PNG signature")
    at = 8
    compressed = bytearray()
    header = None
    ended = False
    while at < len(blob):
        length, kind = struct.unpack_from(">I4s", blob, at)
        body = blob[at + 8:at + 8 + length]
        crc, = struct.unpack_from(">I", blob, at + 8 + length)
        require(crc == zlib.crc32(kind + body), "PNG chunk CRC mismatch")
        at += length + 12
        if kind == b"IHDR":
            header = struct.unpack(">IIBBBBB", body)
        elif kind == b"IDAT":
            compressed.extend(body)
        elif kind == b"IEND":
            ended = True
            break
    w, h = 320 * scale, 240 * scale
    require(ended and header == (w, h, 8, 2, 0, 0, 0), "unexpected PNG dimensions or format")
    stride = 1 + w * 3
    raw = zlib.decompress(compressed)
    require(len(raw) == stride * h and set(raw[::stride]) == {0}, "unexpected PNG scanlines")
    return b"".join(raw[y + 1:y + stride] for y in range(0, len(raw), stride))


def region(frame, x0, y0, x1, y1):
    return b"".join(frame[3 * (320 * y + x0):3 * (320 * y + x1)] for y in range(y0, y1))


def text(frame, box, expected, digest):
    # Fingerprints of only the indicated text/indicator regions, reviewed against this
    # build's fonts and the sketch's expected values. Coordinates are physical glass pixels.
    observed = sha256(region(frame, *box)).hexdigest()[:24]
    require(observed == digest, f"frame region {box} does not show {expected} ({observed})")


def color_counts(frame):
    return Counter(zip(frame[0::3], frame[1::3], frame[2::3]))


def changed(a, b):
    return sum(a[i:i + 3] != b[i:i + 3] for i in range(0, len(a), 3))


def bmp_mirrored_rgb565(blob):
    start, = struct.unpack_from("<I", blob, 10)
    w, h, planes, bits, compression = struct.unpack_from("<iiHHI", blob, 18)
    require(blob[:2] == b"BM" and (w, h, planes, bits, compression) == (320, 240, 1, 24, 0)
            and len(blob) == start + w * h * 3, "expected 320x240 RGB24 camera BMP")
    rgb = bytearray()
    for i in range(len(blob) - 3, start - 1, -3):
        b, g, r = blob[i:i + 3]
        rgb.extend((r // 8 * 255 // 31, g // 4 * 255 // 63, b // 8 * 255 // 31))
    return rgb


def fat32_files(path, names):
    """Read requested root files from the runner's MBR/FAT32 SD fixture."""
    with path.open("rb") as disk:
        def read_at(offset, size):
            disk.seek(offset)
            data = disk.read(size)
            require(len(data) == size, "truncated SD image")
            return data

        mbr = read_at(0, 512)
        require(mbr[510:] == b"\x55\xaa" and mbr[450] in (0x0b, 0x0c), "expected MBR/FAT32")
        base, = struct.unpack_from("<I", mbr, 454)
        boot = read_at(base * 512, 512)
        sector, spc, reserved, fats = struct.unpack_from("<HBHB", boot, 11)
        total, fat_size = struct.unpack_from("<II", boot, 32)
        root, = struct.unpack_from("<I", boot, 44)
        require(boot[510:] == b"\x55\xaa" and sector == 512 and spc > 0 and fats > 0
                and total > reserved + fats * fat_size and fat_size > 0, "invalid FAT32 BPB")
        first_data = base + reserved + fats * fat_size
        cluster_count = (total - reserved - fats * fat_size) // spc

        def clusters(cluster):
            visited = set()
            while cluster < 0x0ffffff8:
                require(2 <= cluster < cluster_count + 2 and cluster < fat_size * sector // 4
                        and cluster not in visited, "invalid FAT32 cluster chain")
                visited.add(cluster)
                yield read_at((first_data + (cluster - 2) * spc) * sector, spc * sector)
                entry, = struct.unpack("<I", read_at((base + reserved) * sector + cluster * 4, 4))
                cluster = entry & 0x0fffffff

        files = {}
        long_name = []
        for block in clusters(root):
            for i in range(0, len(block), 32):
                e = block[i:i + 32]
                if not e[0]:
                    break
                if e[0] == 0xe5:
                    long_name.clear()
                elif e[11] == 0x0f:
                    if e[0] & 0x40:
                        long_name.clear()
                    long_name.insert(0, e[1:11] + e[14:26] + e[28:32])
                else:
                    if not e[11] & 0x18:
                        short = e[:8].rstrip().decode("ascii") + "." + e[8:11].rstrip().decode("ascii")
                        name = b"".join(long_name).decode("utf-16-le").split("\0")[0].rstrip("\uffff") if long_name else short
                        if name in names:
                            high, = struct.unpack_from("<H", e, 20)
                            low, size = struct.unpack_from("<HI", e, 26)
                            payload = bytearray()
                            for chunk in clusters(high << 16 | low):
                                payload.extend(chunk)
                                if len(payload) >= size:
                                    break
                            require(len(payload) >= size, f"truncated FAT32 file {name}")
                            files[name] = bytes(payload[:size])
                    long_name.clear()
            else:
                continue
            break
        require(files.keys() == set(names), f"FAT32 files missing: {set(names) - files.keys()}")
        return files


def rms(samples):
    return math.sqrt(sum(x * x for x in samples) / len(samples)) if samples else 0


def frequency(samples, rate):
    rises = [i for i in range(1, len(samples)) if samples[i - 1] <= 0 < samples[i]]
    return (len(rises) - 1) * rate / (rises[-1] - rises[0]) if len(rises) > 1 else 0


def audio(path):
    with wave.open(str(path), "rb") as wav:
        rate = wav.getframerate()
        require((wav.getnchannels(), wav.getsampwidth(), rate) == (1, 2, 48000),
                "expected mono 48 kHz signed PCM16")
        pcm = array("h", wav.readframes(wav.getnframes()))
        require(len(pcm) == wav.getnframes(), "truncated WAV")
    if sys.byteorder != "little":
        pcm.byteswap()
    step = rate // 1000
    runs = []
    start = None
    for i in range(0, len(pcm) + 1, step):
        active = rms(pcm[i:i + step]) >= 100 and i + step <= len(pcm)
        if active and start is None:
            start = i
        elif not active and start is not None:
            if i - start >= 5 * step:
                runs.append((start / rate, (i - start) / rate, rms(pcm[start:i]), frequency(pcm[start:i], rate)))
            start = None
    if start is not None and len(pcm) - start >= 5 * step:
        runs.append((start / rate, (len(pcm) - start) / rate, rms(pcm[start:]), frequency(pcm[start:], rate)))
    return len(pcm), runs


SECONDS = {
    "display": (6, 4), "quickstart-bargraph": (6, 4), "touch": (3.6, 3.2), "button": (5, 2.5),
    "power": (10, 6), "imu": (5, 9), "ltr553": (10, 5), "rtc": (10, 8), "speaker": (6,),
    "mic": (9, 4), "camera": (6, 6), "wakeup": (12, 8), "sdcard": (4.5, 4), "pic": (4.4, 3.4),
    "wav": (14,),
}


def capture(out, label, seconds, asleep=False):
    require((out / f"{label}.status").read_text().strip() == "0", f"{label}: emulator exited nonzero")
    report = (out / f"{label}.err").read_text()
    stop = re.search(r"\[emu\] stop: Halted .*?emulated (\d+\.\d+)s", report)
    require(stop and float(stop[1]) == seconds, f"{label}: run did not reach {seconds}s")
    panel = re.search(r"cores3: soft-i2c (\d+) transactions; (\d+) RAMWR, (\d+) pixels, "
                      r"madctl=(0x[\da-f]+) .*?on=(true|false) sleeping=(true|false)", report)
    expected_state = ("false", "true") if asleep else ("true", "false")
    require(panel and int(panel[1]) > 0 and panel.groups()[-2:] == expected_state
            and (asleep or min(int(panel[2]), int(panel[3])) > 0), f"{label}: unexpected LCD/I2C report")
    expected_madctl = 0 if asleep else 0x6c if label.split("-")[0] in ("speaker", "mic") else 8
    require(int(panel[4], 16) == expected_madctl, f"{label}: unexpected MADCTL {panel[4]}")
    resets = report.count("[emu] chip reset at")
    require(resets == (1 if label == "wakeup" else 0), f"{label}: unexpected reset count {resets}")
    frame = png_rgb((out / f"{label}.png").read_bytes())
    return frame, report


def verify(out, case):
    frame, report = capture(out, case, SECONDS[case][0])
    if len(SECONDS[case]) == 2:
        before, before_report = capture(out, case + "-before", SECONDS[case][1], asleep=case == "wakeup")
        require(changed(frame, before) > 0, "both captures are identical")
    if case == "display":
        for screen in (before, frame):
            colors = color_counts(screen)
            # This sketch draws circles and rectangles; setup configures a font size but renders no text.
            require(len(colors) > 100 and colors.most_common(1)[0][1] <= 44 * 44, "missing multicolored shapes")
        require(changed(frame, before) > 20_000, "insufficient drawing changes")
        return "multicolored circles and rectangles change"
    if case == "quickstart-bargraph":
        for screen in (before, frame):
            bottom = color_counts(region(screen, 0, 239, 320, 240))
            require(bottom[(0, 0, 0)] == 64 and len(bottom) > 55, "missing bars or one-pixel column gutters")
            require(color_counts(screen)[(255, 255, 255)] > 350, "missing peak markers")
        require(changed(frame, before) > 2500, "insufficient bar animation")
        return "colored columns and white peaks animate"
    if case == "button":
        for screen in (before, frame):
            text(screen, (25, 0, 295, 30), "Touch Button Test in red", "c4a38b120d47ced5d52ee13c")
        require(not any(region(before, 85, 75, 235, 108)), "Btn PWR present before key press")
        text(frame, (85, 75, 235, 108), "Btn PWR in red", "41f349b8c87d6799dcadc65e")
        return "250 ms key press displays Btn PWR"
    if case == "touch":
        text(before, (0, 70, 320, 105), "flick", "72177605486541263641eac9")
        text(frame, (0, 70, 320, 105), "none after release", "5fcf5edfb604f4b013c4106c")
        for screen in (before, frame):
            text(screen, (65, 180, 260, 215), "X:100 / Y:120", "f539d8e80d4187463654a36c")
        require(region(before, 100, 120, 101, 121) == b"\x00\xff\x00", "missing point at (100,120)")
        return "flick/release, coordinates and green point"
    if case == "power":
        lines = [
            ("Bat Charging", 1, 0, "f83e71bb2ade05b54cb74d90", "d7a1fc59369cf9ece5b018ec"),
            ("Bat Voltage (mV)", 4123, 3678, "93d433c5af778794b4162220", "f7ae615f67944bd91195cf68"),
            ("Bat Level", 87, 42, "bb3d061653d6a42cb33e8ddc", "9fa0d54e0c4a58da2ca17251"),
            ("VBus Voltage (mV)", 5000, 0, "ce2226aaf0b274dfd02ccadb", "4ceee824322d265b87dda6c5"),
        ]
        for y, (label, plugged, unplugged, on_digest, off_digest) in zip(range(30, 110, 20), lines):
            box = (10, y, 260, y + 20)
            text(before, box, f"{label} {plugged}", on_digest)
            text(frame, box, f"{label} {unplugged}", off_digest)
        return "battery 4123→3678 mV, 87→42%, VBUS 5000→0 mV"
    if case == "imu":
        text(frame, (0, 20, 170, 198), "first accelerometer, gyro and magnetometer vector", "99ee1de8968435d2c82d0617")
        text(before, (0, 20, 170, 198), "second accelerometer, gyro and magnetometer vector", "2d18f7a0bc9baa5f29142146")
        return "two injected vectors on LCD (HWCDC uninitialized by sketch)"
    if case == "ltr553":
        text(before, (25, 106, 290, 133), "PS:321 / ALS:260", "d81b684af029a96628ec684a")
        text(frame, (25, 106, 290, 133), "PS:1456 / ALS:1440", "96a270409805a3f5c5097c32")
        return "PS 321→1456, ALS 260→1440 (HWCDC uninitialized by sketch)"
    if case == "rtc":
        text(before, (0, 0, 320, 145), "RTC and ESP32 UTC 16:00:07, local 00:00:07", "4594798bb982eaf5ea46bcdd")
        text(frame, (0, 0, 320, 145), "RTC and ESP32 UTC 16:00:09, local 00:00:09", "1553aa222a55e8a50f6c93cc")
        for log in (before_report, report):
            require("state Associated" in log and re.search(r"\b[1-9]\d* NTP answers", log), "no Wi-Fi/NTP exchange")
        return "associated AP, NTP reply, RTC/system/local time advance"
    if case == "camera":
        for label, screen, log in ((case, frame, report), (case + "-before", before, before_report)):
            counters = re.search(r"camera: (\d+) VSYNCs, (\d+) frames with dropped data", log)
            require(counters and int(counters[1]) >= 40 and int(counters[2]) == 0, "camera data dropped or missing")
            require(screen == bmp_mirrored_rgb565((out / f"{label}-input.bmp").read_bytes()),
                    f"{label}: LCD differs from expected SCCB h-mirrored RGB565 capture")
        return "two asymmetric mirrored RGB565 fixtures, zero dropped frames"
    if case == "wakeup":
        require(not any(before) and "powered=false backlight_mv=0" in before_report, "sleep capture is powered")
        power = re.search(r"off_cause=0x02 off_cycle=Some\((\d+)\) on_cause=0x02 on_cycle=Some\((\d+)\).*?"
                          r"timer_expiry_cycle=Some\((\d+)\)", report)
        require(power and abs(int(power[3]) - int(power[1]) - 5 * 240_000_000) < 12_000_000
                and int(power[2]) - int(power[3]) == 3_840_000, "missing five-second RTC power cycle")
        text(frame, (103, 45, 219, 199), "restored Touch to sleep / After 5s Wakeup", "36e46dbb2e43157fe07853e1")
        return "black asleep, RTC expiration, PMIC restart and restored UI"
    if case in ("sdcard", "pic"):
        stem = "cores3_test_picture" if case == "sdcard" else "TestPicture"
        first, second = f"{stem}01.png", f"{stem}02.png"
        names = ("WriteTest.txt", first, second)
        for label, screen, log, png in ((case, frame, report, second),
                                        (case + "-before", before, before_report, first)):
            files = fat32_files(out / f"sd-{label}.img", names)
            require(files["WriteTest.txt"] == b"Hello, world! \nSD card write success! \n",
                    f"{label}: incorrect WriteTest.txt contents")
            require(screen == png_rgb(files[png]), f"{label}: LCD does not match {png}")
            require(re.search(r"\[sd-spi\] \d+ commands, [1-9]\d* blocks read, [1-9]\d* blocks written; ALDO4 on",
                              log), f"{label}: missing SD data path")
        return "FAT32 WriteTest.txt matches and both PNGs render byte-for-byte"

    samples, tones = audio(out / f"{case}.wav")
    counters = re.search(r"i2s frames out: \d+ \(i2s0 @ \d+ Hz\) (\d+) \(i2s1 @ 48000 Hz\)", report)
    require(counters and int(counters[1]) == samples, "WAV sample count differs from I2S1 output")
    if case == "speaker":
        text(frame, (140, 25, 175, 215), "vertical green Speaker Test", "01b8244a0b5987a287f3e569")
        require(len(tones) == 6, f"expected six alternating tone bursts, got {len(tones)}")
        for i, (start, duration, level, hz) in enumerate(tones):
            expected_hz, expected_duration = (4000, .020) if i % 2 else (10000, .100)
            require(abs(hz - expected_hz) < 30 and abs(duration - expected_duration) < .008
                    and abs(start - i) < .080 and 600 < level < 800, f"invalid speaker tone {i + 1}: {tones[i]}")
        return "vertical title and 3× each of 10 kHz / 4 kHz"
    if case == "mic":
        text(before, (286, 118, 317, 220), "recording REC and red dot", "77ce6d390d17a9c13ab0bfbc")
        text(frame, (286, 118, 317, 220), "PLAY and blue triangle", "9d78dc091536eb82961d55ed")
        trace = region(before, 110, 0, 210, 240)
        require(color_counts(trace)[(255, 255, 255)] > 1500, "recording waveform missing/flat")
        require(len(tones) == 1, "expected one microphone playback segment")
        _, duration, level, hz = tones[0]
        require(2.5 < duration < 2.9 and abs(hz - 733) < 2 and 800 < level < 1100,
                f"incorrect microphone playback: {tones[0]}")
        return f"REC waveform → PLAY, {hz:.2f} Hz, RMS {level:.1f}"

    serial = (out / "wav.log").read_text(errors="replace")
    for marker in ("SD Card Size: 64MB", "WAV: 16000Hz, 1ch, 16bit",
                   "Starting segmented playback...", "Segmented playback completed!"):
        require(marker in serial, f"missing USB console marker {marker}")
    require(re.search(r"\[sd-spi\] \d+ commands, [1-9]\d* blocks read, 0 blocks written; ALDO4 on", report),
            "missing SD reads or unexpected writes")
    require(len(tones) == 24 and 11.9 <= sum(t[1] for t in tones) <= 12.2
            and all(160 < t[2] < 200 for t in tones), "incomplete or distorted SD playback")
    require(abs(tones[0][3] - 440) < 2 and abs(tones[2][3] - 660) < 2, "wrong SD fixture frequencies")
    require(color_counts(frame)[(0, 0, 0)] > 3000 and color_counts(frame)[(255, 255, 255)] > 40_000,
            "missing LCD playback report")
    return "SD reads, 12 s of 440/660 Hz audio and USB completion marker"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("outdir", type=Path)
    args = parser.parse_args()
    failures = 0
    print("| Case | Verdict | Evidence |")
    print("| --- | --- | --- |")
    for case in SECONDS:
        try:
            details = verify(args.outdir, case)
            verdict = "PASS"
        except (OSError, ValueError, struct.error, wave.Error, zlib.error, EOFError) as error:
            verdict, details = "FAIL", str(error)
            failures += 1
        print(f"| {case} | {verdict} | {details.replace('|', '/')} |")
    print(f"\n{len(SECONDS) - failures}/{len(SECONDS)} passed")
    return bool(failures)


if __name__ == "__main__":
    sys.exit(main())

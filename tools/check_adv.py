#!/usr/bin/env python3
"""Check the 11 prebuilt Cardputer-ADV documentation examples."""
import argparse
from array import array
from collections import Counter
from hashlib import sha256
import io
from pathlib import Path
import re
import struct
import sys
import wave
import zlib

from check_cores3 import audio, fat32_files, frequency, require, rms

WIDTH, HEIGHT = 240, 135
SECONDS = {
    "display": 8, "quickstart-display": 8, "keyboard": 8, "button": 8, "ir_nec": 8,
    "battery": 6, "imu": 6, "speaker": 8, "mic": 10, "sdcard": 8, "speaker-sd-wav": 13.5,
}

def frame_rgb(path):
    blob = path.read_bytes()
    require(blob[:8] == b"\x89PNG\r\n\x1a\n", "invalid PNG")
    chunks = {}
    at = 8
    while at < len(blob):
        size, kind = struct.unpack_from(">I4s", blob, at)
        data = blob[at + 8:at + 8 + size]
        crc, = struct.unpack_from(">I", blob, at + 8 + size)
        require(crc == zlib.crc32(kind + data), "PNG CRC mismatch")
        chunks.setdefault(kind, []).append(data)
        at += size + 12
    require(chunks.get(b"IEND") == [b""] and chunks.get(b"IHDR") == [
        struct.pack(">IIBBBBB", WIDTH, HEIGHT, 8, 2, 0, 0, 0)
    ], "expected a 240×135 RGB8 frame")
    raw = zlib.decompress(b"".join(chunks.get(b"IDAT", [])))
    stride = WIDTH * 3 + 1
    require(len(raw) == stride * HEIGHT and set(raw[::stride]) == {0}, "invalid PNG rows")
    return b"".join(raw[at + 1:at + stride] for at in range(0, len(raw), stride))

def colors(rgb):
    return Counter(zip(rgb[0::3], rgb[1::3], rgb[2::3]))

def region(frame, box):
    x0, y0, x1, y1 = box
    return b"".join(frame[(WIDTH * y + x0) * 3:(WIDTH * y + x1) * 3] for y in range(y0, y1))

def text(frame, box, digest, meaning):
    # Only the reviewed text region is pinned, in fixed physical glass coordinates.
    require(sha256(region(frame, box)).hexdigest()[:24] == digest, f"missing LCD text {meaning}")

def capture(out, label, seconds, madctl=0x6c):
    require((out / f"{label}.status").read_text().strip() == "0", f"{label}: nonzero simulator exit")
    report = (out / f"{label}.err").read_text(errors="replace")
    serial = (out / f"{label}.log").read_text(errors="replace")
    stop = re.search(r"stop: Halted .*?emulated (\d+\.\d+)s", report)
    require(stop and float(stop[1]) == seconds, f"{label}: missed virtual deadline")
    require("chip reset at" not in report and not re.search(r"panic'ed|Unsupported board type", serial),
            f"{label}: reset, panic or wrong board")
    panel = re.search(r"cardputer-adv: (\d+) GPIO LCD reads, (\d+) SPI3 LCD bytes, (\d+) RAMWR, "
                      r"(\d+) pixels, MADCTL=(0x[\da-f]+), backlight=true; (\d+) GPIO I2C ACKs, "
                      r"(\d+) keyboard events read", report)
    require(panel and min(map(int, panel.groups()[:4])) > 0 and int(panel[6]) > 0
            and int(panel[5], 16) == madctl, f"{label}: missing LCD or ADV TCA8418 probe")
    return frame_rgb(out / f"{label}.png"), report, serial, int(panel[7])

def pcm(source):
    with wave.open(source, "rb") as wav:
        rate = wav.getframerate()
        require((wav.getnchannels(), wav.getsampwidth()) == (1, 2), "expected mono PCM16")
        samples = array("h", wav.readframes(wav.getnframes()))
        require(len(samples) == wav.getnframes(), "truncated PCM")
    if sys.byteorder == "big":
        samples.byteswap()
    return rate, samples

def i2s1(report, samples):
    require(f"{samples} (i2s1 @ 48000 Hz)" in report, "captured WAV does not match I2S1 samples")

def verify(out, case):
    madctl = 0xdc if case in {"keyboard", "ir_nec", "mic", "sdcard"} else 0x6c
    frame, report, serial, keys = capture(out, case, SECONDS[case], madctl)
    if case in {"display", "quickstart-display"}:
        before, *_ = capture(out, case + "-before", 3)
        require(min(len(colors(before)), len(colors(frame))) > 80, "missing multicolored shapes")
        require(sum(a != b for a, b in zip(before, frame)) > 32_400, "shapes did not animate")
        return "colored circles and rectangles change from 3 to 8 s"
    if case == "keyboard":
        before, _, _, idle_keys = capture(out, "keyboard-before", 1.8, 0xdc)
        chord, _, _, chord_keys = capture(out, "keyboard-chord", 5, 0xdc)
        require((idle_keys, keys, chord_keys) == (0, 14, 16), "missing TCA8418 press/release events")
        box = (170, 0, 196, 40)
        text(before, box, "3f121ab8ee6ca8c0d4926044", "empty canvas")
        text(frame, box, "852935b2f64953f21eeaf361", "a bc, bare Backspace behavior")
        text(chord, box, "0e280735426d7badd9031833", "a c after Fn+Del and Enter")
        return "space, Backspace and Enter through FIFO; Fn+Del deletes"
    if case == "button":
        events = re.findall(r"Button A (Pressed|Released)", serial)
        require(events == ["Pressed", "Released"] * 3, "missing G0 press/release pairs")
        return "three press/release pairs via numeric pin, btn and g0"
    if case == "ir_nec":
        before, _, _, _ = capture(out, "ir_nec-before", 3, 0xdc)
        box = (64, 0, 98, 135)
        text(before, box, "134791a69bf381914755871d", "command 0x36")
        text(frame, box, "2f4d71620fdd362c436524b2", "command 0x3a")
        voltage_frames = re.findall(r"address=0x([\da-f]+)/command=0x([\da-f]+)", report)
        expected = [("1111", f"{cmd:02x}") for cmd in range(0x34, 0x3b)]
        require(voltage_frames == expected, "GPIO44 NEC payload mismatch")
        require(re.findall(r"address=0x([\da-f]+), command=0x([\da-f]+)", serial.lower()) == expected,
                "serial and GPIO44 decode disagree")
        return "serial and waveform agree for 0x1111/0x34..0x3a"
    if case == "battery":
        before, *_ = capture(out, "battery-before", 3)
        box = (10, 48, 230, 91)
        text(before, box, "6e98f3de158d87b262a5528a", "3700 mV/50%")
        text(frame, box, "bdd867131e4f42fa076776ec", "4000 mV/87%")
        return "3700 mV / 50% to 4000 mV / 87% from ADC"
    if case == "imu":
        before, *_ = capture(out, "imu-before", 3)
        box = (0, 28, 239, 109)
        text(before, box, "df7c2d129e8bea6a4bd1a86f", "first injected vector")
        text(frame, box, "c3ae57e4110b94592be5fa60", "second injected vector")
        return "two injected accel/gyro vectors displayed"
    if case == "speaker":
        samples, tones = audio(out / "speaker.wav")
        i2s1(report, samples)
        require(len(tones) == 8, f"expected 8 tone bursts, got {len(tones)}")
        for i, (_, duration, level, hz) in enumerate(tones):
            expected_hz, expected_duration = (4000, .02) if i % 2 else (7000, .10)
            require(abs(hz - expected_hz) < 30 and abs(duration - expected_duration) < .012
                    and level > 500, f"bad tone burst {i + 1}: {tones[i]}")
        return "four 7000 Hz and four 4000 Hz bursts on I2S1"
    if case == "mic":
        rec, *_ = capture(out, "mic-rec", 3.9, 0xdc)
        silence, *_ = capture(out, "mic-silence", 3.9, 0xdc)
        play, *_ = capture(out, "mic-play", 5.5, 0xdc)
        require(colors(rec)[(255, 0, 0)] > 100 and colors(frame)[(255, 0, 0)] > 100
                and colors(play)[(24, 145, 255)] > 50, "missing REC / PLAY / REC display states")
        box = (75, 0, 165, HEIGHT)
        require(colors(region(rec, box))[(255, 255, 255)] > 2 * colors(region(silence, box))[(255, 255, 255)],
                "missing REC waveform response")
        rate, samples = pcm(str(out / "mic.wav"))
        i2s1(report, len(samples))
        section = samples[rate:rate * 3]
        hz = frequency(section, rate)
        require(rate == 48000 and 3.5 < len(samples) / rate < 3.8 and abs(hz - 733) < 5 and rms(section) > 500,
                f"unexpected mic playback {hz:.1f} Hz")
        return f"REC reacts to input, G0 replays {hz:.1f} Hz then REC resumes"
    if case == "sdcard":
        capture(out, "sdcard-first", 12, 0xdc)
        for marker in ("SDHC", "Dir created", "Dir removed", "File written", "Message appended",
                       "Hello World!", "File deleted", "File renamed", "1048576 bytes read", "1048576 bytes written"):
            require(marker in serial, f"missing SD serial marker {marker}")
        require(colors(frame)[(0, 255, 0)] > 500 and re.search(
            r"\[sd-spi\] \d+ commands, [1-9]\d* blocks read, [1-9]\d* blocks written", report),
            "missing displayed status or SPI SD data-plane activity")
        files = fat32_files(out / "sd-files.img", ("FOO.TXT", "TEST.TXT"))
        require(files == {"FOO.TXT": b"Hello World!\n", "TEST.TXT": bytes(1048576)}, "wrong persisted FAT32 contents")
        return "FAT32 create/append/rename/delete, 1 MiB read/write, on-disk data verified"
    require(case == "speaker-sd-wav", f"unsupported case {case}")
    require(re.search(r"\[sd-spi\] \d+ commands, [1-9]\d* blocks read, 0 blocks written", report),
            "no SD reads or unexpected writes")
    source = fat32_files(out / "sd-wav.img", ("sample-12s.wav",))["sample-12s.wav"]
    source_rate, source_pcm = pcm(io.BytesIO(source))
    rate, samples = pcm(str(out / "speaker-sd-wav.wav"))
    i2s1(report, len(samples))
    require(rate == 48000 and len(samples) >= 12 * rate and len(source_pcm) == 12 * source_rate,
            "expected at least 12 seconds of playback")
    for sec in range(12):
        original = source_pcm[(4 * sec + 1) * source_rate // 4:(4 * sec + 3) * source_rate // 4]
        output = samples[(4 * sec + 1) * rate // 4:(4 * sec + 3) * rate // 4]
        expected_hz = 660 if sec % 2 else 440
        require(abs(frequency(original, source_rate) - expected_hz) < 5
                and abs(frequency(output, rate) - expected_hz) < 5 and rms(output) > 100,
                f"wrong SD WAV audio at second {sec}")
    return "SPI SD source and I2S1 capture agree for all 12 alternating 440/660 Hz seconds"

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("outdir", type=Path)
    args = parser.parse_args()
    print("| Case | Verdict | Evidence |")
    print("| --- | --- | --- |")
    passed = 0
    for case in SECONDS:
        try:
            details = verify(args.outdir, case)
            verdict = "PASS"
            passed += 1
        except (OSError, ValueError, wave.Error, struct.error, zlib.error, EOFError) as exc:
            verdict, details = "FAIL", str(exc)
        print(f"| {case} | {verdict} | {details.replace('|', '/')} |")
    print(f"\n{passed}/{len(SECONDS)} passed")
    return int(passed != len(SECONDS))

if __name__ == "__main__":
    sys.exit(main())

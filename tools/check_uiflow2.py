#!/usr/bin/env python3
"""Check CoreS3 UIFlow2 USB CDC and framebuffer output from the pinned replay."""

import argparse
from pathlib import Path

from check_cores3 import png_rgb


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("run", type=Path)
    args = parser.parse_args()
    serial = (args.run / "uiflow.log").read_text(errors="replace")
    report = (args.run / "uiflow.err").read_text(errors="replace")
    frame = png_rgb((args.run / "uiflow-red.png").read_bytes())
    checks = {
        "UIFlow2 version": "V2.5.3" in serial,
        "CDC REPL response": ">>> print(1+1)\n2\n" in serial,
        "USB host completion": "script done" in report and "0 data PID errors" in report,
        "red framebuffer": frame == b"\xff\x00\x00" * (320 * 240),
    }
    for name, passed in checks.items():
        print(f"{name} = {'PASS' if passed else 'FAIL'}")
    return int(not all(checks.values()))


if __name__ == "__main__":
    raise SystemExit(main())

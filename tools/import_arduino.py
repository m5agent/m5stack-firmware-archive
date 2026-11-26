#!/usr/bin/env python3
"""Verify an existing prebuilt Arduino export and copy its exact files into .local."""

import argparse
from hashlib import sha256
import json
from pathlib import Path
import shutil
import sys

from verify import ROOT, check


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("firmware_dir", type=Path, help="directory containing cores3/ and cardputer-adv/")
    args = parser.parse_args()
    index = json.loads((ROOT / "index.json").read_text())
    mappings = {}
    boot_app = None
    try:
        for case in index["cases"]:
            if case["source"] != "arduino-docs":
                continue
            manifest = json.loads((ROOT / case["manifest"]).read_text())
            source = args.firmware_dir / case["device_id"] / case["case"]
            segments = manifest["flash_layout"]["segments"]
            for name, record in manifest["files"].items():
                target = ROOT / record["path"]
                if name == segments["boot_app0"]["file"]:
                    with (source / manifest["merged_image"]).open("rb") as merged:
                        merged.seek(int(segments["boot_app0"]["offset"], 16))
                        data = merged.read(record["bytes"])
                    if len(data) != record["bytes"] or sha256(data).hexdigest() != record["sha256"]:
                        raise ValueError(f"{source}: mismatched boot_app0 segment")
                    boot_app = (target, data)
                else:
                    check(source / name, record)
                    mappings[target] = source / name
        for target, original in mappings.items():
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(original, target)
        if boot_app:
            target, data = boot_app
            target.write_bytes(data)
    except (OSError, ValueError) as error:
        print(f"Originals import failed: {error}", file=sys.stderr)
        return 1
    print(f"Imported {len(mappings)} Arduino files and the shared boot_app0 segment.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

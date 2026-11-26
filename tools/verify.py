#!/usr/bin/env python3
"""Verify the public archive, and optionally its exact local firmware files."""

import argparse
import gzip
from hashlib import file_digest
import json
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]


def check(path, record, decompress=False):
    opener = gzip.open if decompress else open
    if not decompress and path.stat().st_size != record["bytes"]:
        raise ValueError("size mismatch")
    with opener(path, "rb") as stream:
        digest = file_digest(stream, "sha256").hexdigest()
    if digest != record["sha256"]:
        raise ValueError("SHA-256 mismatch (run git lfs pull for LFS assets)")


def check_published_case(case, index, rebuilt_dir):
    manifest = json.loads((ROOT / case["manifest"]).read_text())
    files = manifest["files"]
    merged = files[manifest["merged_image"]]
    if merged["sha256"] != case["firmware_sha256"]:
        raise ValueError("case SHA-256 differs from the merged image")
    if merged["sha256"] != manifest["validation"]["firmware_sha256"]:
        raise ValueError("merged image differs from the validation input")
    for record in files.values():
        indexed = index["public_files"][record["path"]]
        if any(indexed[field] != record[field] for field in ("bytes", "sha256")):
            raise ValueError(f"manifest/index mismatch for {record['path']}")
    with (ROOT / merged["path"]).open("rb") as stream:
        for segment in manifest["flash_layout"]["segments"].values():
            record = files[segment["file"]]
            stream.seek(int(segment["offset"], 16))
            if stream.read(record["bytes"]) != (ROOT / record["path"]).read_bytes():
                raise ValueError(f"merged contents differ from segment {segment['file']}")
    if rebuilt_dir:
        folder = rebuilt_dir / case["device_id"] / case["case"]
        boot_app = manifest["flash_layout"]["segments"]["boot_app0"]["file"]
        for name, record in files.items():
            if name != boot_app:
                check(folder / name, record)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--with-local", action="store_true", help="require and verify every original firmware file")
    parser.add_argument("--rebuilt-dir", type=Path, help="compare a rebuild's firmware directory against the archive")
    args = parser.parse_args()
    index = json.loads((ROOT / "index.json").read_text())
    files = index["public_files"] | (index["local_files"] if args.with_local else {})
    errors = []
    for name, record in sorted(files.items()):
        try:
            check(ROOT / name, record)
            if "uncompressed" in record:
                check(ROOT / name, record["uncompressed"], decompress=True)
        except (OSError, ValueError) as error:
            errors.append(f"{name}: {error}")
    published = [case for case in index["cases"] if "original_build" in case]
    expected_count = index.get("arduino_rebuild", {}).get("published_cases", 0)
    if len(published) != expected_count or (args.rebuilt_dir and not published):
        errors.append("published Arduino case count does not match the index")
    for case in published:
        try:
            check_published_case(case, index, args.rebuilt_dir)
        except (KeyError, OSError, ValueError) as error:
            errors.append(f"{case['manifest']}: {error}")
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    deferred = 0 if args.with_local else len(index["local_files"])
    print(f"Verified {len(files)} files, including the decoded SD image. Local-only checks skipped: {deferred}.")
    if published:
        print(f"Verified {len(published)} published case manifests, flash segments and validated image hashes.")
    if args.rebuilt_dir:
        print(f"Verified {len(published)} rebuilt firmware sets against the archive.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

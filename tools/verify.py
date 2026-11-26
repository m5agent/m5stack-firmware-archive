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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--with-local", action="store_true", help="require and verify every original firmware file")
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
    if errors:
        print("\n".join(errors), file=sys.stderr)
        return 1
    deferred = 0 if args.with_local else len(index["local_files"])
    print(f"Verified {len(files)} files, including the decoded SD image. Local-only checks skipped: {deferred}.")
    return 0


if __name__ == "__main__":
    sys.exit(main())

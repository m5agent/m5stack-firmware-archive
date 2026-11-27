#!/usr/bin/env python3
"""Scan firmware strings for private paths and report known SDK build prefixes."""

import argparse
from collections import Counter
import getpass
from pathlib import Path
import re
import socket
import subprocess
import sys


SDK_PREFIXES = (
    "/home/runner/work/esp32-arduino-lib-builder/",
    "/home/runner/.espressif/tools/xtensa-esp-elf/",
    "/home/runner/.espressif/tools/riscv32-esp-elf/",
    "/builds/idf/crosstool-NG/",
    "/IDF/components/",
)
PRIVATE_PATH = re.compile(
    r"(?<![\w./-])/(?:Users|home|root|tmp|private|Volumes|var/folders|mnt|workspace|Applications)/"
    r"|[A-Z]:[\\/](?:Users|Documents and Settings)[\\/]",
    flags=re.IGNORECASE,
)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=Path, help="directory with firmware .bin and .elf files")
    parser.add_argument("--forbid", action="append", default=[], help="additional identity or path to reject")
    args = parser.parse_args()
    files = sorted(p for p in args.root.rglob("*") if p.is_file() and p.suffix in (".bin", ".elf"))
    if not files:
        parser.error("no BIN or ELF files found")
    # Root is a system account name and also a common word in SDK diagnostics.
    username = getpass.getuser()
    terms = {socket.gethostname(), socket.gethostname().split(".")[0], *args.forbid}
    if username not in {"root", "runner"}:
        terms.add(username)
    identities = re.compile(
        r"(?<![\w])(?:" + "|".join(re.escape(term) for term in sorted(terms) if term) + r")(?![\w])",
        re.IGNORECASE,
    )
    sdk_counts = Counter()
    hits = 0
    for path in files:
        result = subprocess.run(["strings", "-a", str(path)], check=True, capture_output=True, text=True)
        bad = 0
        for line in result.stdout.splitlines():
            for prefix in SDK_PREFIXES:
                if prefix in line:
                    sdk_counts[prefix] += 1
                    line = line.replace(prefix, "sdk/")
            if PRIVATE_PATH.search(line) or identities.search(line):
                bad += 1
        if bad:
            print(f"{path.relative_to(args.root)}: {bad} private path or identity matches", file=sys.stderr)
        hits += bad
    print(f"Scanned {len(files)} files with strings -a. Private path or identity matches: {hits}.")
    for prefix, count in sorted(sdk_counts.items()):
        print(f"Espressif prefix {prefix} ({count} strings)")
    return bool(hits)


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""Download the original M5Burner image and publish it locally only after verification."""

import json
from pathlib import Path
import shutil
import sys
from tempfile import NamedTemporaryFile
from urllib.request import urlopen

from verify import ROOT, check


def main():
    manifest = ROOT / "devices/cores3/uiflow2/v2.5.3/firmware.json"
    info = json.loads(manifest.read_text())
    record = info["files"]["uiflow2-v2.5.3.bin"]
    target = ROOT / record["path"]
    target.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        if target.exists():
            check(target, record)
        else:
            with NamedTemporaryFile(dir=target.parent, delete=False) as output:
                temporary = Path(output.name)
                with urlopen(info["download_url"], timeout=60) as response:
                    shutil.copyfileobj(response, output)
            check(temporary, record)
            temporary.replace(target)
        print(target)
        return 0
    except (OSError, ValueError) as error:
        print(f"UIFlow2 download or verification failed: {error}", file=sys.stderr)
        return 1
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


if __name__ == "__main__":
    sys.exit(main())

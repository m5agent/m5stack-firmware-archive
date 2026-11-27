#!/usr/bin/env python3
"""Rebuild the native esp32sim Arduino cases from the archived sketches."""

import argparse
from concurrent.futures import ThreadPoolExecutor
from hashlib import file_digest
import json
import os
from pathlib import Path
import shutil
import subprocess
from urllib.request import urlopen
from zipfile import ZipFile

from verify import ROOT, check


SUFFIXES = (".bin", ".bootloader.bin", ".partitions.bin", ".merged.bin", ".elf")
EPOCH = "1795651200"
FLAG_PROPERTIES = ("compiler.c.extra_flags", "compiler.cpp.extra_flags", "compiler.S.extra_flags")


def digest(path):
    with path.open("rb") as stream:
        return file_digest(stream, "sha256").hexdigest()


def copy_dependencies(dependencies, source, supplied_zip):
    archive = source.parent / "LTR5XX.zip"
    if supplied_zip:
        shutil.copyfile(supplied_zip, archive)
    else:
        with urlopen(dependencies["url"], timeout=60) as response, archive.open("wb") as stream:
            shutil.copyfileobj(response, stream)
    if digest(archive) != dependencies["zip_sha256"]:
        raise ValueError("LTR5XX ZIP SHA-256 mismatch")
    with ZipFile(archive) as bundle:
        for name, record in dependencies["extracted_files"].items():
            member, = (item for item in bundle.namelist() if Path(item).name == name)
            output = source / name
            output.write_bytes(bundle.read(member))
            check(output, record)


def rebuild(entry, args, flags, env):
    board, case = entry["device_id"], entry["case"]
    manifest = json.loads((ROOT / entry["manifest"]).read_text())
    build_info = manifest["toolchain"]
    scratch = args.out / "work" / board / case
    reproducibility = manifest.get("reproducibility", {})
    build_name = reproducibility.get("build_sketch_name", board)
    source = scratch / build_name
    source.mkdir(parents=True)
    firmware = args.out / "firmware" / board / case
    firmware.mkdir(parents=True)
    for name, record in manifest["sketch_files"].items():
        original = ROOT / record["path"]
        check(original, record)
        shutil.copyfile(original, source / (build_name + ".ino" if name.endswith(".ino") else name))
    if "dependencies" in build_info:
        copy_dependencies(build_info["dependencies"], source, args.ltr_zip)
    command = [
        str(args.cli), "--config-file", str(args.config), "compile",
        "--fqbn", build_info["fqbn"], "-j", str(args.compiler_jobs),
        "--build-path", str(scratch / "build"),
    ]
    if args.esptool_dir:
        command += ["--build-property", f"runtime.tools.esptool_py.path={args.esptool_dir}"]
    case_flags = flags
    if reproducibility:
        work_name = "override-" + case if build_info.get("library_overrides") else "normal"
        case_flags += " " + prefix_flags({scratch: f"/build-env/work/{board}/{work_name}"})
    for library, info in build_info.get("library_overrides", {}).items():
        if args.library_overrides is None:
            raise ValueError(f"{board}/{case} requires --library-overrides")
        location = args.library_overrides / library
        if not location.is_dir():
            location = location.with_name(f"{library}-{info['version']}")
        if not location.is_dir():
            raise ValueError(f"{board}/{case} missing {library} {info['version']}")
        command += ["--library", str(location)]
    for prop in FLAG_PROPERTIES:
        command += ["--build-property", f"{prop}={case_flags}"]
    log_path = scratch / "compile.log"
    with log_path.open("w") as log:
        rc = subprocess.run([*command, str(source)], cwd=scratch, env=env,
                            stdout=log, stderr=subprocess.STDOUT).returncode
    if rc:
        raise RuntimeError(f"{board}/{case} build failed ({rc}): {log_path}")
    sketch = manifest["merged_image"].removesuffix(".merged.bin")
    for suffix in SUFFIXES:
        shutil.copyfile(scratch / "build" / (build_name + ".ino" + suffix), firmware / (sketch + suffix))
    print(f"{board}/{case} {digest(firmware / manifest['merged_image'])}", flush=True)


def prefix_flags(paths):
    # GCC applies the last matching prefix, so put nested paths after their parents.
    return " ".join(
        f'"-f{kind}-prefix-map={path}={paths[path]}"'
        for path in sorted(paths, key=lambda item: len(item.parts))
        for kind in ("file", "debug", "macro")
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", type=Path, required=True)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--data", type=Path, required=True, help="Arduino data directory in the CLI config")
    parser.add_argument("--user", type=Path, required=True, help="Arduino user directory in the CLI config")
    parser.add_argument("--esptool-dir", type=Path, help="optional esptool override directory")
    parser.add_argument("--library-overrides", type=Path, help="directory containing pinned override libraries")
    parser.add_argument("--ltr-zip", type=Path, help="use a local copy of the pinned LTR5XX ZIP")
    parser.add_argument("--out", type=Path, required=True, help="new build output directory")
    parser.add_argument("--jobs", type=int, default=2, help="concurrent cases")
    parser.add_argument("--compiler-jobs", type=int, default=4, help="jobs per compiler invocation")
    parser.add_argument("cases", nargs="*", metavar="DEVICE/CASE")
    args = parser.parse_args()
    for key in ("cli", "config", "data", "user", "esptool_dir", "library_overrides", "ltr_zip", "out"):
        value = getattr(args, key)
        if value is not None:
            setattr(args, key, value.resolve())

    paths = {
        args.data: "/build-env/arduino-data",
        args.user: "/build-env/arduino-user",
        args.out: "/build-env/work",
    }
    if len(paths) != 3:
        parser.error("data, user and out must be distinct directories")
    if args.library_overrides:
        paths[args.library_overrides] = "/build-env/library-overrides"
    flags = prefix_flags(paths)
    env = dict(
        os.environ,
        ARDUINO_DIRECTORIES_DATA=str(args.data),
        ARDUINO_DIRECTORIES_USER=str(args.user),
        SOURCE_DATE_EPOCH=EPOCH,
        TZ="UTC",
    )
    entries = {
        f"{case['device_id']}/{case['case']}": case
        for case in json.loads((ROOT / "index.json").read_text())["cases"]
        if case["source"] == "arduino-docs" and case["lane"] == "native esp32sim" and "original_build" in case
    }
    if unknown := set(args.cases) - entries.keys():
        parser.error(f"unknown cases: {', '.join(sorted(unknown))}")
    args.out.mkdir(parents=True)
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        jobs = [pool.submit(rebuild, entries[key], args, flags, env) for key in args.cases or entries]
        for job in jobs:
            job.result()


if __name__ == "__main__":
    main()

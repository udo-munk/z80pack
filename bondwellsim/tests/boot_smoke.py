#!/usr/bin/env python3
"""Opt-in integration test using user-supplied Bondwell ROMs and SYSTEM1.

Reference ROMs and SYSTEM1 are included under roms/ and disks/.
"""
import argparse
from pathlib import Path
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--exe", type=Path, required=True)
parser.add_argument("--rom", type=Path, required=True)
parser.add_argument("--chargen", type=Path, required=True)
parser.add_argument("--disk", type=Path, required=True)
args = parser.parse_args()
base = [str(args.exe.resolve()), "--headless", "--turbo", "--text",
        "--rom", str(args.rom.resolve()), "--chargen", str(args.chargen.resolve())]


def run(options, expected):
    result = subprocess.run(base + options, text=True, capture_output=True, timeout=90)
    if result.returncode or any(token not in result.stdout for token in expected):
        raise SystemExit(f"Boot regression failed\n{result.stdout}\n{result.stderr}")
    print(result.stderr.strip())


for model, memory in [("12", "64K"), ("14", "128K")]:
    run(["--model", model, "--cycles", "50000000"],
        [f"{memory} RAM IS INSTALLED", "PASS ALL TESTS", "PRESS ANY KEY"])
disk = str(args.disk.resolve())
run(["--disk-a", disk, "--disk-b", disk, "--cycles", "430000000",
     "--type", " EDIR B:\r", "--type-delay", "10000"],
    ["A>DIR B:", "B: DEVICE", "CPM3", "SYS", "AUTORUN"])
print("Original-ROM diagnostics (12/14), CP/M 3 boot, keyboard, and drive B DIR passed")

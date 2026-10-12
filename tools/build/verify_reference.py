#!/usr/bin/env python3
"""Verify an explicitly supplied private dependency; never import or execute its scripts."""
import hashlib
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

def main():
    if len(sys.argv) != 2:
        raise SystemExit("usage: verify_reference.py PRIVATE_ADAPTATION_ROOT")
    private = Path(sys.argv[1]).resolve()
    manifest = json.loads((ROOT / "sources/wudada_reference/fix6_audit.lock.json").read_text())
    for name, expected in manifest["adapted_compile_files"].items():
        p = private / name
        if not p.resolve().is_relative_to(private) or not p.is_file():
            raise SystemExit(f"Missing or unsafe private dependency: {name}")
        if hashlib.sha256(p.read_bytes()).hexdigest() != expected:
            raise SystemExit(f"Unaudited private dependency version: {name}")
    print(f"PASS: {len(manifest['adapted_compile_files'])} private compile inputs match audited hashes")

if __name__ == "__main__":
    main()

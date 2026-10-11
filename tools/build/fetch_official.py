#!/usr/bin/env python3
"""Clone only pinned research sources; never run upstream scripts or device tools."""
import argparse
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def git(path, *args):
    return subprocess.check_output(["git", "-C", str(path), *args], text=True).strip()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--destination", type=Path, default=ROOT / "sources/official/checkouts")
    parser.add_argument("--repo", action="append", help="Select a manifest repository name")
    args = parser.parse_args()
    manifest = json.loads((ROOT / "sources/official/manifest.lock.json").read_text())
    entries = manifest["repositories"]
    if args.repo and set(args.repo) - {item["name"] for item in entries}:
        parser.error("Unknown repository; choose an existing manifest entry")
    destination = args.destination.resolve()
    destination.mkdir(parents=True, exist_ok=True)
    for entry in entries:
        name, sha, url = entry["name"], entry["commit"], entry["url"]
        if args.repo and name not in args.repo:
            continue
        if not re.fullmatch(r"[A-Za-z0-9_-]+", name):
            raise ValueError("Invalid source directory name")
        if not re.fullmatch(r"[0-9a-f]{40}", sha):
            raise ValueError("Source must use a complete fixed SHA")
        if url != f"https://github.com/sindarin-inc/{name}.git":
            raise ValueError("Unexpected official source URL")
        path = destination / name
        if path.exists():
            if path.is_symlink() or not (path / ".git").is_dir():
                raise RuntimeError(f"Refusing to modify existing non-checkout: {path}")
            if git(path, "rev-parse", "HEAD") != sha or git(path, "status", "--porcelain"):
                raise RuntimeError(f"Existing source has a different revision or changes: {path}")
        else:
            subprocess.run(["git", "clone", "--no-checkout", url, str(path)], check=True)
            subprocess.run(["git", "-C", str(path), "checkout", "--detach", sha], check=True)
        if git(path, "rev-parse", "HEAD") != sha:
            raise RuntimeError(f"Revision mismatch: {name}")
        print(f"{name}: {sha} ({entry['license_status']})")
    print("Research checkout only. Submodules, toolchain, build and device access were not run.")


if __name__ == "__main__":
    main()

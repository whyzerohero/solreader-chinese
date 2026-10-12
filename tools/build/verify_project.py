#!/usr/bin/env python3
"""Verify project structure and source references without network or device access."""
import argparse
import json
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DOCS = [
    "01_项目目标.md", "02_官方源码清单.md", "03_第一代硬件与版本.md",
    "04_屏幕与电源.md", "05_遥控器协议.md", "06_中文阅读引擎.md",
    "07_刷机备份与恢复.md", "08_自制极简阅读固件设计参考.md", "09_九宫格与笔记.md", "10_开发进度.md",
]


def git(path, *args):
    return subprocess.check_output(["git", "-C", str(path), *args], text=True).strip()


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", type=Path)
    args = parser.parse_args()
    files = ["AGENTS.md", "README.md", ".gitignore",
             "sources/official/manifest.lock.json", "sources/official/ecosystem_inventory.json",
             "sources/official/release_v1.6.0.json", "sources/wudada_reference/README.md",
             "firmware/solreader_custom/include/solreader/contracts.hpp",
             "tools/build/README.md", "tools/build/fetch_official.py",
             "tools/build/verify_project.py", "tools/build/audit_fonts.py",
             "tools/flash/README.md", "tools/recovery/README.md", "tests/contract_compile.cpp",
             "tests/README.md", "backups/README.md", "outputs/README.md"]
    files += ["docs/" + name for name in DOCS]
    for name in files:
        path = ROOT / name
        require(path.is_file() and path.stat().st_size > 0, f"Missing or empty: {name}")
    # Scan tracked project text for the removed dependency and routing assumptions.
    forbidden = ["WAITING_" + "FOR_SOURCE_IMPORT", "原厂遥控器驱动的" + "单手九宫格",
                 "遥控器操作的" + "软件九宫格", "virtual void navigate(const " + "InputEvent"]
    for name in git(ROOT, "ls-files").splitlines():
        path = ROOT / name
        if not path.is_file():  # A renamed file can remain in the index before staging.
            continue
        text = path.read_text(encoding="utf-8")
        for phrase in forbidden:
            require(phrase not in text, f"Obsolete architecture assumption: {name}")
    require(not (ROOT / "docs/08_武哒哒移植.md").exists(), "Obsolete migration plan exists")
    require(not re.search(r"wudada|武哒哒", (ROOT / "README.md").read_text(), re.IGNORECASE),
            "Internal name exposed in public README")
    rules = (ROOT / "AGENTS.md").read_text()
    for number in range(1, 21):
        require(re.search(rf"(?m)^{number}\. ", rules), f"Missing project rule {number}")
    manifest = json.loads((ROOT / "sources/official/manifest.lock.json").read_text())
    require(manifest["reproducible_firmware_build_verified"] is False, "Unverified build claim")
    for entry in manifest["repositories"]:
        require(re.fullmatch(r"[0-9a-f]{40}", entry["commit"]), "Incomplete SHA")
        require(entry["url"] == f"https://github.com/sindarin-inc/{entry['name']}.git",
                "Source URL does not match repository")
        require(entry["license_status"], "Missing license status")
        for sub in entry["submodules"]:
            require(re.fullmatch(r"[0-9a-f]{40}", sub["commit"]), "Incomplete submodule SHA")
        if args.source_root:
            path = args.source_root / entry["name"]
            require(git(path, "rev-parse", "HEAD") == entry["commit"],
                    f"Checkout revision differs: {entry['name']}")
            require(not git(path, "status", "--porcelain", "--untracked-files=no"),
                    f"Tracked source modified: {entry['name']}")
            actual = {}
            for line in git(path, "ls-tree", "-r", "HEAD").splitlines():
                metadata, name = line.split("\t", 1)
                mode, _, sha = metadata.split()
                if mode == "160000":
                    actual[name] = sha
            require(actual == {s["path"]: s["commit"] for s in entry["submodules"]},
                    f"Submodule tree differs: {entry['name']}")
    probes = ["backups/device.bin", "outputs/render.png", "books/private.epub",
              "notes/private.txt", ".env", "private/device_info.txt",
              "sources/official/checkouts/embedded-app/src/main.cpp",
              "sources/wudada_reference/imported/private.cpp", "logs/session.log",
              "firmware/solreader_custom/build/example.o", "private/access.token",
              "private/key.pem"]
    for probe in probes:
        result = subprocess.run(["git", "-C", str(ROOT), "check-ignore", "-q", probe])
        require(result.returncode == 0, f"Private/generated path is not ignored: {probe}")
    for name in files:
        result = subprocess.run(["git", "-C", str(ROOT), "check-ignore", "-q", name])
        require(result.returncode == 1, f"Project deliverable accidentally ignored: {name}")
    release = json.loads((ROOT / "sources/official/release_v1.6.0.json").read_text())
    require(release["archive_sha256_verified"] is True, "Missing actual release checksum result")
    require(release["device_binary_identity_verified"] is False, "Unverified device identity claim")
    print(f"PASS: {len(files)} project files; 20 rules; fixed references; privacy ignore probes")
    print("PASS: checkout HEAD and submodule trees" if args.source_root else
          "NOT RUN: checkout validation (supply --source-root)")
    print("NOT RUN: firmware build, upstream tests, simulator, device, backup or recovery")


if __name__ == "__main__":
    main()

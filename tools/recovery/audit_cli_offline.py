#!/usr/bin/env python3
"""Source-pinned Python behavior models and ZIP inspection; never run the CLI.

This does not execute or compile Haskell, import serial/esptool, open ports,
or invoke package scripts. The source guards tie models to the audited revision.
"""
import argparse
import hashlib
import json
from pathlib import Path, PurePosixPath
import struct
import subprocess
import zipfile

SHA = "db769460455e05ea54b1460b86493ecf4f9785eb"
ZIP_SHA = "14c2df908082b83a860f0cc02aa1c97edc1c06554796684a07642cbe0e8bb0e8"


def check(condition, name):
    if not condition:
        raise AssertionError(name)
    print("PASS", name)


def flash_model(override=None, discovered=None, arrives_at=None):
    """Translate Main.flash only; all operations are strings, not I/O."""
    if override is not None:
        return ["esptool:" + override]
    if discovered is not None:
        return ["esptool:" + discovered]
    events = ["PROGRAM"]
    if arrives_at is not None and arrives_at <= 15:
        events.append("esptool:mock-port")
    else:
        events.append("port-not-found")
    return events


def discover_model(by_id, existing, jtag=False):
    fragment = "JTAG_serial_debug_unit" if jtag else "Sol_Reader"
    candidates = sorted("/dev/serial/by-id/" + n for n in by_id if fragment in n)
    if not jtag:
        candidates.append("/dev/cu.usbmodemsolreader1")
    return next((p for p in candidates if p in existing), None)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cli", required=True, type=Path)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()
    root = args.cli.resolve()
    head = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    dirty = subprocess.check_output(["git", "-C", str(root), "status", "--porcelain"], text=True)
    if head != SHA or dirty:
        raise RuntimeError("CLI checkout must be the clean pinned revision")
    paths = ["app/Main.hs", "src/Sol/Device.hs", "src/Sol/Serial.hs",
             "src/Sol/Ports.hs", "src/Sol/Firmware.hs", "solreader.cabal", "flake.nix", "flake.lock"]
    sources = {p: (root / p).read_text() for p in paths}
    main_src, ports = sources[paths[0]], sources[paths[3]]
    check(all(s in main_src for s in [
        "maybe (findJtagPort >>= enterProgrammingMode) (pure . Just) jtagOverride",
        "enterProgrammingMode (Just p) = pure (Just p)",
        "withDevice port program", "waitForPort 15 findJtagPort"]), "flash model source guards")
    check('program = sendLine "PROGRAM"' in sources[paths[1]] and
          'line <> "\\n"' in sources[paths[1]], "PROGRAM wire source guards")
    check(all(s in ports for s in ['"/dev/serial/by-id"', 'linuxPorts "JTAG_serial_debug_unit"',
          'linuxPorts "Sol_Reader"', '"/dev/cu.usbmodemsolreader1"', 'threadDelay 200000']),
          "discovery model source guards")
    check('ws <- words <$> readFile script' in sources[paths[4]] and
          'drop 1 ws' in sources[paths[4]], "argument parser source guards")
    check(flash_model(override="COM6") == ["esptool:COM6"], "override skips PROGRAM without port validation (model)")
    check(flash_model(discovered="other-esp") == ["esptool:other-esp"], "already discovered port skips PROGRAM, identity unchecked (model)")
    check(flash_model(arrives_at=4) == ["PROGRAM", "esptool:mock-port"], "4-second arrival fits CLI 15-second wait (model)")
    check(flash_model(arrives_at=16) == ["PROGRAM", "port-not-found"], "no port within window fails before esptool (model)")
    check(discover_model([], {"COM6", "COM7"}) is None and
          discover_model([], {"COM8"}, jtag=True) is None, "Windows COM paths are not autodiscovered (model)")
    check(discover_model([], {"/dev/cu.usbmodemjtag1"}, jtag=True) is None,
          "macOS JTAG path is not autodiscovered (model)")
    name = "usb-Espressif_USB_JTAG_serial_debug_unit_OTHER-if00"
    check(discover_model([name], {"/dev/serial/by-id/" + name}, jtag=True) is not None,
          "Linux discovery matches other ESP32 by product fragment (model)")
    bundle = next((root / "assets/glasses-firmware").glob("*.zip"))
    check(hashlib.sha256(bundle.read_bytes()).hexdigest() == ZIP_SHA, "pinned bundle SHA-256")
    with zipfile.ZipFile(bundle) as archive:
        check(archive.testzip() is None and all(not PurePosixPath(i.filename).is_absolute()
              and ".." not in PurePosixPath(i.filename).parts for i in archive.infolist()), "ZIP CRC and member paths")
        members = {PurePosixPath(i.filename).name: archive.read(i) for i in archive.infolist()
                   if not i.is_dir() and not i.filename.startswith("__MACOSX/")}
    app = members["embedded-app.bin"]
    version = app[0x30:0x50].split(b"\0")[0].decode()
    idf = app[0x90:0xb0].split(b"\0")[0].decode()
    check(app[0] == 0xe9 and struct.unpack_from("<H", app, 12)[0] == 9 and
          struct.unpack_from("<I", app, 32)[0] == 0xabcd5432 and
          version == "release/v1.0.6-8-gb3db33b2" and idf == "v5.1.4", "ESP32-S3 image and historical app description")
    script = members["flash.sh"].decode()
    # Haskell words/drop 1 translation, deliberately not shell parsing/execution.
    argv = ["-p", "MOCK_ONLY"] + ["/mock/bundle/" + w if w.endswith((".bin", ".hex")) else w
                                     for w in script.split()[1:]]
    check("write_flash" in argv and "default_reset" in argv and "hard_reset" in argv and
          argv[-8::2] == ["0x0", "0x8000", "0xe000", "0x10000"], "bundle retains reset and four write targets (model)")
    partitions = []
    table = members["partition-table.bin"]
    for offset in range(0, len(table), 32):
        magic, ty, sub, start, size, label, flags = struct.unpack_from("<HBBII16sI", table, offset)
        if magic != 0x50aa:
            break
        partitions.append(dict(name=label.split(b"\0")[0].decode(), offset=hex(start),
                               size=hex(size), type=ty, subtype=sub, flags=flags))
    check([p["name"] for p in partitions] == ["nvs", "otadata", "app0", "app1", "coredump", "data"] and
          hashlib.sha256(table).hexdigest() == "ec5edbf63cab4565bb611c4dce085eaa3212642869707f924adf66a95c902afe",
          "six partitions, identical table hash to official v1.6.0 candidate")
    report = dict(repository="https://github.com/collinarnett/solreader-cli", commit=head,
                  source_sha256={p: hashlib.sha256((root / p).read_bytes()).hexdigest() for p in paths},
                  bundle=dict(name=bundle.name, bytes=bundle.stat().st_size, sha256=ZIP_SHA,
                              app_version=version, idf_version=idf,
                              secure_version=struct.unpack_from("<I", app, 36)[0],
                              files={p: dict(bytes=len(b), sha256=hashlib.sha256(b).hexdigest()) for p, b in members.items()},
                              flash_script_text=script, partitions=partitions),
                  offline_checks=dict(passed=16, failed=0, kind="Python source guards/models and binary inspection",
                                      haskell_compiled=False, haskell_executed=False, device_operations=False,
                                      scripts_executed=False))
    if args.report:
        args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n")
    print("16 passed, 0 failed; models/ZIP only, not Haskell or device validation")


if __name__ == "__main__":
    main()

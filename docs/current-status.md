# Current status — November 3, 2026

**Validated functional baseline:** Oxford4_FINAL_ACE, built from the locally modified Sol Reader First Edition release/v1.6.0 source. The source baseline is a working tree with many changes after its older Oxford3 checkpoint; the older Git HEAD alone does not reproduce the device image.

| Result | Evidence level |
| --- | --- |
| 3,654,960-byte ESP32-S3 image; SHA-256 `b67fa8e05c8e72dc489f8e9d135329d8436524522ea616f2d657ed3ea04fe4ae` | Local build artifact and hash |
| Full production build, 16/16 registered host regression suites | Local build/test logs |
| Single app1-only flash at `0x410000`, full image readback hash match; bootloader/partition/OTA/NVS/app0 prefix unchanged | Local device procedure and readback records; private |
| ACE menu, A/E/C switching and restart persistence; reading position, Chinese/English page turns, BLE and full first dictionary definition screen | Owner-reported device acceptance |
| Stable over extended use, all books, full network sync | Not established |

The device has two ET013TT1 E Ink displays, ESP32-S3, 64 MB flash, and 4 MiB app1 at `0x410000`. No complete 64 MB device backup was obtained; a fresh complete app1 backup and protected-prefix backup were verified before the controlled write. The user accepted proceeding with that recovery scope. Backups and the matching ELF remain local and private.

**Current blockers:** immediate “Sync unsuccessful!” was reported for network sync; its exact stage needs new device evidence. A prior occasional rapid-lookup restart has not been explained. Long-term runtime and panel-lifetime tests remain open. See [known issues](known-issues.md).

This milestone does not supersede the unmerged [Chinese TXT prototype PR #2](https://github.com/whyzerohero/solreader-chinese/pull/2). Earlier [development history](10_开发进度.md) documents the October planning/prototype stage and is preserved as historical material.

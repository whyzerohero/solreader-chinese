# Build, validation, and publication boundary

## Environment and artifact lineage

The locally installed firmware derives from [Sindarin embedded-app release/v1.6.0](https://github.com/sindarin-inc/embedded-app/tree/release/v1.6.0), pinned at `7dcf0850daa5cec076c8d68e2f56cf413ff707aa` in the [upstream manifest](../sources/official/manifest.lock.json). The successful local First Edition builds used **ESP-IDF v5.3.2, Xtensa GCC 13.2.0, glasses-release, and the checked 64 MB partition layout**.

| Local milestone | Application bytes | BIN SHA-256 |
| --- | ---: | --- |
| Oxford4_FINAL_ACE | 3,654,960 | `b67fa8e05c8e72dc489f8e9d135329d8436524522ea616f2d657ed3ea04fe4ae` |
| Chinese typography TYPE2 | 3,662,224 | `46f892bbb33aac23bdde74b1ff36ac5b382dc8b33a31f79649ee5ffc013e8c9b` |
| Oxford4 NET4 | 3,671,440 | `59de4634c422a901781e17bd5b84bab816309cc3ebd3365f2e053054d44111a0` |

NET4 fits the checked 4,194,304-byte app1 at `0x410000`, with 522,864 bytes before the partition boundary. The BIN header's ELF digest was matched to its retained ELF. The NET4 private source snapshot is `caf69908a5b9a824709ba39f1e7ccd6ed9b180f7`; it is an archived local source revision, not a commit offered in this public repository. The older main workspace Git HEAD alone does not reproduce the installed image.

The NET4 clean build completed 1,453 steps through compile, link, image generation, and partition checking. An unavailable PowerShell hash-display command then caused a nonzero wrapper result; that original log was retained, the helper was corrected, and final verification exited zero with independently checked hashes. An earlier restricted-environment CMake startup stall is also retained separately. These were not silently relabeled as successful runs.

The existing BT_NIMBLE Kconfig warnings and an unsigned-comparison warning in the Kindle table-of-contents screen remain. The modified NET4 production units introduced no new compiler warnings. Build/test records and matching binaries remain private.

## Recorded test evidence

These rows belong to different revisions and overlapping scopes. **Do not sum them into a single test total.** They are reviewed archived results, not tests rerun for this documentation-only PR.

| Revision / scope | Recorded result | Evidence boundary |
| --- | --- | --- |
| FINAL_ACE registered host regression | **16/16 suites passed** | Includes existing display, reading, dictionary, lifecycle, and network models; not device networking proof |
| ACE dictionary fixture rerun | **192,983 assertions passed** | Two previously missing-path fixture cases later ran with the private pack and a neutral fixture; not exhaustive optical entry testing |
| TYPE2 exact line-end punctuation | **84 cases passed** | Production pager/font logic in host adapters |
| TYPE2 V8 punctuation raster/edge canary | **36 cases passed** | Retained ink and buffer bounds; no physical-panel contrast measurement |
| TYPE2 simulated pagination | **27,744 pages; 318,652 painted rows passed** | Three sizes × eight spacings × 26 heights; production tokenizer/tokens/pager, adapted display/storage |
| TYPE2 dictionary font faces | **3/3 passed** | Independent font check |
| Network baseline regression | **7/7 suites passed** | Wi-Fi model, sync UI, Core0 startup, Dispatch, sleep/wake queue, HTTP lifecycle, DNS; overlaps ACE coverage |
| Production HTTPS integration | **17/17 cases passed** | Actual production factory, SDK HTTP/parser/transport, host mbedTLS/TCP; local certificates and RTOS adapters |
| NET3 affected HTTPS subset | **7 cases passed** | Chunking, incomplete bodies, 64-bit size, redirects, empty error status, and shutdown; not seven additional independent suites |
| NET3 download/ZIP safety | **56 cases passed** | Production FileDownload/Unzipper; network, disk, timing, and resource adapters with failure injection |
| NET3 EPUB loading | **7 cases passed** | Extracted production loading methods with real miniz/pugixml; model/DRM and heap-query adapters |
| NET3 LocalStore retry | **10 cases passed** | Extracted production retry method and adapted API/model |
| TOC iteration | **5,000-level nested case passed** | Existing order retained; no recursive traversal stack; DOM allocation remains separate |
| Existing file-transfer regression | **9 cases / 16,224 assertions passed** | Existing bounded transfer path; no device transfer implied |
| NET4 affected regression | **2/2 CTest suites passed** | Production telemetry queue and sync screen; threaded RTOS and storage/network/UI adapters |
| NET4 telemetry scenarios | **5 scenarios passed**, including 500 same-owner metrics | Self-queue avoidance, full external queue, cancellation, cleanup ordering, closing queue; part of the affected target |

The former telemetry behavior was reproduced with actual production Telemetry and Dispatch: ten queued jobs filled the owner queue and the eleventh submission could not observe cancellation. Host shutdown released that test only; it was not a forced device cleanup technique.

Production HTTPS negative cases rejected wrong hostnames, untrusted issuers, and expired certificates. An earlier local loopback restriction was recorded; completed integration runs used valid new local fixture certificates with verification enabled. Host HTTPS is not an authenticated test of every live service/account.

Host download tests used a constant 4 KiB payload buffer even for an 8 MiB body. Their explicit PSRAM peak reached 61,648 bytes in one download/archive scenario and returned to zero after cleanup; that excludes other allocators and firmware baselines. Target stack auditing of the NET4 changed units found a largest single static frame of 304 bytes, not a full call-chain or hardware stack peak.

## Device verification is a separate level

ACE installation and owner acceptance established Chinese/English reading, dictionary first-screen drawing, BLE controls, reading position, and saved A/C/E selection on one device. TYPE2 was installed and retained production font/pager tests; every layout edge is not independently certified on the displays.

NET4 was separately written to app1 and fully read back with the hash above. Before normal startup, protected-prefix, Core Dump region, and unwritten app1-tail comparisons matched. This does not mean every byte of 64 MB was compared. Local records confirm a complete prior app1 backup and retained recovery images, not a publicly supplied full-flash recovery package.

NET4 logs then established verified TLS, HTTP sync-list retrieval, and at least 8 completed file promotions. The owner reported cancellation exit, a visible clock, and advancing progress. Whole-batch completion and a fresh SNTP transaction were not established. See [network recovery](network-recovery.md) for stage and memory details.

## Public development and publication scope

The earlier [Chinese TXT prototype PR #2](https://github.com/whyzerohero/solreader-chinese/pull/2) has separate tests and remains unmerged. This public checkout does not provide all modified upstream source, SDK overlays, approved font resources, and private dictionary inputs needed for a bit-identical ACE/TYPE2/NET4 rebuild. A pinned research manifest is not a complete reproducibility claim.

Use the existing repository research tools only for their documented source/structure checks; they do not build the installed firmware. Upstream default flashing commands may target bootloader, partition/OTA, or app0 regions and are not a verified app1-only procedure for another device.

This update publishes original English technical summaries and public source references only. It excludes firmware BIN/ELF files, source bundles and patches pending rights review, waveform arrays, private dictionary/font/book data, flash backups, raw logs, credentials, and device identifiers. No blanket license is asserted over upstream material. Historical notes are retained without treating their old blockers as the latest status.

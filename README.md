# SolReader-Chinese: Chinese-First Firmware for Sol Reader

An independent firmware customization project for **Sol Reader First Edition**, extending its original reading experience with Chinese typography, an integrated English dictionary, selectable E Ink refresh modes, and restored network downloading. The goal is comfortable, focused reading in Chinese and English while preserving the original controls, reading positions, and device functionality.

## Current milestone: Oxford4 NET4

**Updated November 9, 2026.** Oxford4_FINAL_ACE remains the retained functional baseline. Subsequent Chinese typography and NET4 network fixes build on that work.

NET4 has been built, installed through a controlled app1-only write, and verified by complete image readback. Physical-device logs confirm Wi-Fi association and DHCP, DNS, TCP, certificate-verified TLS, a successful synchronization-list response, and actual file downloads. At the end of the retained capture, **at least 8 files totaling 6,666,088 bytes had passed verification and been saved**, with further downloading in progress. The owner confirmed that OK could exit synchronization, the menu clock was visible, and download progress began advancing.

**This is a verified network/download milestone, not a completed whole-library or long-term reliability certification.** The final Books/Articles batch result and an independently captured SNTP completion remain unverified. See [current status](docs/current-status.md) and [network recovery](docs/network-recovery.md).

| Area | Implemented and verified scope |
| --- | --- |
| Chinese and English reading | Device-tested reading, page turns, font-size operation, and position restoration |
| Chinese typography | Small / Medium / Large at 16 / 20 / 24 pixel cells; bottom-line preservation, punctuation hanging and compression implemented and host-tested |
| Oxford dictionary | English word lookup, preview/candidate presentation, complete definitions, and correct first full-definition screen tested on device |
| A/C/E refresh | On-device selection and restart persistence tested; C is the default and preferred visual result |
| Networking | Wi-Fi, DNS, verified HTTPS, sync-list retrieval, and saved file downloads confirmed in NET4 device logs |
| Cancellation and clock display | Owner confirmed OK could exit and the main-menu clock appeared; SNTP completion itself was not captured |
| One-handed Chinese T9 input and notes | Future development; not an installed Sol Reader feature |

## Chinese reading and typography

The current firmware adds a file-backed Chinese glyph fallback and a finite **21,791-character mapping**, mixed Chinese/English rendering, and per-book reading-position preservation. Small, Medium, and Large use **16, 20, and 24 pixel character cells**. The underlying V8 bitmap resources are not three independently designed font families; larger-cell appearance still needs broader glyph-by-glyph evaluation.

Pagination fixes preserve the last line at page boundaries. The TYPE2 punctuation work compresses verified blank bearings and permits bounded punctuation hanging into available right-margin space, keeping a final character and closing punctuation together when they genuinely fit. It preserves the glyph's ink and English character advances rather than squeezing or clipping text. See [Chinese reading and typography](docs/chinese-reading.md).

## Integrated dictionary

The reading workflow supports **English-English Oxford word lookup**, preview/candidate selection, and complete definitions. The first full-definition page now renders immediately with its required FULL refresh while retaining the low-flash cursor behavior. The owner verified reading, BLE controls, and the full first definition screen.

The private dictionary pack is not distributed here. A historical restart during rapid repeated lookups remains without a confirmed cause; this project does not claim every entry or every lookup sequence has been validated. See [dictionary behavior](docs/dictionary.md).

## E Ink refresh: A, C, E

The two ET013TT1 displays retain three selectable modes:

| Mode | Eligible ordinary text-page sequence | Owner's device comparison |
| --- | --- | --- |
| **A — Original** | Original FULL, 23 frames | Reference behavior |
| **C — Best Quality (default)** | FULL 23 → real menu-equivalent PARTIAL 11 → FAST 4 → target-page FULL 23; normally **61 frames** | Darkest blacks, sharpest text, least visible ghosting |
| **E — Faster Refresh** | Existing temperature-band CLEAR → FULL; **13 + 19 = 32 frames** at 27–31°C | Clearer than A and faster than C |

Choose **Main Menu → Settings → Reading Style → Refresh Mode**, use Up/Down, and press OK to save. The selection survives restart. C reproduces the actual menu's intermediate drawing state and established drive behavior; it does not merely concatenate arbitrary frames. E uses unchanged existing tables and a consistent white intermediate state. Eligibility checks and protection fallbacks still apply.

The upstream waveform source labels original data as E Ink-provided. That provenance **does not certify the project's combined sequences or their long-term panel lifespan**. The ranking C > E > A is an empirical observation on the owner's device, not a calibrated optical measurement. New refresh-mode development is paused. See [ACE engineering](docs/eink-refresh-ace.md).

## Wi-Fi, HTTPS, and time

NET4 uses the production HTTP factory with a custom transport, asynchronous DNS, bounded connection/I/O deadlines, and strict TLS certificate-chain, hostname, and validity-date checks. Device logs record TLS verification success, sync HTTP 200, and downloaded files reaching verified storage promotion. Earlier immediate failures and the indefinite “Stopping sync” condition are documented with their specific fixes in [network recovery](docs/network-recovery.md).

Downloads stream to storage using a bounded **4 KiB payload buffer**, serialize complete download transactions, release HTTP/TLS ownership before expensive file validation, and validate temporary files before replacing their destination. They do not allocate an entire book-sized download buffer. Existing settings are retained; restoring connectivity does not introduce automatic startup synchronization or new downloads.

The original SolClock SNTP path remains implemented. The owner now sees the clock, but the retained NET4 capture does not prove a fresh SNTP exchange completed. A visible clock, Wi-Fi connection, or partially filled progress bar is not evidence that every synchronization step succeeded.

## Platform, builds, and validation

The verified hardware is **First Edition, ESP32-S3, 64 MB flash, two ET013TT1 displays, and PSRAM**. Local production builds use **ESP-IDF v5.3.2, Xtensa GCC 13.2.0, and the glasses-release configuration**. The checked device layout has a 4 MiB app1 at `0x410000`; this is not a generic flashing address for other hardware.

The retained NET4 image is **3,671,440 bytes**, SHA-256:

```text
59de4634c422a901781e17bd5b84bab816309cc3ebd3365f2e053054d44111a0
```

The independent complete production build and final checks passed. Historical ACE regressions, Chinese typography cases, production-path HTTPS tests, download/ZIP failure injection, EPUB loading, retry, and NET4 queue/cancellation checks are reported **separately** in [build and validation](docs/build-and-validation.md); overlapping suites are not added into one total. No firmware tests were rerun merely to publish these documents.

The public repository contains documentation, earlier research, and a separate prototype. It does **not** contain all locally modified upstream firmware, SDK overlays, private fonts, or dictionary assets required to reproduce the installed image. Firmware BIN/ELF files and raw logs are not published.

## Known limitations

- Whole-library Books/Articles completion and post-download opening of every new book were not established by the retained capture.
- Fresh SNTP completion, clock accuracy across power cycles, long-term Wi-Fi recovery, and sustained mixed-operation stability need further validation.
- Real-device memory samples are available, but complete peak allocation, all task stacks, extreme EPUB metadata/chapters, and every allocation-failure path are not certified safe.
- Large-size glyph quality, unusual EPUB markup, punctuation edge cases, and cache regeneration need broader real-book regression coverage.
- The historical rapid-dictionary-lookup restart and long-term panel wear remain open.
- Validation is limited to the owner's First Edition configuration. Upstream code, waveform, font, dictionary, and binary redistribution rights remain unresolved.

See the [issue register](docs/known-issues.md) and [stability notes](docs/system-stability.md).

## Roadmap and community

Next steps are whole-session network acceptance, clearer progress diagnostics, memory and reconnect measurements, and further Chinese typography and dictionary regression work. A dedicated **one-handed Chinese T9 input device**, prediction, editing, and note taking remain future exploration; the existing remote continues to control reading. See the [roadmap](docs/roadmap.md).

Questions, feedback, technical discussions, and collaboration are welcome on Reddit: **[u/Brave_Hall_6126](https://www.reddit.com/user/Brave_Hall_6126/)**.

Original Sol Reader firmware and hardware work belong to [Sindarin](https://github.com/sindarin-inc/embedded-app). This is an independent project. [PR #2](https://github.com/whyzerohero/solreader-chinese/pull/2) remains separate and unmerged; the earlier [research history](docs/10_开发进度.md) and [pinned upstream manifest](sources/official/manifest.lock.json) are preserved. This documentation update is submitted for review and does not merge or push directly to main.

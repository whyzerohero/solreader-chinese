# SolReader-Chinese: Chinese-First Firmware for Sol Reader

An independent firmware development project for **Sol Reader First Edition**. It focuses on Chinese reading, English dictionary usability, E Ink refresh quality, and system stability. One-handed Chinese writing is a future goal.

## Current status

**Oxford4_FINAL_ACE is a functionally validated device baseline (November 3, 2026).** A complete ESP32-S3 firmware build succeeded, the app was written to the device's app1 partition and verified by readback, and the owner confirmed Chinese and English page turning, reading-position restoration, BLE, the full first screen of dictionary definitions, and A/C/E mode selection retained after restart. These checks establish a working baseline, not long-term stability.

The project retains the original Sol Reader reading, storage, display, and remote-control foundations. Its local changes add Chinese glyph and pagination support, improve the existing English dictionary presentation, harden task and display lifecycles, and offer three selectable refresh modes. The full modified upstream firmware and firmware binary are **not distributed here** while redistribution rights remain unresolved.

| Milestone | Status |
| --- | --- |
| Chinese and English reading, page turns, position recovery | Verified on one First Edition device |
| English-English Oxford dictionary and complete first definition screen | Verified on device |
| A/C/E E Ink modes and saved selection | Verified on device; quality ranking is observational |
| Core0 task, memory, cancellation, display scheduling fixes | Built and regression-tested; long-term behavior still under study |
| Wi-Fi connection, online sync, online book download | Known issue; not verified working on this firmware |
| Dedicated one-handed Chinese input and note taking | Planned |

## Chinese reading

The current device firmware includes Chinese glyph fallback, mixed Chinese/English rendering, pagination and layout work, font-size selection, and per-book reading-place restoration. The owner has read Chinese and English text and confirmed continuous page turning after the ACE update. Coverage is bounded by the integrated font and mapping; complete Unicode/CJK coverage and flawless punctuation layout are not claimed. Older cached pages may still show earlier punctuation rules until regenerated. See [Chinese reading](docs/chinese-reading.md).

## Dictionary

The current implementation is **English-English**, using an Oxford index/data pack stored privately on the device. It provides word lookup, cursor/preview interaction, and a full definition view. The first definition screen now renders correctly on the device. A historical occasional restart during rapid repeated lookups has no confirmed root cause. This is not a completed Chinese or bilingual dictionary. See [Dictionary](docs/dictionary.md).

## E Ink refresh: A, C, E

Two ET013TT1 displays are driven by the existing firmware display stack. **C — Best Quality** is the saved default after the owner's direct comparison: deeper blacks, sharper text, and less visible ghosting than E or A. **E — Faster Refresh** was observed to look better than A and refresh faster than C. **A — Original** retains the reference behavior. These are user observations, not calibrated optical or lifetime measurements.

Select a mode on the glasses at **Main Menu → Settings → Reading Style → Refresh Mode**. Use Up/Down, then OK to save; the choice survives restart. The enhanced sequences apply only to eligible ordinary text-page FULL refreshes; other screens retain their existing behavior. See [ACE refresh engineering](docs/eink-refresh-ace.md).

## Engineering and validation

The ACE image targets the **First Edition ESP32-S3 with 64 MB flash**, using ESP-IDF v5.3.2 and the glasses-release profile. The app1 partition starts at `0x410000` and holds 4 MiB. The built image is **3,654,960 bytes** with SHA-256 `b67fa8e05c8e72dc489f8e9d135329d8436524522ea616f2d657ed3ea04fe4ae`. The local release passed a clean production build and 16/16 host regression suites. A single authorized app1-only device write passed full image readback; the owner then confirmed the functions above. No whole-flash backup or public flashing package is supplied here.

Development has also addressed static Core0 task stack/TCB allocation, concurrent queue creation, PSRAM/resource lifetimes, shutdown cancellation, and HTTP/DNS/TLS integration. Isolated network tests do **not** prove working device connectivity. See [status](docs/current-status.md), [stability](docs/system-stability.md), and [build and validation](docs/build-and-validation.md).

## Known Limitations and Open Issues

- **Network connectivity remains unresolved.** The glasses previously showed “Sync unsuccessful!” immediately after Connect Internet / Sync. The precise current failure stage is not established. Wi-Fi, authentication, online sync, and book download require device-level diagnosis.
- A historical restart during rapid dictionary lookups remains unexplained; longer reading/BLE/sleep cycles need further observation.
- Refresh quality comparisons are visual reports; combined waveform safety and panel lifetime have not been independently certified.
- Font coverage, older-page punctuation cache behavior, and some format/layout cases remain incomplete or unverified.
- The build is specific to the verified First Edition partition map and hardware. Do not apply it to V2 or other boards.
- Upstream firmware and waveform redistribution rights are unresolved. No firmware BIN, proprietary waveform table, private book, dictionary pack, flash dump, credential, or device identifier is published here.

See the [issue register](docs/known-issues.md) for status and evidence boundaries.

## Roadmap

Next: diagnose device networking, investigate dictionary restart evidence, continue Chinese layout and font work, and measure sustained device stability. Later: a dedicated **12-key one-handed Chinese T9 keyboard**, candidate/phrase prediction, note taking, and possible BLE keyboard integration for comfortable long-form writing. These writing features are planned; they are not present in the validated firmware. See [roadmap](docs/roadmap.md).

## Build, sources, and acknowledgments

This repository keeps its earlier independent Chinese reading prototype and historical research. PR #2 contains unmerged prototype work; it is not the installed ACE firmware. To study the device baseline, start with [build and validation](docs/build-and-validation.md) and the pinned [upstream source manifest](sources/official/manifest.lock.json). Original Sol Reader functionality and hardware work belong to [Sindarin](https://github.com/sindarin-inc/embedded-app). The upstream release/v1.6.0 waveform tool identifies original waveform data as supplied by E Ink; that attribution does not certify our multi-stage sequences or any particular panel batch.

The public repository has no blanket license covering upstream material. We publish documentation and links while source, font, waveform, binary, and private-data rights are reviewed. Earlier Chinese documents remain available as dated historical records.

### Contact & Community

Questions, suggestions, bug reports, and collaboration ideas are welcome. Reach me on Reddit: [u/Brave_Hall_6126](https://www.reddit.com/user/Brave_Hall_6126/). I welcome conversations with developers, E Ink enthusiasts, and people interested in Chinese reading and one-handed writing.

# Known limitations and remaining validation

**Updated November 9, 2026.** Resolved historical paths are separated from what is still unknown.

## Superseded failure descriptions

- The old immediate Wi-Fi initialization failure was traced to `ESP_ERR_NO_MEM`; the subsequent USB RX storage change is retained, and NET4 device logs show successful initialization and DHCP.
- A same-Core1 telemetry queue deadlock was reproduced and fixed in NET4. The owner now reports OK can exit synchronization.
- “No device download has succeeded” is superseded: at least eight files were verified and saved in the NET4 capture.
- The owner now sees the menu clock. A fresh SNTP exchange is still a separate unverified event.

These results do not resolve every possible network failure or establish full-session completion.

## Current register

| Area | Current status | Evidence and next step |
| --- | --- | --- |
| Entire Books/Articles batch | **Not yet established** | Transfers and individual saved files are confirmed; capture ended with work in progress. Verify final result and open new content. |
| SNTP and retained clock accuracy | **Partially verified** | Clock display confirmed by owner; no full SNTP transaction captured. Check fresh/retained time and power-cycle behavior without bypassing TLS date checks. |
| Progress feedback | **Needs broader validation** | A batch-weighted integer display can show zero while a large first file transfers. Capture lacked total count and every UI value; avoid interpreting this as a uniquely diagnosed UI bug. |
| Sustained networking and recovery | **Unverified beyond sampled session** | Reconnects, interruptions, deadlines, and cancellation have host coverage; wider physical-device conditions still need testing. |
| Peak memory / extreme EPUBs | **Open validation and source limits** | Real heap/block/stack samples exist, but not exhaustive peaks, every task, or all operations. Large XML DOM/metadata and arbitrary STL OOM are not fully certified safe. |
| Repeated dictionary lookup | **Unresolved historical event** | A rapid-lookup sequence coincided with a restart; no conclusive matching crash cause. Preserve exact-version evidence if repeated. |
| Chinese glyph/layout coverage | **Finite / incomplete validation** | 21,791-character mapping and targeted punctuation/page tests do not cover every glyph, large-size result, or complex EPUB. Earlier cached-layout reports are not automatically current defects. |
| Long reading / sleep / BLE combinations | **Further validation needed** | Functional baseline passed; sustained mixed-operation and all reconnection patterns are not established. |
| E Ink quality, energy, lifetime | **Observed / unqualified** | C > E > A is an owner comparison, not calibrated optical data or manufacturer qualification of combined stages. |
| Other hardware and redistribution | **Limited / unresolved** | Evidence covers one First Edition configuration. Firmware, fonts, waveforms, private content, and binary redistribution remain outside this publication. |

See [network evidence](network-recovery.md), [Chinese reading](chinese-reading.md), [validation](build-and-validation.md), and [roadmap](roadmap.md). Earlier Chinese research and the unmerged PR #2 remain historical/separate work.

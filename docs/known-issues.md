# Known Limitations and Open Issues

| Area | Status | Evidence and next step |
| --- | --- | --- |
| Network connectivity and synchronization | **Known issue** | The owner reported “Sync unsuccessful!” almost immediately after Connect Internet / Sync. Existing HTTP/DNS/TLS code and host tests do not locate the current device failure stage. Collect matching logs before claiming Wi-Fi, authentication, or download success. |
| Repeated dictionary lookups | **Known issue** | One historical rapid-lookup sequence caused a restart; no conclusive matching crash evidence for that event. Continue observation and preserve a dump with its exact ELF if repeated. |
| Long-term runtime stability | **Unverified** | Functional acceptance and a short communication check passed; sustained reading, BLE reconnection, repeated sleep/wake and memory high-water behavior need measurement. |
| E Ink contrast and lifetime | **Observed / unverified** | C > E > A is an owner visual comparison, not calibrated contrast, energy, or panel-lifetime data. Combined stages have no separate manufacturer qualification on record. |
| Chinese coverage and cached layout | **Partial** | Finite mapped glyph set; older cached pages can retain earlier punctuation layout. Complete typography and all TXT/EPUB variants are unverified. |
| Hardware compatibility | **Limited** | The image and app1 procedure were verified on one First Edition ESP32-S3/64 MB layout. Other models and partitions are not established. |
| Redistribution | **Open** | Upstream firmware, font, waveform, dictionary and binary rights have not been cleared for public distribution. The public update contains documentation and links, not those assets. |

The October Chinese prototype work in PR #2 is separate from the November ACE device image. Status labels refer to the evidence named above.

# Dictionary behavior and status

The current device dictionary is an **English-English Oxford word-definition pack**. It is not a Chinese dictionary or a bilingual translation system. The pack is installed privately on the glasses and is not published here.

The firmware provides cursor-driven word lookup, a compact preview, and a full definition page reached through the existing reading controls. Local fixes addressed text/glyph fallback, definition wrapping, the first screen's display transaction, and related allocation/lifecycle behavior. The owner confirmed that the first full-definition screen appears correctly on the ACE firmware, alongside normal BLE and reading operation.

Host tests covered index bounds, exact/phrase lookup, decompression failure, the real private pack read-only, and a synthetic converted fixture. The ACE delivery recorded 16/16 host regression suites; two dictionary fixture cases initially skipped without their environment paths and were subsequently run separately with 192,983 passing assertions. These tests do not establish every entry's visual rendering on the physical displays.

**Known issue:** a rapid sequence of dictionary lookups once coincided with a restart. The device recovered to reading and BLE functioned afterward. There is no conclusive matching crash analysis for that event, so its cause and cure are still open. If it recurs, preserve logs and use the ELF matching the actually installed image.

The preview/candidate workflow belongs to English lookup, not a completed Chinese T9 candidate system. NET4 preserves the dictionary implementation; a successful file download does not substitute for checking lookup during and after networking. See [current status](current-status.md), [validation](build-and-validation.md), and [community contact](../README.md#roadmap-and-community).

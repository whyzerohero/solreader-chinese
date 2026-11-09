# Roadmap

## Implemented foundation

Chinese/English reading, 16/20/24 pixel Chinese cells, punctuation compression/hanging, bottom-line preservation, reading-place recovery, English dictionary interaction, and saved A/C/E refresh selection are implemented in the local firmware. NET4 adds the proven telemetry deadlock fix and has device evidence of authenticated sync-list access and verified file downloading. These are current work, not future ideas.

## Next engineering work

1. Complete and record entire manual Books/Articles sessions, open downloaded content, and improve progress diagnostics where evidence warrants it.
2. Measure interrupted-download recovery, reconnects, sustained Wi-Fi, resource cleanup after the whole session, and internal/PSRAM/stack peaks under mixed operations.
3. Verify fresh SNTP completion, retained clock accuracy, and main-menu updates across power states while keeping strict TLS validation.
4. Continue Chinese typography, large-size glyph, punctuation-cluster, cache, and complex EPUB regression work using actual problem samples.
5. Investigate any repeated dictionary restart using a dump and ELF matching the installed image; improve lookup behavior without overstating current coverage.
6. Measure existing ACE optical quality, energy, and long-term wear. Keep C as preferred quality and E as the faster choice; new waveform modes remain paused.

## Future one-handed Chinese writing

A dedicated one-handed Chinese T9 input device is a future exploration for comfortable writing. Prediction, candidate/phrase selection, editing, note taking, and possible BLE integration are planned areas, not installed capabilities. Key layout and implementation should follow dedicated design work; the existing Sol Reader remote remains a reading controller.

Questions, technical discussions, and collaboration: [u/Brave_Hall_6126 on Reddit](https://www.reddit.com/user/Brave_Hall_6126/).

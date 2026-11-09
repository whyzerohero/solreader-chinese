# E Ink refresh research and ACE modes

Sol Reader First Edition uses two ET013TT1 E Ink displays. The local investigation compared the existing full-refresh path with temperature selection and extra cleaning paths. At around 30°C, a shorter temperature table made little visible difference. A repeated original FULL (“CLEAN_ONCE”) also produced no noticeable improvement. A menu-equivalent sequence did improve perceived text depth and ghosting. The owner later compared A/C/E on the device and reported **C > E > A** for clarity, with E faster than C. These are observations, not optical measurements.

| Mode | Eligible ordinary text-page behavior | Device observation |
| --- | --- | --- |
| A — Original | Existing 10–13°C FULL table, 23 frames | Reference; more ghosting |
| C — Best Quality (default) | Existing reading FULL 23, actual menu PARTIAL 11 and FAST 4, final target-page FULL 23; normally 61 frames | Deepest black, sharpest text, least ghosting |
| E — Faster Refresh | Existing temperature-band CLEAR then FULL; at 27–31°C on both eyes, 13 + 19 = 32 frames | Better clarity than A; faster than C |

C reuses the real menu drawing state and its saved reading bitmap, original FAST VCOM behavior, counters, and final target bitmap. It is not simply 61 arbitrary frames. E establishes a known white intermediate state and then draws the target; when eye temperatures differ, each eye may require a separate pair. Its selector uses existing bands from 0 to below 50°C, with sensor, pump, inter-eye spread, cancellation, and driver-state guards. An exceptional temperature or an ineligible page may use the original path. Exact frame count therefore depends on the actual transaction. The implementation does not alter raw LUT entries, VCOM values, polarity, or drive frequency.

The ACE modes are selected at **Main Menu → Settings → Reading Style → Refresh Mode**. Use Up/Down and OK. The selection persists; missing or invalid preference values resolve to C. The enhanced route is limited to eligible ordinary pure-text reading FULL page turns. Dictionary overlays, menus, sleep/wake, images, and partial cursor refreshes retain their established paths.

[Sol Reader release/v1.6.0 waveform source](https://github.com/sindarin-inc/embedded-app/blob/release/v1.6.0/scripts/waveformtool/Waveforms.hpp) identifies original tables as E Ink-provided. The local waveform tool reproduced 56 tables matching the production table data. The project's custom partial/FAST stages and combined C/E sequences have **no separately established E Ink certification** for this device or every panel lot. Optical gains, power cost, temperature effects, and long-term panel wear require further observation. Further new waveform-mode experiments are paused.

## Preserved through NET4

The owner's ACE menu/persistence checks passed after the FINAL_ACE installation. TYPE2 and NET4 retain the A/C/E algorithms; NET4 startup reported C / MENU_EQUIVALENT_FULL without any mode-change command. Existing eligibility rules still decide which individual refreshes use the enhanced path. The approximately 61-frame C and temperature-specific E examples are not promises that every menu, dictionary, image, or wake refresh uses those counts. The E combination is a project implementation built from existing tables, not a separately published ET013TT1 manufacturer recipe or a lifespan guarantee. See [current status](current-status.md).

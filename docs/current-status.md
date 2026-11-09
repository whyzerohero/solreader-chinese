# Current status — November 9, 2026

Oxford4 is a locally developed customization of the original Sol Reader First Edition firmware. **Oxford4_FINAL_ACE** is the retained functional baseline; **TYPE2** adds Chinese punctuation refinements; **NET4** is the latest installed network-recovery image. These names identify successive milestones, not claims of universal stability.

## Evidence at the latest checkpoint

| Result | Evidence level |
| --- | --- |
| ACE: Chinese/English page turns, reading-place restoration, BLE, full first dictionary page, saved A/C/E selection | Owner-confirmed device acceptance |
| C preferred over E and A; E faster than C | Owner visual comparison; no calibrated contrast or lifetime study |
| TYPE2 bottom-line preservation, punctuation compression/hanging, 16/20/24 pixel cells | Production-source review and targeted host tests; broad typography behavior used on device, exhaustive edge cases not optically certified |
| NET4 image: 3,671,440 bytes; complete production build and matched ELF | Local build and artifact records |
| NET4 app1-only installation and complete BIN hash readback | Controlled device write/readback records |
| Wi-Fi initialization, association, DHCP, DNS, TCP, strict TLS verification, sync-list HTTP 200 | NET4 physical-device logs |
| At least 8 files, 6,666,088 bytes, verified and promoted to storage; additional downloading active | NET4 physical-device logs |
| OK could exit synchronization; menu clock visible; subsequent progress advancing | Owner-confirmed observations |
| Entire Books/Articles batch complete; all new books open correctly | Not established at capture end |
| Fresh SNTP exchange, long-term stability, complete memory safety | Not established |

NET4 SHA-256: `59de4634c422a901781e17bd5b84bab816309cc3ebd3365f2e053054d44111a0`.

The current logs supersede the older “networking has never worked” status. They do not support replacing it with “all synchronization and timekeeping are fully verified.” Read the [network evidence](network-recovery.md) and [issue register](known-issues.md).

## Preserved baselines and recovery scope

The hardware checked during the controlled procedure was ESP32-S3 with 64 MB flash and a 4 MiB app1 at `0x410000`. The installed image was backed up as a complete app1 before replacement. The protected prefix, Core Dump region, and unwritten app1 tail were compared before normal startup. No claim is made that all 64 MB was reread and compared.

ACE, typography, earlier network images, matched ELFs, and recovery records remain private and retained. This public documentation is not a flashing package, full-flash backup, or evidence that another board has the same layout.

[PR #3](https://github.com/whyzerohero/solreader-chinese/pull/3) merged the earlier ACE documentation. [PR #2](https://github.com/whyzerohero/solreader-chinese/pull/2) remains unchanged, separate, and unmerged. October [planning history](10_开发进度.md) is preserved as dated history. The public repository does not contain the complete installed firmware source; see [build and validation](build-and-validation.md).

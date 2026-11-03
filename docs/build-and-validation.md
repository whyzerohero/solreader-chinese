# Build, validation, and publication boundary

## Source and environment

The validated device image was built locally from a modified [Sindarin embedded-app release/v1.6.0](https://github.com/sindarin-inc/embedded-app/tree/release/v1.6.0) First Edition source tree. The upstream source candidate is pinned at `7dcf0850daa5cec076c8d68e2f56cf413ff707aa` in the [public manifest](../sources/official/manifest.lock.json). The local Oxford4 working tree contains later uncommitted changes; its older Oxford3 Git HEAD is **not** the full ACE source. No claim is made that the public prototype branch can reproduce this image.

The successful local build used ESP-IDF v5.3.2 (commit `9d7f2d69f50d1288526d4f1027108e314e8c879f`), Xtensa GCC 13.2.0, the glasses-release configuration, the existing 64 MB partition table, and an isolated clean build directory with ccache off. The application image is 3,654,960 bytes, SHA-256 `b67fa8e05c8e72dc489f8e9d135329d8436524522ea616f2d657ed3ea04fe4ae`. It fits app1 at `0x410000` (4 MiB). The image and matched ELF are retained privately.

The local delivery recorded a complete production build; 16/16 registered host regression suites; focused ACE persistence/navigation, display, font, dictionary, Core0 task, cancellation, network model, and stack checks. Two dictionary fixture cases initially skipped for missing paths and then passed separately with the private pack and a neutral fixture. The single authorized app1-only write was read back to the complete BIN length with an identical hash. The owner confirmed functional acceptance afterward. These are different evidence levels: a host test cannot prove optical contrast or online service connectivity.

## Public build guidance

This repository's earlier [Chinese TXT prototype PR #2](https://github.com/whyzerohero/solreader-chinese/pull/2) has its own independent test scripts. It is not the exact installed ACE source. The manifest pins research sources, but this public documentation branch does not provide all local modifications, SDK overlay components, font assets, and permitted redistribution material needed for a bit-identical ACE rebuild. Avoid treating an upstream default `idf.py flash` command as an app1-only device procedure: it can target bootloader, partition, OTA, and app0 regions.

## Publication scope

This update contains original English engineering documentation and public links. It excludes the ACE BIN/ELF, full upstream source tree, E Ink waveform arrays, modified upstream source and build patch pending rights review, private Oxford pack, books, credentials, logs, flash dumps, and device identifiers. Upstream being publicly readable does not alone establish redistribution permission. The repository has no blanket license for those materials. Seek license/permission clarity before adding source patches or binaries.

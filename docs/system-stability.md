# System stability engineering

## Preserved reading and task foundation

An earlier core dump identified a failed Core0BG task-creation assertion on a BLE/PageTurner path. The local Oxford4 baseline reserves static internal stack/TCB storage and guards concurrent queue creation. Host tests cover startup and queue lifecycles. That evidence explains the older failure, not every later restart.

The installed lineage retains PSRAM ownership/destruction fixes, download stack/resource work, serialized display transactions, cancellation/shutdown guards, sleep/wake ordering, reading-position recovery, BLE controls, and full-definition first-screen drawing. ACE selection waits for the existing refresh transaction; unknown physical display state remains guarded after failure. NET4 did not replace these foundations or disable BLE.

## NET4 queue and network ownership

Manual synchronization runs on the existing Core1BG owner. Telemetry formerly submitted to that same single-consumer queue and could block forever when its ten slots filled. NET4 runs same-owner telemetry immediately and bounds submissions from other tasks, logging diagnostic rejection rather than blocking input or cancellation. Upload cleanup is ordered before subsequent same-owner metrics. Production Dispatch itself and its task layout are unchanged.

The owner reported OK could exit after NET4 installation. Subsequent physical logs show real download progress and verified file promotion. Exact cancellation latency and every teardown stage were not captured for the first cancelled attempt; this is not an all-scenario cancellation guarantee.

Transport contexts, TLS, and sockets stay on their existing owner; cancellation signals intent instead of deleting another task's handles. Streaming uses bounded buffers, with network resources released before expensive verification. See [network recovery](network-recovery.md) for details and remaining EPUB/STL limits.

## What measurement does and does not show

The ACE delivery passed 16/16 registered host suites. NET4's two affected CTest targets and five telemetry scenarios passed separately; historical network suites were not falsely rerun. The modified target units had a largest single static frame of 304 bytes. None of those figures is a complete target stack peak.

NET4 device samples include current internal free heap down to 18,611 bytes, largest internal-block samples down to 8,692 bytes, and Core1 network-owner stack high-water reporting down to 2,432 bytes. Boot-global internal minimum reporting reached 12,367 bytes. Samples are not continuous instrumentation or a measure of every task. The session was still active when capture ended, so complete final cleanup and all mixed BLE/dictionary/refresh workloads remain unverified.

No panic, watchdog, or assertion was detected within the retained logging windows; gaps and limited duration prevent a blanket no-crash claim. The historical rapid-lookup restart, extreme metadata OOM, sustained sleep/wake/reconnect behavior, and long-term panel effects remain in the [issue register](known-issues.md). Updated [status](current-status.md) distinguishes observed recovery from pending acceptance.

# Network recovery: NET1 through NET4

**Checkpoint: November 9, 2026.** This records actual source, host-test, and device evidence separately. It does not announce a completed whole-library synchronization or a generally stable firmware release.

## What changed on the physical device

The older immediate failure was localized in NET1 logs to Wi-Fi driver initialization: `ESP_ERR_NO_MEM`, with only one of four expected static RX buffers allocated. It occurred before DNS, TLS, service authentication, or book delivery.

The following memory fix moved the existing roughly 32 KiB USB RX payload storage to explicit PSRAM while retaining its internal control storage and bounded protocol capacity. It preserved BLE and the Core0BG static stack/TCB fix. NET4 logs now show Wi-Fi driver initialization returning zero, successful association, and DHCP/IP readiness.

NET3 subsequently exposed a different failure: progress could remain at zero and OK cancellation could wait indefinitely at “Stopping sync.” The deterministic source defect was a **same-queue telemetry deadlock**:

- Manual synchronization occupied the single Core1BG consumer.
- Telemetry submitted work back to that same ten-entry queue.
- The eleventh submission could wait for space that only the blocked consumer could free.
- That wait prevented the synchronization worker from reaching cancellation checkpoints or teardown.

Production Telemetry and the full production Dispatch implementation reproduced the failure using a threaded host RTOS adapter. This establishes the source defect; a matching live Core1 stack was not captured from the stuck NET3 session.

NET4 executes telemetry immediately when already in its existing owner queue. Other tasks attempt bounded asynchronous submission; rejected diagnostic records produce an explicit warning. Synchronization errors are not discarded or converted into success. Successful telemetry-upload cleanup follows the same ownership rule so it cannot later delete newly written metrics. No extra task, global queue rewrite, or BLE disable was introduced.

## NET4 device results

| Stage | Retained result |
| --- | --- |
| Wi-Fi initialization / association / DHCP | Success; IP ready after about 3,079 ms in the captured attempt |
| DNS and TCP | Success |
| TLS | Verification succeeded with `verify_flags=0`; checks were not disabled |
| Telemetry upload | HTTP 201 |
| Synchronization list | HTTP 200 and response parsed |
| Book payload requests | HTTP 200 and repeated body progress events |
| Verified file completion | At least 8 files, **6,666,088 bytes**, validated and promoted to final storage |
| Cancellation | Owner reported OK could exit; exact cancellation latency was not captured across the logging gaps |
| Clock display | Owner reported the menu clock was visible |
| Full Books/Articles session | Still progressing when the retained capture ended; final batch success unverified |

A 2,310,492-byte file's HTTP request took about 74 seconds, followed by storage/ZIP validation. A zero or small screen percentage is not itself evidence of no data movement: the current UI weights progress over the batch and converts it to integers. The owner later saw progress. The capture did not record the denominator and each UI value, so this is not a uniquely proven explanation of every zero-percent observation.

Articles completion is not inferred from book-file delivery, and a saved file is not evidence that all its chapters have been opened or paginated. The latest local records supersede historical blanket statements that device networking never worked, while whole-session acceptance remains open.

## Production transport and timeouts

The production `SindarinAPI::createClient` factory selects the custom Wi-Fi transport on its existing Core1 owner. Asynchronous DNS feeds nonblocking TCP and mbedTLS; application-level deadlines and cancellation bound the owner operations. The production factory and linked target were checked, rather than testing only an unused transport prototype.

Certificate-chain, hostname/SNI, and certificate validity-date checks remain mandatory. Invalid certificates are not accepted to hide a connection failure. Automatic HTTP redirects are disabled; authentication is restricted to the trusted API origin. HTTP status, transport stage/native error, verification flags, and elapsed time remain diagnostic outputs. An account-specific HTTP 401/403 or future service outage must still be reported as a failure.

The normal factory caps DNS at 10 seconds and connect/idle at 30 seconds within its total budget. File download, disk readback, and compressed/archive verification share a cooperative 600-second transaction budget. These limits are not hard preemption of filesystem or cryptographic library internals.

Recovery retains cancellation and bounded error reporting. Resource, space, invalid-format, and transaction-limit failures do not trigger repeated hammering retries; ordinary recoverable network retries retain their existing limits. No new automatic startup synchronization/download behavior was added, and existing user settings remain intact.

## Streaming, storage, and EPUB safety

The book body streams through a bounded **4,096-byte PSRAM buffer** to a temporary file. The whole book is not buffered in RAM. One admission slot covers the complete download/validate/replace transaction; there is no task per book. HTTP/socket/TLS ownership is released before the expensive disk/archive verification phase.

Admission uses actual free storage and metadata headroom, together with the existing API size range. Fixed and chunked bodies are checked for valid lengths, parser completion, actual byte limits, and unexpected data. Incorrect lengths, oversize headers, interrupted transfers, short writes, insufficient space, and failed allocations return errors.

A temporary file is flushed and closed, its length and disk-readback SHA-256 checked against received bytes, then gzip or EPUB/ZIP data is validated before final rename. gzip checks include CRC/ISIZE; ZIP checks include member ranges, actual output, stream completion, and CRC. Existing destination files are not deleted first. This validates transfer/storage integrity, not an unverified publisher signature or every EPUB semantic rule.

ZIP processing scans the directory with bounded buffers rather than retaining a complete directory index. EPUB reading still uses chapter/member input plus an in-place XML DOM, not a constant-memory whole-book parser. Input/DOM lifetime and allocation checks were improved; a 5,000-level TOC test covers iterative traversal. Arbitrary STL metadata OOM and optional accumulated text paths are not exhaustively safe.

## SNTP and the menu clock are separate evidence

The original SolClock Wi-Fi path uses SNTP with a 10-second wait and cancellation polling; its notification is installed before initialization. BLE time fallback requires an enabled, connected proxy and is not automatically exercised by Wi-Fi-only manual synchronization.

A visible clock does not establish a fresh SNTP exchange. The retained NET4 capture has no complete SNTP transaction. The existing time-validity check accepts an epoch above 1,000,000,000, so an old retained RTC value can bypass a fresh attempt. Clock rendering also depends on menu layout and time-validity state. The owner confirmed the display outcome; **SNTP completion, time accuracy, and behavior across power cycles remain unverified**. Neither the clock logic nor TLS date validation was weakened in NET4.

## Measured memory and remaining acceptance

In the retained NET4 samples, lowest current internal free heap was **18,611 bytes**, the smallest sampled largest internal block was **8,692 bytes**, and the Core1 network owner's lowest reported stack high-water value was **2,432 bytes**. Internal historical-low reporting reached 12,367 bytes; that is a boot-global metric, not a per-request peak allocation. PSRAM samples and cleanup records were also retained privately.

These measurements establish observations during several real downloads, not a complete peak-memory model. Wi-Fi was still serving the ongoing session when capture ended; full post-session recovery and every concurrent BLE/dictionary/display case were not measured. Host allocator peaks and target single-function stack estimates must not be added together or presented as a physical-device safety certificate.

Next acceptance work is an entire manual Books/Articles session, opening saved content, interrupted-transfer recovery, sustained Wi-Fi/reconnect use, retained clock correctness, and mixed-operation resource measurements. See [validation](build-and-validation.md), [stability](system-stability.md), [known issues](known-issues.md), and [community contact](../README.md#roadmap-and-community).

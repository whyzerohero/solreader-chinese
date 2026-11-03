# Chinese reading on First Edition

**Verified on device:** The owner confirmed Chinese and English reading, consecutive page turns, font-size operation in earlier trials, and reading-place restoration after restart. The ACE acceptance also confirmed the reading position and continuous Chinese/English page turns.

The device firmware extends the existing Sol Reader renderer, pager, EPUB/TXT text handling, and font path. Local work added a bounded CJK fallback index and glyph data, mixed-script rendering fixes, punctuation and line-break handling, and cache/version changes. It also addressed allocations and resource lifetimes relevant to rendering. The original firmware still supplies the reading UI, book handling, display driver, and persistent per-book place storage.

Font coverage is a finite mapped set, not every Chinese or Unicode code point. The local V8 font and 21,791-character mapping are implementation assets, not evidence of complete CJK support. Mixed Chinese/English pages were used on device; exhaustive EPUB/TXT format coverage, all punctuation cases, and every glyph have not been measured. Forward-generated pages use the newer Chinese punctuation rules; navigating backward into older cached pages can show an older layout until those pages are regenerated.

A separate public Chinese TXT prototype exists in [PR #2](https://github.com/whyzerohero/solreader-chinese/pull/2). Its test results describe that prototype. It is **not** evidence that its private reference implementation is linked into the current device image. See [current status](current-status.md) and [build notes](build-and-validation.md).

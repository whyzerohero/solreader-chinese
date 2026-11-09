# Chinese reading and typography

## Implemented rendering

The locally installed Oxford4 firmware extends the original reader, pager, EPUB/TXT handling, and font path. It retains original English glyph handling and adds a file-backed V8 Chinese bitmap fallback with a finite **21,791-character mapping**. The resource is read as needed instead of loading its complete bitmap file into RAM.

| Reading size | Chinese character cell |
| --- | --- |
| Small | 16 × 16 pixels |
| Medium | 20 × 20 pixels |
| Large | 24 × 24 pixels |

These are cell sizes in the actual reading-font path, not three independently drawn Chinese fonts. The mapping is not complete Unicode/CJK coverage. Mixed Chinese and English retain their respective rendering paths and existing Latin advances. The original per-book reading position mechanism is preserved.

## Pagination and punctuation fixes

The earlier pagination fix preserves the final painted line at a page boundary instead of losing it during line/page finalization. TYPE2 retains that fix and addresses the reported case where a final Han character and punctuation unnecessarily move together to the next line:

1. Han characters can use the full text-line width; the pager no longer reserves a speculative quarter-em at the end.
2. Eligible closing punctuation first reclaims measured blank bearings. Its visible ink is not flattened or removed.
3. Punctuation can hang into actual available right-margin space, bounded by the display edge and at most one em.
4. Closing punctuation clusters can reclaim their own blank spacing. Ordinary Han and Latin advances remain unchanged.
5. If the result still cannot fit safely, the existing line-break prohibitions take precedence. No forced overlap or clipping is claimed.
6. Drawing and word-selection bounds use the final token width.

Examples checked offline: eight 16-pixel Han cells plus a comma fit a 132-pixel text line after its advance becomes 4 pixels; eight 24-pixel cells on a 192-pixel line can retain an eligible closing mark in an available 8-pixel margin.

## Evidence and limits

The owner confirmed Chinese/English reading, consecutive page turns, font-size operation in earlier trials, and reading-place restoration. The installed NET4 lineage preserves the typography code. That does not establish every punctuation configuration or all larger-size glyphs on the optical displays.

The TYPE2 host record contains **84 exact end-punctuation cases**, **36 actual V8 raster/edge-canary cases**, and **27,744 simulated pages / 318,652 painted rows**. The tokenizer, tokens, pager, and font code were production implementations; display and file containers were host adapters. Assertions checked nonblank UTF-8 coverage, no lost/repeated text, line prohibitions, glyph bounds, and retained ink. These are not 27,744 physical-device pages or complete EPUB parser tests.

Earlier V9 cached-page layout differences are historical observations. Later pagination/cache work and TYPE2 improvements should be assessed against the installed version; this document does not label every old cached-page symptom as a reproduced NET4 defect. Forward/backward regeneration, complex EPUB markup, and mixed punctuation still belong in real-book regression tests.

The separate [Chinese TXT prototype in PR #2](https://github.com/whyzerohero/solreader-chinese/pull/2) is not the installed firmware. Private font resources are not redistributed. See [validation](build-and-validation.md), [known issues](known-issues.md), and [community contact](../README.md#roadmap-and-community).

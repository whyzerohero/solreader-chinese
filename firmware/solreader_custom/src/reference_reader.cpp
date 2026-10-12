#include "solreader/reference_reader.hpp"
// Supplied privately via -I; these files are never vendored into the public project.
#include "reader/paginator.h"
#include "reader/encoding.h"
#include "reader/progress.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace solreader::reference {
struct Reader::Impl {
    ByteSource source;
    txt::Layout layout;
    txt::Measure measure;
    wudada::Paginator paginator;
    std::uint32_t fingerprint = 2166136261U;
    bool bom = false;
    mutable std::array<char,512> mapping_cache{};
    mutable std::size_t mapping_start=0, mapping_size=0;
    static std::size_t read(void *context, std::size_t offset, char *out, std::size_t capacity) {
        auto &self = *static_cast<Impl *>(context);
        if (offset >= self.source.size) return 0;
        const auto wanted = std::min(capacity, self.source.size - offset);
        const auto count = self.source.read(offset, out, wanted);
        if (count > wanted) throw std::runtime_error("byte source returned an invalid count");
        return count;
    }
    static std::size_t encoding_read(void *context, std::size_t offset, std::uint8_t *out, std::size_t capacity) {
        return read(context, offset, reinterpret_cast<char *>(out), capacity);
    }
    static float advance(std::uint32_t cp, float size, void *context) {
        auto &self = *static_cast<Impl *>(context);
        const int width = self.measure(cp, static_cast<int>(size));
        if (width < 0 || width > 65534) throw std::invalid_argument("invalid measured glyph width");
        return static_cast<float>(width);
    }
    Impl(ByteSource src, txt::Layout cfg, txt::Measure metrics)
        : source(std::move(src)), layout(cfg), measure(std::move(metrics)) {
        if (!source.read || !measure || source.size > UINT32_MAX || source.document_id.size() >= 256 ||
            source.document_id.find('\0') != source.document_id.npos || cfg.width <= 0 || cfg.width > 65534 ||
            cfg.height <= 0 || cfg.height > 65535 || cfg.line_height <= 0 || cfg.line_height > cfg.height ||
            cfg.font_size <= 0 || cfg.font_size > 4096)
            throw std::invalid_argument("invalid source or viewport");
        // Validate UTF-8 once with the real bounded decoder, no full-book allocation.
        wudada::encoding::StreamingDecoder decoder(encoding_read, this, source.size,
                                                   wudada::encoding::Encoding::Utf8);
        bom = decoder.detection().utf8Bom;
        wudada::encoding::DecodedScalar scalar;
        while (decoder.next(scalar)) {
            if (scalar.error || (scalar.codepoint < 0x20 && scalar.codepoint != '\r' &&
                scalar.codepoint != '\n' && scalar.codepoint != '\t'))
                throw std::invalid_argument("invalid UTF-8 TXT source");
        }
        if (decoder.ioError()) throw std::runtime_error("TXT validation read failed");
        // Include BOM and exact source bytes in the content identity, not normalized text.
        char buffer[512];
        for (std::size_t at = 0; at < source.size;) {
            const auto n = read(this, at, buffer, sizeof buffer);
            if (!n) throw std::runtime_error("TXT fingerprint read failed");
            for (std::size_t i = 0; i < n; ++i) fingerprint = (fingerprint ^ static_cast<unsigned char>(buffer[i])) * 16777619U;
            at += n;
        }
        wudada::Layout l;
        // Reference contentWidth reserves one pixel. This neutralizes that legacy inset:
        // callers provide the actual content box; no old screen/font constants are used.
        l.width = static_cast<std::uint16_t>(cfg.width + 1); l.height = static_cast<std::uint16_t>(cfg.height);
        l.marginLeft = 0; l.contentTop = 0; l.contentBottom = l.height;
        l.fontPixels = static_cast<float>(cfg.font_size); l.lineHeight = static_cast<float>(cfg.line_height);
        l.maxRows = static_cast<std::uint16_t>(cfg.height / cfg.line_height);
        l.baselineOffset = 0; l.maxGlyphDescent = 0;
        if (!paginator.index({read, this, source.size, wudada::encoding::Encoding::Utf8, bom}, l, advance, this))
            throw std::runtime_error("reference paginator initialization failed");
        paginator.configureNavigation(wudada::nav::hashPath(source.document_id), fingerprint, nullptr);
    }
    char byte(std::size_t at) const {
        if (at >= source.size) return 0;
        if (at < mapping_start || at - mapping_start >= mapping_size) {
            mapping_start=at;
            mapping_size=read(const_cast<Impl *>(this),at,mapping_cache.data(),mapping_cache.size());
            if (!mapping_size) throw std::runtime_error("TXT mapping read failed");
        }
        return mapping_cache[at-mapping_start];
    }
};
Reader::Reader(ByteSource s, txt::Layout l, txt::Measure m) : impl_(std::make_unique<Impl>(std::move(s),l,std::move(m))) {}
Reader::~Reader() = default;
Reader::Reader(Reader &&) noexcept = default;
Reader &Reader::operator=(Reader &&) noexcept = default;
Page Reader::page() const {
    if (busy()) throw std::runtime_error("finish cooperative navigation before drawing");
    auto &i = *impl_;
    const auto p = i.paginator.render(0);
    if (i.paginator.lastError() != wudada::PaginatorError::None) throw std::runtime_error("reference page read failed");
    Page result{p.byteOffset,p.nextByteOffset,{}};
    auto cursor = p.byteOffset;
    for (const auto &s : p.lines) {
        const auto end = cursor + s.size();
        if (end > p.nextByteOffset) throw std::runtime_error("reference source/display mapping mismatch");
        // Adapted UTF-8 mode preserves every display byte, only explicit CR/LF are hidden.
        for (std::size_t j=0;j<s.size();++j) if (i.byte(cursor+j)!=s[j]) throw std::runtime_error("reference text changed");
        auto next = end;
        if (next < p.nextByteOffset && i.byte(next)=='\r') ++next;
        if (next < p.nextByteOffset && i.byte(next)=='\n') ++next;
        const auto cps = txt::decode_utf8(s);
        const bool degraded = !cps.empty() && wudada::Paginator::isNonstarter(static_cast<std::uint32_t>(cps[0].value));
        result.lines.push_back({s,cursor,end,next,degraded}); cursor=next;
    }
    if (cursor != p.nextByteOffset) throw std::runtime_error("incomplete reference source mapping");
    return result;
}
bool Reader::next() { auto &p=impl_->paginator; bool moved=p.advanceStream(false); if(p.lastError()!=wudada::PaginatorError::None)throw std::runtime_error("next page failed");return moved; }
bool Reader::previous() {
    auto &p=impl_->paginator; const bool moved=p.previousStream();
    if (p.lastError()!=wudada::PaginatorError::None ||
        (!p.navigationJobActive() && (p.navigationResult()==wudada::nav::Result::ReadFailed ||
         p.navigationResult()==wudada::nav::Result::CacheInvalid)))
        throw std::runtime_error("previous-page recovery failed");
    return moved;
}
bool Reader::seek(std::size_t offset) {
    if (offset > impl_->source.size || (offset < impl_->source.size &&
        (static_cast<unsigned char>(impl_->byte(offset)) & 0xc0)==0x80)) throw std::invalid_argument("invalid UTF-8 reading offset");
    return impl_->paginator.requestOffset(static_cast<std::uint32_t>(offset));
}
bool Reader::busy() const { return impl_->paginator.navigationJobActive(); }
bool Reader::step() {
    auto &p=impl_->paginator; bool changed=p.stepNavigationJob();
    auto error=p.navigationJobStatus().error;
    if (!p.navigationJobActive() && error!=wudada::nav::JobError::None && error!=wudada::nav::JobError::Eof)
        throw std::runtime_error("navigation recovery failed");
    return changed;
}
void Reader::reflow(txt::Layout layout) {
    auto next=std::make_unique<Impl>(impl_->source,layout,impl_->measure);
    const auto old=offset();
    next->paginator.requestOffset(static_cast<std::uint32_t>(old));
    impl_=std::move(next);
}
std::size_t Reader::offset() const { return impl_->paginator.streamOffset(); }
std::size_t Reader::cached_pages() const { return impl_->paginator.pageOffsets().size(); }
std::string Reader::save() const {
    if (busy()) throw std::runtime_error("navigation must finish before saving progress");
    wudada::progress::Record r;
    r.path=impl_->source.document_id;r.bookHash=wudada::progress::bookPathHash(r.path);
    r.sourceFingerprint=impl_->fingerprint;r.fileSize=static_cast<std::uint32_t>(impl_->source.size);
    r.textOffset=r.pageStartOffset=static_cast<std::uint32_t>(offset());
    wudada::progress::Bytes bytes;
    if (!wudada::progress::encode(r,bytes)) throw std::runtime_error("progress serialization failed");
    return std::string(reinterpret_cast<const char *>(bytes.data()),bytes.size());
}
bool Reader::restore(std::string_view record) {
    wudada::progress::Record r;
    if (!wudada::progress::decode(reinterpret_cast<const std::uint8_t *>(record.data()),record.size(),r) ||
        r.path!=impl_->source.document_id || r.sourceFingerprint!=impl_->fingerprint || r.fileSize!=impl_->source.size)
        throw std::invalid_argument("progress is corrupt or belongs to different content");
    return seek(r.textOffset);
}
} // namespace solreader::reference

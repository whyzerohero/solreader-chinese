#include "solreader/txt_reader.hpp"
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <cstdio>

namespace solreader::txt {
namespace {
std::uint64_t hash(std::string_view s) {
    // FNV-1a: content binding/checksum, not a cryptographic authentication mechanism.
    std::uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : s) { h ^= c; h *= 1099511628211ULL; }
    return h;
}
bool closing(char32_t c) {
    return std::u32string_view(U"，。！？；：、）》】」』〉〕］｝…,.!?;:)]}").find(c) != std::u32string_view::npos;
}
bool newline(char32_t c) { return c == U'\n' || c == U'\r'; }
void validate(Layout l) {
    if (l.width <= 0 || l.width > 1000000 || l.height <= 0 || l.height > 1000000 ||
        l.line_height <= 0 || l.line_height > l.height || l.font_size <= 0 || l.font_size > 4096)
        throw std::invalid_argument("invalid layout dimensions");
}
}
std::vector<Character> decode_utf8(std::string_view s) {
    std::vector<Character> out;
    for (std::size_t i = 0; i < s.size();) {
        const auto start = i;
        const auto lead = static_cast<unsigned char>(s[i++]);
        char32_t cp{}; unsigned n{};
        if (lead < 0x80) cp = lead;
        else if (lead >= 0xc2 && lead <= 0xdf) { cp = lead & 31; n = 1; }
        else if (lead >= 0xe0 && lead <= 0xef) { cp = lead & 15; n = 2; }
        else if (lead >= 0xf0 && lead <= 0xf4) { cp = lead & 7; n = 3; }
        else throw std::invalid_argument("invalid UTF-8 lead at byte " + std::to_string(start));
        if (n > s.size() - i) throw std::invalid_argument("truncated UTF-8 at byte " + std::to_string(start));
        for (unsigned k = 0; k < n; ++k) {
            auto c = static_cast<unsigned char>(s[i++]);
            if ((c & 0xc0) != 0x80) throw std::invalid_argument("invalid UTF-8 continuation");
            cp = (cp << 6) | (c & 63);
        }
        if ((n == 1 && cp < 0x80) || (n == 2 && cp < 0x800) || (n == 3 && cp < 0x10000) ||
            cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
            throw std::invalid_argument("non-scalar or overlong UTF-8");
        if (cp < 0x20 && cp != U'\n' && cp != U'\r' && cp != U'\t')
            throw std::invalid_argument("unsupported TXT control character");
        out.push_back({cp, start, i});
    }
    return out;
}
std::string read_file(const std::string &path, std::size_t limit) {
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("cannot open TXT file");
    std::string result; char block[4096];
    while (f.read(block, sizeof block) || f.gcount()) {
        auto n = static_cast<std::size_t>(f.gcount());
        if (n > limit - result.size()) throw std::length_error("TXT exceeds configured size limit");
        result.append(block, n);
    }
    if (!f.eof()) throw std::runtime_error("TXT read failed");
    return result;
}
Reader::Reader(std::string text, Layout layout, Measure measure)
    : text_(std::move(text)), chars_(decode_utf8(text_)), measure_(std::move(measure)) {
    if (!measure_) throw std::invalid_argument("font measurement required");
    fingerprint_ = hash(text_);
    paginate(layout);
}
void Reader::paginate(Layout layout) {
    validate(layout);
    std::vector<Line> lines;
    std::size_t i = 0, begin = 0;
    while (i < chars_.size()) {
        const auto first = i;
        int width = 0; bool hard = false, oversized = false;
        // UTF-8 BOM remains in original offsets, but is not a rendered glyph.
        if (i == 0 && chars_[i].value == 0xfeff) ++i;
        const auto visible_begin = i == first ? begin : chars_[first].end;
        bool prohibited = i < chars_.size() && closing(chars_[i].value);
        while (i < chars_.size()) {
            auto c = chars_[i].value;
            if (newline(c)) { hard = true; break; }
            int advance = measure_(c, layout.font_size);
            if (advance < 0 || advance > 1000000) throw std::invalid_argument("invalid font advance");
            if (advance > layout.width - width) {
                if (i == first || (i == first + 1 && visible_begin != begin)) {
                    // Single glyph wider than the viewport: consume once, flag for clipped rendering.
                    width = advance; oversized = true; ++i;
                }
                break;
            }
            width += advance; ++i;
        }
        auto end = i < chars_.size() ? chars_[i].begin : text_.size();
        auto source_end = end;
        if (hard) {
            const auto c = chars_[i++].value;
            if (c == U'\r' && i < chars_.size() && chars_[i].value == U'\n') ++i;
            source_end = chars_[i - 1].end;
        }
        // Strict width, no deletion or backtracking. If a closing mark cannot fit,
        // next line is explicitly flagged as a kinsoku degradation.
        lines.push_back({begin, visible_begin, end, source_end, width, hard, prohibited, oversized});
        begin = source_end;
    }
    if (lines.empty() || lines.back().hard_break)
        lines.push_back({text_.size(), text_.size(), text_.size(), text_.size(), 0, false, false, false});
    const auto count = static_cast<std::size_t>(layout.height / layout.line_height);
    std::vector<Page> pages;
    for (std::size_t j = 0; j < lines.size(); j += count) {
        const auto n = std::min(count, lines.size() - j);
        // BOM is included in the first page's original-source interval.
        pages.push_back({j, n, j == 0 ? 0 : lines[j].begin, lines[j + n - 1].source_end});
    }
    layout_ = layout; lines_ = std::move(lines); pages_ = std::move(pages);
    seek(anchor_);
}
bool Reader::next() {
    if (page_ + 1 == pages_.size()) return false;
    anchor_ = pages_[++page_].begin; return true;
}
bool Reader::previous() {
    if (page_ == 0) return false;
    anchor_ = pages_[--page_].begin; return true;
}
void Reader::seek(std::size_t offset) {
    if (offset > text_.size() || (offset < text_.size() &&
        (static_cast<unsigned char>(text_[offset]) & 0xc0) == 0x80))
        throw std::invalid_argument("reading offset is not a UTF-8 boundary");
    const auto p = std::upper_bound(pages_.begin(), pages_.end(), offset,
        [](std::size_t value, const Page &page) { return value < page.begin; });
    page_ = p == pages_.begin() ? 0 : static_cast<std::size_t>(p - pages_.begin() - 1);
    anchor_ = offset;
}
void Reader::reflow(Layout layout) { paginate(layout); }
Position Reader::position() const { return {fingerprint_, text_.size(), anchor_}; }
void Reader::restore(Position p) {
    if (p.fingerprint != fingerprint_ || p.byte_length != text_.size() || p.byte_offset > text_.size())
        throw std::invalid_argument("position belongs to different content");
    seek(static_cast<std::size_t>(p.byte_offset));
}
std::string Reader::encode_position() const {
    std::ostringstream s;
    s << "SRTXT1 " << fingerprint_ << ' ' << text_.size() << ' ' << anchor_;
    auto body = s.str(); return body + " " + std::to_string(hash(body));
}
void Reader::restore_record(std::string_view record) {
    const auto last = record.rfind(' ');
    if (last == record.npos) throw std::invalid_argument("bad progress record");
    std::istringstream s{std::string(record)};
    std::string magic, extra; Position p{}; std::uint64_t checksum{};
    if (!(s >> magic >> p.fingerprint >> p.byte_length >> p.byte_offset >> checksum) ||
        (s >> extra) || magic != "SRTXT1" || checksum != hash(record.substr(0, last)))
        throw std::invalid_argument("bad progress version/checksum");
    restore(p);
}
void draw_page(const Reader &reader, Rect area, const Measure &measure, const DrawGlyph &draw) {
    const auto l = reader.layout();
    if (!measure || !draw || area.x < 0 || area.y < 0 || area.x > 1000000 || area.y > 1000000 ||
        area.width != l.width || area.height != l.height)
        throw std::invalid_argument("drawing viewport differs from pagination");
    const auto &p = reader.pages().at(reader.page_index());
    for (std::size_t n = 0; n < p.line_count; ++n) {
        const auto &line = reader.lines()[p.first_line + n];
        int x = area.x;
        for (auto c : decode_utf8(std::string_view(reader.text()).substr(line.begin, line.end - line.begin))) {
            auto advance = measure(c.value, l.font_size);
            if (advance < 0 || advance > 1000000) throw std::invalid_argument("invalid draw advance");
            // Backend must clip glyph ink to area; y is line top, not font baseline.
            draw(c.value, x, area.y + static_cast<int>(n) * l.line_height, l.font_size);
            x += advance;
        }
    }
}
ReadingCommand official_command(int c) {
    switch (c) {
    case 0x8: return ReadingCommand::previous;
    case 0x9: case 0xA: return ReadingCommand::next;
    case 0x5: case 0xD: case 0xE: return ReadingCommand::menu;
    default: return ReadingCommand::none;
    }
}
void save_position_file(const Reader &reader, const std::string &path) {
    // Caller serializes writes to a document-specific path. No private text in record.
    const auto temp = path + ".tmp";
    { std::ofstream f(temp, std::ios::binary | std::ios::trunc);
      f << reader.encode_position(); f.flush();
      if (!f) throw std::runtime_error("progress write failed"); }
    if (std::rename(temp.c_str(), path.c_str()) != 0) throw std::runtime_error("progress rename failed");
}
void restore_position_file(Reader &reader, const std::string &path) { reader.restore_record(read_file(path, 256)); }
} // namespace solreader::txt

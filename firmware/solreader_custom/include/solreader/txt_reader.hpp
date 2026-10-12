#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace solreader::txt {
struct Character { char32_t value; std::size_t begin, end; };
// Strict rejection: no replacement, normalization or partially accepted document.
std::vector<Character> decode_utf8(std::string_view text);
std::string read_file(const std::string &path, std::size_t limit = 16 * 1024 * 1024);
using Measure = std::function<int(char32_t, int)>;
struct Layout { int width, height, line_height, font_size; };
struct Line {
    std::size_t source_begin, begin, end, source_end; // Original interval vs visible text (BOM/newlines hidden).
    int width;
    bool hard_break, prohibited_start, oversized_glyph;
};
struct Page { std::size_t first_line, line_count, begin, end; };
struct Position { std::uint64_t fingerprint, byte_length, byte_offset; };
class Reader {
public:
    Reader(std::string text, Layout layout, Measure measure);
    const std::string &text() const { return text_; }
    const std::vector<Line> &lines() const { return lines_; }
    const std::vector<Page> &pages() const { return pages_; }
    std::size_t page_index() const { return page_; }
    const Layout &layout() const { return layout_; }
    bool next();
    bool previous();
    void seek(std::size_t byte_offset); // Valid UTF-8 boundary required.
    void reflow(Layout layout); // Keeps original byte anchor, not obsolete page index.
    Position position() const;
    void restore(Position position); // Reject changed content and invalid offset.
    std::string encode_position() const; // Versioned, checksummed portable record.
    void restore_record(std::string_view record);
private:
    void paginate(Layout layout); // Commit only when measurement/layout succeeds.
    std::string text_;
    std::vector<Character> chars_;
    Measure measure_;
    Layout layout_;
    std::vector<Line> lines_;
    std::vector<Page> pages_;
    std::uint64_t fingerprint_{};
    std::size_t page_{}, anchor_{};
};
// Backend adapter: caller supplies a real font's measurement/drawing and storage.
struct Rect { int x, y, width, height; };
using DrawGlyph = std::function<void(char32_t, int, int, int)>;
void draw_page(const Reader &reader, Rect area, const Measure &measure, const DrawGlyph &draw);
enum class ReadingCommand { previous, next, menu, none };
ReadingCommand official_command(int remote_command);
// Atomic rename on a host filesystem; no claim of power-loss durability on NVS/LittleFS.
void save_position_file(const Reader &reader, const std::string &path);
void restore_position_file(Reader &reader, const std::string &path);
} // namespace solreader::txt

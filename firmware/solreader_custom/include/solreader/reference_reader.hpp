#pragma once
#include "solreader/txt_reader.hpp"
#include <memory>

namespace solreader::reference {
// Public glue only: algorithm implementation remains in the private reference tree.
struct ByteSource {
    std::function<std::size_t(std::size_t, char *, std::size_t)> read;
    std::size_t size;
    std::string document_id;
};
struct Line { std::string utf8; std::size_t begin, end, source_end; bool prohibited_start; };
struct Page { std::size_t begin, end; std::vector<Line> lines; };
class Reader {
public:
    Reader(ByteSource source, txt::Layout layout, txt::Measure measure);
    ~Reader();
    Reader(Reader &&) noexcept;
    Reader &operator=(Reader &&) noexcept;
    Reader(const Reader &) = delete;
    Reader &operator=(const Reader &) = delete;
    Page page() const;
    bool next(); // Clamped, never wraps EOF to first page.
    bool previous(); // May start a cooperative recovery job.
    bool seek(std::size_t offset); // May start a cooperative job.
    bool busy() const;
    bool step(); // Call from UI/task queue, never bypass platform sleep lifecycle.
    void reflow(txt::Layout layout);
    std::size_t offset() const;
    std::size_t cached_pages() const;
    std::string save() const; // Reuses audited reference CRC-protected progress codec.
    bool restore(std::string_view record);
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace solreader::reference

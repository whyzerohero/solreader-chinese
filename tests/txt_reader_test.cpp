#include "solreader/txt_reader.hpp"
#include <functional>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <cstdio>
using namespace solreader::txt;
namespace {
void check(bool ok) { if (!ok) throw std::runtime_error("assertion failed"); }
template<class F> void rejects(F f) { bool rejected = false; try { f(); } catch (const std::exception &) { rejected = true; } check(rejected); }
int measure(char32_t c, int size) { return c < 128 || c == U'，' || c == U'。' ? size / 2 : size; }
Layout box(int w = 40, int size = 10) { return {w, 40, size, size}; }
void integrity(const Reader &r) {
    std::string reconstructed; std::size_t last = 0;
    for (const auto &l : r.lines()) {
        check(l.source_begin == last && l.source_begin <= l.begin && l.begin <= l.end && l.end <= l.source_end);
        auto view = std::string_view(r.text()).substr(l.begin, l.end - l.begin);
        for (auto c : decode_utf8(view)) check(c.value != U'\r' && c.value != U'\n');
        check(l.width <= r.layout().width || l.oversized_glyph);
        reconstructed += r.text().substr(l.source_begin, l.source_end - l.source_begin);
        last = l.source_end;
    }
    check(last == r.text().size() && reconstructed == r.text());
    std::string pages; last = 0;
    for (auto p : r.pages()) { check(p.begin == last && p.end >= p.begin); pages += r.text().substr(p.begin,p.end-p.begin); last=p.end; }
    check(pages == r.text());
}
std::string visible(const Reader &r, std::size_t n) { auto l=r.lines().at(n); return r.text().substr(l.begin,l.end-l.begin); }
}
int main(int argc, char **argv) {
    const std::string temp = argc > 1 ? argv[1] : "/tmp/solreader-txt-test";
    const std::vector<std::pair<std::string, std::function<void()>>> tests = {
        {"Chinese paragraph", [] { Reader r(u8"春天到了，窗外的树叶轻轻摇动。我们开始阅读中文。",box(),measure); integrity(r); check(r.lines().size()>1); }},
        {"mixed Chinese English numbers", [] { Reader r(u8"中文 English 2026：版本 v1.6.0，price $12.50!",box(),measure); integrity(r); }},
        {"punctuation fits actual advance", [] { Reader r(u8"甲乙丙，丁",box(35),measure); check(visible(r,0)==u8"甲乙丙，"); check(!r.lines()[1].prohibited_start); integrity(r); }},
        {"strict-width fallback no backwards Han move", [] { Reader r(u8"甲乙丙丁，戊",box(40),measure); check(visible(r,0)==u8"甲乙丙丁"); check(visible(r,1)==u8"，戊"); check(r.lines()[1].prohibited_start); integrity(r); }},
        {"closing punctuation chain", [] { Reader r(u8"甲乙，。！？；：、）》】」』尾",box(10),measure); integrity(r); check(r.lines()[2].prohibited_start); }},
        {"consecutive blank lines", [] { Reader r(u8"甲\n\n\n乙\n",box(),measure); check(r.lines().size()==5); check(visible(r,1).empty()); integrity(r); }},
        {"CR LF CRLF map original bytes", [] { Reader r(u8"甲\r\n乙\r丙\n",box(),measure); check(r.lines().size()==4); integrity(r); }},
        {"oversized glyph narrow viewport", [] { Reader r(u8"甲乙",box(1),measure); check(r.lines().size()==2); check(r.lines()[0].oversized_glyph); integrity(r); }},
        {"empty document", [] { Reader r("",box(),measure); check(r.pages().size()==1); check(!r.next()&&!r.previous()); integrity(r); }},
        {"one character", [] { Reader r(u8"中",box(),measure); check(r.lines().size()==1); integrity(r); }},
        {"UTF8 BOM mapped not rendered", [] { Reader r("\xef\xbb\xbf" u8"中文",box(),measure); check(visible(r,0)==u8"中文"); integrity(r); Reader b("\xef\xbb\xbf",box(),measure); integrity(b); }},
        {"UTF8 four-byte scalars", [] { Reader r(u8"𠀀😀中文",box(),measure); check(decode_utf8(r.text()).size()==4); integrity(r); }},
        {"invalid UTF8 rejected", [] { for (auto s : {std::string("\xc0\xaf"),std::string("\xe0\x80\xaf"),std::string("\xed\xa0\x80"),std::string("\xf4\x90\x80\x80"),std::string("\x80"),std::string("\xe4\xb8"),std::string("\xe4\x41\xad"),std::string("\xf5\x80\x80\x80"),std::string("\0",1)}) rejects([&] { Reader r(s,box(),measure); }); }},
        {"page navigation and boundaries", [] { Reader r(u8"甲乙丙丁戊己庚辛壬癸甲乙",{20,20,10,10},measure); check(!r.previous()); auto count=r.pages().size(); for(std::size_t i=1;i<count;++i)check(r.next()); check(!r.next()); for(std::size_t i=1;i<count;++i)check(r.previous()); check(!r.previous()); }},
        {"progress restore and changed content reject", [] { Reader r(u8"甲乙丙丁戊己庚辛",{20,10,10,10},measure); r.next(); auto p=r.position(); Reader other(r.text(),r.layout(),measure); other.restore(p); check(other.page_index()==r.page_index()); Reader changed(u8"甲乙丙丁戊己庚壬",r.layout(),measure); rejects([&]{changed.restore(p);}); }},
        {"font-size reflow keeps byte anchor", [] { Reader r(u8"甲乙丙丁戊己庚辛壬癸",{40,20,10,10},measure); r.seek(18); auto anchor=r.position().byte_offset; r.reflow({40,20,20,20}); check(r.position().byte_offset==anchor); check(r.pages()[r.page_index()].begin<=anchor); integrity(r); }},
        {"width reflow", [] { Reader r(u8"中文阅读 mixed text 123。",box(),measure); for(int width:{1,5,10,19,35,40,196,256}) {r.reflow(box(width)); integrity(r);} }},
        {"invalid UTF8 position reject", [] { Reader r(u8"甲乙",box(),measure); rejects([&]{r.seek(1);}); rejects([&]{r.seek(7);}); r.seek(6); check(r.position().byte_offset==6); }},
        {"progress record corruption", [] { Reader r(u8"中文",box(),measure); auto s=r.encode_position(); r.restore_record(s); s[0]='X'; rejects([&]{r.restore_record(s);}); rejects([&]{r.restore_record("SRTXT1 0 0 0 0");}); }},
        {"file TXT and progress roundtrip", [&] { Reader r(u8"甲乙丙丁戊己",{10,10,10,10},measure); r.next(); save_position_file(r,temp); Reader other(r.text(),r.layout(),measure); restore_position_file(other,temp); check(other.page_index()==r.page_index()); check(read_file(temp)==r.encode_position()); rejects([&]{read_file(temp,1);}); std::remove(temp.c_str()); }},
        {"invalid dimensions and metrics", [] { rejects([]{Reader r("a",{0,1,1,10},measure);}); rejects([]{Reader r("a",{10,1,2,10},measure);}); Reader r("abc",box(),measure); auto old=r.lines().size(); rejects([&]{r.reflow({0,0,0,0});}); check(r.lines().size()==old); rejects([]{Reader r("a",box(),[](char32_t,int){return -1;});}); }},
        {"zero-advance combining characters terminate", [] { Reader r(u8"á́́b",box(),[](char32_t c,int){return c==0x301?0:10;}); integrity(r); }},
        {"official reading command mapping", [] {check(official_command(8)==ReadingCommand::previous); for(int c:{9,10})check(official_command(c)==ReadingCommand::next); for(int c:{5,13,14})check(official_command(c)==ReadingCommand::menu); check(official_command(15)==ReadingCommand::none); }},
        {"page draw positions and viewport", [] { Reader r(u8"甲a乙\n丙",box(),measure); std::u32string seen; draw_page(r,{30,20,40,40},measure,[&](char32_t c,int x,int y,int size){check(x>=30&&x<=70&&y>=20&&y<60&&size==10);seen+=c;}); check(seen==U"甲a乙丙"); rejects([&]{draw_page(r,{0,0,39,40},measure,[](char32_t,int,int,int){});}); }},
        {"long TXT and superlong paragraph", [] { std::string s; for(int i=0;i<20000;++i)s+=u8"长篇中文TXT，mixed 123。"; Reader r(s,{196,216,20,16},measure); integrity(r); check(r.pages().size()>100); r.seek(s.size()); check(!r.next()); }},
        {"random layouts source integrity", [] { std::mt19937 rng(20261012); const std::vector<std::string> chars={u8"中",u8"文",u8"，",u8"。",u8"😀","a"," ","\n","\r\n"}; for(int t=0;t<300;++t){std::string s;for(int i=0;i<100;++i)s+=chars[rng()%chars.size()];Reader r(s,{static_cast<int>(1+rng()%256),40,10,10},measure);integrity(r);} }},
    };
    int failed=0;
    for(auto &test:tests) { try {test.second();std::cout<<"PASS "<<test.first<<'\n';}catch(const std::exception &e){++failed;std::cerr<<"FAIL "<<test.first<<": "<<e.what()<<'\n';} }
    std::cout<<tests.size()-failed<<" passed, "<<failed<<" failed\n"; return failed?1:0;
}

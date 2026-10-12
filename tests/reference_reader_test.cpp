#include "solreader/reference_reader.hpp"
#include "solreader/reading_controller.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
using namespace solreader;
namespace {
void check(bool b) { if(!b)throw std::runtime_error("assertion failed"); }
template<class F>void rejects(F f) { bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}check(rejected); }
int metrics(char32_t c,int size) {return c<128||c==U'，'||c==U'。'?size/2:size;}
txt::Layout box(int width=40,int size=10) {return {width,40,size,size};}
reference::ByteSource source(const std::string &s,std::size_t *count=nullptr,std::size_t chunk=512) {
 return {[&s,count,chunk](std::size_t at,char*out,std::size_t n){n=std::min({n,s.size()-at,chunk});std::memcpy(out,s.data()+at,n);if(count)*count+=n;return n;},s.size(),"synthetic.txt"};
}
void finish(reference::Reader &r) {unsigned steps=0;while(r.busy()){r.step();check(++steps<100000);} }
void integrity(reference::Reader &r,const std::string&s) {
 std::size_t at=r.offset();std::string actual=s.substr(0,at);
 do {
  auto p=r.page();check(p.begin==at&&p.end>=p.begin&&p.end<=s.size());
  auto line_at=p.begin;
  for(auto &l:p.lines){check(l.begin==line_at&&l.end>=l.begin&&l.source_end>=l.end);check(s.substr(l.begin,l.end-l.begin)==l.utf8);line_at=l.source_end;}
  check(line_at==p.end);actual+=s.substr(p.begin,p.end-p.begin);at=p.end;
  check(r.cached_pages()<=32);
 }while(r.next());
 check(at==s.size()&&actual==s);
}
}
int main(){
 const std::vector<std::pair<std::string,std::function<void()>>> tests={
 {"Chinese/mixed/number paragraph integrity",[]{std::string s=u8"中文阅读 English 2026：测试，。！？123 $5.\n第二段。";reference::Reader r(source(s),box(),metrics);integrity(r,s);}},
 {"leading trailing and fullwidth whitespace retained",[]{std::string s=u8"  甲乙　\t\n \n　丙 \t";reference::Reader r(source(s),box(20),metrics);integrity(r,s);}},
 {"no paragraph merging before punctuation",[]{std::string s=u8"甲\n\n，乙\r\n。丙";reference::Reader r(source(s),box(),metrics);auto p=r.page();check(p.lines.size()==4);check(p.lines[0].utf8==u8"甲");check(p.lines[1].utf8.empty());check(p.lines[2].utf8==u8"，乙");integrity(r,s);}},
 {"measured punctuation no deleted or moved Han",[]{std::string s=u8"甲乙丙，丁";reference::Reader r(source(s),box(35),metrics);check(r.page().lines[0].utf8==u8"甲乙丙，");std::string t=u8"甲乙丙丁，戊";reference::Reader q(source(t),box(),metrics);check(q.page().lines[0].utf8==u8"甲乙丙丁");check(q.page().lines[1].prohibited_start);integrity(q,t);}},
 {"continuous blank lines CR LF CRLF",[]{std::string s=u8"\n\r\n甲\r乙\n\n";reference::Reader r(source(s),box(),metrics);integrity(r,s);}},
 {"empty and short and BOM",[]{for(const std::string &s:{std::string(),std::string(u8"中"),std::string("\xef\xbb\xbf"),std::string("\xef\xbb\xbf" u8"中文")}){reference::Reader r(source(s),box(),metrics);integrity(r,s);}}},
 {"one-byte reads across UTF8 boundaries",[]{std::string s=u8"中文𠀀😀。mixed\r\n第二段";reference::Reader r(source(s,nullptr,1),box(),metrics);integrity(r,s);}},
 {"invalid UTF8 rejects before pagination",[]{for(std::string s:{std::string("\xc0\xaf"),std::string("\xed\xa0\x80"),std::string("\xf4\x90\x80\x80"),std::string("\xe4\xb8"),std::string("\0",1)})rejects([&]{reference::Reader r(source(s),box(),metrics);});}},
 {"reading navigation is clamped",[]{std::string s(1000,'a');reference::Reader r(source(s),box(),metrics);check(!r.previous());for(int i=0;i<5;++i)check(r.next());auto before=r.offset();check(r.previous());finish(r);check(r.offset()<before);integrity(r,s);check(!r.next());}},
 {"progress CRC binding restore",[]{std::string s(1000,'a');reference::Reader r(source(s),box(),metrics);check(r.next());auto record=r.save();auto saved=r.offset();reference::Reader q(source(s),box(),metrics);q.restore(record);finish(q);check(q.offset()==saved);auto bad=record;bad[20]^=1;rejects([&]{q.restore(bad);});std::string changed=s;changed[0]='b';reference::Reader different(source(changed),box(),metrics);rejects([&]{different.restore(record);});}},
 {"font and width reflow recover actual page",[]{std::string s;for(int i=0;i<500;++i)s+=u8"中文阅读，mixed 123。";reference::Reader r(source(s),box(),metrics);for(int i=0;i<40;++i)check(r.next());auto anchor=r.offset();r.reflow({196,216,20,16});if(r.busy()){rejects([&]{r.page();});rejects([&]{r.save();});}finish(r);auto p=r.page();check(p.begin<=anchor&&p.end>anchor);}},
 {"long streaming cache bounded",[]{std::string s;for(int i=0;i<30000;++i)s+=u8"长篇中文 mixed 123，。";std::size_t read=0;reference::Reader r(source(s,&read),{196,216,20,16},metrics);read=0;r.page();check(read<=1024);integrity(r,s);}},
 {"history exhaustion cooperative previous",[]{std::string s(30000,'a');reference::Reader r(source(s),box(),metrics);for(int i=0;i<50;++i)check(r.next());for(int i=0;i<45;++i){auto before=r.offset();bool moved=r.previous();finish(r);check(moved||r.offset()<before);check(r.offset()<before);} }},
 {"cooperative layout parity mixed and paragraphs",[]{std::string s;for(int i=0;i<500;++i)s+=u8"  中文 abc123，\n\n。甲乙\t尾\r\n";reference::Reader r(source(s),box(35),metrics);std::vector<std::size_t> starts;do{starts.push_back(r.offset());r.page();}while(r.next());for(auto target:{starts[45],starts[100],starts[3],starts[70]}){r.seek(target);finish(r);check(r.offset()==target);} }},
 {"UTF8 seek boundary and measurement rejection",[]{std::string s=u8"中文";reference::Reader r(source(s),box(),metrics);rejects([&]{r.seek(1);});rejects([&]{r.seek(7);});rejects([&]{r.reflow({0,0,0,0});});check(r.page().lines[0].utf8==s);reference::Reader bad(source(s),box(),[](char32_t,int){return -1;});rejects([&]{bad.page();});}},
 {"source read errors and excessive count reject",[]{std::string s(1000,'a');auto src=source(s);src.read=[](std::size_t,char*,std::size_t){return 0;};rejects([&]{reference::Reader r(src,box(),metrics);});src.read=[](std::size_t,char*,std::size_t n){return n+1;};rejects([&]{reference::Reader r(src,box(),metrics);});}},
 {"variable widths and zero advance terminate",[]{std::string s=u8"á́中，\n文。abc";for(int w:{1,5,10,19,35,196,256}){reference::Reader r(source(s),box(w),[](char32_t c,int size){return c==0x301?0:metrics(c,size);});integrity(r,s);}}},
 {"bounded zero-advance source runs",[]{std::string s;for(int i=0;i<50000;++i)s+=u8"́";reference::Reader r(source(s),box(),[](char32_t,int){return 0;});for(const auto &line:r.page().lines)check(txt::decode_utf8(line.utf8).size()<=4096);integrity(r,s);r.seek(65536);finish(r);check(r.offset()==65536);}},
 {"factory remote controller reading-only route",[]{std::string s(1000,'a');reference::Reader r(source(s),box(),metrics);int menus=0;ReadingController c(r,[&]{++menus;});InputEvent event;event.action=InputAction::right;c.on_input(event);check(r.offset()>0&&c.take_redraw());event.action=InputAction::left;c.on_input(event);check(r.offset()==0&&c.take_redraw());for(auto action:{InputAction::up,InputAction::down,InputAction::back}){event.action=action;c.on_input(event);}check(menus==3);event.action=InputAction::none;c.on_input(event);check(!c.take_redraw());}},
 {"random widths source mapping",[]{std::mt19937 rng(20261012);std::vector<std::string> chars={u8"中",u8"，",u8"。",u8"😀","a"," ","\n","\r\n"};for(int j=0;j<100;++j){std::string s;for(int k=0;k<100;++k)s+=chars[rng()%chars.size()];reference::Reader r(source(s),box(static_cast<int>(1+rng()%196)),metrics);integrity(r,s);}}}
 };
 int failed=0;for(const auto &t:tests){try{t.second();std::cout<<"PASS "<<t.first<<'\n';}catch(const std::exception&e){++failed;std::cerr<<"FAIL "<<t.first<<": "<<e.what()<<'\n';}}
 std::cout<<tests.size()-failed<<" passed, "<<failed<<" failed\n";return failed?1:0;
}

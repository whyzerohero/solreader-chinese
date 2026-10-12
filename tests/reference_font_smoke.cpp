#include "solreader/reference_reader.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <cstring>
int main(int argc,char**argv) {
 try {
  if(argc!=4)throw std::invalid_argument("usage: font-smoke TXT METRICS LINE_HEIGHT");
  const auto text=solreader::txt::read_file(argv[1]);
  std::ifstream file(argv[2]);int size{};file>>size;
  std::map<char32_t,int> widths;unsigned cp;int width;
  while(file>>cp>>width)widths[static_cast<char32_t>(cp)]=width;
  if(!file.eof()||size<=0)throw std::runtime_error("invalid actual font metrics");
  auto measure=[&](char32_t c,int requested){if(requested!=size)throw std::runtime_error("font size mismatch");return widths.at(c);};
  solreader::reference::ByteSource src{[&](std::size_t at,char*out,std::size_t n){n=std::min(n,text.size()-at);std::memcpy(out,text.data()+at,n);return n;},text.size(),"font-smoke.txt"};
  solreader::reference::Reader r(src,{196,216,std::stoi(argv[3]),size},measure);
  int y=0;auto p=r.page();std::size_t count=0;
  for(auto &line:p.lines){int x=0;for(auto c:solreader::txt::decode_utf8(line.utf8)){std::cout<<static_cast<unsigned>(c.value)<<'\t'<<x<<'\t'<<y<<'\n';x+=measure(c.value,size);++count;} y+=std::stoi(argv[3]);}
  if(!count)throw std::runtime_error("no glyphs executed");
  std::cerr<<"Actual-font layout: "<<count<<" glyphs, "<<p.lines.size()<<" lines\n";
  return 0;
 }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
}

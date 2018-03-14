#include "driver.hh"
#include "parser.hh"

lre_driver::lre_driver():trace_scanning(false),trace_parsing(false){
}

lre_driver::~lre_driver(){
}

int lre_driver::parse(const std::string &f){
  file = f;
  scan_begin();
  yy::lre_parser parser(*this);
  parser.set_debug_level(trace_parsing);
  int res = parser.parse();
  scan_end();
  return res;
}

void lre_driver::error(const yy::location& l, const std::string& m){
  std::cerr<<l<<": "<<m<<std::endl;
}

void lre_driver::error(const std::string& m){
  std::cerr<<m<<std::endl;
}

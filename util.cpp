#include <fstream>
#include <sstream>
#include <string>

#include "util.h"

std::string UTIL::get_whole_file(std::string path) {
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

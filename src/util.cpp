#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "astnode.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"

using std::string;
using std::vector;

string UTIL::get_whole_file(string path) {
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

vector<string> UTIL::split_argv(string argv_raw) {
  /*
    Just splits on space right now.
    Later, could add logic for quotes, other whitespace, substitutions, etc.

    Also unicode is probably totally unhandled here.
  */

  vector<string> argv;
  std::stringstream current_string;
  bool scratch = false;

  for (size_t i = 0; i < argv_raw.size(); ++i) {
    if (argv_raw[i] == ' ') {
      argv.push_back(current_string.str());
      current_string = std::stringstream{""};
      scratch = false;
    } else {
      current_string << argv_raw[i];
      scratch = true;
    }
  }

  // add last string in case argv_raw doesn't end in a space.
  // (this scratch thing is a total hack but works for now)
  if (scratch) {
    argv.push_back(current_string.str());
  }

  return argv;
}

string UTIL::get_file_path_directory(const string &fname) {
  size_t pos = fname.find_last_of("\\/");
  return (std::string::npos == pos) ? "" : fname.substr(0, pos);
}

ASTNode UTIL::load_module(string path) {
  return parse_tokens(lex_string(get_whole_file(path)));
}
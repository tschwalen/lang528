#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "astnode.h"

using std::string;

const string STRING_TRUE = "true";
const string STRING_FALSE = "false";
const string STRING_NOTHING = "nothing";

extern string RUNTIME_DIR_PATH;
extern std::ostream *EMIT_TARGET;

namespace UTIL {
std::string get_whole_file(std::string path);
std::vector<std::string> split_argv(std::string argv_raw);
ASTNode load_module(std::string path);
std::string get_file_path_directory(const std::string &fname);
} // namespace UTIL
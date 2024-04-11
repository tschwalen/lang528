#pragma once

#include <vector>
#include <string>

#include "astnode.h"

namespace UTIL {
std::string get_whole_file(std::string path);
std::vector<std::string> split_argv(std::string argv_raw);
ASTNode load_module(std::string path);
std::string get_file_path_directory(const std::string& fname);
}
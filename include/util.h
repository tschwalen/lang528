#pragma once

#include <vector>
#include <string>

namespace UTIL {
std::string get_whole_file(std::string path);
std::vector<std::string> split_argv(std::string argv_raw);
}
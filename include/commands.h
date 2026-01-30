#pragma once
#include <string>

using std::string;

namespace Commands {
int test();
int lex(string input_file_path, bool dump_json);
int parse(string input_file_path, bool dump_json);
int interpret(string input_file_path, string program_args);
int compile(string input_file_path);
int compile_end_to_end(string input_file_path, string output_file_path);
} // namespace Commands
#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "lexer.h"
#include "parser.h"
#include "token.h"
#include "tokentype.h"
#include "util.h"
#include "unittests.h"

using json = nlohmann::json;

using std::string;
using std::vector;

struct Options {
  bool test;
  bool dump_json;
  bool lex;
  bool parse;
  string input_file_path;
};

Options handle_commandline_args(int argc, char **argv) {
  vector<string> args(argv + 1, argv + argc);

  string input_file_path_option = "--input=";

  // TODO: refactor this if/as options grow. for now this works fine
  Options options{false, false, false, false, ""};
  for (auto &string_argument : args) {
    if (string_argument == "--test") {
      options.test = true;
    }
    if (string_argument == "--dump-json") {
      options.dump_json = true;
    }
    if (string_argument == "--lex") {
      options.lex = true;
    }
    if (string_argument == "--parse") {
      options.parse = true;
    }
    if (string_argument.rfind(input_file_path_option) == 0) {
      options.input_file_path =
          string_argument.substr(input_file_path_option.size());
    }
  }
  return options;
}

int main(int argc, char **argv) {
  auto opts = handle_commandline_args(argc, argv);

  // TEST ENTRYPOINT
  if (opts.test) {
    // run tests
    std::cout << "test option passed \n";
    TESTS::run_all_unittests();
    return 0;
  }

  // LEXER ENTRYPOINT
  if (opts.lex && !opts.input_file_path.empty()) {
    auto file_contents = UTIL::get_whole_file(opts.input_file_path);
    // std::cout << file_contents;
    auto tokens = lex_string(file_contents);

    if (opts.dump_json) {
      json j = tokens;
      std::cout << j.dump(2) << "\n";
    }

    return 0;
  }

  // PARSER ENTRYPOINT
  if (opts.parse && !opts.input_file_path.empty()) {
    auto file_contents = UTIL::get_whole_file(opts.input_file_path);
    auto tokens = lex_string(file_contents);
    auto ast = parse_tokens(tokens);
    // ... then parse them ... 
  }

  return 0;
}
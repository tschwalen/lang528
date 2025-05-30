#include <iostream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "codegen.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "unittests.h"
#include "util.h"

using json = nlohmann::json;

using std::string;
using std::vector;

struct Options {
  bool test;
  bool dump_json;
  bool lex;
  bool parse;
  bool exec;
  bool comp;
  string input_file_path;
  string program_args;
};

Options handle_commandline_args(int argc, char **argv) {
  vector<string> args(argv + 1, argv + argc);

  string input_file_path_option = "--input=";
  string program_args_option = "--argv=";

  // TODO: refactor this if/as options grow. for now this works fine
  Options options{false, false, false, false, false, false, "", ""};
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
    if (string_argument == "--exec") {
      options.exec = true;
    }
    if (string_argument == "--comp") {
      options.comp = true;
    }
    if (string_argument.rfind(input_file_path_option) == 0) {
      options.input_file_path =
          string_argument.substr(input_file_path_option.size());
    }
    if (string_argument.rfind(program_args_option) == 0) {
      options.program_args = string_argument.substr(program_args_option.size());
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

    if (opts.dump_json) {
      json j = ast;
      std::cout << j.dump(2) << "\n";
    }

    return 0;
  }

  // INTERPRETER ENTRYPOINT
  if (opts.exec && !opts.input_file_path.empty()) {
    auto file_path = opts.input_file_path;
    auto file_contents = UTIL::get_whole_file(file_path);
    auto tokens = lex_string(file_contents);
    auto ast = parse_tokens(tokens);

    // get CWD relative to source file we're running
    auto module_wd = UTIL::get_file_path_directory(file_path);

    // split program argv
    vector<string> program_argv{};
    if (opts.program_args.size() > 0) {
      program_argv = UTIL::split_argv(opts.program_args);
    }

    eval_top_level(ast, module_wd, program_argv);
    return 0;
  }

  if (opts.comp && !opts.input_file_path.empty()) {
    auto file_path = opts.input_file_path;
    auto file_contents = UTIL::get_whole_file(file_path);
    auto tokens = lex_string(file_contents);
    auto ast = parse_tokens(tokens);

    gen_node(ast);
    return 0;
  }

  return 0;
}
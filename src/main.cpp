#include <filesystem>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>
#include <vector>

#include <nlohmann/json.hpp>

#include "interpreter.h"
#include "parser.h"
#include "util.h"

#include "commands.h"

using json = nlohmann::json;

using std::string;
using std::vector;

std::ostream *EMIT_TARGET = &std::cout;
string RUNTIME_DIR_PATH;

struct Options {
  bool test;
  bool dump_json;
  bool lex;
  bool parse;
  bool exec;
  bool comp;
  bool comp_e2e;
  string input_file_path;
  string output_file_path;
  string program_args;
};

void init() {
  // TODO: we need some way for this to be configured.
  // This works now where we're building and running locally, but if the
  // compiler were ever shipped, then the runtime would need to go in some
  // standard/expected install location (e.g. /opt, /var, or whatever
  // Windowsism)
  auto file_dir = std::filesystem::path{__FILE__};
  RUNTIME_DIR_PATH = file_dir.parent_path().parent_path().append("runtime");
}

Options handle_commandline_args(int argc, char **argv) {
  vector<string> args(argv + 1, argv + argc);

  string input_file_path_option = "--input=";
  string output_file_path_option = "--output=";
  string program_args_option = "--argv=";

  Options options{false, false, false, false, false, false, false, "", "", ""};
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
    if (string_argument == "--comp-e2e") {
      options.comp_e2e = true;
    }
    if (string_argument.rfind(input_file_path_option) == 0) {
      options.input_file_path =
          string_argument.substr(input_file_path_option.size());
    }
    if (string_argument.rfind(output_file_path_option) == 0) {
      options.output_file_path =
          string_argument.substr(output_file_path_option.size());
    }
    if (string_argument.rfind(program_args_option) == 0) {
      options.program_args = string_argument.substr(program_args_option.size());
    }
  }
  return options;
}

int main(int argc, char **argv) {
  init();
  auto opts = handle_commandline_args(argc, argv);

  // TEST ENTRYPOINT
  if (opts.test) {
    return Commands::test();
  }

  // LEXER ENTRYPOINT
  if (opts.lex && !opts.input_file_path.empty()) {
    return Commands::lex(opts.input_file_path, opts.dump_json);
  }

  // PARSER ENTRYPOINT
  if (opts.parse && !opts.input_file_path.empty()) {
    return Commands::parse(opts.input_file_path, opts.dump_json);
  }

  // INTERPRETER ENTRYPOINT
  if (opts.exec && !opts.input_file_path.empty()) {
    return Commands::interpret(opts.input_file_path, opts.program_args);
  }

  // COMPILER ENTRYPOINT
  if (opts.comp && !opts.input_file_path.empty()) {
    return Commands::compile(opts.input_file_path);
  }

  // COMPILER E2E ENTRYPOINT
  if (opts.comp_e2e && !opts.input_file_path.empty() &&
      !opts.output_file_path.empty()) {
    return Commands::compile_end_to_end(opts.input_file_path,
                                        opts.output_file_path);
  }

  std::cerr << "Invalid argument combination passed.\n";
  return 1;
}
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <unistd.h>
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

std::ostream *EMIT_TARGET = &std::cout;

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

  // TODO: refactor this if/as options grow. for now this works fine
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

void compile_to_file(Options opts) {
  // read, lex, and parse the input file
  auto input_file_path = opts.input_file_path;
  auto file_contents = UTIL::get_whole_file(input_file_path);
  auto tokens = lex_string(file_contents);
  auto ast = parse_tokens(tokens);
  auto module_wd = UTIL::get_file_path_directory(input_file_path);

  // open file stream to file where the C codegen goes to
  auto cwd = std::filesystem::current_path();
  auto work_dir = cwd / ".l528work";
  std::filesystem::create_directory(work_dir);
  auto codegen_target_file_path = work_dir / "prog.c";
  std::ofstream codegen_target_file(codegen_target_file_path);
  if (!codegen_target_file) {
    throw std::runtime_error("failed to codegen target file");
  }

  // run the compile step, spilling the results into the codegen target file
  auto emit_target_old = EMIT_TARGET;
  EMIT_TARGET = &codegen_target_file;
  gen_node_root(ast, module_wd);
  codegen_target_file.close();
  EMIT_TARGET = emit_target_old;

  // get the paths to the runtime libraries and the output file
  auto runtime_library_path = std::filesystem::path{RUNTIME_DIR_PATH};
  auto runtime_include_path = runtime_library_path / "include";
  auto output_file_path = std::filesystem::path{opts.output_file_path};

  // run the external compilation step (compiling the generated C program and
  // linking against the runtime)
  vector<string> subprocess_args = {"cc",
                                    "-O2",
                                    "-o",
                                    output_file_path.string(),
                                    codegen_target_file_path.string(),
                                    "-I" + runtime_include_path.string(),
                                    "-L" + runtime_library_path.string(),
                                    "-lruntime"};

  std::vector<char *> argv;
  for (auto &s : subprocess_args)
    argv.push_back(s.data());
  argv.push_back(nullptr);

  pid_t pid = fork();
  if (pid < 0) {
    throw std::runtime_error("fork failed");
  }

  if (pid == 0) {
    // Child process
    execvp(argv[0], argv.data());

    // Only reached if exec fails
    perror("execvp");
    _exit(127);
  }

  // Parent process
  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    throw std::runtime_error("waitpid failed");
  }

  // Then, handle cleanup
  try {
    std::uintmax_t count = std::filesystem::remove_all(work_dir);
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
}

int main(int argc, char **argv) {
  init();
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

  // COMPILER ENTRYPOINT
  if (opts.comp && !opts.input_file_path.empty()) {
    auto file_path = opts.input_file_path;
    auto file_contents = UTIL::get_whole_file(file_path);
    auto tokens = lex_string(file_contents);
    auto ast = parse_tokens(tokens);
    auto module_wd = UTIL::get_file_path_directory(file_path);
    gen_node_root(ast, module_wd);
    return 0;
  }

  // COMPILER E2E ENTRYPOINT
  if (opts.comp_e2e && !opts.input_file_path.empty() &&
      !opts.output_file_path.empty()) {
    compile_to_file(opts);
    return 0;
  }

  std::cerr << "Invalid argument combination passed.\n";
  return 1;
}
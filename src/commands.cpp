#include "commands.h"
#include "codegen.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "unittests.h"
#include "util.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <string>
#include <unistd.h>
#include <vector>

using std::string;
using json = nlohmann::json;

namespace Commands {

int test() {
  // run tests
  std::cout << "test option passed \n";
  TESTS::run_all_unittests();
  return 0;
}

int lex(string input_file_path, bool dump_json) {
  auto file_contents = UTIL::get_whole_file(input_file_path);
  auto tokens = lex_string(file_contents);

  if (dump_json) {
    json j = tokens;
    std::cout << j.dump(2) << "\n";
  }

  return 0;
}

int parse(string input_file_path, bool dump_json) {
  auto file_contents = UTIL::get_whole_file(input_file_path);
  auto tokens = lex_string(file_contents);
  auto ast = parse_tokens(tokens);
  // ... then parse them ...

  if (dump_json) {
    json j = ast;
    std::cout << j.dump(2) << "\n";
  }

  return 0;
}

int interpret(string input_file_path, string program_args) {
  auto file_path = input_file_path;
  auto file_contents = UTIL::get_whole_file(file_path);
  auto tokens = lex_string(file_contents);
  auto ast = parse_tokens(tokens);

  // get CWD relative to source file we're running
  auto module_wd = UTIL::get_file_path_directory(file_path);

  // split program argv
  vector<string> program_argv{};
  if (program_args.size() > 0) {
    program_argv = UTIL::split_argv(program_args);
  }

  eval_top_level(ast, module_wd, program_argv);
  return 0;
}

int compile(string input_file_path) {
  auto file_path = input_file_path;
  auto file_contents = UTIL::get_whole_file(file_path);
  auto tokens = lex_string(file_contents);
  auto ast = parse_tokens(tokens);
  auto module_wd = UTIL::get_file_path_directory(file_path);
  gen_node_root(ast, module_wd);
  return 0;
}

int compile_end_to_end(string input_file_path, string output_file_path) {
  // read, lex, and parse the input file
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
  auto output_file_path_fs = std::filesystem::path{output_file_path};

  // run the external compilation step (compiling the generated C program and
  // linking against the runtime)
  vector<string> subprocess_args = {"cc",
                                    "-O2",
                                    "-o",
                                    output_file_path_fs.string(),
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

  if (WIFEXITED(status)) {
    int exit_code = WEXITSTATUS(status);
    if (exit_code != 0) {
      // cc ran but failed
      std::exit(exit_code);
    }
  } else if (WIFSIGNALED(status)) {
    // cc was killed by a signal (e.g. SIGSEGV)
    int sig = WTERMSIG(status);
    std::fprintf(stderr, "cc terminated by signal %d\n", sig);
    std::exit(128 + sig); // conventional shell mapping
  } else {
    // Other abnormal termination
    std::exit(1);
  }

  // Then, handle cleanup
  try {
    std::uintmax_t count = std::filesystem::remove_all(work_dir);
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
  return 0;
}
} // namespace Commands
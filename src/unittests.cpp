
#include <iostream>
#include <sstream>

#include <nlohmann/json.hpp>

#include "token.h"
#include "tokentype.h"
#include "unittests.h"

using json = nlohmann::json;

/*
Terminal Colors:
         foreground background
black        30         40
red          31         41
green        32         42
yellow       33         43
blue         34         44
magenta      35         45
cyan         36         46
white        37         47
*/

void msg_green(std::string s) {
  std::cout << "\033[1;32m";
  std::cout << s;
  std::cout << "\033[0m\n";
}

void msg_red(std::string s) {
  std::cout << "\033[1;31m";
  std::cout << s;
  std::cout << "\033[0m\n";
}

void test_assert(bool passed, std::string testcase) {
  std::stringstream message;
  message << " - " << testcase;
  if (passed) {
    message << " [✓]";
    msg_green(message.str());
  } else {
    message << " [x]";
    msg_red(message.str());
  }
}

void test_that_token_serializes_to_json_as_expected() {
  Token t1{
      TokenType::STRING_LITERAL,
      "this is the string",
      {44, 15},
  };

  json json_from_t1 = t1;

  json expected_json = {
      {"metadata",
       {
           {"column", 15},
           {"line", 44},
       }},
      {"type_int", (int)TokenType::STRING_LITERAL},
      {"type_string", "STRING_LITERAL"},
      {"value", "this is the string"},
  };

  test_assert(json_from_t1 == expected_json, __func__);
}

void test_that_token_serializes_from_json_as_expected() {

  json json1 = {
      {"metadata",
       {
           {"column", 15},
           {"line", 44},
       }},
      {"type_int", (int)TokenType::STRING_LITERAL},
      {"type_string", "STRING_LITERAL"},
      {"value", "this is the string"},
  };

  Token token_from_json1 = json1;

  // TODO: should define the == operator for Tokens
  bool all_equal =
      token_from_json1.type == TokenType::STRING_LITERAL &&
      std::get<std::string>(token_from_json1.value) == "this is the string" &&
      token_from_json1.metadata.column == 15 &&
      token_from_json1.metadata.line == 44;

  test_assert(all_equal, __func__);
}

void test_token_json_methods() {
  test_that_token_serializes_to_json_as_expected();
  test_that_token_serializes_from_json_as_expected();
}

void TESTS::run_all_unittests() {
  std::cout << "Running all unit tests:\n...\n";
  test_token_json_methods();
}
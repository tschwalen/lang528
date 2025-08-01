#pragma once
#include "astnode.h"
#include "interpreter.h"
#include <cstddef>

struct CompNodeResult {
  std::optional<string> result_loc;
  std::optional<string> accessee_loc; // only used by field access
  bool final_return = false;          // only used by gen_block
  size_t argc = 0;                    // only used by expr_list
  bool ptr_result = false;
};

enum class CompTableEntryType { VAR, CONST, FUNC, BUILTIN };

struct CompTableEntry {
  std::string location;
  CompTableEntryType type;
  std::optional<std::string> metadata;
};

struct CompSymbolTable {
  CompSymbolTable *parent;
  std::unordered_map<string, CompTableEntry> entries;
  int locals = 0;
  int intermediates = 0;
  bool is_module = false;
};

CompNodeResult gen_node_root(ASTNode &node);
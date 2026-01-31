#pragma once
#include "astnode.h"
#include "interpreter.h"
#include <cstddef>

struct CompNodeResult {
  std::optional<string> result_loc;
  std::optional<string> accessee_loc; // used by field access
  size_t argc = 0;                    // used by expr_list
  bool final_return = false;          // used by gen_block
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
  std::optional<CompTableEntry> lookup_symbol(string symbol);
  std::string new_intmdt();
  bool is_toplevel();
};

CompNodeResult gen_node_root(ASTNode &node, string module_wd);
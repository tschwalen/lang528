#pragma once
#include "astnode.h"

struct CompNodeResult {
  std::optional<string> result_loc;
  bool final_return = false; // only used by gen_block
};

enum class CompTableEntryType { VAR, FUNC, BUILTIN };

struct CompTableEntry {
  std::string location;
  CompTableEntryType type;
};

struct CompSymbolTable {
  CompSymbolTable *parent;
  std::unordered_map<string, CompTableEntry> entries;
  int locals = 0;
  int intermediates = 0;
};

CompNodeResult gen_node_root(ASTNode &node);
#pragma once
#include "astnode.h"

struct CompNodeResult {
  std::optional<string> result_loc;
};

CompNodeResult gen_node(ASTNode &node);
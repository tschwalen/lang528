#pragma once

#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <variant>
#include <vector>

#include "astnode.h"

using std::optional;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

class NotImplemented : public std::logic_error {
public:
  NotImplemented() : std::logic_error("Function not yet implemented"){};
};

struct EvalResult;
struct BoxedValue;
struct SymbolTable;

enum class ValueType { LVALUE, RVALUE };

enum class DataType {
  NOTHING,
  BOOL,
  FLOAT,
  INT,
  STRING,
  VECTOR,
  DICT,
  MODULE,
  FUNCTION
};

struct Function {
  string name;
  vector<string> args;
  ASTNode body;

  // some built-in functions need the "this" value, which refers to the actual
  // instance of the object that the function is being called on, e.g. .length()
  // for vec, string, and dict
  shared_ptr<BoxedValue> _this = nullptr;

  // when we call a function from a module context, e.g. module.function(), the
  // module's symbol table needs to be set because the function may refer to
  // symbols that only exist in the module's symbol table.
  //
  // if functions become closures later, this will be unnecessary
  shared_ptr<SymbolTable> module_st = nullptr;
};

struct Module {
  string name;
  shared_ptr<SymbolTable> symbol_table;
};

enum class VarType {
  // TODO: is there any reason to have "FUNCTION" here? We already have that in
  // BoxedValue
  // Consider removing...
  CONST,
  VAR,
  FUNCTION
};

struct SymbolTableEntry {
  VarType type;
  shared_ptr<BoxedValue> value;
};

struct SymbolTable {
  // this is fine, I guess.
  // But if closures are ever implemented, some kind of
  // ownership is going to have to happen here.
  SymbolTable *parent = nullptr;
  vector<shared_ptr<SymbolTable>> module_symbol_tables;
  unordered_map<string, SymbolTableEntry> entries;

  EvalResult lookup_rvalue(string var);
  EvalResult lookup_lvalue(string var);
};

// Dictionary
typedef unordered_map<string, std::pair<BoxedValue, shared_ptr<BoxedValue>>>
    Dict;

// HeVec = (He)terogenous (Vec)tor
typedef vector<shared_ptr<BoxedValue>> HeVec;

typedef std::variant<std::monostate, bool, int, double, string,
                     shared_ptr<HeVec>, shared_ptr<Dict>, Module, Function>
    RawValue;

class BoxedValue {
public:
  DataType type;
  RawValue value;

  // Default constructor
  BoxedValue() : type{}, value{} {}

  BoxedValue(DataType _type, RawValue _value) : type{_type}, value{_value} {}
};

class LValue {
public:
  virtual void assign(BoxedValue value) { throw NotImplemented(); };

  virtual BoxedValue currentValue() { throw NotImplemented(); }
};

class VariableLV : public LValue {
public:
  SymbolTable *symbol_table;
  string identifier;

  VariableLV(SymbolTable *_symbol_table, string _identifier)
      : symbol_table{_symbol_table}, identifier{_identifier} {}

  void assign(BoxedValue value) override;
  BoxedValue currentValue() override;
};

class VectorIndexLV : public LValue {
public:
  shared_ptr<HeVec> vector;
  BoxedValue index;

  VectorIndexLV(shared_ptr<HeVec> _vector, BoxedValue _index)
      : vector{_vector}, index{_index} {}

  void assign(BoxedValue value) override;
  BoxedValue currentValue() override;
};

class DictIndexLV : public LValue {
public:
  shared_ptr<Dict> dict;
  BoxedValue key;

  DictIndexLV(shared_ptr<Dict> _dict, BoxedValue _key)
      : dict{_dict}, key{_key} {}

  void assign(BoxedValue value) override;
  BoxedValue currentValue() override;
};

struct EvalResult {
  optional<BoxedValue> rv_result;
  shared_ptr<LValue> lv_result;
  bool returned = false;
};

EvalResult eval_top_level(ASTNode &node, vector<string> argv = {});
EvalResult eval_top_level(ASTNode &node, string module_wd,
                          vector<string> argv = {});
#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "astnode.h"

using std::string;
using std::vector;
using std::shared_ptr;
using std::optional;
using std::unordered_map;

class NotImplemented : public std::logic_error
{
public:
    NotImplemented() : std::logic_error("Function not yet implemented") { };
};

struct EvalResult;

enum class ValueType {
    LVALUE,
    RVALUE
};

enum class DataType {
    BOOL,
    FLOAT,
    INT,
    STRING,
    VECTOR,
    FUNCTION
};

struct Function {
    string name;
    vector<string> args;
    ASTNode body;
};

enum class VarType {
    // TODO: is there any reason to have "FUNCTION" here? We already have that in BoxedValue
    // Consider removing...
    CONST, VAR, FUNCTION
};

struct BoxedValue;
struct SymbolTableEntry {
    VarType type;
    shared_ptr<BoxedValue> value;
};

struct SymbolTable;
struct SymbolTable {
    // this is fine, I guess. 
    // But if closures are ever implemented, some kind of 
    // ownership is going to have to happen here.
    SymbolTable *parent = nullptr;
    unordered_map<string, SymbolTableEntry> entries;

    EvalResult lookup_rvalue(string var);
    EvalResult lookup_lvalue(string var);
};

// change this unless at some point we put things other than a symbol table in the execution context
typedef SymbolTable ExecutionContext;
// struct ExecutionContext {

// };

// HeVec = (He)terogenous (Vec)tor
typedef vector<shared_ptr<BoxedValue>> HeVec;

typedef std::variant<
    std::monostate, 
    bool, 
    int, 
    float, 
    string, 
    shared_ptr<HeVec>, 
    Function
> RawValue;

struct BoxedValue {
    DataType type;
    RawValue value;
};

class LValue {
public:
    virtual void assign(BoxedValue value) {
        throw NotImplemented();
    };

    virtual BoxedValue currentValue() {
        throw NotImplemented();
    }
};

class VariableLV : LValue {
public:
    SymbolTable* symbol_table;
    string identifier;

    void assign(BoxedValue value) override;
    BoxedValue currentValue() override;
};

class VectorIndexLV : LValue {
public:
    shared_ptr<HeVec> vector;
    BoxedValue index;

    void assign(BoxedValue value) override;
    BoxedValue currentValue() override;
};

// class FieldAccessLV: LValue {
    
// };

struct EvalResult {
    optional<BoxedValue> rv_result;
    shared_ptr<LValue> lv_result;
    bool returned = false;
};
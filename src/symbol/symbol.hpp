#pragma once

#include "lex/token.hpp"
#include <string_view>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace TCC_Pro {

class Type;
class TypeContext;
class Arena;

enum SymbolKind {
    SK_Tag, SK_TypedefName, SK_EnumConstant, SK_Constant, SK_Variable, SK_Temp,
    SK_Offset, SK_String, SK_Label, SK_Function, SK_Register
};

struct BBlock;
struct InitData;

class ValueDef {
public:
    class Symbol* dst = nullptr;
    int op = 0;
    Symbol* src1 = nullptr;
    Symbol* src2 = nullptr;
    BBlock* ownBB = nullptr;
    ValueDef* link = nullptr;
};

class ValueUse {
public:
    ValueDef* def = nullptr;
    ValueUse* next_ = nullptr;
};

class Symbol {
public:
    SymbolKind kind;
    const char* name = nullptr;
    const char* aname = nullptr;
    Type* ty = nullptr;
    int level = 0;
    int sclass = 0;
    int ref = 0;
    int defined : 1;
    int addressed : 1;
    int needwb : 1;
    Value val;
    Symbol* reg = nullptr;
    Symbol* link = nullptr;
    Symbol* next = nullptr;

    Symbol() : defined(0), addressed(0), needwb(0) {}
    virtual ~Symbol() = default;
};

class VariableSymbol : public Symbol {
public:
    InitData* idata = nullptr;
    ValueDef* def = nullptr;
    ValueUse* uses = nullptr;
    int offset = 0;

    VariableSymbol() { kind = SK_Variable; }
};

class FunctionSymbol : public Symbol {
public:
    Symbol* params = nullptr;
    Symbol* locals = nullptr;
    Symbol** lastv = &params;
    int nbblock = 0;
    BBlock* entryBB = nullptr;
    BBlock* exitBB = nullptr;
    ValueDef* valNumTable[16]{};

    FunctionSymbol() { kind = SK_Function; }
};

class Scope {
public:
    Scope* outer = nullptr;
    int level = 0;

    Symbol* lookup(const char* name);
    void add(Symbol* sym);

    Symbol** buckets() { return buckets_; }

    static unsigned hashName(const char* name);

private:
    static constexpr unsigned HASH_MASK = 127;
    Symbol* buckets_[HASH_MASK + 1]{};
};

class SymbolTable {
public:
    SymbolTable();

    void enterScope();
    void exitScope();

    Symbol* lookupID(const char* name);
    Symbol* lookupTag(const char* name);

    Symbol* addTag(const char* name, Type* ty);
    Symbol* addEnumConstant(const char* name, Type* ty, int val);
    Symbol* addTypedefName(const char* name, Type* ty);
    Symbol* addVariable(const char* name, Type* ty, int sclass);
    Symbol* addFunction(const char* name, Type* ty, int sclass);
    Symbol* addConstant(Type* ty, Value val);
    Symbol* intConstant(int i);
    Symbol* addString(Type* ty, void* str);
    Symbol* createTemp(Type* ty);
    Symbol* createLabel();
    Symbol* createOffset(Type* ty, Symbol* base, int coff);

    int level() const { return level_; }

    Symbol* functions = nullptr;
    Symbol* globals = nullptr;
    Symbol* strings = nullptr;
    Symbol* floatConstants = nullptr;

    FunctionSymbol* currentFunc = nullptr;

private:
    Scope* identifiers_ = nullptr;
    Scope* tags_ = nullptr;
    Scope globalIDs_;
    Scope globalTags_;
    Scope constants_;

    Symbol** functionTail_ = &functions;
    Symbol** globalTail_ = &globals;
    Symbol** stringTail_ = &strings;
    Symbol** floatTail_ = &floatConstants;

    int level_ = 0;
    int tempNum_ = 0;
    int labelNum_ = 0;
    int stringNum_ = 0;

    static unsigned hashName(const char* name);
};

} // namespace TCC_Pro

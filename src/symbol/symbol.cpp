#include "symbol/symbol.hpp"
#include "type/type.hpp"
#include "common/arena.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cassert>

namespace TCC_Pro {

unsigned Scope::hashName(const char* name) {
    return reinterpret_cast<unsigned long>(name) & 127;
}

Symbol* Scope::lookup(const char* name) {
    unsigned h = hashName(name);
    for (auto* s = this; s; s = s->outer) {
        for (auto* p = s->buckets_[h]; p; p = p->link) {
            if (p->name == name) return p;
        }
    }
    return nullptr;
}

void Scope::add(Symbol* sym) {
    unsigned h = hashName(sym->name);
    sym->link = buckets_[h];
    sym->level = level;
    buckets_[h] = sym;
}

SymbolTable::SymbolTable() {
    identifiers_ = &globalIDs_;
    tags_ = &globalTags_;
}

void SymbolTable::enterScope() {
    level_++;
    auto* t = new Scope();
    t->level = level_;
    t->outer = identifiers_;
    identifiers_ = t;

    t = new Scope();
    t->level = level_;
    t->outer = tags_;
    tags_ = t;
}

void SymbolTable::exitScope() {
    level_--;
    identifiers_ = identifiers_->outer;
    tags_ = tags_->outer;
}

Symbol* SymbolTable::lookupID(const char* name) {
    return identifiers_->lookup(name);
}

Symbol* SymbolTable::lookupTag(const char* name) {
    return tags_->lookup(name);
}

Symbol* SymbolTable::addTag(const char* name, Type* ty) {
    auto* p = new Symbol();
    p->kind = SK_Tag;
    p->name = name;
    p->ty = ty;
    tags_->add(p);
    return p;
}

Symbol* SymbolTable::addEnumConstant(const char* name, Type* ty, int val) {
    auto* p = new Symbol();
    p->kind = SK_EnumConstant;
    p->name = name;
    p->ty = ty;
    p->val.i[0] = val;
    identifiers_->add(p);
    return p;
}

Symbol* SymbolTable::addTypedefName(const char* name, Type* ty) {
    auto* p = new Symbol();
    p->kind = SK_TypedefName;
    p->name = name;
    p->ty = ty;
    identifiers_->add(p);
    return p;
}

Symbol* SymbolTable::addVariable(const char* name, Type* ty, int sclass) {
    auto* p = new VariableSymbol();
    p->kind = SK_Variable;
    p->name = name;
    p->ty = ty;
    p->sclass = sclass;

    if (level_ == 0 || sclass == 8 /*TK_STATIC*/) {
        *globalTail_ = p;
        globalTail_ = &p->next;
    } else if (sclass != 1 /*TK_EXTERN*/) {
        *currentFunc->lastv = p;
        currentFunc->lastv = &p->next;
    }
    identifiers_->add(p);
    return p;
}

Symbol* SymbolTable::addFunction(const char* name, Type* ty, int sclass) {
    auto* p = new FunctionSymbol();
    p->kind = SK_Function;
    p->name = name;
    p->ty = ty;
    p->sclass = sclass;
    p->lastv = &p->params;

    *functionTail_ = p;
    functionTail_ = &p->next;

    globalIDs_.add(p);
    return p;
}

Symbol* SymbolTable::addConstant(Type* ty, Value val) {
    ty = TypeContext().unqual(ty);
    if (ty->isInteg())
        ty = TypeContext().intType();
    else if (ty->isPtr())
        ty = TypeContext().pointerType();
    else if (ty->categ == T_LONGDOUBLE)
        ty = TypeContext().doubleType();

    unsigned h = static_cast<unsigned>(val.i[0]) & 127;
    for (auto* p = constants_.buckets()[h]; p; p = p->link) {
        if (p->ty == ty && p->val.i[0] == val.i[0] && p->val.i[1] == val.i[1])
            return p;
    }

    auto* p = new Symbol();
    p->kind = SK_Constant;
    char buf[64];
    switch (ty->categ) {
    case T_INT: snprintf(buf, sizeof(buf), "%d", val.i[0]); break;
    case T_POINTER:
        if (val.i[0] == 0) snprintf(buf, sizeof(buf), "0");
        else snprintf(buf, sizeof(buf), "0x%x", val.i[0]);
        break;
    case T_FLOAT: snprintf(buf, sizeof(buf), "%g", val.f); break;
    case T_DOUBLE: snprintf(buf, sizeof(buf), "%g", val.d); break;
    default: assert(0);
    }
    p->name = strdup(buf);
    p->ty = ty;
    p->sclass = 3 /*TK_STATIC*/;
    p->val = val;

    p->link = constants_.buckets()[h];
    constants_.buckets()[h] = p;

    if (ty->categ == T_FLOAT || ty->categ == T_DOUBLE) {
        *floatTail_ = p;
        floatTail_ = &p->next;
    }
    return p;
}

Symbol* SymbolTable::intConstant(int i) {
    Value v; v.i[0] = i; v.i[1] = 0;
    return addConstant(TypeContext().intType(), v);
}

Symbol* SymbolTable::addString(Type* ty, void* str) {
    auto* p = new Symbol();
    p->kind = SK_String;
    char buf[32];
    snprintf(buf, sizeof(buf), "str%d", stringNum_++);
    p->name = strdup(buf);
    p->ty = ty;
    p->sclass = 3;
    p->val.p = str;
    *stringTail_ = p;
    stringTail_ = &p->next;
    return p;
}

Symbol* SymbolTable::createTemp(Type* ty) {
    auto* p = new VariableSymbol();
    p->kind = SK_Temp;
    char buf[32];
    snprintf(buf, sizeof(buf), "t%d", tempNum_++);
    p->name = strdup(buf);
    p->ty = ty;
    p->level = 1;
    *currentFunc->lastv = p;
    currentFunc->lastv = &p->next;
    // Assign stack offset
    static int tempOffset = -4;
    p->offset = tempOffset;
    tempOffset -= 4;
    return p;
}

Symbol* SymbolTable::createLabel() {
    auto* p = new Symbol();
    p->kind = SK_Label;
    char buf[32];
    snprintf(buf, sizeof(buf), "BB%d", labelNum_++);
    p->name = strdup(buf);
    return p;
}

Symbol* SymbolTable::createOffset(Type* ty, Symbol* base, int coff) {
    if (coff == 0) return base;
    auto* p = new VariableSymbol();
    if (base->kind == SK_Offset) {
        coff += static_cast<VariableSymbol*>(base)->offset;
        base = base->link;
    }
    p->addressed = 1;
    p->kind = SK_Offset;
    p->ty = ty;
    p->link = base;
    static_cast<VariableSymbol*>(p)->offset = coff;
    char buf[64];
    snprintf(buf, sizeof(buf), "%s[%d]", base->name, coff);
    p->name = strdup(buf);
    base->ref++;
    return p;
}

} // namespace TCC_Pro

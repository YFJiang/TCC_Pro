#include "ast/expr.hpp"
#include "ast/stmt.hpp"
#include "ast/decl.hpp"
#include <cassert>

namespace TCC_Pro {

// OpInfo table: op, prec, name, kind
// kind: 0=Error, 1=Binary, 2=Unary, 3=Postfix, 4=Primary, 5=Conditional, 6=Assignment, 7=Comma
static constexpr OpInfo opInfoTable[] = {
    {OpKind::OP_COMMA,         1,  ",",      7},
    {OpKind::OP_ASSIGN,        2,  "=",      6},
    {OpKind::OP_BITOR_ASSIGN,  2,  "|=",     6},
    {OpKind::OP_BITXOR_ASSIGN, 2,  "^=",     6},
    {OpKind::OP_BITAND_ASSIGN, 2,  "&=",     6},
    {OpKind::OP_LSHIFT_ASSIGN, 2,  "<<=",    6},
    {OpKind::OP_RSHIFT_ASSIGN, 2,  ">>=",    6},
    {OpKind::OP_ADD_ASSIGN,    2,  "+=",     6},
    {OpKind::OP_SUB_ASSIGN,    2,  "-=",     6},
    {OpKind::OP_MUL_ASSIGN,    2,  "*=",     6},
    {OpKind::OP_DIV_ASSIGN,    2,  "/=",     6},
    {OpKind::OP_MOD_ASSIGN,    2,  "%=",     6},
    {OpKind::OP_QUESTION,      3,  "?",      5},
    {OpKind::OP_COLON,         3,  ":",      0},
    {OpKind::OP_OR,            4,  "||",     1},
    {OpKind::OP_AND,           5,  "&&",     1},
    {OpKind::OP_BITOR,         6,  "|",      1},
    {OpKind::OP_BITXOR,        7,  "^",      1},
    {OpKind::OP_BITAND,        8,  "&",      1},
    {OpKind::OP_EQUAL,         9,  "==",     1},
    {OpKind::OP_UNEQUAL,       9,  "!=",     1},
    {OpKind::OP_GREAT,         10, ">",      1},
    {OpKind::OP_LESS,          10, "<",      1},
    {OpKind::OP_GREAT_EQ,      10, ">=",     1},
    {OpKind::OP_LESS_EQ,       10, "<=",     1},
    {OpKind::OP_LSHIFT,        11, "<<",     1},
    {OpKind::OP_RSHIFT,        11, ">>",     1},
    {OpKind::OP_ADD,           12, "+",      1},
    {OpKind::OP_SUB,           12, "-",      1},
    {OpKind::OP_MUL,           13, "*",      1},
    {OpKind::OP_DIV,           13, "/",      1},
    {OpKind::OP_MOD,           13, "%",      1},
    {OpKind::OP_CAST,          14, "cast",   2},
    {OpKind::OP_PREINC,        14, "++",     2},
    {OpKind::OP_PREDEC,        14, "--",     2},
    {OpKind::OP_ADDRESS,       14, "&",      2},
    {OpKind::OP_DEREF,         14, "*",      2},
    {OpKind::OP_POS,           14, "+",      2},
    {OpKind::OP_NEG,           14, "-",      2},
    {OpKind::OP_COMP,          14, "~",      2},
    {OpKind::OP_NOT,           14, "!",      2},
    {OpKind::OP_SIZEOF,        14, "sizeof", 2},
    {OpKind::OP_INDEX,         15, "[]",     3},
    {OpKind::OP_CALL,          15, "call",   3},
    {OpKind::OP_MEMBER,        15, ".",      3},
    {OpKind::OP_PTR_MEMBER,    15, "->",     3},
    {OpKind::OP_POSTINC,       15, "++",     3},
    {OpKind::OP_POSTDEC,       15, "--",     3},
    {OpKind::OP_ID,            16, "id",     4},
    {OpKind::OP_CONST,         16, "const",  4},
    {OpKind::OP_STR,           16, "str",    4},
    {OpKind::OP_NONE,          17, "nop",    0},
};

const OpInfo* getOpInfo(OpKind op) {
    for (auto& info : opInfoTable) {
        if (info.op == op) return &info;
    }
    return &opInfoTable[sizeof(opInfoTable)/sizeof(opInfoTable[0]) - 1]; // OP_NONE
}

const char* opName(OpKind op) {
    return getOpInfo(op)->name;
}

int opPrecedence(OpKind op) {
    return getOpInfo(op)->prec;
}

// accept implementations

void ConstantExpr::accept(Visitor& v)        { v.visit(this); }
void IdentifierExpr::accept(Visitor& v)      { v.visit(this); }
void StringExpr::accept(Visitor& v)          { v.visit(this); }
void UnaryExpr::accept(Visitor& v)           { v.visit(this); }
void BinaryExpr::accept(Visitor& v)          { v.visit(this); }
void ConditionalExpr::accept(Visitor& v)     { v.visit(this); }
void CastExpr::accept(Visitor& v)            { v.visit(this); }
void CallExpr::accept(Visitor& v)            { v.visit(this); }
void IndexExpr::accept(Visitor& v)           { v.visit(this); }
void MemberExpr::accept(Visitor& v)          { v.visit(this); }
void PtrMemberExpr::accept(Visitor& v)       { v.visit(this); }
void PostfixExpr::accept(Visitor& v)         { v.visit(this); }
void AssignExpr::accept(Visitor& v)          { v.visit(this); }
void CommaExpr::accept(Visitor& v)           { v.visit(this); }

void ExprStmt::accept(Visitor& v)            { v.visit(this); }
void LabelStmt::accept(Visitor& v)           { v.visit(this); }
void CaseStmt::accept(Visitor& v)            { v.visit(this); }
void DefaultStmt::accept(Visitor& v)         { v.visit(this); }
void IfStmt::accept(Visitor& v)              { v.visit(this); }
void SwitchStmt::accept(Visitor& v)          { v.visit(this); }
void WhileStmt::accept(Visitor& v)           { v.visit(this); }
void DoStmt::accept(Visitor& v)              { v.visit(this); }
void ForStmt::accept(Visitor& v)             { v.visit(this); }
void GotoStmt::accept(Visitor& v)            { v.visit(this); }
void BreakStmt::accept(Visitor& v)           { v.visit(this); }
void ContinueStmt::accept(Visitor& v)        { v.visit(this); }
void ReturnStmt::accept(Visitor& v)          { v.visit(this); }
void CompoundStmt::accept(Visitor& v)        { v.visit(this); }

void TranslationUnit::accept(Visitor& v)     { v.visit(this); }
void Function::accept(Visitor& v)            { v.visit(this); }
void Declaration::accept(Visitor& v)         { v.visit(this); }
void Specifiers::accept(Visitor& v)          { v.visit(this); }
void TypeName::accept(Visitor& v)            { v.visit(this); }
void InitDeclarator::accept(Visitor& v)      { v.visit(this); }
void Initializer::accept(Visitor& v)         { v.visit(this); }
void ParameterDeclaration::accept(Visitor& v) { v.visit(this); }
void ParameterTypeList::accept(Visitor& v)   { v.visit(this); }
void EnumSpecifier::accept(Visitor& v)       { v.visit(this); }
void Enumerator::accept(Visitor& v)          { v.visit(this); }
void StructSpecifier::accept(Visitor& v)     { v.visit(this); }
void UnionSpecifier::accept(Visitor& v)      { v.visit(this); }
void StructDeclaration::accept(Visitor& v)   { v.visit(this); }
void StructDeclarator::accept(Visitor& v)    { v.visit(this); }
void TypedefName::accept(Visitor& v)         { v.visit(this); }
void TokenNode::accept(Visitor& v)           { v.visit(this); }

void NameDeclarator::accept(Visitor& v)      { v.visit(this); }
void FunctionDeclarator::accept(Visitor& v)  { v.visit(this); }
void ArrayDeclarator::accept(Visitor& v)     { v.visit(this); }
void PointerDeclarator::accept(Visitor& v)   { v.visit(this); }

} // namespace TCC_Pro

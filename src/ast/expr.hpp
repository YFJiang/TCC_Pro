#pragma once

#include "ast/node.hpp"
#include "lex/token.hpp"
#include <string_view>

namespace TCC_Pro {

class Type;

enum class OpKind : int {
    OP_COMMA,
    OP_ASSIGN, OP_BITOR_ASSIGN, OP_BITXOR_ASSIGN, OP_BITAND_ASSIGN,
    OP_LSHIFT_ASSIGN, OP_RSHIFT_ASSIGN, OP_ADD_ASSIGN, OP_SUB_ASSIGN,
    OP_MUL_ASSIGN, OP_DIV_ASSIGN, OP_MOD_ASSIGN,
    OP_QUESTION, OP_COLON,
    OP_OR, OP_AND,
    OP_BITOR, OP_BITXOR, OP_BITAND,
    OP_EQUAL, OP_UNEQUAL, OP_GREAT, OP_LESS, OP_GREAT_EQ, OP_LESS_EQ,
    OP_LSHIFT, OP_RSHIFT,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_CAST,
    OP_PREINC, OP_PREDEC, OP_ADDRESS, OP_DEREF,
    OP_POS, OP_NEG, OP_COMP, OP_NOT, OP_SIZEOF,
    OP_INDEX, OP_CALL, OP_MEMBER, OP_PTR_MEMBER,
    OP_POSTINC, OP_POSTDEC,
    OP_ID, OP_CONST, OP_STR,
    OP_NONE,
};

struct OpInfo {
    OpKind op;
    int prec;
    const char* name;
    int opKind;  // 0=Error, 1=Binary, 2=Unary, 3=Postfix, 4=Primary, 5=Conditional, 6=Assignment, 7=Comma
};

const OpInfo* getOpInfo(OpKind op);
const char* opName(OpKind op);
int opPrecedence(OpKind op);

// Base expression class
class Expr : public Node {
public:
    Type* ty = nullptr;
    bool isArray = false;
    bool isFunc = false;
    bool lvalue = false;
    bool bitField = false;
    bool inReg = false;

    explicit Expr(NodeKind kind) : Node(kind) {}
};

class ConstantExpr : public Expr {
public:
    Value value;
    TokenKind tokenKind;

    ConstantExpr() : Expr(NodeKind::NK_ConstantExpr) {}
    void accept(Visitor& v) override;
};

class IdentifierExpr : public Expr {
public:
    const char* name;

    IdentifierExpr() : Expr(NodeKind::NK_IdentifierExpr) {}
    void accept(Visitor& v) override;
};

class StringExpr : public Expr {
public:
    void* str;

    StringExpr() : Expr(NodeKind::NK_StringExpr) {}
    void accept(Visitor& v) override;
};

class UnaryExpr : public Expr {
public:
    OpKind op;
    Expr* operand;

    UnaryExpr() : Expr(NodeKind::NK_UnaryExpr) {}
    void accept(Visitor& v) override;
};

class BinaryExpr : public Expr {
public:
    OpKind op;
    Expr* left;
    Expr* right;

    BinaryExpr() : Expr(NodeKind::NK_BinaryExpr) {}
    void accept(Visitor& v) override;
};

class ConditionalExpr : public Expr {
public:
    Expr* cond;
    Expr* thenExpr;
    Expr* elseExpr;

    ConditionalExpr() : Expr(NodeKind::NK_ConditionalExpr) {}
    void accept(Visitor& v) override;
};

class CastExpr : public Expr {
public:
    Type* castTy;
    Expr* operand;

    CastExpr() : Expr(NodeKind::NK_CastExpr) {}
    void accept(Visitor& v) override;
};

class CallExpr : public Expr {
public:
    Expr* func;
    Expr* args;

    CallExpr() : Expr(NodeKind::NK_CallExpr) {}
    void accept(Visitor& v) override;
};

class IndexExpr : public Expr {
public:
    Expr* base;
    Expr* index;

    IndexExpr() : Expr(NodeKind::NK_IndexExpr) {}
    void accept(Visitor& v) override;
};

class MemberExpr : public Expr {
public:
    Expr* obj;
    const char* member;

    MemberExpr() : Expr(NodeKind::NK_MemberExpr) {}
    void accept(Visitor& v) override;
};

class PtrMemberExpr : public Expr {
public:
    Expr* obj;
    const char* member;

    PtrMemberExpr() : Expr(NodeKind::NK_PtrMemberExpr) {}
    void accept(Visitor& v) override;
};

class PostfixExpr : public Expr {
public:
    OpKind op;
    Expr* operand;

    PostfixExpr() : Expr(NodeKind::NK_PostfixExpr) {}
    void accept(Visitor& v) override;
};

class AssignExpr : public Expr {
public:
    OpKind op;
    Expr* left;
    Expr* right;

    AssignExpr() : Expr(NodeKind::NK_AssignExpr) {}
    void accept(Visitor& v) override;
};

class CommaExpr : public Expr {
public:
    Expr* left;
    Expr* right;

    CommaExpr() : Expr(NodeKind::NK_CommaExpr) {}
    void accept(Visitor& v) override;
};

} // namespace TCC_Pro

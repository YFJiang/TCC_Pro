#pragma once

#include "ast/node.hpp"
#include "lex/token.hpp"
#include <vector>

namespace TCC_Pro {

class Expr;
class Declaration;

class Stmt : public Node {
public:
    explicit Stmt(NodeKind kind) : Node(kind) {}
};

class ExprStmt : public Stmt {
public:
    Expr* expr = nullptr;

    ExprStmt() : Stmt(NodeKind::NK_ExprStmt) {}
    void accept(Visitor& v) override;
};

class LabelStmt : public Stmt {
public:
    const char* id;
    Stmt* stmt = nullptr;
    void* label = nullptr;  // Label*

    LabelStmt() : Stmt(NodeKind::NK_LabelStmt) {}
    void accept(Visitor& v) override;
};

class CaseStmt : public Stmt {
public:
    Expr* expr = nullptr;
    Stmt* stmt = nullptr;
    CaseStmt* nextCase = nullptr;
    void* respBB = nullptr;  // BBlock*

    CaseStmt() : Stmt(NodeKind::NK_CaseStmt) {}
    void accept(Visitor& v) override;
};

class DefaultStmt : public Stmt {
public:
    Stmt* stmt = nullptr;
    void* respBB = nullptr;  // BBlock*

    DefaultStmt() : Stmt(NodeKind::NK_DefaultStmt) {}
    void accept(Visitor& v) override;
};

class IfStmt : public Stmt {
public:
    Expr* expr = nullptr;
    Stmt* thenStmt = nullptr;
    Stmt* elseStmt = nullptr;

    IfStmt() : Stmt(NodeKind::NK_IfStmt) {}
    void accept(Visitor& v) override;
};

class SwitchStmt : public Stmt {
public:
    Expr* expr = nullptr;
    Stmt* stmt = nullptr;
    CaseStmt* cases = nullptr;
    DefaultStmt* defStmt = nullptr;
    void* buckets = nullptr;
    int nbucket = 0;
    void* nextBB = nullptr;
    void* defBB = nullptr;

    SwitchStmt() : Stmt(NodeKind::NK_SwitchStmt) {}
    void accept(Visitor& v) override;
};

class LoopStmt : public Stmt {
public:
    Expr* expr = nullptr;
    Stmt* stmt = nullptr;
    void* loopBB = nullptr;
    void* contBB = nullptr;
    void* nextBB = nullptr;

    explicit LoopStmt(NodeKind kind) : Stmt(kind) {}
};

class WhileStmt : public LoopStmt {
public:
    WhileStmt() : LoopStmt(NodeKind::NK_WhileStmt) {}
    void accept(Visitor& v) override;
};

class DoStmt : public LoopStmt {
public:
    DoStmt() : LoopStmt(NodeKind::NK_DoStmt) {}
    void accept(Visitor& v) override;
};

class ForStmt : public LoopStmt {
public:
    Expr* initExpr = nullptr;
    Expr* incrExpr = nullptr;
    void* testBB = nullptr;

    ForStmt() : LoopStmt(NodeKind::NK_ForStmt) {}
    void accept(Visitor& v) override;
};

class GotoStmt : public Stmt {
public:
    const char* id;
    void* label = nullptr;

    GotoStmt() : Stmt(NodeKind::NK_GotoStmt) {}
    void accept(Visitor& v) override;
};

class BreakStmt : public Stmt {
public:
    Stmt* target = nullptr;

    BreakStmt() : Stmt(NodeKind::NK_BreakStmt) {}
    void accept(Visitor& v) override;
};

class ContinueStmt : public Stmt {
public:
    LoopStmt* target = nullptr;

    ContinueStmt() : Stmt(NodeKind::NK_ContinueStmt) {}
    void accept(Visitor& v) override;
};

class ReturnStmt : public Stmt {
public:
    Expr* expr = nullptr;

    ReturnStmt() : Stmt(NodeKind::NK_ReturnStmt) {}
    void accept(Visitor& v) override;
};

class CompoundStmt : public Stmt {
public:
    Node* decls = nullptr;
    Node* stmts = nullptr;
    void* iLocals = nullptr;  // Vector*

    CompoundStmt() : Stmt(NodeKind::NK_CompoundStmt) {}
    void accept(Visitor& v) override;
};

} // namespace TCC_Pro

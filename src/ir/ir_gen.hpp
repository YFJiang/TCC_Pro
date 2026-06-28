#pragma once

#include "ir/ir.hpp"
#include "ast/node.hpp"
#include "ast/expr.hpp"
#include "ast/stmt.hpp"
#include "ast/decl.hpp"
#include "symbol/symbol.hpp"
#include "type/type.hpp"
#include "common/diagnostics.hpp"
#include <vector>

namespace TCC_Pro {

class IRGenerator : public Visitor {
public:
    IRGenerator(SymbolTable& syms, TypeContext& types, Diagnostics& diag);

    void generate(TranslationUnit* unit);

    void visit(ConstantExpr* node) override;
    void visit(IdentifierExpr* node) override;
    void visit(StringExpr* node) override;
    void visit(UnaryExpr* node) override;
    void visit(BinaryExpr* node) override;
    void visit(ConditionalExpr* node) override;
    void visit(CastExpr* node) override;
    void visit(CallExpr* node) override;
    void visit(IndexExpr* node) override;
    void visit(MemberExpr* node) override;
    void visit(PtrMemberExpr* node) override;
    void visit(PostfixExpr* node) override;
    void visit(AssignExpr* node) override;
    void visit(CommaExpr* node) override;

    void visit(ExprStmt* node) override;
    void visit(LabelStmt* node) override;
    void visit(CaseStmt* node) override;
    void visit(DefaultStmt* node) override;
    void visit(IfStmt* node) override;
    void visit(SwitchStmt* node) override;
    void visit(WhileStmt* node) override;
    void visit(DoStmt* node) override;
    void visit(ForStmt* node) override;
    void visit(GotoStmt* node) override;
    void visit(BreakStmt* node) override;
    void visit(ContinueStmt* node) override;
    void visit(ReturnStmt* node) override;
    void visit(CompoundStmt* node) override;

    void visit(TranslationUnit* node) override;
    void visit(Function* node) override;
    void visit(Declaration* node) override;
    void visit(Specifiers* node) override;
    void visit(TypeName* node) override;
    void visit(InitDeclarator* node) override;
    void visit(Initializer* node) override;
    void visit(ParameterDeclaration* node) override;
    void visit(ParameterTypeList* node) override;
    void visit(EnumSpecifier* node) override;
    void visit(Enumerator* node) override;
    void visit(StructSpecifier* node) override;
    void visit(UnionSpecifier* node) override;
    void visit(StructDeclaration* node) override;
    void visit(StructDeclarator* node) override;
    void visit(TypedefName* node) override;
    void visit(TokenNode* node) override;

    void visit(FunctionDeclarator* node) override;
    void visit(ArrayDeclarator* node) override;
    void visit(PointerDeclarator* node) override;
    void visit(NameDeclarator* node) override;

    // Access generated IR
    FunctionSymbol* currentFunc() { return currentFunc_; }
    std::vector<FunctionSymbol*>& functions() { return functions_; }

private:
    Symbol* translateExpr(Expr* expr);
    Symbol* translatePrimaryExpr(Expr* expr);
    Symbol* translateBranchExpr(Expr* expr);
    Symbol* translateCallExpr(CallExpr* expr);
    Symbol* translateMemberAccess(Expr* expr);
    Symbol* translateArrayIndex(IndexExpr* expr);
    Symbol* translateIncrement(Expr* expr, bool isPost);
    Symbol* translateAssign(AssignExpr* expr);
    Symbol* translateUnary(UnaryExpr* expr);
    Symbol* translateBinary(BinaryExpr* expr);
    Symbol* translateConditional(ConditionalExpr* expr);
    Symbol* translateComma(CommaExpr* expr);

    void translateBranch(Expr* expr, BBlock* trueBB, BBlock* falseBB);

    // IR generation helpers
    BBlock* createBBlock();
    void startBBlock(BBlock* bb);
    void appendInst(IRInst* inst);
    void genMove(Type* ty, Symbol* dst, Symbol* src);
    void genIndirectMove(Type* ty, Symbol* dst, Symbol* src);
    void genAssign(Type* ty, Symbol* dst, OpCode opcode, Symbol* src1, Symbol* src2);
    void genBranch(Type* ty, BBlock* dstBB, OpCode opcode, Symbol* src1, Symbol* src2);
    void genJump(BBlock* dstBB);
    void genReturn(Type* ty, Symbol* src);
    void genCall(Type* ty, Symbol* recv, Symbol* faddr, std::vector<Symbol*> args);
    void drawCFGEdge(BBlock* from, BBlock* to);

    Symbol* simplify(Type* ty, OpCode op, Symbol* src1, Symbol* src2);
    Symbol* addressOf(Symbol* sym);
    Symbol* deref(Type* ty, Symbol* addr);
    Symbol* tryAddValue(Type* ty, OpCode op, Symbol* src1, Symbol* src2);

    SymbolTable& syms_;
    TypeContext& types_;
    Diagnostics& diag_;

    FunctionSymbol* currentFunc_ = nullptr;
    BBlock* currentBB_ = nullptr;
    std::vector<FunctionSymbol*> functions_;
    Symbol* resultSym_ = nullptr;
};

} // namespace TCC_Pro

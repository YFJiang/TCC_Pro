#pragma once

#include "ast/node.hpp"
#include "ast/expr.hpp"
#include "ast/stmt.hpp"
#include "ast/decl.hpp"
#include "symbol/symbol.hpp"
#include "type/type.hpp"
#include "common/diagnostics.hpp"

namespace TCC_Pro {

class SemanticChecker : public Visitor {
public:
    SemanticChecker(SymbolTable& syms, TypeContext& types, Diagnostics& diag)
        : syms_(syms), types_(types), diag_(diag) {}

    void check(TranslationUnit* unit);

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

private:
    SymbolTable& syms_;
    TypeContext& types_;
    Diagnostics& diag_;
};

class ConstantFolder : public Visitor {
public:
    ConstantFolder(TypeContext& types) : types_(types) {}

    Expr* fold(Expr* expr);

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

private:
    TypeContext& types_;
    Expr* result_ = nullptr;
};

class Simplifier {
public:
    Simplifier(SymbolTable& syms) : syms_(syms) {}

private:
    SymbolTable& syms_;
};

} // namespace TCC_Pro

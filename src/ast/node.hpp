#pragma once

#include "common/source_location.hpp"
#include <string_view>

namespace TCC_Pro {

class Expr;
class Stmt;
class Decl;

class ConstantExpr;
class IdentifierExpr;
class StringExpr;
class UnaryExpr;
class BinaryExpr;
class ConditionalExpr;
class CastExpr;
class CallExpr;
class IndexExpr;
class MemberExpr;
class PtrMemberExpr;
class PostfixExpr;
class AssignExpr;
class CommaExpr;

class TranslationUnit;
class Function;

class ExprStmt;
class LabelStmt;
class CaseStmt;
class DefaultStmt;
class IfStmt;
class SwitchStmt;
class WhileStmt;
class DoStmt;
class ForStmt;
class GotoStmt;
class BreakStmt;
class ContinueStmt;
class ReturnStmt;
class CompoundStmt;

class Declaration;
class Specifiers;
class TypeName;
class InitDeclarator;
class Initializer;
class ParameterDeclaration;
class ParameterTypeList;
class EnumSpecifier;
class Enumerator;
class StructSpecifier;
class UnionSpecifier;
class StructDeclaration;
class StructDeclarator;
class TypedefName;
class TokenNode;

class FunctionDeclarator;
class ArrayDeclarator;
class PointerDeclarator;
class NameDeclarator;
class Declarator;

enum class NodeKind {
    // expressions
    NK_ConstantExpr,
    NK_IdentifierExpr,
    NK_StringExpr,
    NK_UnaryExpr,
    NK_BinaryExpr,
    NK_ConditionalExpr,
    NK_CastExpr,
    NK_CallExpr,
    NK_IndexExpr,
    NK_MemberExpr,
    NK_PtrMemberExpr,
    NK_PostfixExpr,
    NK_AssignExpr,
    NK_CommaExpr,

    // statements
    NK_ExprStmt,
    NK_LabelStmt,
    NK_CaseStmt,
    NK_DefaultStmt,
    NK_IfStmt,
    NK_SwitchStmt,
    NK_WhileStmt,
    NK_DoStmt,
    NK_ForStmt,
    NK_GotoStmt,
    NK_BreakStmt,
    NK_ContinueStmt,
    NK_ReturnStmt,
    NK_CompoundStmt,

    // declarations
    NK_TranslationUnit,
    NK_Function,
    NK_Declaration,
    NK_TypeName,
    NK_Specifiers,
    NK_InitDeclarator,
    NK_Initializer,
    NK_ParameterDeclaration,
    NK_ParameterTypeList,
    NK_EnumSpecifier,
    NK_Enumerator,
    NK_StructSpecifier,
    NK_UnionSpecifier,
    NK_StructDeclaration,
    NK_StructDeclarator,
    NK_TypedefName,
    NK_Token,

    // declarators
    NK_FunctionDeclarator,
    NK_ArrayDeclarator,
    NK_PointerDeclarator,
    NK_NameDeclarator,
};

class Visitor;

class Node {
public:
    NodeKind nodeKind;
    SourceLocation loc;
    Node* next = nullptr;

    explicit Node(NodeKind kind) : nodeKind(kind) {}
    virtual ~Node() = default;
    virtual void accept(Visitor& v) = 0;
};

class Visitor {
public:
    virtual ~Visitor() = default;

    virtual void visit(ConstantExpr* node) = 0;
    virtual void visit(IdentifierExpr* node) = 0;
    virtual void visit(StringExpr* node) = 0;
    virtual void visit(UnaryExpr* node) = 0;
    virtual void visit(BinaryExpr* node) = 0;
    virtual void visit(ConditionalExpr* node) = 0;
    virtual void visit(CastExpr* node) = 0;
    virtual void visit(CallExpr* node) = 0;
    virtual void visit(IndexExpr* node) = 0;
    virtual void visit(MemberExpr* node) = 0;
    virtual void visit(PtrMemberExpr* node) = 0;
    virtual void visit(PostfixExpr* node) = 0;
    virtual void visit(AssignExpr* node) = 0;
    virtual void visit(CommaExpr* node) = 0;

    virtual void visit(ExprStmt* node) = 0;
    virtual void visit(LabelStmt* node) = 0;
    virtual void visit(CaseStmt* node) = 0;
    virtual void visit(DefaultStmt* node) = 0;
    virtual void visit(IfStmt* node) = 0;
    virtual void visit(SwitchStmt* node) = 0;
    virtual void visit(WhileStmt* node) = 0;
    virtual void visit(DoStmt* node) = 0;
    virtual void visit(ForStmt* node) = 0;
    virtual void visit(GotoStmt* node) = 0;
    virtual void visit(BreakStmt* node) = 0;
    virtual void visit(ContinueStmt* node) = 0;
    virtual void visit(ReturnStmt* node) = 0;
    virtual void visit(CompoundStmt* node) = 0;

    virtual void visit(TranslationUnit* node) = 0;
    virtual void visit(Function* node) = 0;
    virtual void visit(Declaration* node) = 0;
    virtual void visit(Specifiers* node) = 0;
    virtual void visit(TypeName* node) = 0;
    virtual void visit(InitDeclarator* node) = 0;
    virtual void visit(Initializer* node) = 0;
    virtual void visit(ParameterDeclaration* node) = 0;
    virtual void visit(ParameterTypeList* node) = 0;
    virtual void visit(EnumSpecifier* node) = 0;
    virtual void visit(Enumerator* node) = 0;
    virtual void visit(StructSpecifier* node) = 0;
    virtual void visit(UnionSpecifier* node) = 0;
    virtual void visit(StructDeclaration* node) = 0;
    virtual void visit(StructDeclarator* node) = 0;
    virtual void visit(TypedefName* node) = 0;
    virtual void visit(TokenNode* node) = 0;

    virtual void visit(FunctionDeclarator* node) = 0;
    virtual void visit(ArrayDeclarator* node) = 0;
    virtual void visit(PointerDeclarator* node) = 0;
    virtual void visit(NameDeclarator* node) = 0;
};

} // namespace TCC_Pro

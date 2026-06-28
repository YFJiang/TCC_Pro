#pragma once

#include "lex/lexer.hpp"
#include "ast/expr.hpp"
#include "ast/stmt.hpp"
#include "ast/decl.hpp"
#include "common/compiler_context.hpp"

namespace TCC_Pro {

class Parser {
public:
    explicit Parser(CompilerContext& ctx, Lexer& lexer);

    TranslationUnit* parseTranslationUnit();

private:
    void nextToken();
    void expect(TokenKind kind);
    bool currentTokenIn(std::initializer_list<TokenKind> toks);

    // Declarations
    Declaration* parseDeclaration();
    Specifiers* parseSpecifiers();
    Declarator* parseDeclarator(int mode);
    TypeName* parseTypeName();
    Function* parseFunction(Specifiers* specs, Declarator* dec);
    ParameterTypeList* parseParameterTypeList();
    ParameterDeclaration* parseParameterDeclaration();
    Initializer* parseInitializer();
    InitDeclarator* parseInitDeclarator();
    EnumSpecifier* parseEnumSpecifier();
    Enumerator* parseEnumerator();
    StructSpecifier* parseStructSpecifier();
    UnionSpecifier* parseUnionSpecifier();
    StructDeclaration* parseStructDeclaration();
    StructDeclarator* parseStructDeclarator();

    // Statements
    Stmt* parseStatement();
    CompoundStmt* parseCompoundStatement();
    Stmt* parseIfStatement();
    Stmt* parseSwitchStatement();
    Stmt* parseWhileStatement();
    Stmt* parseDoStatement();
    Stmt* parseForStatement();
    Stmt* parseGotoStatement();
    Stmt* parseBreakStatement();
    Stmt* parseContinueStatement();
    Stmt* parseReturnStatement();
    Stmt* parseLabelStatement();
    Stmt* parseExpressionStatement();

    // Expressions
    Expr* parseExpression();
    Expr* parseAssignmentExpression();
    Expr* parseConstantExpression();
    Expr* parseBinaryExpression(int minPrec);
    Expr* parseCastExpression();
    Expr* parseUnaryExpression();
    Expr* parsePostfixExpression(Expr* base);
    Expr* parsePrimaryExpression();

    CompilerContext& ctx_;
    Lexer& lexer_;
    Token currentTok_;
};

} // namespace TCC_Pro

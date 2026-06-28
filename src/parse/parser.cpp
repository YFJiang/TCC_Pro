#include "parse/parser.hpp"

namespace TCC_Pro {

Parser::Parser(CompilerContext& ctx, Lexer& lexer)
    : ctx_(ctx), lexer_(lexer)
{
    currentTok_ = lexer_.nextToken();
}

void Parser::nextToken() {
    currentTok_ = lexer_.nextToken();
}

void Parser::expect(TokenKind kind) {
    if (currentTok_.kind != kind) {
        ctx_.diagnostics().error(currentTok_.loc,
            std::string("expected ") + tokenName(kind) +
            " but got " + std::string(tokenName(currentTok_.kind)));
    } else {
        nextToken();
    }
}

bool Parser::currentTokenIn(std::initializer_list<TokenKind> toks) {
    for (auto t : toks) {
        if (currentTok_.kind == t) return true;
    }
    return false;
}

static bool isTypeSpecifier(TokenKind k) {
    switch (k) {
    case TokenKind::TK_VOID: case TokenKind::TK_CHAR: case TokenKind::TK_SHORT:
    case TokenKind::TK_INT: case TokenKind::TK_LONG: case TokenKind::TK_FLOAT:
    case TokenKind::TK_DOUBLE: case TokenKind::TK_SIGNED: case TokenKind::TK_UNSIGNED:
    case TokenKind::TK_STRUCT: case TokenKind::TK_UNION: case TokenKind::TK_ENUM:
    case TokenKind::TK_TYPEDEF: case TokenKind::TK_EXTERN: case TokenKind::TK_STATIC:
    case TokenKind::TK_CONST: case TokenKind::TK_VOLATILE: case TokenKind::TK_AUTO:
    case TokenKind::TK_REGISTER: case TokenKind::TK_INT64:
        return true;
    default: return false;
    }
}

TranslationUnit* Parser::parseTranslationUnit() {
    auto* unit = new TranslationUnit();
    unit->loc = currentTok_.loc;

    Node** tail = &unit->extDecls;
    while (currentTok_.kind != TokenKind::TK_END) {
        auto* spec = parseSpecifiers();
        auto* dec = parseDeclarator(DEC_CONCRETE);

        if (currentTok_.kind == TokenKind::TK_LBRACE) {
            auto* func = parseFunction(spec, dec);
            func->next = nullptr;
            *tail = func;
            tail = &func->next;
        } else {
            auto* decl = new Declaration();
            decl->loc = spec->loc;
            decl->specs = spec;
            decl->next = nullptr;
            *tail = decl;
            tail = &decl->next;
            expect(TokenKind::TK_SEMICOLON);
        }
    }
    return unit;
}

Specifiers* Parser::parseSpecifiers() {
    auto* s = new Specifiers();
    s->loc = currentTok_.loc;
    while (isTypeSpecifier(currentTok_.kind)) {
        nextToken();
    }
    return s;
}

Declarator* Parser::parseDeclarator(int /*mode*/) {
    Declarator* d = new NameDeclarator();
    d->loc = currentTok_.loc;

    // Handle pointer declarators (stars)
    while (currentTok_.kind == TokenKind::TK_MUL) {
        nextToken();
    }

    // Parse identifier or parenthesized declarator
    if (currentTok_.kind == TokenKind::TK_LPAREN) {
        nextToken();
        d = parseDeclarator(DEC_ABSTRACT);
        if (currentTok_.kind == TokenKind::TK_RPAREN) {
            nextToken();
        }
    } else if (currentTok_.kind == TokenKind::TK_ID) {
        static_cast<NameDeclarator*>(d)->id = currentTok_.id;
        nextToken();
    }

    // Handle postfix declarators (array, function)
    while (currentTok_.kind == TokenKind::TK_LBRACKET ||
           currentTok_.kind == TokenKind::TK_LPAREN) {
        if (currentTok_.kind == TokenKind::TK_LBRACKET) {
            nextToken();
            if (currentTok_.kind != TokenKind::TK_RBRACKET) {
                // array size expression
                while (currentTok_.kind != TokenKind::TK_RBRACKET &&
                       currentTok_.kind != TokenKind::TK_END) {
                    nextToken();
                }
            }
            expect(TokenKind::TK_RBRACKET);
        } else {
            // function parameters
            nextToken();
            while (currentTok_.kind != TokenKind::TK_RPAREN &&
                   currentTok_.kind != TokenKind::TK_END) {
                nextToken();
            }
            expect(TokenKind::TK_RPAREN);
        }
    }

    return d;
}

TypeName* Parser::parseTypeName() {
    auto* t = new TypeName();
    t->loc = currentTok_.loc;
    t->specs = parseSpecifiers();
    t->dec = parseDeclarator(DEC_ABSTRACT);
    return t;
}

Function* Parser::parseFunction(Specifiers* specs, Declarator* dec) {
    auto* f = new Function();
    f->loc = currentTok_.loc;
    f->specs = specs;
    f->dec = dec;
    f->fdec = nullptr;

    f->stmt = parseCompoundStatement();
    return f;
}

ParameterTypeList* Parser::parseParameterTypeList() {
    auto* p = new ParameterTypeList();
    p->loc = currentTok_.loc;
    return p;
}

ParameterDeclaration* Parser::parseParameterDeclaration() {
    auto* p = new ParameterDeclaration();
    p->loc = currentTok_.loc;
    return p;
}

Initializer* Parser::parseInitializer() {
    auto* i = new Initializer();
    i->loc = currentTok_.loc;
    if (currentTok_.kind == TokenKind::TK_LBRACE) {
        i->lbrace = 1;
        nextToken();
        // parse initializer list
        while (currentTok_.kind != TokenKind::TK_RBRACE &&
               currentTok_.kind != TokenKind::TK_END) {
            parseInitializer();
            if (currentTok_.kind == TokenKind::TK_COMMA) nextToken();
        }
        expect(TokenKind::TK_RBRACE);
    } else {
        i->lbrace = 0;
        i->expr = parseAssignmentExpression();
    }
    return i;
}

InitDeclarator* Parser::parseInitDeclarator() {
    auto* i = new InitDeclarator();
    i->loc = currentTok_.loc;
    // parse declarator + optional initializer
    i->dec = parseDeclarator(DEC_CONCRETE);
    if (currentTok_.kind == TokenKind::TK_ASSIGN) {
        nextToken();
        i->init = parseInitializer();
    }
    return i;
}

EnumSpecifier* Parser::parseEnumSpecifier() {
    auto* e = new EnumSpecifier();
    e->loc = currentTok_.loc;
    return e;
}

Enumerator* Parser::parseEnumerator() {
    auto* e = new Enumerator();
    e->loc = currentTok_.loc;
    return e;
}

StructSpecifier* Parser::parseStructSpecifier() {
    auto* s = new StructSpecifier();
    s->loc = currentTok_.loc;
    return s;
}

UnionSpecifier* Parser::parseUnionSpecifier() {
    auto* s = new UnionSpecifier();
    s->loc = currentTok_.loc;
    return s;
}

StructDeclaration* Parser::parseStructDeclaration() {
    auto* s = new StructDeclaration();
    s->loc = currentTok_.loc;
    return s;
}

StructDeclarator* Parser::parseStructDeclarator() {
    auto* s = new StructDeclarator();
    s->loc = currentTok_.loc;
    return s;
}

Stmt* Parser::parseStatement() {
    switch (currentTok_.kind) {
    case TokenKind::TK_LBRACE:
        return parseCompoundStatement();
    case TokenKind::TK_IF:
        return parseIfStatement();
    case TokenKind::TK_SWITCH:
        return parseSwitchStatement();
    case TokenKind::TK_WHILE:
        return parseWhileStatement();
    case TokenKind::TK_DO:
        return parseDoStatement();
    case TokenKind::TK_FOR:
        return parseForStatement();
    case TokenKind::TK_GOTO:
        return parseGotoStatement();
    case TokenKind::TK_BREAK:
        return parseBreakStatement();
    case TokenKind::TK_CONTINUE:
        return parseContinueStatement();
    case TokenKind::TK_RETURN:
        return parseReturnStatement();
    case TokenKind::TK_CASE:
    case TokenKind::TK_DEFAULT:
        return parseStatement(); // handled by switch, just skip here
    case TokenKind::TK_SEMICOLON:
        nextToken();
        return new ExprStmt();
    case TokenKind::TK_ID:
        // could be label
        lexer_.beginPeek();
        nextToken();
        if (currentTok_.kind == TokenKind::TK_COLON) {
            currentTok_ = lexer_.currentToken();
            return parseLabelStatement();
        }
        // restore lexer state and sync parser cache
        lexer_.endPeek();
        currentTok_ = lexer_.currentToken();
        // fall through to expression statement
    default:
        return parseExpressionStatement();
    }
}

CompoundStmt* Parser::parseCompoundStatement() {
    auto* cs = new CompoundStmt();
    cs->loc = currentTok_.loc;
    if (currentTok_.kind == TokenKind::TK_LBRACE) {
        nextToken(); // consume '{'

        // Parse declarations inside compound statement
        Node** declTail = &cs->decls;
        while (isTypeSpecifier(currentTok_.kind)) {
            auto* decl = parseDeclaration();
            *declTail = decl;
            declTail = &decl->next;
        }

        // Parse statements
        Node** stmtTail = &cs->stmts;
        while (currentTok_.kind != TokenKind::TK_RBRACE &&
               currentTok_.kind != TokenKind::TK_END) {
            auto* stmt = parseStatement();
            *stmtTail = stmt;
            stmtTail = &stmt->next;
        }

        expect(TokenKind::TK_RBRACE);
    }
    return cs;
}

Stmt* Parser::parseIfStatement() {
    auto* s = new IfStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_IF);
    expect(TokenKind::TK_LPAREN);
    s->expr = parseExpression();
    expect(TokenKind::TK_RPAREN);
    s->thenStmt = parseStatement();
    if (currentTok_.kind == TokenKind::TK_ELSE) {
        nextToken();
        s->elseStmt = parseStatement();
    }
    return s;
}

Stmt* Parser::parseSwitchStatement() {
    auto* s = new SwitchStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_SWITCH);
    expect(TokenKind::TK_LPAREN);
    s->expr = parseExpression();
    expect(TokenKind::TK_RPAREN);
    s->stmt = parseStatement();
    return s;
}

Stmt* Parser::parseWhileStatement() {
    auto* s = new WhileStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_WHILE);
    expect(TokenKind::TK_LPAREN);
    s->expr = parseExpression();
    expect(TokenKind::TK_RPAREN);
    s->stmt = parseStatement();
    return s;
}

Stmt* Parser::parseDoStatement() {
    auto* s = new DoStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_DO);
    s->stmt = parseStatement();
    expect(TokenKind::TK_WHILE);
    expect(TokenKind::TK_LPAREN);
    s->expr = parseExpression();
    expect(TokenKind::TK_RPAREN);
    expect(TokenKind::TK_SEMICOLON);
    return s;
}

Stmt* Parser::parseForStatement() {
    auto* s = new ForStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_FOR);
    expect(TokenKind::TK_LPAREN);
    if (currentTok_.kind != TokenKind::TK_SEMICOLON) {
        s->initExpr = parseExpression();
    }
    expect(TokenKind::TK_SEMICOLON);
    if (currentTok_.kind != TokenKind::TK_SEMICOLON) {
        s->expr = parseExpression();
    }
    expect(TokenKind::TK_SEMICOLON);
    if (currentTok_.kind != TokenKind::TK_RPAREN) {
        s->incrExpr = parseExpression();
    }
    expect(TokenKind::TK_RPAREN);
    s->stmt = parseStatement();
    return s;
}

Stmt* Parser::parseGotoStatement() {
    auto* s = new GotoStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_GOTO);
    s->id = currentTok_.id;
    expect(TokenKind::TK_ID);
    expect(TokenKind::TK_SEMICOLON);
    return s;
}

Stmt* Parser::parseBreakStatement() {
    auto* s = new BreakStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_BREAK);
    expect(TokenKind::TK_SEMICOLON);
    return s;
}

Stmt* Parser::parseContinueStatement() {
    auto* s = new ContinueStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_CONTINUE);
    expect(TokenKind::TK_SEMICOLON);
    return s;
}

Stmt* Parser::parseReturnStatement() {
    auto* s = new ReturnStmt();
    s->loc = currentTok_.loc;
    expect(TokenKind::TK_RETURN);
    if (currentTok_.kind != TokenKind::TK_SEMICOLON) {
        s->expr = parseExpression();
    }
    expect(TokenKind::TK_SEMICOLON);
    return s;
}

Stmt* Parser::parseLabelStatement() {
    auto* s = new LabelStmt();
    s->loc = currentTok_.loc;
    s->id = currentTok_.id;
    expect(TokenKind::TK_ID);
    expect(TokenKind::TK_COLON);
    s->stmt = parseStatement();
    return s;
}

Stmt* Parser::parseExpressionStatement() {
    auto* s = new ExprStmt();
    s->loc = currentTok_.loc;
    if (currentTok_.kind != TokenKind::TK_SEMICOLON) {
        s->expr = parseExpression();
    }
    expect(TokenKind::TK_SEMICOLON);
    return s;
}

Declaration* Parser::parseDeclaration() {
    auto* d = new Declaration();
    d->loc = currentTok_.loc;
    d->specs = parseSpecifiers();
    Node** tail = &d->initDecs;
    if (currentTok_.kind != TokenKind::TK_SEMICOLON) {
        do {
            if (currentTok_.kind == TokenKind::TK_COMMA) nextToken();
            auto* initDec = parseInitDeclarator();
            *tail = initDec;
            tail = &initDec->next;
        } while (currentTok_.kind == TokenKind::TK_COMMA);
    }
    expect(TokenKind::TK_SEMICOLON);
    return d;
}

// Expression parsing
Expr* Parser::parseExpression() {
    return parseBinaryExpression(1);
}

Expr* Parser::parseAssignmentExpression() {
    return parseBinaryExpression(2);  // Skip comma (prec=1)
}

Expr* Parser::parseConstantExpression() {
    return parseAssignmentExpression();
}

Expr* Parser::parseBinaryExpression(int minPrec) {
    Expr* left = parseCastExpression();

    while (true) {
        // Check for binary operators
        TokenKind kind = currentTok_.kind;
        int prec = 0;
        switch (kind) {
        case TokenKind::TK_OR: prec = 4; break;
        case TokenKind::TK_AND: prec = 5; break;
        case TokenKind::TK_BITOR: prec = 6; break;
        case TokenKind::TK_BITXOR: prec = 7; break;
        case TokenKind::TK_BITAND: prec = 8; break;
        case TokenKind::TK_EQUAL: case TokenKind::TK_UNEQUAL: prec = 9; break;
        case TokenKind::TK_GREAT: case TokenKind::TK_LESS:
        case TokenKind::TK_GREAT_EQ: case TokenKind::TK_LESS_EQ: prec = 10; break;
        case TokenKind::TK_LSHIFT: case TokenKind::TK_RSHIFT: prec = 11; break;
        case TokenKind::TK_ADD: case TokenKind::TK_SUB: prec = 12; break;
        case TokenKind::TK_MUL: case TokenKind::TK_DIV: case TokenKind::TK_MOD: prec = 13; break;
        case TokenKind::TK_ASSIGN: case TokenKind::TK_ADD_ASSIGN:
        case TokenKind::TK_SUB_ASSIGN: case TokenKind::TK_MUL_ASSIGN:
        case TokenKind::TK_DIV_ASSIGN: case TokenKind::TK_MOD_ASSIGN:
        case TokenKind::TK_LSHIFT_ASSIGN: case TokenKind::TK_RSHIFT_ASSIGN:
        case TokenKind::TK_BITOR_ASSIGN: case TokenKind::TK_BITXOR_ASSIGN:
        case TokenKind::TK_BITAND_ASSIGN: prec = 2; break;
        case TokenKind::TK_COMMA: prec = 1; break;
        default: prec = 0; break;
        }
        if (prec < minPrec) break;

        nextToken();
        Expr* right = parseCastExpression();

        auto* bin = new BinaryExpr();
        bin->loc = left->loc;
        switch (kind) {
        case TokenKind::TK_ADD: bin->op = OpKind::OP_ADD; break;
        case TokenKind::TK_SUB: bin->op = OpKind::OP_SUB; break;
        case TokenKind::TK_MUL: bin->op = OpKind::OP_MUL; break;
        case TokenKind::TK_DIV: bin->op = OpKind::OP_DIV; break;
        case TokenKind::TK_MOD: bin->op = OpKind::OP_MOD; break;
        case TokenKind::TK_LSHIFT: bin->op = OpKind::OP_LSHIFT; break;
        case TokenKind::TK_RSHIFT: bin->op = OpKind::OP_RSHIFT; break;
        case TokenKind::TK_OR: bin->op = OpKind::OP_OR; break;
        case TokenKind::TK_AND: bin->op = OpKind::OP_AND; break;
        case TokenKind::TK_BITOR: bin->op = OpKind::OP_BITOR; break;
        case TokenKind::TK_BITXOR: bin->op = OpKind::OP_BITXOR; break;
        case TokenKind::TK_BITAND: bin->op = OpKind::OP_BITAND; break;
        case TokenKind::TK_EQUAL: bin->op = OpKind::OP_EQUAL; break;
        case TokenKind::TK_UNEQUAL: bin->op = OpKind::OP_UNEQUAL; break;
        case TokenKind::TK_GREAT: bin->op = OpKind::OP_GREAT; break;
        case TokenKind::TK_LESS: bin->op = OpKind::OP_LESS; break;
        case TokenKind::TK_GREAT_EQ: bin->op = OpKind::OP_GREAT_EQ; break;
        case TokenKind::TK_LESS_EQ: bin->op = OpKind::OP_LESS_EQ; break;
        case TokenKind::TK_ASSIGN: bin->op = OpKind::OP_ASSIGN; break;
        case TokenKind::TK_COMMA: bin->op = OpKind::OP_COMMA; break;
        default: bin->op = OpKind::OP_NONE; break;
        }
        bin->left = left;
        bin->right = right;
        left = bin;
    }
    return left;
}

Expr* Parser::parseCastExpression() {
    return parseUnaryExpression();
}

Expr* Parser::parseUnaryExpression() {
    switch (currentTok_.kind) {
    case TokenKind::TK_SUB: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_NEG;
        nextToken();
        u->operand = parseCastExpression();
        return u;
    }
    case TokenKind::TK_NOT: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_NOT;
        nextToken();
        u->operand = parseCastExpression();
        return u;
    }
    case TokenKind::TK_COMP: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_COMP;
        nextToken();
        u->operand = parseCastExpression();
        return u;
    }
    case TokenKind::TK_MUL: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_DEREF;
        nextToken();
        u->operand = parseCastExpression();
        return u;
    }
    case TokenKind::TK_BITAND: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_ADDRESS;
        nextToken();
        u->operand = parseCastExpression();
        return u;
    }
    case TokenKind::TK_ADD: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_POS;
        nextToken();
        u->operand = parseCastExpression();
        return u;
    }
    case TokenKind::TK_INC: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_PREINC;
        nextToken();
        u->operand = parseUnaryExpression();
        return u;
    }
    case TokenKind::TK_DEC: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_PREDEC;
        nextToken();
        u->operand = parseUnaryExpression();
        return u;
    }
    case TokenKind::TK_SIZEOF: {
        auto* u = new UnaryExpr();
        u->loc = currentTok_.loc;
        u->op = OpKind::OP_SIZEOF;
        nextToken();
        if (currentTok_.kind == TokenKind::TK_LPAREN &&
            isTypeSpecifier(currentTok_.kind)) {
            // sizeof(type)
            u->operand = parseUnaryExpression();
        } else {
            u->operand = parseUnaryExpression();
        }
        return u;
    }
    default:
        return parsePostfixExpression(nullptr);
    }
}

Expr* Parser::parsePostfixExpression(Expr* base) {
    Expr* expr = base ? base : parsePrimaryExpression();

    while (true) {
        switch (currentTok_.kind) {
        case TokenKind::TK_LBRACKET: {
            auto* idx = new IndexExpr();
            idx->loc = currentTok_.loc;
            nextToken();
            idx->base = expr;
            idx->index = parseExpression();
            expect(TokenKind::TK_RBRACKET);
            expr = idx;
            break;
        }
        case TokenKind::TK_LPAREN: {
            auto* call = new CallExpr();
            call->loc = currentTok_.loc;
            nextToken();
            call->func = expr;
            call->args = nullptr;
            if (currentTok_.kind != TokenKind::TK_RPAREN) {
                call->args = parseAssignmentExpression();
                Expr** tail = &call->args;
                while (currentTok_.kind == TokenKind::TK_COMMA) {
                    nextToken();
                    auto* arg = parseAssignmentExpression();
                    (*tail)->next = arg;
                    tail = reinterpret_cast<Expr**>(&(*tail)->next);
                }
            }
            expect(TokenKind::TK_RPAREN);
            expr = call;
            break;
        }
        case TokenKind::TK_DOT: {
            auto* m = new MemberExpr();
            m->loc = currentTok_.loc;
            nextToken();
            m->obj = expr;
            m->member = currentTok_.id;
            expect(TokenKind::TK_ID);
            expr = m;
            break;
        }
        case TokenKind::TK_POINTER: {
            auto* m = new PtrMemberExpr();
            m->loc = currentTok_.loc;
            nextToken();
            m->obj = expr;
            m->member = currentTok_.id;
            expect(TokenKind::TK_ID);
            expr = m;
            break;
        }
        case TokenKind::TK_INC: {
            auto* p = new PostfixExpr();
            p->loc = currentTok_.loc;
            p->op = OpKind::OP_POSTINC;
            nextToken();
            p->operand = expr;
            expr = p;
            break;
        }
        case TokenKind::TK_DEC: {
            auto* p = new PostfixExpr();
            p->loc = currentTok_.loc;
            p->op = OpKind::OP_POSTDEC;
            nextToken();
            p->operand = expr;
            expr = p;
            break;
        }
        default:
            return expr;
        }
    }
}

Expr* Parser::parsePrimaryExpression() {
    auto loc = currentTok_.loc;

    switch (currentTok_.kind) {
    case TokenKind::TK_ID: {
        auto* e = new IdentifierExpr();
        e->loc = loc;
        e->name = currentTok_.id;
        nextToken();
        return e;
    }
    case TokenKind::TK_INTCONST:
    case TokenKind::TK_UINTCONST:
    case TokenKind::TK_LONGCONST:
    case TokenKind::TK_ULONGCONST:
    case TokenKind::TK_LLONGCONST:
    case TokenKind::TK_ULLONGCONST:
    case TokenKind::TK_FLOATCONST:
    case TokenKind::TK_DOUBLECONST:
    case TokenKind::TK_LDOUBLECONST: {
        auto* e = new ConstantExpr();
        e->loc = loc;
        e->value = currentTok_.value;
        e->tokenKind = currentTok_.kind;
        nextToken();
        return e;
    }
    case TokenKind::TK_STRING:
    case TokenKind::TK_WIDESTRING: {
        auto* e = new StringExpr();
        e->loc = loc;
        e->str = currentTok_.value.p;
        nextToken();
        return e;
    }
    case TokenKind::TK_LPAREN: {
        nextToken();
        auto* e = parseExpression();
        expect(TokenKind::TK_RPAREN);
        return e;
    }
    default:
        ctx_.diagnostics().error(loc, "unexpected token in expression: " +
            std::string(tokenName(currentTok_.kind)));
        nextToken();
        auto* e = new ConstantExpr();
        e->loc = loc;
        e->value.i[0] = 0;
        e->value.i[1] = 0;
        return e;
    }
}

} // namespace TCC_Pro

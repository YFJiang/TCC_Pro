#include "sema/sema.hpp"

namespace TCC_Pro {

void SemanticChecker::check(TranslationUnit* unit) {
    unit->accept(*this);
}

#define UNIMPL visit
#define VISIT_IMPL(type) void SemanticChecker::visit(type* node) { (void)node; }

void SemanticChecker::visit(ConstantExpr* node) {
    if (node->tokenKind == TokenKind::TK_INTCONST) node->ty = types_.intType();
    else if (node->tokenKind == TokenKind::TK_FLOATCONST) node->ty = types_.floatType();
    else if (node->tokenKind == TokenKind::TK_DOUBLECONST) node->ty = types_.doubleType();
    else node->ty = types_.intType();
}

void SemanticChecker::visit(IdentifierExpr* node) {
    Symbol* sym = syms_.lookupID(node->name);
    if (sym) {
        node->ty = sym->ty;
        node->lvalue = true;
    } else {
        diag_.error(node->loc, std::string("undeclared identifier: ") + node->name);
        node->ty = types_.intType();
    }
}

VISIT_IMPL(StringExpr)

VISIT_IMPL(UnaryExpr)

void SemanticChecker::visit(BinaryExpr* node) {
    if (node->left) node->left->accept(*this);
    if (node->right) node->right->accept(*this);
    
    if (node->left && node->right && node->left->ty && node->right->ty) {
        if (node->left->ty->isArith() && node->right->ty->isArith()) {
            node->ty = types_.commonRealType(node->left->ty, node->right->ty);
            if (!node->ty) node->ty = types_.intType();
        } else {
            node->ty = types_.intType();
        }
    } else {
        node->ty = types_.intType();
    }
}

VISIT_IMPL(ConditionalExpr)
VISIT_IMPL(CastExpr)
VISIT_IMPL(CallExpr)
VISIT_IMPL(IndexExpr)
VISIT_IMPL(MemberExpr)
VISIT_IMPL(PtrMemberExpr)
VISIT_IMPL(PostfixExpr)

void SemanticChecker::visit(AssignExpr* node) {
    if (node->left) node->left->accept(*this);
    if (node->right) node->right->accept(*this);
    
    if (node->left && !node->left->lvalue) {
        diag_.error(node->loc, "lvalue required as left operand of assignment");
    }
    if (node->left) node->ty = node->left->ty;
}

VISIT_IMPL(CommaExpr)

void SemanticChecker::visit(ExprStmt* node) {
    if (node->expr) node->expr->accept(*this);
}

VISIT_IMPL(LabelStmt)
VISIT_IMPL(CaseStmt)
VISIT_IMPL(DefaultStmt)

void SemanticChecker::visit(IfStmt* node) {
    if (node->expr) node->expr->accept(*this);
    if (node->thenStmt) node->thenStmt->accept(*this);
    if (node->elseStmt) node->elseStmt->accept(*this);
}

VISIT_IMPL(SwitchStmt)

void SemanticChecker::visit(WhileStmt* node) {
    if (node->expr) node->expr->accept(*this);
    if (node->stmt) node->stmt->accept(*this);
}

void SemanticChecker::visit(DoStmt* node) {
    if (node->stmt) node->stmt->accept(*this);
    if (node->expr) node->expr->accept(*this);
}

void SemanticChecker::visit(ForStmt* node) {
    if (node->initExpr) node->initExpr->accept(*this);
    if (node->expr) node->expr->accept(*this);
    if (node->incrExpr) node->incrExpr->accept(*this);
    if (node->stmt) node->stmt->accept(*this);
}
VISIT_IMPL(GotoStmt)
VISIT_IMPL(BreakStmt)
VISIT_IMPL(ContinueStmt)

void SemanticChecker::visit(ReturnStmt* node) {
    if (node->expr) node->expr->accept(*this);
}


void SemanticChecker::visit(CompoundStmt* node) {
    syms_.enterScope();
    for (auto* decl = node->decls; decl; decl = decl->next) {
        if (decl) decl->accept(*this);
    }
    for (auto* stmt = node->stmts; stmt; stmt = stmt->next) {
        if (stmt) stmt->accept(*this);
    }
    syms_.exitScope();
}


void SemanticChecker::visit(TranslationUnit* node) {
    for (auto* decl = node->extDecls; decl; decl = decl->next)
        if (decl) decl->accept(*this);
}

void SemanticChecker::visit(Function* node) {
    const char* name = "main";
    if (node->dec) {
        if (auto* nd = dynamic_cast<NameDeclarator*>(node->dec))
            if (nd->id) name = nd->id;
    }
    if (node->fdec) {
        if (node->fdec->id) name = node->fdec->id;
    }
    auto* fsym = static_cast<FunctionSymbol*>(node->fsym);
    if (!fsym) {
        fsym = static_cast<FunctionSymbol*>(syms_.addFunction(name, types_.intType(), 0));
        node->fsym = fsym;
    }
    syms_.currentFunc = fsym;

    syms_.enterScope();
    if (node->stmt) node->stmt->accept(*this);
    syms_.exitScope();
    
    syms_.currentFunc = nullptr;
}



void SemanticChecker::visit(Declaration* node) {
    for (auto* dec = node->initDecs; dec; dec = dec->next) {
        if (dec) dec->accept(*this);
    }
}

VISIT_IMPL(Specifiers)
VISIT_IMPL(TypeName)

void SemanticChecker::visit(InitDeclarator* node) {
    if (node->init) node->init->accept(*this);
    if (node->dec && node->dec->id) {
        syms_.addVariable(node->dec->id, types_.intType(), 0);
    }
}

VISIT_IMPL(Initializer)

VISIT_IMPL(ParameterDeclaration)
VISIT_IMPL(ParameterTypeList)
VISIT_IMPL(EnumSpecifier)
VISIT_IMPL(Enumerator)
VISIT_IMPL(StructSpecifier)
VISIT_IMPL(UnionSpecifier)
VISIT_IMPL(StructDeclaration)
VISIT_IMPL(StructDeclarator)
VISIT_IMPL(TypedefName)
VISIT_IMPL(TokenNode)
VISIT_IMPL(FunctionDeclarator)
VISIT_IMPL(ArrayDeclarator)
VISIT_IMPL(PointerDeclarator)
VISIT_IMPL(NameDeclarator)

// ConstantFolder implementations
Expr* ConstantFolder::fold(Expr* expr) {
    if (expr) expr->accept(*this);
    return result_;
}

void ConstantFolder::visit(ConstantExpr* node) { result_ = node; }
void ConstantFolder::visit(IdentifierExpr* node) { result_ = node; }
void ConstantFolder::visit(StringExpr* node) { result_ = node; }

void ConstantFolder::visit(UnaryExpr* node) {
    if (node->operand) node->operand->accept(*this);
    auto* op = result_;
    // TODO: evaluate constant unary operations
    if (op && op->nodeKind == NodeKind::NK_ConstantExpr) {
        auto* ce = static_cast<ConstantExpr*>(op);
        auto* res = new ConstantExpr();
        res->loc = node->loc;
        res->ty = node->ty;
        switch (node->op) {
        case OpKind::OP_NEG:
            if (node->ty && node->ty->isInteg())
                res->value.i[0] = -ce->value.i[0];
            else
                res->value.d = -ce->value.d;
            break;
        case OpKind::OP_NOT:
            res->value.i[0] = !ce->value.i[0];
            break;
        case OpKind::OP_COMP:
            res->value.i[0] = ~ce->value.i[0];
            break;
        case OpKind::OP_POS:
            res->value = ce->value;
            break;
        case OpKind::OP_ADDRESS:
        case OpKind::OP_DEREF:
        case OpKind::OP_PREINC:
        case OpKind::OP_PREDEC:
        case OpKind::OP_SIZEOF:
        case OpKind::OP_CAST:
            result_ = node; return;
        default: result_ = node; return;
        }
        result_ = res;
    } else {
        result_ = node;
    }
}

void ConstantFolder::visit(BinaryExpr* node) {
    if (node->left) node->left->accept(*this);
    auto* left = result_;
    if (node->right) node->right->accept(*this);
    auto* right = result_;

    if (left && right &&
        left->nodeKind == NodeKind::NK_ConstantExpr &&
        right->nodeKind == NodeKind::NK_ConstantExpr) {
        auto* lc = static_cast<ConstantExpr*>(left);
        auto* rc = static_cast<ConstantExpr*>(right);
        auto* res = new ConstantExpr();
        res->loc = node->loc;
        res->ty = node->ty;
        // TODO: evaluate constant binary operations
        switch (node->op) {
        case OpKind::OP_ADD:
            res->value.i[0] = lc->value.i[0] + rc->value.i[0]; break;
        case OpKind::OP_SUB:
            res->value.i[0] = lc->value.i[0] - rc->value.i[0]; break;
        case OpKind::OP_MUL:
            res->value.i[0] = lc->value.i[0] * rc->value.i[0]; break;
        case OpKind::OP_DIV:
            res->value.i[0] = lc->value.i[0] / rc->value.i[0]; break;
        case OpKind::OP_MOD:
            res->value.i[0] = lc->value.i[0] % rc->value.i[0]; break;
        default: result_ = node; return;
        }
        result_ = res;
    } else {
        result_ = node;
    }
}

#define CF_VISIT_IMPL(type) void ConstantFolder::visit(type* node) { (void)node; }

void ConstantFolder::visit(ConditionalExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(CastExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(CallExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(IndexExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(MemberExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(PtrMemberExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(PostfixExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(AssignExpr* node) { (void)node; result_ = node; }
void ConstantFolder::visit(CommaExpr* node) { (void)node; result_ = node; }

CF_VISIT_IMPL(ExprStmt)
CF_VISIT_IMPL(LabelStmt)
CF_VISIT_IMPL(CaseStmt)
CF_VISIT_IMPL(DefaultStmt)
CF_VISIT_IMPL(IfStmt)
CF_VISIT_IMPL(SwitchStmt)
CF_VISIT_IMPL(WhileStmt)
CF_VISIT_IMPL(DoStmt)
CF_VISIT_IMPL(ForStmt)
CF_VISIT_IMPL(GotoStmt)
CF_VISIT_IMPL(BreakStmt)
CF_VISIT_IMPL(ContinueStmt)
CF_VISIT_IMPL(ReturnStmt)
CF_VISIT_IMPL(CompoundStmt)
CF_VISIT_IMPL(TranslationUnit)
CF_VISIT_IMPL(Function)
CF_VISIT_IMPL(Declaration)
CF_VISIT_IMPL(Specifiers)
CF_VISIT_IMPL(TypeName)
CF_VISIT_IMPL(InitDeclarator)
CF_VISIT_IMPL(Initializer)
CF_VISIT_IMPL(ParameterDeclaration)
CF_VISIT_IMPL(ParameterTypeList)
CF_VISIT_IMPL(EnumSpecifier)
CF_VISIT_IMPL(Enumerator)
CF_VISIT_IMPL(StructSpecifier)
CF_VISIT_IMPL(UnionSpecifier)
CF_VISIT_IMPL(StructDeclaration)
CF_VISIT_IMPL(StructDeclarator)
CF_VISIT_IMPL(TypedefName)
CF_VISIT_IMPL(TokenNode)
CF_VISIT_IMPL(FunctionDeclarator)
CF_VISIT_IMPL(ArrayDeclarator)
CF_VISIT_IMPL(PointerDeclarator)
CF_VISIT_IMPL(NameDeclarator)

} // namespace TCC_Pro

#include "sema/sema.hpp"

namespace TCC_Pro {

void SemanticChecker::check(TranslationUnit* unit) {
    unit->accept(*this);
}

#define UNIMPL visit
#define VISIT_IMPL(type) void SemanticChecker::visit(type* node) { (void)node; }

VISIT_IMPL(ConstantExpr)
VISIT_IMPL(IdentifierExpr)
VISIT_IMPL(StringExpr)
VISIT_IMPL(UnaryExpr)
VISIT_IMPL(BinaryExpr)
VISIT_IMPL(ConditionalExpr)
VISIT_IMPL(CastExpr)
VISIT_IMPL(CallExpr)
VISIT_IMPL(IndexExpr)
VISIT_IMPL(MemberExpr)
VISIT_IMPL(PtrMemberExpr)
VISIT_IMPL(PostfixExpr)
VISIT_IMPL(AssignExpr)
VISIT_IMPL(CommaExpr)
VISIT_IMPL(ExprStmt)
VISIT_IMPL(LabelStmt)
VISIT_IMPL(CaseStmt)
VISIT_IMPL(DefaultStmt)
VISIT_IMPL(IfStmt)
VISIT_IMPL(SwitchStmt)
VISIT_IMPL(WhileStmt)
VISIT_IMPL(DoStmt)
VISIT_IMPL(ForStmt)
VISIT_IMPL(GotoStmt)
VISIT_IMPL(BreakStmt)
VISIT_IMPL(ContinueStmt)
VISIT_IMPL(ReturnStmt)

void SemanticChecker::visit(CompoundStmt* node) {
    (void)node;
}

void SemanticChecker::visit(TranslationUnit* node) {
    for (auto* decl = node->extDecls; decl; decl = decl->next)
        if (decl) decl->accept(*this);
}

void SemanticChecker::visit(Function* node) {
    if (node->stmt) node->stmt->accept(*this);
}

VISIT_IMPL(Declaration)
VISIT_IMPL(Specifiers)
VISIT_IMPL(TypeName)
VISIT_IMPL(InitDeclarator)
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

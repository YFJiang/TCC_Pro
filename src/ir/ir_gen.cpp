#include "ir/ir_gen.hpp"
#include <cassert>
#include <cstring>

namespace TCC_Pro {

IRGenerator::IRGenerator(SymbolTable& syms, TypeContext& types, Diagnostics& diag)
    : syms_(syms), types_(types), diag_(diag) {}

// ============= IR Helpers =============

BBlock* IRGenerator::createBBlock() {
    auto* bb = new BBlock();
    bb->insth.opcode = OpCode::NOP;
    bb->insth.prev = bb->insth.next = &bb->insth;
    return bb;
}

void IRGenerator::startBBlock(BBlock* bb) {
    if (currentBB_) {
        currentBB_->next = bb;
        bb->prev = currentBB_;
        auto* lasti = currentBB_->insth.prev;
        if (lasti->opcode != OpCode::JMP && lasti->opcode != OpCode::IJMP
            && lasti->opcode != OpCode::RET)
            drawCFGEdge(currentBB_, bb);
    }
    currentBB_ = bb;
}

void IRGenerator::appendInst(IRInst* inst) {
    assert(currentBB_);
    currentBB_->insth.prev->next = inst;
    inst->prev = currentBB_->insth.prev;
    inst->next = &currentBB_->insth;
    currentBB_->insth.prev = inst;
    currentBB_->ninst++;
}

void IRGenerator::genMove(Type* ty, Symbol* dst, Symbol* src) {
    auto* inst = new IRInst();
    dst->ref++; src->ref++;
    inst->ty = ty;
    inst->opcode = OpCode::MOV;
    inst->opds[0] = dst;
    inst->opds[1] = src;
    appendInst(inst);
}

void IRGenerator::genIndirectMove(Type* ty, Symbol* dst, Symbol* src) {
    auto* inst = new IRInst();
    dst->ref++; src->ref++;
    inst->ty = ty;
    inst->opcode = OpCode::IMOV;
    inst->opds[0] = dst;
    inst->opds[1] = src;
    appendInst(inst);
}

void IRGenerator::genAssign(Type* ty, Symbol* dst, OpCode opcode, Symbol* src1, Symbol* src2) {
    auto* inst = new IRInst();
    dst->ref++; src1->ref++;
    if (src2) src2->ref++;
    inst->ty = ty;
    inst->opcode = opcode;
    inst->opds[0] = dst;
    inst->opds[1] = src1;
    inst->opds[2] = src2;
    appendInst(inst);
}

void IRGenerator::genBranch(Type* ty, BBlock* dstBB, OpCode opcode, Symbol* src1, Symbol* src2) {
    auto* inst = new IRInst();
    dstBB->ref++; src1->ref++;
    if (src2) src2->ref++;
    drawCFGEdge(currentBB_, dstBB);
    inst->ty = ty;
    inst->opcode = opcode;
    inst->opds[0] = reinterpret_cast<Symbol*>(dstBB);
    inst->opds[1] = src1;
    inst->opds[2] = src2;
    appendInst(inst);
}

void IRGenerator::genJump(BBlock* dstBB) {
    auto* inst = new IRInst();
    dstBB->ref++;
    drawCFGEdge(currentBB_, dstBB);
    inst->ty = types_.voidType();
    inst->opcode = OpCode::JMP;
    inst->opds[0] = reinterpret_cast<Symbol*>(dstBB);
    appendInst(inst);
}

void IRGenerator::genReturn(Type* ty, Symbol* src) {
    auto* inst = new IRInst();
    src->ref++;
    inst->ty = ty;
    inst->opcode = OpCode::RET;
    inst->opds[0] = src;
    appendInst(inst);
}

void IRGenerator::genCall(Type* ty, Symbol* recv, Symbol* faddr, std::vector<Symbol*> args) {
    auto* inst = new IRInst();
    if (recv) recv->ref++;
    faddr->ref++;
    for (auto* a : args) a->ref++;
    inst->ty = ty;
    inst->opcode = OpCode::CALL;
    inst->opds[0] = recv;
    inst->opds[1] = faddr;
    // Store args vector on heap for persistence
    auto* savedArgs = new std::vector<Symbol*>(std::move(args));
    inst->opds[2] = reinterpret_cast<Symbol*>(savedArgs);
    appendInst(inst);
}

void IRGenerator::drawCFGEdge(BBlock* from, BBlock* to) {
    auto* e = new CFGEdge();
    e->bb = to;
    e->next = from->succs;
    from->succs = e;
    from->nsucc++;

    auto* pe = new CFGEdge();
    pe->bb = from;
    pe->next = to->preds;
    to->preds = pe;
    to->npred++;
}

Symbol* IRGenerator::simplify(Type* ty, OpCode op, Symbol* src1, Symbol* src2) {
    return tryAddValue(ty, op, src1, src2);
}

Symbol* IRGenerator::addressOf(Symbol* sym) {
    sym->addressed = 1;
    return tryAddValue(types_.pointerType(), OpCode::ADDR, sym, nullptr);
}

Symbol* IRGenerator::deref(Type* ty, Symbol* addr) {
    if (addr->kind == SK_Temp) {
        auto* v = static_cast<VariableSymbol*>(addr);
        if (v->def && v->def->op == static_cast<int>(OpCode::ADDR))
            return v->def->src1;
    }
    auto* tmp = syms_.createTemp(ty);
    genAssign(ty, tmp, OpCode::DEREF, addr, nullptr);
    return tmp;
}

static unsigned hashOp(Symbol* s1, Symbol* s2, int op) {
    return (reinterpret_cast<uintptr_t>(s1) + reinterpret_cast<uintptr_t>(s2) + op) & 15;
}

Symbol* IRGenerator::tryAddValue(Type* ty, OpCode op, Symbol* src1, Symbol* src2) {
    int h = hashOp(src1, src2, static_cast<int>(op));
    auto* def = currentFunc_ ? currentFunc_->valNumTable[h] : nullptr;

    if (op != OpCode::ADDR && (src1->addressed || (src2 && src2->addressed)))
        goto new_temp;

    while (def) {
        if (def->op == static_cast<int>(op) && def->src1 == src1 && def->src2 == src2)
            break;
        def = def->link;
    }

    if (def && def->ownBB == reinterpret_cast<BBlock*>(currentBB_) && def->dst)
        return def->dst;

new_temp:
    {
        auto* t = syms_.createTemp(ty);
        genAssign(ty, t, op, src1, src2);
        return t;
    }
}

// ============= Expression Translation =============

Symbol* IRGenerator::translateExpr(Expr* expr) {
    if (!expr) return nullptr;
    expr->accept(*this);
    return resultSym_;
}

Symbol* IRGenerator::translatePrimaryExpr(Expr* expr) {
    if (auto* c = dynamic_cast<ConstantExpr*>(expr)) {
        Value v = c->value;
        auto* ty = expr->ty ? expr->ty : types_.intType();
        if (ty->isPtr() && v.i[0] == 0 && v.i[1] == 0)
            return syms_.intConstant(0);
        return syms_.addConstant(ty, v);
    }
    if (auto* s = dynamic_cast<StringExpr*>(expr)) {
        // Create a string symbol and return its address
        auto* strSym = syms_.addString(types_.charType(), s->str);
        return addressOf(strSym);
    }
    if (auto* id = dynamic_cast<IdentifierExpr*>(expr)) {
        auto* sym = syms_.lookupID(id->name);
        if (sym) {
            if (expr->isArray || expr->isFunc)
                return addressOf(sym);
            return sym;
        }
        auto* var = syms_.addVariable(id->name, types_.intType(), 1);
        var->defined = 0;
        if (expr->isFunc) var->kind = SK_Function;
        return var;
    }
    return nullptr;
}

Symbol* IRGenerator::translateBranchExpr(Expr* expr) {
    auto* t = syms_.createTemp(expr->ty);
    auto* nextBB = createBBlock();
    auto* trueBB = createBBlock();
    auto* falseBB = createBBlock();

    translateBranch(expr, trueBB, falseBB);

    startBBlock(falseBB);
    genMove(expr->ty, t, syms_.intConstant(0));
    genJump(nextBB);

    startBBlock(trueBB);
    genMove(expr->ty, t, syms_.intConstant(1));

    startBBlock(nextBB);
    return t;
}

void IRGenerator::translateBranch(Expr* expr, BBlock* trueBB, BBlock* falseBB) {
    if (auto* bin = dynamic_cast<BinaryExpr*>(expr)) {
        switch (bin->op) {
        case OpKind::OP_AND: {
            auto* rtestBB = createBBlock();
            translateBranch(bin->left, rtestBB, falseBB);
            startBBlock(rtestBB);
            translateBranch(bin->right, trueBB, falseBB);
            return;
        }
        case OpKind::OP_OR: {
            auto* rtestBB = createBBlock();
            translateBranch(bin->left, trueBB, rtestBB);
            startBBlock(rtestBB);
            translateBranch(bin->right, trueBB, falseBB);
            return;
        }
        case OpKind::OP_EQUAL: case OpKind::OP_UNEQUAL:
        case OpKind::OP_GREAT: case OpKind::OP_LESS:
        case OpKind::OP_GREAT_EQ: case OpKind::OP_LESS_EQ: {
            auto* src1 = translateExpr(bin->left);
            auto* src2 = translateExpr(bin->right);
            OpCode oc;
            switch (bin->op) {
            case OpKind::OP_EQUAL: oc = OpCode::JE; break;
            case OpKind::OP_UNEQUAL: oc = OpCode::JNE; break;
            case OpKind::OP_GREAT: oc = OpCode::JG; break;
            case OpKind::OP_LESS: oc = OpCode::JL; break;
            case OpKind::OP_GREAT_EQ: oc = OpCode::JGE; break;
            case OpKind::OP_LESS_EQ: oc = OpCode::JLE; break;
            default: oc = OpCode::JE; break;
            }
            genBranch(bin->left->ty, trueBB, oc, src1, src2);
            return;
        }
        default: break;
        }
    }
    if (auto* u = dynamic_cast<UnaryExpr*>(expr)) {
        if (u->op == OpKind::OP_NOT) {
            auto* src1 = translateExpr(u->operand);
            genBranch(u->operand->ty, falseBB, OpCode::JNZ, src1, nullptr);
            return;
        }
    }
    if (auto* c = dynamic_cast<ConstantExpr*>(expr)) {
        if (!(c->value.i[0] == 0 && c->value.i[1] == 0))
            genJump(trueBB);
        return;
    }
    // Default: evaluate and branch on result
    auto* src1 = translateExpr(expr);
    if (src1 && src1->kind == SK_Constant) {
        if (!(src1->val.i[0] == 0 && src1->val.i[1] == 0))
            genJump(trueBB);
    } else {
        genBranch(expr->ty, trueBB, OpCode::JNZ, src1, nullptr);
    }
}

Symbol* IRGenerator::translateCallExpr(CallExpr* expr) {
    if (!expr->func) return nullptr;
    expr->func->isFunc = false;
    auto* faddr = translateExpr(expr->func);

    // Collect args
    std::vector<Symbol*> args;
    if (expr->args) {
        args.push_back(translateExpr(expr->args));
        for (auto* arg = static_cast<Expr*>(expr->args->next); arg;
             arg = static_cast<Expr*>(arg->next)) {
            args.push_back(translateExpr(arg));
        }
    }

    // For printf, create string argument symbol
    for (size_t i = 0; i < args.size(); i++) {
        if (!args[i]) {
            args[i] = syms_.intConstant(0);
        }
    }

    Symbol* recv = nullptr;
    if (expr->ty && !expr->ty->isVoid())
        recv = syms_.createTemp(expr->ty);
    genCall(expr->ty, recv, faddr, args);
    return recv;
}

Symbol* IRGenerator::translateMemberAccess(Expr* expr) {
    if (auto* pm = dynamic_cast<PtrMemberExpr*>(expr)) {
        auto* addr = translateExpr(pm->obj);
        return deref(pm->ty, addr);
    }
    if (auto* m = dynamic_cast<MemberExpr*>(expr)) {
        auto* obj = translateExpr(m->obj);
        if (expr->isArray)
            return addressOf(obj);
        return obj;
    }
    return nullptr;
}

Symbol* IRGenerator::translateArrayIndex(IndexExpr* expr) {
    auto* base = translateExpr(expr->base);
    auto* idx = translateExpr(expr->index);
    auto* addr = simplify(types_.pointerType(), OpCode::ADD, base, idx);
    auto* dst = deref(expr->ty, addr);
    return expr->isArray ? addressOf(dst) : dst;
}

Symbol* IRGenerator::translateIncrement(Expr* expr, bool isPost) {
    auto* operand = translateExpr(static_cast<UnaryExpr*>(expr)->operand);
    OpCode oc = OpCode::INC;
    // TODO: handle DEC
    auto* result = simplify(operand->ty, oc, operand, nullptr);
    genMove(operand->ty, operand, result);
    return operand;
}

Symbol* IRGenerator::translateAssign(AssignExpr* expr) {
    auto* dst = translateExpr(expr->left);
    auto* src = translateExpr(expr->right);
    genMove(expr->ty, dst, src);
    return dst;
}

Symbol* IRGenerator::translateUnary(UnaryExpr* expr) {
    switch (expr->op) {
    case OpKind::OP_NOT:
        return translateBranchExpr(expr);
    case OpKind::OP_NEG: {
        auto* src = translateExpr(expr->operand);
        return simplify(expr->ty, OpCode::NEG, src, nullptr);
    }
    case OpKind::OP_COMP: {
        auto* src = translateExpr(expr->operand);
        return simplify(expr->ty, OpCode::BCOM, src, nullptr);
    }
    case OpKind::OP_ADDRESS: {
        auto* src = translateExpr(expr->operand);
        return addressOf(src);
    }
    case OpKind::OP_DEREF: {
        auto* src = translateExpr(expr->operand);
        return deref(expr->ty, src);
    }
    case OpKind::OP_PREINC: case OpKind::OP_PREDEC:
        return translateIncrement(expr, false);
    case OpKind::OP_CAST: {
        auto* src = translateExpr(expr->operand);
        if (expr->ty == expr->operand->ty) return src;
        auto* dst = syms_.createTemp(expr->ty);
        genAssign(expr->ty, dst, OpCode::MOV, src, nullptr);
        return dst;
    }
    default: break;
    }
    return translateExpr(expr->operand);
}

Symbol* IRGenerator::translateBinary(BinaryExpr* expr) {
    switch (expr->op) {
    case OpKind::OP_OR: case OpKind::OP_AND:
    case OpKind::OP_EQUAL: case OpKind::OP_UNEQUAL:
    case OpKind::OP_GREAT: case OpKind::OP_LESS:
    case OpKind::OP_GREAT_EQ: case OpKind::OP_LESS_EQ:
        return translateBranchExpr(expr);
    default: break;
    }
    auto* src1 = translateExpr(expr->left);
    auto* src2 = translateExpr(expr->right);
    OpCode oc;
    switch (expr->op) {
    case OpKind::OP_ADD: oc = OpCode::ADD; break;
    case OpKind::OP_SUB: oc = OpCode::SUB; break;
    case OpKind::OP_MUL: oc = OpCode::MUL; break;
    case OpKind::OP_DIV: oc = OpCode::DIV; break;
    case OpKind::OP_MOD: oc = OpCode::MOD; break;
    case OpKind::OP_LSHIFT: oc = OpCode::LSH; break;
    case OpKind::OP_RSHIFT: oc = OpCode::RSH; break;
    case OpKind::OP_BITOR: oc = OpCode::BOR; break;
    case OpKind::OP_BITXOR: oc = OpCode::BXOR; break;
    case OpKind::OP_BITAND: oc = OpCode::BAND; break;
    default: return src1;
    }
    return simplify(expr->ty, oc, src1, src2);
}

Symbol* IRGenerator::translateConditional(ConditionalExpr* expr) {
    Symbol* t = nullptr;
    if (expr->ty && !expr->ty->isVoid())
        t = syms_.createTemp(expr->ty);
    auto* trueBB = createBBlock();
    auto* falseBB = createBBlock();
    auto* nextBB = createBBlock();

    translateBranch(expr->cond, trueBB, falseBB);

    startBBlock(falseBB);
    auto* t2 = translateExpr(expr->elseExpr);
    if (t2) genMove(expr->ty, t, t2);
    genJump(nextBB);

    startBBlock(trueBB);
    auto* t1 = translateExpr(expr->thenExpr);
    if (t1) genMove(expr->ty, t, t1);

    startBBlock(nextBB);
    return t;
}

Symbol* IRGenerator::translateComma(CommaExpr* expr) {
    translateExpr(expr->left);
    return translateExpr(expr->right);
}

#define GEN_VISIT_EXPR(cls, func) \
    void IRGenerator::visit(cls* node) { resultSym_ = func(node); }

GEN_VISIT_EXPR(ConstantExpr, translatePrimaryExpr)
GEN_VISIT_EXPR(IdentifierExpr, translatePrimaryExpr)
GEN_VISIT_EXPR(StringExpr, translatePrimaryExpr)
GEN_VISIT_EXPR(UnaryExpr, translateUnary)
GEN_VISIT_EXPR(BinaryExpr, translateBinary)
GEN_VISIT_EXPR(ConditionalExpr, translateConditional)
GEN_VISIT_EXPR(CommaExpr, translateComma)
GEN_VISIT_EXPR(AssignExpr, translateAssign)
GEN_VISIT_EXPR(CallExpr, translateCallExpr)
GEN_VISIT_EXPR(IndexExpr, translateArrayIndex)
GEN_VISIT_EXPR(MemberExpr, translateMemberAccess)
GEN_VISIT_EXPR(PtrMemberExpr, translateMemberAccess)

void IRGenerator::visit(CastExpr* node) {
    auto* src = translateExpr(node->operand);
    if (node->ty && node->operand->ty && node->ty != node->operand->ty) {
        auto* dst = syms_.createTemp(node->ty);
        genAssign(node->ty, dst, OpCode::MOV, src, nullptr);
        resultSym_ = dst;
    } else {
        resultSym_ = src;
    }
}

void IRGenerator::visit(PostfixExpr* node) {
    if (node->op == OpKind::OP_POSTINC || node->op == OpKind::OP_POSTDEC)
        resultSym_ = translateIncrement(node, true);
    else
        resultSym_ = translateExpr(node->operand);
}

// ============= Statement Translation =============

void IRGenerator::visit(ExprStmt* node) {
    if (node->expr) translateExpr(node->expr);
}

void IRGenerator::visit(ReturnStmt* node) {
    Symbol* src;
    if (node->expr) {
        src = translateExpr(node->expr);
        auto* ty = node->expr->ty ? node->expr->ty : types_.intType();
        genReturn(ty, src);
    } else {
        src = syms_.intConstant(0);
        genReturn(types_.intType(), src);
    }
}

void IRGenerator::visit(IfStmt* node) {
    auto* trueBB = createBBlock();
    auto* falseBB = createBBlock();
    auto* nextBB = createBBlock();

    translateBranch(node->expr, trueBB, falseBB);

    startBBlock(trueBB);
    if (node->thenStmt) node->thenStmt->accept(*this);
    genJump(nextBB);

    startBBlock(falseBB);
    if (node->elseStmt) node->elseStmt->accept(*this);

    startBBlock(nextBB);
}

void IRGenerator::visit(WhileStmt* node) {
    auto* loopBB = createBBlock();
    auto* bodyBB = createBBlock();
    auto* nextBB = createBBlock();
    node->loopBB = reinterpret_cast<void*>(loopBB);
    node->contBB = reinterpret_cast<void*>(bodyBB);
    node->nextBB = reinterpret_cast<void*>(nextBB);

    genJump(loopBB);
    startBBlock(loopBB);
    translateBranch(node->expr, bodyBB, nextBB);

    startBBlock(bodyBB);
    if (node->stmt) node->stmt->accept(*this);
    genJump(loopBB);

    startBBlock(nextBB);
}

void IRGenerator::visit(DoStmt* node) {
    auto* bodyBB = createBBlock();
    auto* loopBB = createBBlock();
    auto* nextBB = createBBlock();

    genJump(bodyBB);
    startBBlock(bodyBB);
    if (node->stmt) node->stmt->accept(*this);
    genJump(loopBB);

    startBBlock(loopBB);
    translateBranch(node->expr, bodyBB, nextBB);

    startBBlock(nextBB);
}

void IRGenerator::visit(ForStmt* node) {
    if (node->initExpr) translateExpr(node->initExpr);
    auto* testBB = createBBlock();
    auto* bodyBB = createBBlock();
    auto* incrBB = createBBlock();
    auto* nextBB = createBBlock();
    node->testBB = reinterpret_cast<void*>(testBB);
    node->contBB = reinterpret_cast<void*>(incrBB);
    node->nextBB = reinterpret_cast<void*>(nextBB);

    genJump(testBB);
    startBBlock(testBB);
    if (node->expr)
        translateBranch(node->expr, bodyBB, nextBB);
    else
        genJump(bodyBB);

    startBBlock(bodyBB);
    if (node->stmt) node->stmt->accept(*this);

    startBBlock(incrBB);
    if (node->incrExpr) translateExpr(node->incrExpr);
    genJump(testBB);

    startBBlock(nextBB);
}

void IRGenerator::visit(CompoundStmt* node) {
    // Process declarations first (variable init)
    for (auto* d = node->decls; d; d = d->next)
        if (d) d->accept(*this);

    // Process statements
    for (auto* s = node->stmts; s; s = s->next)
        if (s) s->accept(*this);
}

void IRGenerator::visit(Declaration* node) {
    for (auto* initDec = node->initDecs; initDec; initDec = initDec->next) {
        if (auto* idc = dynamic_cast<InitDeclarator*>(initDec)) {
            if (idc->dec) {
                auto* nd = dynamic_cast<NameDeclarator*>(idc->dec);
                const char* name = nd ? (nd->id ? nd->id : "") : "";
                if (!name || !*name) continue;
                auto* var = syms_.addVariable(name, types_.intType(), 0);
                if (idc->init && idc->init->expr) {
                    auto* val = translateExpr(idc->init->expr);
                    if (val && var) {
                        genMove(types_.intType(), var, val);
                    }
                }
            }
        }
    }
}

void IRGenerator::visit(GotoStmt* node) {
    // stub
}

void IRGenerator::visit(BreakStmt* node) {
    // stub
}

void IRGenerator::visit(ContinueStmt* node) {
    // stub
}

void IRGenerator::visit(LabelStmt* node) { if (node->stmt) node->stmt->accept(*this); }
void IRGenerator::visit(CaseStmt* node) { if (node->stmt) node->stmt->accept(*this); }
void IRGenerator::visit(DefaultStmt* node) { if (node->stmt) node->stmt->accept(*this); }
void IRGenerator::visit(SwitchStmt* node) { if (node->stmt) node->stmt->accept(*this); }

// ============= Declaration Translation =============

void IRGenerator::visit(Function* node) {
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
    fsym->defined = 1;
    currentFunc_ = fsym;
    syms_.currentFunc = fsym;
    functions_.push_back(fsym);

    auto* entryBB = createBBlock();
    auto* exitBB = createBBlock();
    fsym->entryBB = reinterpret_cast<BBlock*>(entryBB);
    fsym->exitBB = reinterpret_cast<BBlock*>(exitBB);

    currentBB_ = nullptr;
    startBBlock(entryBB);

    syms_.enterScope();  // Enter function scope

    if (node->stmt) node->stmt->accept(*this);

    // If body doesn't end with return, add one (reuse exitBB)
    auto* lasti = currentBB_ ? currentBB_->insth.prev : nullptr;
    if (!lasti || (lasti->opcode != OpCode::RET && lasti->opcode != OpCode::JMP)) {
        startBBlock(exitBB);
        genReturn(types_.intType(), syms_.intConstant(0));
    }

    syms_.exitScope();   // Exit function scope
    currentFunc_ = nullptr;
    syms_.currentFunc = nullptr;
}

void IRGenerator::visit(TranslationUnit* node) {
    for (auto* decl = node->extDecls; decl; decl = decl->next) {
        if (decl) decl->accept(*this);
    }
}

void IRGenerator::generate(TranslationUnit* unit) {
    unit->accept(*this);
}

void IRGenerator::visit(Specifiers* node) { (void)node; }
void IRGenerator::visit(TypeName* node) { (void)node; }
void IRGenerator::visit(InitDeclarator* node) { (void)node; }
void IRGenerator::visit(Initializer* node) { (void)node; }
void IRGenerator::visit(ParameterDeclaration* node) { (void)node; }
void IRGenerator::visit(ParameterTypeList* node) { (void)node; }
void IRGenerator::visit(EnumSpecifier* node) { (void)node; }
void IRGenerator::visit(Enumerator* node) { (void)node; }
void IRGenerator::visit(StructSpecifier* node) { (void)node; }
void IRGenerator::visit(UnionSpecifier* node) { (void)node; }
void IRGenerator::visit(StructDeclaration* node) { (void)node; }
void IRGenerator::visit(StructDeclarator* node) { (void)node; }
void IRGenerator::visit(TypedefName* node) { (void)node; }
void IRGenerator::visit(TokenNode* node) { (void)node; }
void IRGenerator::visit(FunctionDeclarator* node) { (void)node; }
void IRGenerator::visit(ArrayDeclarator* node) { (void)node; }
void IRGenerator::visit(PointerDeclarator* node) { (void)node; }
void IRGenerator::visit(NameDeclarator* node) { (void)node; }

} // namespace TCC_Pro

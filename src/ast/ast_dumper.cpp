#include "ast/ast_dumper.hpp"

namespace TCC_Pro {

void AstDumper::indent() {
    for (int i = 0; i < depth_; ++i) os_ << "  ";
}

#define DUMP_BEGIN(kind) indent(); os_ << kind << "\n"; depth_++;
#define DUMP_END() depth_--;

void AstDumper::visit(ConstantExpr* node)         { DUMP_BEGIN("ConstantExpr"); DUMP_END(); }
void AstDumper::visit(IdentifierExpr* node)       { DUMP_BEGIN("IdentifierExpr(" << (node->name ? node->name : "?") << ")"); DUMP_END(); }
void AstDumper::visit(StringExpr* node)           { DUMP_BEGIN("StringExpr"); DUMP_END(); }
void AstDumper::visit(UnaryExpr* node)            { DUMP_BEGIN("UnaryExpr " << opName(node->op)); DUMP_END(); }
void AstDumper::visit(BinaryExpr* node)           { DUMP_BEGIN("BinaryExpr " << opName(node->op)); DUMP_END(); }
void AstDumper::visit(ConditionalExpr* node)      { DUMP_BEGIN("ConditionalExpr"); DUMP_END(); }
void AstDumper::visit(CastExpr* node)             { DUMP_BEGIN("CastExpr"); DUMP_END(); }
void AstDumper::visit(CallExpr* node)             { DUMP_BEGIN("CallExpr"); DUMP_END(); }
void AstDumper::visit(IndexExpr* node)            { DUMP_BEGIN("IndexExpr"); DUMP_END(); }
void AstDumper::visit(MemberExpr* node)           { DUMP_BEGIN("MemberExpr " << (node->member ? node->member : "?")); DUMP_END(); }
void AstDumper::visit(PtrMemberExpr* node)        { DUMP_BEGIN("PtrMemberExpr " << (node->member ? node->member : "?")); DUMP_END(); }
void AstDumper::visit(PostfixExpr* node)          { DUMP_BEGIN("PostfixExpr " << opName(node->op)); DUMP_END(); }
void AstDumper::visit(AssignExpr* node)           { DUMP_BEGIN("AssignExpr " << opName(node->op)); DUMP_END(); }
void AstDumper::visit(CommaExpr* node)            { DUMP_BEGIN("CommaExpr"); DUMP_END(); }

void AstDumper::visit(ExprStmt* node)             { DUMP_BEGIN("ExprStmt"); DUMP_END(); }
void AstDumper::visit(LabelStmt* node)            { DUMP_BEGIN("LabelStmt " << (node->id ? node->id : "?")); DUMP_END(); }
void AstDumper::visit(CaseStmt* node)             { DUMP_BEGIN("CaseStmt"); DUMP_END(); }
void AstDumper::visit(DefaultStmt* node)          { DUMP_BEGIN("DefaultStmt"); DUMP_END(); }
void AstDumper::visit(IfStmt* node)               { DUMP_BEGIN("IfStmt"); DUMP_END(); }
void AstDumper::visit(SwitchStmt* node)           { DUMP_BEGIN("SwitchStmt"); DUMP_END(); }
void AstDumper::visit(WhileStmt* node)            { DUMP_BEGIN("WhileStmt"); DUMP_END(); }
void AstDumper::visit(DoStmt* node)               { DUMP_BEGIN("DoStmt"); DUMP_END(); }
void AstDumper::visit(ForStmt* node)              { DUMP_BEGIN("ForStmt"); DUMP_END(); }
void AstDumper::visit(GotoStmt* node)             { DUMP_BEGIN("GotoStmt " << (node->id ? node->id : "?")); DUMP_END(); }
void AstDumper::visit(BreakStmt* node)            { DUMP_BEGIN("BreakStmt"); DUMP_END(); }
void AstDumper::visit(ContinueStmt* node)         { DUMP_BEGIN("ContinueStmt"); DUMP_END(); }
void AstDumper::visit(ReturnStmt* node)           { DUMP_BEGIN("ReturnStmt"); DUMP_END(); }
void AstDumper::visit(CompoundStmt* node)         { DUMP_BEGIN("CompoundStmt"); DUMP_END(); }

void AstDumper::visit(TranslationUnit* node)      { DUMP_BEGIN("TranslationUnit"); DUMP_END(); }
void AstDumper::visit(Function* node)             { DUMP_BEGIN("Function"); DUMP_END(); }
void AstDumper::visit(Declaration* node)          { DUMP_BEGIN("Declaration"); DUMP_END(); }
void AstDumper::visit(Specifiers* node)           { DUMP_BEGIN("Specifiers"); DUMP_END(); }
void AstDumper::visit(TypeName* node)             { DUMP_BEGIN("TypeName"); DUMP_END(); }
void AstDumper::visit(InitDeclarator* node)       { DUMP_BEGIN("InitDeclarator"); DUMP_END(); }
void AstDumper::visit(Initializer* node)          { DUMP_BEGIN("Initializer"); DUMP_END(); }
void AstDumper::visit(ParameterDeclaration* node) { DUMP_BEGIN("ParameterDeclaration"); DUMP_END(); }
void AstDumper::visit(ParameterTypeList* node)    { DUMP_BEGIN("ParameterTypeList"); DUMP_END(); }
void AstDumper::visit(EnumSpecifier* node)        { DUMP_BEGIN("EnumSpecifier " << (node->id ? node->id : "?")); DUMP_END(); }
void AstDumper::visit(Enumerator* node)           { DUMP_BEGIN("Enumerator " << (node->id ? node->id : "?")); DUMP_END(); }
void AstDumper::visit(StructSpecifier* node)      { DUMP_BEGIN("StructSpecifier " << (node->id ? node->id : "?")); DUMP_END(); }
void AstDumper::visit(UnionSpecifier* node)       { DUMP_BEGIN("UnionSpecifier " << (node->id ? node->id : "?")); DUMP_END(); }
void AstDumper::visit(StructDeclaration* node)    { DUMP_BEGIN("StructDeclaration"); DUMP_END(); }
void AstDumper::visit(StructDeclarator* node)     { DUMP_BEGIN("StructDeclarator"); DUMP_END(); }
void AstDumper::visit(TypedefName* node)          { DUMP_BEGIN("TypedefName " << (node->id ? node->id : "?")); DUMP_END(); }
void AstDumper::visit(TokenNode* node)            { DUMP_BEGIN("TokenNode"); DUMP_END(); }

void AstDumper::visit(FunctionDeclarator* node)   { DUMP_BEGIN("FunctionDeclarator"); DUMP_END(); }
void AstDumper::visit(ArrayDeclarator* node)      { DUMP_BEGIN("ArrayDeclarator"); DUMP_END(); }
void AstDumper::visit(PointerDeclarator* node)    { DUMP_BEGIN("PointerDeclarator"); DUMP_END(); }
void AstDumper::visit(NameDeclarator* node)       { DUMP_BEGIN("NameDeclarator " << (node->id ? node->id : "?")); DUMP_END(); }

} // namespace TCC_Pro

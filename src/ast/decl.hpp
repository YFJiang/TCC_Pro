#pragma once

#include "ast/node.hpp"
#include "ast/stmt.hpp"
#include "lex/token.hpp"

namespace TCC_Pro {

class Type;
class Symbol;

enum { DEC_ABSTRACT = 0x01, DEC_CONCRETE = 0x02 };
enum { POINTER_TO, ARRAY_OF, FUNCTION_RETURN };

class TypeDeriv {
public:
    int ctor;
    union {
        int len;
        int qual;
        void* sig;  // Signature*
    };
    TypeDeriv* next = nullptr;
};

class Declarator : public Node {
public:
    Declarator* dec = nullptr;
    const char* id = nullptr;
    TypeDeriv* tyDrvList = nullptr;

    explicit Declarator(NodeKind kind) : Node(kind) {}
};

class NameDeclarator : public Declarator {
public:
    NameDeclarator() : Declarator(NodeKind::NK_NameDeclarator) {}
    void accept(Visitor& v) override;
};

class FunctionDeclarator : public Declarator {
public:
    void* ids = nullptr;  // Vector*
    class ParameterTypeList* paramTyList = nullptr;
    int partOfDef = 0;
    void* sig = nullptr;  // Signature*

    FunctionDeclarator() : Declarator(NodeKind::NK_FunctionDeclarator) {}
    void accept(Visitor& v) override;
};

class ArrayDeclarator : public Declarator {
public:
    class Expr* expr = nullptr;

    ArrayDeclarator() : Declarator(NodeKind::NK_ArrayDeclarator) {}
    void accept(Visitor& v) override;
};

class PointerDeclarator : public Declarator {
public:
    Node* tyQuals = nullptr;

    PointerDeclarator() : Declarator(NodeKind::NK_PointerDeclarator) {}
    void accept(Visitor& v) override;
};

class Specifiers : public Node {
public:
    Node* stgClasses = nullptr;
    Node* tyQuals = nullptr;
    Node* tySpecs = nullptr;
    int sclass = 0;
    Type* ty = nullptr;

    Specifiers() : Node(NodeKind::NK_Specifiers) {}
    void accept(Visitor& v) override;
};

class TypeName : public Node {
public:
    Specifiers* specs = nullptr;
    Declarator* dec = nullptr;

    TypeName() : Node(NodeKind::NK_TypeName) {}
    void accept(Visitor& v) override;
};

class InitDeclarator : public Node {
public:
    Declarator* dec = nullptr;
    class Initializer* init = nullptr;

    InitDeclarator() : Node(NodeKind::NK_InitDeclarator) {}
    void accept(Visitor& v) override;
};

class Initializer : public Node {
public:
    int lbrace = 0;
    union {
        Node* initials;
        Expr* expr;
    };

    Initializer() : Node(NodeKind::NK_Initializer) { initials = nullptr; }
    void accept(Visitor& v) override;
};

class ParameterDeclaration : public Node {
public:
    Specifiers* specs = nullptr;
    Declarator* dec = nullptr;

    ParameterDeclaration() : Node(NodeKind::NK_ParameterDeclaration) {}
    void accept(Visitor& v) override;
};

class ParameterTypeList : public Node {
public:
    Node* paramDecls = nullptr;
    int ellipse = 0;

    ParameterTypeList() : Node(NodeKind::NK_ParameterTypeList) {}
    void accept(Visitor& v) override;
};

class EnumSpecifier : public Node {
public:
    const char* id = nullptr;
    Node* enumers = nullptr;

    EnumSpecifier() : Node(NodeKind::NK_EnumSpecifier) {}
    void accept(Visitor& v) override;
};

class Enumerator : public Node {
public:
    const char* id = nullptr;
    Expr* expr = nullptr;

    Enumerator() : Node(NodeKind::NK_Enumerator) {}
    void accept(Visitor& v) override;
};

class StructSpecifier : public Node {
public:
    const char* id = nullptr;
    Node* stDecls = nullptr;

    StructSpecifier() : Node(NodeKind::NK_StructSpecifier) {}
    void accept(Visitor& v) override;
};

class UnionSpecifier : public Node {
public:
    const char* id = nullptr;
    Node* stDecls = nullptr;

    UnionSpecifier() : Node(NodeKind::NK_UnionSpecifier) {}
    void accept(Visitor& v) override;
};

class StructDeclaration : public Node {
public:
    Specifiers* specs = nullptr;
    Node* stDecs = nullptr;

    StructDeclaration() : Node(NodeKind::NK_StructDeclaration) {}
    void accept(Visitor& v) override;
};

class StructDeclarator : public Node {
public:
    Declarator* dec = nullptr;
    Expr* expr = nullptr;

    StructDeclarator() : Node(NodeKind::NK_StructDeclarator) {}
    void accept(Visitor& v) override;
};

class TypedefName : public Node {
public:
    const char* id = nullptr;
    Symbol* sym = nullptr;

    TypedefName() : Node(NodeKind::NK_TypedefName) {}
    void accept(Visitor& v) override;
};

class TokenNode : public Node {
public:
    int token;

    TokenNode() : Node(NodeKind::NK_Token) {}
    void accept(Visitor& v) override;
};

class Declaration : public Node {
public:
    Specifiers* specs = nullptr;
    Node* initDecs = nullptr;

    Declaration() : Node(NodeKind::NK_Declaration) {}
    void accept(Visitor& v) override;
};

class Function : public Node {
public:
    Specifiers* specs = nullptr;
    Declarator* dec = nullptr;
    FunctionDeclarator* fdec = nullptr;
    Node* decls = nullptr;
    Stmt* stmt = nullptr;
    void* fsym = nullptr;  // FunctionSymbol*
    void* labels = nullptr;
    void* loops = nullptr;    // Vector*
    void* swtches = nullptr;  // Vector*
    void* breakable = nullptr;  // Vector*
    int hasReturn = 0;

    Function() : Node(NodeKind::NK_Function) {}
    void accept(Visitor& v) override;
};

class TranslationUnit : public Node {
public:
    Node* extDecls = nullptr;

    TranslationUnit() : Node(NodeKind::NK_TranslationUnit) {}
    void accept(Visitor& v) override;
};

} // namespace TCC_Pro

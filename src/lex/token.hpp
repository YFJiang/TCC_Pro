#pragma once

#include "common/source_location.hpp"
#include <cstdint>
#include <string_view>

namespace TCC_Pro {

enum class TokenKind : int {
    TK_BEGIN = -1,

    TK_AUTO, TK_EXTERN, TK_REGISTER, TK_STATIC, TK_TYPEDEF,
    TK_CONST, TK_VOLATILE, TK_SIGNED, TK_UNSIGNED,
    TK_SHORT, TK_LONG, TK_CHAR, TK_INT, TK_INT64,
    TK_FLOAT, TK_DOUBLE, TK_ENUM, TK_STRUCT, TK_UNION, TK_VOID,
    TK_BREAK, TK_CASE, TK_CONTINUE, TK_DEFAULT, TK_DO, TK_ELSE,
    TK_FOR, TK_GOTO, TK_IF, TK_RETURN, TK_SWITCH, TK_WHILE, TK_SIZEOF,

    TK_ID,

    TK_INTCONST, TK_UINTCONST, TK_LONGCONST, TK_ULONGCONST,
    TK_LLONGCONST, TK_ULLONGCONST,
    TK_FLOATCONST, TK_DOUBLECONST, TK_LDOUBLECONST,
    TK_STRING, TK_WIDESTRING,

    TK_COMMA, TK_QUESTION, TK_COLON, TK_ASSIGN,
    TK_BITOR_ASSIGN, TK_BITXOR_ASSIGN, TK_BITAND_ASSIGN,
    TK_LSHIFT_ASSIGN, TK_RSHIFT_ASSIGN,
    TK_ADD_ASSIGN, TK_SUB_ASSIGN, TK_MUL_ASSIGN, TK_DIV_ASSIGN, TK_MOD_ASSIGN,
    TK_OR, TK_AND, TK_BITOR, TK_BITXOR, TK_BITAND,
    TK_EQUAL, TK_UNEQUAL, TK_GREAT, TK_LESS, TK_GREAT_EQ, TK_LESS_EQ,
    TK_LSHIFT, TK_RSHIFT,
    TK_ADD, TK_SUB, TK_MUL, TK_DIV, TK_MOD,
    TK_INC, TK_DEC, TK_NOT, TK_COMP,
    TK_DOT, TK_POINTER,
    TK_LPAREN, TK_RPAREN, TK_LBRACKET, TK_RBRACKET,

    TK_LBRACE, TK_RBRACE, TK_SEMICOLON, TK_ELLIPSE,
    TK_POUND, TK_NEWLINE,

    TK_END,
};

union Value {
    int i[2];
    float f;
    double d;
    void* p;
    Value() : i{0, 0} {}
};

struct Token {
    TokenKind kind = TokenKind::TK_END;
    Value value;
    SourceLocation loc;
    const char* id = nullptr;

    std::string_view kindName() const;
};

const char* tokenName(TokenKind kind);

} // namespace TCC_Pro

#include "lex/token.hpp"

namespace TCC_Pro {

#define TOKEN_ENTRY(k, s) case TokenKind::k: return s

const char* tokenName(TokenKind kind) {
    switch (kind) {
    TOKEN_ENTRY(TK_AUTO, "auto");
    TOKEN_ENTRY(TK_EXTERN, "extern");
    TOKEN_ENTRY(TK_REGISTER, "register");
    TOKEN_ENTRY(TK_STATIC, "static");
    TOKEN_ENTRY(TK_TYPEDEF, "typedef");
    TOKEN_ENTRY(TK_CONST, "const");
    TOKEN_ENTRY(TK_VOLATILE, "volatile");
    TOKEN_ENTRY(TK_SIGNED, "signed");
    TOKEN_ENTRY(TK_UNSIGNED, "unsigned");
    TOKEN_ENTRY(TK_SHORT, "short");
    TOKEN_ENTRY(TK_LONG, "long");
    TOKEN_ENTRY(TK_CHAR, "char");
    TOKEN_ENTRY(TK_INT, "int");
    TOKEN_ENTRY(TK_INT64, "__int64");
    TOKEN_ENTRY(TK_FLOAT, "float");
    TOKEN_ENTRY(TK_DOUBLE, "double");
    TOKEN_ENTRY(TK_ENUM, "enum");
    TOKEN_ENTRY(TK_STRUCT, "struct");
    TOKEN_ENTRY(TK_UNION, "union");
    TOKEN_ENTRY(TK_VOID, "void");
    TOKEN_ENTRY(TK_BREAK, "break");
    TOKEN_ENTRY(TK_CASE, "case");
    TOKEN_ENTRY(TK_CONTINUE, "continue");
    TOKEN_ENTRY(TK_DEFAULT, "default");
    TOKEN_ENTRY(TK_DO, "do");
    TOKEN_ENTRY(TK_ELSE, "else");
    TOKEN_ENTRY(TK_FOR, "for");
    TOKEN_ENTRY(TK_GOTO, "goto");
    TOKEN_ENTRY(TK_IF, "if");
    TOKEN_ENTRY(TK_RETURN, "return");
    TOKEN_ENTRY(TK_SWITCH, "switch");
    TOKEN_ENTRY(TK_WHILE, "while");
    TOKEN_ENTRY(TK_SIZEOF, "sizeof");
    TOKEN_ENTRY(TK_ID, "ID");
    TOKEN_ENTRY(TK_INTCONST, "int");
    TOKEN_ENTRY(TK_UINTCONST, "unsigned int");
    TOKEN_ENTRY(TK_LONGCONST, "long");
    TOKEN_ENTRY(TK_ULONGCONST, "unsigned long");
    TOKEN_ENTRY(TK_LLONGCONST, "long long");
    TOKEN_ENTRY(TK_ULLONGCONST, "unsigned long long");
    TOKEN_ENTRY(TK_FLOATCONST, "float");
    TOKEN_ENTRY(TK_DOUBLECONST, "double");
    TOKEN_ENTRY(TK_LDOUBLECONST, "long double");
    TOKEN_ENTRY(TK_STRING, "STR");
    TOKEN_ENTRY(TK_WIDESTRING, "WSTR");
    TOKEN_ENTRY(TK_COMMA, ",");
    TOKEN_ENTRY(TK_QUESTION, "?");
    TOKEN_ENTRY(TK_COLON, ":");
    TOKEN_ENTRY(TK_ASSIGN, "=");
    TOKEN_ENTRY(TK_BITOR_ASSIGN, "|=");
    TOKEN_ENTRY(TK_BITXOR_ASSIGN, "^=");
    TOKEN_ENTRY(TK_BITAND_ASSIGN, "&=");
    TOKEN_ENTRY(TK_LSHIFT_ASSIGN, "<<=");
    TOKEN_ENTRY(TK_RSHIFT_ASSIGN, ">>=");
    TOKEN_ENTRY(TK_ADD_ASSIGN, "+=");
    TOKEN_ENTRY(TK_SUB_ASSIGN, "-=");
    TOKEN_ENTRY(TK_MUL_ASSIGN, "*=");
    TOKEN_ENTRY(TK_DIV_ASSIGN, "/=");
    TOKEN_ENTRY(TK_MOD_ASSIGN, "%=");
    TOKEN_ENTRY(TK_OR, "||");
    TOKEN_ENTRY(TK_AND, "&&");
    TOKEN_ENTRY(TK_BITOR, "|");
    TOKEN_ENTRY(TK_BITXOR, "^");
    TOKEN_ENTRY(TK_BITAND, "&");
    TOKEN_ENTRY(TK_EQUAL, "==");
    TOKEN_ENTRY(TK_UNEQUAL, "!=");
    TOKEN_ENTRY(TK_GREAT, ">");
    TOKEN_ENTRY(TK_LESS, "<");
    TOKEN_ENTRY(TK_GREAT_EQ, ">=");
    TOKEN_ENTRY(TK_LESS_EQ, "<=");
    TOKEN_ENTRY(TK_LSHIFT, "<<");
    TOKEN_ENTRY(TK_RSHIFT, ">>");
    TOKEN_ENTRY(TK_ADD, "+");
    TOKEN_ENTRY(TK_SUB, "-");
    TOKEN_ENTRY(TK_MUL, "*");
    TOKEN_ENTRY(TK_DIV, "/");
    TOKEN_ENTRY(TK_MOD, "%");
    TOKEN_ENTRY(TK_INC, "++");
    TOKEN_ENTRY(TK_DEC, "--");
    TOKEN_ENTRY(TK_NOT, "!");
    TOKEN_ENTRY(TK_COMP, "~");
    TOKEN_ENTRY(TK_DOT, ".");
    TOKEN_ENTRY(TK_POINTER, "->");
    TOKEN_ENTRY(TK_LPAREN, "(");
    TOKEN_ENTRY(TK_RPAREN, ")");
    TOKEN_ENTRY(TK_LBRACKET, "[");
    TOKEN_ENTRY(TK_RBRACKET, "]");
    TOKEN_ENTRY(TK_LBRACE, "{");
    TOKEN_ENTRY(TK_RBRACE, "}");
    TOKEN_ENTRY(TK_SEMICOLON, ";");
    TOKEN_ENTRY(TK_ELLIPSE, "...");
    TOKEN_ENTRY(TK_POUND, "#");
    TOKEN_ENTRY(TK_NEWLINE, "\\n");
    TOKEN_ENTRY(TK_END, "EOF");
    default: return "???";
    }
}

std::string_view Token::kindName() const {
    return tokenName(kind);
}

} // namespace TCC_Pro

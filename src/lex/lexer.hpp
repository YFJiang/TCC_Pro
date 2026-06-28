#pragma once

#include "lex/token.hpp"
#include "common/compiler_context.hpp"
#include <array>
#include <string>

namespace TCC_Pro {

class Lexer {
public:
    static constexpr unsigned char END_OF_FILE = 255;

    explicit Lexer(CompilerContext& ctx);
    ~Lexer();

    Lexer(const Lexer&) = delete;
    Lexer& operator=(const Lexer&) = delete;

    bool open(const std::string& filename);
    void close();

    Token nextToken();

    Token currentToken() const { return currentToken_; }

    void beginPeek();
    void endPeek();

    const std::string& filename() const { return filename_; }

private:
    using ScannerFunc = TokenKind (Lexer::*)();

    void setupScanners();
    void skipWhiteSpace();
    void scanPPLine();

    int scanEscapeChar(int wide);
    int findKeyword(const char* str, int len);
    TokenKind scanIntLiteral(unsigned char* start, int len, int base);
    TokenKind scanFloatLiteral(unsigned char* start);
    TokenKind scanNumericLiteral();
    TokenKind scanCharLiteral();
    TokenKind scanStringLiteral();
    TokenKind scanIdentifier();

    TokenKind scanPlus();
    TokenKind scanMinus();
    TokenKind scanStar();
    TokenKind scanSlash();
    TokenKind scanPercent();
    TokenKind scanLess();
    TokenKind scanGreat();
    TokenKind scanExclamation();
    TokenKind scanEqual();
    TokenKind scanBar();
    TokenKind scanAmpersand();
    TokenKind scanCaret();
    TokenKind scanDot();

    TokenKind scanSingle();
    TokenKind scanBadChar();
    TokenKind scanEOF();

    static bool isLetter(int c);
    static bool isDigit(int c);
    static bool isHexDigit(int c);
    static bool isOctDigit(int c);
    static bool isLetterOrDigit(int c);
    static int toUpper(int c);

    CompilerContext& ctx_;
    std::string filename_;

    unsigned char* base_ = nullptr;
    unsigned char* cursor_ = nullptr;
    unsigned char* lineHead_ = nullptr;
    int line_ = 0;
    size_t size_ = 0;
    void* file_ = nullptr;
    void* fileMapping_ = nullptr;

    Token currentToken_;
    SourceLocation prevCoord_;

    unsigned char* peekCursor_ = nullptr;
    Token peekToken_;
    SourceLocation peekCoord_;

    std::array<ScannerFunc, 256> scanners_{};
};

} // namespace TCC_Pro

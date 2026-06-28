#include "lex/lexer.hpp"
#include "lex/token.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string_view>

namespace TCC_Pro {

// Keyword table
struct Keyword {
    const char* name;
    int token;  // raw index
};

// Sorted alphabetically by first letter for fast lookup
static const Keyword keywordsA[] = {{"auto", 0}, {nullptr, 0}};
static const Keyword keywordsB[] = {{"break", 1}, {nullptr, 0}};
static const Keyword keywordsC[] = {{"case", 2}, {"char", 3}, {"const", 4}, {"continue", 5}, {nullptr, 0}};
static const Keyword keywordsD[] = {{"default", 6}, {"do", 7}, {"double", 8}, {nullptr, 0}};
static const Keyword keywordsE[] = {{"else", 9}, {"enum", 10}, {"extern", 11}, {nullptr, 0}};
static const Keyword keywordsF[] = {{"float", 12}, {"for", 13}, {nullptr, 0}};
static const Keyword keywordsG[] = {{"goto", 14}, {nullptr, 0}};
static const Keyword keywordsI[] = {{"if", 15}, {"int", 16}, {nullptr, 0}};
static const Keyword keywordsL[] = {{"long", 17}, {nullptr, 0}};
static const Keyword keywordsR[] = {{"register", 18}, {"return", 19}, {nullptr, 0}};
static const Keyword keywordsS[] = {{"short", 20}, {"signed", 21}, {"sizeof", 22},
                                     {"static", 23}, {"struct", 24}, {"switch", 25}, {nullptr, 0}};
static const Keyword keywordsT[] = {{"typedef", 26}, {nullptr, 0}};
static const Keyword keywordsU[] = {{"union", 27}, {"unsigned", 28}, {nullptr, 0}};
static const Keyword keywordsV[] = {{"void", 29}, {"volatile", 30}, {nullptr, 0}};
static const Keyword keywordsW[] = {{"while", 31}, {nullptr, 0}};
static const Keyword keywordsUnderscore[] = {{"__int64", 32}, {nullptr, 0}};

static const Keyword* keywordTable[] = {
    // index by first char: '_'=0, 'A'=1, 'B'=2, ...
    keywordsUnderscore, keywordsA, keywordsB, keywordsC,
    keywordsD, keywordsE, keywordsF, keywordsG,
    nullptr, keywordsI, nullptr, nullptr,
    keywordsL, nullptr, nullptr, nullptr,
    nullptr, nullptr, keywordsR, keywordsS,
    keywordsT, keywordsU, keywordsV, keywordsW,
    nullptr, nullptr, nullptr
};

// Maps keyword index to TokenKind
static constexpr TokenKind keywordTokens[] = {
    TokenKind::TK_AUTO, TokenKind::TK_BREAK, TokenKind::TK_CASE,
    TokenKind::TK_CHAR, TokenKind::TK_CONST, TokenKind::TK_CONTINUE,
    TokenKind::TK_DEFAULT, TokenKind::TK_DO, TokenKind::TK_DOUBLE,
    TokenKind::TK_ELSE, TokenKind::TK_ENUM, TokenKind::TK_EXTERN,
    TokenKind::TK_FLOAT, TokenKind::TK_FOR, TokenKind::TK_GOTO,
    TokenKind::TK_IF, TokenKind::TK_INT, TokenKind::TK_LONG,
    TokenKind::TK_REGISTER, TokenKind::TK_RETURN, TokenKind::TK_SHORT,
    TokenKind::TK_SIGNED, TokenKind::TK_SIZEOF, TokenKind::TK_STATIC,
    TokenKind::TK_STRUCT, TokenKind::TK_SWITCH, TokenKind::TK_TYPEDEF,
    TokenKind::TK_UNION, TokenKind::TK_UNSIGNED, TokenKind::TK_VOID,
    TokenKind::TK_VOLATILE, TokenKind::TK_WHILE, TokenKind::TK_INT64,
};

bool Lexer::isLetter(int c) { return (c >= 'a' && c <= 'z') || c == '_' || (c >= 'A' && c <= 'Z'); }
bool Lexer::isDigit(int c) { return c >= '0' && c <= '9'; }
bool Lexer::isHexDigit(int c) { return isDigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'); }
bool Lexer::isOctDigit(int c) { return c >= '0' && c <= '7'; }
bool Lexer::isLetterOrDigit(int c) { return isLetter(c) || isDigit(c); }
int Lexer::toUpper(int c) { return c & ~0x20; }

Lexer::Lexer(CompilerContext& ctx) : ctx_(ctx) {
    setupScanners();
}

Lexer::~Lexer() {
    close();
}

bool Lexer::open(const std::string& filename) {
    filename_ = filename;

    int fno = ::open(filename.c_str(), O_RDWR);
    if (fno == -1) {
        ctx_.diagnostics().fatal("Can't open file " + filename);
    }

    struct stat st;
    if (::fstat(fno, &st) == -1) {
        ctx_.diagnostics().fatal("Can't stat file " + filename);
    }
    size_ = st.st_size;

    base_ = static_cast<unsigned char*>(
        ::mmap(nullptr, size_ + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fno, 0));
    if (base_ == MAP_FAILED) {
        ctx_.diagnostics().fatal("Can't mmap file " + filename);
    }

    file_ = reinterpret_cast<void*>(static_cast<intptr_t>(fno));
    base_[size_] = END_OF_FILE;
    cursor_ = lineHead_ = base_;
    line_ = 1;

    currentToken_.loc.filename = ctx_.interner().intern(filename.c_str());
    currentToken_.loc.line = 1;
    currentToken_.loc.ppline = 0;
    currentToken_.loc.col = 1;

    return true;
}

void Lexer::close() {
    if (base_ && base_ != MAP_FAILED) {
        ::munmap(base_, size_ + 1);
        base_ = nullptr;
    }
    if (file_) {
        ::close(static_cast<int>(reinterpret_cast<intptr_t>(file_)));
        file_ = nullptr;
    }
    cursor_ = nullptr;
    lineHead_ = nullptr;
}

void Lexer::scanPPLine() {
    cursor_++;
    while (*cursor_ == ' ' || *cursor_ == '\t') cursor_++;

    if (isDigit(*cursor_)) {
        goto read_line;
    } else if (std::strncmp(reinterpret_cast<char*>(cursor_), "line", 4) == 0) {
        cursor_ += 4;
        while (*cursor_ == ' ' || *cursor_ == '\t') cursor_++;
read_line:
        int line = 0;
        while (isDigit(*cursor_)) {
            line = 10 * line + *cursor_ - '0';
            cursor_++;
        }
        currentToken_.loc.ppline = line - 1;

        while (*cursor_ == ' ' || *cursor_ == '\t') cursor_++;
        const char* fstart = reinterpret_cast<char*>(++cursor_);
        while (*cursor_ != '"' && *cursor_ != END_OF_FILE && *cursor_ != '\n') cursor_++;
        currentToken_.loc.filename = ctx_.interner().intern(
            fstart, reinterpret_cast<char*>(cursor_) - fstart);
    }

    while (*cursor_ != '\n' && *cursor_ != END_OF_FILE) cursor_++;
}

void Lexer::skipWhiteSpace() {
again:
    int ch = *cursor_;
    while (ch == '\t' || ch == '\v' || ch == '\f' || ch == ' ' ||
           ch == '\r' || ch == '\n' || ch == '/'  || ch == '#') {
        switch (ch) {
        case '\n':
            currentToken_.loc.ppline++;
            line_++;
            lineHead_ = ++cursor_;
            break;
        case '#':
            scanPPLine();
            break;
        case '/':
            if (cursor_[1] != '/' && cursor_[1] != '*') return;
            cursor_++;
            if (*cursor_ == '/') {
                cursor_++;
                while (*cursor_ != '\n' && *cursor_ != END_OF_FILE) cursor_++;
            } else {
                cursor_ += 2;
                while (cursor_[0] != '*' || cursor_[1] != '/') {
                    if (*cursor_ == '\n') {
                        currentToken_.loc.ppline++;
                        line_++;
                    } else if (cursor_[0] == END_OF_FILE || cursor_[1] == END_OF_FILE) {
                        ctx_.diagnostics().error(currentToken_.loc, "Comment is not closed");
                        return;
                    }
                    cursor_++;
                }
                cursor_ += 2;
            }
            break;
        default:
            cursor_++;
            break;
        }
        ch = *cursor_;
    }
    // ExtraWhiteSpace not supported in this basic lexer
}

int Lexer::scanEscapeChar(int wide) {
    cursor_++;
    switch (*cursor_++) {
    case 'a': return '\a';
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case 'v': return '\v';
    case '\'': case '"': case '\\': case '\?': return *(cursor_ - 1);
    case 'x': {
        if (!isHexDigit(*cursor_)) {
            ctx_.diagnostics().error(currentToken_.loc, "Expect hex digit");
            return 'x';
        }
        int v = 0;
        bool overflow = false;
        while (isHexDigit(*cursor_)) {
            if (v >> (/*wchar size*/ 4 - 4)) overflow = true;
            v = (v << 4) + (isDigit(*cursor_) ? *cursor_ - '0' : toUpper(*cursor_) - 'A' + 10);
            cursor_++;
        }
        if (overflow || (!wide && v > 255))
            ctx_.diagnostics().warning(currentToken_.loc, "Hexadecimal escape sequence overflow");
        return v;
    }
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7': {
        int v = *(cursor_ - 1) - '0';
        if (isOctDigit(*cursor_)) { v = (v << 3) + *cursor_++ - '0'; }
        if (isOctDigit(*cursor_)) { v = (v << 3) + *cursor_++ - '0'; }
        return v;
    }
    default:
        ctx_.diagnostics().warning(currentToken_.loc,
            "Unrecognized escape sequence: \\%c" + std::to_string(*cursor_));
        return *cursor_;
    }
}

int Lexer::findKeyword(const char* str, int len) {
    if (*str != '_') {
        int index = toUpper(*str) - 'A' + 1;
        auto* p = keywordTable[index];
        if (p) {
            for (int i = 0; p[i].name; ++i) {
                if (static_cast<int>(std::strlen(p[i].name)) == len &&
                    std::strncmp(str, p[i].name, len) == 0) {
                    return static_cast<int>(keywordTokens[p[i].token]);
                }
            }
        }
    } else {
        auto* p = keywordTable[0];
        for (int i = 0; p[i].name; ++i) {
            if (static_cast<int>(std::strlen(p[i].name)) == len &&
                std::strncmp(str, p[i].name, len) == 0) {
                return static_cast<int>(keywordTokens[p[i].token]);
            }
        }
    }
    return static_cast<int>(TokenKind::TK_ID);
}

TokenKind Lexer::scanIntLiteral(unsigned char* start, int len, int base) {
    unsigned int i[2] = {0, 0};
    int tok = static_cast<int>(TokenKind::TK_INTCONST);
    bool overflow = false;

    for (int idx = 0; idx < len; ++idx) {
        int d;
        if (base == 16) {
            if (start[idx] >= 'A' && start[idx] <= 'F') d = start[idx] - 'A' + 10;
            else if (start[idx] >= 'a' && start[idx] <= 'f') d = start[idx] - 'a' + 10;
            else d = start[idx] - '0';
        } else {
            d = start[idx] - '0';
        }

        unsigned carry0 = 0, carry1 = 0;
        switch (base) {
        case 16:
            carry0 = i[0] >> (32 - 4);
            carry1 = i[1] >> (32 - 4);
            i[0] <<= 4; i[1] <<= 4;
            break;
        case 8:
            carry0 = i[0] >> (32 - 3);
            carry1 = i[1] >> (32 - 3);
            i[0] <<= 3; i[1] <<= 3;
            break;
        case 10: {
            carry0 = (i[0] >> (32 - 3)) + (i[0] >> (32 - 1));
            carry1 = (i[1] >> (32 - 3)) + (i[1] >> (32 - 1));
            unsigned t1 = i[0] << 3, t2 = i[0] << 1;
            if (t1 > 0xFFFFFFFFu - t2) carry0++;
            i[0] = t1 + t2;
            t1 = i[1] << 3; t2 = i[1] << 1;
            if (t1 > 0xFFFFFFFFu - t2) carry1++;
            i[1] = t1 + t2;
            break;
        }
        }
        if (i[0] > 0xFFFFFFFFu - static_cast<unsigned>(d)) carry0++;
        if (carry1 || (i[1] > 0xFFFFFFFFu - carry0)) overflow = true;
        i[0] += d;
        i[1] += carry0;
    }

    if (overflow || i[1] != 0)
        ctx_.diagnostics().warning(currentToken_.loc, "Integer literal is too big");

    currentToken_.value.i[1] = 0;
    currentToken_.value.i[0] = static_cast<int>(i[0]);

    if (*cursor_ == 'U' || *cursor_ == 'u') {
        cursor_++;
        tok = (tok == static_cast<int>(TokenKind::TK_INTCONST)) ?
              static_cast<int>(TokenKind::TK_UINTCONST) :
              static_cast<int>(TokenKind::TK_ULLONGCONST);
    }
    if (*cursor_ == 'L' || *cursor_ == 'l') {
        cursor_++;
        if (tok == static_cast<int>(TokenKind::TK_INTCONST)) tok = static_cast<int>(TokenKind::TK_LONGCONST);
        else if (tok == static_cast<int>(TokenKind::TK_UINTCONST)) tok = static_cast<int>(TokenKind::TK_ULONGCONST);
        if (*cursor_ == 'L' || *cursor_ == 'l') {
            cursor_++;
            if (tok < static_cast<int>(TokenKind::TK_LLONGCONST))
                tok = static_cast<int>(TokenKind::TK_LLONGCONST);
        }
    }
    return static_cast<TokenKind>(tok);
}

TokenKind Lexer::scanFloatLiteral(unsigned char* start) {
    if (*cursor_ == '.') {
        cursor_++;
        while (isDigit(*cursor_)) cursor_++;
    }
    if (*cursor_ == 'e' || *cursor_ == 'E') {
        cursor_++;
        if (*cursor_ == '+' || *cursor_ == '-') cursor_++;
        if (!isDigit(*cursor_))
            ctx_.diagnostics().error(currentToken_.loc, "Expect exponent value");
        else
            while (isDigit(*cursor_)) cursor_++;
    }

    errno = 0;
    double d = std::strtod(reinterpret_cast<char*>(start), nullptr);
    if (errno == ERANGE) {
        ctx_.diagnostics().warning(currentToken_.loc, "Float literal overflow");
    }
    currentToken_.value.d = d;

    if (*cursor_ == 'f' || *cursor_ == 'F') {
        cursor_++;
        currentToken_.value.f = static_cast<float>(d);
        return TokenKind::TK_FLOATCONST;
    } else if (*cursor_ == 'L' || *cursor_ == 'l') {
        cursor_++;
        return TokenKind::TK_LDOUBLECONST;
    }
    return TokenKind::TK_DOUBLECONST;
}

TokenKind Lexer::scanNumericLiteral() {
    unsigned char* start = cursor_;
    int base = 10;

    if (*cursor_ == '.') return scanFloatLiteral(start);

    if (*cursor_ == '0' && (cursor_[1] == 'x' || cursor_[1] == 'X')) {
        cursor_ += 2;
        start = cursor_;
        base = 16;
        if (!isHexDigit(*cursor_)) {
            ctx_.diagnostics().error(currentToken_.loc, "Expect hex digit");
            currentToken_.value.i[0] = 0;
            return TokenKind::TK_INTCONST;
        }
        while (isHexDigit(*cursor_)) cursor_++;
    } else if (*cursor_ == '0') {
        cursor_++;
        base = 8;
        while (isOctDigit(*cursor_)) cursor_++;
    } else {
        cursor_++;
        while (isDigit(*cursor_)) cursor_++;
    }

    if (base == 16 || (*cursor_ != '.' && *cursor_ != 'e' && *cursor_ != 'E'))
        return scanIntLiteral(start, static_cast<int>(cursor_ - start), base);
    else
        return scanFloatLiteral(start);
}

TokenKind Lexer::scanCharLiteral() {
    int count = 0;
    int wide = 0;
    int ch = 0;

    if (*cursor_ == 'L') { cursor_++; wide = 1; }
    cursor_++;
    while (*cursor_ != '\'') {
        if (*cursor_ == '\n' || *cursor_ == END_OF_FILE) break;
        ch = (*cursor_ == '\\') ? scanEscapeChar(wide) : *cursor_++;
        count++;
    }
    if (*cursor_ != '\'') {
        ctx_.diagnostics().error(currentToken_.loc, "Expect '");
        goto end_char;
    }
    cursor_++;
    if (count > 1) ctx_.diagnostics().warning(currentToken_.loc, "Too many characters");

end_char:
    currentToken_.value.i[0] = ch;
    currentToken_.value.i[1] = 0;
    return TokenKind::TK_INTCONST;
}

TokenKind Lexer::scanStringLiteral() {
    auto& arena = ctx_.programArena();
    char tmp[512];
    int wide = 0;
    int len = 0;
    int maxlen = 512;
    int ch;

    // Allocate string struct on arena
    struct StrBuf { char* chs; int length; };
    auto* str = static_cast<StrBuf*>(arena.allocate(sizeof(StrBuf)));
    str->chs = nullptr;
    str->length = 0;

    if (*cursor_ == 'L') { cursor_++; wide = 1; maxlen /= 4; }
    cursor_++;

next_string:
    while (*cursor_ != '"') {
        if (*cursor_ == '\n' || *cursor_ == END_OF_FILE) break;
        ch = (*cursor_ == '\\') ? scanEscapeChar(wide) : *cursor_++;
        if (wide)
            reinterpret_cast<int*>(tmp)[len] = ch;
        else
            tmp[len] = static_cast<char>(ch);
        len++;
        if (len >= maxlen) {
            // Append to string
            int size = str->length + len + 1;
            int times = wide ? 4 : 1;
            auto* p = static_cast<char*>(arena.allocate(size * times));
            if (str->chs) {
                std::memcpy(p, str->chs, str->length * times);
            }
            std::memcpy(p + str->length * times, tmp, len * times);
            str->chs = p;
            str->length = size - 1;
            if (!wide) p[size - 1] = 0;
            else reinterpret_cast<int*>(p)[size - 1] = 0;
            len = 0;
        }
    }
    if (*cursor_ != '"') {
        ctx_.diagnostics().error(currentToken_.loc, "Expect \"");
        goto end_string;
    }
    cursor_++;
    skipWhiteSpace();
    if (cursor_[0] == '"') {
        if (wide) ctx_.diagnostics().error(currentToken_.loc, "String wideness mismatch");
        cursor_++;
        goto next_string;
    } else if (cursor_[0] == 'L' && cursor_[1] == '"') {
        if (!wide) ctx_.diagnostics().error(currentToken_.loc, "String wideness mismatch");
        cursor_ += 2;
        goto next_string;
    }

end_string:
    {
        int size = str->length + len + 1;
        int times = wide ? 4 : 1;
        auto* p = static_cast<char*>(arena.allocate(size * times));
        if (str->chs) std::memcpy(p, str->chs, str->length * times);
        std::memcpy(p + str->length * times, tmp, len * times);
        str->chs = p;
        str->length = size - 1;
        if (!wide) p[size - 1] = 0;
        else reinterpret_cast<int*>(&p[(size - 1) * 4])[0] = 0;
    }
    currentToken_.value.p = str;
    return wide ? TokenKind::TK_WIDESTRING : TokenKind::TK_STRING;
}

TokenKind Lexer::scanIdentifier() {
    unsigned char* start = cursor_;

    if (*cursor_ == 'L' && (cursor_[1] == '\'' || cursor_[1] == '"')) {
        return (cursor_[1] == '\'') ? scanCharLiteral() : scanStringLiteral();
    }

    cursor_++;
    while (isLetterOrDigit(*cursor_)) cursor_++;

    int tok = findKeyword(reinterpret_cast<char*>(start), static_cast<int>(cursor_ - start));
    if (tok == static_cast<int>(TokenKind::TK_ID)) {
        currentToken_.value.p = const_cast<char*>(
            ctx_.interner().intern(reinterpret_cast<char*>(start), cursor_ - start));
        currentToken_.id = static_cast<const char*>(currentToken_.value.p);
    }
    return static_cast<TokenKind>(tok);
}

TokenKind Lexer::scanPlus() {
    cursor_++;
    if (*cursor_ == '+') { cursor_++; return TokenKind::TK_INC; }
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_ADD_ASSIGN; }
    return TokenKind::TK_ADD;
}

TokenKind Lexer::scanMinus() {
    cursor_++;
    if (*cursor_ == '-') { cursor_++; return TokenKind::TK_DEC; }
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_SUB_ASSIGN; }
    if (*cursor_ == '>') { cursor_++; return TokenKind::TK_POINTER; }
    return TokenKind::TK_SUB;
}

TokenKind Lexer::scanStar() {
    cursor_++;
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_MUL_ASSIGN; }
    return TokenKind::TK_MUL;
}

TokenKind Lexer::scanSlash() {
    cursor_++;
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_DIV_ASSIGN; }
    return TokenKind::TK_DIV;
}

TokenKind Lexer::scanPercent() {
    cursor_++;
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_MOD_ASSIGN; }
    return TokenKind::TK_MOD;
}

TokenKind Lexer::scanLess() {
    cursor_++;
    if (*cursor_ == '<') {
        cursor_++;
        if (*cursor_ == '=') { cursor_++; return TokenKind::TK_LSHIFT_ASSIGN; }
        return TokenKind::TK_LSHIFT;
    }
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_LESS_EQ; }
    return TokenKind::TK_LESS;
}

TokenKind Lexer::scanGreat() {
    cursor_++;
    if (*cursor_ == '>') {
        cursor_++;
        if (*cursor_ == '=') { cursor_++; return TokenKind::TK_RSHIFT_ASSIGN; }
        return TokenKind::TK_RSHIFT;
    }
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_GREAT_EQ; }
    return TokenKind::TK_GREAT;
}

TokenKind Lexer::scanExclamation() {
    cursor_++;
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_UNEQUAL; }
    return TokenKind::TK_NOT;
}

TokenKind Lexer::scanEqual() {
    cursor_++;
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_EQUAL; }
    return TokenKind::TK_ASSIGN;
}

TokenKind Lexer::scanBar() {
    cursor_++;
    if (*cursor_ == '|') { cursor_++; return TokenKind::TK_OR; }
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_BITOR_ASSIGN; }
    return TokenKind::TK_BITOR;
}

TokenKind Lexer::scanAmpersand() {
    cursor_++;
    if (*cursor_ == '&') { cursor_++; return TokenKind::TK_AND; }
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_BITAND_ASSIGN; }
    return TokenKind::TK_BITAND;
}

TokenKind Lexer::scanCaret() {
    cursor_++;
    if (*cursor_ == '=') { cursor_++; return TokenKind::TK_BITXOR_ASSIGN; }
    return TokenKind::TK_BITXOR;
}

TokenKind Lexer::scanDot() {
    if (isDigit(cursor_[1])) return scanFloatLiteral(cursor_);
    if (cursor_[1] == '.' && cursor_[2] == '.') { cursor_ += 3; return TokenKind::TK_ELLIPSE; }
    cursor_++;
    return TokenKind::TK_DOT;
}

TokenKind Lexer::scanBadChar() {
    ctx_.diagnostics().error(currentToken_.loc,
        "illegal character: \\x" + std::to_string(*cursor_));
    return nextToken().kind;
}

TokenKind Lexer::scanEOF() {
    return TokenKind::TK_END;
}

// Lookup table for single-character tokens, default to TK_END sentinel
static TokenKind singleCharTokens[256] = {};

static void initSingleCharTokens() {
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    for (auto& t : singleCharTokens) t = TokenKind::TK_END;
    using TK = TokenKind;
    singleCharTokens['{'] = TK::TK_LBRACE;
    singleCharTokens['}'] = TK::TK_RBRACE;
    singleCharTokens['['] = TK::TK_LBRACKET;
    singleCharTokens[']'] = TK::TK_RBRACKET;
    singleCharTokens['('] = TK::TK_LPAREN;
    singleCharTokens[')'] = TK::TK_RPAREN;
    singleCharTokens[','] = TK::TK_COMMA;
    singleCharTokens[';'] = TK::TK_SEMICOLON;
    singleCharTokens['~'] = TK::TK_COMP;
    singleCharTokens['?'] = TK::TK_QUESTION;
    singleCharTokens[':'] = TK::TK_COLON;
}

TokenKind Lexer::scanSingle() {
    cursor_++;
    return singleCharTokens[*(cursor_ - 1)];
}

void Lexer::setupScanners() {
    initSingleCharTokens();
    for (int i = 0; i < 256; ++i) {
        if (isLetter(i)) scanners_[i] = &Lexer::scanIdentifier;
        else if (isDigit(i)) scanners_[i] = &Lexer::scanNumericLiteral;
        else if (singleCharTokens[i] != TokenKind::TK_END) scanners_[i] = &Lexer::scanSingle;
        else scanners_[i] = &Lexer::scanBadChar;
    }
    scanners_[END_OF_FILE] = &Lexer::scanEOF;
    scanners_['\''] = &Lexer::scanCharLiteral;
    scanners_['"']  = &Lexer::scanStringLiteral;
    scanners_['+']  = &Lexer::scanPlus;
    scanners_['-']  = &Lexer::scanMinus;
    scanners_['*']  = &Lexer::scanStar;
    scanners_['/']  = &Lexer::scanSlash;
    scanners_['%']  = &Lexer::scanPercent;
    scanners_['<']  = &Lexer::scanLess;
    scanners_['>']  = &Lexer::scanGreat;
    scanners_['!']  = &Lexer::scanExclamation;
    scanners_['=']  = &Lexer::scanEqual;
    scanners_['|']  = &Lexer::scanBar;
    scanners_['&']  = &Lexer::scanAmpersand;
    scanners_['^']  = &Lexer::scanCaret;
    scanners_['.']  = &Lexer::scanDot;
}

Token Lexer::nextToken() {
    prevCoord_ = currentToken_.loc;
    skipWhiteSpace();
    currentToken_.loc.line = line_;
    currentToken_.loc.col = static_cast<int>(cursor_ - lineHead_ + 1);

    currentToken_.kind = (this->*scanners_[*cursor_])();
    return currentToken_;
}

void Lexer::beginPeek() {
    peekCursor_ = cursor_;
    peekToken_ = currentToken_;
    peekCoord_ = currentToken_.loc;
}

void Lexer::endPeek() {
    cursor_ = peekCursor_;
    currentToken_ = peekToken_;
    currentToken_.loc = peekCoord_;
}

} // namespace TCC_Pro

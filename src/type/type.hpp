#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace TCC_Pro {

enum TypeCateg : int {
    T_CHAR, T_UCHAR, T_SHORT, T_USHORT, T_INT, T_UINT,
    T_LONG, T_ULONG, T_LONGLONG, T_ULONGLONG, T_ENUM,
    T_FLOAT, T_DOUBLE, T_LONGDOUBLE, T_POINTER, T_VOID,
    T_UNION, T_STRUCT, T_ARRAY, T_FUNCTION
};

enum { CONST = 0x1, VOLATILE = 0x2 };

enum TypeCode { I1, U1, I2, U2, I4, U4, F4, F8, V, B };

struct Field {
    int offset = 0;
    const char* id = nullptr;
    int bits = 0;
    int pos = 0;
    class Type* ty = nullptr;
    Field* next = nullptr;
};

struct Parameter {
    const char* id = nullptr;
    class Type* ty = nullptr;
    int reg = 0;
};

struct Signature {
    int hasProto = 0;
    int hasEllipse = 0;
    std::vector<Parameter*> params;
};

class Type {
public:
    TypeCateg categ;
    int qual = 0;
    int align = 0;
    int size = 0;
    Type* bty = nullptr;

    explicit Type(TypeCateg c) : categ(c) {}
    virtual ~Type() = default;

    bool isInteg()    const { return categ <= T_ENUM; }
    bool isUnsigned() const { return categ & 0x1; }
    bool isReal()     const { return categ >= T_FLOAT && categ <= T_LONGDOUBLE; }
    bool isArith()    const { return categ <= T_LONGDOUBLE; }
    bool isScalar()   const { return categ <= T_POINTER; }
    bool isPtr()      const { return categ == T_POINTER; }
    bool isRecord()   const { return categ == T_STRUCT || categ == T_UNION; }
    bool isFunction() const { return categ == T_FUNCTION; }
    bool isArray()    const { return categ == T_ARRAY; }
    bool isVoid()     const { return categ == T_VOID; }
};

class RecordType : public Type {
public:
    const char* id = nullptr;
    Field* flds = nullptr;
    Field** tail = &flds;
    int hasConstFld = 0;
    int hasFlexArray = 0;

    RecordType(TypeCateg c) : Type(c) {}
};

class EnumType : public Type {
public:
    const char* id = nullptr;

    EnumType() : Type(T_ENUM) {}
};

class FunctionType : public Type {
public:
    Signature* sig = nullptr;

    FunctionType() : Type(T_FUNCTION) {}
};

class TypeContext {
public:
    TypeContext();
    ~TypeContext();

    Type* charType()       { return types_[T_CHAR]; }
    Type* ucharType()      { return types_[T_UCHAR]; }
    Type* shortType()      { return types_[T_SHORT]; }
    Type* ushortType()     { return types_[T_USHORT]; }
    Type* intType()        { return types_[T_INT]; }
    Type* uintType()       { return types_[T_UINT]; }
    Type* longType()       { return types_[T_LONG]; }
    Type* ulongType()      { return types_[T_ULONG]; }
    Type* longlongType()   { return types_[T_LONGLONG]; }
    Type* ulonglongType()  { return types_[T_ULONGLONG]; }
    Type* floatType()      { return types_[T_FLOAT]; }
    Type* doubleType()     { return types_[T_DOUBLE]; }
    Type* longdoubleType() { return types_[T_LONGDOUBLE]; }
    Type* pointerType()    { return types_[T_POINTER]; }
    Type* voidType()       { return types_[T_VOID]; }

    Type* type(TypeCateg c) { return types_[c]; }

    Type* qualify(int qual, Type* ty);
    Type* unqual(Type* ty);
    Type* pointerTo(Type* ty);
    Type* arrayOf(int len, Type* ty);
    Type* functionReturn(Type* ty, Signature* sig);
    Type* promote(Type* ty);
    Type* adjustParameter(Type* ty);

    RecordType* startRecord(const char* id, TypeCateg categ);
    Field* addField(Type* ty, const char* id, Type* fty, int bits);
    Field* lookupField(Type* ty, const char* id);
    void endRecord(Type* ty);

    Type* compositeType(Type* ty1, Type* ty2);
    Type* commonRealType(Type* ty1, Type* ty2);
    int isCompatibleType(Type* ty1, Type* ty2);

    TypeCode typeCode(Type* ty);
    std::string typeToString(Type* ty);

    Type* allocType(TypeCateg categ);

    class Arena* arena_ = nullptr;

private:
    Type* types_[T_VOID - T_CHAR + 1]{};
    Type* defaultFunctionType_ = nullptr;
    Type* wcharType_ = nullptr;

    int isCompatibleFunction(FunctionType* fty1, FunctionType* fty2);
    void addOffset(RecordType* rty, int offset);
    void setupBuiltins();
};

} // namespace TCC_Pro

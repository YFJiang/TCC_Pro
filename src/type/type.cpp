#include "type/type.hpp"
#include "common/arena.hpp"
#include <cassert>
#include <cstring>
#include <cstdio>

namespace TCC_Pro {

static constexpr int CHAR_SIZE = 1, SHORT_SIZE = 2, INT_SIZE = 4, LONG_SIZE = 4;
static constexpr int LONG_LONG_SIZE = 8, FLOAT_SIZE = 4, DOUBLE_SIZE = 8, LONG_DOUBLE_SIZE = 12;

TypeContext::TypeContext() {
    setupBuiltins();
}

TypeContext::~TypeContext() {
    // Types are arena-allocated, freed by arena
}

void TypeContext::setupBuiltins() {
    // Allocate built-in type objects
    for (int i = T_CHAR; i <= T_VOID; ++i) {
        types_[i] = new Type(static_cast<TypeCateg>(i));
    }

    types_[T_CHAR]->size = types_[T_UCHAR]->size = CHAR_SIZE;
    types_[T_SHORT]->size = types_[T_USHORT]->size = SHORT_SIZE;
    types_[T_INT]->size = types_[T_UINT]->size = INT_SIZE;
    types_[T_LONG]->size = types_[T_ULONG]->size = LONG_SIZE;
    types_[T_LONGLONG]->size = types_[T_ULONGLONG]->size = LONG_LONG_SIZE;
    types_[T_FLOAT]->size = FLOAT_SIZE;
    types_[T_DOUBLE]->size = DOUBLE_SIZE;
    types_[T_LONGDOUBLE]->size = LONG_DOUBLE_SIZE;
    types_[T_POINTER]->size = INT_SIZE;

    for (int i = T_CHAR; i <= T_VOID; ++i) {
        types_[i]->categ = static_cast<TypeCateg>(i);
        types_[i]->align = types_[i]->size;
    }

    auto* fty = new FunctionType();
    fty->categ = T_FUNCTION;
    fty->align = fty->size = types_[T_POINTER]->size;
    fty->bty = types_[T_INT];
    fty->sig = new Signature;
    fty->sig->hasProto = 0;
    fty->sig->hasEllipse = 0;

    defaultFunctionType_ = fty;
}

Type* TypeContext::allocType(TypeCateg categ) {
    if (arena_) {
        return new (arena_->allocate(sizeof(Type))) Type(categ);
    }
    return new Type(categ);
}

Type* TypeContext::qualify(int qual, Type* ty) {
    if (qual == 0 || qual == ty->qual) return ty;
    auto* qty = allocType(ty->categ);
    *qty = *ty;
    qty->qual |= qual;
    if (ty->qual != 0)
        qty->bty = ty->bty;
    else
        qty->bty = ty;
    return qty;
}

Type* TypeContext::unqual(Type* ty) {
    if (ty->qual) ty = ty->bty;
    return ty;
}

Type* TypeContext::pointerTo(Type* ty) {
    auto* pty = allocType(T_POINTER);
    pty->categ = T_POINTER;
    pty->qual = 0;
    pty->align = types_[T_POINTER]->align;
    pty->size = types_[T_POINTER]->size;
    pty->bty = ty;
    return pty;
}

Type* TypeContext::arrayOf(int len, Type* ty) {
    auto* aty = allocType(T_ARRAY);
    aty->categ = T_ARRAY;
    aty->qual = 0;
    aty->size = len * ty->size;
    aty->align = ty->align;
    aty->bty = ty;
    return aty;
}

Type* TypeContext::functionReturn(Type* ty, Signature* sig) {
    auto* fty = new FunctionType();
    fty->categ = T_FUNCTION;
    fty->qual = 0;
    fty->size = types_[T_POINTER]->size;
    fty->align = types_[T_POINTER]->align;
    fty->sig = sig;
    fty->bty = ty;
    return fty;
}

Type* TypeContext::promote(Type* ty) {
    if (ty->categ < T_INT) return types_[T_INT];
    if (ty->categ == T_FLOAT) return types_[T_DOUBLE];
    return ty;
}

Type* TypeContext::adjustParameter(Type* ty) {
    ty = unqual(ty);
    if (ty->categ == T_ARRAY) return pointerTo(ty->bty);
    if (ty->categ == T_FUNCTION) return pointerTo(ty);
    return ty;
}

RecordType* TypeContext::startRecord(const char* id, TypeCateg categ) {
    auto* rty = new RecordType(categ);
    rty->id = id;
    rty->tail = &rty->flds;
    return rty;
}

Field* TypeContext::addField(Type* ty, const char* id, Type* fty, int bits) {
    auto* rty = static_cast<RecordType*>(ty);
    if (fty->size == 0) {
        assert(fty->categ == T_ARRAY);
        rty->hasFlexArray = 1;
    }
    if (fty->qual & CONST) rty->hasConstFld = 1;

    auto* fld = new Field();
    fld->id = id;
    fld->ty = fty;
    fld->bits = bits;
    fld->next = nullptr;
    *rty->tail = fld;
    rty->tail = &fld->next;
    return fld;
}

Field* TypeContext::lookupField(Type* ty, const char* id) {
    auto* rty = static_cast<RecordType*>(ty);
    for (auto* fld = rty->flds; fld; fld = fld->next) {
        if (fld->id == nullptr && fld->ty->isRecord()) {
            auto* p = lookupField(fld->ty, id);
            if (p) return p;
        } else if (fld->id == id) {
            return fld;
        }
    }
    return nullptr;
}

void TypeContext::addOffset(RecordType* rty, int offset) {
    for (auto* fld = rty->flds; fld; fld = fld->next) {
        fld->offset += offset;
        if (fld->id == nullptr && fld->ty->isRecord())
            addOffset(static_cast<RecordType*>(fld->ty), fld->offset);
    }
}

void TypeContext::endRecord(Type* ty) {
    auto* rty = static_cast<RecordType*>(ty);
    int bits = 0;
    int intSize = types_[T_INT]->size;
    int intBits = intSize * 8;

    if (rty->categ == T_STRUCT) {
        for (auto* fld = rty->flds; fld; fld = fld->next) {
            fld->offset = rty->size = (rty->size + fld->ty->align - 1) & ~(fld->ty->align - 1);
            if (!fld->id && fld->ty->isRecord())
                addOffset(static_cast<RecordType*>(fld->ty), fld->offset);
            if (fld->bits == 0) {
                if (bits != 0)
                    fld->offset = rty->size = (rty->size + intSize + fld->ty->align - 1) & ~(fld->ty->align - 1);
                bits = 0;
                rty->size += fld->ty->size;
            } else if (bits + fld->bits <= intBits) {
                fld->pos = bits;
                bits += fld->bits;
                if (bits == intBits) { rty->size += intSize; bits = 0; }
            } else {
                rty->size += intSize;
                fld->offset += intSize;
                fld->pos = 0;
                bits = fld->bits;
            }
            if (fld->ty->align > rty->align) rty->align = fld->ty->align;
        }
        if (bits != 0) rty->size += intSize;
        rty->size = (rty->size + rty->align - 1) & ~(rty->align - 1);
    } else {
        for (auto* fld = rty->flds; fld; fld = fld->next) {
            if (fld->ty->align > rty->align) rty->align = fld->ty->align;
            if (fld->ty->size > rty->size) rty->size = fld->ty->size;
        }
    }
}

int TypeContext::isCompatibleFunction(FunctionType* fty1, FunctionType* fty2) {
    if (!isCompatibleType(fty1->bty, fty2->bty)) return 0;

    auto* sig1 = fty1->sig;
    auto* sig2 = fty2->sig;

    if (!sig1->hasProto && !sig2->hasProto) return 1;

    if (sig1->hasProto && sig2->hasProto) {
        int len1 = sig1->params.size();
        int len2 = sig2->params.size();
        if (sig1->hasEllipse != sig2->hasEllipse || len1 != len2) return 0;
        for (int i = 0; i < len1; ++i)
            if (!isCompatibleType(sig1->params[i]->ty, sig2->params[i]->ty)) return 0;
        return 1;
    }

    if (!sig1->hasProto && sig2->hasProto) {
        std::swap(sig1, sig2);
        std::swap(fty1, fty2);
    }

    int len1 = sig1->params.size();
    int len2 = sig2->params.size();
    if (sig1->hasEllipse) return 0;
    if (len2 == 0) {
        for (auto* p : sig1->params)
            if (!isCompatibleType(promote(p->ty), p->ty)) return 0;
        return 1;
    }
    if (len1 != len2) return 0;
    for (int i = 0; i < len1; ++i)
        if (!isCompatibleType(sig1->params[i]->ty, promote(sig2->params[i]->ty))) return 0;
    return 1;
}

int TypeContext::isCompatibleType(Type* ty1, Type* ty2) {
    if (ty1 == ty2) return 1;
    if (ty1->qual != ty2->qual) return 0;
    ty1 = unqual(ty1);
    ty2 = unqual(ty2);

    if (ty1->categ == T_ENUM && ty2 == ty1->bty) return 1;
    if (ty2->categ == T_ENUM && ty1 == ty2->bty) return 1;
    if (ty1->categ != ty2->categ) return 0;

    switch (ty1->categ) {
    case T_POINTER: return isCompatibleType(ty1->bty, ty2->bty);
    case T_ARRAY:
        return isCompatibleType(ty1->bty, ty2->bty) &&
               (ty1->size == ty2->size || ty1->size == 0 || ty2->size == 0);
    case T_FUNCTION:
        return isCompatibleFunction(static_cast<FunctionType*>(ty1), static_cast<FunctionType*>(ty2));
    default: return ty1 == ty2;
    }
}

Type* TypeContext::compositeType(Type* ty1, Type* ty2) {
    assert(isCompatibleType(ty1, ty2));
    if (ty1->categ == T_ENUM) return ty1;
    if (ty2->categ == T_ENUM) return ty2;
    switch (ty1->categ) {
    case T_POINTER:
        return qualify(ty1->qual, pointerTo(compositeType(ty1->bty, ty2->bty)));
    case T_ARRAY: return ty1->size != 0 ? ty1 : ty2;
    case T_FUNCTION: {
        auto* fty1 = static_cast<FunctionType*>(ty1);
        auto* fty2 = static_cast<FunctionType*>(ty2);
        fty1->bty = compositeType(fty1->bty, fty2->bty);
        if (fty1->sig->hasProto && fty2->sig->hasProto) {
            int len = fty1->sig->params.size();
            for (int i = 0; i < len; ++i)
                fty1->sig->params[i]->ty = compositeType(
                    fty1->sig->params[i]->ty, fty2->sig->params[i]->ty);
            return ty1;
        }
        return fty1->sig->hasProto ? ty1 : ty2;
    }
    default: return ty1;
    }
}

Type* TypeContext::commonRealType(Type* ty1, Type* ty2) {
    if (ty1->categ == T_LONGDOUBLE || ty2->categ == T_LONGDOUBLE) return types_[T_LONGDOUBLE];
    if (ty1->categ == T_DOUBLE || ty2->categ == T_DOUBLE) return types_[T_DOUBLE];
    if (ty1->categ == T_FLOAT || ty2->categ == T_FLOAT) return types_[T_FLOAT];

    ty1 = ty1->categ < T_INT ? types_[T_INT] : ty1;
    ty2 = ty2->categ < T_INT ? types_[T_INT] : ty2;

    if (ty1->categ == ty2->categ) return ty1;
    if (!(ty1->isUnsigned() ^ ty2->isUnsigned()))
        return ty1->categ > ty2->categ ? ty1 : ty2;

    if (ty2->isUnsigned()) std::swap(ty1, ty2);

    if (ty1->categ >= ty2->categ) return ty1;
    if (ty2->size > ty1->size) return ty2;
    return types_[ty2->categ + 1];
}

TypeCode TypeContext::typeCode(Type* ty) {
    static TypeCode codes[] = {I1, U1, I2, U2, I4, U4, I4, U4, I4, U4, I4, F4, F8, F8, U4, V, B, B, B};
    assert(ty->categ != T_FUNCTION);
    return codes[ty->categ];
}

std::string TypeContext::typeToString(Type* ty) {
    const char* names[] = {
        "char", "unsigned char", "short", "unsigned short", "int", "unsigned int",
        "long", "unsigned long", "long long", "unsigned long long", "enum",
        "float", "double", "long double"
    };

    if (ty->qual != 0) {
        int qual = ty->qual;
        ty = unqual(ty);
        const char* qs = (qual == CONST) ? "const" : (qual == VOLATILE) ? "volatile" : "const volatile";
        char buf[256];
        snprintf(buf, sizeof(buf), "%s %s", qs, typeToString(ty).c_str());
        return buf;
    }

    if (ty->categ >= T_CHAR && ty->categ <= T_LONGDOUBLE && ty->categ != T_ENUM)
        return names[ty->categ];

    switch (ty->categ) {
    case T_ENUM: {
        char buf[128]; snprintf(buf, sizeof(buf), "enum %s",
            static_cast<EnumType*>(ty)->id); return buf;
    }
    case T_POINTER: return typeToString(ty->bty) + " *";
    case T_UNION: {
        char buf[128]; snprintf(buf, sizeof(buf), "union %s",
            static_cast<RecordType*>(ty)->id); return buf;
    }
    case T_STRUCT: {
        char buf[128]; snprintf(buf, sizeof(buf), "struct %s",
            static_cast<RecordType*>(ty)->id); return buf;
    }
    case T_ARRAY: {
        char buf[128]; snprintf(buf, sizeof(buf), "%s[%d]",
            typeToString(ty->bty).c_str(), ty->size / ty->bty->size); return buf;
    }
    case T_VOID: return "void";
    case T_FUNCTION: return typeToString(ty->bty) + " ()";
    default: return "???";
    }
}

} // namespace TCC_Pro

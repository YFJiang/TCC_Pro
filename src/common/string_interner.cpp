#include "common/string_interner.hpp"
#include "common/arena.hpp"
#include <cstring>

namespace TCC_Pro {

unsigned StringInterner::elfHash(const char* str, size_t len) {
    unsigned h = 0;
    for (size_t i = 0; i < len; ++i) {
        h = (h << 4) + static_cast<unsigned char>(*str++);
        unsigned x = h & 0xF0000000u;
        if (x != 0) {
            h ^= x >> 24;
            h &= ~x;
        }
    }
    return h;
}

StringInterner::StringInterner(Arena& arena) : arena_(arena) {}

const char* StringInterner::intern(const char* s, size_t len) {
    unsigned h = elfHash(s, len) & HASH_MASK;
    for (Bucket* p = buckets_[h]; p; p = p->link) {
        if (p->len == static_cast<int>(len) && std::strncmp(p->name, s, len) == 0) {
            return p->name;
        }
    }

    auto* p = static_cast<Bucket*>(arena_.allocate(sizeof(Bucket)));
    p->name = static_cast<char*>(arena_.allocate(len + 1));
    std::memcpy(p->name, s, len);
    p->name[len] = '\0';
    p->len = static_cast<int>(len);
    p->link = buckets_[h];
    buckets_[h] = p;
    return p->name;
}

const char* StringInterner::intern(const char* s) {
    return intern(s, std::strlen(s));
}

const char* StringInterner::intern(std::string_view s) {
    return intern(s.data(), s.size());
}

void StringInterner::reset() {
    for (auto& b : buckets_) {
        b = nullptr;
    }
}

} // namespace TCC_Pro

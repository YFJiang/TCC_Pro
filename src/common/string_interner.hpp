#pragma once

#include <string_view>
#include <cstddef>

namespace TCC_Pro {

class Arena;

class StringInterner {
public:
    static constexpr size_t HASH_MASK = 1023;

    explicit StringInterner(Arena& arena);

    StringInterner(const StringInterner&) = delete;
    StringInterner& operator=(const StringInterner&) = delete;

    const char* intern(const char* s, size_t len);
    const char* intern(const char* s);
    const char* intern(std::string_view s);

    void reset();

private:
    struct Bucket {
        char* name;
        int len;
        Bucket* link;
    };

    static unsigned elfHash(const char* str, size_t len);

    Bucket* buckets_[HASH_MASK + 1]{};
    Arena& arena_;
};

} // namespace TCC_Pro

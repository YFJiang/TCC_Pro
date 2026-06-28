#pragma once

#include <cstddef>
#include <cstdint>

namespace TCC_Pro {

class Arena {
public:
    static constexpr size_t DEFAULT_BLOCK_SIZE = 4 * 1024;

    explicit Arena(size_t blockSize = DEFAULT_BLOCK_SIZE);
    ~Arena();

    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;
    Arena(Arena&&) noexcept;
    Arena& operator=(Arena&&) noexcept;

    void* allocate(size_t size, size_t alignment = alignof(std::max_align_t));
    void reset();
    void clear();

private:
    struct Block {
        Block* next;
        char* begin;
        char* avail;
        char* end;
    };

    static Block* freeBlocks_;

    Block head_;
    Block* last_;
    size_t blockSize_;
};

} // namespace TCC_Pro

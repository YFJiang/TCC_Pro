#include "common/arena.hpp"
#include <cstdlib>
#include <new>

namespace TCC_Pro {

Arena::Block* Arena::freeBlocks_ = nullptr;

static inline size_t alignUp(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

Arena::Arena(size_t blockSize)
    : head_{nullptr, nullptr, nullptr, nullptr}
    , last_(&head_)
    , blockSize_(blockSize)
{}

Arena::~Arena() {
    for (Block* b = head_.next; b;) {
        Block* next = b->next;
        std::free(b);
        b = next;
    }
    for (Block* b = freeBlocks_; b;) {
        Block* next = b->next;
        std::free(b);
        b = next;
    }
}

Arena::Arena(Arena&& other) noexcept
    : head_(other.head_)
    , last_(other.last_ == &other.head_ ? &head_ : other.last_)
    , blockSize_(other.blockSize_)
{
    other.head_ = {nullptr, nullptr, nullptr, nullptr};
    other.last_ = &other.head_;
}

Arena& Arena::operator=(Arena&& other) noexcept {
    if (this != &other) {
        this->~Arena();
        head_ = other.head_;
        last_ = other.last_ == &other.head_ ? &head_ : other.last_;
        blockSize_ = other.blockSize_;
        other.head_ = {nullptr, nullptr, nullptr, nullptr};
        other.last_ = &other.head_;
    }
    return *this;
}

void* Arena::allocate(size_t size, size_t alignment) {
    size = alignUp(size, alignment);

    Block* blk = last_;
    while (static_cast<size_t>(blk->end - blk->avail) < size) {
        if ((blk->next = freeBlocks_) != nullptr) {
            freeBlocks_ = freeBlocks_->next;
            blk = blk->next;
        } else {
            size_t m = size + blockSize_ + sizeof(Block);
            blk->next = static_cast<Block*>(std::malloc(m));
            blk = blk->next;
            if (blk == nullptr) {
                throw std::bad_alloc();
            }
            blk->end = reinterpret_cast<char*>(blk) + m;
        }
        blk->avail = blk->begin = reinterpret_cast<char*>(blk + 1);
        blk->next = nullptr;
        last_ = blk;
    }

    blk->avail += size;
    return blk->avail - size;
}

void Arena::reset() {
    if (last_->next) {
        last_->next->next = freeBlocks_;
        freeBlocks_ = last_->next;
        last_->next = nullptr;
    }
    last_ = &head_;
    if (head_.next) {
        head_.next->avail = head_.next->begin;
        last_ = head_.next;
    }
}

void Arena::clear() {
    last_->next = freeBlocks_;
    freeBlocks_ = head_.next;
    head_.next = nullptr;
    last_ = &head_;
}

} // namespace TCC_Pro

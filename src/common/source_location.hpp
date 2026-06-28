#pragma once

#include <cstdint>

namespace TCC_Pro {

struct SourceLocation {
    const char* filename = nullptr;
    int ppline = 0;
    int line = 0;
    int col = 0;

    bool valid() const { return filename != nullptr; }
};

} // namespace TCC_Pro

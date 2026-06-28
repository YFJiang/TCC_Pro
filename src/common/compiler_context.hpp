#pragma once

#include "common/arena.hpp"
#include "common/string_interner.hpp"
#include "common/source_location.hpp"
#include "common/diagnostics.hpp"
#include "type/type.hpp"
#include "symbol/symbol.hpp"

namespace TCC_Pro {

class CompilerContext {
public:
    CompilerContext();

    Arena& programArena() { return programArena_; }
    Arena& fileArena() { return fileArena_; }
    StringInterner& interner() { return interner_; }
    Diagnostics& diagnostics() { return diag_; }
    TypeContext& types() { return types_; }
    SymbolTable& symbols() { return syms_; }

    void beginFile();
    void endFile();

private:
    Arena programArena_;
    Arena fileArena_;
    StringInterner interner_;
    Diagnostics diag_;
    TypeContext types_;
    SymbolTable syms_;
};

} // namespace TCC_Pro

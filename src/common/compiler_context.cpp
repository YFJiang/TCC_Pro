#include "common/compiler_context.hpp"

namespace TCC_Pro {

CompilerContext::CompilerContext()
    : programArena_()
    , fileArena_()
    , interner_(programArena_)
    , diag_()
    , types_()
    , syms_()
{}

void CompilerContext::beginFile() {
    fileArena_.reset();
    diag_.reset();
}

void CompilerContext::endFile() {
    fileArena_.reset();
}

} // namespace TCC_Pro

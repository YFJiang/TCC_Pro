#pragma once

#include "codegen/backend.hpp"
#include "codegen/x86_backend.hpp"
#include "common/compiler_context.hpp"
#include <string>
#include <memory>
#include <vector>

namespace TCC_Pro {

class Driver {
public:
    Driver(CompilerContext& ctx);

    bool compileFile(const std::string& file, const std::string& output);

    void setTarget(const std::string& target);
    void setDumpAST(bool v) { dumpAST_ = v; }
    void setDumpIR(bool v) { dumpIR_ = v; }

private:
    CompilerContext& ctx_;
    std::unique_ptr<X86LinuxBackend> backend_;
    bool dumpAST_ = false;
    bool dumpIR_ = false;
};

} // namespace TCC_Pro

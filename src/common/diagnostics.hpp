#pragma once

#include "common/source_location.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <iostream>

namespace TCC_Pro {

enum class DiagLevel {
    Warning,
    Error,
    Fatal,
};

struct Diagnostic {
    DiagLevel level;
    SourceLocation loc;
    std::string message;
};

class Diagnostics {
public:
    Diagnostics() : output_(&std::cerr) {}

    void setOutput(std::ostream& os) { output_ = &os; }

    void error(SourceLocation loc, std::string_view msg);
    void error(std::string_view msg);
    void warning(SourceLocation loc, std::string_view msg);
    void warning(std::string_view msg);
    [[noreturn]] void fatal(std::string_view msg);

    int errorCount() const { return errorCount_; }
    int warningCount() const { return warningCount_; }
    bool hasErrors() const { return errorCount_ > 0; }

    void reset();

private:
    void emit(DiagLevel level, SourceLocation loc, std::string_view msg);

    int errorCount_ = 0;
    int warningCount_ = 0;
    std::ostream* output_;
};

} // namespace TCC_Pro

#include "common/diagnostics.hpp"
#include <sstream>
#include <cstdlib>

namespace TCC_Pro {

void Diagnostics::emit(DiagLevel level, SourceLocation loc, std::string_view msg) {
    std::ostringstream oss;
    if (loc.valid()) {
        oss << "(" << loc.filename << "," << loc.ppline << "):";
    }
    switch (level) {
    case DiagLevel::Warning:  oss << "warning:"; break;
    case DiagLevel::Error:    oss << "error:"; break;
    case DiagLevel::Fatal:    oss << "fatal:"; break;
    }
    oss << msg << "\n";
    *output_ << oss.str();
}

void Diagnostics::error(SourceLocation loc, std::string_view msg) {
    ++errorCount_;
    emit(DiagLevel::Error, loc, msg);
}

void Diagnostics::error(std::string_view msg) {
    error(SourceLocation{}, msg);
}

void Diagnostics::warning(SourceLocation loc, std::string_view msg) {
    ++warningCount_;
    emit(DiagLevel::Warning, loc, msg);
}

void Diagnostics::warning(std::string_view msg) {
    warning(SourceLocation{}, msg);
}

void Diagnostics::fatal(std::string_view msg) {
    emit(DiagLevel::Fatal, SourceLocation{}, msg);
    std::exit(EXIT_FAILURE);
}

void Diagnostics::reset() {
    errorCount_ = 0;
    warningCount_ = 0;
}

} // namespace TCC_Pro

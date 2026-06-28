#include "driver/driver.hpp"
#include "lex/lexer.hpp"
#include "parse/parser.hpp"
#include "ast/ast_dumper.hpp"
#include "sema/sema.hpp"
#include "ir/ir_gen.hpp"
#include <iostream>
#include <fstream>

namespace TCC_Pro {

Driver::Driver(CompilerContext& ctx)
    : ctx_(ctx)
    , backend_(std::make_unique<X86LinuxBackend>(ctx_.symbols(), ctx_.types()))
{}

void Driver::setTarget(const std::string&) {
    // Only x86linux supported for now
}

bool Driver::compileFile(const std::string& file, const std::string& output) {
    ctx_.beginFile();

    // 1. Lex + Parse
    Lexer lexer(ctx_);
    if (!lexer.open(file)) { ctx_.endFile(); return false; }

    Parser parser(ctx_, lexer);
    TranslationUnit* unit = parser.parseTranslationUnit();

    if (ctx_.diagnostics().hasErrors()) { ctx_.endFile(); return false; }

    if (dumpAST_) {
        AstDumper dumper(std::cout);
        unit->accept(dumper);
    }

    // 2. Semantic check
    SemanticChecker checker(ctx_.symbols(), ctx_.types(), ctx_.diagnostics());
    checker.check(unit);
    if (ctx_.diagnostics().hasErrors()) { ctx_.endFile(); return false; }

    // 3. IR Generation
    IRGenerator irgen(ctx_.symbols(), ctx_.types(), ctx_.diagnostics());
    irgen.generate(unit);

    if (dumpIR_) {
        // TODO: IR dump
    }

    // 4. Code emission
    backend_->beginProgram();

    // Emit data segment: strings
    bool hasData = false;
    for (auto* s = ctx_.symbols().strings; s; s = s->next) {
        if (!hasData) { backend_->segment(DATA); hasData = true; }
        backend_->defineGlobal(s);
        backend_->defineString(s->val.p, 0); // size will be determined by null terminator
    }
    if (hasData) backend_->segment(CODE);
    else backend_->segment(CODE);

    // Emit functions
    for (auto* func : irgen.functions()) {
        backend_->emitFunction(func);
    }

    backend_->endProgram();

    // 5. Write assembly output
    auto asmName = output + ".s";
    {
        std::ofstream ofs(asmName);
        ofs << backend_->getOutput();
    }

    // 6. Assemble + link (Linux, 32-bit)
    std::string cmd = "as --32 -o " + output + ".o " + asmName +
                      " && gcc -m32 -o " + output + " " + output + ".o";
    int ret = system(cmd.c_str());

    ctx_.endFile();
    return ret == 0;
}

} // namespace TCC_Pro

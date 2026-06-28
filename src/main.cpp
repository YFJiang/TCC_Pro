#include "common/compiler_context.hpp"
#include "driver/driver.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    bool dumpAST = false;
    bool dumpIR = false;
    std::string file;
    std::string output = "a.out";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dump-ast") {
            dumpAST = true;
        } else if (arg == "--dump-IR") {
            dumpIR = true;
        } else if (arg == "-o" && i + 1 < argc) {
            output = argv[++i];
        } else if (arg.rfind("--", 0) != 0 && arg[0] != '-') {
            file = arg;
        }
    }

    if (file.empty()) {
        std::cerr << "usage: TCC_Pro [--dump-ast] [--dump-IR] [-o out] file\n";
        return 1;
    }

    TCC_Pro::CompilerContext ctx;
    TCC_Pro::Driver driver(ctx);

    driver.setDumpAST(dumpAST);
    driver.setDumpIR(dumpIR);

    bool ok = driver.compileFile(file, output);
    return ok ? 0 : 1;
}

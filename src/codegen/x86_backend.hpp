#pragma once

#include "codegen/backend.hpp"
#include "ir/ir.hpp"
#include "symbol/symbol.hpp"
#include "type/type.hpp"
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>

namespace TCC_Pro {

class X86LinuxBackend : public Backend {
public:
    X86LinuxBackend(SymbolTable& syms, TypeContext& types);

    void beginProgram() override;
    void endProgram() override;
    void segment(int seg) override;
    void import(Symbol* p) override;
    void export_(Symbol* p) override;
    void defineGlobal(Symbol* p) override;
    void defineCommData(Symbol* p) override;
    void defineString(void* str, int size) override;
    void defineFloatConstant(Symbol* p) override;
    void defineAddress(Symbol* p) override;
    void defineLabel(Symbol* p) override;
    void defineValue(Type* ty, Value val) override;
    void space(int size) override;
    void emitFunction(Symbol* func) override;
    void emitPrologue() override;
    void emitEpilogue() override;
    void emitInst(int opcode, Symbol* opds[]) override;
    void putASMCode(int code, Symbol* opds[]) override;
    void setupRegisters() override;
    void clearRegs() override;
    Symbol* getReg() override;
    Symbol* getByteReg() override;
    Symbol* getWordReg() override;
    void storeVar(Symbol* reg, Symbol* v) override;
    void spillReg(Symbol* reg) override;
    std::string targetName() const override { return "x86linux"; }

    std::string getOutput();

    void emitFunctionBody(FunctionSymbol* fsym);

private:
    void emit(const std::string& s);
    void emit(const char* s);
    void emitLine(const std::string& s);
    const char* getAccessName(Symbol* p);
    void allocateReg(IRInst* inst, int index);
    void emitIR(IRInst* inst);

    SymbolTable& syms_;
    TypeContext& types_;
    std::ostringstream out_;
    std::string lastLabel_;
    int org_ = 0;

    struct RegInfo {
        Symbol* reg;
        Symbol* byteReg;
        Symbol* wordReg;
    };
    RegInfo regs_[8];
    int usedRegs_ = 0;
    int floatNum_ = 0;
    int tempNum_ = 0;
    int stackOffset_ = -4;
    std::unordered_map<Symbol*, int> stackSlots_;
};

} // namespace TCC_Pro

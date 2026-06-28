#pragma once

#include "lex/token.hpp"
#include <string>
#include <vector>
#include <memory>

namespace TCC_Pro {

class Symbol;
class Type;

enum { CODE, DATA };

class Backend {
public:
    virtual ~Backend() = default;

    virtual void beginProgram() = 0;
    virtual void endProgram() = 0;

    virtual void segment(int seg) = 0;
    virtual void import(Symbol* p) = 0;
    virtual void export_(Symbol* p) = 0;
    virtual void defineGlobal(Symbol* p) = 0;
    virtual void defineCommData(Symbol* p) = 0;
    virtual void defineString(void* str, int size) = 0;
    virtual void defineFloatConstant(Symbol* p) = 0;
    virtual void defineAddress(Symbol* p) = 0;
    virtual void defineLabel(Symbol* p) = 0;
    virtual void defineValue(Type* ty, Value val) = 0;
    virtual void space(int size) = 0;

    virtual void emitFunction(Symbol* func) = 0;
    virtual void emitPrologue() = 0;
    virtual void emitEpilogue() = 0;
    virtual void emitInst(int opcode, Symbol* opds[]) = 0;

    virtual void putASMCode(int code, Symbol* opds[]) = 0;

    virtual void setupRegisters() = 0;
    virtual void clearRegs() = 0;
    virtual Symbol* getReg() = 0;
    virtual Symbol* getByteReg() = 0;
    virtual Symbol* getWordReg() = 0;
    virtual void storeVar(Symbol* reg, Symbol* v) = 0;
    virtual void spillReg(Symbol* reg) = 0;

    virtual std::string targetName() const = 0;
};

class X86Backend : public Backend {
public:
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
    std::string targetName() const override { return "x86"; }

private:
    // TODO: x86 register management
};

class ArmBackend : public Backend {
public:
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
    std::string targetName() const override { return "arm"; }
};

class BackendFactory {
public:
    static std::unique_ptr<Backend> create(const std::string& target);
};

} // namespace TCC_Pro

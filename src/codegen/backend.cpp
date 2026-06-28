#include "codegen/backend.hpp"
#include <stdexcept>
#include <iostream>

namespace TCC_Pro {

// X86Backend stubs
void X86Backend::beginProgram() {}
void X86Backend::endProgram() {}
void X86Backend::segment(int) {}
void X86Backend::import(Symbol*) {}
void X86Backend::export_(Symbol*) {}
void X86Backend::defineGlobal(Symbol*) {}
void X86Backend::defineCommData(Symbol*) {}
void X86Backend::defineString(void*, int) {}
void X86Backend::defineFloatConstant(Symbol*) {}
void X86Backend::defineAddress(Symbol*) {}
void X86Backend::defineLabel(Symbol*) {}
void X86Backend::defineValue(Type*, Value) {}
void X86Backend::space(int) {}
void X86Backend::emitFunction(Symbol*) {}
void X86Backend::emitPrologue() {}
void X86Backend::emitEpilogue() {}
void X86Backend::emitInst(int, Symbol*[]) {}
void X86Backend::putASMCode(int, Symbol*[]) {}
void X86Backend::setupRegisters() {}
void X86Backend::clearRegs() {}
Symbol* X86Backend::getReg() { return nullptr; }
Symbol* X86Backend::getByteReg() { return nullptr; }
Symbol* X86Backend::getWordReg() { return nullptr; }
void X86Backend::storeVar(Symbol*, Symbol*) {}
void X86Backend::spillReg(Symbol*) {}

// ArmBackend stubs (unimplemented)
#define ARM_UNIMPLEMENTED throw std::runtime_error("ARM backend not implemented")

void ArmBackend::beginProgram() { ARM_UNIMPLEMENTED; }
void ArmBackend::endProgram() { ARM_UNIMPLEMENTED; }
void ArmBackend::segment(int) { ARM_UNIMPLEMENTED; }
void ArmBackend::import(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::export_(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::defineGlobal(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::defineCommData(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::defineString(void*, int) { ARM_UNIMPLEMENTED; }
void ArmBackend::defineFloatConstant(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::defineAddress(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::defineLabel(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::defineValue(Type*, Value) { ARM_UNIMPLEMENTED; }
void ArmBackend::space(int) { ARM_UNIMPLEMENTED; }
void ArmBackend::emitFunction(Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::emitPrologue() { ARM_UNIMPLEMENTED; }
void ArmBackend::emitEpilogue() { ARM_UNIMPLEMENTED; }
void ArmBackend::emitInst(int, Symbol*[]) { ARM_UNIMPLEMENTED; }
void ArmBackend::putASMCode(int, Symbol*[]) { ARM_UNIMPLEMENTED; }
void ArmBackend::setupRegisters() { ARM_UNIMPLEMENTED; }
void ArmBackend::clearRegs() { ARM_UNIMPLEMENTED; }
Symbol* ArmBackend::getReg() { ARM_UNIMPLEMENTED; }
Symbol* ArmBackend::getByteReg() { ARM_UNIMPLEMENTED; }
Symbol* ArmBackend::getWordReg() { ARM_UNIMPLEMENTED; }
void ArmBackend::storeVar(Symbol*, Symbol*) { ARM_UNIMPLEMENTED; }
void ArmBackend::spillReg(Symbol*) { ARM_UNIMPLEMENTED; }

std::unique_ptr<Backend> BackendFactory::create(const std::string& target) {
    if (target == "x86" || target == "x86linux" || target == "x86win32") {
        return std::make_unique<X86Backend>();
    }
    if (target == "arm" || target == "aarch64") {
        return std::make_unique<ArmBackend>();
    }
    throw std::runtime_error("Unknown target: " + target);
}

} // namespace TCC_Pro

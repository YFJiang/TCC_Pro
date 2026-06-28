#pragma once

#include <cstdint>
#include <string>

namespace TCC_Pro {

class Type;
class Symbol;

enum class OpCode : int {
    BOR, BXOR, BAND, LSH, RSH,
    ADD, SUB, MUL, DIV, MOD,
    NEG, BCOM,
    JZ, JNZ, JE, JNE, JG, JL, JGE, JLE,
    JMP, IJMP,
    INC, DEC,
    ADDR, DEREF,
    EXTI1, EXTU1, EXTI2, EXTU2,
    TRUI1, TRUI2,
    CVTI4F4, CVTI4F8, CVTU4F4, CVTU4F8,
    CVTF4, CVTF4I4, CVTF4U4,
    CVTF8, CVTF8I4, CVTF8U4,
    MOV, IMOV, CALL, RET, CLR, NOP,
};

struct opcode_info {
    OpCode code;
    const char* name;
    const char* category;  // "Assign", "Branch", "Jump", "Move", etc.
};

const opcode_info* getOpcodeInfo(OpCode code);
const char* opcodeName(OpCode code);

class IRInst {
public:
    IRInst* prev = nullptr;
    IRInst* next = nullptr;
    Type* ty = nullptr;
    OpCode opcode = OpCode::NOP;
    Symbol* opds[3]{};
};

class CFGEdge {
public:
    class BBlock* bb = nullptr;
    CFGEdge* next = nullptr;
};

class BBlock {
public:
    BBlock* prev = nullptr;
    BBlock* next = nullptr;
    Symbol* sym = nullptr;
    CFGEdge* succs = nullptr;
    CFGEdge* preds = nullptr;
    IRInst insth;  // head sentinel
    int ninst = 0;
    int nsucc = 0;
    int npred = 0;
    int ref = 0;
    int no = 0;
};

// IRGenerator is defined in ir/ir_gen.hpp (Visitor subclass)

} // namespace TCC_Pro

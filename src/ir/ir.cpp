#include "ir/ir.hpp"

namespace TCC_Pro {

static constexpr opcode_info opcodeTable[] = {
    {OpCode::BOR,  "|",  "Assign"}, {OpCode::BXOR, "^",  "Assign"},
    {OpCode::BAND, "&",  "Assign"}, {OpCode::LSH,  "<<", "Assign"},
    {OpCode::RSH,  ">>", "Assign"}, {OpCode::ADD,  "+",  "Assign"},
    {OpCode::SUB,  "-",  "Assign"}, {OpCode::MUL,  "*",  "Assign"},
    {OpCode::DIV,  "/",  "Assign"}, {OpCode::MOD,  "%",  "Assign"},
    {OpCode::NEG,  "-",  "Assign"}, {OpCode::BCOM, "~",  "Assign"},
    {OpCode::JZ,   "",   "Branch"}, {OpCode::JNZ,  "!",  "Branch"},
    {OpCode::JE,   "==", "Branch"}, {OpCode::JNE,  "!=", "Branch"},
    {OpCode::JG,   ">",  "Branch"}, {OpCode::JL,   "<",  "Branch"},
    {OpCode::JGE,  ">=", "Branch"}, {OpCode::JLE,  "<=", "Branch"},
    {OpCode::JMP,  "jmp","Jump"},   {OpCode::IJMP, "ijmp","IndirectJump"},
    {OpCode::INC,  "++", "Inc"},    {OpCode::DEC,  "--", "Dec"},
    {OpCode::ADDR, "&",  "Address"},{OpCode::DEREF,"*",  "Deref"},
    {OpCode::EXTI1,"(i)(c)","Cast"},{OpCode::EXTU1,"(i)(uc)","Cast"},
    {OpCode::EXTI2,"(i)(s)","Cast"},{OpCode::EXTU2,"(i)(us)","Cast"},
    {OpCode::TRUI1,"(c)(i)","Cast"},{OpCode::TRUI2,"(s)(i)","Cast"},
    {OpCode::CVTI4F4,"(f)(i)","Cast"},{OpCode::CVTI4F8,"(d)(i)","Cast"},
    {OpCode::CVTU4F4,"(f)(u)","Cast"},{OpCode::CVTU4F8,"(d)(u)","Cast"},
    {OpCode::CVTF4,"(d)(f)","Cast"},
    {OpCode::CVTF4I4,"(i)(f)","Cast"},{OpCode::CVTF4U4,"(u)(f)","Cast"},
    {OpCode::CVTF8,"(f)(d)","Cast"},
    {OpCode::CVTF8I4,"(i)(d)","Cast"},{OpCode::CVTF8U4,"(u)(d)","Cast"},
    {OpCode::MOV,  "=",  "Move"},   {OpCode::IMOV, "*=", "IndirectMove"},
    {OpCode::CALL, "call","Call"},   {OpCode::RET,  "ret","Return"},
    {OpCode::CLR,  "",   "Clear"},   {OpCode::NOP,  "NOP","NOP"},
};

const opcode_info* getOpcodeInfo(OpCode code) {
    for (auto& info : opcodeTable) {
        if (info.code == code) return &info;
    }
    return &opcodeTable[sizeof(opcodeTable)/sizeof(opcodeTable[0]) - 1];
}

const char* opcodeName(OpCode code) {
    return getOpcodeInfo(code)->name;
}

} // namespace TCC_Pro

#ifndef EMU_H
#define EMU_H

#include <cstdint>
#include <cstdlib>
#include "rv32.h"

using uint = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;

// Instruction Decoding
uint signExtend(uint x, uint b);

typedef struct
{
    uint rs1;
    uint rs2;
    uint imm;
} FormatB;

FormatB parse_FormatB(uint word);

typedef struct
{
    uint csr;
    uint rs;
    uint rd;
    uint value;
} FormatCSR;

FormatCSR parse_FormatCSR(uint word);

typedef struct
{
    uint rd;
    uint rs1;
    uint imm;
} FormatI;

FormatI parse_FormatI(uint word);

typedef struct
{
    uint rd;
    uint imm;
} FormatJ;

FormatJ parse_FormatJ(uint word);

typedef struct
{
    uint rd;
    uint rs1;
    uint rs2;
    uint rs3;
} FormatR;

FormatR parse_FormatR(uint word);

typedef struct
{
    uint rs1;
    uint rs2;
    uint imm;
} FormatS;

FormatS parse_FormatS(uint word);

typedef struct
{
    uint rd;
    uint imm;
} FormatU;

FormatU parse_FormatU(uint word);

typedef struct
{
} FormatEmpty;

FormatEmpty parse_FormatEmpty(uint word);



// Emulator
#define def(name, fmt_t) \
    void emu_##name(uint ins_word, ins_ret *ret, fmt_t ins)

const int MEM_SIZE = 1024 * 1024 * 128; // 128MiB

class Emulator
{
public:
    uint8_t *memory;
    RV32 cpu;

    // debugging
    bool singleStep = false;

    Emulator(/* args */);
    ~Emulator();

    void initialize();
    void initializeElf(const char *path);
    void initializeBin(const char *path);
    void emulate(); // formerly cpu_tick
    ins_ret insSelect(uint ins_word);

    // Instruction defintion
    def(add, FormatR); // rv32i
    def(addi, FormatI);      // rv32i
    def(amoswap_w, FormatR); // rv32a
    def(amoadd_w, FormatR); // rv32a
    def(amoxor_w, FormatR); // rv32a
    def(amoand_w, FormatR);  // rv32a
    def(amoor_w, FormatR);  // rv32a
    def(amomin_w, FormatR);  // rv32a
    def(amomax_w, FormatR);  // rv32a
    def(amominu_w, FormatR);  // rv32a
    def(amomaxu_w, FormatR);  // rv32a
    def(and, FormatR); // rv32i
    def(andi, FormatI); // rv32i
    def(auipc, FormatU); // rv32i
    def(beq, FormatB);  // rv32i
    def(bge, FormatB);  // rv32i
    def(bgeu, FormatB);  // rv32i
    def(blt, FormatB);  // rv32i
    def(bltu, FormatB);  // rv32i
    def(bne, FormatB);  // rv32i
    def(csrrc, FormatCSR);  // system
    def(csrrci, FormatCSR);  // system
    def(csrrs, FormatCSR);  // system
    def(csrrsi, FormatCSR);  // system
    def(csrrw, FormatCSR);  // system
    def(csrrwi, FormatCSR);  // system
    def(div, FormatR);  // rv32m
    def(divu, FormatR);  // rv32m
    def(ebreak, FormatEmpty);  // rv32i
    def(ecall, FormatEmpty);  // system
    def(fence, FormatEmpty); // rv32i
    def(fence_i, FormatEmpty);  // rv32i
    def(jal, FormatJ);  // rv32i
    def(jalr, FormatI);  // rv32i
    def(lb, FormatI);  // rv32i
    def(lbu, FormatI);  // rv32i
    def(lh, FormatI);  // rv32i
    def(lhu, FormatI);  // rv32i
    def(lr_w, FormatR);  // rv32a
    def(lui, FormatU); // rv32i
    def(lw, FormatI);  // rv32i
    def(mret, FormatEmpty);  // system
    def(mul, FormatR);  // rv32m
    def(mulh, FormatR);  // rv32m
    def(mulhsu, FormatR);  // rv32m
    def(mulhu, FormatR);  // rv32m
    def(or, FormatR); // rv32i
    def(ori, FormatI); // rv32i
    def(rem, FormatR);  // rv32m
    def(remu, FormatR);  // rv32m
    def(sb, FormatS);  // rv32i
    def(sc_w, FormatR);  // rv32a
    def(sfence_vma, FormatEmpty); // system
    def(sh, FormatS);  // rv32i
    def(sll, FormatR); // rv32i
    def(slli, FormatR);  // rv32i
    def(slt, FormatR);  // rv32i
    def(slti, FormatI);  // rv32i
    def(sltiu, FormatI);  // rv32i
    def(sltu, FormatR);  // rv32i
    def(sra, FormatR);  // rv32i
    def(srai, FormatR);  // rv32i
    def(sret, FormatEmpty);  // system
    def(srl, FormatR); // rv32i
    def(srli, FormatR);  // rv32i
    def(sub, FormatR);  // rv32i
    def(sw, FormatS);  // rv32i
    def(uret, FormatEmpty); // system
    def(wfi, FormatEmpty); // system
    def(xor, FormatR); // rv32i
    def(xori, FormatI); // rv32i
};

#endif
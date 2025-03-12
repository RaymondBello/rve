#include "emu.h"

using uint = uint32_t;
using uint16 = uint16_t;
using uint8 = uint8_t;
////////////////////////////////////////////////////////////////
// Instruction Decoding
////////////////////////////////////////////////////////////////
FormatB parse_FormatB(uint word)
{
    FormatB ret;
    ret.rs1 = (word >> 15) & 0x1f;
    ret.rs2 = (word >> 20) & 0x1f;
    ret.imm = (word & 0x80000000 ? 0xfffff000 : 0) |
              ((word << 4) & 0x00000800) |
              ((word >> 20) & 0x000007e0) |
              ((word >> 7) & 0x0000001e);
    return ret;
}

FormatCSR parse_FormatCSR(uint word)
{
    FormatCSR ret;
    ret.csr = (word >> 20) & 0xfff;
    ret.rs = (word >> 15) & 0x1f;
    ret.rd = (word >> 7) & 0x1f;
    return ret;
}

FormatI parse_FormatI(uint word)
{
    FormatI ret;
    ret.rd = (word >> 7) & 0x1f;
    ret.rs1 = (word >> 15) & 0x1f;
    ret.imm = (word & 0x80000000 ? 0xfffff800 : 0) |
              ((word >> 20) & 0x000007ff);
    return ret;
}

FormatJ parse_FormatJ(uint word)
{
    FormatJ ret;
    ret.rd = (word >> 7) & 0x1f;
    ret.imm = (word & 0x80000000 ? 0xfff00000 : 0) |
              (word & 0x000ff000) |
              ((word & 0x00100000) >> 9) |
              ((word & 0x7fe00000) >> 20);
    return ret;
}

FormatR parse_FormatR(uint word)
{
    FormatR ret;
    ret.rd = (word >> 7) & 0x1f;
    ret.rs1 = (word >> 15) & 0x1f;
    ret.rs2 = (word >> 20) & 0x1f;
    ret.rs3 = (word >> 27) & 0x1f;
    return ret;
}

FormatS parse_FormatS(uint word)
{
    FormatS ret;
    ret.rs1 = (word >> 15) & 0x1f;
    ret.rs2 = (word >> 20) & 0x1f;
    ret.imm = (word & 0x80000000 ? 0xfffff000 : 0) |
              ((word >> 20) & 0xfe0) |
              ((word >> 7) & 0x1f);
    return ret;
}

FormatU parse_FormatU(uint word)
{
    FormatU ret;
    ret.rd = (word >> 7) & 0x1f;
    ret.imm = word & 0xfffff000;
    return ret;
}

FormatEmpty parse_FormatEmpty(uint word)
{
    FormatEmpty ret;
    return ret;
}

////////////////////////////////////////////////////////////////
// Instruction Implement
////////////////////////////////////////////////////////////////
#define AS_SIGNED(val) (*(int32_t *)&val)
#define AS_UNSIGNED(val) (*(uint *)&val)
const uint ZERO = 0;
const uint ONE = 1;

#define imp(name, fmt_t, code) \
    void Emulator::emu_##name(uint ins_word, ins_ret *ret, fmt_t ins) { code }

#define WR_RD(code)                         \
    {                                       \
        ret->write_reg = ins.rd;            \
        ret->write_val = AS_UNSIGNED(code); \
    }
#define WR_PC(code)         \
    {                       \
        ret->pc_val = code; \
    }
#define WR_CSR(code)              \
    {                             \
        ret->csr_write = ins.csr; \
        ret->csr_val = code;      \
    }

imp(add, FormatR, { // rv32i
    WR_RD(AS_SIGNED(cpu.xreg[ins.rs1]) + AS_SIGNED(cpu.xreg[ins.rs2]));
})
imp(addi, FormatI, { // rv32i
    WR_RD(AS_SIGNED(cpu.xreg[ins.rs1]) + AS_SIGNED(ins.imm));
})
imp(amoswap_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    cpu.memSetWord(cpu.xreg[ins.rs1], cpu.xreg[ins.rs2]);
    WR_RD(tmp)
})
imp(amoadd_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    cpu.memSetWord(cpu.xreg[ins.rs1], cpu.xreg[ins.rs2] + tmp);
    WR_RD(tmp)
})
imp(amoxor_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    cpu.memSetWord(cpu.xreg[ins.rs1], cpu.xreg[ins.rs2] ^ tmp);
    WR_RD(tmp)
})
imp(amoand_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    cpu.memSetWord(cpu.xreg[ins.rs1], cpu.xreg[ins.rs2] & tmp);
    WR_RD(tmp)
})
imp(amoor_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    cpu.memSetWord(cpu.xreg[ins.rs1], cpu.xreg[ins.rs2] | tmp);
    WR_RD(tmp)
})
imp(amomin_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    uint sec = cpu.xreg[ins.rs2];
    cpu.memSetWord(cpu.xreg[ins.rs1], AS_SIGNED(sec) < AS_SIGNED(tmp) ? sec : tmp);
    WR_RD(tmp)
})
imp(amomax_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    uint sec = cpu.xreg[ins.rs2];
    cpu.memSetWord(cpu.xreg[ins.rs1], AS_SIGNED(sec) > AS_SIGNED(tmp) ? sec : tmp);
    WR_RD(tmp)
})
imp(amominu_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    uint sec = cpu.xreg[ins.rs2];
    cpu.memSetWord(cpu.xreg[ins.rs1], sec < tmp ? sec : tmp);
    WR_RD(tmp)
})
imp(amomaxu_w, FormatR, { // rv32a
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1]);
    uint sec = cpu.xreg[ins.rs2];
    cpu.memSetWord(cpu.xreg[ins.rs1], sec > tmp ? sec : tmp);
    WR_RD(tmp)
})
imp(and, FormatR, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] & cpu.xreg[ins.rs2])
})
imp(andi, FormatI, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] & ins.imm)
})
imp(auipc, FormatU, {// rv32i
    WR_RD(cpu.pc + ins.imm)
})
imp(beq, FormatB, { // rv32i
    if (cpu.xreg[ins.rs1] == cpu.xreg[ins.rs2])
    {
        WR_PC(cpu.pc + ins.imm);
    }
})
imp(bge, FormatB, { // rv32i
    if (AS_SIGNED(cpu.xreg[ins.rs1]) >= AS_SIGNED(cpu.xreg[ins.rs2]))
    {
        WR_PC(cpu.pc + ins.imm);
    }
})
imp(bgeu, FormatB, { // rv32i
    if (AS_UNSIGNED(cpu.xreg[ins.rs1]) >= AS_UNSIGNED(cpu.xreg[ins.rs2]))
    {
        WR_PC(cpu.pc + ins.imm);
    }
})
imp(blt, FormatB, { // rv32i
    if (AS_SIGNED(cpu.xreg[ins.rs1]) < AS_SIGNED(cpu.xreg[ins.rs2]))
    {
        WR_PC(cpu.pc + ins.imm);
    }
})
imp(bltu, FormatB, { // rv32i
    if (AS_UNSIGNED(cpu.xreg[ins.rs1]) < AS_UNSIGNED(cpu.xreg[ins.rs2]))
    {
        WR_PC(cpu.pc + ins.imm);
    }
})
imp(bne, FormatB, { // rv32i
    if (cpu.xreg[ins.rs1] != cpu.xreg[ins.rs2])
    {
        WR_PC(cpu.pc + ins.imm);
    }
})
imp(csrrc, FormatCSR, { // system
    uint rs = cpu.xreg[ins.rs];
    if (rs != 0)
    {
        WR_CSR(ins.value & ~rs);
    }
    WR_RD(ins.value)
})
imp(csrrci, FormatCSR, { // system
    if (ins.rs != 0)
    {
        WR_CSR(ins.value & (~ins.rs));
    }
    WR_RD(ins.value)
})
imp(csrrs, FormatCSR, { // system
    uint rs = cpu.xreg[ins.rs];
    if (rs != 0)
    {
        WR_CSR(ins.value | rs);
    }
    WR_RD(ins.value)
})
imp(csrrsi, FormatCSR, { // system
    if (ins.rs != 0)
    {
        WR_CSR(ins.value | ins.rs);
    }
    WR_RD(ins.value)
})
imp(csrrw, FormatCSR, { // system
    WR_CSR(cpu.xreg[ins.rs]);
    WR_RD(ins.value)
})
imp(csrrwi, FormatCSR, { // system
    WR_CSR(ins.rs);
    WR_RD(ins.value)
})
imp(div, FormatR, { // rv32m
    uint dividend = cpu.xreg[ins.rs1];
    uint divisor = cpu.xreg[ins.rs2];
    uint result;
    if (divisor == 0)
    {
        result = 0xFFFFFFFF;
    }
    else if (dividend == 0x80000000 && divisor == 0xFFFFFFFF)
    {
        result = dividend;
    }
    else
    {
        int32_t tmp = AS_SIGNED(dividend) / AS_SIGNED(divisor);
        result = AS_UNSIGNED(tmp);
    }
    WR_RD(result)
})
imp(divu, FormatR, { // rv32m
    uint dividend = cpu.xreg[ins.rs1];
    uint divisor = cpu.xreg[ins.rs2];
    uint result;
    if (divisor == 0)
    {
        result = 0xFFFFFFFF;
    }
    else
    {
        result = dividend / divisor;
    }
    WR_RD(result)
})
imp(ebreak, FormatEmpty, {
    // system
    // unnecessary?
})
imp(ecall, FormatEmpty, { // system
    if (cpu.xreg[17] == 93)
    {
        // EXIT CALL
        uint status = cpu.xreg[10] >> 1;
        printf("ecall EXIT = %d (0x%x)\n", status, status);
        exit(status);
    }

    ret->trap.en = true;
    ret->trap.value = cpu.pc;
    if (cpu.csr.privilege == PRIV_USER)
    {
        ret->trap.type = trap_EnvironmentCallFromUMode;
    }
    else if (cpu.csr.privilege == PRIV_SUPERVISOR)
    {
        ret->trap.type = trap_EnvironmentCallFromSMode;
    }
    else
    { // PRIV_MACHINE
        ret->trap.type = trap_EnvironmentCallFromMMode;
    }
})
imp(fence, FormatEmpty, {
    // rv32i
    // skip
})
imp(fence_i, FormatEmpty, {
    // rv32i
    // skip
})
imp(jal, FormatJ, { // rv32i
    WR_RD(cpu.pc + 4);
    WR_PC(cpu.pc + ins.imm);
})
imp(jalr, FormatI, { // rv32i
    WR_RD(cpu.pc + 4);
    WR_PC(cpu.xreg[ins.rs1] + ins.imm);
})
// imp(lb, FormatI, { // rv32i
//     uint tmp = sign_extend(cpu.memGetByte(cpu.xreg[ins.rs1] + ins.imm), 8);
//     WR_RD(tmp)
// })
imp(lbu, FormatI, { // rv32i
    uint tmp = cpu.memGetByte(cpu.xreg[ins.rs1] + ins.imm);
    WR_RD(tmp)
})
// imp(lh, FormatI, { // rv32i
//     uint tmp = sign_extend(cpu.memGetHalfWord(cpu.xreg[ins.rs1] + ins.imm), 16);
//     WR_RD(tmp)
// })
imp(lhu, FormatI, { // rv32i
    uint tmp = cpu.memGetHalfWord(cpu.xreg[ins.rs1] + ins.imm);
    WR_RD(tmp)
})
imp(lr_w, FormatR, { // rv32a
    uint addr = cpu.xreg[ins.rs1];
    uint tmp = cpu.memGetWord(addr);
    cpu.reservation_en = true;
    cpu.reservation_addr = addr;
    WR_RD(tmp)
})
imp(lui, FormatU, {// rv32i
    WR_RD(ins.imm)
})
imp(lw, FormatI, { // rv32i
    // would need sign extend for xlen > 32
    uint tmp = cpu.memGetWord(cpu.xreg[ins.rs1] + ins.imm);
    WR_RD(tmp)
})
imp(mret, FormatEmpty, { // system
    uint newpc = cpu.getCsr(CSR_MEPC, ret);
    if (!ret->trap.en)
    {
        uint status = cpu.readCsrRaw(CSR_MSTATUS);
        uint mpie = (status >> 7) & 1;
        uint mpp = (status >> 11) & 0x3;
        uint mprv = mpp == PRIV_MACHINE ? ((status >> 17) & 1) : 0;
        uint new_status = (status & ~0x21888) | (mprv << 17) | (mpie << 3) | (1 << 7);
        cpu.writeCsrRaw(CSR_MSTATUS, new_status);
        cpu.csr.privilege = mpp;
        WR_PC(newpc)
    }
})
imp(mul, FormatR, { // rv32m
    uint tmp = AS_SIGNED(cpu.xreg[ins.rs1]) * AS_SIGNED(cpu.xreg[ins.rs2]);
    WR_RD(tmp)
})
imp(mulh, FormatR, { // rv32m
    uint tmp = ((int64_t)AS_SIGNED(cpu.xreg[ins.rs1]) * (int64_t)AS_SIGNED(cpu.xreg[ins.rs2])) >> 32;
    WR_RD(tmp)
})
imp(mulhsu, FormatR, { // rv32m
    uint tmp = ((int64_t)AS_SIGNED(cpu.xreg[ins.rs1]) * (uint64_t)AS_UNSIGNED(cpu.xreg[ins.rs2])) >> 32;
    WR_RD(tmp)
})
imp(mulhu, FormatR, { // rv32m
    uint tmp = ((uint64_t)AS_UNSIGNED(cpu.xreg[ins.rs1]) * (uint64_t)AS_UNSIGNED(cpu.xreg[ins.rs2])) >> 32;
    WR_RD(tmp)
})
imp(or, FormatR, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] | cpu.xreg[ins.rs2])
})
imp(ori, FormatI, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] | ins.imm)
})
imp(rem, FormatR, { // rv32m
    uint dividend = cpu.xreg[ins.rs1];
    uint divisor = cpu.xreg[ins.rs2];
    uint result;
    if (divisor == 0)
    {
        result = dividend;
    }
    else if (dividend == 0x80000000 && divisor == 0xFFFFFFFF)
    {
        result = 0;
    }
    else
    {
        int32_t tmp = AS_SIGNED(dividend) % AS_SIGNED(divisor);
        result = AS_UNSIGNED(tmp);
    }
    WR_RD(result)
})
imp(remu, FormatR, { // rv32m
    uint dividend = cpu.xreg[ins.rs1];
    uint divisor = cpu.xreg[ins.rs2];
    uint result;
    if (divisor == 0)
    {
        result = dividend;
    }
    else
    {
        result = dividend % divisor;
    }
    WR_RD(result)
})
imp(sb, FormatS, { // rv32i
    cpu.memSetByte(cpu.xreg[ins.rs1] + ins.imm, cpu.xreg[ins.rs2]);
})
imp(sc_w, FormatR, { // rv32a
    // I'm pretty sure this is not it chief, but it does the trick for now
    uint addr = cpu.xreg[ins.rs1];
    if (cpu.reservation_en && cpu.reservation_addr == addr)
    {
        cpu.memSetWord(addr, cpu.xreg[ins.rs2]);
        cpu.reservation_en = false;
        WR_RD(ZERO)
    }
    else
    {
        WR_RD(ONE)
    }
})
imp(sfence_vma, FormatEmpty, {
    // system
    // skip
})
imp(sh, FormatS, { // rv32i
    cpu.memSetHalfWord(cpu.xreg[ins.rs1] + ins.imm, cpu.xreg[ins.rs2]);
})
imp(sll, FormatR, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] << cpu.xreg[ins.rs2])
})
imp(slli, FormatR, { // rv32i
    uint shamt = (ins_word >> 20) & 0x1F;
    WR_RD(cpu.xreg[ins.rs1] << shamt)
})
imp(slt, FormatR, { // rv32i
    if (AS_SIGNED(cpu.xreg[ins.rs1]) < AS_SIGNED(cpu.xreg[ins.rs2]))
    {
        WR_RD(ONE)
    }
    else
    {
        WR_RD(ZERO)
    }
})
imp(slti, FormatI, { // rv32i
    if (AS_SIGNED(cpu.xreg[ins.rs1]) < AS_SIGNED(ins.imm))
    {
        WR_RD(ONE)
    }
    else
    {
        WR_RD(ZERO)
    }
})
imp(sltiu, FormatI, { // rv32i
    if (AS_UNSIGNED(cpu.xreg[ins.rs1]) < AS_UNSIGNED(ins.imm))
    {
        WR_RD(ONE)
    }
    else
    {
        WR_RD(ZERO)
    }
})
imp(sltu, FormatR, { // rv32i
    if (AS_UNSIGNED(cpu.xreg[ins.rs1]) < AS_UNSIGNED(cpu.xreg[ins.rs2]))
    {
        WR_RD(ONE)
    }
    else
    {
        WR_RD(ZERO)
    }
})
imp(sra, FormatR, { // rv32i
    uint msr = cpu.xreg[ins.rs1] & 0x80000000;
    WR_RD(msr ? ~(~cpu.xreg[ins.rs1] >> cpu.xreg[ins.rs2]) : cpu.xreg[ins.rs1] >> cpu.xreg[ins.rs2])
})
imp(srai, FormatR, { // rv32i
    uint msr = cpu.xreg[ins.rs1] & 0x80000000;
    uint shamt = (ins_word >> 20) & 0x1F;
    WR_RD(msr ? ~(~cpu.xreg[ins.rs1] >> shamt) : cpu.xreg[ins.rs1] >> shamt)
})
imp(sret, FormatEmpty, { // system
    uint newpc = cpu.getCsr(CSR_SEPC, ret);
    if (!ret->trap.en)
    {
        uint status = cpu.readCsrRaw(CSR_SSTATUS);
        uint spie = (status >> 5) & 1;
        uint spp = (status >> 8) & 1;
        uint mprv = spp == PRIV_MACHINE ? ((status >> 17) & 1) : 0;
        uint new_status = (status & ~0x20122) | (mprv << 17) | (spie << 1) | (1 << 5);
        cpu.writeCsrRaw(CSR_SSTATUS, new_status);
        cpu.csr.privilege = spp;
        WR_PC(newpc)
    }
})
imp(srl, FormatR, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] >> cpu.xreg[ins.rs2])
})
imp(srli, FormatR, { // rv32i
    uint shamt = (ins_word >> 20) & 0x1F;
    WR_RD(cpu.xreg[ins.rs1] >> shamt)
})
imp(sub, FormatR, { // rv32i
    WR_RD(AS_SIGNED(cpu.xreg[ins.rs1]) - AS_SIGNED(cpu.xreg[ins.rs2]));
})
imp(sw, FormatS, { // rv32i
    cpu.memSetWord(cpu.xreg[ins.rs1] + ins.imm, cpu.xreg[ins.rs2]);
})
imp(uret, FormatEmpty, {
    // system
    // unnecessary?
})
imp(wfi, FormatEmpty, {
    // system
    // no-op is valid here, so skip
})
imp(xor, FormatR, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] ^ cpu.xreg[ins.rs2])
})
imp(xori, FormatI, {// rv32i
    WR_RD(cpu.xreg[ins.rs1] ^ ins.imm)
})

////////////////////////////////////////////////////////////////
// Emulator Functions
////////////////////////////////////////////////////////////////
Emulator::Emulator(/* args */)
{
}

Emulator::~Emulator()
{
}

void Emulator::initialize()
{
    printf("INFO: Emulator started\n");
    cpu = RV32();
    memory = (uint8_t *)malloc(MEM_SIZE);
}

void Emulator::initializeElf(const char *path)
{
    initialize();
    // Load ELF image
    // loadElf()
    cpu.init(memory, false);
    // cpu.dump();
}

void Emulator::initializeBin(const char *path)
{
    initialize();
    // Load binary image
    // loadBin()
}

void Emulator::emulate()
{
    cpu.tick();

    uint32_t ins_word = 0;
    ins_ret ret;

    if ((cpu.pc & 0x3) == 0) {
        // ins_word = cpu.memGetWord(cpu.pc);
        // ret = ins



    }
}
#include "rv32.h"


RV32::RV32(/* args */)
{
}

RV32::~RV32()
{
}

bool RV32::init(uint8* memory, bool debug_mode=false) 
{
    // reset clock
    clock = 0;
    for (uint i = 0; i < 32; i++) {
        xreg[i] = 0;
    }
    xreg[0xb] = 0x1020; // For Linux?
    pc = 0x80000000;
    mem = memory;
    reservation_en = false;

    initCSRs();

    debug_single_step = debug_mode;

    return true;
}

void RV32::initCSRs()
{
    csr.privilege = PRIV_MACHINE;
    for (uint i = 0; i < 4096; i++)
    {
        csr.data[i] = 0;
    }
    // RV32AIMSU
    csr.data[CSR_MISA] = 0b01000000000101000001000100000001;
}

void RV32::dump()
{
    printf("======================================\n");
    printf("DUMP: CPU state @%d:\n", clock);
    for (int i = 0; i < 32; i += 4)
    {
        printf("DUMP: .x%02d = %08x  .x%02d = %08x  .%02d = %08x  .%02d = %08x\n",
               i, xreg[i],
               i + 1, xreg[i + 1],
               i + 2, xreg[i + 2],
               i + 3, xreg[i + 3]);
    }
    printf("DUMP: .pc  = %08x\n", pc);
    printf("DUMP: next ins: %08x\n", *(uint *)(mem + (pc & 0x7FFFFFFF)));
}

void RV32::tick()
{
    clock++;
    // emulate(cpu);
}

ins_ret RV32::insReturnNoop()
{
    ins_ret ret;
    memset(&ret, 0, sizeof(ins_ret));
    ret.pc_val = pc + 4;
    return ret;
}

///////////////////////////////////////
// CSR Functions
///////////////////////////////////////
bool RV32::hasCsrAccessPrivilege(uint addr)
{
    uint privilege = (addr >> 8) & 0x3;
    return privilege <= csr.privilege;
}

// SSTATUS, SIE, and SIP are subsets of MSTATUS, MIE, and MIP
uint RV32::readCsrRaw(uint address)
{
    switch (address)
    {
    case CSR_SSTATUS:
        return csr.data[CSR_MSTATUS] & 0x000de162;
    case CSR_SIE:
        return csr.data[CSR_MIE] & 0x222;
    case CSR_SIP:
        return csr.data[CSR_MIP] & 0x222;
    case CSR_CYCLE:
        return clock;
    case CSR_MHARTID:
        return 0; // this has to be 0, always
    /* case CSR_TIME => self.mmu.get_clint().read_mtime(), */
    default:
        return csr.data[address & 0xffff];
    }
}

void RV32::writeCsrRaw(uint address, uint value)
{
    switch (address)
    {
    case CSR_SSTATUS:
        csr.data[CSR_MSTATUS] &= !0x000de162;
        csr.data[CSR_MSTATUS] |= value & 0x000de162;
        /* self.mmu.update_mstatus(self.read_csr_raw(CSR_MSTATUS)); */
        break;
    case CSR_SIE:
        csr.data[CSR_MIE] &= !0x222;
        csr.data[CSR_MIE] |= value & 0x222;
        break;
    case CSR_SIP:
        csr.data[CSR_MIP] &= !0x222;
        csr.data[CSR_MIP] |= value & 0x222;
        break;
    case CSR_MIDELEG:
        csr.data[address] = value & 0x666; // from qemu
        break;
    /* case CSR_MSTATUS: */
    /*     csr.data[address] = value; */
    /*     self.mmu.update_mstatus(self.read_csr_raw(CSR_MSTATUS)); */
    /*     break; */
    /* CSR_TIME => { */
    /*     self.mmu.get_mut_clint().write_mtime(value); */
    /* }, */
    default:
        csr.data[address] = value;
        break;
    };
}

uint RV32::getCsr(uint address, ins_ret *ret)
{
    if (hasCsrAccessPrivilege(address))
    {
        uint r = readCsrRaw(address);
#ifdef VERBOSE
        printf("CSR read @%03x = %08x\n", address, r);
#endif
        return r;
    }
    else
    {
        ret->trap.en = true;
        ret->trap.type = trap_IllegalInstruction;
        ret->trap.value = pc;
        return 0;
    }
}

void RV32::setCsr(uint address, uint value, ins_ret *ret)
{
#ifdef VERBOSE
    printf("CSR write @%03x = %08x\n", address, value);
#endif
    if (hasCsrAccessPrivilege(address))
    {
        bool read_only = ((address >> 10) & 0x3) == 0x3;
        if (read_only)
        {
            ret->trap.en = true;
            ret->trap.type = trap_IllegalInstruction;
            ret->trap.value = pc;
        }
        else
        {
            writeCsrRaw(address, value);
            if (address == CSR_SATP)
            {
                // TODO: update MMU addressing mode
            }
        }
    }
    else
    {
        ret->trap.en = true;
        ret->trap.type = trap_IllegalInstruction;
        ret->trap.value = pc;
    }
}

///////////////////////////////////////
// TRAPS
///////////////////////////////////////
// returns true if IRQ was handled or !isInterrupt
bool RV32::handleTrap(ins_ret *ret, bool isInterrupt)
{
    Trap t = ret->trap;
    uint current_privilege = csr.privilege;

    uint mdeleg = readCsrRaw(isInterrupt ? CSR_MIDELEG : CSR_MEDELEG);
    uint sdeleg = readCsrRaw(isInterrupt ? CSR_SIDELEG : CSR_SEDELEG);
    uint pos = t.type & 0xFFFF;

    uint new_privilege = ((mdeleg >> pos) & 1) == 0 ? PRIV_MACHINE : (((sdeleg >> pos) & 1) == 0 ? PRIV_SUPERVISOR : PRIV_USER);

    uint mstatus = readCsrRaw(CSR_MSTATUS);
    uint sstatus = readCsrRaw(CSR_SSTATUS);
    uint current_status = current_privilege == PRIV_MACHINE ? mstatus : (current_privilege == PRIV_SUPERVISOR ? sstatus : readCsrRaw(CSR_USTATUS));

    // check if IRQ should be ignored
    if (isInterrupt)
    {
        uint ie = new_privilege == PRIV_MACHINE ? readCsrRaw(CSR_MIE) : (new_privilege == PRIV_SUPERVISOR ? readCsrRaw(CSR_SIE) : readCsrRaw(CSR_UIE));

        uint current_mie = (current_status >> 3) & 1;
        uint current_sie = (current_status >> 1) & 1;
        uint current_uie = current_status & 1;

        uint msie = (ie >> 3) & 1;
        uint ssie = (ie >> 1) & 1;
        uint usie = ie & 1;

        uint mtie = (ie >> 7) & 1;
        uint stie = (ie >> 5) & 1;
        uint utie = (ie >> 4) & 1;

        uint meie = (ie >> 11) & 1;
        uint seie = (ie >> 9) & 1;
        uint ueie = (ie >> 8) & 1;

        if (new_privilege < current_privilege)
        {
            return false;
        }
        else if (new_privilege == current_privilege)
        {
            if (current_privilege == PRIV_MACHINE && current_mie == 0)
            {
                return false;
            }
            else if (current_privilege == PRIV_SUPERVISOR && current_sie == 0)
            {
                return false;
            }
            else if (current_privilege == PRIV_USER && current_uie == 0)
            {
                return false;
            }
        }

    #define MASK(trap, val)   \
        case trap:            \
        if (val == 0)     \
        {                 \
            return false; \
        }                 \
        else              \
        {                 \
            break;        \
        }

        switch (t.type)
        {
            MASK(trap_UserSoftwareInterrupt, usie)
            MASK(trap_SupervisorSoftwareInterrupt, ssie)
            MASK(trap_MachineSoftwareInterrupt, msie)
            MASK(trap_UserTimerInterrupt, utie)
            MASK(trap_SupervisorTimerInterrupt, stie)
            MASK(trap_MachineTimerInterrupt, mtie)
            MASK(trap_UserExternalInterrupt, ueie)
            MASK(trap_SupervisorExternalInterrupt, seie)
            MASK(trap_MachineExternalInterrupt, meie)
        }
#undef MASK
    }

    // should be handled
    csr.privilege = new_privilege;

    uint csr_epc_addr = new_privilege == PRIV_MACHINE ? CSR_MEPC : (new_privilege == PRIV_SUPERVISOR ? CSR_SEPC : CSR_UEPC);
    uint csr_cause_addr = new_privilege == PRIV_MACHINE ? CSR_MCAUSE : (new_privilege == PRIV_SUPERVISOR ? CSR_SCAUSE : CSR_UCAUSE);
    uint csr_tval_addr = new_privilege == PRIV_MACHINE ? CSR_MTVAL : (new_privilege == PRIV_SUPERVISOR ? CSR_STVAL : CSR_UTVAL);
    uint csr_tvec_addr = new_privilege == PRIV_MACHINE ? CSR_MTVEC : (new_privilege == PRIV_SUPERVISOR ? CSR_STVEC : CSR_UTVEC);

    writeCsrRaw(csr_epc_addr, pc);
    writeCsrRaw(csr_cause_addr, t.type);
    writeCsrRaw(csr_tval_addr, t.value);
    ret->pc_val = readCsrRaw(csr_tvec_addr);

    if ((ret->pc_val & 0x3) != 0)
    {
        // vectored handler
        ret->pc_val = (ret->pc_val & ~0x3) + 4 * pos;
    }

    // NOTE: No user mode interrupt/exception handling!
    if (new_privilege == PRIV_MACHINE)
    {
        uint mie = (mstatus >> 3) & 1;
        uint new_status = (mstatus & !0x1888) | (mie << 7) | (current_privilege << 11);
        writeCsrRaw(CSR_MSTATUS, new_status);
    }
    else
    { // PRIV_SUPERVISOR
        uint sie = (sstatus >> 3) & 1;
        uint new_status = (sstatus & !0x122) | (sie << 5) | ((current_privilege & 1) << 8);
        writeCsrRaw(CSR_SSTATUS, new_status);
    }

#ifdef RV32_VERBOSE
    printf("trap: type=%08x value=%08x (IRQ: %d) moved PC from @%08x to @%08x\n", t.type, t.value, is_interrupt, pc, ret->pc_val);
#endif
    /* debug_single_step = true; */

    return true;
}

///////////////////////////////////////
// Memory Functions
///////////////////////////////////////
// little endian, zero extended
uint RV32::memGetByte(uint addr)
{
    /* printf("TRACE: memGetByte(%d)\n", addr); */
    assert(addr & 0x80000000);
    return mem[addr & 0x7FFFFFFF];
}

uint RV32::memGetHalfWord(uint addr)
{
    return memGetByte(addr) | ((uint16_t)memGetByte(addr + 1) << 8);
}

uint RV32::memGetWord(uint addr)
{
    return memGetByte(addr) |
           ((uint16_t)memGetByte(addr + 1) << 8) |
           ((uint16_t)memGetByte(addr + 2) << 16) |
           ((uint16_t)memGetByte(addr + 3) << 24);
}

void RV32::memSetByte(uint addr, uint val)
{
    assert(addr & 0x80000000);
    mem[addr & 0x7FFFFFFF] = val;
}

void RV32::memSetHalfWord(uint addr, uint val)
{
    memSetByte(addr, val & 0xFF);
    memSetByte(addr + 1, (val >> 8) & 0xFF);
}

void RV32::memSetWord(uint addr, uint val)
{
    memSetByte(addr, val & 0xFF);
    memSetByte(addr + 1, (val >> 8) & 0xFF);
    memSetByte(addr + 2, (val >> 16) & 0xFF);
    memSetByte(addr + 3, val >> 24);
}

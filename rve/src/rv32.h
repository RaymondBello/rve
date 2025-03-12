#ifndef RV32IMA_H
#define RV32IMA_H

#include <cstdint>
#include <stdio.h>
#include <assert.h>

#include "types.h"


using uint   = uint32_t;
using uint16 = uint16_t;
using uint8  = uint8_t;

// CSR
const uint CSR_USTATUS = 0x000;
const uint CSR_UIE = 0x004;
const uint CSR_UTVEC = 0x005;
const uint _CSR_USCRATCH = 0x040;
const uint CSR_UEPC = 0x041;
const uint CSR_UCAUSE = 0x042;
const uint CSR_UTVAL = 0x043;
const uint _CSR_UIP = 0x044;
const uint CSR_SSTATUS = 0x100;
const uint CSR_SEDELEG = 0x102;
const uint CSR_SIDELEG = 0x103;
const uint CSR_SIE = 0x104;
const uint CSR_STVEC = 0x105;
const uint _CSR_SSCRATCH = 0x140;
const uint CSR_SEPC = 0x141;
const uint CSR_SCAUSE = 0x142;
const uint CSR_STVAL = 0x143;
const uint CSR_SIP = 0x144;
const uint CSR_SATP = 0x180;
const uint CSR_MSTATUS = 0x300;
const uint CSR_MISA = 0x301;
const uint CSR_MEDELEG = 0x302;
const uint CSR_MIDELEG = 0x303;
const uint CSR_MIE = 0x304;
const uint CSR_MTVEC = 0x305;
const uint _CSR_MSCRATCH = 0x340;
const uint CSR_MEPC = 0x341;
const uint CSR_MCAUSE = 0x342;
const uint CSR_MTVAL = 0x343;
const uint CSR_MIP = 0x344;
const uint _CSR_PMPCFG0 = 0x3a0;
const uint _CSR_PMPADDR0 = 0x3b0;
const uint _CSR_MCYCLE = 0xb00;
const uint CSR_CYCLE = 0xc00;
const uint CSR_TIME = 0xc01;
const uint _CSR_INSERT = 0xc02;
const uint CSR_MHARTID = 0xf14;




// Traps/Interrupts
#define PRIV_USER 0
#define PRIV_SUPERVISOR 1
#define PRIV_MACHINE 3

static const uint interrupt_offset = 0x80000000;
const uint trap_InstructionAddressMisaligned = 0;
const uint trap_InstructionAccessFault = 1;
const uint trap_IllegalInstruction = 2;
const uint trap_Breakpoint = 3;
const uint trap_LoadAddressMisaligned = 4;
const uint trap_LoadAccessFault = 5;
const uint trap_StoreAddressMisaligned = 6;
const uint trap_StoreAccessFault = 7;
const uint trap_EnvironmentCallFromUMode = 8;
const uint trap_EnvironmentCallFromSMode = 9;
const uint trap_EnvironmentCallFromMMode = 11;
const uint trap_InstructionPageFault = 12;
const uint trap_LoadPageFault = 13;
const uint trap_StorePageFault = 15;
const uint trap_UserSoftwareInterrupt = interrupt_offset + 0;
const uint trap_SupervisorSoftwareInterrupt = interrupt_offset + 1;
const uint trap_MachineSoftwareInterrupt = interrupt_offset + 3;
const uint trap_UserTimerInterrupt = interrupt_offset + 4;
const uint trap_SupervisorTimerInterrupt = interrupt_offset + 5;
const uint trap_MachineTimerInterrupt = interrupt_offset + 7;
const uint trap_UserExternalInterrupt = interrupt_offset + 8;
const uint trap_SupervisorExternalInterrupt = interrupt_offset + 9;
const uint trap_MachineExternalInterrupt = interrupt_offset + 11;


class RV32
{
public:
    uint clock;
    // Registers
    uint xreg[32];
    // Program counter
    uint pc;
    uint8 *mem;
    csr_state csr;

    bool reservation_en;
    uint reservation_addr;

    bool debug_single_step;

    RV32();
    ~RV32();

    bool init(uint8 *memory, bool debug_mode);
    void dump();
    void tick();

    // Noop
    ins_ret insReturnNoop();

    // CSR Functions
    bool hasCsrAccessPrivilege(uint addr);
    uint readCsrRaw(uint address);
    void writeCsrRaw(uint address, uint value);
    uint getCsr(uint address, ins_ret *ret);
    void setCsr(uint address, uint value, ins_ret *ret);
    void initCSRs();

    // Trap Functions
    bool handleTrap(ins_ret *ret, bool isInterrupt);
    
    // Memory Functions
    // Getters
    uint memGetByte(uint addr);
    uint memGetHalfWord(uint addr);
    uint memGetWord(uint addr);
    // Setters
    void memSetByte(uint addr, uint val);
    void memSetHalfWord(uint addr, uint val);
    void memSetWord(uint addr, uint val);
};

#endif
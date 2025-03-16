#ifndef RV32IMA_H
#define RV32IMA_H

#include <cstdint>
#include <stdio.h>
#include <assert.h>

#include "types.h"

using u32   = uint32_t;
using uint16 = uint16_t;
using u8  = uint8_t;

// Constants for Control and Status Registers (CSRs) in RISC-V architecture.
// User-level CSRs
const u32 CSR_USTATUS = 0x000;     // User status register
const u32 CSR_UIE = 0x004;         // User interrupt-enable register
const u32 CSR_UTVEC = 0x005;       // User trap handler base address
const u32 _CSR_USCRATCH = 0x040;   // User scratch register (reserved)
const u32 CSR_UEPC = 0x041;        // User exception program counter
const u32 CSR_UCAUSE = 0x042;      // User trap cause
const u32 CSR_UTVAL = 0x043;       // User bad address or instruction
const u32 _CSR_UIP = 0x044;        // User interrupt pending (reserved)

// Supervisor-level CSRs
const u32 CSR_SSTATUS = 0x100;     // Supervisor status register
const u32 CSR_SEDELEG = 0x102;     // Exception delegation register
const u32 CSR_SIDELEG = 0x103;     // Interrupt delegation register
const u32 CSR_SIE = 0x104;         // Supervisor interrupt-enable register
const u32 CSR_STVEC = 0x105;       // Supervisor trap handler base address
const u32 _CSR_SSCRATCH = 0x140;   // Supervisor scratch register (reserved)
const u32 CSR_SEPC = 0x141;        // Supervisor exception program counter
const u32 CSR_SCAUSE = 0x142;      // Supervisor trap cause
const u32 CSR_STVAL = 0x143;       // Supervisor bad address or instruction
const u32 CSR_SIP = 0x144;         // Supervisor interrupt pending
const u32 CSR_SATP = 0x180;        // Supervisor address translation

// Machine-level CSRs
const u32 CSR_MSTATUS = 0x300;     // Machine status register
const u32 CSR_MISA = 0x301;        // Machine ISA and extensions
const u32 CSR_MEDELEG = 0x302;     // Machine exception delegation register
const u32 CSR_MIDELEG = 0x303;     // Machine interrupt delegation register
const u32 CSR_MIE = 0x304;         // Machine interrupt-enable register
const u32 CSR_MTVEC = 0x305;       // Machine trap handler base address
const u32 _CSR_MSCRATCH = 0x340;   // Machine scratch register (reserved)
const u32 CSR_MEPC = 0x341;        // Machine exception program counter
const u32 CSR_MCAUSE = 0x342;      // Machine trap cause
const u32 CSR_MTVAL = 0x343;       // Machine bad address or instruction
const u32 CSR_MIP = 0x344;         // Machine interrupt pending
const u32 _CSR_PMPCFG0 = 0x3a0;    // Physical memory protection config (reserved)
const u32 _CSR_PMPADDR0 = 0x3b0;   // Physical memory protection address (reserved)
const u32 _CSR_MCYCLE = 0xb00;     // Cycle counter for machines (reserved)
const u32 CSR_CYCLE = 0xc00;       // User mode cycle counter
const u32 CSR_TIME = 0xc01;        // Timer register for user mode
const u32 _CSR_INSERT = 0xc02;     // Insert reserved CSR (reserved)
const u32 CSR_MHARTID = 0xf14;     // Hardware thread ID

// Trap and interrupt constants with privilege levels for RISC-V architecture.
#define PRIV_USER 0                // Privilege level for User mode
#define PRIV_SUPERVISOR 1          // Privilege level for Supervisor mode
#define PRIV_MACHINE 3             // Privilege level for Machine mode

static const u32 interrupt_offset = 0x80000000;  // Offset to identify interrupts from exceptions

// Exception codes
const u32 trap_InstructionAddressMisaligned = 0;    // Address misaligned during instruction fetch
const u32 trap_InstructionAccessFault = 1;          // Instruction access violation
const u32 trap_IllegalInstruction = 2;              // Execution of illegal instruction
const u32 trap_Breakpoint = 3;                      // Breakpoint hit
const u32 trap_LoadAddressMisaligned = 4;           // Misaligned address on load
const u32 trap_LoadAccessFault = 5;                 // Access fault on load
const u32 trap_StoreAddressMisaligned = 6;          // Misaligned address on store
const u32 trap_StoreAccessFault = 7;                // Access fault on store
const u32 trap_EnvironmentCallFromUMode = 8;        // Environment call from U-mode
const u32 trap_EnvironmentCallFromSMode = 9;        // Environment call from S-mode
const u32 trap_EnvironmentCallFromMMode = 11;       // Environment call from M-mode
const u32 trap_InstructionPageFault = 12;           // Page fault during instruction fetch
const u32 trap_LoadPageFault = 13;                  // Page fault during data load
const u32 trap_StorePageFault = 15;                 // Page fault during data store

// Interrupt codes
const u32 trap_UserSoftwareInterrupt = interrupt_offset + 0;   // Software interrupt in U-mode
const u32 trap_SupervisorSoftwareInterrupt = interrupt_offset + 1; // Software interrupt in S-mode
const u32 trap_MachineSoftwareInterrupt = interrupt_offset + 3; // Software interrupt in M-mode
const u32 trap_UserTimerInterrupt = interrupt_offset + 4;      // Timer interrupt in U-mode
const u32 trap_SupervisorTimerInterrupt = interrupt_offset + 5; // Timer interrupt in S-mode
const u32 trap_MachineTimerInterrupt = interrupt_offset + 7;   // Timer interrupt in M-mode
const u32 trap_UserExternalInterrupt = interrupt_offset + 8;   // External device interrupt in U-mode
const u32 trap_SupervisorExternalInterrupt = interrupt_offset + 9; // External device interrupt in S-mode
const u32 trap_MachineExternalInterrupt = interrupt_offset + 11; // External device interrupt in M-mode

// Machine Interrupt Pending (MIP) register bits in RISC-V architecture.
// These constants represent different interrupt pending bits within the MIP register.
const u32 MIP_MEIP = 0x800;  // Machine External Interrupt Pending
const u32 MIP_MTIP = 0x080;  // Machine Timer Interrupt Pending
const u32 MIP_MSIP = 0x008;  // Machine Software Interrupt Pending
const u32 MIP_SEIP = 0x200;  // Supervisor External Interrupt Pending
const u32 MIP_STIP = 0x020;  // Supervisor Timer Interrupt Pending
const u32 MIP_SSIP = 0x002;  // Supervisor Software Interrupt Pending

// Composite value representing all possible interrupt pending bits.
// This is a combination of all defined MIP bits indicating any type of interrupt is pending.
const u32 MIP_ALL = MIP_MEIP | MIP_MTIP | MIP_MSIP | MIP_SEIP | MIP_STIP | MIP_SSIP;


// UART
// Constants defining bit shifts for different UART registers
const u32 SHIFT_RBR = 0;  // Receiver Buffer Register
const u32 SHIFT_THR = 8;  // Transmitter Holding Register
const u32 SHIFT_IER = 16; // Interrupt Enable Register
const u32 SHIFT_IIR = 24; // Interrupt Identification Register
const u32 SHIFT_LCR = 0;  // Line Control Register
const u32 SHIFT_MCR = 8;  // Modem Control Register
const u32 SHIFT_LSR = 16; // Line Status Register
const u32 SHIFT_SCR = 24; // Scratch Register

// Interrupt Enable Register (IER) bits for UART.
// These constants are used to enable specific types of UART interrupts.
const u32 IER_RXINT_BIT = 0x1;        // Receive data available interrupt
const u32 IER_THREINT_BIT = 0x2;      // Transmitter holding register empty interrupt

// Interrupt Identification Register (IIR) flags for UART.
// These constants indicate the type or status of an interrupt.
const u32 IIR_THR_EMPTY = 0x2;        // Indicates transmitter holding register is empty
const u32 IIR_RD_AVAILABLE = 0x4;     // Indicates received data is available
const u32 IIR_NO_INTERRUPT = 0x7;     // Indicates no pending interrupts

// Line Status Register (LSR) flags for UART.
// These status flags provide information about the line status and data availability.
const u32 LSR_DATA_AVAILABLE = 0x1;   // Data available in receiver buffer
const u32 LSR_THR_EMPTY = 0x20;       // Transmitter holding register is empty


// Macros to extract 8-bit data from specific bit positions in UART registers.
//
// UART_GET1 extracts a byte from `rbr_thr_ier_iir` register of the `uart` structure.
// The macro shifts the register value right by `SHIFT_##x` bits and masks it with 0xFF
// to isolate the intended byte.
#define UART_GET1(x) ((uart.rbr_thr_ier_iir >> SHIFT_##x) & 0xFF)

// UART_GET2 extracts a byte from `lcr_mcr_lsr_scr` register of the `uart` structure.
// Similar to UART_GET1, shifts the register and applies a mask to obtain the desired byte.
#define UART_GET2(x) ((uart.lcr_mcr_lsr_scr >> SHIFT_##x) & 0xff)

// Macros to set 8-bit data into specific bit positions in UART registers.
//
// UART_SET1 sets a byte in `rbr_thr_ier_iir` register of the `uart` structure.
// It clears the bits at the position specified by `SHIFT_##x` using a mask,
// then places the new value `val` at that position.
#define UART_SET1(x, val) uart.rbr_thr_ier_iir = (uart.rbr_thr_ier_iir & ~(0xff << SHIFT_##x)) | (val << SHIFT_##x)

// UART_SET2 sets a byte in `lcr_mcr_lsr_scr` register of the `uart` structure.
// Utilizes the same process as UART_SET1 to modify the bits at the specified position.
#define UART_SET2(x, val) uart.lcr_mcr_lsr_scr = (uart.lcr_mcr_lsr_scr & ~(0xff << SHIFT_##x)) | (val << SHIFT_##x)




class RV32
{
public:
    u32 clock;
    // Registers
    u32 xreg[32];
    // Program counter
    u32 pc;
    u8 *mem;
    u8 *dtb;
    csr_state csr;
    clint_state clint;
    uart_state uart;

    bool reservation_en;
    u32 reservation_addr;

    bool debug_single_step;

    RV32();
    ~RV32();

    bool init(u8 *memory, bool debug_mode);
    void dump();
    void tick();

    // Noop
    ins_ret insReturnNoop();

    // CSR Functions
    bool hasCsrAccessPrivilege(u32 addr);
    u32 readCsrRaw(u32 address);
    void writeCsrRaw(u32 address, u32 value);
    u32 getCsr(u32 address, ins_ret *ret);
    void setCsr(u32 address, u32 value, ins_ret *ret);
    void initCSRs();

    // Trap Functions
    bool handleTrap(ins_ret *ret, bool isInterrupt);
    void handleIrqAndTrap(ins_ret *ret);

    // Memory Functions
    // Getters
    u32 memGetByte(u32 addr);
    u32 memGetHalfWord(u32 addr);
    u32 memGetWord(u32 addr);
    // Setters
    void memSetByte(u32 addr, u32 val);
    void memSetHalfWord(u32 addr, u32 val);
    void memSetWord(u32 addr, u32 val);
    // UART Functions
    void uartUpdateIir();
    void uartTick();
};

#endif
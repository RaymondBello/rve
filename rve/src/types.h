#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

using uint = uint32_t;
using uint8 = uint8_t;

// Clocking options
enum CLK_SPEED
{
    CLK_MAX = 0,
    CLK_1HZ = 1,
    CLK_5HZ = 5,
    CLK_10HZ = 10,
    CLK_100HZ = 100,
    CLK_1000HZ = 1000
};

typedef struct
{
    bool en;
    uint type;
    uint value;
} Trap;

typedef struct
{
    uint write_reg;
    uint write_val;
    uint pc_val;
    uint csr_write;
    uint csr_val;
    Trap trap;
} ins_ret;

typedef struct
{
    uint data[4096];
    uint privilege;
} csr_state;


#endif

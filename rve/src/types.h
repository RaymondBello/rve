#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

using uint = uint32_t;
using uint8 = uint8_t;

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

#ifndef BACKEND_COMMON_HEADER
#define BACKEND_COMMON_HEADER

#include <stdint.h>

// commented registers - when there is R or B bit equal to 1
// in REX prefix
typedef enum
{
    kRAX = 0x0, // also r8
    kRCX = 0x1, // also r9
    kRDX = 0x2, // also r10
    kRBX = 0x3, // also r11
    kRSP = 0x4, // also r12
    kRBP = 0x5, // also r13
    kRSI = 0x6, // also r14
    kRDI = 0x7, // also r15
} RegisterCode_t;

typedef enum
{
    kNotOpcode,
    kPushRegister = 0x50,
} Opcode_t;

struct Instruction
{
    size_t   begin_address;

    size_t   instruction_size;

    char     rex_prefix;

    Opcode_t op_code;

    char     mod_rm;

    char     displacement;

    uint32_t immediate_arg;
};

#endif

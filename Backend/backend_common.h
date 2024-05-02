#ifndef BACKEND_COMMON_HEADER
#define BACKEND_COMMON_HEADER

#include <stdint.h>

typedef enum
{
    kRAX = 0x0,
    kRCX = 0x1,
    kRDX = 0x2,
    kRBX = 0x3,
    kRSP = 0x4,
    kRBP = 0x5,
    kRSI = 0x6,
    kRDI = 0x7,
    kR8  = 0x0,
    kR9  = 0x1,
    kR10 = 0x2,
    kR11 = 0x3,
    kR12 = 0x4,
    kR13 = 0x5,
    kR14 = 0x6,
    kR15 = 0x7,

    kNotRegister,
} RegisterCode_t;

typedef enum
{
    kNotOpcode,
    kPushRegister           = 0x50,
    kPushImmediate          = 0x68,

// These two instructions have same opcode,
// So we need to divide them
// Dont be amazed seeing kMovRegisterToRegister
// in SetInstruction() func when is moving
// register to memory.

// That's why i don't initialize kMovRegisterToMemory

    kMovRegisterToRegister  = 0x89,
    kMovRegisterToMemory,

    kRet                    = 0xC3,
    kPopInRegister          = 0x58,
    kMovImmediateToRegister = 0xB8,
    kLeave                  = 0xC9,
    kAddImmediateToRegister = 0x81,
    kAddRegisterToRegister  = 0x01,
} Opcode_t;

typedef enum
{
    kRegisterMemoryMode           = 0x0,
    kRegisterMemory8Displacement  = 0x40,
    kRegisterMemory16Displacement = 0x80,
    kRegister                     = 0xC0,
} ModRmCode_t;

typedef enum
{
    kRexHeader         = 0x40,
    kQwordUsing        = 0x08,
    kRegisterExtension = 0x04,
    kSibExtension      = 0x02,
    kModRmExtension    = 0x01,
} RexPrefixCode_t;

struct Instruction
{
    size_t        begin_address;

    size_t        instruction_size;

    uint8_t       rex_prefix;

    uint8_t       op_code;

    uint8_t       mod_rm;

    uint32_t      displacement;

    int64_t       immediate_arg;
};

static const RegisterCode_t ArgPassingRegisters[] =
{
    kRSI,
    kRDX,
    kRCX,
    kR8,
    kR9
};

static const size_t kArgPassingRegisterCount = sizeof(ArgPassingRegisters) / sizeof(RegisterCode_t);

#endif

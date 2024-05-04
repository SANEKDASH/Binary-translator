#ifndef BACKEND_COMMON_HEADER
#define BACKEND_COMMON_HEADER

#include <stdint.h>

typedef int64_t  ImmediateType_t;

typedef uint32_t DisplacementType_t;

static const ImmediateType_t kJmpPoison = 0x10101010;
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
    kR8  = 0x8,
    kR9  = 0x9,
    kR10 = 0xA,
    kR11 = 0xB,
    kR12 = 0xC,
    kR13 = 0xD,
    kR14 = 0xE,
    kR15 = 0xF,

    kNotRegister,
} RegisterCode_t;

typedef enum
{
    kPushR64          = 0x50,
    kPushImm32        = 0x68,

    kPopR64           = 0x58,

    kMovR64ToRm64     = 0x89,
    kMovImmToR64      = 0xB8,

    kMovImmToRm64     = 0xC7,
    kMovRm64ToR64     = 0x8B,

    kRet              = 0xC3,
    kLeave            = 0xC9,

    kAddImmToRm64     = 0x81,
    kAddR64ToRm64     = 0x01,

    kSubR64FromRm64   = 0x29,
    kSubImmFromRm64   = 0x81,

    kXorRm64WithR64   = 0x31,

    kDivRm64          = 0xF7,
    kImulRm64         = 0xF7,

    kCmpRm64WithImm32 = 0x81,
    kCmpRm64WithR64   = 0x39,

    kJbeRel32         = 0x0f86,

    kCallRel32        = 0xE8,
} Opcode_t;

typedef enum
{
    kNotOpcode,
    kLogicPushRegister,
    kLogicPushImmediate,

    kLogicPopInRegister,

    kLogicMovRegisterToRegister,
    kLogicMovRegisterToMemory,
    kLogicMovImmediateToRegister,
    kLogicMovRmToRegister,

    kLogicRet,
    kLogicLeave,

    kLogicAddImmediateToRegister,
    kLogicAddRegisterToRegister,

    kLogicSubRegisterFromRegister,
    kLogicSubImmediateFromRegister,

    kLogicXorRegisterWithRegister,
    kLogicXorRmWithRegister,

    kLogicDivRegisterOnRax,

    kLogicImulRegisterOnRax,

    kLogicCmpRegisterToImmediate,
    kLogicCmpRegisterToRegister,
    kLogicJumpIfLessOrEqual,
} LogicalOpcode_t;

typedef enum
{
    kRegisterMemoryMode           = 0x0,
    kRegisterMemory8Displacement  = 0x40,
    kRegisterMemory16Displacement = 0x80,
    kRegister                     = 0xC0,
} ModRmCode_t;

typedef enum
{
    kRexHeader          = 0x40,
    kQwordUsing         = 0x08,
    kRegisterExtension  = 0x04,
    kSibExtension       = 0x02,
    kModRmExtension     = 0x01,
    kRexPrefixNoOptions = 0x0,
} RexPrefixCode_t;

struct Instruction
{
    uint8_t           logical_op_code;

    size_t             begin_address;

    size_t             instruction_size;

    size_t             op_code_size;

    uint8_t            rex_prefix;

    uint16_t           op_code;

    uint8_t            mod_rm;

    DisplacementType_t displacement;

    ImmediateType_t    immediate_arg;
};

static const RegisterCode_t ArgPassingRegisters[] =
{
    kRDI,
    kRSI,
    kRDX,
    kRCX,
    kR8,
    kR9
};

static const size_t kArgPassingRegisterCount = sizeof(ArgPassingRegisters) / sizeof(RegisterCode_t);

#endif

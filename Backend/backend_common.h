#ifndef BACKEND_COMMON_HEADER
#define BACKEND_COMMON_HEADER

#include <stdint.h>

typedef int64_t  ImmediateType_t;

typedef int32_t DisplacementType_t;

typedef int32_t RelativeAddrType_t;

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
    kMovImmToR64      = 0xb8,

    kMovImmToRm64     = 0xc7,
    kMovRm64ToR64     = 0x8b,

    kRet              = 0xc3,
    kLeave            = 0xc9,

    kAddR64ToRm64     = 0x01,
    kAddImmToRm64     = 0x81,

    kSubR64FromRm64   = 0x29,
    kSubImm32FromRm64   = 0x81,

    kXorRm64WithR64   = 0x31,

    kDivRm64          = 0xf7,
    kImulRm64         = 0xf7,

    kCmpRm64WithImm32 = 0x81,
    kCmpRm64WithR64   = 0x39,

    kJbeRel32         = 0x860f,
    kJbRel32          = 0x820f,

    kJaeRel32         = 0x830f,
    kJaRel32          = 0x870f,

    kJeRel32          = 0x840f,
    kJneRel32         = 0x850f,

    kJmpRel32         = 0xe9,

    kCallRel32        = 0xe8,

    kAndR64Rm64       = 0x23,
    kOrR64Rm64        = 0x0b,
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
    kLogicCall,
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
    kLogicJumpIfLess,

    kLogicJumpIfAbove,
    kLogicJumpIfAboveOrEqual,

    kLogicJumpIfEqual,
    kLogicJumpIfNotEqual,

    kLogicJmp,
    kLogicRegisterAndRegister,
    kLogicRegisterOrRegister,
} LogicalOpcode_t;

typedef enum
{
    kRegisterMemoryMode           = 0x0,
    kRegisterMemory8Displacement  = 0x40,
    kRegisterMemory32Displacement = 0x80,
    kRegister                     = 0xC0,
} ModRmCode_t;

typedef enum
{
    kRexHeader          = 0x40,
    kQwordUsing         = 0x08,
    kRegisterExtension  = 0x04,
    kSibExtension       = 0x02,
    kModRmExtension     = 0x01,
    kRexPrefixNoOptions = 0x00,
} RexPrefixCode_t;

struct Instruction
{
    int32_t            label_identifier;

    LogicalOpcode_t    logical_op_code;

    size_t             begin_address;

    size_t             instruction_size;
    size_t             immediate_size;
    size_t             displacement_size;
    size_t             op_code_size;

    uint8_t            rex_prefix;

    uint16_t           op_code;

    uint8_t            mod_rm;

    DisplacementType_t displacement;

    ImmediateType_t    immediate_arg;
};

static const RegisterCode_t ArgPassingRegisters[] =
{
    kRSI,
    kRDI,
    kRDX,
    kRCX,
    kR8,
    kR9
};

static const size_t kArgPassingRegisterCount = sizeof(ArgPassingRegisters) / sizeof(RegisterCode_t);

struct Jump
{
    LogicalOpcode_t  logical_op_code;
    const char      *jump_str;
};

static const Jump kJumpsArray[] =
{
    kLogicJumpIfLessOrEqual, "jbe",
    kLogicJumpIfLess,        "je",

    kLogicJumpIfAbove,       "ja",
    kLogicJumpIfAboveOrEqual,"jae",

    kLogicJumpIfEqual,       "je",
    kLogicJumpIfNotEqual,    "jne",

    kLogicJmp,               "jne",
};

static const size_t kJumpCount = sizeof(kJumpsArray) / sizeof(Jump);


#endif

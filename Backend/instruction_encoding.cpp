#include <stdio.h>

#include "backend.h"

#include "backend_common.h"
#include "backend_dump.h"

#include "instruction_encoding.h"


static RegisterCode_t GetRegisterBase(RegisterCode_t reg);

static bool IsNewRegister(RegisterCode_t reg);

//==============================================================================

bool IsImmediateUsing(LogicalOpcode_t logical_op_code)
{
    return logical_op_code == kLogicPushImmediate            ||
           logical_op_code == kLogicMovImmediateToRegister   ||
           logical_op_code == kLogicAddImmediateToRegister   ||
           logical_op_code == kLogicSubImmediateFromRegister ||
           logical_op_code == kLogicCmpRegisterToImmediate   ||
           logical_op_code == kLogicJumpIfLessOrEqual;
}

//==============================================================================

bool IsDisplacementUsing(LogicalOpcode_t logical_op_code)
{
    return logical_op_code == kLogicMovRegisterToMemory ||
           logical_op_code == kLogicMovRmToRegister;
}

//==============================================================================

bool IsJumpInstruction(LogicalOpcode_t logical_op_code)
{
    return logical_op_code == kLogicJumpIfLessOrEqual  ||
           logical_op_code == kLogicJumpIfLess         ||
           logical_op_code == kLogicJumpIfAboveOrEqual ||
           logical_op_code == kLogicJumpIfAbove        ||
           logical_op_code == kLogicJumpIfEqual        ||
           logical_op_code == kLogicJumpIfNotEqual     ||
           logical_op_code == kLogicJmp                ||
           logical_op_code == kLogicCall;
}

//==============================================================================

static bool IsNewRegister(RegisterCode_t reg)
{
    return reg >> 3;
}

//==============================================================================

BackendErrs_t SetInstructionSize(Instruction *instruction)
{
    CHECK(instruction);

    instruction->instruction_size = 0;

    if (instruction->rex_prefix != 0)
    {
        instruction->instruction_size += sizeof(instruction->rex_prefix);
    }

    if (instruction->op_code <= 0xff)
    {
        instruction->op_code_size = 1;
    }
    else
    {
        instruction->op_code_size = 2;
    }

    instruction->instruction_size += instruction->op_code_size;

    if (instruction->mod_rm != 0)
    {
        instruction->instruction_size += sizeof(instruction->mod_rm);
    }

    instruction->instruction_size += instruction->displacement_size;

    instruction->instruction_size += instruction->immediate_size;

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t SetInstruction(Instruction        *instruction,
                             BackendContext     *backend_context,
                             uint16_t            op_code,
                             DisplacementType_t  displacement,
                             ImmediateType_t     immediate_arg,
                             LogicalOpcode_t     logical_op_code,
                             size_t              immediate_size,
                             size_t              displacement_size)
{
    CHECK(instruction);

    instruction->begin_address     = backend_context->cur_address;
    instruction->op_code           = op_code;
    instruction->displacement      = displacement;
    instruction->immediate_arg     = immediate_arg;
    instruction->logical_op_code   = logical_op_code;
    instruction->immediate_size    = immediate_size;
    instruction->displacement_size = displacement_size;

    SetInstructionSize(instruction);

    backend_context->cur_address += instruction->instruction_size;

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t SetRexPrefix(Instruction *instruction,
                           uint8_t      qword_usage,
                           uint8_t      register_extension,
                           uint8_t      sib_extension,
                           uint8_t      mod_rm_extension)
{
    CHECK(instruction);

    instruction->rex_prefix = 0;

    instruction->rex_prefix |= kRexHeader;

    instruction->rex_prefix |= qword_usage;
    instruction->rex_prefix |= register_extension;
    instruction->rex_prefix |= sib_extension;
    instruction->rex_prefix |= mod_rm_extension;

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t SetModRm(Instruction    *instruction,
                       ModRmCode_t     mode,
                       RegisterCode_t  source_register,
                       RegisterCode_t  receiver_register)
{
    CHECK(instruction);

    instruction->mod_rm = 0;

    instruction->mod_rm |= mode;

    if (source_register != kNotRegister)
    {
        instruction->mod_rm |= source_register << 3;
    }

    if (receiver_register != kNotRegister)
    {
        instruction->mod_rm |= receiver_register;
    }

    return kBackendSuccess;
}

//==============================================================================

#define ADD_INSTRUCTION(instr) ListAddAfter(backend_context->instruction_list,       \
                                            backend_context->instruction_list->tail, \
                                            instr);

#define SET_INSTRUCTION(op_code, displacement, imm_arg, logical_op_code, immediate_size, displacement_size) SetInstruction(&instruction,                  \
                                                                                                                            backend_context,              \
                                                                                                                            op_code,                      \
                                                                                                                            displacement,                 \
                                                                                                                            imm_arg,                      \
                                                                                                                            logical_op_code,              \
                                                                                                                            immediate_size,               \
                                                                                                                            displacement_size)            \

#define SET_MOD_RM(mode, source_reg, receiver_reg) SetModRm(&instruction,  \
                                                             mode,         \
                                                             source_reg,   \
                                                             receiver_reg)

#define SET_REX_PREFIX(qword_usage, register_extension, sib_extension, mod_rm_extension) SetRexPrefix(&instruction,        \
                                                                                                       qword_usage,        \
                                                                                                       register_extension, \
                                                                                                       sib_extension,      \
                                                                                                       mod_rm_extension)

//==============================================================================

RegisterCode_t GetRegisterBase(RegisterCode_t reg)
{
    uint32_t reg_code = reg;

    return (RegisterCode_t) (reg_code & (~((~0) << 3)));
}

//==============================================================================

BackendErrs_t EncodeCall(BackendContext  *backend_context,
                         LanguageContext *language_context,
                         int32_t          func_pos)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(kCallRel32, 0, kJmpPoison, kLogicCall, sizeof(RelativeAddrType_t), 0);

    ADD_INSTRUCTION(&instruction);

    if (func_pos != kCallPoison)
    {
        AddFuncLabelRequest(backend_context,
                            backend_context->instruction_list->tail,
                            func_pos);

        BackendDumpPrintCall(language_context, func_pos);
    }

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodePushRegister(BackendContext *backend_context,
                                 RegisterCode_t  reg)
{
    Instruction instruction = {0};

    if (IsNewRegister(reg))
    {
        SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, kModRmExtension);
    }

    SET_INSTRUCTION(kPushR64 + GetRegisterBase(reg), 0, 0, kLogicPushRegister, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeJump(BackendContext  *backend_context,
                         LogicalOpcode_t  logical_opcode,
                         Opcode_t         op_code,
                         int32_t          label_identifier)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(op_code, 0, kJmpPoison, logical_opcode, sizeof(RelativeAddrType_t), 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintJump(&instruction, label_identifier);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeMovRegisterToRegister(BackendContext *backend_context,
                                          RegisterCode_t  src_reg,
                                          RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kMovR64ToRm64, 0, 0, kLogicMovRegisterToRegister, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeRet(BackendContext *backend_context)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(kRet, 0, 0, kLogicRet, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodePopInRegister(BackendContext *backend_context,
                                  RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    if (IsNewRegister(dest_reg))
    {
        SET_REX_PREFIX(kRexPrefixNoOptions,
                       kRexPrefixNoOptions,
                       kRexPrefixNoOptions,
                       kModRmExtension);
    }

    SET_INSTRUCTION(kPopR64 + GetRegisterBase(dest_reg), 0, 0, kLogicPopInRegister, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeMovImmediateToRegister(BackendContext  *backend_context,
                                           ImmediateType_t  immediate,
                                           RegisterCode_t   dest_reg)
{
    Instruction instruction = {0};

    if (IsNewRegister(dest_reg))
    {
        SET_REX_PREFIX(kQwordUsing,
                       kRexPrefixNoOptions,
                       kRexPrefixNoOptions,
                       kModRmExtension);
    }
    else
    {
        SET_REX_PREFIX(kQwordUsing,
                       kRexPrefixNoOptions,
                       kRexPrefixNoOptions,
                       kRexPrefixNoOptions);
    }

    SET_INSTRUCTION(kMovImmToR64, 0, immediate, kLogicMovImmediateToRegister, sizeof(ImmediateType_t), 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeMovRegisterToRegisterMemory(BackendContext *backend_context,
                                                RegisterCode_t  src_reg,
                                                RegisterCode_t  dest_reg,
                                                DisplacementType_t displacement)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegisterMemory32Displacement,
               GetRegisterBase(src_reg),
               GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kMovR64ToRm64, displacement, 0, kLogicMovRegisterToMemory, 0, sizeof(DisplacementType_t));

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeAddImmediateToRegister(BackendContext  *backend_context,
                                           ImmediateType_t  immediate,
                                           RegisterCode_t   dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, kRAX, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kAddImmToRm64, 0, immediate, kLogicAddImmediateToRegister, sizeof(ImmediateType_t), 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeAddRegisterToRegister(BackendContext *backend_context,
                                          RegisterCode_t  src_reg,
                                          RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kAddR64ToRm64, 0, 0, kLogicAddRegisterToRegister, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeMovRegisterMemoryToRegister(BackendContext     *backend_context,
                                                RegisterCode_t      src_reg,
                                                RegisterCode_t      dest_reg,
                                                DisplacementType_t  displacement)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegisterMemory32Displacement, GetRegisterBase(src_reg),
                                              GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kMovRm64ToR64, displacement, 0, kLogicMovRegisterToMemory, 0, sizeof(DisplacementType_t));

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeSubRegisterFromRegister(BackendContext *backend_context,
                                            RegisterCode_t  src_reg,
                                            RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kSubR64FromRm64, 0, 0, kLogicSubRegisterFromRegister, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeSubImmediateFromRegister(BackendContext *backend_context,
                                             RegisterCode_t  dest_reg,
                                             ImmediateType_t immediate)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x5, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kSubImm32FromRm64, 0, immediate, kLogicSubImmediateFromRegister, sizeof(int32_t), 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeRegisterAndRegister(BackendContext *backend_context,
                                        RegisterCode_t  dest_reg,
                                        RegisterCode_t  src_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(src_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, 0, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(dest_reg), GetRegisterBase(src_reg));

    SET_INSTRUCTION(kAndR64Rm64, 0, 0, kLogicRegisterAndRegister, 0, 0);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeRegisterOrRegister(BackendContext *backend_context,
                                       RegisterCode_t  dest_reg,
                                       RegisterCode_t  src_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(src_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, 0, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(dest_reg), GetRegisterBase(src_reg));

    SET_INSTRUCTION(kOrR64Rm64, 0, 0, kLogicRegisterAndRegister, 0, 0);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeXorRegisterWithRegister(BackendContext *backend_context,
                                            RegisterCode_t  dest_reg,
                                            RegisterCode_t  src_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, 0, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kXorRm64WithR64, 0, 0, kLogicXorRegisterWithRegister, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeDivRegister(BackendContext *backend_context,
                                RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension = (RexPrefixCode_t) 0;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, 0, 0, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x7, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kDivRm64, 0, 0, kLogicDivRegisterOnRax, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeImulRegister(BackendContext *backend_context,
                                 RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, 0, 0, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x4, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kImulRm64, 0, 0, kLogicImulRegisterOnRax, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeCmpRegisterWithImmediate(BackendContext *backend_context,
                                             RegisterCode_t  dest_reg,
                                             ImmediateType_t immediate)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, 0, 0, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x7, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kCmpRm64WithImm32, 0, immediate, kLogicCmpRegisterToImmediate, sizeof(int32_t), 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeCmpRegisterWithRegister(BackendContext *backend_context,
                                            RegisterCode_t  dest_reg,
                                            RegisterCode_t  src_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, 0, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kCmpRm64WithR64, 0, 0, kLogicCmpRegisterToRegister, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EncodeLeave(BackendContext *backend_context)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(kLeave, 0, 0, kLogicLeave, 0, 0);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrintInstruction(backend_context, &instruction);

    return kBackendSuccess;
}

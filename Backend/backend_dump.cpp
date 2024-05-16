#include "backend_dump.h"

static FILE *BackendDumpFile = nullptr;

static uint8_t GetDestRegister(Instruction *instruction);
static uint8_t GetSrcRegister (Instruction *instruction);
//==============================================================================

#define DUMP_PRINT(...) fprintf(BackendDumpFile, __VA_ARGS__)

#define SOURCE_REGISTER   kRegisterArray[source_register].name
#define RECEIVER_REGISTER kRegisterArray[receiver_register].name

#define IMMEDIATE instruction->immediate_arg

#define DISPLACEMENT instruction->displacement

//==============================================================================

BackendErrs_t BeginBackendDump()
{
    if (BackendDumpFile != nullptr)
    {
        return kBackendDumpAlreadyStarted;
    }

    BackendDumpFile = fopen(kBackendDumpFileName, "w");

    if (BackendDumpFile == nullptr)
    {
        ColorPrintf(kRed, "%s() failed to open file\n");

        perror("");

        return kBackendFailedToOpenFile;
    }

    DUMP_PRINT("section .text\n\n"
               "extern пишу_твоей_матери\n"
               "extern скажи_мне\n"
               "extern main\n\n");

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t EndBackendDump()
{
    if (BackendDumpFile == nullptr)
    {
        return kBackendDumpAlreadyClosed;
    }

    fclose(BackendDumpFile);

    BackendDumpFile = nullptr;

    return kBackendSuccess;
}


//==============================================================================

static const size_t kDestRegisterMask = 0x7;
static const size_t kSrcRegisterMask  = kDestRegisterMask << 3;

//==============================================================================

static uint8_t GetSrcRegister(Instruction *instruction)
{
    uint8_t src_register_code = (instruction->mod_rm & kSrcRegisterMask) >> 3;

    if (instruction->rex_prefix & kRegisterExtension)
    {
        src_register_code |= 1 << 3;
    }

    return src_register_code;
}

//==============================================================================

static uint8_t GetDestRegister(Instruction *instruction)
{
    uint8_t dest_register_code =  (instruction->mod_rm & kDestRegisterMask);

    if (instruction->rex_prefix & kModRmExtension)
    {
        dest_register_code |= 1 << 3;
    }

    return dest_register_code;
}

//==============================================================================

BackendErrs_t BackendDumpPrintFuncLabel(LanguageContext *language_context,
                                        int32_t           func_pos)
{
    if (func_pos == language_context->tables.main_id_pos)
    {
        DUMP_PRINT("%s:\n", kAsmMainName);
    }
    else
    {
        DUMP_PRINT("%s:\n", language_context->identifiers.identifier_array[func_pos].id);
    }

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendDumpPrintString(const char *str)
{
    DUMP_PRINT("%s", str);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendDumpPrintCall(LanguageContext *language_context,
                                   int32_t          func_pos)
{
    DUMP_PRINT("\tcall %s\n\n", language_context->identifiers.identifier_array[func_pos].id);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t DumpPrintCommonLabel(int32_t identification_number)
{
    DUMP_PRINT("label_%d:\n", identification_number);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendDumpPrintJump(Instruction *instruction,
                                   int32_t      label_identifier)
{
    size_t jump_pos = 0;

    while (instruction->logical_op_code != kJumpsArray[jump_pos].logical_op_code)
    {
        jump_pos++;
    }

    DUMP_PRINT("\t%s label_%d\n\n", kJumpsArray[jump_pos].jump_str,
                                    label_identifier);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendDumpPrintInstruction(BackendContext  *backend_context,
                                          Instruction     *instruction)
{
    uint8_t source_register   = GetSrcRegister(instruction);
    uint8_t receiver_register = GetDestRegister(instruction);

    switch (instruction->logical_op_code)
    {
        case kLogicPushRegister:
        {
            DUMP_PRINT("\tpush %s\n", kRegisterArray[instruction->op_code - kPushR64]);

            break;
        }

        case kLogicPushImmediate:
        {
            DUMP_PRINT("\tpush %d\n", IMMEDIATE);

            break;
        }

        case kLogicMovRegisterToRegister:
        {
            DUMP_PRINT("\tmov %s, %s\n", RECEIVER_REGISTER,
                                         SOURCE_REGISTER);
            break;
        }

        case kLogicMovRegisterToMemory:
        {
            DUMP_PRINT("\tmov [%s + (%d)], %s\n", RECEIVER_REGISTER,
                                                  DISPLACEMENT,
                                                  SOURCE_REGISTER);
            break;
        }

        case kLogicRet:
        {
            DUMP_PRINT("\tret\n");

            break;
        }

        case kLogicPopInRegister:
        {
            DUMP_PRINT("\tpop %s\n", RECEIVER_REGISTER);

            break;
        }

        case kLogicMovImmediateToRegister:
        {
            DUMP_PRINT("\tmov %s, %d\n", RECEIVER_REGISTER,
                                         IMMEDIATE);
            break;
        }

        case kLogicLeave:
        {
            DUMP_PRINT("\tleave\n");

            break;
        }

        case kLogicAddImmediateToRegister:
        {
            DUMP_PRINT("\tadd %s, %d\n", RECEIVER_REGISTER,
                                         IMMEDIATE);
            break;
        }

        case kLogicAddRegisterToRegister:
        {
            DUMP_PRINT("\tadd %s, %s", RECEIVER_REGISTER,
                                       SOURCE_REGISTER);
            break;
        }

        case kLogicMovRmToRegister:
        {
            DUMP_PRINT("\tmov %s, [%s + (%d)]\n", RECEIVER_REGISTER,
                                                  SOURCE_REGISTER,
                                                  IMMEDIATE);
            break;
        }

        case kLogicSubRegisterFromRegister:
        {
            DUMP_PRINT("\tsub %s, %s\n", RECEIVER_REGISTER,
                                         SOURCE_REGISTER);
            break;
        }

        case kLogicDivRegisterOnRax:
        {
            DUMP_PRINT("\tdiv %s\n", RECEIVER_REGISTER);

            break;
        }

        case kLogicXorRegisterWithRegister:
        {
            DUMP_PRINT("\txor %s, %s\n", RECEIVER_REGISTER,
                                         SOURCE_REGISTER);
            break;
        }

        case kLogicSubImmediateFromRegister:
        {
            DUMP_PRINT("\tsub %s, %d\n", RECEIVER_REGISTER,
                                         IMMEDIATE);
            break;
        }

        case kLogicImulRegisterOnRax:
        {
            DUMP_PRINT("\timul %s\n", RECEIVER_REGISTER);

            break;
        }

        case kLogicCmpRegisterToImmediate:
        {
            DUMP_PRINT("\tcmp %s, %d\n", RECEIVER_REGISTER,
                                         IMMEDIATE);
            break;
        }

        case kLogicCmpRegisterToRegister:
        {
            DUMP_PRINT("\tcmp %s, %s\n", RECEIVER_REGISTER,
                                          SOURCE_REGISTER);
            break;
        }

        case kLogicRegisterAndRegister:
        {
            DUMP_PRINT("\tand %s, %s\n", RECEIVER_REGISTER,
                                         SOURCE_REGISTER);
            break;
        }

        case kLogicRegisterOrRegister:
        {
            DUMP_PRINT("\tor %s, %s\n", RECEIVER_REGISTER,
                                        SOURCE_REGISTER);
            break;
        }

        default:
        {
            ColorPrintf(kRed, "%s() - unknown opcode %d\n", __func__, instruction->logical_op_code);

            break;
        }
    }

    DUMP_PRINT("\n");

    return kBackendSuccess;
}


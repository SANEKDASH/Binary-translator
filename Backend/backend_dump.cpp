#include "backend_dump.h"

static FILE *BackendDumpFile = nullptr;

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
    }

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

#define DUMP_PRINT(...) fprintf(BackendDumpFile, __VA_ARGS__)

#define SOURCE_REGISTER   kRegisterArray[source_register].name
#define RECEIVER_REGISTER kRegisterArray[receiver_register].name

#define IMMEDIATE instruction->immediate_arg

#define DISPLACEMENT instruction->displacement

//==============================================================================

BackendErrs_t BackendDumpPrintLabel(size_t label_pos)
{
    DUMP_PRINT("Label%d:\n\n", label_pos);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendDumpPrintFuncLabel(LanguageContext *language_context,
                                        size_t           func_pos)
{
    if (func_pos == language_context->tables.main_id_pos)
    {
        DUMP_PRINT("main:\n");
    }
    else
    {
        DUMP_PRINT("%s:\n", language_context->identifiers.identifier_array[func_pos].id);

    }

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendPrintCall(LanguageContext *language_context,
                               size_t           func_pos)
{
    DUMP_PRINT("\tcall %s\n\n", language_context->identifiers.identifier_array[func_pos].id);

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendPrintJump(Opcode_t jump_code,
                               size_t   label_pos)
{
    switch (jump_code)
    {
        case kJbeRel32:
        {
            DUMP_PRINT("\tjbe Label%d\n", label_pos);

            break;
        }

        default:
        {
            ColorPrintf(kRed, "%s() unknown jump code %d\n", jump_code);

            return kBackendUnknownOpcode;

            break;
        }
    }

    DUMP_PRINT("\n");

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendDumpPrint(Instruction     *instruction,
                               LogicalOpcode_t  opcode,
                               RegisterCode_t   source_register,
                               RegisterCode_t   receiver_register)
{
    size_t source_reg   = source_register;
    size_t receiver_reg = receiver_register;

    if (instruction->rex_prefix & kRegisterExtension)
    {
        source_reg |= 1 << 3;
    }

    if (instruction->rex_prefix & kModRmExtension)
    {
        receiver_reg |= 1 << 3;
    }

    switch (opcode)
    {
        case kLogicPushRegister:
        {
            DUMP_PRINT("\tpush %s\n", SOURCE_REGISTER);

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
            DUMP_PRINT("\tpop %s\n", receiver_register);

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
            DUMP_PRINT("\t cmp %s, %s\n", RECEIVER_REGISTER,
                                          SOURCE_REGISTER);

            break;
        }

        default:
        {
            ColorPrintf(kRed, "%s() - unknown opcode %d\n", __func__, opcode);

            break;
        }
    }

    DUMP_PRINT("\n");

    return kBackendSuccess;
}


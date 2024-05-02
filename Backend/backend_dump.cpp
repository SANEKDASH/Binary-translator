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

BackendErrs_t BackendDumpPrint(Instruction    *instruction,
                               Opcode_t        opcode,
                               RegisterCode_t  source_register,
                               RegisterCode_t  receiver_register)
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
        case kPushRegister:
        {
            DUMP_PRINT("\tpush %s\n", SOURCE_REGISTER);

            break;
        }

        case kPushImmediate:
        {
            DUMP_PRINT("\tpush %d\n", IMMEDIATE);

            break;
        }

        case kMovRegisterToRegister:
        {
            DUMP_PRINT("\tmov %s, %s\n", RECEIVER_REGISTER,
                                         SOURCE_REGISTER);
            break;
        }

        case kMovRegisterToMemory:
        {
            DUMP_PRINT("\tmov [%s + (%d)], %s\n", RECEIVER_REGISTER,
                                                  DISPLACEMENT,
                                                  SOURCE_REGISTER);
            break;
        }

        case kRet:
        {
            DUMP_PRINT("\tret\n");

            break;
        }

        case kPopInRegister:
        {
            DUMP_PRINT("\tpop %s\n", receiver_register);

            break;
        }

        case kMovImmediateToRegister:
        {
            DUMP_PRINT("\tmov %s, %d\n", RECEIVER_REGISTER,
                                         IMMEDIATE);
            break;
        }

        case kLeave:
        {
            DUMP_PRINT("\tleave\n");

            break;
        }

        case kAddImmediateToRegister:
        {
            DUMP_PRINT("\tadd %s, %d", RECEIVER_REGISTER,
                                       IMMEDIATE);
            break;
        }

        case kNotOpcode:
        default:
        {
            ColorPrintf(kRed, "%s() - unknown opcode\n", __func__);

            break;
        }
    }

    DUMP_PRINT("\n");

    return kBackendSuccess;
}


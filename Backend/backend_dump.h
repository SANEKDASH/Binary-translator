#ifndef BACKEND_DUMP_HEADER
#define BACKEND_DUMP_HEADER

#include "backend.h"
#include "backend_common.h"

static const char *kBackendDumpFileName = "backend_dump.asm";


BackendErrs_t BackendDumpPrint(Instruction     *instruction,
                               LogicalOpcode_t  opcode,
                               RegisterCode_t   source_register,
                               RegisterCode_t   receiver_register);

BackendErrs_t BackendPrintJump(Opcode_t jump_code,
                               size_t   label_pos);

BackendErrs_t BackendDumpPrintLabel(size_t label_pos);


BackendErrs_t BackendDumpPrintFuncLabel(LanguageContext *language_context,
                                        size_t           func_pos);
BackendErrs_t BeginBackendDump();

BackendErrs_t EndBackendDump();


BackendErrs_t BackendPrintCall(LanguageContext *language_context,
                               size_t           func_pos);

struct Register
{
    const char *name;
    size_t code;
};

static Register kRegisterArray[] =
{
    "rax", 0x0,
    "rcx", 0x1,
    "rdx", 0x2,
    "rbx", 0x3,
    "rsp", 0x4,
    "rbp", 0x5,
    "rsi", 0x6,
    "rdi", 0x7,
    "r8",  0x8,
    "r9",  0x9,
    "r10", 0xa,
    "r11", 0xb,
    "r12", 0xc,
    "r13", 0xd,
    "r15", 0xf,
};

static size_t kRegisterArraySize = sizeof(kRegisterArray) / sizeof(Register);

#endif

#ifndef BACKEND_HEADER
#define BACKEND_HEADER

#include <elf.h>

#include "../Common/trees.h"
#include "../Common/NameTable.h"
#include "backend_common.h"

#include "FastList/list.h"
#include "ListDump/list_dump.h"

static const char *kAsmMainName = "main";

static const int32_t kStackAlignSize = 16;

static const int32_t kSizeOfArg = 8;

typedef enum
{
    kBackendSuccess,
    kMissingKeywordCode,
    kCantFindSuchVariable,
    kBackendUnknownNodeType,
    kBackendNullTree,
    kListConstructorError,
    kListDestructorError,
    kBackendUnknownOpcode,
    kBackendNotAssign,
    kBackendNotVarDecl,
    kCantFindNameTable,
    kBackendDumpAlreadyStarted,
    kBackendDumpAlreadyClosed,
    kBackendFailedAllocation,
    kCantFindVariable,
    kBackendLabelTableInitError,
    kBackendLabelTableDestroyError,
    kBackendAddressRequestsInitError,
    kBackendDestroyAddressRequestsError,
    kBackendNullArgs,
    kBackendFailedToOpenFile,
    kFailedToFindSymbolIndex,
    kBackendInconsistentSizes,
    kBackendUnknownOpcodeSize,
} BackendErrs_t;

static const size_t kBaseRelocationTableCapacity = 16;

struct RelocationTable
{
    Elf64_Rela *relocation_array;

    size_t      relocation_count;

    size_t      capacity;
};

static const size_t kBaseSymbolTableCapacity = 16;

struct SymbolTable
{
    Elf64_Sym *sym_array;

    size_t     sym_count;

    size_t     capacity;
};

static const size_t kBaseStringTableCapacity = 16;

struct StringTable
{
    char    **string_array;

    size_t    capacity;

    size_t    string_count;

    size_t    cur_size;
};

static const int32_t kFuncLabelPosPoison          = -1;
static const int32_t kCommonLabelIdentifierPoison = -1;

struct Label
{
    uint32_t address;

    int32_t  identification_number;

    int32_t  func_pos;
};

static size_t kBaseLabelTableSize = 32;

struct LabelTable
{
    Label  *label_array;

    size_t  capacity;

    size_t  label_count;

    uint32_t identify_counter;
};

struct Request
{
    size_t jmp_instruction_list_pos;

    int32_t func_pos;

    int32_t label_identifier;
};

static const size_t kBaseCallRequestArraySize = 8;

struct AddressRequests
{
    Request *requests;

    size_t capacity;

    size_t request_count;
};

struct BackendContext
{
    RelocationTable *relocation_table;

    SymbolTable     *symbol_table;

    StringTable     *strings;

    List            *instruction_list;

    size_t           cur_address;

    LabelTable      *label_table;

    AddressRequests *address_requests;
};

TreeErrs_t WriteAsmCodeInFile(LanguageContext *language_context,
                              const char      *output_file_name);

BackendErrs_t GetAsmInstructionsOutLanguageContext(BackendContext  *backend_context,
                                                   LanguageContext *language_context);

BackendErrs_t BackendContextInit   (BackendContext *backend_context);
BackendErrs_t BackendContextDestroy(BackendContext *backend_context);

BackendErrs_t AddFuncLabelRequest(BackendContext *backend_context,
                                  size_t          jump_instruction_list_pos,
                                  int32_t         func_pos);

BackendErrs_t AddCommonLabelRequest(BackendContext *backend_context,
                                    size_t          jump_instruction_list_pos,
                                    int32_t         identification_number);

#endif

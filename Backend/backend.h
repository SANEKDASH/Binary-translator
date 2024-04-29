#ifndef BACKEND_HEADER
#define BACKEND_HEADER

#include "../Common/trees.h"
#include "../Common/NameTable.h"
#include "backend_common.h"

#include "FastList/list.h"
#include "ListDump/list_dump.h"

static const char *kAsmMainName = "main";

typedef enum
{
    kBackendSuccess,
    kMissingKeywordCode,
    kCantFindSuchVariable,
    kBackendUnknownNodeType,
    kBackendNullTree,
    kListConstructorError,
    kListDestructorError,
} BackendErrs_t;

struct BackendContext
{
    List *instruction_list;
};

TreeErrs_t WriteAsmCodeInFile(LanguageContext *language_context,
                              const char      *output_file_name);

BackendErrs_t GetAsmInstructionsOutLanguageContext(List            *instruction_list,
                                                  LanguageContext *language_context);

BackendErrs_t BackendContextInit   (BackendContext *backend_context);
BackendErrs_t BackendContextDestroy(BackendContext *backend_context);

#endif

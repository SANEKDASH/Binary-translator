#include <stdio.h>

#include "backend.h"
#include "../Common/tree_dump.h"
#include "../Common/trees.h"
#include "elf_ctor.h"

int main(int argc, char *argv[])
{
    InitTreeGraphDump();
    BeginListGraphDump();

    LanguageContext      language_context = {0};
    LanguageContextInit(&language_context);

    ReadLanguageContextOutOfFile(&language_context, argv[1], argv[2]);

    BackendContext      backend_context = {0};
    BackendContextInit(&backend_context);

    GetAsmInstructionsOutLanguageContext(&backend_context,
                                         &language_context);

    CreateElfRelocatableFile(&backend_context,
                             &language_context,
                              argv[3]);

    LanguageContextDtor(&language_context);
    BackendContextDestroy(&backend_context);

    EndListGraphDump();
    EndTreeGraphDump();

    return 0;
}

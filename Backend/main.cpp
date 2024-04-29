#include <stdio.h>

#include "backend.h"
#include "../Common/tree_dump.h"
#include "../Common/trees.h"

int main(int argc, char *argv[])
{
    InitTreeGraphDump();
    BeginListGraphDump();

    LanguageContext language_context = {0};
    LanguageContextInit(&language_context);
    ReadLanguageContextOutOfFile(&language_context, argv[1], argv[2]);

    BackendContext backend_context = {0};
    BackendContextInit(&backend_context);

    GetAsmInstructionsOutLanguageContext(&instruction_list, &language_context);

    LanguageContextDtor(&language_context);
    BackendContextDestroy(&backend_context);

    EndListGraphDump();
    EndTreeGraphDump();

    return 0;
}

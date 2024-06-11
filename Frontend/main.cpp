#include <stdio.h>

#include "parse.h"
#include "../Common/trees.h"
#include "../Common/tree_dump.h"

int main(int argc, char *argv[])
{
    InitTreeGraphDump();

    LanguageContext      language_context = {0};
    LanguageContextInit(&language_context);

    if (argc < 2)
    {
        printf(">> FRONTEND: you must put an arg \"Frontend <file_name>\"\n");

        return 0;
    }

    language_context.syntax_tree.root = GetSyntaxTree(&language_context.identifiers, argv[1]);

    GRAPH_DUMP_TREE(&language_context.syntax_tree);

    EndTreeGraphDump();

    if (PrintTreeInFile(&language_context, "tree_save.txt") != kTreeSuccess || language_context.syntax_tree.root == nullptr)
    {
        printf(">> Иди нахуй, чел... У нас так не базарят.\n");

        LanguageContextDtor(&language_context);

        return -1;
    }

    LanguageContextDtor(&language_context);

    printf(">> Так уж и быть, скомпилю тебе это дерьмо: \"%s\".\n", argv[1]);

    return 0;
}

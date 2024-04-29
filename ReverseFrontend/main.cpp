#include <stdio.h>

#include "reverse_frontend.h"

int main(int argc, char *argv[])
{
    InitTreeGraphDump();

    LanguageContext language_context = {0};

    if (argc < 3)
    {
        printf(">>Никак же вы блять не научитесь\n"
               "  ReverseFrontend call looks like: ReverseFrontend <name_of_tree_file> <name_of_id_table_file>\n");

        return -1;
    }


    ReadLanguageContextOutOfFile(&language_context, argv[1], argv[2]);


    if (ReverseFrontend(&language_context, argv[3]) != 0)
    {
        printf("You are dumb\n");

        return -1;
    }

    EndTreeGraphDump();

    return 0;
}

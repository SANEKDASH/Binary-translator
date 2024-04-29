#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../debug/color_print.h"
#include "../debug/debug.h"
#include "../TextParse/text_parse.h"
#include "trees.h"
#include "tree_dump.h"
#include "NameTable.h"

static const char *kTreeSaveFileName = "tree_save.txt";

static const int kPoisonVal = 0xBADBABA;

static FILE *dump_file = nullptr;

static const char *dmp_name = "suka_dump.txt";

static TreeNode* CreateNodeFromText(Identifiers     *identifiers,
                                    Text          *text,
                                    size_t        *iterator);

static TreeNode *CreateNodeFromBrackets(Identifiers     *identifiers,
                                        Text          *text,
                                        size_t        *iterator);

static TreeErrs_t PrintTree(const TreeNode *root,
                            Identifiers *identifiers,
                            FILE           *output_file);

static TreeErrs_t ReallocVarArray(Identifiers *identifiers,
                                  size_t          new_size);

static int ReadNameTablesOutOfFile(LanguageContext *language_context,
                                   const char    *tables_file_name);

static int ReallocNameTables(NameTables *tables, size_t new_size);

static int ReallocTableOfNames(TableOfNames *table, size_t new_size);

static const size_t kBaseVarCount = 16;

static const size_t kBaseTablesCount = 4;

static const size_t kBaseNamesCount  = 8;

//==============================================================================

int InitNamesLog()
{
    dump_file = fopen(dmp_name, "w");

    return 0;
}

//==============================================================================

int CloseNamesLog()
{
    fclose(dump_file);

    dump_file = nullptr;

    return 0;
}

//==============================================================================

int DumpNameTables(NameTables *tables,
                   const char *func,
                   int         line)
{
    #define DMP_PRINT(...) fprintf(dump_file, __VA_ARGS__)

    DMP_PRINT("FUNC: %s, LINE: %d\n", func, line);

    DMP_PRINT("TABLE_COUNT : %lu, TABLES_CAP %lu\n", tables->tables_count,
                                                     tables->capacity);

    for (size_t i = 0; i < tables->tables_count; i++)
    {
        DMP_PRINT("\tTABLE[%lu, POINTER %p] name_count - %lu, capacity %lu\n", i, tables->name_tables[i],
                                                                                  tables->name_tables[i]->name_count,
                                                                                  tables->name_tables[i]->capacity);

        DMP_PRINT("\t{\n");

        for (size_t j = 0; j < tables->name_tables[i]->capacity; j++)
        {
            DMP_PRINT("\t\t%lu %d\n", tables->name_tables[i]->names[j].pos,
                                      tables->name_tables[i]->names[j].type);
        }

        DMP_PRINT("\t}\n\n");
    }

    DMP_PRINT("+--------------------------------------------------------+\n");

    return 0;
}

//==============================================================================

int NameTablesInit(NameTables *tables)
{
    tables->capacity     = kBaseTablesCount;
    tables->tables_count = 0;
    tables->name_tables  = (TableOfNames **) calloc(tables->capacity, sizeof(TableOfNames));

    return 0;
}

//==============================================================================

static int ReallocNameTables(NameTables *tables, size_t new_size)
{
    size_t old_size = tables->tables_count;

    tables->capacity = new_size;

    tables->name_tables = (TableOfNames **) realloc(tables->name_tables, tables->capacity * sizeof(TableOfNames));

    for (size_t i = old_size; i < tables->capacity; i++)
    {
        tables->name_tables[i] = nullptr;
    }

    return 0;
}

//==============================================================================

TableOfNames *AddTableOfNames(NameTables *tables,
                              int         func_code)
{
    if (tables->tables_count >= tables->capacity)
    {
        ReallocNameTables(tables, tables->capacity * 2);
    }

    tables->name_tables[tables->tables_count] = (TableOfNames *) calloc(1, sizeof(TableOfNames));

    tables->name_tables[tables->tables_count]->pos = tables->tables_count;

    TablesOfNamesInit(tables->name_tables[tables->tables_count], func_code);

    tables->tables_count++;

    return tables->name_tables[tables->tables_count - 1];
}

//==============================================================================

static int ReallocTableOfNames(TableOfNames *table, size_t new_size)
{
    size_t old_size = table->name_count;

    table->capacity = new_size;

    table->names = (Name *) realloc(table->names, table->capacity * sizeof(Name));

    if (table->names == nullptr)
    {
        perror("ReallocTableOfNames() failed to realloc\n");

        return -1;
    }

    for (size_t i = old_size; i < table->capacity; i++)
    {
        table->names[i].pos  = 228;
        table->names[i].type = kUndefined;
    }

    return 0;
}

//==============================================================================

int TablesOfNamesInit(TableOfNames *table,
                      int           func_code)
{
    table->name_count = 0;
    table->func_code  = func_code;
    table->capacity   = kBaseNamesCount;

    table->names = (Name *) calloc(table->capacity, sizeof(Name));

    for (size_t i = 0; i < table->capacity; i++)
    {
        table->names[i].pos  = 228;
        table->names[i].type = kUndefined;
    }

    return 0;
}

//==============================================================================

int AddName(TableOfNames *table,
            size_t        id,
            IdType_t      type)
{
    if (table->name_count >= table->capacity)
    {
        ReallocTableOfNames(table, table->capacity * 2);
    }

    table->names[table->name_count].pos  = id;
    table->names[table->name_count].type = type;

    table->name_count++;

    return 0;
}

//==============================================================================

int VarArrayInit(Identifiers *identifiers)
{
    identifiers->identifier_array = (Identifier *) calloc(kBaseVarCount, sizeof(Identifier));

    if (identifiers->identifier_array == nullptr)
    {
        return -1;
    }

    identifiers->size = kBaseVarCount;

    identifiers->identifier_count = 0;

    return 0;
}

//==============================================================================

int SeekIdentifier(Identifiers *identifiers, const char *var_name)
{
    for (size_t i = 0; i < identifiers->identifier_count; i++)
    {
        if (strcmp(var_name, identifiers->identifier_array[i].id) == 0)
        {
            return i;
        }
    }

    return -1;
}

//==============================================================================

int AddIdentifier(Identifiers *identifiers, char *var_name)
{
    if (identifiers->identifier_count >= identifiers->size)
    {
        ReallocVarArray(identifiers, identifiers->size * 2);
    }

    identifiers->identifier_array[identifiers->identifier_count].id                = var_name;
    identifiers->identifier_array[identifiers->identifier_count].declaration_state = false;
    identifiers->identifier_array[identifiers->identifier_count].id_type           = kUndefined;
    ++identifiers->identifier_count;

    return identifiers->identifier_count - 1;
}

//==============================================================================

int VarArrayDtor(Identifiers *identifiers)
{
    free(identifiers->identifier_array);

    identifiers->identifier_array = nullptr;

    identifiers->size = identifiers->identifier_count = 0;

    return 0;
}

//==============================================================================

static TreeErrs_t ReallocVarArray(Identifiers *identifiers,
                                  size_t     new_size)
{
    identifiers->size = new_size;
    identifiers->identifier_array = (Identifier *) realloc(identifiers->identifier_array, identifiers->size * sizeof(Identifier));

    if (identifiers->identifier_array == nullptr)
    {
        perror("ReallocVarArray() failed to realloc Identifier array");

        return kFailedRealloc;
    }

    return kTreeSuccess;
}

//==============================================================================

TreeErrs_t TreeCtor(Tree *tree)
{
    CHECK(tree);

    tree->root = nullptr;

    return kTreeSuccess;
}

//==============================================================================

TreeErrs_t TreeDtor(TreeNode *root)
{
    if (root != nullptr)
    {
        if (root->left != nullptr)
        {
            TreeDtor(root->left);
        }

        if (root->right != nullptr)
        {
            TreeDtor(root->right);
        }
    }

    free(root);

    return kTreeSuccess;
}

//==============================================================================

TreeNode *NodeCtor(TreeNode         *parent_node,
                   TreeNode         *left,
                   TreeNode         *right,
                   ExpressionType_t  type,
                   double            data)
{
    TreeNode *node = (TreeNode *) calloc(1, sizeof(TreeNode));

    if (node == nullptr)
    {
        return nullptr;
    }

    node->type = type;

    switch (type)
    {
        case kOperator:
        {
            node->data.key_word_code = (KeyCode_t) data;

            break;
        }

        case kIdentifier:
        {
            node->data.variable_pos = data;

            break;
        }

        case kConstNumber:
        {
            node->data.const_val = data;

            break;
        }

        case kVarDecl:
        {
            node->data.variable_pos = data;

            break;
        }

        case kFuncDef:
        {
            node->data.variable_pos = data;

            break;
        }

        case kParamsNode:
        {
            node->data.key_word_code = (KeyCode_t) data;

            break;
        }

        case kCall:
        {
            //do nothing

            break;
        }

        default:
        {
            printf("NodeCtor() unknown type %d\n", type);

            free(node);

            return nullptr;
        }

    }

    node->left  = left;
    node->right = right;
    node->parent = parent_node;

    return node;
}

//==============================================================================

static TreeErrs_t PrintTree(const TreeNode *root,
                            Identifiers      *identifiers,
                            FILE           *output_file)
{
    CHECK(output_file);

    fprintf(output_file, "( ");

    if (root == nullptr)
    {
        fprintf(output_file,"_ ");

        return kTreeSuccess;
    }

    switch (root->type)
    {
        case kOperator:
        {
            fprintf(output_file,
                    "%d %d ",
                    kOperator,
                    root->data.key_word_code);

            break;
        }
        case kConstNumber:
        {
            fprintf(output_file,
                    "%d %lg ",
                    kConstNumber,
                    root->data.const_val);

            break;
        }
        case kIdentifier:
        {
            fprintf(output_file,
                    "%d %lu ",
                    kIdentifier,
                    root->data.variable_pos);

            break;
        }
        case kFuncDef:
        {
            fprintf(output_file,
                    "%d %lu ",
                    kFuncDef,
                    root->data.variable_pos);

            break;
        }
        case kParamsNode:
        {
            fprintf(output_file,
                    "%d ",
                    kParamsNode);

            break;
        }
        case kVarDecl:
        {
            fprintf(output_file,
                    "%d %lu ",
                    kVarDecl,
                    root->data.variable_pos);

            break;
        }
        case kCall:
        {
            fprintf(output_file,
                    "%d ",
                    kCall);

            break;
        }

        default:
        {
            printf(">>PrintTreeInFile() unknown node type\n");

            break;
        }
    }

    if (root->left != nullptr)
    {
        PrintTree(root->left, identifiers, output_file);
    }
    else
    {
        fprintf(output_file, "_ ");
    }

    if (root->right != nullptr)
    {
        PrintTree(root->right, identifiers, output_file);
    }
    else
    {
        fprintf(output_file, "_ ");
    }

    fprintf(output_file, ") ");

    return kTreeSuccess;
}

//==============================================================================

TreeErrs_t PrintTreeInFile(LanguageContext *language_context,
                           const char    *file_name)
{
    FILE *output_file = fopen(file_name, "wb");

    if (output_file == nullptr)
    {
        return kFailedToOpenFile;
    }

    int main_id = kPoisonVal;

    for (size_t i = 0; i < language_context->identifiers.identifier_count; i++)
    {
        if (strcmp(language_context->identifiers.identifier_array[i].id, kMainFuncName) == 0)
        {
            main_id = i;

            break;
        }
    }

    if (main_id == kPoisonVal)
    {
        printf("Долбоеб, уже 20-я минута. Где твой Аганим?\n");

        fclose(output_file);

        return kMissingMain;
    }

    fprintf(output_file, "%d ", main_id);

    PrintTree(language_context->syntax_tree.root, &language_context->identifiers, output_file);

    fclose(output_file);

    return kTreeSuccess;
}

//==============================================================================

TreeErrs_t ReadLanguageContextOutOfFile(LanguageContext *language_context,
                                        const char      *tree_file_name,
                                        const char      *tables_file_name)
{
    CHECK(language_context);
    CHECK(tree_file_name);
    CHECK(tables_file_name);

    Text tree_text = {0};


    if (ReadWordsFromFile(&tree_text, tree_file_name) != kSuccess)
    {
        printf("\nReadTreeOutOfFile() failed to read text from file\n");

        return kFailedToReadText;

        TextDtor(&tree_text);
    }

    size_t iterator = 0;

    language_context->tables.main_id_pos = atoi(tree_text.lines_ptr[iterator++].str);

    language_context->syntax_tree.root = CreateNodeFromText(&language_context->identifiers, &tree_text, &iterator);

    if (language_context->syntax_tree.root == nullptr)
    {
        printf("ReatTreeOutOfFile() failed to read tree");

        return kFailedToReadTree;
    }

    ReadNameTablesOutOfFile(language_context, tables_file_name);

    free(language_context->identifiers.identifier_array[language_context->tables.main_id_pos].id);

    language_context->identifiers.identifier_array[language_context->tables.main_id_pos].id = strdup(kMainFuncName);

    GRAPH_DUMP_TREE(&language_context->syntax_tree);

    return kTreeSuccess;
}

//==============================================================================

#define CUR_TOKEN table_tokens.lines_ptr[i].str

#define GO_TO_NEXT_TOKEN i++

//==============================================================================

static int ReadNameTablesOutOfFile(LanguageContext *language_context,
                                   const char    *tables_file_name)
{
    CHECK(language_context);
    CHECK(tables_file_name);

    Text table_tokens;

    ReadWordsFromFile(&table_tokens, tables_file_name);

    size_t i = 0;

    language_context->identifiers.identifier_count = atoi(CUR_TOKEN);

    language_context->identifiers.identifier_array = (Identifier *) calloc(language_context->identifiers.identifier_count, sizeof(Identifier));

    GO_TO_NEXT_TOKEN;

    for (size_t j = 0; j < language_context->identifiers.identifier_count; j++)
    {
        language_context->identifiers.identifier_array[j].id = strdup(CUR_TOKEN);

        GO_TO_NEXT_TOKEN;
    }

    language_context->tables.tables_count = atoi(CUR_TOKEN);
    language_context->tables.name_tables  = (TableOfNames **) calloc(language_context->tables.tables_count, sizeof(TableOfNames *));

    GO_TO_NEXT_TOKEN;

    for (size_t j = 0; j < language_context->tables.tables_count; j++)
    {
        language_context->tables.name_tables[j] = (TableOfNames *) calloc(1, sizeof(TableOfNames));

        language_context->tables.name_tables[j]->name_count = atoi(CUR_TOKEN);

        language_context->tables.name_tables[j]->capacity   = atoi(CUR_TOKEN);

        language_context->tables.name_tables[j]->names = (Name *) calloc(language_context->tables.name_tables[j]->capacity, sizeof(Name));

        GO_TO_NEXT_TOKEN;

        language_context->tables.name_tables[j]->func_code = atoi(CUR_TOKEN);

        GO_TO_NEXT_TOKEN;

        for (size_t z = 0; z < language_context->tables.name_tables[j]->capacity; z++)
        {

            language_context->tables.name_tables[j]->names[z].pos = atoi(CUR_TOKEN);

            GO_TO_NEXT_TOKEN;

            language_context->tables.name_tables[j]->names[z].type = (IdType_t) atoi(CUR_TOKEN);

            GO_TO_NEXT_TOKEN;
        }
    }

    return 0;
}

//==============================================================================

#undef CUR_TOKEN

#undef GO_TO_NEXT_TOKEN

//==============================================================================

#define CUR_TOKEN text->lines_ptr[*iterator].str

#define GO_TO_NEXT_TOKEN ++(*iterator)

//==============================================================================

static TreeNode* CreateNodeFromText(Identifiers    *identifiers,
                                    Text           *text,
                                    size_t         *iterator)
{
    CHECK(iterator);
    CHECK(text);

    TreeNode *node = nullptr;

    if (*CUR_TOKEN == '(')
    {
        GO_TO_NEXT_TOKEN;

        ExpressionType_t type = (ExpressionType_t) atoi(text->lines_ptr[*iterator].str);

        switch (type)
        {
            case kConstNumber:
            {
                GO_TO_NEXT_TOKEN;

                node = NodeCtor(nullptr,
                                nullptr,
                                nullptr,
                                kConstNumber,
                                strtod(CUR_TOKEN, nullptr));
                break;
            }

            case kOperator:
            {
                GO_TO_NEXT_TOKEN;

                node = NodeCtor(nullptr,
                                nullptr,
                                nullptr,
                                kOperator,
                                atoi(CUR_TOKEN));
                break;
            }

            case kIdentifier:
            {
                GO_TO_NEXT_TOKEN;

                int pos = atoi(CUR_TOKEN);

                node = NodeCtor(nullptr,
                                nullptr,
                                nullptr,
                                kIdentifier,
                                pos);

                break;
            }
            case kFuncDef:
            {
                GO_TO_NEXT_TOKEN;

                int pos = atoi(CUR_TOKEN);

                node = NodeCtor(nullptr,
                                nullptr,
                                nullptr,
                                kFuncDef,
                                pos);

                break;
            }

            case kParamsNode:
            {
                node = NodeCtor(nullptr,
                                nullptr,
                                nullptr,
                                kParamsNode,
                                0);
                break;
            }

            case kVarDecl:
            {
                GO_TO_NEXT_TOKEN;

                int pos = atoi(CUR_TOKEN);

                node = NodeCtor(nullptr,
                                nullptr,
                                nullptr,
                                kVarDecl,
                                pos);

                break;
            }

            case kCall:
            {
                node = NodeCtor(nullptr,
                                nullptr,
                                nullptr,
                                kCall,
                                0);

                break;
            }
            default:
            {
                printf("CreateNodeFromText() -> KAVO? 1000-7 ???");

                break;
            }
        }
    }

    GO_TO_NEXT_TOKEN;


    node->left =  CreateNodeFromBrackets(identifiers, text, iterator);

    node->right = CreateNodeFromBrackets(identifiers, text, iterator);

    if (*CUR_TOKEN != ')')
    {
        TreeDtor(node);

        return nullptr;;
    }

    return node;
}

//==============================================================================

static TreeNode *CreateNodeFromBrackets(Identifiers *identifiers,
                                        Text           *text,
                                        size_t         *iterator)
{
    TreeNode *node = nullptr;


    if (*CUR_TOKEN == '(')
    {
        node = CreateNodeFromText(identifiers, text, iterator);

        GO_TO_NEXT_TOKEN;

        if (node == nullptr)
        {
            return nullptr;
        }
    }
    else if (*CUR_TOKEN != ')')
    {
        if (*CUR_TOKEN == '_')
        {
            GO_TO_NEXT_TOKEN;

            return nullptr;
        }
    }

    return node;
}
//==============================================================================

#undef CUR_TOKEN
#undef GO_TO_NEXT_TOKEN

//==============================================================================

TreeNode *CopyNode(const TreeNode *src_node)
{
    if (src_node == nullptr)
    {
        return nullptr;
    }

    TreeNode *node = (TreeNode *) calloc(1, sizeof(TreeNode));

    node->data        = src_node->data;
    node->line_number = src_node->line_number;
    node->type        = src_node->type;

    node->left   = CopyNode(src_node->left);
    node->right  = CopyNode(src_node->right);

    return node;
}

//==============================================================================

TreeErrs_t SetParents(TreeNode *parent_node)
{
    if (parent_node == nullptr)
    {
        return kTreeSuccess;
    }

    if (parent_node->left != nullptr)
    {
        parent_node->left->parent = parent_node;

        SetParents(parent_node->left);
    }
    if (parent_node->right != nullptr)
    {
        parent_node->right->parent = parent_node;

        SetParents(parent_node->right);
    }

    return kTreeSuccess;
}

//==============================================================================

TreeErrs_t LanguageContextInit(LanguageContext *language_context)
{
    TreeCtor(&language_context->syntax_tree);

    VarArrayInit(&language_context->identifiers);

    NameTablesInit(&language_context->tables);

    return kTreeSuccess;
}

//==============================================================================

TreeErrs_t LanguageContextDtor(LanguageContext *language_context)
{
    TreeDtor(language_context->syntax_tree.root);
    VarArrayDtor(&language_context->identifiers);

    return kTreeSuccess;
}

//==============================================================================

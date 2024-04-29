#include <stdio.h>

#include "backend.h"

static BackendErrs_t GetVarPos(TableOfNames *table,
                               size_t        var_id_pos,
                               size_t       *ret_id_pos);


static BackendErrs_t AsmExternalDeclarations(List            *instruction_list,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node);

static BackendErrs_t AsmFuncDeclaration     (List            *instruction_list,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node);


static BackendErrs_t AsmFuncExit(BackendContext  *backend_context,
                                 LanguageContext *language_context,
                                 TreeNode        *cur_node);


static BackendErrs_t AsmFuncEntry(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  TreeNode        *cur_node);



static size_t if_count         = 0;
static size_t logical_op_count = 0;
static size_t cycle_op_count   = 0;

//==============================================================================


BackendErrs_t BackendContextInit(BackendContext *backend_context)
{
    CHECK(backend_context);

    backend_context->instruction_list = (List *) calloc(1, sizeof(List));

    if (ListConstructor(backend_context->instruction_list) != kListClear)
    {
        return kListConstructorError;
    }

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendContextDestroy(BackendContext *backend_context)
{
    CHECK(backend_context);

    if (ListDestructor(backend_context->instruction_list) != kListClear)
    {
        return kListDestructorError;
    }

    free(backend_context->instruction_list);

    return kBackendSuccess;
}


//==============================================================================

static BackendErrs_t GetVarPos(TableOfNames *table,
                               size_t        var_id_pos,
                               size_t       *ret_id_pos)
{
    for (size_t i = 0; i < table->name_count; i++)
    {
        if (var_id_pos == table->names[i].pos)
        {
            *ret_id_pos = i;

            return kBackendSuccess;
        }
    }

    return kCantFindSuchVariable;
}

//==============================================================================

BackendErrs_t GetAsmInstructionsOutLanguageContext(BackendContext  *backend_context,
                                                   LanguageContext *language_context)
{
    CHECK(backend_context);
    CHECK(language_context);

    TreeNode *root = language_context->syntax_tree.root;

    if (root == nullptr)
    {
        printf("%s(): null tree\n", __func__);

        return kBackendNullTree;
    }

    AsmExternalDeclarations(instruction_list, language_context, root);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AsmExternalDeclarations(BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);

    TreeNode *cur_decl = cur_node->left;

    while (cur_decl != nullptr)
    {
        switch (cur_decl->type)
        {
            case kFuncDef:
            {
                AsmFuncDeclaration(backend_context, language_context, cur_node);

                break;
            }

            case kIdentifier:
            case kOperator:
            case kParamsNode:
            case kVarDecl:
            case kCall:
            default:
            {
                printf("%s(): unknown node type\n", __func__);

                return kBackendUnknownNodeType;

                break;
            }
        }
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AsmFuncDeclaration(BackendContext  *backend_context,
                                        LanguageContext *language_context,
                                        TreeNode        *cur_node)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);

    AsmFuncEntry(backend_context, language_context, cur_node);

    AsmFuncExit(backend_context, language_context, cur_node);

    return kBackendSuccess;;
}

#define PUSH_REGISTER

//==============================================================================

static BackendErrs_t AddInstruction(BackendContext *backend_context,
                                    Opcode_t        op_code,
                                    RegisterCode_t  source_reg_arg,
                                    RegisterCode_t  receiver_reg_arg,
                                    uint32_t        immediate_arg)
{

}

//==============================================================================

static BackendErrs_t AsmFuncEntry(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  TreeNode        *cur_node)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AsmFuncExit(BackendContext  *backend_context,
                                 LanguageContext *language_context,
                                 TreeNode        *cur_node)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);

    return kBackendSuccess;
}
//==============================================================================

TreeErrs_t WriteAsmCodeInFile(LanguageContext *language_context,
                              const char      *output_file_name)
{
    FILE *output_file = fopen(output_file_name, "w");

    if (output_file == nullptr)
    {
        perror("MakeAsmCode() failed to open output_file");

        return kFailedToOpenFile;
    }

    fclose(output_file);

    return kTreeSuccess;
}

//==============================================================================

#define ASM_PRINT(...)                fprintf(output_file, __VA_ARGS__)
#define ASM_GO_TO_NEXT_LINE()         fprintf(output_file, "\n")

#define ASM_NODE(node)      //AssembleOp(node, language_context, cur_table, output_file)

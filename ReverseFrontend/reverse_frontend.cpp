#include "reverse_frontend.h"

static TreeErrs_t PrintExternalDeclarations(TreeNode      *node,
                                            LanguageContext *language_context,
                                            FILE          *output_file);

static TreeErrs_t PrintFuncDefinition(TreeNode      *node,
                                      LanguageContext *language_context,
                                      FILE          *output_file);

static TreeErrs_t PrintDeclaration(TreeNode      *node,
                                   LanguageContext *language_context,
                                   FILE          *output_file);

static const char *FindKeyword(KeyCode_t code);

static TreeErrs_t PrintFuncDefinition(TreeNode      *node,
                                      LanguageContext *language_context,
                                      FILE          *output_file);

static TreeErrs_t PrintDefParams(TreeNode      *node,
                                 LanguageContext *language_context,
                                 FILE          *output_file);

static TreeErrs_t PrintInstructions(TreeNode      *node,
                                    LanguageContext *language_context,
                                    FILE          *output_file);

static TreeErrs_t PrintOperator(TreeNode      *node,
                                LanguageContext *language_context,
                                FILE          *output_file);

static TreeErrs_t PrintFuncCall(TreeNode      *node,
                                LanguageContext *language_context,
                                FILE          *output_file);

static TreeErrs_t PrintParams(TreeNode       *node,
                              LanguageContext  *language_context,
                              FILE           *output_file);

static TreeErrs_t PrintVarDeclaration(TreeNode      *node,
                                      LanguageContext *language_context,
                                      FILE          *output_file);

static TreeErrs_t PrintNode(TreeNode      *node,
                            LanguageContext *language_context,
                            FILE          *output_file);


//==============================================================================

TreeErrs_t ReverseFrontend(LanguageContext *language_context,
                           const char    *output_file_name)
{
    FILE *output_file = fopen(output_file_name, "w");

    if (output_file == nullptr)
    {
        printf("ReverseFrontend() failed to open file");

        return kFailedToOpenFile;
    }

    PrintExternalDeclarations(language_context->syntax_tree.root, language_context, output_file);

    fclose(output_file);

    return kTreeSuccess;
}

//==============================================================================

static const char *FindKeyword(KeyCode_t code)
{
    for (size_t i = 0; i < kKeyWordCount; i++)
    {
        if (NameTable[i].key_code == code)
        {
            return NameTable[i].key_word;
        }
    }

    return nullptr;
}

//==============================================================================

#define FRONT_PRINT(...)    fprintf(output_file, __VA_ARGS__)

#define PRINT_END_OF_LINE() fprintf(output_file, "%s\n", FindKeyword(kEndOfLine));

#define PRINT_KWD(code) fprintf(output_file, "%s\n", FindKeyword(code))

//==============================================================================

static TreeErrs_t PrintExternalDeclarations(TreeNode      *node,
                                            LanguageContext *language_context,
                                            FILE          *output_file)
{
    TreeNode *decl_node = node;

    while (decl_node != nullptr)
    {
    printf("HUY %d\n", __LINE__);

        PrintDeclaration(decl_node->left, language_context, output_file);

        FRONT_PRINT("\n");

        decl_node = decl_node->right;
    }

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintDeclaration(TreeNode      *node,
                                   LanguageContext *language_context,
                                   FILE          *output_file)
{
    if (node->type == kVarDecl)
    {
        PrintVarDeclaration(node, language_context, output_file);
    }
    else if (node->type == kFuncDef)
    {
        PrintFuncDefinition(node, language_context, output_file);
    }
    else
    {
        printf("What the fuck?\n");

        return kUnknownType;
    }

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintVarDeclaration(TreeNode      *node,
                                      LanguageContext *language_context,
                                      FILE          *output_file)
{
    FRONT_PRINT("%s ", FindKeyword(node->left->data.key_word_code));

    PrintNode(node->right, language_context, output_file);


    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintFuncDefinition(TreeNode        *node,
                                      LanguageContext *language_context,
                                      FILE            *output_file)
{
    FRONT_PRINT("%s %s ", FindKeyword(node->left->data.key_word_code),
                          language_context->identifiers.identifier_array[node->data.variable_pos]);

    TreeNode *params_node = node->right;


    PrintDefParams(params_node->left, language_context, output_file);

    printf("HUY %d\n", __LINE__);

    FRONT_PRINT("%s\n", FindKeyword(kLeftZoneBracket));

    PrintInstructions(params_node->right, language_context, output_file);

    FRONT_PRINT("%s\n", FindKeyword(kRightZoneBracket));

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintDefParams(TreeNode      *node,
                                 LanguageContext *language_context,
                                 FILE          *output_file)
{
    FRONT_PRINT("%s ", FindKeyword(kLeftBracket));

    TreeNode *params_node = node;

    if (params_node != nullptr)
    {
        if (params_node->left != nullptr)
        {
            PrintDeclaration(params_node->left, language_context, output_file);
        }

        params_node = params_node->right;

        while (params_node != nullptr)
        {
            FRONT_PRINT("%s ", FindKeyword(kEnumOp));

            PrintDeclaration(params_node->left, language_context, output_file);

            params_node = params_node->right;
        }
    }

    FRONT_PRINT("%s\n", FindKeyword(kRightBracket));

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintInstructions(TreeNode      *node,
                                    LanguageContext *language_context,
                                    FILE          *output_file)
{
    TreeNode *instruction_node = node;

    while (instruction_node != nullptr)
    {
        PrintNode(instruction_node->left, language_context, output_file);

        instruction_node = instruction_node->right;
    }

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintNode(TreeNode      *node,
                            LanguageContext *language_context,
                            FILE          *output_file)
{
    if (node == nullptr)
    {
        return kNullTree;
    }

    switch (node->type)
    {
        case kConstNumber:
        {
            FRONT_PRINT("%lg ", node->data.const_val);

            break;
        }

        case kIdentifier:
        {
            FRONT_PRINT("%s ", language_context->identifiers.identifier_array[node->data.variable_pos].id);

            break;
        }

        case kOperator:
        {
            PrintOperator(node, language_context, output_file);

            break;
        }


        case kVarDecl:
        {
            PrintVarDeclaration(node, language_context, output_file);

            break;
        }

        case kCall:
        {
            PrintFuncCall(node, language_context, output_file);

            break;
        }

        case kFuncDef:

        default:
        {
            printf("LUCHSHE BE BOSS THEN OBSOS\n");

            break;
        }
    }

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintOperator(TreeNode      *node,
                                LanguageContext *language_context,
                                FILE          *output_file)
{
    if (node == nullptr)
    {
        return kNullTree;
    }

    switch (node->data.key_word_code)
    {
        case kAdd:
        case kSub:
        case kMult:
        case kDiv:
        case kMore:
        case kLess:
        case kAnd:
        case kOr:
        case kMoreOrEqual:
        case kLessOrEqual:
        case kEqual:
        {
            PrintNode(node->left, language_context, output_file);

            FRONT_PRINT("%s ", FindKeyword(node->data.key_word_code));

            PrintNode(node->right, language_context, output_file);

            break;
        }

        case kFloor:
        case kDiff:
        case kSqrt:
        case kScan:
        {
            FRONT_PRINT("%s %s ", FindKeyword(node->data.key_word_code),
                                  FindKeyword(kLeftBracket));

            PrintNode(node->right, language_context, output_file);

            FRONT_PRINT("%s ", FindKeyword(kRightBracket));

            break;
        }

        case kReturn:
        case kContinue:
        case kAbort:
        case kBreak:
        case kPrint:
        {
            FRONT_PRINT("%s %s ", FindKeyword(node->data.key_word_code),
                                  FindKeyword(kLeftBracket));

            PrintNode(node->right, language_context, output_file);

            FRONT_PRINT("%s ", FindKeyword(kRightBracket));

            PRINT_KWD(kEndOfLine);

            FRONT_PRINT("\n");

            break;
        }

        case kWhile:
        {
            FRONT_PRINT("%s %s ", FindKeyword(kWhile), FindKeyword(kLeftBracket));

            PrintNode(node->left, language_context, output_file);

            FRONT_PRINT("%s\n%s\n", FindKeyword(kRightBracket), FindKeyword(kLeftZoneBracket));

            PrintInstructions(node->right, language_context, output_file);

            FRONT_PRINT("%s\n", FindKeyword(kRightZoneBracket));

            break;
        }

        case kIf:
        {
            FRONT_PRINT("%s %s ", FindKeyword(kIf), FindKeyword(kLeftBracket));

            PrintNode(node->left, language_context, output_file);

            FRONT_PRINT("%s\n%s\n", FindKeyword(kRightBracket), FindKeyword(kLeftZoneBracket));

            PrintInstructions(node->right, language_context, output_file);

            FRONT_PRINT("%s\n", FindKeyword(kRightZoneBracket));

            break;
        }

        case kAssign:
        {
            PrintNode(node->right, language_context, output_file);

            FRONT_PRINT("%s ", FindKeyword(kAssign));

            PrintNode(node->left, language_context, output_file);

            PRINT_KWD(kEndOfLine);

            FRONT_PRINT("\n");

            break;
        }

        case kEndOfLine:
        {
            PrintNode(node->left, language_context, output_file);

            PrintNode(node->right, language_context, output_file);

            break;
        }

        default:
        {
            printf("UNKNOWN OP_CODE!!! %d; node pointer = %p\n", node->data.key_word_code, node);

            return kUnknownKeyCode;

            break;
        }
    }

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintFuncCall(TreeNode      *node,
                                LanguageContext *language_context,
                                FILE          *output_file)
{
    FRONT_PRINT("%s ", language_context->identifiers.identifier_array[node->right->data.variable_pos].id);

    PrintParams(node->left, language_context, output_file);

    return kTreeSuccess;
}

//==============================================================================

static TreeErrs_t PrintParams(TreeNode       *node,
                              LanguageContext  *language_context,
                              FILE           *output_file)
{
    FRONT_PRINT("%s ", FindKeyword(kLeftBracket));

    TreeNode *params_node = node;

    if (params_node->left != nullptr)
    {
        PrintNode(params_node->left, language_context, output_file);
    }

    params_node = params_node->right;

    while (params_node != nullptr)
    {
        FRONT_PRINT("%s ", FindKeyword(kEnumOp));

        PrintNode(params_node->left, language_context, output_file);

        params_node = params_node->right;
    }

    FRONT_PRINT("%s ", FindKeyword(kRightBracket));

    return kTreeSuccess;
}


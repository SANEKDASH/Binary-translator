#include <stdio.h>

#include "backend.h"

#include "backend_common.h"
#include "backend_dump.h"

static const char *id_table_file_name = "id_table.txt";

static BackendErrs_t GetVariablePos(TableOfNames *table,
                                    size_t        var_id_pos,
                                    size_t       *ret_id_pos);

static int GetNameTablePos(NameTables *name_tables,
                           int         func_code);

static BackendErrs_t AddInstruction(BackendContext *backend_context,
                                    Opcode_t        op_code,
                                    RegisterCode_t  source_reg_arg,
                                    RegisterCode_t  receiver_reg_arg,
                                    int32_t         immediate_arg,
                                    int32_t         displacement);

static BackendErrs_t AsmExternalDeclarations(BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node);

static BackendErrs_t AsmFuncDeclaration     (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node);

static BackendErrs_t AsmFuncEntry           (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node);

static BackendErrs_t AsmFuncExit            (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node);

static BackendErrs_t SetInstruction(Instruction   *instruction,
                                    size_t         begin_address,
                                    uint8_t        op_code,
                                    int32_t        displacement,
                                    int64_t        immediate_arg);

static BackendErrs_t AsmLanguageInstructions(BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node,
                                             TableOfNames    *cur_table);

static BackendErrs_t AsmVariableDeclaration (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node,
                                             TableOfNames    *cur_table);

static BackendErrs_t AsmGetFuncParams       (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node,
                                             TableOfNames    *cur_table);

static BackendErrs_t AsmFunctionCall        (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node,
                                             TableOfNames    *cur_table);

static BackendErrs_t AsmOperator            (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node,
                                             TableOfNames    *cur_table);

static BackendErrs_t GetInstructionSize(Instruction *instruction);

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

static int GetNameTablePos(NameTables *name_tables,
                           int         func_code)
{
    for (size_t i = 0; i < name_tables->tables_count; i++)
    {
        if (func_code == name_tables->name_tables[i]->func_code)
        {
            return i;
        }
    }

    return -1;
}

//==============================================================================

static BackendErrs_t GetVariablePos(TableOfNames *table,
                                    size_t        var_id_pos,
                                    size_t       *ret_id_pos)
{
    CHECK(table);
    CHECK(ret_id_pos);

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

    BeginBackendDump();

    AsmExternalDeclarations(backend_context, language_context, root);

    EndBackendDump();

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

    while (cur_node != nullptr)
    {

        TreeNode *cur_decl = cur_node->left;

        switch (cur_decl->type)
        {
            case kFuncDef:
            {
                AsmFuncDeclaration(backend_context, language_context, cur_decl);

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

        cur_node = cur_node->right;
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

    int name_table_pos = GetNameTablePos(&language_context->tables, cur_node->data.variable_pos);

    if (name_table_pos < 0)
    {
        return kCantFindNameTable;
    }

    AsmFuncEntry(backend_context, language_context, cur_node);

    TreeNode *params_node = cur_node->right;

// strange naming
    AsmGetFuncParams(backend_context,
                     language_context,
                     params_node->left,
                     language_context->tables.name_tables[name_table_pos]);

    AsmLanguageInstructions(backend_context,
                            language_context,
                            params_node->right,
                            language_context->tables.name_tables[name_table_pos]);

    return kBackendSuccess;;
}

//==============================================================================

#define PUSH_REGISTER(reg)                                                 AddInstruction(backend_context, kPushRegister          , reg         , kNotRegister, 0  , 0)
#define PUSH_IMMEDIATE(imm)                                                AddInstruction(backend_context, kPushImmediate         , kNotRegister, kNotRegister, imm, 0)

#define RET()                                                              AddInstruction(backend_context, kRet                   , kNotRegister, kNotRegister, 0  , 0)
#define LEAVE()                                                            AddInstruction(backend_context, kLeave                 , kNotRegister, kNotRegister, 0  , 0)
#define POP_IN_REGISTER(reg)                                               AddInstruction(backend_context, kPopInRegister         , kNotRegister, reg         , 0  , 0)

#define MOV_IMM_TO_REGISTER(imm, reg)                                      AddInstruction(backend_context, kMovImmediateToRegister, kNotRegister, reg         , imm, 0)
#define MOV_REGISTER_TO_REGISTER(source_reg, receiver_reg)                 AddInstruction(backend_context, kMovRegisterToRegister , source_reg  , receiver_reg, 0  , 0)
#define MOV_REGISTER_TO_REG_MEMORY(source_reg, receiver_reg, displacement) AddInstruction(backend_context, kMovRegisterToMemory   , source_reg  , receiver_reg, 0  , displacement)

#define ADD_IMM_TO_REGISTER(imm, receiver_reg)                             AddInstruction(backemd_context, kAddImmediateToRegister, kNotRegister, receiver_reg, imm, 0)
#define ADD_REGISTER_TO_REGISTER(receiver_reg, source_reg)                 AddInstruction(backend_context, kAddRegisterToRegister , source_reg  , receiver_reg, 0  , 0)

#define ADD_INSTRUCTION(instr) ListAddAfter(backend_context->instruction_list, backend_context->instruction_list->tail, instr);

//==============================================================================

static BackendErrs_t AsmLanguageInstructions(BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node,
                                             TableOfNames    *cur_table)
{

    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);
    CHECK(cur_table);

    while (cur_node != nullptr)
    {
        TreeNode *instruction_node = cur_node->left;

        switch(instruction_node->type)
        {
            case kOperator:
            {
                 AsmOperator(backend_context,
                             language_context,
                             instruction_node,
                             cur_table);
                break;
            }

            case kCall:
            {
                AsmFunctionCall(backend_context,
                                language_context,
                                instruction_node,
                                cur_table);

                break;
            }

            case kVarDecl:
            {
                AsmVariableDeclaration(backend_context,
                                       language_context,
                                       instruction_node,
                                       cur_table);
                break;
            }

            default:
            {
                ColorPrintf(kRed, "%s() unknown node type - %d\n", __func__, instruction_node->type);

                return kBackendUnknownNodeType;

                break;
            }
        }

        cur_node = cur_node->right;
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AsmFunctionCall(BackendContext  *backend_context,
                                     LanguageContext *language_context,
                                     TreeNode        *cur_node,
                                     TableOfNames    *cur_table)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);
    CHECK(cur_table);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AsmVariableDeclaration(BackendContext  *backend_context,
                                            LanguageContext *language_context,
                                            TreeNode        *cur_node,
                                            TableOfNames    *cur_table)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);
    CHECK(cur_table);

    if (cur_node->type != kVarDecl)
    {
        return kBackendNotVarDecl;
    }

    TreeNode *assign_node = cur_node->right;

    if (assign_node->type               != kOperator ||
        assign_node->data.key_word_code != kAssign)
    {
        return kBackendNotAssign;
    }

    AsmOperator(backend_context, language_context, assign_node->left, cur_table);

    size_t variable_pos = 0;

    if (GetVariablePos(cur_table, assign_node->right->data.variable_pos, &variable_pos) != kBackendSuccess)
    {
        ColorPrintf(kRed, "%s() cant find variable in current name table\n", __func__);

        return kCantFindVariable;
    }

    MOV_REGISTER_TO_REG_MEMORY(kRAX, kRBP, - (variable_pos + 1) * 8 );

    return kBackendSuccess;
}

//==============================================================================
// AsmNode?
static BackendErrs_t AsmOperator(BackendContext  *backend_context,
                                 LanguageContext *language_context,
                                 TreeNode        *cur_node,
                                 TableOfNames    *cur_table)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_table);

    if (cur_node == nullptr)
    {
        return kBackendNullTree;
    }

    if (cur_node->type == kConstNumber)
    {
        MOV_IMM_TO_REGISTER(cur_node->data.const_val, kRAX);

        return kBackendSuccess;
    }

    switch(cur_node->data.key_word_code)
    {
        case kReturn:
        {
            AsmOperator(backend_context,
                        language_context,
                        cur_node->right,
                        cur_table);

            LEAVE();

            RET();

            break;
        }

        case kAdd:
        {


            break;
        }

        case kAssign:
        {
            AsmOperator(backend_context,
                        language_context,
                        cur_node->left,
                        cur_table);

            size_t variable_pos = 0;

            if (GetVariablePos(cur_table, cur_node->right->data.variable_pos, &variable_pos) != kBackendSuccess)
            {
                ColorPrintf(kRed, "%s() cant find variable in current name table\n", __func__);

                return kCantFindVariable;
            }

            MOV_REGISTER_TO_REG_MEMORY(kRAX, kRBP, - (variable_pos + 1) * 8 );

            break;
        }

        default:
        {
            ColorPrintf(kRed, "%s() unknown operator\n", __func__);

            return kBackendUnknownNodeType;

            break;
        }
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AsmGetFuncParams(BackendContext  *backend_context,
                                      LanguageContext *language_context,
                                      TreeNode        *cur_node,
                                      TableOfNames    *cur_table)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);
    CHECK(cur_table);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AsmFuncEntry(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  TreeNode        *cur_node)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);

    PUSH_REGISTER(kRBP);

    MOV_REGISTER_TO_REGISTER(kRSP, kRBP);

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

    POP_IN_REGISTER(kRBP);

    RET();

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t GetInstructionSize(Instruction *instruction)
{
    CHECK(instruction);

    instruction->instruction_size = 0;

    if (instruction->displacement != 0)
    {
        instruction->instruction_size += sizeof(instruction->displacement);
    }

    if (instruction->immediate_arg != 0)
    {
        instruction->instruction_size += sizeof(instruction->immediate_arg);
    }

    if (instruction->mod_rm != 0)
    {
        instruction->instruction_size += sizeof(instruction->mod_rm);
    }

    if (instruction->rex_prefix != 0)
    {
        instruction->instruction_size += sizeof(instruction->rex_prefix);
    }

    if (instruction->op_code != 0)
    {
        instruction->instruction_size += sizeof(instruction->op_code);
    }

    return kBackendSuccess;
}

//==============================================================================
static BackendErrs_t SetInstruction(Instruction   *instruction,
                                    size_t         begin_address,
                                    uint8_t        op_code,
                                    int32_t        displacement,
                                    int64_t        immediate_arg)
{
    CHECK(instruction);

    instruction->begin_address = begin_address;
    instruction->op_code       = op_code;
    instruction->displacement  = displacement;
    instruction->immediate_arg = immediate_arg;

    GetInstructionSize(instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetRexPrefix(Instruction *instruction,
                                  uint8_t      qword_usage,
                                  uint8_t      register_extension,
                                  uint8_t      sib_extension,
                                  uint8_t      mod_rm_extension)
{
    CHECK(instruction);

    instruction->rex_prefix = 0;

    instruction->rex_prefix |= kRexHeader;

    instruction->rex_prefix |= qword_usage;
    instruction->rex_prefix |= register_extension;
    instruction->rex_prefix |= sib_extension;
    instruction->rex_prefix |= mod_rm_extension;

    return kBackendSuccess;;
}

//==============================================================================

static BackendErrs_t SetModRm(Instruction    *instruction,
                              ModRmCode_t     mode,
                              RegisterCode_t  source_register,
                              RegisterCode_t  receiver_register)
{
    CHECK(instruction);

    instruction->mod_rm = 0;

    instruction->mod_rm |= mode;

    if (source_register != kNotRegister)
    {
        instruction->mod_rm |= source_register << 3;
    }

    if (receiver_register != kNotRegister)
    {
        instruction->mod_rm |= receiver_register;
    }

    return kBackendSuccess;
}

#define SET_INSTRUCTION(op_code, displacement, imm_arg) SetInstruction(&instruction,                 \
                                                                       backend_context->cur_address, \
                                                                       op_code,                      \
                                                                       displacement,                 \
                                                                       imm_arg)

#define SET_MOD_RM(mode, source_reg, receiver_reg) SetModRm(&instruction,  \
                                                             mode,         \
                                                             source_reg,   \
                                                             receiver_reg)

#define SET_REX_PREFIX(qword_usage, register_extension, sib_extension, mod_rm_extension) SetRexPrefix(&instruction,        \
                                                                                                       qword_usage,        \
                                                                                                       register_extension, \
                                                                                                       sib_extension,      \
                                                                                                       mod_rm_extension)

//==============================================================================

static BackendErrs_t AddInstruction(BackendContext *backend_context,
                                    Opcode_t        op_code,
                                    RegisterCode_t  source_reg_arg,
                                    RegisterCode_t  receiver_reg_arg,
                                    int32_t         immediate_arg,
                                    int32_t         displacement)
{
    Instruction instruction = {0};

    switch (op_code)
    {
        case kPushRegister:
        {
            SET_INSTRUCTION(op_code + source_reg_arg, 0, 0);

            break;
        }

        case kMovRegisterToRegister:
        {
            SET_REX_PREFIX(kQwordUsing, 0, 0, 0);

            SET_MOD_RM(kRegister, kRSP, kRBP);

            SET_INSTRUCTION(op_code, 0, 0);

            break;
        }

        case kRet:
        {
            SET_INSTRUCTION(kRet, 0, 0);

            break;
        }

        case kPopInRegister:
        {
            // if new register we need rex prefix

            SET_INSTRUCTION(op_code + receiver_reg_arg, 0, 0);

            break;
        }

        case kMovImmediateToRegister:
        {
            SET_REX_PREFIX(kQwordUsing, 0, 0, 0);

            SET_INSTRUCTION(op_code + receiver_reg_arg, 0, immediate_arg);

            break;
        }

        case kMovRegisterToMemory:
        {
            SET_REX_PREFIX(kQwordUsing, 0, 0, 0);

            SET_MOD_RM(kRegisterMemory16Displacement, source_reg_arg, receiver_reg_arg);

            SET_INSTRUCTION(kMovRegisterToRegister, displacement, 0);

            break;
        }

        case kAddImmediateToRegister:
        {
            SET_REX_PREFIX(kQwordUsing, 0, 0, 0);

            SET_MOD_RM(kRegister, receiver_reg_arg, kNotRegister);

            SET_INSTRUCTION(op_code, 0, immediate_arg);

            break;
        }

        case kAddRegisterToRegister:
        {
            SET_REX_PREFIX(kQwordUsing, 0, 0, 0);

            SET_MOD_RM(kRegister, source_reg_arg, receiver_reg_arg);

            SET_INSTRUCTION(op_code, 0, 0);
        }

        case kLeave:
        {
            SET_INSTRUCTION(kLeave, 0, 0);

            break;
        }

        default:
        {
            ColorPrintf(kRed, "%s() Unknown opcode", __func__);

            return kBackendUnknownOpcode;

            break;
        }
    }

    ADD_INSTRUCTION(&instruction);

    backend_context->cur_address += instruction.instruction_size;

    BackendDumpPrint(&instruction,
                      op_code,
                      source_reg_arg,
                      receiver_reg_arg);

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

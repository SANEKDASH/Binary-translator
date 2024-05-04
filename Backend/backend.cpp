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

static BackendErrs_t AsmFuncEntry(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  TreeNode        *cur_node,
                                  TableOfNames    *cur_table);

static BackendErrs_t AsmFuncExit            (BackendContext  *backend_context,
                                             LanguageContext *language_context,
                                             TreeNode        *cur_node);

static BackendErrs_t SetInstruction(Instruction     *instruction,
                                    size_t           begin_address,
                                    uint8_t          op_code,
                                    int32_t          displacement,
                                    int64_t          immediate_arg,
                                    LogicalOpcode_t  logical_op_code);

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

static BackendErrs_t SetInstructionSize(Instruction *instruction);


static BackendErrs_t InitLabelTable(LabelTable *label_table);


static BackendErrs_t ReallocLabelTable(LabelTable *label_table,
                                       size_t      new_size);

static BackendErrs_t EncodeLeave(BackendContext *backend_context);

static BackendErrs_t EncodeCmpRegisterWithRegister(BackendContext *backend_context,
                                                   RegisterCode_t  dest_reg,
                                                   RegisterCode_t  src_reg);

static BackendErrs_t EncodeCmpRegisterWithImmediate(BackendContext *backend_context,
                                                    RegisterCode_t  dest_reg,
                                                    ImmediateType_t immediate);

static BackendErrs_t EncodeImulRegister(BackendContext *backend_context,
                                        RegisterCode_t  dest_reg);

static BackendErrs_t EncodeDivRegister(BackendContext *backend_context,
                                       RegisterCode_t  dest_reg);

static BackendErrs_t EncodeXorRegisterWithRegister(BackendContext *backend_context,
                                                   RegisterCode_t  dest_reg,
                                                   RegisterCode_t  src_reg);

static BackendErrs_t EncodeSubImmediateFromRegister(BackendContext *backend_context,
                                                    RegisterCode_t  dest_reg,
                                                    ImmediateType_t immediate);

static BackendErrs_t EncodeSubRegisterFromRegister(BackendContext *backend_context,
                                                   RegisterCode_t  src_reg,
                                                   RegisterCode_t  dest_reg);

static BackendErrs_t EncodeMovRegisterMemoryToRegister(BackendContext     *backend_context,
                                                       RegisterCode_t      src_reg,
                                                       RegisterCode_t      dest_reg,
                                                       DisplacementType_t  displacement);

static BackendErrs_t EncodeAddRegisterToRegister(BackendContext *backend_context,
                                                 RegisterCode_t  src_reg,
                                                 RegisterCode_t  dest_reg);

static BackendErrs_t EncodeAddImmediateToRegister(BackendContext  *backend_context,
                                                  ImmediateType_t  immediate,
                                                  RegisterCode_t   dest_reg);

static BackendErrs_t EncodeMovRegisterToRegisterMemory(BackendContext     *backend_context,
                                                       RegisterCode_t      src_reg,
                                                       RegisterCode_t      dest_reg,
                                                       DisplacementType_t  displacement);

static BackendErrs_t EncodeMovImmediateToRegister(BackendContext  *backend_context,
                                                  ImmediateType_t  immediate,
                                                  RegisterCode_t   dest_reg);

static BackendErrs_t EncodePopInRegister(BackendContext *backend_context,
                                         RegisterCode_t  dest_reg);

static BackendErrs_t EncodeRet(BackendContext *backend_context);

static BackendErrs_t EncodeMovRegisterToRegister(BackendContext *backend_context,
                                                 RegisterCode_t  src_reg,
                                                 RegisterCode_t  dest_reg);

static BackendErrs_t EncodePushRegister(BackendContext *backend_context,
                                        RegisterCode_t  reg);

static BackendErrs_t EncodeCall(BackendContext  *backend_context,
                                LanguageContext *language_context,
                                size_t           func_pos);

static BackendErrs_t AddLabel(LabelTable *label_table,
                              size_t      address,
                              int32_t     func_pos,
                              int32_t     identification_number);

static size_t GetCurSize(BackendContext *backend_context);

static BackendErrs_t SetJumpRelativeAddress(Instruction *jump_instruction,
                                            int32_t      label_pos);

static BackendErrs_t RespondAddressRequests(BackendContext *backend_context);

static BackendErrs_t EncodeJumpIfLessOrEqual(BackendContext *backend_context);

static BackendErrs_t DestroyLabelTable(LabelTable *label_table);

static bool IsNewRegister(RegisterCode_t reg);

static BackendErrs_t PassFuncArgs(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  TreeNode        *cur_node,
                                  TableOfNames    *cur_table);

static RegisterCode_t GetRegisterBase(RegisterCode_t reg);

static BackendErrs_t InitAddressRequests(AddressRequests *address_requests);

static BackendErrs_t DestroyAddressRequests(AddressRequests *address_requests);


static BackendErrs_t AddAddressRequest(AddressRequests *address_requests,
                                       Instruction     *call_instruction,
                                       int32_t          func_pos,
                                       int32_t          label_identifier);


static BackendErrs_t ReallocAddressRequests(AddressRequests *address_requests,
                                            size_t           new_size);

static BackendErrs_t SetCallRelativeAddress(Instruction *call_instruction,
                                            uint32_t     address);


static BackendErrs_t AddFuncLabelRequest(BackendContext *backend_context,
                                         Instruction    *jump_instruction,
                                         int32_t         func_pos);

static BackendErrs_t AddCommonLabelRequest(BackendContext *backend_context,
                                           Instruction    *jump_instruction,
                                           int32_t         identification_number);

static bool IsImmediateUsing(LogicalOpcode_t logical_op_code);

static bool IfDisplacementUsing(LogicalOpcode_t logical_op_code);

static size_t if_count         = 0;
static size_t logical_op_count = 0;
static size_t cycle_op_count   = 0;



static size_t kBaseLabelTableSize = 32;

//==============================================================================

static BackendErrs_t InitAddressRequests(AddressRequests *address_requests)
{
    address_requests->capacity = kBaseCallRequestArraySize;

    address_requests->requests = (Request *) calloc(address_requests->capacity, sizeof(Request));

    if (address_requests->requests == nullptr)
    {
        return kBackendFailedAllocation;
    }

    address_requests->request_count = 0;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t DestroyAddressRequests(AddressRequests *address_requests)
{
    free(address_requests->requests);

    address_requests->requests = nullptr;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AddFuncLabelRequest(BackendContext *backend_context,
                                         Instruction    *jump_instruction,
                                         int32_t         func_pos)
{
    return AddAddressRequest(backend_context->address_requests,
                             jump_instruction,
                             func_pos,
                             kCommonLabelIdentifierPoison);
}
//==============================================================================

static BackendErrs_t AddCommonLabelRequest(BackendContext *backend_context,
                                           Instruction    *jump_instruction,
                                           int32_t         identification_number)
{
    return AddAddressRequest(backend_context->address_requests,
                             jump_instruction,
                             kFuncLabelPosPoison,
                             identification_number);
}

//==============================================================================

static BackendErrs_t AddAddressRequest(AddressRequests *address_requests,
                                       Instruction     *jmp_instruction,
                                       int32_t          func_pos,
                                       int32_t          label_identifier)
{
    if (address_requests->request_count >= address_requests->capacity)
    {
        ReallocAddressRequests(address_requests, 2 * address_requests->capacity);
    }

    address_requests->requests[address_requests->request_count].jmp_instruction    = jmp_instruction;
    address_requests->requests[address_requests->request_count].func_pos           = func_pos;
    address_requests->requests[address_requests->request_count++].label_identifier = label_identifier;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t ReallocAddressRequests(AddressRequests *address_requests,
                                            size_t           new_size)
{
    address_requests->capacity = new_size;
    address_requests->requests = (Request *) realloc(address_requests->requests,
                                                     address_requests->capacity * sizeof(Request));

    for (size_t i = address_requests->request_count;
                i < address_requests->capacity;
                i++)
    {
        address_requests->requests[i].jmp_instruction  = nullptr;
        address_requests->requests[i].func_pos         = kFuncLabelPosPoison;
        address_requests->requests[i].label_identifier = kCommonLabelIdentifierPoison;
    }

    return kBackendSuccess;
}

static BackendErrs_t SetCallRelativeAddress(Instruction *call_instruction,
                                            uint32_t     address)
{
    call_instruction->immediate_arg = address - (call_instruction->begin_address +
                                                 call_instruction->instruction_size);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t RespondAddressRequests(BackendContext *backend_context)
{
    for (size_t i = 0; i < backend_context->address_requests->request_count; i++)
    {
        for (size_t j = 0; i < backend_context->label_table->label_count; j++)
        {
            if ((backend_context->address_requests->requests[i].func_pos ==
                 backend_context->label_table->label_array[i].func_pos) &&
                (backend_context->address_requests->requests[i].label_identifier ==
                 backend_context->label_table->label_array[i].identification_number))
            {
                SetCallRelativeAddress(backend_context->address_requests->requests[i].jmp_instruction,
                                       backend_context->label_table->label_array[i].address);
            }
        }
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t InitLabelTable(LabelTable *label_table)
{
    CHECK(label_table);

    label_table->label_count = 0;

    label_table->capacity = kBaseLabelTableSize;

    label_table->label_array = (Label *) calloc(label_table->capacity, sizeof(Label));

    if (label_table->label_array == nullptr)
    {
        ColorPrintf(kRed, "%s() failed allocation. Restart your computer.\n", __func__);

        return kBackendFailedAllocation;
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t ReallocLabelTable(LabelTable *label_table,
                                       size_t      new_size)
{
    label_table->capacity = new_size;

    label_table->label_array = (Label *) realloc(label_table->label_array, label_table->capacity * sizeof(Label));

    for (size_t i = label_table->label_count + 1; i < label_table->capacity; i++)
    {
        label_table->label_array[i].address = 0;
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AddLabel(LabelTable *label_table,
                              size_t      address,
                              int32_t     func_pos,
                              int32_t     identification_number)
{
    if (label_table->label_count >= label_table->capacity)
    {
        ReallocLabelTable(label_table, label_table->capacity * 2);
    }

    label_table->label_array[label_table->label_count].address    = address;
    label_table->label_array[label_table->label_count++].func_pos = func_pos;

    label_table->label_array[label_table->label_count].identification_number = identification_number;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t DestroyLabelTable(LabelTable *label_table)
{
    CHECK(label_table);

    free(label_table->label_array);

    label_table->label_array = nullptr;

    return kBackendSuccess;
}

//==============================================================================

BackendErrs_t BackendContextInit(BackendContext *backend_context)
{
    CHECK(backend_context);

    backend_context->instruction_list = (List *) calloc(1, sizeof(List));

    if (ListConstructor(backend_context->instruction_list) != kListClear)
    {
        return kListConstructorError;
    }

    backend_context->label_table = (LabelTable *) calloc(1, sizeof(LabelTable));

    if (InitLabelTable(backend_context->label_table) != kBackendSuccess)
    {
        return kBackendLabelTableInitError;
    }

    backend_context->address_requests = (AddressRequests *) calloc(1, sizeof(AddressRequests));

    if (InitAddressRequests(backend_context->address_requests) != kBackendSuccess)
    {
        return kBackendAddressRequestsInitError;
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

    backend_context->instruction_list = nullptr;

    if (DestroyLabelTable(backend_context->label_table) != kBackendSuccess)
    {
        return kBackendLabelTableDestroyError;
    }

    free(backend_context->label_table);

    backend_context->label_table = nullptr;

    if (DestroyAddressRequests(backend_context->address_requests) != kBackendSuccess)
    {
        return kBackendDestroyAddressRequestsError;
    }

    free(backend_context->address_requests);

    backend_context->address_requests = nullptr;

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
        printf("%d\n", i);

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

    BackendDumpPrintFuncLabel(language_context, cur_node->data.variable_pos);

    AsmFuncEntry(backend_context,
                 language_context,
                 cur_node,
                 language_context->tables.name_tables[name_table_pos]);

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

static bool IsNewRegister(RegisterCode_t reg)
{
    return reg << 3;
}

//==============================================================================

#define PUSH_REGISTER(reg)                                                 EncodePushRegister(backend_context, reg)
#define PUSH_IMMEDIATE(imm)                                                AddInstruction(backend_context, kPushImmediate          , kNotRegister, kNotRegister, imm, 0)

#define RET()                                                              EncodeRet(backend_context)
#define LEAVE()                                                            EncodeLeave(backend_context)
#define POP_IN_REGISTER(reg)                                               EncodePopInRegister(backend_context, reg)

#define MOV_IMM_TO_REGISTER(imm, reg)                                      EncodeMovImmediateToRegister(backend_context, imm, reg)
#define MOV_REGISTER_TO_REGISTER(source_reg, dest_reg)                     EncodeMovRegisterToRegister(backend_context, source_reg, dest_reg)
#define MOV_REGISTER_TO_REG_MEMORY(source_reg, receiver_reg, displacement) EncodeMovRegisterToRegisterMemory(backend_context, source_reg, receiver_reg, displacement)
#define MOV_REG_MEMORY_TO_REGISTER(source_reg, displacement, receiver_reg) EncodeMovRegisterMemoryToRegister(backend_context, source_reg, receiver_reg, displacement)

#define ADD_IMM_TO_REGISTER(imm, receiver_reg)                             EncodeAddImmediateToRegister(backend_context, imm, receiver_reg)
#define ADD_REGISTER_TO_REGISTER(source_reg, receiver_reg)                 EncodeAddRegisterToRegister(backend_context, source_reg, receiver_reg)

#define SUB_REGISTER_FROM_REGISTER(source_reg, receiver_reg)               EncodeSubRegisterFromRegister(backend_context, source_reg, receiver_reg)
#define SUB_IMMEDIATE_FROM_REGISTER(immediate, receiver_reg)               EncodeSubImmediateFromRegister(backend_context, receiver_reg, immediate)

#define DIV_REGISTER(reg)                                                  EncodeDivRegister(backend_context, reg)

#define XOR_REGISTER_WITH_REGISTER(source_reg, receiver_reg)               EncodeXorRegisterWithRegister(backend_context, source_reg  , receiver_reg)

#define IMUL_ON_REGISTER(receiver_reg)                                     EncodeImulRegister(backend_context, receiver_reg)

#define CMP_REGISTER_TO_IMMEDIATE(dest_reg, immediate)                     EncodeCmpRegisterWithImmediate(backend_context, dest_reg, immediate)

#define JUMP_IF_LESS_OR_EQUAL()                                            EncodeJumpIfLessOrEqual(backend_context)

#define ADD_INSTRUCTION(instr)                                             ListAddAfter(backend_context->instruction_list, backend_context->instruction_list->tail, instr);

#define CALL(func_pos)                                                     EncodeCall(backend_context, language_context, func_pos)
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

#define ASM_OPERATOR(node) AsmOperator(backend_context, language_context, node, cur_table)

//==============================================================================

// strange naming?
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
    else if (cur_node->type == kCall)
    {
        AsmFunctionCall(backend_context, language_context, cur_node, cur_table);
    }
    else if (cur_node->type == kConstNumber)
    {
        MOV_IMM_TO_REGISTER(cur_node->data.const_val, kRAX);
    }
    else if (cur_node->type == kVarDecl)
    {
        AsmVariableDeclaration (backend_context, language_context, cur_node, cur_table);
    }
    else if (cur_node->type == kIdentifier)
    {
        size_t variable_pos = 0;

        if (GetVariablePos(cur_table, cur_node->data.variable_pos, &variable_pos) != kBackendSuccess)
        {
            ColorPrintf(kRed, "%s() failed to find variable. CUR_NODE_PTR - %p\n", __func__, cur_node);
        }

        MOV_REG_MEMORY_TO_REGISTER(kRBP, - (variable_pos + 1) * 8, kRAX);
    }
    else
    {
        switch(cur_node->data.key_word_code)
        {
            case kEndOfLine:
            {
                ASM_OPERATOR(cur_node->left);

                break;
            }

            case kReturn:
            {
                ASM_OPERATOR(cur_node->right);

                LEAVE();

                RET();

                break;
            }

            case kAdd:
            {
                ASM_OPERATOR(cur_node->right);

                MOV_REGISTER_TO_REGISTER(kRAX, kRBX);

                ASM_OPERATOR(cur_node->left);

                ADD_REGISTER_TO_REGISTER(kRBX, kRAX);

                break;
            }

            case kSub:
            {
                ASM_OPERATOR(cur_node->right);

                MOV_REGISTER_TO_REGISTER(kRAX, kRBX);

                ASM_OPERATOR(cur_node->left);

                SUB_REGISTER_FROM_REGISTER(kRBX, kRAX);

                break;
            }

            case kDiv:
            {
                ASM_OPERATOR(cur_node->left);

                MOV_REGISTER_TO_REGISTER(kRAX, kRBX);

                ASM_OPERATOR(cur_node->right);

                XOR_REGISTER_WITH_REGISTER(kRDX, kRDX);

                DIV_REGISTER(kRBX);

                break;
            }

            case kMult:
            {
                ASM_OPERATOR(cur_node->right);

                MOV_REGISTER_TO_REGISTER(kRAX, kRBX);

                ASM_OPERATOR(cur_node->right);

                IMUL_ON_REGISTER(kRBX);

                break;
            }

            case kAssign:
            {
                ASM_OPERATOR(cur_node->left);

                size_t variable_pos = 0;

                if (GetVariablePos(cur_table, cur_node->right->data.variable_pos, &variable_pos) != kBackendSuccess)
                {
                    ColorPrintf(kRed, "%s() cant find variable in current name table. CUR_NODE_PTR - %p\n", __func__, cur_node);

                    return kCantFindVariable;
                }

                MOV_REGISTER_TO_REG_MEMORY(kRAX, kRBP, - (variable_pos + 1) * 8 );

                break;
            }

            case kIf:
            {
                ASM_OPERATOR(cur_node->left);

                CMP_REGISTER_TO_IMMEDIATE(kRAX, 0);

                JUMP_IF_LESS_OR_EQUAL();

                Instruction *jump_instruction = GetLastInstruction(backend_context->instruction_list);

                int32_t cur_identify = backend_context->label_table->identify_counter++;

                AddCommonLabelRequest(backend_context, jump_instruction, cur_identify);

                ASM_OPERATOR(cur_node->right);

                size_t label_pos = 0;

                AddLabel(backend_context->label_table,
                        backend_context->cur_address,
                        kFuncLabelPosPoison,
                        cur_identify);
                break;
            }

            case kEqual:
            case kLess:
            case kMore:
            case kLessOrEqual:
            case kMoreOrEqual:
            case kNotEqual:
            case kAnd:
            case kOr:

            default:
            {
                ColorPrintf(kRed, "%s() unknown operator. Node pointer - %p\n", __func__, cur_node);

                return kBackendUnknownNodeType;

                break;
            }
        }
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

    PassFuncArgs(backend_context, language_context, cur_node->left, cur_table);

    CALL(cur_node->right->data.variable_pos);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t PassFuncArgs(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  TreeNode        *cur_node,
                                  TableOfNames    *cur_table)
{
    for (size_t i = 0; (i < kArgPassingRegisterCount) && (cur_node != nullptr); i++)
    {
        ASM_OPERATOR(cur_node->left);

        MOV_REGISTER_TO_REGISTER(kRAX, ArgPassingRegisters[i]);

        cur_node = cur_node->right;
    }

    while (cur_node != nullptr)
    {
        ASM_OPERATOR(cur_node->left);

        PUSH_REGISTER(kRAX);
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetJumpRelativeAddress(Instruction *jump_instruction,
                                            int32_t      label_pos)
{
    jump_instruction->immediate_arg = label_pos - jump_instruction->begin_address
                                                - jump_instruction->instruction_size;

    return kBackendSuccess;
}

//==============================================================================

static size_t GetCurSize(BackendContext *backend_context)
{
    return backend_context->cur_address;
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
                                  TreeNode        *cur_node,
                                  TableOfNames    *cur_table)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(cur_node);

    PUSH_REGISTER(kRBP);

    MOV_REGISTER_TO_REGISTER(kRSP, kRBP);

    SUB_IMMEDIATE_FROM_REGISTER((cur_table->name_count + 1) * 8, kRSP);

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

static bool IsImmediateUsing(LogicalOpcode_t logical_op_code)
{
    return logical_op_code == kLogicPushImmediate            ||
           logical_op_code == kLogicMovImmediateToRegister   ||
           logical_op_code == kLogicAddImmediateToRegister   ||
           logical_op_code == kLogicSubImmediateFromRegister ||
           logical_op_code == kLogicCmpRegisterToImmediate   ||
           logical_op_code == kLogicJumpIfLessOrEqual;
}

//==============================================================================

static bool IfDisplacementUsing(LogicalOpcode_t logical_op_code)
{
    return logical_op_code == kLogicMovRegisterToMemory ||
           logical_op_code == kLogicMovRmToRegister;
}

//==============================================================================

static BackendErrs_t SetInstructionSize(Instruction *instruction)
{
    CHECK(instruction);

    instruction->instruction_size = 0;

    if (IfDisplacementUsing((LogicalOpcode_t)instruction->logical_op_code))
    {
        instruction->instruction_size += sizeof(instruction->displacement);
    }

    if (IsImmediateUsing((LogicalOpcode_t)instruction->logical_op_code))
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
        instruction->instruction_size += instruction->op_code_size;
    }

    return kBackendSuccess;
}

//==============================================================================
static BackendErrs_t SetInstruction(Instruction     *instruction,
                                    size_t           begin_address,
                                    uint8_t          op_code,
                                    int32_t          displacement,
                                    int64_t          immediate_arg,
                                    LogicalOpcode_t  logical_op_code)
{
    CHECK(instruction);

    instruction->begin_address   = begin_address;
    instruction->op_code         = op_code;
    instruction->displacement    = displacement;
    instruction->immediate_arg   = immediate_arg;
    instruction->logical_op_code = logical_op_code;

    if (instruction->op_code <= 0xff)
    {
        instruction->op_code_size = 1;
    }
    else
    {
        instruction->op_code_size = 2;
    }

    SetInstructionSize(instruction);

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

#define SET_INSTRUCTION(op_code, displacement, imm_arg, logical_op_code) SetInstruction(&instruction,                 \
                                                                                        backend_context->cur_address, \
                                                                                        op_code,                      \
                                                                                        displacement,                 \
                                                                                        imm_arg,                      \
                                                                                        logical_op_code)

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

static RegisterCode_t GetRegisterBase(RegisterCode_t reg)
{
    uint32_t reg_code = reg;

    return (RegisterCode_t) (reg_code & (~((~0) << 3)));
}

//==============================================================================

static BackendErrs_t EncodeCall(BackendContext  *backend_context,
                                LanguageContext *language_context,
                                size_t           func_pos)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(kCallRel32, 0, 0, kLogicRet);

    Instruction *call_instruction = GetLastInstruction(backend_context->instruction_list);

    AddFuncLabelRequest(backend_context,
                        call_instruction,
                        func_pos);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodePushRegister(BackendContext *backend_context,
                                        RegisterCode_t  reg)
{
    Instruction instruction = {0};

    if (IsNewRegister(reg))
    {
        SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, kModRmExtension);
    }

    SET_INSTRUCTION(kPushR64 + GetRegisterBase(reg), 0, 0, kLogicPushRegister);

    ADD_INSTRUCTION(&instruction);

    BackendDumpPrint(&instruction, kLogicPushRegister, reg, kNotRegister);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeJumpIfLessOrEqual(BackendContext *backend_context)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(kJbeRel32, 0, kJmpPoison, kLogicJumpIfLessOrEqual);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeMovRegisterToRegister(BackendContext *backend_context,
                                                 RegisterCode_t  src_reg,
                                                 RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kMovR64ToRm64, 0, 0, kLogicMovRegisterToRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeRet(BackendContext *backend_context)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(kRet, 0, 0, kLogicRet);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodePopInRegister(BackendContext *backend_context,
                                         RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    if (IsNewRegister(dest_reg))
    {
        SET_REX_PREFIX(kRexPrefixNoOptions, kRegisterExtension, kRexPrefixNoOptions, kRexPrefixNoOptions);
    }

    SET_INSTRUCTION(kPopR64 + GetRegisterBase(dest_reg), 0, 0, kLogicPopInRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeMovImmediateToRegister(BackendContext  *backend_context,
                                                  ImmediateType_t  immediate,
                                                  RegisterCode_t   dest_reg)
{
    Instruction instruction = {0};

    if (IsNewRegister(dest_reg))
    {
        SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, kModRmExtension);
    }
    else
    {
        SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, kRexPrefixNoOptions);
    }

    SET_INSTRUCTION(kMovImmToR64, 0, immediate, kLogicMovImmediateToRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeMovRegisterToRegisterMemory(BackendContext *backend_context,
                                                       RegisterCode_t  src_reg,
                                                       RegisterCode_t  dest_reg,
                                                       DisplacementType_t displacement)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegisterMemory16Displacement, GetRegisterBase(src_reg),
                                              GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kMovR64ToRm64, displacement, 0, kLogicMovRegisterToMemory);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeAddImmediateToRegister(BackendContext  *backend_context,
                                                  ImmediateType_t  immediate,
                                                  RegisterCode_t   dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, kRAX, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kAddImmToRm64, 0, immediate, kLogicAddImmediateToRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeAddRegisterToRegister(BackendContext *backend_context,
                                                 RegisterCode_t  src_reg,
                                                 RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kAddR64ToRm64, 0, 0, kLogicAddRegisterToRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeMovRegisterMemoryToRegister(BackendContext     *backend_context,
                                                       RegisterCode_t      src_reg,
                                                       RegisterCode_t      dest_reg,
                                                       DisplacementType_t  displacement)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegisterMemory16Displacement, GetRegisterBase(src_reg),
                                              GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kMovRm64ToR64, displacement, 0, kLogicMovRegisterToMemory);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeSubRegisterFromRegister(BackendContext *backend_context,
                                                   RegisterCode_t  src_reg,
                                                   RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, kRexPrefixNoOptions, rm_extension);

    SET_INSTRUCTION(kSubR64FromRm64, 0, 0, kLogicSubRegisterFromRegister);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeSubImmediateFromRegister(BackendContext *backend_context,
                                                    RegisterCode_t  dest_reg,
                                                    ImmediateType_t immediate)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, kRexPrefixNoOptions, kRexPrefixNoOptions, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x5, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kSubImmFromRm64, 0, immediate, kLogicSubImmediateFromRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeXorRegisterWithRegister(BackendContext *backend_context,
                                                   RegisterCode_t  dest_reg,
                                                   RegisterCode_t  src_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, 0, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kXorRm64WithR64, 0, 0, kLogicXorRegisterWithRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeDivRegister(BackendContext *backend_context,
                                       RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension = (RexPrefixCode_t) 0;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, 0, 0, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x6, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kDivRm64, 0, 0, kLogicDivRegisterOnRax);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeImulRegister(BackendContext *backend_context,
                                        RegisterCode_t  dest_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, 0, 0, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x4, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kImulRm64, 0, 0, kLogicImulRegisterOnRax);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeCmpRegisterWithImmediate(BackendContext *backend_context,
                                                    RegisterCode_t  dest_reg,
                                                    ImmediateType_t immediate)
{
    Instruction instruction = {0};

    RexPrefixCode_t rm_extension = kRexPrefixNoOptions;

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, 0, 0, rm_extension);

    SET_MOD_RM(kRegister, (RegisterCode_t) 0x7, GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kCmpRm64WithImm32, 0, immediate, kLogicCmpRegisterToImmediate);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t EncodeCmpRegisterWithRegister(BackendContext *backend_context,
                                                   RegisterCode_t  dest_reg,
                                                   RegisterCode_t  src_reg)
{
    Instruction instruction = {0};

    RexPrefixCode_t reg_extension = kRexPrefixNoOptions;
    RexPrefixCode_t rm_extension  = kRexPrefixNoOptions;

    if (IsNewRegister(src_reg))
    {
        reg_extension = kRegisterExtension;
    }

    if (IsNewRegister(dest_reg))
    {
        rm_extension = kModRmExtension;
    }

    SET_REX_PREFIX(kQwordUsing, reg_extension, 0, rm_extension);

    SET_MOD_RM(kRegister, GetRegisterBase(src_reg), GetRegisterBase(dest_reg));

    SET_INSTRUCTION(kCmpRm64WithR64, 0, 0, kLogicCmpRegisterToRegister);

    ADD_INSTRUCTION(&instruction);

    return kBackendSuccess;
}

//==============================================================================
static BackendErrs_t EncodeLeave(BackendContext *backend_context)
{
    Instruction instruction = {0};

    SET_INSTRUCTION(kLeave, 0, 0, kLogicLeave);

    ADD_INSTRUCTION(&instruction);

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

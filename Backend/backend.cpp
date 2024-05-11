#include <stdio.h>

#include "backend.h"

#include "backend_common.h"
#include "backend_dump.h"

#include "instruction_encoding.h"

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

static BackendErrs_t SetInstruction(Instruction        *instruction,
                                    BackendContext     *backend_context,
                                    uint16_t            op_code,
                                    DisplacementType_t  displacement,
                                    ImmediateType_t     immediate_arg,
                                    LogicalOpcode_t     logical_op_code);

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


static BackendErrs_t InitStringTable(StringTable *strings);

static size_t AddLabel(LanguageContext *language_context,
                       LabelTable      *label_table,
                       size_t           address,
                       int32_t          func_pos,
                       int32_t          identification_number);

static size_t GetCurSize(BackendContext *backend_context);

static BackendErrs_t SetJumpRelativeAddress(Instruction *jump_instruction,
                                            int32_t      label_pos);

static BackendErrs_t RespondAddressRequests(BackendContext *backend_context);

static BackendErrs_t DestroyLabelTable(LabelTable *label_table);

static BackendErrs_t PassFuncArgs(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  TreeNode        *cur_node,
                                  TableOfNames    *cur_table);

static BackendErrs_t InitAddressRequests(AddressRequests *address_requests);

static BackendErrs_t DestroyAddressRequests(AddressRequests *address_requests);


static BackendErrs_t AddAddressRequest(AddressRequests *address_requests,
                                       size_t           jmp_instruction_list_pos,
                                       int32_t          func_pos,
                                       int32_t          label_identifier);

static BackendErrs_t ReallocAddressRequests(AddressRequests *address_requests,
                                            size_t           new_size);

static BackendErrs_t SetCallRelativeAddress(Instruction *call_instruction,
                                            uint32_t     address);

static int32_t AddLabelIdentifier(BackendContext *backend_context);

static BackendErrs_t AddString(StringTable *strings,
                               char        *str);

static BackendErrs_t ReallocStringTable(StringTable *strings,
                                        size_t       new_size);

static BackendErrs_t ReallocSymbolTable(SymbolTable *sym_table,
                                        size_t       new_size);

static BackendErrs_t InitRelocationTable   (RelocationTable *relocation_table);
static BackendErrs_t DestroyRelocationTable(RelocationTable *relocation_table);
static BackendErrs_t ReallocRelocationTable(RelocationTable *relocation_table,
                                            size_t           new_size);
static BackendErrs_t AddRelocation         (RelocationTable *relocation_table,
                                            Elf64_Rela      *relocation);

//==============================================================================

static BackendErrs_t InitRelocationTable(RelocationTable *relocation_table)
{

    relocation_table->relocation_array = (Elf64_Rela *) calloc(relocation_table->capacity, sizeof(Elf64_Rela));

    relocation_table->capacity = kBaseRelocationTableCapacity;
    relocation_table->relocation_count = 0;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t DestroyRelocationTable(RelocationTable *relocation_table)
{
    free(relocation_table->relocation_array);

    relocation_table->capacity = 0;

    relocation_table->relocation_count = 0;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AddRelocation(RelocationTable *relocation_table,
                                   Elf64_Rela      *relocation)
{
    if (relocation_table->relocation_count >= relocation_table->capacity)
    {
        ReallocRelocationTable(relocation_table, relocation_table->capacity * 2);
    }

    relocation_table->relocation_array[relocation_table->relocation_count] = *relocation;

    relocation_table->relocation_count += 1;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t ReallocRelocationTable(RelocationTable *relocation_table,
                                            size_t           new_size)
{
    relocation_table->capacity = new_size;

    relocation_table->relocation_array = (Elf64_Rela *) realloc(relocation_table->relocation_array,
                                                                sizeof(Elf64_Rela) * relocation_table->capacity);
    if (relocation_table->relocation_array == nullptr)
    {
        ColorPrintf(kRed, "%s() failed reallocation\n", __func__);

        return kBackendFailedAllocation;
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t InitSymbolTable(SymbolTable *sym_table)
{
    sym_table->capacity = kBaseSymbolTableCapacity;

    sym_table->sym_array = (Elf64_Sym *) calloc(sym_table->capacity, sizeof(Elf64_Sym));

    if (sym_table->sym_array == nullptr)
    {
        return kBackendFailedAllocation;
    }

    sym_table->sym_count = 0;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t DestroySymbolTable(SymbolTable *sym_table)
{
    free(sym_table->sym_array);

    sym_table->sym_array = nullptr;

    sym_table->capacity = 0;

    sym_table->sym_count = 0;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AddSymbol(SymbolTable *sym_table,
                               Elf64_Word   string_table_name,
                               unsigned char info,
                               unsigned char visibility,
                               Elf64_Section section_index,
                               Elf64_Addr    value,
                               Elf64_Xword   symbol_size)
{
    if (sym_table->sym_count >= sym_table->capacity)
    {
        ReallocSymbolTable(sym_table, sym_table->capacity * 2);
    }

    sym_table->sym_array[sym_table->sym_count].st_name  = string_table_name;
    sym_table->sym_array[sym_table->sym_count].st_info  = info;
    sym_table->sym_array[sym_table->sym_count].st_other = visibility;
    sym_table->sym_array[sym_table->sym_count].st_shndx = section_index;
    sym_table->sym_array[sym_table->sym_count].st_value = value;
    sym_table->sym_array[sym_table->sym_count].st_size  = symbol_size;

    sym_table->sym_count += 1;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t ReallocSymbolTable(SymbolTable *sym_table,
                                        size_t       new_size)
{
    sym_table->capacity = new_size;

    sym_table->sym_array = (Elf64_Sym *) realloc(sym_table->sym_array, sym_table->capacity);

    for (size_t i = sym_table->sym_count; i < sym_table->capacity; i++)
    {
        sym_table->sym_array[i] = {0};
    }

    return kBackendSuccess;
}

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

BackendErrs_t AddFuncLabelRequest(BackendContext *backend_context,
                                  size_t          jump_instruction_list_pos,
                                  int32_t         func_pos)
{
    return AddAddressRequest(backend_context->address_requests,
                             jump_instruction_list_pos,
                             func_pos,
                             kCommonLabelIdentifierPoison);
}

//==============================================================================

BackendErrs_t AddCommonLabelRequest(BackendContext *backend_context,
                                    size_t          jump_instruction_list_pos,
                                    int32_t         identification_number)
{
    return AddAddressRequest(backend_context->address_requests,
                             jump_instruction_list_pos,
                             kFuncLabelPosPoison,
                             identification_number);
}

//==============================================================================

static BackendErrs_t AddAddressRequest(AddressRequests *address_requests,
                                       size_t           jmp_instruction_list_pos,
                                       int32_t          func_pos,
                                       int32_t          label_identifier)
{
    if (address_requests->request_count >= address_requests->capacity)
    {
        ReallocAddressRequests(address_requests, 2 * address_requests->capacity);
    }

    address_requests->requests[address_requests->request_count].jmp_instruction_list_pos = jmp_instruction_list_pos;
    address_requests->requests[address_requests->request_count].func_pos                 = func_pos;
    address_requests->requests[address_requests->request_count++].label_identifier       = label_identifier;

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
        address_requests->requests[i].jmp_instruction_list_pos  = 0;
        address_requests->requests[i].func_pos                  = kFuncLabelPosPoison;
        address_requests->requests[i].label_identifier          = kCommonLabelIdentifierPoison;
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t RespondAddressRequests(BackendContext *backend_context)
{
    for (size_t i = 0; i < backend_context->address_requests->request_count; i++)
    {
        for (size_t j = 0; j < backend_context->label_table->label_count; j++)
        {
            if ((backend_context->address_requests->requests[i].func_pos ==
                 backend_context->label_table->label_array[j].func_pos) &&
                (backend_context->address_requests->requests[i].label_identifier ==
                 backend_context->label_table->label_array[j].identification_number))
            {
                SetJumpRelativeAddress(&backend_context->instruction_list->data[backend_context->address_requests->requests[i].jmp_instruction_list_pos],
                                        backend_context->label_table->label_array[j].address);
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

    label_table->identify_counter = 0;

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

static size_t AddLabel(LanguageContext *language_context,
                       LabelTable      *label_table,
                       size_t           address,
                       int32_t          func_pos,
                       int32_t          identification_number)
{
    if (label_table->label_count >= label_table->capacity)
    {
        ReallocLabelTable(label_table, label_table->capacity * 2);
    }

    label_table->label_array[label_table->label_count].address    = address;
    label_table->label_array[label_table->label_count].func_pos = func_pos;

    label_table->label_array[label_table->label_count++].identification_number = identification_number;

    if (identification_number != kCommonLabelIdentifierPoison)
    {
        DumpPrintCommonLabel(identification_number);
    }
    else if (func_pos != kFuncLabelPosPoison)
    {
        BackendDumpPrintFuncLabel(language_context, func_pos);
    }

    return label_table->label_count - 1;
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

    backend_context->strings = (StringTable *) calloc(1, sizeof(StringTable));

    if (InitStringTable(backend_context->strings) != kBackendSuccess)
    {
        return kBackendFailedAllocation;
    }

    backend_context->symbol_table = (SymbolTable *) calloc(1, sizeof(SymbolTable));

    if (InitSymbolTable(backend_context->symbol_table) != kBackendSuccess)
    {
        return kBackendFailedAllocation;
    }

    backend_context->relocation_table = (RelocationTable *) calloc(1, sizeof(RelocationTable));

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t InitStringTable(StringTable *strings)
{
    strings->capacity = kBaseStringTableCapacity;

    strings->cur_size = 0;

    strings->byte_array = (char *) calloc(strings->capacity, sizeof(char));

    if (strings->byte_array == nullptr)
    {
        return kBackendFailedAllocation;
    }

    strings->cur_size += 1;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t AddString(StringTable *strings,
                               char        *str)
{
    size_t size = strlen(str) + 1;

    if (strings->cur_size + size >= strings->capacity)
    {
        ReallocStringTable(strings, strings->capacity * 2);
    }

    sprintf(strings->byte_array + strings->cur_size, "%s", str);

    strings->cur_size += size;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t ReallocStringTable(StringTable *strings,
                                        size_t       new_size)
{
    strings->capacity = new_size;

    strings->byte_array = (char *) realloc(strings->byte_array, strings->capacity);

    if (strings->byte_array == nullptr)
    {
        return kBackendFailedAllocation;
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t DestroyStringTable(StringTable *strings)
{
    free(strings->byte_array);

    strings->byte_array = nullptr;

    strings->capacity = 0;
    strings->cur_size = 0;

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

    DestroyStringTable(backend_context->strings);

    free(backend_context->strings);

    backend_context->strings = nullptr;

    DestroySymbolTable(backend_context->symbol_table);

    free(backend_context->symbol_table);

    backend_context->symbol_table = nullptr;

    free(backend_context->relocation_table);

    return kBackendSuccess;
}


//==============================================================================

static int GetNameTablePos(NameTables *name_tables,
                           int         func_code)
{
    for (size_t i = 0; i < name_tables->tables_count; i++)
    {
        if (name_tables->name_tables[i]->func_code == func_code)
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

    RespondAddressRequests(backend_context);

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

    AddLabel(language_context,
             backend_context->label_table,
             backend_context->cur_address,
             cur_node->data.variable_pos,
             kCommonLabelIdentifierPoison);

    AsmFuncEntry(backend_context,
                 language_context,
                 cur_node,
                 language_context->tables.name_tables[name_table_pos]);

    TreeNode *params_node = cur_node->right;

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

#define PUSH_REGISTER(reg)                                                 EncodePushRegister(backend_context, reg)

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

#define XOR_REGISTER_WITH_REGISTER(source_reg, receiver_reg)               EncodeXorRegisterWithRegister(backend_context, source_reg, receiver_reg)

#define IMUL_ON_REGISTER(receiver_reg)                                     EncodeImulRegister(backend_context, receiver_reg)

#define CMP_REGISTER_TO_IMMEDIATE(dest_reg, immediate)                     EncodeCmpRegisterWithImmediate(backend_context, dest_reg, immediate)
#define CMP_REGISTER_TO_REGISTER(dest_reg, source_reg)                     EncodeCmpRegisterWithRegister(backend_context, dest_reg, source_reg)


#define JUMP_IF_ABOVE(label_id)                                            EncodeJump(backend_context, kLogicJumpIfAbove,        kJaRel32 , label_id)
#define JUMP_IF_ABOVE_OR_EQUAL(label_id)                                   EncodeJump(backend_context, kLogicJumpIfAboveOrEqual, kJaeRel32, label_id)

#define JUMP_IF_LESS_OR_EQUAL(label_id)                                    EncodeJump(backend_context, kLogicJumpIfLessOrEqual,  kJbeRel32, label_id)
#define JUMP_IF_LESS(label_id)                                             EncodeJump(backend_context, kLogicJumpIfLess,         kJbRel32,  label_id)

#define JUMP_IF_EQUAL(label_id)                                            EncodeJump(backend_context, kLogicJumpIfEqual,        kJeRel32,  label_id)
#define JUMP_IF_NOT_EQUAL(label_id)                                        EncodeJump(backend_context, kLogicJumpIfNotEqual,     kJneRel32, label_id)

#define JUMP(label_id)                                                     EncodeJump(backend_context, kLogicJmp,                kJmpRel32, label_id)


#define CALL(func_pos)                                                     EncodeCall(backend_context, language_context, func_pos)

#define AND(dest_reg, source_reg)                                          EncodeRegisterAndRegister(backend_context, dest_reg, source_reg);

#define OR(dest_reg, source_reg)                                           EncodeRegisterOrRegister(backend_context, dest_reg, source_reg)
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

static int32_t AddLabelIdentifier(BackendContext *backend_context)
{
    int32_t label_identifier = backend_context->label_table->identify_counter++;

    return label_identifier;
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

                int32_t cur_identify = AddLabelIdentifier(backend_context);

                JUMP_IF_LESS_OR_EQUAL(cur_identify);

                AddCommonLabelRequest(backend_context,
                                      backend_context->instruction_list->tail,
                                      cur_identify);

                ASM_OPERATOR(cur_node->right);

                size_t label_pos = 0;

                AddLabel(language_context,
                         backend_context->label_table,
                         backend_context->cur_address,
                         kFuncLabelPosPoison,
                         cur_identify);
                break;
            }

#define LOGICAL_OPERATOR_CODE_GEN(const_name, encode_func_name, code)                                           \
            case const_name:                                                                                    \
            {                                                                                                   \
                ASM_OPERATOR(cur_node->right);                                                                  \
                                                                                                                \
                MOV_REGISTER_TO_REGISTER(kRAX, kRBX);                                                           \
                                                                                                                \
                ASM_OPERATOR(cur_node->left);                                                                   \
                                                                                                                \
                code                                                                                            \
                                                                                                                \
                CMP_REGISTER_TO_REGISTER(kRAX, kRBX);                                                           \
                                                                                                                \
                int32_t logic_op_start_label_id = AddLabelIdentifier(backend_context);                          \
                int32_t logic_op_end_label_id   = AddLabelIdentifier(backend_context);                          \
                                                                                                                \
                encode_func_name(logic_op_start_label_id);                                                      \
                                                                                                                \
                int32_t jump_on_start_pos = backend_context->instruction_list->tail;                            \
                                                                                                                \
                JUMP(logic_op_start_label_id);                                                                  \
                                                                                                                \
                int32_t jump_on_end_pos = backend_context->instruction_list->tail;                              \
                                                                                                                \
                size_t start_label_pos = AddLabel(language_context,                                             \
                                                  backend_context->label_table,                                 \
                                                  backend_context->cur_address,                                 \
                                                  kFuncLabelPosPoison,                                          \
                                                  logic_op_start_label_id);                                     \
                MOV_IMM_TO_REGISTER(1, kRAX);                                                                   \
                                                                                                                \
                size_t end_label_pos = AddLabel(language_context,                                               \
                                                backend_context->label_table,                                   \
                                                backend_context->cur_address,                                   \
                                                kFuncLabelPosPoison,                                            \
                                                logic_op_end_label_id);                                         \
                                                                                                                \
                SetJumpRelativeAddress(&backend_context->instruction_list->data[jump_on_start_pos],             \
                                        backend_context->label_table->label_array[start_label_pos].address);    \
                                                                                                                \
                SetJumpRelativeAddress(&backend_context->instruction_list->data[jump_on_end_pos],               \
                                        backend_context->label_table->label_array[end_label_pos].address);      \
                                                                                                                \
                break;                                                                                          \
            }

            #include "logical_operators_code.gen.h"

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

    TreeNode *cur_arg_node = cur_node;

    size_t args_count = 0;

    while (cur_arg_node->right != nullptr)
    {
        if (cur_arg_node->left != nullptr)
        {
            cur_arg_node = cur_arg_node->right;

            args_count++;
        }
    }

    if (args_count = 0 && cur_node->left == nullptr)
    {
        return kBackendNullArgs;
    }

    size_t passed_args_count = 0;

    for (; passed_args_count < args_count &&
           passed_args_count < kArgPassingRegisterCount;
           passed_args_count++)
    {
        MOV_REGISTER_TO_REG_MEMORY(ArgPassingRegisters[passed_args_count],
                                   kRBP,
                                   (passed_args_count + 1) * (-8));
    }

    if (passed_args_count < kArgPassingRegisterCount)
    {
        return kBackendSuccess;
    }

    while (passed_args_count < args_count)
    {
        MOV_REG_MEMORY_TO_REGISTER(kRBP, (passed_args_count - kArgPassingRegisterCount + 3) * 8, kRAX);

        MOV_REGISTER_TO_REG_MEMORY(kRAX, kRBP, (passed_args_count + 1) * (-8));
    }

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

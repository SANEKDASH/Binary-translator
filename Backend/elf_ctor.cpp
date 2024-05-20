#include "elf_ctor.h"

#include "instruction_encoding.h"

static BackendErrs_t SetRelocatableFileHeader(RelocatableFile *rel_file);

static BackendErrs_t WriteElf(BackendContext  *backend_context,
                              LanguageContext *language_context,
                              RelocatableFile *rel_file,
                              FILE            *output_file);

static BackendErrs_t WriteElfHeadersInFile(RelocatableFile *rel_file,
                                           FILE            *output_file);

static BackendErrs_t SetSectionHeaders(BackendContext  *backend_context,
                                       LanguageContext *language_context,
                                       RelocatableFile *rel_file);

static BackendErrs_t SetSectionHeader(Elf64_Shdr *header,
                                      Elf64_Word  header_name_index,
                                      Elf64_Word  header_type,
                                      Elf64_Xword header_flags,
                                      Elf64_Addr  memory_image_address,
                                      Elf64_Off   data_first_byte_offset,
                                      Elf64_Xword data_size,
                                      Elf64_Word  link_table_index,
                                      Elf64_Word  additional_info,
                                      Elf64_Xword address_alignment,
                                      Elf64_Xword entry_size);


static BackendErrs_t SetSectionTextHeader(BackendContext  *backend_context,
                                          LanguageContext *language_context,
                                          RelocatableFile *rel_file);

static BackendErrs_t SetSectionStringTableHeader(BackendContext  *backend_context,
                                                 LanguageContext *language_context,
                                                 RelocatableFile *rel_file);

static BackendErrs_t SetSectionSymbolTableHeader(BackendContext  *backend_context,
                                                 LanguageContext *language_context,
                                                 RelocatableFile *rel_file);

static BackendErrs_t SetSectionHeaderStringTableHeader(BackendContext  *backend_context,
                                                       LanguageContext *language_context,
                                                       RelocatableFile *rel_file);

static BackendErrs_t SetSectionRelaTextHeader(BackendContext  *backend_context,
                                              LanguageContext *language_context,
                                              RelocatableFile *rel_file);

static BackendErrs_t WriteSectionTextData(BackendContext *backend_context,
                                          FILE           *output_file);

static BackendErrs_t WriteSectionHeaderStringTableData(FILE *output_file);

static BackendErrs_t WriteInstruction(BackendContext *backend_context,
                                      Instruction    *instruction,
                                      uint8_t        *instruction_buffer,
                                      size_t         *buffer_pos);

static BackendErrs_t WriteSectionSymbolTableData(SymbolTable *symbol_table,
                                                 FILE        *output_file);

static BackendErrs_t WriteDataAlign(size_t  data_size,
                                    FILE   *output_file);

static BackendErrs_t WriteSectionStringTableData(StringTable *strings,
                                                 FILE        *output_file);

static BackendErrs_t WriteSectionRelaTextData(RelocationTable *rel_table,
                                              FILE            *output_file);

static size_t GetAlignedSize(size_t data_size);

static size_t GetLastLocalSymbolIndex(SymbolTable* sym_table);

static BackendErrs_t ChangeRelocationSymbolIndex(RelocationTable *relocation_table,
                                                 Elf64_Rela      *new_relocation_table_array,
                                                 size_t          *pos,
                                                 size_t           old_index,
                                                 size_t           new_index);

static BackendErrs_t SortSymbolTable(SymbolTable     *symbol_table,
                                     RelocationTable *relocation_table);

//==============================================================================

BackendErrs_t CreateElfRelocatableFile(BackendContext  *backend_context,
                                       LanguageContext *language_context,
                                       const char      *file_name)
{
    CHECK(backend_context);
    CHECK(language_context);

    RelocatableFile rel_file = {0};

    SortSymbolTable(backend_context->symbol_table,
                    backend_context->relocation_table);

    InitRelocatableFile(backend_context, language_context, &rel_file);

    FILE *output_file = fopen(file_name, "wb");

    if (output_file == nullptr)
    {
        ColorPrintf(kRed, "%s() failed to open file\n", __func__);

        return kBackendFailedToOpenFile;
    }

    WriteElf(backend_context, language_context, &rel_file, output_file);

    fclose(output_file);

    return kBackendSuccess;
}

//==============================================================================

static const char kNullByte = 0;

static const uint64_t kNullAddress              = 0;
static const uint64_t kNullPadding              = 0;
static const uint16_t kFileHeaderSize           = 64;
static const uint16_t kProgramHeaderSize        = 56;
static const uint16_t kSectionHeaderSize        = 64;
static const uint16_t kSectionHeaderCount       = 6;
static const uint16_t kSectionStringHeaderIndex = 2;

//==============================================================================

BackendErrs_t InitRelocatableFile(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  RelocatableFile *rel_file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(rel_file);

    SetRelocatableFileHeader(rel_file);

    SetSectionHeaders(backend_context,
                      language_context,
                      rel_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SortSymbolTable(SymbolTable     *symbol_table,
                                     RelocationTable *relocation_table)
{
    CHECK(symbol_table);
    CHECK(relocation_table);

    Elf64_Sym *new_sym_array = (Elf64_Sym *) calloc(symbol_table->capacity, sizeof(Elf64_Sym));

    Elf64_Rela *new_rel_table_array = (Elf64_Rela *) calloc(relocation_table->capacity, sizeof(Elf64_Rela));

    size_t new_rel_table_array_cur_pos = 0;

    size_t new_sym_table_cur_pos = 0;

    for (size_t i = 0; i < symbol_table->sym_count; i++)
    {
        if (ELF64_ST_BIND(symbol_table->sym_array[i].st_info) == STB_LOCAL)
        {
            new_sym_array[new_sym_table_cur_pos] = symbol_table->sym_array[i];

            ChangeRelocationSymbolIndex(relocation_table, new_rel_table_array, &new_rel_table_array_cur_pos, i, new_sym_table_cur_pos);

            new_sym_table_cur_pos++;
        }
    }

    for (size_t i = 0; i < symbol_table->sym_count; i++)
    {
        if (ELF64_ST_BIND(symbol_table->sym_array[i].st_info) != STB_LOCAL)
        {
            new_sym_array[new_sym_table_cur_pos] = symbol_table->sym_array[i];

            ChangeRelocationSymbolIndex(relocation_table, new_rel_table_array, &new_rel_table_array_cur_pos, i, new_sym_table_cur_pos);

            new_sym_table_cur_pos++;
        }
    }

    if (new_sym_table_cur_pos != symbol_table->sym_count)
    {
        ColorPrintf(kRed, "%s() failed sort\n", __func__);

        free(new_sym_array);
    }

    if (new_rel_table_array_cur_pos != relocation_table->relocation_count)
    {
        ColorPrintf(kRed, "%s() failed to sort fucking relocations\n", __func__);
    }

    free(symbol_table->sym_array);

    symbol_table->sym_array = new_sym_array;

    free(relocation_table->relocation_array);

    relocation_table->relocation_array = new_rel_table_array;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t ChangeRelocationSymbolIndex(RelocationTable *relocation_table,
                                                 Elf64_Rela      *new_relocation_table_array,
                                                 size_t          *pos,
                                                 size_t           old_index,
                                                 size_t           new_index)
{
    for (size_t i = 0; i < relocation_table->relocation_count; i++)
    {
        if (ELF64_R_SYM(relocation_table->relocation_array[i].r_info) == old_index)
        {
            new_relocation_table_array[*pos] = relocation_table->relocation_array[i];

            new_relocation_table_array[(*pos)++].r_info = ELF64_R_INFO(new_index, ELF64_R_TYPE(relocation_table->relocation_array[i].r_info));
        }
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetRelocatableFileHeader(RelocatableFile *rel_file)
{
    CHECK(rel_file);

    rel_file->header.e_ident[EI_MAG0]    = ELFMAG0;
    rel_file->header.e_ident[EI_MAG1]    = ELFMAG1;
    rel_file->header.e_ident[EI_MAG2]    = ELFMAG2;
    rel_file->header.e_ident[EI_MAG3]    = ELFMAG3;
    rel_file->header.e_ident[EI_CLASS]   = ELFCLASS64;
    rel_file->header.e_ident[EI_DATA]    = ELFDATA2LSB;
    rel_file->header.e_ident[EI_VERSION] = EV_CURRENT;
    rel_file->header.e_ident[EI_OSABI]   = ELFOSABI_NONE;

    for (size_t i = EI_OSABI + 1; i < sizeof(rel_file->header.e_ident); i++)
    {
        rel_file->header.e_ident[i] = kNullByte;
    }

    rel_file->header.e_type    = ET_REL;    // file type - relocatable
    rel_file->header.e_machine = EM_X86_64; // machine type
    rel_file->header.e_version = EV_CURRENT; // format version

    rel_file->header.e_entry = kNullAddress;
    rel_file->header.e_phoff = kNullPadding;

    rel_file->header.e_shoff = kFileHeaderSize;

    rel_file->header.e_flags  = 0;
    rel_file->header.e_ehsize = sizeof(rel_file->header);

    rel_file->header.e_phentsize = 0; //because there is no program headers
    rel_file->header.e_phnum     = 0; // same reason
    rel_file->header.e_shentsize = kSectionHeaderSize;
    rel_file->header.e_shnum     = kSectionHeaderCount;
    rel_file->header.e_shstrndx  = kSectionStringHeaderIndex;

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteElf(BackendContext  *backend_context,
                              LanguageContext *language_context,
                              RelocatableFile *rel_file,
                              FILE            *output_file)
{
    CHECK(rel_file);
    CHECK(output_file);

    WriteElfHeadersInFile(rel_file, output_file);

    WriteSectionTextData             (backend_context, output_file);
    WriteSectionHeaderStringTableData(output_file);
    WriteSectionSymbolTableData      (backend_context->symbol_table, output_file);
    WriteSectionStringTableData      (backend_context->strings, output_file);
    WriteSectionRelaTextData         (backend_context->relocation_table, output_file);

    return kBackendSuccess;
}

static const size_t kAlignSize = 16;

//==============================================================================

static BackendErrs_t WriteSectionRelaTextData(RelocationTable *rel_table,
                                              FILE            *output_file)
{
    fwrite(rel_table->relocation_array, sizeof(Elf64_Rela), rel_table->relocation_count, output_file);

    WriteDataAlign(rel_table->relocation_count * sizeof(Elf64_Rela), output_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteSectionStringTableData(StringTable *strings,
                                                 FILE        *output_file)
{
    for (size_t i = 0; i < strings->string_count; i++)
    {
        fwrite(strings->string_array[i], sizeof(char), strlen(strings->string_array[i]) + 1, output_file);
    }

    WriteDataAlign(strings->cur_size, output_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteSectionSymbolTableData(SymbolTable *symbol_table,
                                                 FILE        *output_file)
{
    fwrite(symbol_table->sym_array, sizeof(Elf64_Sym), symbol_table->sym_count, output_file);


    WriteDataAlign(sizeof(Elf64_Sym) * symbol_table->sym_count, output_file);

    return kBackendSuccess;
}

//==============================================================================

static size_t GetAlignedSize(size_t data_size)
{
    if (data_size % kAlignSize == 0)
    {
        return data_size;
    }

    return data_size + kAlignSize - (data_size % kAlignSize);
}

//==============================================================================

static BackendErrs_t WriteDataAlign(size_t  data_size,
                                    FILE   *output_file)
{
    size_t align_size = kAlignSize - ftell(output_file) % kAlignSize;

    if (align_size == kAlignSize)
    {
        return kBackendSuccess;
    }

    uint8_t *null_array = (uint8_t *) calloc(align_size, sizeof(uint8_t));

    fwrite(null_array, sizeof(uint8_t), align_size, output_file);

    free(null_array);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteSectionHeaderStringTableData(FILE *output_file)
{
    fwrite(HeaderStringTable, sizeof(char), kHeaderStringTableSize, output_file);

    WriteDataAlign(kHeaderStringTableSize, output_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteSectionTextData(BackendContext *backend_context,
                                          FILE           *output_file)
{
    CHECK(backend_context);
    CHECK(output_file);

    uint8_t *instruction_buffer = (uint8_t *) calloc(backend_context->cur_address, sizeof(uint8_t));

    size_t cur_buffer_pos = 0;

    size_t cur_node_pos = backend_context->instruction_list->next[backend_context->instruction_list->head];

    while (cur_node_pos != backend_context->instruction_list->head)
    {
        WriteInstruction(backend_context,
                         &backend_context->instruction_list->data[cur_node_pos],
                         instruction_buffer,
                         &cur_buffer_pos);

        cur_node_pos = backend_context->instruction_list->next[cur_node_pos];
    }

    if (cur_buffer_pos != backend_context->cur_address)
    {
        ColorPrintf(kRed, "%s() inconsistent sizes\n", __func__);

        return kBackendInconsistentSizes;
    }

    fwrite(instruction_buffer, sizeof(uint8_t), cur_buffer_pos, output_file);

    free(instruction_buffer);

    WriteDataAlign(cur_buffer_pos, output_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteInstruction(BackendContext *backend_context,
                                      Instruction    *instruction,
                                      uint8_t        *instruction_buffer,
                                      size_t         *buffer_pos)
{
    CHECK(backend_context);
    CHECK(instruction);

    if (instruction->rex_prefix != 0)
    {
        *(uint8_t *) (instruction_buffer + *buffer_pos) = instruction->rex_prefix;

        *buffer_pos += sizeof(instruction->rex_prefix);
    }

    if (instruction->op_code_size == 1)
    {
        *(uint8_t *) (instruction_buffer + *buffer_pos) = (uint8_t) instruction->op_code;
    }
    else if (instruction->op_code_size == 2)
    {
        *(uint16_t *) (instruction_buffer + *buffer_pos) = instruction->op_code;
    }
    else
    {
        ColorPrintf(kRed, "%s() unknown opcode size\n", __func__);

        return kBackendUnknownOpcodeSize;
    }

    *buffer_pos += instruction->op_code_size;

    if (instruction->mod_rm != 0)
    {
        *(uint8_t *) (instruction_buffer + *buffer_pos) = instruction->mod_rm;

        *buffer_pos += sizeof(instruction->mod_rm);
    }

    if (instruction->immediate_size != 0)
    {
        *(ImmediateType_t *) (instruction_buffer + *buffer_pos) = instruction->immediate_arg;

        *buffer_pos += instruction->immediate_size;
    }

    if (instruction->displacement_size != 0)
    {
        *(DisplacementType_t *) (instruction_buffer + *buffer_pos) = instruction->displacement;

        *buffer_pos += instruction->displacement_size;
    }

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteElfHeadersInFile(RelocatableFile *rel_file,
                                           FILE            *output_file)
{
    CHECK(rel_file);
    CHECK(output_file);

    fwrite(rel_file, sizeof(RelocatableFile), 1, output_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetSectionHeaders(BackendContext  *backend_context,
                                       LanguageContext *language_context,
                                       RelocatableFile *rel_file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(rel_file);

    SetSectionTextHeader             (backend_context, language_context, rel_file);
    SetSectionHeaderStringTableHeader(backend_context, language_context, rel_file);
    SetSectionSymbolTableHeader      (backend_context, language_context, rel_file);
    SetSectionStringTableHeader      (backend_context, language_context, rel_file);
    SetSectionRelaTextHeader         (backend_context, language_context, rel_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetSectionTextHeader(BackendContext  *backend_context,
                                          LanguageContext *language_context,
                                          RelocatableFile *file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(file);

    SetSectionHeader(&file->section_text_header,
                     kSectionTextTableIndex,
                     SHT_PROGBITS,
                     SHF_ALLOC | SHF_EXECINSTR,
                     kNullAddress,
                     sizeof(RelocatableFile),
                     backend_context->cur_address,
                     0,
                     0,
                     16,
                     0);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetSectionHeaderStringTableHeader(BackendContext  *backend_context,
                                                       LanguageContext *language_context,
                                                       RelocatableFile *file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(file);

    SetSectionHeader(&file->section_header_string_table_header,
                     kSectionHeaderStringTableIndex,
                     SHT_STRTAB,
                     0,
                     kNullAddress,
                     file->section_text_header.sh_offset + GetAlignedSize(file->section_text_header.sh_size),
                     sizeof(HeaderStringTable),
                     0,
                     0,
                     1,
                     0);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetSectionSymbolTableHeader(BackendContext  *backend_context,
                                                 LanguageContext *language_context,
                                                 RelocatableFile *file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(file);

    SetSectionHeader(&file->section_symbol_table_header,
                     kSectionSymbolTableIndex,
                     SHT_SYMTAB,
                     0,
                     kNullAddress,
                     file->section_header_string_table_header.sh_offset + GetAlignedSize(file->section_header_string_table_header.sh_size),
                     backend_context->symbol_table->sym_count * sizeof(Elf64_Sym),
                     4,
                     GetLastLocalSymbolIndex(backend_context->symbol_table),
                     0x8,
                     sizeof(Elf64_Sym));



    return kBackendSuccess;
}

//==============================================================================

static size_t GetLastLocalSymbolIndex(SymbolTable* sym_table)
{
    size_t cur_index = 0;

    while ((cur_index < sym_table->sym_count) && (ELF64_ST_BIND(sym_table->sym_array[cur_index].st_info) == STB_LOCAL))
    {
        cur_index++;
    }

    return cur_index;
}

//==============================================================================

static BackendErrs_t SetSectionStringTableHeader(BackendContext  *backend_context,
                                                 LanguageContext *language_context,
                                                 RelocatableFile *file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(file);

    SetSectionHeader(&file->section_string_table_header,
                     kSectionStringTableIndex,
                     SHT_STRTAB,
                     0,
                     kNullAddress,
                     file->section_symbol_table_header.sh_offset + GetAlignedSize(file->section_symbol_table_header.sh_size),
                     backend_context->strings->cur_size,
                     0,
                     0,
                     1,
                     0);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetSectionRelaTextHeader(BackendContext  *backend_context,
                                              LanguageContext *language_context,
                                              RelocatableFile *file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(file);

    SetSectionHeader(&file->section_rela_rext_header,
                     kSectionRelaTextTableIndex,
                     SHT_RELA,
                     0,
                     kNullAddress,
                     file->section_string_table_header.sh_offset + GetAlignedSize(file->section_string_table_header.sh_size),
                     backend_context->relocation_table->relocation_count * sizeof(Elf64_Rela),
                     3,
                     1,
                     0x8,
                     sizeof(Elf64_Rela));

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t SetSectionHeader(Elf64_Shdr *header,
                                      Elf64_Word  header_name_index,
                                      Elf64_Word  header_type,
                                      Elf64_Xword header_flags,
                                      Elf64_Addr  memory_image_address,
                                      Elf64_Off   data_first_byte_offset,
                                      Elf64_Xword data_size,
                                      Elf64_Word  link_table_index,
                                      Elf64_Word  additional_info,
                                      Elf64_Xword address_alignment,
                                      Elf64_Xword entry_size)
{
    CHECK(header);

    header->sh_name      = header_name_index;
    header->sh_type      = header_type;
    header->sh_flags     = header_flags;
    header->sh_addr      = memory_image_address;
    header->sh_offset    = data_first_byte_offset;
    header->sh_size      = data_size;
    header->sh_link      = link_table_index;
    header->sh_info      = additional_info;
    header->sh_addralign = address_alignment;
    header->sh_entsize   = entry_size;

    return kBackendSuccess;
}

//==============================================================================

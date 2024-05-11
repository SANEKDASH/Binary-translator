#include "elf_ctor.h"

static BackendErrs_t SetRelocatableFileHeader(RelocatableFile *rel_file);

static BackendErrs_t WriteElf(RelocatableFile *rel_file,
                              FILE            *output_file);

static BackendErrs_t WriteElfHeaderInFile(RelocatableFile *rel_file,
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

//==============================================================================

BackendErrs_t CreateElfRelocatableFile(BackendContext  *backend_context,
                                       LanguageContext *language_context,
                                       const char      *file_name)
{
    CHECK(backend_context);
    CHECK(language_context);

    RelocatableFile rel_file = {0};

    InitRelocatableFile(backend_context,
                        language_context,
                        &rel_file);

    FILE *output_file = fopen(file_name, "wb");

    if (output_file == nullptr)
    {
        ColorPrintf(kRed, "%s() failed to open file\n", __func__);

        return kBackendFailedToOpenFile;
    }

    WriteElf(&rel_file, output_file);

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
    rel_file->header.e_ehsize = 0;

    //rel_file->header.e_ehsize    = kFileHeaderSize; // возможно это хуйня из-за которой там пустой хедер

    rel_file->header.e_phentsize = 0; //because there is no pr headers
    rel_file->header.e_phnum     = 0;
    rel_file->header.e_shentsize = kSectionHeaderSize;
    rel_file->header.e_shnum     = kSectionHeaderCount;
    rel_file->header.e_shstrndx  = kSectionStringHeaderIndex;

    return kBackendSuccess;
}

//==============================================================================


static BackendErrs_t WriteElf(RelocatableFile *rel_file,
                              FILE            *output_file)
{
    CHECK(rel_file);
    CHECK(output_file);

    WriteElfHeaderInFile(rel_file, output_file);

    return kBackendSuccess;
}

//==============================================================================

static BackendErrs_t WriteElfHeaderInFile(RelocatableFile *rel_file,
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
    SetSectionStringTableHeader      (backend_context, language_context, rel_file);
    SetSectionSymbolTableHeader      (backend_context, language_context, rel_file);
    SetSectionHeaderStringTableHeader(backend_context, language_context, rel_file);
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

static BackendErrs_t SetSectionStringTableHeader(BackendContext  *backend_context,
                                                 LanguageContext *language_context,
                                                 RelocatableFile *file)
{
    CHECK(backend_context);
    CHECK(language_context);
    CHECK(file);

    SetSectionHeader(&file->section_string_table_header,
                     kSectionSymbolTableIndex,
                     SHT_STRTAB,
                     0,
                     kNullAddress,
                     file->section_symbol_table_header.sh_offset + file->section_symbol_table_header.sh_size /*+ alignment*/,
                     backend_context->strings->cur_size,
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
                     file->section_header_string_table_header.sh_offset +
                     file->section_header_string_table_header.sh_size,
                     backend_context->symbol_table->sym_count * sizeof(Elf64_Sym),
                     0, // hz
                     0, // hz
                     0, // hz
                     sizeof(Elf64_Sym));

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

    SetSectionHeader(&file->section_string_table_header,
                     kSectionHeaderStringTableIndex,
                     SHT_STRTAB,
                     0,
                     kNullAddress,
                     file->section_text_header.sh_offset + file->section_text_header.sh_size/* + alignment*/,
                     sizeof(HeaderStringTable),
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
                     file->section_string_table_header.sh_offset +
                     file->section_string_table_header.sh_size,
                     backend_context->relocation_table->relocation_count * sizeof(Elf64_Rela),
                     0, // ультра хз
                     0, // hz
                     0, // hz
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

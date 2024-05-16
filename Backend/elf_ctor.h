#ifndef ELF_CTOR_HEADER
#define ELF_CTOR_HEADER

#include <elf.h>
#include "backend.h"

typedef struct
{
    Elf64_Ehdr header;

    Elf64_Shdr garbage_section_header;
    Elf64_Shdr section_text_header;
    Elf64_Shdr section_header_string_table_header;
    Elf64_Shdr section_symbol_table_header;
    Elf64_Shdr section_string_table_header;
    Elf64_Shdr section_rela_rext_header;
} __attribute__((packed)) RelocatableFile;

static const char HeaderStringTable[] = {'\0','.', 't', 'e', 'x', 't', '\0',
                                              '.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', '\0',
                                              '.', 's', 'y', 'm', 't', 'a', 'b', '\0',
                                              '.', 's', 't', 'r', 't', 'a', 'b', '\0',
                                              '.', 'r', 'e', 'l', 'a', '.', 't', 'e', 'x', 't', '\0'};

static const size_t kHeaderStringTableSize = sizeof(HeaderStringTable);

static const size_t kSectionTextTableIndex         = 1;
static const size_t kSectionHeaderStringTableIndex = 7;
static const size_t kSectionSymbolTableIndex       = 17;
static const size_t kSectionStringTableIndex       = 25;
static const size_t kSectionRelaTextTableIndex     = 33;


static const size_t kSectionTextIndex = 1;

BackendErrs_t CreateElfRelocatableFile(BackendContext  *backend_context,
                                       LanguageContext *language_context,
                                       const char      *file_name);

BackendErrs_t InitRelocatableFile(BackendContext  *backend_context,
                                  LanguageContext *language_context,
                                  RelocatableFile *rel_file);

#endif

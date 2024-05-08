#ifndef ELF_CTOR_HEADER
#define ELF_CTOR_HEADER

#include <elf.h>
#include "backend.h"

BackendErrs_t CreateElfRelocatableFile(BackendContext  *backend_context,
                                       LanguageContext *language_context);

#endif

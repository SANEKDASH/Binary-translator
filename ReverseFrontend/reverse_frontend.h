#ifndef REVERSE_FRONTEND_HEADER
#define REVERSE_FRONTEND_HEADER

#include "../Backend/backend.h"
#include "../Common/tree_dump.h"
#include "../Common/trees.h"
#include "../Common/NameTable.h"

TreeErrs_t ReverseFrontend(LanguageContext *language_context,
                           const char    *output_file_name);

#endif

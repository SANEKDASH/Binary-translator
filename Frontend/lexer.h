#ifndef LEXER_HEADER
#define LEXER_HEADER

#include <locale.h>

#include "../TextParse/text_parse.h"
#include "../Stack/stack.h"
#include "../Common/trees.h"

typedef enum
{
    kLexerSuccess,
    kSyntaxError,
    kCommentLine,
} LexerErrs_t;

LexerErrs_t SplitOnLexems(Text *text,
                          Stack *tokens,
                          Identifiers *identifiers);

#endif

#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "../Common/trees.h"
#include "../Stack/stack.h"
#include "../Common/NameTable.h"

static LexerErrs_t GetLexem(Stack          *tokens,
                            Expr           *expr,
                            Identifiers *identifiers);

static TreeNode *GetIdentifier    (Expr           *expr,
                                      Identifiers *identifiers);
static TreeNode *GetUnknownExpression(Expr           *expr,
                                      Identifiers *identifiers);

static TreeNode *GetKeyword(Expr *expr);
static TreeNode *GetNumber (Expr *expr);

static void SkipExprSpaces(Expr *expr);

static bool CyrillicIsalpha(Expr *expr);
static bool IsCyrillicLowerByte(char byte);

static bool IsOperatorSymbol(char symbol);

static bool IsRegularExpression(Expr *expr);

static int MissComment(char *string);


//==============================================================================


LexerErrs_t SplitOnLexems(Text        *text,
                          Stack       *tokens,
                          Identifiers *identifiers)
{
    setlocale(LC_ALL, "en_US.utf8");

    for (size_t i = 0; i < text->lines_count; i++)
    {
        Expr expr;

        expr.pos         = 0;
        expr.string      = text->lines_ptr[i].str;
        expr.line_number = text->lines_ptr[i].real_line_number;

        printf("huy %d - [%s]\n", i, expr.string);

        MissComment(expr.string);
        SkipExprSpaces(&expr);

        while (expr.string[expr.pos] != '\0')
        {
            LexerErrs_t lexer_status = GetLexem(tokens, &expr, identifiers);

            if (lexer_status == kSyntaxError)
            {
                printf("Syntax error. POS: %d, LINE: %d\n", expr.pos + 1, i + 1);

                return kSyntaxError;
            }
        }
    }


    return kLexerSuccess;
}

//==============================================================================

static int MissComment(char *string)
{
    char *comment = strchr(string, '#');

    if (comment != nullptr)
    {
        printf("HUY\n");
        *comment = '\0';
    }

    return 0;
}

//==============================================================================

#define OP_CTOR(op_code) NodeCtor(nullptr, nullptr, nullptr, kOperator,      op_code)
#define NUM_CTOR(val)    NodeCtor(nullptr, nullptr, nullptr, kConstNumber,   val)
#define ID_CTOR(pos)     NodeCtor(nullptr, nullptr, nullptr, kIdentifier, pos)

#define CUR_CHAR expr->string[expr->pos]
#define STRING   expr->string
#define POS      expr->pos
#define CUR_STR  expr->string + expr->pos

#define D_PR
//printf("LINE: %d, STRING[%s], POS: %d\n", __LINE__, CUR_STR, POS);
//==============================================================================

static LexerErrs_t GetLexem(Stack          *tokens,
                            Expr           *expr,
                            Identifiers *identifiers)
{
    TreeNode *node = GetKeyword(expr);

    if (node == nullptr)
    {
        node = GetUnknownExpression(expr, identifiers);
    }

    node->line_number = expr->line_number;

    if (node == nullptr)
    {
        return kSyntaxError;
    }

    Push(tokens, node);

    return kLexerSuccess;
}

//==============================================================================

static TreeNode *GetUnknownExpression(Expr           *expr,
                                      Identifiers *identifiers)
{
    TreeNode *node = nullptr;

    if (isdigit(CUR_CHAR) || (CUR_CHAR == '-'))
    {
D_PR
        node = GetNumber(expr);
D_PR

    }
    else if (isalpha(CUR_CHAR) || CyrillicIsalpha(expr))
    {
        node = GetIdentifier(expr, identifiers);
    }

    SkipExprSpaces(expr);

    return node;
}

//==============================================================================

static void SkipExprSpaces(Expr *expr)
{
    while (isspace(CUR_CHAR))
    {
        (POS)++;
    }
}

//==============================================================================

TreeNode *GetKeyword(Expr *expr)
{
    CHECK(expr);

    for (size_t i = 0; i < kKeyWordCount; i++)
    {
        if ((memcmp(CUR_STR, NameTable[i].key_word, NameTable[i].word_len)) == 0 && (*(CUR_STR + NameTable[i].word_len) == '\0' ||
                                                                                     *(CUR_STR + NameTable[i].word_len) == ' ')   )
        {
            POS += NameTable[i].word_len;

            SkipExprSpaces(expr);

            return OP_CTOR(NameTable[i].key_code);
        }
    }

    SkipExprSpaces(expr);

    return nullptr;
}

//==============================================================================

static const size_t kMaxIdentifierNameSize = 256;
static const size_t kSizeOfCyrillicWchar      = 2;

static TreeNode *GetIdentifier(Expr           *expr,
                                  Identifiers *identifiers)
{
    CHECK(expr);
    CHECK(identifiers);

    TreeNode *Identifier_node = nullptr;

    const char *Identifier_start_ptr = CUR_STR;

    char Identifier_name[kMaxIdentifierNameSize] = {0};

    size_t Identifier_name_length = 0;

    while(!isspace(CUR_CHAR) && CUR_CHAR != '\0' && !IsOperatorSymbol(CUR_CHAR))
    {
        ++Identifier_name_length;

        ++POS;
    }

    memcpy(Identifier_name, Identifier_start_ptr, Identifier_name_length);

    int Identifier_pos = SeekIdentifier(identifiers, Identifier_name);

    if (Identifier_pos == -1)
    {
        Identifier_pos = AddIdentifier(identifiers, strdup(Identifier_name));
    }

    Identifier_node = ID_CTOR(Identifier_pos);

    SkipExprSpaces(expr);

    return Identifier_node;
}

//==============================================================================

static TreeNode *GetNumber(Expr *expr)
{
    CHECK(expr);

    TreeNode *node = nullptr;

    char *number_end = nullptr;

    double number = strtod(CUR_STR, &number_end);

    if (CUR_STR == number_end)
    {
        return nullptr;
    }

    node = NUM_CTOR(number);

    POS = number_end - STRING;

    SkipExprSpaces(expr);

    return node;
}

//==============================================================================

static bool CyrillicIsalpha(Expr *expr)
{
    CHECK(expr);

    wchar_t wide_symbol = 0;

    mbtowc(&wide_symbol, CUR_STR, kSizeOfCyrillicWchar);

    char *wide_symbol_bytes = (char *) &wide_symbol;

//constants
    return ((wide_symbol_bytes[1] == 0x04) && IsCyrillicLowerByte(wide_symbol_bytes[0]));
}

//==============================================================================

static bool IsCyrillicLowerByte(char byte)
{
    return (byte >= 0x10 && byte <= 0x4f) || byte == 0x51 || byte == 0x01;
}

//==============================================================================

static bool IsOperatorSymbol(char symbol)
{
    return symbol == '?' ||
           symbol == ',' ||
           symbol == '!';
}

//==============================================================================

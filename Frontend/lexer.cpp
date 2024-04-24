#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "../Common/trees.h"
#include "../Stack/stack.h"
#include "../Common/NameTable.h"

static LexerErrs_t GetLexem(Stack          *stk,
                            Expr           *expr,
                            Identificators *vars);

static TreeNode *GetIdentificator    (Expr           *expr,
                                      Identificators *vars);
static TreeNode *GetUnknownExpression(Expr           *expr,
                                      Identificators *vars);

static TreeNode *GetKeyword(Expr *expr);
static TreeNode *GetNumber (Expr *expr);

static void SkipExprSpaces(Expr *expr);

static bool CyrillicIsalpha(Expr *expr);
static bool IsCyrillicLowerByte(char byte);

static bool IsRegularExpression(Expr *expr);

static int MissComment(char *string);


//==============================================================================


LexerErrs_t SplitOnLexems(Text      *text,
                          Stack     *stk,
                          Identificators *vars)
{
    setlocale(LC_ALL, "en_US.utf8");

    for (size_t i = 0; i < text->lines_count; i++)
    {
        printf("LINE -----> %d\n", i + 1);
        Expr expr;

        expr.pos         = 0;
        expr.string      = text->lines_ptr[i].str;
        expr.line_number = text->lines_ptr[i].real_line_number;

        printf("strlen %d\n", strlen(expr.string));

        MissComment(expr.string);

        while (expr.string[expr.pos] != '\0')
        {
            if (GetLexem(stk, &expr, vars) != kLexerSuccess)
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
        *comment = '\0';
    }

    return 0;
}

//==============================================================================

#define OP_CTOR(op_code) NodeCtor(nullptr, nullptr, nullptr, kOperator,      op_code)
#define NUM_CTOR(val)    NodeCtor(nullptr, nullptr, nullptr, kConstNumber,   val)
#define ID_CTOR(pos)     NodeCtor(nullptr, nullptr, nullptr, kIdentificator, pos)

#define CUR_CHAR expr->string[expr->pos]
#define STRING   expr->string
#define POS      expr->pos
#define CUR_STR  expr->string + expr->pos

#define D_PR printf("LINE: %d, STRING[%s], POS: %d\n", __LINE__, CUR_STR, POS);
//==============================================================================

static LexerErrs_t GetLexem(Stack          *stk,
                            Expr           *expr,
                            Identificators *vars)
{
    SkipExprSpaces(expr);
D_PR
    TreeNode *node = GetKeyword(expr);
D_PR

    if (node == nullptr)
    {
        node = GetUnknownExpression(expr, vars);
    }
D_PR

    Push(stk, node);
D_PR

    return kLexerSuccess;
}

//==============================================================================

static TreeNode *GetUnknownExpression(Expr           *expr,
                                      Identificators *vars)
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
        node = GetIdentificator(expr, vars);
    }

    SkipExprSpaces(expr);

    return node;
}

//==============================================================================

static void SkipExprSpaces(Expr *expr)
{
    while (isspace(CUR_CHAR) && CUR_CHAR != '\0')
    {
        POS++;
    }
}

//==============================================================================

TreeNode *GetKeyword(Expr *expr)
{
    CHECK(expr);

    for (size_t i = 0; i < kKeyWordCount; i++)
    {
        if (strncmp(CUR_STR, NameTable[i].key_word, NameTable[i].word_len) == 0)
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

static const size_t kMaxIdentificatorNameSize = 256;
static const size_t kSizeOfCyrillicWchar      = 2;

static TreeNode *GetIdentificator(Expr           *expr,
                                  Identificators *vars)
{
    CHECK(expr);
    CHECK(vars);

    TreeNode *identificator_node = nullptr;

    const char *identificator_start_ptr = CUR_STR;

    char identificator_name[kMaxIdentificatorNameSize] = {0};

    size_t identificator_name_length = 0;

    while(!isspace(CUR_CHAR) && CUR_CHAR != '\0')
    {
        ++identificator_name_length;

        ++POS;
    }

    memcpy(identificator_name, identificator_start_ptr, identificator_name_length);

    int identificator_pos = SeekIdentificator(vars, identificator_name);

    if (identificator_pos == -1)
    {
        identificator_pos = AddIdentificator(vars, strdup(identificator_name));
    }

    identificator_node = ID_CTOR(identificator_pos);

    SkipExprSpaces(expr);

    return identificator_node;
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

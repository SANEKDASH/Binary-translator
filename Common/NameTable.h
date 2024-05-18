#ifndef NAME_TABLE_HEADER
#define NAME_TABLE_HEADER

static const char *kMainFuncName = "Аганим";

typedef enum
{
    kConstNumber    = 1,
    kIdentifier     = 2,
    kOperator       = 3,
    kFuncDef        = 4,
    kParamsNode     = 5,
    kVarDecl        = 6,
    kCall           = 7,
} ExpressionType_t;


typedef enum
{
    kNotAnOperation   = 0,
    kLeftBracket      = 1, //
    kRightBracket     = 2, //
    kLeftZoneBracket  = 3, //
    kRightZoneBracket = 4, //

    kIf               = 11, // if
    kWhile            = 12, // while
    kAssign           = 13, // =

    kSin              = 21,  // sin
    kCos              = 22,  // cos
    kSqrt             = 29,  // sqrt

    kFloor            = 23,  // floor
    kAdd              = 24,  // +
    kSub              = 25,  // -
    kMult             = 26,  // *
    kDiv              = 27,  // /
    kDiff             = 28,  // diff

    kEqual            = 31, // ==
    kLess             = 32, // <
    kMore             = 33, // >
    kLessOrEqual      = 34, // <=
    kMoreOrEqual      = 35, // >=
    kNotEqual         = 36, // !=
    kAnd              = 37, // &&
    kOr               = 38, // ||

    kEndOfLine        = 41,
    kEnumOp           = 42,

    kDoubleType       = 51,

    kPrint            = 62,
    kScan             = 61,

    kReturn           = 71, // return
    kBreak            = 72, // break
    kContinue         = 73, // continue
    kAbort            = 74,

} KeyCode_t;

struct KeyWord
{
    const char *key_word;

    KeyCode_t   key_code;

    size_t      word_len;
};

static const KeyWord NameTable[]=
{
    #define DEF_KEYWORD(str, const) str, const, strlen(str),

    #include "keywords.gen.h"

    #undef DEF_KEYWORD
};

static const size_t kPrintPos = 30;
static const size_t kScanPos  = 31;
static const size_t kCosPos   = 5;
static const size_t kSinPos   = 4;
static const size_t kSqrtPos  = 8;

static const size_t EndOfLinePos = 28;

static const size_t kKeyWordCount = sizeof(NameTable) / sizeof(KeyWord);

#endif

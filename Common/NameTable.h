#ifndef NAME_TABLE_HEADER
#define NAME_TABLE_HEADER

static const char *kMainFuncName = "Аганим";

typedef enum
{
    kConstNumber    = 1,
    kIdentificator  = 2,
    kOperator       = 3,
    kFuncDef        = 4,
    kParamsNode     = 5,
    kVarDecl        = 6,
    kCall           = 7,
} ExpressionType_t;


typedef enum
{
    kNotAnOperation   = 0,
    kLeftBracket      = 1, //�� � ���������, �� � � ������ �� ���
    kRightBracket     = 2, //�� � ���������, �� � � ������ �� ���
    kLeftZoneBracket  = 3, //�� � ���������, �� � � ������ �� ���
    kRightZoneBracket = 4, //�� � ���������, �� � � ������ �� ���

    kIf               = 11, // if
    kWhile            = 12, // while
    kAssign           = 13, // =

    kSin              = 21,  // sin
    kCos              = 22,  // cos
    kFloor            = 23,  // floor
    kAdd              = 24,  // +
    kSub              = 25,  // -
    kMult             = 26,  // *
    kDiv              = 27,  // /
    kDiff             = 28,  // diff
    kSqrt             = 29,  // sqrt

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
    "лежать+сосать"         ,kAdd,              strlen("лежать+сосать"),
    "потерял_птсы"          ,kSub,              strlen("потерял_птсы"),
    "посадить_на_zxc"       ,kMult,             strlen("посадить_на_zxc"),
    "сплитим_на"            ,kDiv,              strlen("сплитим_на"),
    "углы_вымеряет"         ,kSin,              strlen("углы_вымеряет"),
    "это_все_преломления"   ,kCos,              strlen("это_все_преломления"),
    "соси_бля"              ,kFloor,            strlen("соси_бля"),
    "реквием"               ,kDiff,             strlen("реквием" ),
    "трент_ультует"         ,kSqrt,             strlen("трент_ультует"),
    "???"                   ,kIf,               strlen("???"),
    "пока"                  ,kWhile,            strlen("пока"),
    "ты"                    ,kAssign,           strlen("ты"),
    "больше"                ,kMore,             strlen("больше"),
    "меньше"                ,kLess,             strlen("меньше" ),
    "и"                     ,kAnd,              strlen("и" ),
    "или"                   ,kOr,               strlen("или"),
    "не_меньше"             ,kMoreOrEqual,      strlen("не_меньше"),
    "не_больше"             ,kLessOrEqual,      strlen("не_больше" ),
    "точно"                 ,kEqual,            strlen("точно" ),
    "ты_не"                 ,kNotEqual,         strlen("ты_не" ),
    "стой_блять!"           ,kBreak,            strlen("стой_блять!" ),
    "иди_блять!"            ,kContinue,         strlen("иди_блять!" ),
    "верни_курьера_блять"   ,kReturn,           strlen("верни_курьера_блять"),
    "мать"                  ,kLeftBracket,      strlen("мать"),
    "ебал"                  ,kRightBracket,     strlen("ебал"  ),
    "стань"                 ,kLeftZoneBracket,  strlen("стань" ),
    "мид"                   ,kRightZoneBracket, strlen("мид" ),
    ","                     ,kEnumOp,           strlen(","),
    "?"                     ,kEndOfLine,        strlen("?"    ),
    "долбоеб"               ,kDoubleType,       strlen("долбоеб" ),
    "пишу_твоей_матери"     ,kPrint,            strlen("пишу_твоей_матери"),
    "скажи_мне"             ,kScan,             strlen("скажи_мне"),
    "иди_нахуй"             ,kAbort,            strlen("иди_нахуй")
};

static const size_t EndOfLinePos = 28;

static const size_t kKeyWordCount = sizeof(NameTable) / sizeof(KeyWord);

#endif

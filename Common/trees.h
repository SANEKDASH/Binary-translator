#ifndef TREES_HEADER
#define TREES_HEADER

#include "../TextParse/text_parse.h"
#include "NameTable.h"

#ifdef TABLES_DEBUG
#define TABLES_DUMP(tables) DumpNameTables(tables, __func__, __LINE__)
#else
#define TABLES_DUMP(tables)
#endif

typedef double VarType_t;

typedef enum
{
    kVar       = 1,
    kFunc      = 2,
    kType      = 4,
    kUndefined = 3,
} IdType_t;

struct Expr
{
    char *string = nullptr;

    size_t  pos;
    size_t  line_number;

    bool rus_symbol_state;
};

struct Name
{
    size_t   pos;
    IdType_t type;
};

struct TableOfNames
{
    Name *names;

    size_t capacity;
    size_t name_count;

    int func_code;

    size_t pos;
};

struct NameTables
{
    TableOfNames **name_tables;

    size_t main_id_pos;

    size_t capacity;
    size_t tables_count;
};


struct Identifier
{
    char *id =  nullptr;

    bool declaration_state;

    IdType_t id_type;
};

struct Identifiers
{
    VarType_t type;

    Identifier *identifier_array;

    size_t size;

    size_t identifier_count;
};

typedef double NumType_t;

typedef enum
{
    kTreeSuccess,
    kFailedAllocation,
    kFailedToOpenFile,
    kFailedRealloc,
    kFailedToFind,
    kFailedToReadText,
    kFailedToReadTree,
    kTreeOptimized,
    kTreeNotOptimized,
    kNullTree,
    kMissingMain,
    kUnknownType,
    kUnknownKeyCode
} TreeErrs_t;

union NodeData
{
    NumType_t const_val;

    KeyCode_t key_word_code;

    size_t variable_pos;
    size_t line_number;

    const char *id;
};

struct TreeNode
{
    size_t line_number;

    NodeData data;

    ExpressionType_t type;

    TreeNode *parent;
    TreeNode *left;
    TreeNode *right;
};

struct Tree
{
    TreeNode *root;
};

struct LanguageContext
{
    Identifiers identifiers;

    NameTables tables;

    Tree syntax_tree;
};

int NameTablesInit(NameTables *name_tables);

int DumpNameTables(NameTables *tables,
                   const char *func,
                   int         line);

int CloseNamesLog();

int InitNamesLog();

TableOfNames *AddTableOfNames(NameTables *tables,
                              int         func_code);

int TablesOfNamesInit(TableOfNames *table,
                      int           func_code);

int AddName(TableOfNames *table,
            size_t        id,
            IdType_t      type);

int VarArrayDtor(Identifiers *identifiers);

int AddIdentifier(Identifiers *vars,
                  char         *var_name);

int SeekIdentifier(Identifiers *vars,
                   const char  *var_name);

int VarArrayInit(Identifiers *vars);

TreeErrs_t LanguageContextInit(LanguageContext *language_context);

TreeErrs_t LanguageContextDtor(LanguageContext *language_context);

TreeErrs_t TreeVerify(Tree *tree);

TreeErrs_t TreeCtor(Tree *tree);

TreeNode *NodeCtor(TreeNode         *parent_node,
                   TreeNode         *left,
                   TreeNode         *right,
                   ExpressionType_t  type,
                   double            data);

TreeErrs_t TreeDtor(TreeNode *root);

TreeErrs_t PrintTreeInFile(LanguageContext *language_context,
                           const char      *file_name);

TreeErrs_t ReadLanguageContextOutOfFile(LanguageContext *language_context,
                                        const char      *tree_file_name,
                                        const char      *tables_file);

TreeNode *CopyNode(const TreeNode *src_node);

TreeErrs_t SetParents(TreeNode *parent_node);

TreeErrs_t GetDepth(const TreeNode *node,
                    int            *depth);

#endif

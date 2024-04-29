#ifndef DIFF_HEADER
#define DIFF_HEADER

#include "trees.h"
#include "parse.h"

typedef enum
{
    kDiffSuccess,
    kNotANumber,
    kSyntaxError,
    kDiffFailedAlloc,
} DiffErrs_t;

static const size_t kOperationCount = sizeof(OperationArray) / sizeof(Operation);


DiffErrs_t SetNumber(TreeNode   *node,
                     const char *num_str);

DiffErrs_t SetUpData(TreeNode *node,
                     const char *str);

NumType_t Eval(const TreeNode *node);

TreeNode *DiffTree(const TreeNode *node,
                   TreeNode       *parent_node);

TreeErrs_t OptimizeConstants(Tree      *tree,
                             TreeNode **node);

TreeErrs_t OptimizeNeutralExpr(Tree      *tree,
                               TreeNode **node);

TreeErrs_t OptimizeTree(Tree *tree);

bool IsUnaryOp(const OpCode_t op_code);

#endif

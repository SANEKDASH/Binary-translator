#include "../Common/trees.h"
#include "diff.h"

static bool IsValZero( const TreeNode *node);
static bool IsValOne(  const TreeNode *node);
static bool IsNumber(  const TreeNode *node);
static bool IsVariable(const TreeNode *node);

static TreeErrs_t ReconnectTree(TreeNode **dest, TreeNode *src);


// change type, because it is shit
NumType_t Eval(const TreeNode *node)
{
    if (node == nullptr)
    {
        return 0;
    }

    if (node->type == kConstNumber)
    {
        return node->data.const_val;
    }

    NumType_t left  = Eval(vars, node->left);
    NumType_t right = Eval(vars, node->right);

    switch (node->data.op_code)
    {
        case kAdd:
        {
            return left + right;
        }

        case kSub:
        {
            return left - right;
        }

        case kMult:
        {
            return left * right;
        }

        case kDiv:
        {
            if (right != 0)
            {
                return left / right;
            }
            else
            {
                return NAN;
            }
        }

        case kSqrt:
        {
            return sqrt(right);
        }

        case kSin:
        {
            return sin(right);
        }

        case kCos:
        {
            return cos(right);
        }

        case kTg:
        {
            return tan(right);
        }

        case kExp:
        {
            return pow(left, right);
        }

        case kLn:
        {
            return log(right);
        }

        default:
        {
            printf("Eval() got unknown op_code.");

            break;
        }
    }

    return NAN;
}

//==============================================================================

TreeNode *DiffTree(const TreeNode *node,
                   TreeNode       *parent_node)
{
    #define NUM_CTOR(num)          NodeCtor(nullptr, nullptr, nullptr, kConstNumber, num)

    #define ADD_CTOR(left, right)  NodeCtor(nullptr, left, right, kOperator, kAdd)
    #define SUB_CTOR(left, right)  NodeCtor(nullptr, left, right, kOperator, kSub)
    #define DIV_CTOR(left, right)  NodeCtor(nullptr, left, right, kOperator, kDiv)
    #define MULT_CTOR(left, right) NodeCtor(nullptr, left, right, kOperator, kMult)
    #define SQRT_CTOR(right)       NodeCtor(nullptr, nullptr, right, kOperator, kSqrt)
    #define SIN_CTOR(right)        NodeCtor(nullptr, nullptr, right, kOperator, kSin)
    #define COS_CTOR(right)        NodeCtor(nullptr, nullptr, right, kOperator, kCos)
    #define TG_CTOR(right)         NodeCtor(nullptr, nullptr, right, kOperator, kTg)
    #define LN_CTOR(right)         NodeCtor(nullptr, nullptr, right, kOperator, kLn)
    #define POW_CTOR(left, right)  NodeCtor(nullptr, left, right, kOperator, kExp)

    #define D(node) DiffTree(node, nullptr)
    #define C(node) CopyNode(node, nullptr)

    CHECK(node);

    if (node->type == kConstNumber)
    {
        return NodeCtor(parent_node, nullptr, nullptr, kConstNumber, 0);
    }

    if (node->type == kVariable)
    {
        return NodeCtor(parent_node, nullptr, nullptr, kConstNumber, 1);
    }

    switch (node->data.op_code)
    {
        case kAdd:
        {
            return ADD_CTOR(D(node->left),
                            D(node->right));

            break;
        }

        case kSub:
        {
            return SUB_CTOR(D(node->left),
                            D(node->right));

            break;
        }

        case kMult:
        {
                return ADD_CTOR(MULT_CTOR(D(node->left),
                                          C(node->right)),
                                MULT_CTOR(C(node->left),
                                          D(node->right)));

            break;
        }

        case kDiv:
        {
            return DIV_CTOR(SUB_CTOR(MULT_CTOR(D(node->left),
                                               C(node->right)),
                                     MULT_CTOR(C(node->left),
                                               D(node->right))),
                            POW_CTOR(C(node->right),
                                     NUM_CTOR(2)));

            break;

        }

        case kCos:
        {
            return MULT_CTOR(MULT_CTOR(NUM_CTOR(-1),
                                       SIN_CTOR(C(node->right))),
                             D(node->right));
        }

        case kSin:
        {
            return MULT_CTOR(COS_CTOR(C(node->right)),
                             D(node->right));
        }

        case kTg:
        {
            return MULT_CTOR(DIV_CTOR(NUM_CTOR(1),
                                      POW_CTOR(COS_CTOR(C(node->right)),
                                               NUM_CTOR(2))),
                             D(node->right));
        }

        case kLn:
        {
            return MULT_CTOR(DIV_CTOR(NUM_CTOR(1),
                                      C(node->right)),
                             D(node->right));
        }

        case kExp:
        {
            if (!IsValZero(node->right))
            {
                if (IsNumber(node->right))
                {
                    if (IsVariable(node->left))
                    {
                        return MULT_CTOR(NUM_CTOR(node->right->data.const_val),
                                         POW_CTOR(VAR_CTOR(0),
                                                  NUM_CTOR(node->right->data.const_val - 1)));
                    }
                    else
                    {
                        return MULT_CTOR(POW_CTOR(C(node->left),
                                         NUM_CTOR(node->right->data.const_val - 1)), D(node->left));
                    }
                }
            }

            return MULT_CTOR(C(node),
                             ADD_CTOR(MULT_CTOR(DIV_CTOR(C(node->right),
                                                         C(node->left)),
                                                D(node->left)),
                                      MULT_CTOR(D(node->right),
                                                LN_CTOR(C(node->left)))));
        }

        default:
        {
            printf("Diff() don't know such derivative OPCODE : %d, TYPE: %d\n", node->data.op_code, node->type);

            break;
        }
    }

    return nullptr;
}

//==============================================================================

TreeErrs_t OptimizeConstants(Tree      *tree,
                             TreeNode **node)
{
    if ((*node)->type == kConstNumber || (*node)->type == kVariable)
    {
        return kTreeNotOptimized;
    }

    if (!IsUnaryOp((*node)->data.op_code))
    {
        if ((*node)->left->type  == kConstNumber &&
            (*node)->right->type == kConstNumber)
        {

            NumType_t val = Eval(vars, *node);

            TreeDtor(*node);

            *node = NUM_CTOR(val);

            return kTreeOptimized;
        }
    }

    if ((*node)->left != nullptr)
    {
        if (OptimizeConstants(vars, tree, &(*node)->left) == kTreeOptimized)
        {
            return kTreeOptimized;
        }
    }

    if ((*node)->right != nullptr)
    {
        if (OptimizeConstants(vars, tree, &(*node)->right) == kTreeOptimized)
        {
            return kTreeOptimized;
        }
    }

    return kTreeNotOptimized;
}

//==============================================================================


TreeErrs_t OptimizeNeutralExpr(Tree      *tree,
                               TreeNode **node)
{
    if ((*node)->type == kConstNumber || (*node)->type == kVariable)
    {
        return kTreeSuccess;;
    }

    switch ((*node)->data.op_code)
    {
        case kAdd:
        {
            if (IsValZero((*node)->right))
            {
                ReconnectTree(node, C((*node)->left));

                return kTreeOptimized;
            }
            if (IsValZero((*node)->left))
            {
                ReconnectTree(node, C((*node)->right));

                return kTreeOptimized;
            }

            break;
        }

        case kSub:
        {
            if (IsValZero((*node)->right))
            {
                ReconnectTree(node, C((*node)->left));

                return kTreeOptimized;
            }

            break;
        }

        case kMult:
        {
            if (IsValZero((*node)->left) || IsValZero((*node)->right))
            {
                ReconnectTree(node, NUM_CTOR(0));

                return kTreeOptimized;
            }

            if (IsValOne((*node)->left))
            {
                ReconnectTree(node, C((*node)->right));

                return kTreeOptimized;
            }

            if (IsValOne((*node)->right))
            {
                ReconnectTree(node, C((*node)->left));

                return kTreeOptimized;
            }

            break;
        }

        case kDiv:
        {
            if (IsValZero((*node)->left))
            {
                ReconnectTree(node, NUM_CTOR(0));

                return kTreeOptimized;
            }

            if (IsValOne((*node)->right))
            {
                ReconnectTree(node, C((*node)->left));

                return kTreeOptimized;
            }

            break;
        }

        case kExp:
        {
            if (IsValZero((*node)->right))
            {
                ReconnectTree(node, NUM_CTOR(1));

                return kTreeOptimized;
            }

            if (IsValOne((*node)->left))
            {
                ReconnectTree(node, NUM_CTOR(1));

                return kTreeOptimized;
            }

            if (IsValOne((*node)->right))
            {
                ReconnectTree(node, C((*node)->left));

                return kTreeOptimized;
            }

            break;
        }
    }

    if ((*node)->left != nullptr)
    {
        if (OptimizeNeutralExpr(tree, &(*node)->left) == kTreeOptimized)
        {
            return kTreeOptimized;
        }
    }

    if ((*node)->right != nullptr)
    {
        if (OptimizeNeutralExpr(tree, &(*node)->right) == kTreeOptimized)
        {
            return kTreeOptimized;
        }
    }

    return kTreeNotOptimized;
}

//==============================================================================

TreeErrs_t OptimizeTree(Variables *vars)
{
    while (true)
    {

        TreeErrs_t status_1 = OptimizeNeutralExpr(tree, &tree->root);

        TreeErrs_t status_2 = OptimizeConstants(vars, tree, &tree->root);

        if (status_1 == kTreeNotOptimized && status_2 == kTreeNotOptimized)
        {
            break;
        }

    }

    GRAPH_DUMP_TREE(tree);

    return kTreeSuccess;
}

//==============================================================================

bool IsUnaryOp(const OpCode_t op_code)
{
    return op_code == kSqrt ||
           op_code == kSin  ||
           op_code == kCos  ||
           op_code == kTg   ||
           op_code == kLn;
}

//==============================================================================

static bool IsValZero(const TreeNode *node)
{
    return  (node->type == kConstNumber) && (node->data.const_val == 0);
}

//==============================================================================

static bool IsValOne(const TreeNode *node)
{
    return (node->type == kConstNumber) && (node->data.const_val == 1);
}

//==============================================================================

static bool IsVariable(const TreeNode *node)
{
    return (node->type == kVariable);
}

//==============================================================================

static bool IsNumber(const TreeNode *node)
{
    return (node->type == kConstNumber);
}

//==============================================================================

static TreeErrs_t ReconnectTree(TreeNode **dest,
                                TreeNode *src)
{
    TreeDtor(*dest);

    *dest = src;

    return kTreeSuccess;
}

//==============================================================================

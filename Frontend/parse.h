#ifndef PARSE_HEADER
#define PARSE_HEADER

#include <locale.h>

#include "../Common/trees.h"
#include "../Stack/stack.h"

TreeNode *GetSyntaxTree(Identifiers *identifiers,
                        const char     *file_name);

TreeNode *GetAddExpression(Identifiers *identifiers,
                           Stack          *tokens,
                           size_t         *iter);

TreeNode *GetExpression(Identifiers *identifiers,
                        Stack          *tokens,
                        size_t         *iter);

TreeNode *GetMultExpression(Identifiers *identifiers,
                            Stack          *tokens,
                            size_t         *iter);

TreeNode* GetConstant(Identifiers *identifiers,
                      Stack          *tokens,
                      size_t         *iter);

TreeNode *GetIdentifier(Identifiers *identifiers,
                           Stack          *tokens,
                           size_t         *iter);

TreeNode *GetPrimaryExpression(Identifiers *identifiers,
                               Stack          *tokens,
                               size_t         *iter);

#endif

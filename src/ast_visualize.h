#ifndef AST_VISUALIZE_H
#define AST_VISUALIZE_H

#include "parser.h"
#include <string>

void exportASTtoDot(const std::shared_ptr<ASTNode>& root, const std::string& filename);

#endif

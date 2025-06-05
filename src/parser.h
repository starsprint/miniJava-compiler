#ifndef PARSER_H
#define PARSER_H

#include "lex.h"
#include <vector>
#include <string>
#include <memory>

struct ASTNode {
    std::string type; // 节点类型（如 Program, VarDecl, If, While, Assign 等）
    std::string value; // 节点值（如变量名、常量值等）
    std::string varType; // 变量类型（如 int, String）
    int line = 0; // 行号
    std::vector<std::shared_ptr<ASTNode>> children;  // 子节点列表
};

// 错误信息结构体
struct ParseError {
    std::string message;
    int line;
    int column;
};

using ASTPtr = std::shared_ptr<ASTNode>;

ASTPtr parse(const std::vector<Token>& tokens);

void printParseErrors();
extern std::vector<ParseError> parseErrors;
#endif
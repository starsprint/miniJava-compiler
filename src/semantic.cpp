#include "semantic.h"
#include <iostream>
#include <unordered_map>
#include <functional>

bool checkSemantics(const ASTPtr& root) {
    std::unordered_map<std::string, std::string> symbolTable;

    // 表达式类型推断
    std::function<std::string(const ASTPtr&)> exprType = [&](const ASTPtr& node) -> std::string {
        if (!node) return "";
        if (node->type == "Int") return "int";
        if (node->type == "Str") return "String";
        if (node->type == "Var") {
            if (symbolTable.count(node->value)) return symbolTable[node->value];
            return "";
        }
        if (node->type == "Add" || node->type == "Sub" || node->type == "Mul" ||
            node->type == "Lt" || node->type == "Eq") {
            return "int";
        }
        return "";
    };

    std::function<void(const ASTPtr&)> visit = [&](const ASTPtr& node) {
        if (!node) return;
        if (node->type == "VarDecl") {
            symbolTable[node->value] = node->varType;
            // 检查初始化表达式类型
            if (!node->children.empty()) {
                std::string rhsType = exprType(node->children[0]);
                if (!rhsType.empty() && rhsType != node->varType) {
                    std::cerr << "[语义错误] 变量 " << node->value << " 类型不匹配 (行: " << node->line << ")\n";
                }
            }
        }
        if (node->type == "Assign") {
            if (symbolTable.count(node->value) == 0) {
                std::cerr << "[语义错误] 未定义变量: " << node->value << " (行: " << node->line << ")\n";
            } else {
                std::string varType = symbolTable[node->value];
                std::string rhsType = exprType(node->children[0]);
                if (!rhsType.empty() && rhsType != varType) {
                    std::cerr << "[语义错误] 变量 " << node->value << " 类型不匹配 (行: " << node->line << ")\n";
                }
            }
        }
        for (const auto& child : node->children) visit(child);
    };

    visit(root);
    return true;
}
#include "irgen.h"
#include <sstream>
#include <functional>

static int tempVarCount = 0;

std::string newTemp() {
    std::ostringstream oss;
    oss << "t" << tempVarCount++;
    return oss.str();
}

std::vector<Quadruple> generateIR(const ASTPtr& root) {
    std::vector<Quadruple> ir;

    std::function<std::string(const ASTPtr&)> gen = [&](const ASTPtr& node) -> std::string {
        if (!node) return "";
        if (node->type == "Int" || node->type == "Var" || node->type == "Str") return node->value;
        if (node->type == "Add" || node->type == "Sub" || node->type == "Mul" || node->type == "Lt" || node->type == "Eq") {
            std::string t1 = gen(node->children[0]);
            std::string t2 = gen(node->children[1]);
            std::string res = newTemp();
            ir.push_back({node->type, t1, t2, res});
            return res;
        }
        if (node->type == "Assign") {
            std::string rhs = gen(node->children[0]);
            ir.push_back({"=", rhs, "", node->value});
            return node->value;
        }
        if (node->type == "VarDecl") {
            // 可选：生成声明四元式
            // ir.push_back({"decl", node->varType, "_", node->value});
            if (!node->children.empty()) {
                std::string rhs = gen(node->children[0]);
                ir.push_back({"=", rhs, "", node->value});
            }
            return "";
        }
        for (const auto& child : node->children) gen(child);
        return "";
    };

    gen(root);
    return ir;
}
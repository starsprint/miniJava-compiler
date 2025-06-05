#include "lex.h"
#include "parser.h"
#include "semantic.h"
#include "irgen.h"
#include <iostream>
#include <fstream>
#include "ast_visualize.h"

void writeTokenStream(const std::vector<Token>& tokens) {
    std::ofstream fout("../res/tokens.txt");
    for (const auto& tok : tokens) {
        fout << tok.type << " " << tok.value << "\n";
    }
    fout.close();
}

void writeIdentifierTable() {
    std::ofstream fout("../res/identifier_table.txt");
    for (size_t i = 0; i < identifierTable.size(); ++i) {
        fout << i << ": " << identifierTable[i] << "\n";
    }
    fout.close();
}

void writeConstantTable() {
    std::ofstream fout("../res/constant_table.txt");
    for (size_t i = 0; i < constantTable.size(); ++i) {
        fout << i << ": " << constantTable[i] << "\n";
    }
    fout.close();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "请输入测试文件名\n";
        return 1;
    }
    std::string inputFile = argv[1];
    std::vector<Token> tokens = runLexer("../test/" + inputFile);
    writeTokenStream(tokens);
    writeIdentifierTable();
    writeConstantTable();

    // 检查token流
    // for (const auto& tok : tokens) {
    // std::cout << tok.type << " " << tok.value << " " << tok.line << " " << tok.column << std::endl;
    // }
    
    ASTPtr astRoot = parse(tokens);
    printParseErrors();
    if (astRoot && parseErrors.empty()) {
    exportASTtoDot(astRoot, "../res/ast.dot");
    }
    // 有语法错误则不继续语义分析和IR生成
    if (!astRoot || !parseErrors.empty()) return 1;
    if (astRoot) {
        if (checkSemantics(astRoot)) {
            std::vector<Quadruple> ir = generateIR(astRoot);
            std::ofstream irout("../res/ir.txt");
            for (const auto& quad : ir) {
                irout << quad.op << " " << quad.arg1 << " " << quad.arg2 << " " << quad.result << "\n";
            }
            irout.close();
        }
    }
    return 0;
}
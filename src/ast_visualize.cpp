#include "parser.h"
#include <fstream>
#include <sstream>

static int nodeCounter = 0;

void writeDotNode(std::ofstream& out, const std::shared_ptr<ASTNode>& node, int parentId = -1) {
    if (!node) return;
    int myId = nodeCounter++;
    std::ostringstream label;
    label << node->type;
    if (!node->value.empty()) label << "\\n" << node->value;
    out << "  node" << myId << " [label=\"" << label.str() << "\"]" << std::endl;
    if (parentId != -1) {
        out << "  node" << parentId << " -> node" << myId << std::endl;
    }
    for (const auto& child : node->children) {
        writeDotNode(out, child, myId);
    }
}

void exportASTtoDot(const std::shared_ptr<ASTNode>& root, const std::string& filename) {
    std::ofstream fout(filename);
    fout << "digraph AST {" << std::endl;
    fout << "  node [shape=box, style=filled, fillcolor=lightgray];" << std::endl;
    nodeCounter = 0;
    writeDotNode(fout, root);
    fout << "}" << std::endl;
    fout.close();
}
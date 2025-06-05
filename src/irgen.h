#ifndef IRGEN_H
#define IRGEN_H

#include "parser.h"
#include <vector>
#include <string>

struct Quadruple {
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;
};

std::vector<Quadruple> generateIR(const ASTPtr& root);

#endif
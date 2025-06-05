#include "parser.h"
#include <iostream>
#include <memory>
#include <vector>
#include <string>

static size_t current = 0; // 当前token的索引
static std::vector<Token> tokens;
std::vector<ParseError> parseErrors;

// 获取当前token的行列号
static void getTokenPos(int& line, int& column) {
    if (current < tokens.size() && tokens[current].line > 0) {
        line = tokens[current].line;
        column = tokens[current].column;
    } else {
        line = -1;
        column = -1;
    }
}

// 记录语法错误
static void error(const std::string& msg) {
    int line, column;
    getTokenPos(line, column);
    parseErrors.push_back({msg, line, column});
}

// 查看当前token
static Token peek() {
    return current < tokens.size() ? tokens[current] : Token{END_OF_FILE, -1, -1, -1};
}

// 检查当前token是否匹配指定类型和值，传如第二个参数则检查类型和值，不传入则只检查类型
static bool match(TokenType type, int val = -2) {
    if (current >= tokens.size()) return false;
    Token tk = tokens[current];
    if (tk.type == type && (val == -2 || val == tk.value)) {
        current++;
        return true;
    }
    return false;
}

// 创建AST节点
static ASTPtr makeNode(const std::string& type, const std::string& value = "", int line = 0) {
    auto node = std::make_shared<ASTNode>();
    node->type = type;
    node->value = value;
    node->line = line;
    return node;
}

// 语法分析函数
ASTPtr parsePrimary(); // 解析基本因子(整数，标识符，字符串，括号表达式)
ASTPtr parseMul(); // 解析乘法（左结合）
ASTPtr parseAdd(); // 解析加法（左结合）
ASTPtr parseRelational(); // 解析关系运算
ASTPtr parseExpression(); // 解析表达式入口
ASTPtr parseStatement(); // 解析语句（{} 块语句、int/String声明、if/while、赋值）
ASTPtr parseMainClass(); // 解析主类（类结构class main)

ASTPtr parsePrimary() {
    Token tk = peek();
    if (tk.type == INTEGER_LITERAL) {
        // 整数
        current++;
        return makeNode("Int", constantTable[tk.value], tk.line);
    } else if (tk.type == IDENTIFIER) {
        // 标识符
        current++;
        return makeNode("Var", identifierTable[tk.value], tk.line);
    } else if (tk.type == STRING_LITERAL) {
        // 字符串
        current++;
        return makeNode("Str", constantTable[tk.value], tk.line);
    } else if (tk.type == DELIMITER && tk.value == '(') {
        // 括号表达式
        match(DELIMITER, '(');
        auto expr = parseExpression();
        if (!match(DELIMITER, ')')) error("表达式缺少 )");
        return expr;
    } else {
        error("无法识别的表达式");
        return nullptr;
    }
}

ASTPtr parseMul() {
    auto left = parsePrimary();
    while (peek().type == OPERATOR && peek().value == '*') {
        match(OPERATOR, '*');
        auto right = parsePrimary();
        auto node = makeNode("Mul");
        node->children.push_back(left);
        node->children.push_back(right);
        left = node;
    }
    return left;
}

ASTPtr parseAdd() {
    auto left = parseMul();
    while (peek().type == OPERATOR && (peek().value == '+' || peek().value == '-')) {
        int op = peek().value;
        match(OPERATOR, op);
        auto right = parseMul();
        auto node = makeNode(op == '+' ? "Add" : "Sub");
        node->children.push_back(left);
        node->children.push_back(right);
        left = node;
    }
    return left;
}

ASTPtr parseRelational() {
    auto left = parseAdd();
    while (peek().type == OPERATOR && (peek().value == '<' || peek().value == '=')) {
        int op = peek().value;
        match(OPERATOR, op);
        auto right = parseAdd();
        auto node = makeNode(op == '<' ? "Lt" : "Eq");
        node->children.push_back(left);
        node->children.push_back(right);
        left = node;
    }
    return left;
}

ASTPtr parseExpression() {
    return parseRelational();
}

ASTPtr parseStatement() {
    if (match(DELIMITER, '{')) {
        auto block = makeNode("Block");
        while (!match(DELIMITER, '}')) {
            auto stmt = parseStatement();
            if (stmt) block->children.push_back(stmt);
        }
        return block;
    }
    // int 类型变量声明
    else if (match(KEYWORD, KW_INT)) {
        if (!match(IDENTIFIER)) {
            error("变量声明缺少标识符");
            return nullptr;
        }
        int declLine = tokens[current - 1].line;
        std::string varName = identifierTable[tokens[current - 1].value];
        auto decl = makeNode("VarDecl", varName, declLine); // value=变量名
        decl->varType = "int";
        if (match(OPERATOR, '=')) {
            decl->children.push_back(parseExpression());
        }
        if (!match(DELIMITER, ';')) error("变量声明缺少分号");
        return decl;
    }
    // String 类型变量声明
    else if (match(KEYWORD, KW_STRING)) {
        if (!match(IDENTIFIER)) {
            error("变量声明缺少标识符");
            return nullptr;
        }
        int declLine = tokens[current - 1].line;
        std::string varName = identifierTable[tokens[current - 1].value];
        auto decl = makeNode("VarDecl", varName, declLine);
        decl->varType = "String";
        if (match(OPERATOR, '=')) {
            decl->children.push_back(parseExpression());
        }
        if (!match(DELIMITER, ';')) error("变量声明缺少分号");
        return decl;
    }
    else if (match(KEYWORD)) {
        Token tk = tokens[current - 1];
        if (tk.value == KW_IF) {
            // 解析 if 语句
            auto ifNode = makeNode("If");
            match(DELIMITER, '(');
            ifNode->children.push_back(parseExpression());
            match(DELIMITER, ')');
            ifNode->children.push_back(parseStatement());
            if (!match(KEYWORD) || tokens[current - 1].value != KW_ELSE) error("缺少 else");
            ifNode->children.push_back(parseStatement());
            return ifNode;
        } else if (tk.value == KW_WHILE) {
            // 解析 while 语句
            auto whNode = makeNode("While");
            match(DELIMITER, '(');
            whNode->children.push_back(parseExpression());
            match(DELIMITER, ')');
            whNode->children.push_back(parseStatement());
            return whNode;
        }
    } else if (match(IDENTIFIER)) {
        // 解析赋值语句
        int assignLine = tokens[current - 1].line;
        std::string varName = identifierTable[tokens[current - 1].value];
        if (match(OPERATOR, '=')) {
            auto assign = makeNode("Assign", varName, assignLine);
            assign->children.push_back(parseExpression());
            match(DELIMITER, ';');
            return assign;
        }
    }
    error("无法解析的语句");
    return nullptr;
}

ASTPtr parseMainClass() {
    if (!match(KEYWORD)) error("缺少 class");
    if (!match(IDENTIFIER)) error("缺少类名");
    if (!match(DELIMITER, '{')) error("缺少类 { 开始");
    for (int i = 0; i < 4; ++i) if (!match(KEYWORD)) error("缺少 main 关键字头部");
    if (!match(DELIMITER, '(')) error("缺少 (");
    // 匹配main的参数
    match(KEYWORD); // String
    match(DELIMITER, '[');
    match(DELIMITER, ']');
    match(IDENTIFIER); // args
    match(DELIMITER, ')');

    auto mainBody = parseStatement();

    if (!match(DELIMITER, '}')) error("缺少类 } 结束");

    auto root = makeNode("Program");
    root->children.push_back(mainBody);
    return root;
}

ASTPtr parse(const std::vector<Token>& tks) {
    tokens = tks;
    current = 0;
    parseErrors.clear();
    return parseMainClass();
}

// 打印错误
void printParseErrors() {
    for (const auto& err : parseErrors) {
        std::cerr << "[语法错误] " << err.message;
        if (err.line > 0)
            std::cerr << " (行: " << err.line << ", 列: " << err.column << ")";
        std::cerr << std::endl;
    }
}
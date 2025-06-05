#ifndef LEX_H
#define LEX_H

#include <string>
#include <vector>
#include <unordered_map>

// Token结构定义
enum TokenType {
    KEYWORD = 1,  // 关键字
    IDENTIFIER = 2,   // 标识符
    INTEGER_LITERAL = 3,  // 整数字面量
    STRING_LITERAL = 4,  // 字符串字面量
    OPERATOR = 5,  // 运算符
    DELIMITER = 6,  // 界限符
    ERROR = 7,  // 错误
    END_OF_FILE = 8  // 文件结束符
};

// 关键字枚举，用value判断关键字类型
enum KeywordValue {
    KW_CLASS,
    KW_PUBLIC,
    KW_STATIC,
    KW_VOID,
    KW_MAIN,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_RETURN,
    KW_TRUE,
    KW_FALSE,
    KW_STRING,
    KW_INT
};

struct Token {
    TokenType type;
    int value; // 指向符号表或常量表的索引
    int line;  // 行号
    int column; // 列号
};

extern std::vector<std::string> identifierTable;
extern std::vector<std::string> constantTable;

std::vector<Token> runLexer(const std::string& filename);

#endif
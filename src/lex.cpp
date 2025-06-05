#include "lex.h"
#include <fstream>
#include <iostream>
#include <cctype>
#include <algorithm>

/** 核心流程：
 * 1.读取每一行源码
 * 2.按字符分类识别
 *  ·是字母开头 → 尝试识别关键字或标识符
 *  ·是数字开头 → 尝试识别整数
 *  ·是 +, -, *, <, = → 运算符
 *  ·是 {, }, ( 等 → 分隔符
 *  ·是 $ → 文件结束符
 * 3.记录并写入对应 token
 *  ·若是标识符，写入 identifierTable
 *  ·若是数字，写入 constantTable
 */

// 错误类型
enum class LexErrorType {
    UNKNOWN_CHAR,       // 无法识别的字符
    INVALID_IDENTIFIER, // 非法标识符
    INVALID_NUMBER,     // 非法数字格式
    UNTERMINATED_STRING // 未终止的字符串
};

 // 错误信息结构体
struct LexError {
    LexErrorType type;
    int line;
    int column;
    std::string message;
    char problematicChar;
};

//  存储所有识别出的标识符
std::vector<std::string> identifierTable;

// 存储所有识别出的数字常量和字符串常量
std::vector<std::string> constantTable;

// 存储词法分析过程中的错误
std::vector<LexError> errorList;

std::unordered_map<std::string, int> keywordMap = {
    {"class", KW_CLASS}, {"public", KW_PUBLIC}, {"static", KW_STATIC}, {"void", KW_VOID},
    {"main", KW_MAIN}, {"if", KW_IF}, {"else", KW_ELSE}, {"while", KW_WHILE},
    {"return", KW_RETURN}, {"true", KW_TRUE}, {"false", KW_FALSE}, {"String", KW_STRING},{"int", KW_INT}
};

// 判断字符是否是分隔符
bool isDelimiter(char c) {
    return c == '{' || c == '}' || c == '(' || c == ')' ||
           c == '[' || c == ']' || c == ';' || c == ',';
}

// 判断字符是否是运算符
bool isOperator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '<' || c == '=';
}

// 添加错误到错误列表
void addError(LexErrorType type, int line, int column, char c) {
    std::string message;
    switch(type) {
        case LexErrorType::UNKNOWN_CHAR:
            message = "无法识别的字符";
            break;
        case LexErrorType::INVALID_IDENTIFIER:
            message = "非法标识符格式";
            break;
        case LexErrorType::INVALID_NUMBER:
            message = "非法数字格式";
            break;
        case LexErrorType::UNTERMINATED_STRING:
            message = "字符串未正确终止";
            break;
    }
    errorList.push_back({type, line, column, message, c});
}

// 检查标识符是否合法
bool isValidIdentifier(const std::string& id) {
    if (!isalpha(id[0]) && id[0] != '_') return false;
    for (char c : id) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

// 打印所有错误
void printErrors() {
    for (const auto& error : errorList) {
        std::cerr << "[词法错误] 第" << error.line << "行, 第" << error.column << "列: "
                  << error.message << " '" << error.problematicChar << "'\n";
    }
}

// 函数接收文件名作为参数，返回token列表
std::vector<Token> runLexer(const std::string& filename) {
    std::ifstream fin(filename);
    std::vector<Token> tokens;
    std::string line;
    int lineNumber = 1;
    errorList.clear(); // 清空错误列表

    // 逐行读取文件内容
    while (std::getline(fin, line)) {
        size_t i = 0;
        // 逐字符处理当前行
        while (i < line.size()) {
            int column = i + 1; // 列号从1开始
            if (isspace(line[i])) { ++i; continue; } // 跳过空白字符

            // 标识符或关键字识别,isalpha 检查是否为字母
            if (isalpha(line[i]) || line[i] == '_') {
                std::string word;
                // 读取连续的字母数字和下划线组成单词,isalnum 检查是否为字母或数字
                while (i<line.size() && (isalnum(line[i]) || line[i] == '_')) word += line[i++];
                if (!isValidIdentifier(word)) {
                    // 如果标识符不合法，记录错误
                    addError(LexErrorType::INVALID_IDENTIFIER, lineNumber, column, line[i-1]);
                    continue;
                }
                if (keywordMap.count(word)) {
                    // 在关键字表中，添加KEYWORD类型的token
                    tokens.push_back({KEYWORD, keywordMap[word],lineNumber, column});
                } else {
                    // 在标识符表中查找
                    auto it = std::find(identifierTable.begin(), identifierTable.end(), word);
                    int index = it == identifierTable.end() ? identifierTable.size() : std::distance(identifierTable.begin(), it);
                    // 如果未找到，则添加到标识符表
                    if (it == identifierTable.end()) identifierTable.push_back(word);
                    // 添加IDENTIFIER类型的token
                    tokens.push_back({IDENTIFIER, index, lineNumber, column});
                }
            } 
            // 数字常量识别
            else if (isdigit(line[i])) {
                std::string num;
                bool hasError = false;
                // 检查前导零
                if (line[i] == '0' && i+1 < line.size() && isdigit(line[i+1])) {
                    // 如果数字以0开头且后面还有数字，则记录错误
                    addError(LexErrorType::INVALID_NUMBER, lineNumber, column, line[i]);
                    hasError = true;
                }
                // 读取连续的数字
                while (isdigit(line[i])) num += line[i++]; 
                // 检查数字后是否紧跟字母或下划线
                if (i < line.size() && (isalpha(line[i]) || line[i] == '_')) {
                    addError(LexErrorType::INVALID_IDENTIFIER, lineNumber, column, line[i]);
                    // 跳过后续的非法标识符部分
                    while (i < line.size() && (isalnum(line[i]) || line[i] == '_')) ++i;
                    continue;
                }
                if(!hasError)
                {
                    // 在常量表中查找
                    auto it = std::find(constantTable.begin(), constantTable.end(), num);
                    int index = it == constantTable.end() ? constantTable.size() : std::distance(constantTable.begin(), it);
                    // 如果未找到，则添加到常量表
                    if (it == constantTable.end()) constantTable.push_back(num);
                    // 添加INTEGER_LITERAL类型的token
                    tokens.push_back({INTEGER_LITERAL, index,lineNumber, column});
                }  
            } 
            // 字符串字面量识别
            else if (line[i] == '"') {
                std::string str;
                int startColumn = column;
                bool terminated = false;
                i++; // 跳过开始的引号
                
                while (i < line.size()) {
                    if (line[i] == '"') {
                        terminated = true;
                        i++;
                        break;
                    }
                    str += line[i++];
                }
                
                if (!terminated) {
                    addError(LexErrorType::UNTERMINATED_STRING, lineNumber, startColumn, '"');
                } else {
                    // 处理字符串常量，添加到常量表
                    auto it = std::find(constantTable.begin(), constantTable.end(), str);
                    int index = it == constantTable.end() ? constantTable.size() : std::distance(constantTable.begin(), it);
                    if (it == constantTable.end()) constantTable.push_back(str);
                    tokens.push_back({STRING_LITERAL, index,lineNumber, startColumn});
                }
            }
            else if (isOperator(line[i])) {
                // 运算符识别，直接记录字符的ASCII码作为token值
                tokens.push_back({OPERATOR, (int)line[i++], lineNumber, column});
            } else if (isDelimiter(line[i])) {
                // 分隔符识别，直接记录字符的ASCII码作为token值
                tokens.push_back({DELIMITER, (int)line[i++],lineNumber, column});
            } else if (line[i] == '$') {
                // 文件结束符识别
                tokens.push_back({END_OF_FILE, -1, lineNumber, column});
                ++i;
            } else if (line[i] == '/') {
                // 注释处理
                if (i + 1 < line.size() && line[i + 1] == '/') {
                    break; // 跳过行注释
                }
            } else {
                // 无法识别的字符，记录为错误
                addError(LexErrorType::UNKNOWN_CHAR, lineNumber, column, line[i]);
                ++i;
            }
        }
        ++lineNumber;
    }

    // 分析结束后打印所有错误
    if (!errorList.empty()) {
        printErrors();
    }

    return tokens;
}
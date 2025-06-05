# WHU编译原理大作业
简化版 MiniJava 语言的编译器
## 项目结构
1. res（结果输出）:
  - ast.dot:可视化抽象语法树
  - constant_table.txt:常量表
  - identifier_table.txt:标识符表
  - ir.txt：中间代码生成结果，四元式
  - tokens.txt:tokens流
2. src（源文件）:
  - lex.cpp:词法分析程序
  - parser.cpp：语法分析程序
  - semantic.cpp:语义分析程序
  - irgen.cpp：中间代码生成程序
  - ast_visualize.cpp：AST可视化程序
  - main.cpp：主程序
3. test（测试文件）
# 编译
```
g++ -std=c++11 -o miniJava main.cpp lex.cpp parser.cpp semantic.cpp irgen.cpp ast_visualize.cpp
```
# 运行
```
./test test_parser.txt
```
# 查看抽象语法树
```
xdot ast.dot
```
![image](https://github.com/user-attachments/assets/b2116964-6336-40ca-9d92-a61a08376c5b)

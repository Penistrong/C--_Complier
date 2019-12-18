/*
* @Author:	Penistrong
* @Date:	2019-12-17 21:36:27
* @Last Modified by:	Penistrong
* @Last Modified time:	2019-12-17 21:36:27
* @Description: Exp3 TAC code generation
*/
#ifndef _TAC_H
#define _TAC_H
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semanticAnalyzer.h"

FILE* err_out;
FILE* TAC_out;

struct opn{
    int type;                   //操作数实际数据类型
    int kind;                   //标明联合成员的属性
    union member
    {
        int const_int;
        float const_float;
        double const_double;
        char const_char;
        char id[33];            //变量|临时变量的别名or标号字符串
    };
    int level;          //层号
    int offset;         //偏移量
};

typedef struct TACnode{
    int op;                         //操作类型说明
    struct opn *opn1, *opn2, *result;  //操作数1,操作数2,运算结果
    struct TACnode *next, *prior;
}*pTACnode;

struct XASTnode{
    struct ASTnode ori;
    struct TACnode tac_head;        //拓展AST节点拥有的中间代码序列头指针
    char *Jwbt, *Jwbf;              //当表达式解析为bool结果时,标明Jump的标号,Jump while bool true|false
};

int initTACgenerator();
struct opn* newOpn();
pTACnode newTACnode();
pTACnode generateTAC(int op, int pNum, ...);

int TAC_Traversal(struct ASTnode* root);

//Assume "generator analyze" as GA
int GA_ExtDef(struct ASTnode* extDef);

enum OperationType{
    LABEL,
    FUNCTION,
    ASSIGN,
    ADD,
    SUB,
    MUL,
    DIV,
    GOTO,
    PARAM,
    ARG,
    CALL,
    AND,
    OR,
    NOT,
    JG,
    JGE,
    JL,
    JLE,
    EQ,
    NEQ,
};



#endif
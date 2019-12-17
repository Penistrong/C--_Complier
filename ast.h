#include <stdio.h>
#include <string.h>
#include "singleParser.tab.h"
#include <stdarg.h>
#include <stdlib.h>

#define MAX_BUFFER_SIZE 100

#ifndef _AST_H
#define _AST_H

struct ASTnode{
    int kind;
    int pos;                            //行数，标明在第几行
    char* name;                         //节点类型名
    char* semVal;                       //节点语义值
    union {
          char type_id[33];             //由标识符生成的叶结点
          int type_int;                 //由整常数生成的叶结点
          float type_float;             //由单精度浮点常数生成的叶结点
          double type_double;           //由双精度浮点常数生成的叶结点
          };
    struct ASTnode *childNode[8];            //子树指针，最多8个，由kind决定
};

struct ASTnode *createASTnode(int num, int kind, char* name, int pos, ...);

void printAST(struct ASTnode *root, int lvl, int prelvl, char* prefix, int hasBro);
unsigned int hashCode(char *str);

//非终结符标识
//NT=NoTerminal，非终结符
enum yyNTtype{
    NT = 999,
    PROGRAM = 400,
    EXTDEFLIST = 401,
    EXTDEF = 402,
    SPECIFIER = 403,
    STRUCTSPECIFIER = 404,
    OPTSTRUCTNAME = 405,
    STRUCTDECL = 406,
    EXTDECLIST = 407,
    FUNCDEC = 408,
    COMPST = 409,
    VARLIST = 410,
    VARDEC = 411,
    PARAMDEC = 412,
    STMT = 413,
    STMLIST = 414,
    DEFLIST = 415,
    DEF = 416,
    DECLIST = 417,
    DEC = 418,
    EXP = 419,
    ARGS = 420,
    ARRAYDEC = 421
};

#endif

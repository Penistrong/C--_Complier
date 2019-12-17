/*
* @Author: Penistrong
* @Date:   2019-12-03 11:06:21
* @Last Modified by:   Penistrong
* @Last Modified time: 2019-12-03 11:06:25
*/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifndef _SEMANTICANALYZER_H
#define _SEMANTICANALYZER_H
#include "ast.h"
#include "stack.h"

#define NONE         "\033[m"
#define RED          "\033[0;32;31m"
#define LIGHT_RED    "\033[1;31m"
#define GREEN        "\033[0;32;32m"
#define LIGHT_GREEN  "\033[1;32m"
#define BLUE         "\033[0;32;34m"
#define LIGHT_BLUE   "\033[1;34m"
#define DARY_GRAY    "\033[1;30m"
#define CYAN         "\033[0;36m"
#define LIGHT_CYAN   "\033[1;36m"
#define PURPLE       "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN        "\033[0;33m"
#define YELLOW       "\033[1;33m"
#define LIGHT_GRAY   "\033[0;37m"
#define WHITE        "\033[1;37m"

//变量符号表节点
typedef struct Var_Symbol
{
    char* name;                         //变量名
    char* type;                         //变量类型
    int line;                           //变量所在行数
    int lvl;                            //变量所在作用域层号
    struct Var_Symbol *next;            //指向下一个变量符号
}Var, *pVar, *pVar_ST_Head, *pVar_ST_Tail;

//函数符号表节点
typedef struct Func_Symbol
{
    int isDef;          //1表示定义
    char* name;         //函数名
    char* fType;        //函数返回值类型
    char* rType;        //实际返回值类型
    int paramNum;       //形参个数
    pVar paramList;     //形参列表
    int line;           //函数定义的位置
    struct Func_Symbol *next;           //指向下一个函数符号
}*pFunc, *pFunc_ST_Head, *pFunc_ST_Tail;

//数组符号表节点
typedef struct Array_Symbol
{
    char* name;         //数组名
    int type;           //数组类型
    struct Array_Symbol *next;          //指向下一个数组符号
}*pArray, *pArray_ST_Head, *pArray_ST_Tail;

//结构体符号表节点
typedef struct Struct_Symbol
{
    char* name;         //结构体名
    int line;           //结构体定义/声明位置
    int memberNum;      //成员个数
    pVar memberList;    //结构体成员列表
    struct Struct_Symbol *next;         //指向下一个结构体符号
}*pStruct, *pStruct_ST_Head, *pStruct_ST_Tail;

//作用域符号表，包括以上四种符号表
typedef struct Scope_Symbol_Table
{
    char* scope_name;
    pVar_ST_Head var_st;
    pFunc_ST_Head func_st;          //通常新建函数符号表时会新建一个作用域
    pArray_ST_Head array_st;
    pStruct_ST_Head struct_st;
}Scope_ST, *pScope_ST;

void initializeSymbolTable();
pScope_ST newScope_ST();

void semantic_Analyze(struct ASTnode *root);
void AST_Traversal(struct ASTnode *root);

int isVarDef(struct ASTnode *ID);
int isVarDec(struct ASTnode *ID);
char* typeVar(struct ASTnode* ID);
int newVarSymbol(int num, char* type, ...);
pVar createNewVar();

int isFuncDef(struct ASTnode* ID);
char* typeFunc(struct ASTnode* ID);
pVar plistFunc(struct ASTnode* ID);
int setFuncRtype();
int newFuncSymbol(char* type, char* name, int line, int paramNum, ...);
pFunc createNewFunc();

int isStrucDef(struct ASTnode* ID);
int newStrucSymbol(char* name, int line, int memberNum, ...);
pStruct createNewStruct();

int analyzeExtDef(struct ASTnode *extDef);
int analyzeStrucSpec(struct ASTnode *strucSpec);
int analyzeCompSt(struct ASTnode *compSt, int isInBlock, ...);
int analyzeStmt(struct ASTnode *stmt,int isInBlock, ...);
char* analyzeExp(struct ASTnode *exp);
int analyzeReturn(struct ASTnode * rStmt);

void printScope_ST(pScope_ST scope_st);
void printVar_ST(pVar_ST_Head var_ST_head);


#endif
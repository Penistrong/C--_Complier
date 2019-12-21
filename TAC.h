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
#define DX 3*sizeof(int)    //函数活动记录控制信息大小

FILE* err_out;
FILE* TAC_out;

struct opn{
    int type;                   //操作数实际数据类型
    int kind;                   //标明联合成员的属性
    union{
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
    //struct ASTnode ori;
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
    //Upper is the original ASTnode struct definition except 'childNode'
    struct XASTnode *childNode[8];            //子树指针，最多8个，由kind决定
    struct TACnode *tac_head;        //拓展AST节点拥有的中间代码序列头指针
    char *Jwbt, *Jwbf;              //当表达式解析为bool结果时,标明Jump的标号,Jump while bool true|false
    char *Snext;                    //当前节点对于语句执行结束后下一条语句位置
    int offset;                     //偏移量
    int width;                      //占数据宽度/字节数
    int num;                        //计数器,用来统计形参数量或者一次性定义的多个变量个数
    int place;                      //标识符节点在符号表中的位置,与符号表中的index对应
};

typedef struct SymbolTable{
    struct Var_Symbol * var_symbol;
    struct Func_Symbol * func_symbol;
    int vindex; //变量符号表当前索引位置
    int findex; //函数符号表当前索引位置
}TAC_ST;

char* _strcat_(char* des, char* src);

int initSymbolTable();
int initTACgenerator();
struct opn* newOpn();
pTACnode newTACnode();
pTACnode generateTAC(int op, int pNum, ...);
pTACnode mergeTAC(int num, ...);

int calWidth(int kind, ...);

struct XASTnode* expandAST(struct ASTnode* root);
struct XASTnode* createXASTnode(struct ASTnode* root);

int TAC_Traversal(struct XASTnode* root);
void printXAST(struct XASTnode *root, int lvl, int prelvl, char* prefix, int hasBro);
int fill_ST(int symbol_kind, int num, char* type, int offset, ...);

//自动生成标记、别名、临时变量名,用于填写符号表时按序自增相应量
char* auto_Alias();
char* auto_Temp();
char* auto_Label();

//Assume "generator analyze" as GA
int GA_ExtDef(struct XASTnode* extDef);

void printTAC_ST();

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

enum SymbolType{
    VAR_SYMBOL,
    FUNC_SYMBOL
};


#endif
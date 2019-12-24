%error-verbose
%locations
%{

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "ast.h"
#include "semanticAnalyzer.h"

#define YYDEBUG 1

extern int yylineno;
extern FILE* yyin;
extern FILE* yyout;

int analyzeType(char* type);
void yyerror(const char* fmt, ...);
%}

/*Declared Types 词法单元类型定义*/
%union{
    int type_int;
    float type_float;
    double type_double;

    char type_id[32];
    struct ASTnode *node;
}

/*定义非终结符语义值类型*/
%type <node> Program ExtDefList ExtDef Specifier StructSpecifier OptStructName StructDecl ExtDecList FuncDec CompSt VarList VarDec ArrayDec ParamDec Stmt StmList DefList Def DecList Dec Exp Args

/*Declared Tokens 定义终结符语义值类型*/
%token <node> INT FLOAT DOUBLE
%token <node> VOID CHAR BOOL
%token <node> STRUCT
%token <node> ID TYPE
%token <node> RELOP

/*终结符操作符定义*/
%token <node> ASSIGNOP ADD_OP SUB_OP MUL_OP DIV_OP AND_OP OR_OP NOT_OP INC_OP DEC_OP

/*逻辑判断终结符*/
%token <node> EQ_OP

/*终结符逻辑控制关键字定义*/
%token <node> IF ELSE DO WHILE BREAK RETURN FOR SWITCH CASE CONTINUE

/*界符定义*/
%token <node> SEMI COMMA LP RP LC RC LB RB COLON

%start  Program

/*定义终结符结合性*/
%left ASSIGNOP
%left OR_OP
%left AND_OP
%left RELOP EQ_OP
%left ADD_OP SUB_OP
%left MUL_OP DIV_OP
%right NOT_OP INC_OP DEC_OP

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

/*每条产生式的语义动作会在整条产生式归约完成后执行 $[x]指向的是语法单元 x是其位置*/
/*开始符:Program*/
Program
        : ExtDefList { $$=createASTnode(1, PROGRAM, "Program", yylineno, $1);printAST($$, 0, -1, NULL, 0); fflush(yyout); semantic_Analyze($$);/*待定:开启语义分析*/}
        ; //开始符的产生式最后才归约完成, 打印AST也是建完树后最后才执行

ExtDefList
        : { $$=NULL;}
        | ExtDef ExtDefList { $$=createASTnode(2, EXTDEFLIST, "ExtDefList", yylineno, $1, $2); }
        ; //每一个EXTDEFLIST的结点，其第1棵子树对应一个外部变量声明或函数

ExtDef
        : Specifier ExtDecList SEMI { $$=createASTnode(3, EXTDEF, "ExtDef", yylineno, $1, $2, $3); }
            //该节点对应一个外部变量声明
        | Specifier SEMI { $$=createASTnode(2, EXTDEF, "ExtDef", yylineno, $1, $2); }
            //该节点对应一个结构体定义
        | Specifier FuncDec CompSt { $$=createASTnode(3, EXTDEF, "ExtDef", yylineno, $1, $2, $3); }
            //该节点对应一个函数定义
        | error SEMI { $$=NULL; } //分号作为开始，出现错误
        ;

Specifier
        : TYPE { $$=createASTnode(1, SPECIFIER, "Specifier", yylineno, $1); }
            //Specifier指向 类型 终结符, 本节点对应语法单元的实际类型要靠analyzeType()决定
        | StructSpecifier { $$=createASTnode(1, SPECIFIER, "Specifier", yylineno, $1); }
        ;

StructSpecifier
        : STRUCT OptStructName LC DefList RC { $$=createASTnode(5, STRUCTSPECIFIER, "StructSpecifier", yylineno, $1, $2, $3, $4, $5); }
        | STRUCT StructDecl { $$=createASTnode(2, STRUCTSPECIFIER, "StructSpecifier", yylineno, $1, $2); }
        ;

OptStructName
        : { $$=createASTnode(0, OPTSTRUCTNAME, "OptStructName: None-Anonymous Struct", yylineno); /*匿名结构体定义*/}
        | ID { $$=createASTnode(1, OPTSTRUCTNAME, "OptStructName", yylineno, $1); /*常规结构体定义*/}
        ;

StructDecl
        : ID { $$=createASTnode(1, STRUCTDECL, "StructDecl", yylineno, $1); }

ExtDecList
        : VarDec { $$=createASTnode(1, EXTDECLIST, "ExtDecList", yylineno, $1); }
            //每一个EXTDECLIST节点，它的第一颗子树对应一个变量名(即标识符),第二颗子树对应其他变量名
        | VarDec COMMA ExtDecList { $$=createASTnode(3, EXTDECLIST, "ExtDecList", yylineno, $1, $2, $3); }
        ;

VarDec
        : ID { $$=createASTnode(1, VARDEC, "VarDec", yylineno, $1);}
        | ArrayDec LB INT RB { $$=createASTnode(4, VARDEC, "VarDec", yylineno, $1, $2, $3, $4); }
        ;

ArrayDec
        : ID { $$=createASTnode(1, ARRAYDEC, "ArrayDec", yylineno, $1);}
        ;
FuncDec
        : ID LP VarList RP { $$=createASTnode(4, FUNCDEC, "FuncDec", yylineno, $1, $2, $3, $4);}//带参数列表(1+)函数
        | ID LP RP { $$=createASTnode(3, FUNCDEC, "FuncDec", yylineno, $1, $2, $3);}//无参函数
        ;

VarList
        : ParamDec { $$=createASTnode(1, VARLIST, "VarList", yylineno, $1); }
        | ParamDec COMMA VarList { $$=createASTnode(3, VARLIST, "VarList", yylineno, $1, $2, $3); }
        ;

ParamDec
        : Specifier VarDec { $$=createASTnode(2, PARAMDEC, "ParamDec", yylineno, $1, $2); }
        ;

CompSt
        : LC DefList StmList RC { $$=createASTnode(4, COMPST, "CompSt", yylineno, $1, $2, $3, $4); }
        ;

StmList
        : { $$=NULL; }
        | Stmt StmList { $$=createASTnode(2, STMLIST, "StmList", yylineno, $1, $2); }
        ;

Stmt
        : Exp SEMI   { $$=createASTnode(2, STMT, "Stmt", yylineno, $1, $2); }
        | CompSt { $$=createASTnode(1, STMT, "Stmt", yylineno, $1); } //复合语句节点直接作为语句节点，不再生成新的节点
        | RETURN Exp SEMI { $$=createASTnode(3, STMT, "Stmt", yylineno, $1, $2, $3); }
        | BREAK SEMI { $$=createASTnode(2, STMT, "Stmt", yylineno, $1, $2); }
        | CONTINUE SEMI { $$=createASTnode(2, STMT, "Stmt", yylineno, $1, $2); }
        | IF LP Exp RP Stmt %prec LOWER_THAN_ELSE { $$=createASTnode(5, STMT, "Stmt", yylineno, $1, $2, $3, $4, $5); }
        | IF LP Exp RP Stmt ELSE Stmt { $$=createASTnode(7, STMT, "Stmt", yylineno, $1, $2, $3, $4, $5, $6, $7); }
        | WHILE LP Exp RP Stmt { $$=createASTnode(5, STMT, "Stmt", yylineno, $1, $2, $3, $4, $5); }
        | SWITCH LP Exp RP Stmt { $$=createASTnode(5, STMT, "Stmt", yylineno, $1, $2, $3, $4, $5); }
        | DO Stmt WHILE LP Exp RP SEMI { $$=createASTnode(7, STMT, "Stmt", yylineno, $1, $2, $3, $4, $5, $6, $7); }
        ;

DefList
        : { $$=NULL; }
        | Def DefList { $$=createASTnode(2, DEFLIST, "DefList", yylineno, $1, $2);}
        | error SEMI { $$=NULL; }
        ;

Def
        : Specifier DecList SEMI {$$=createASTnode(3, DEF, "Def", yylineno, $1, $2, $3); }
        ;

DecList
        : Dec {$$=createASTnode(1, DECLIST, "DecList", yylineno, $1); }
        | Dec COMMA DecList { $$=createASTnode(3, DECLIST, "DecList", yylineno, $1, $2, $3); }
        ;

Dec
        : VarDec { $$=createASTnode(1, DEC, "Dec", yylineno, $1); }
        | VarDec ASSIGNOP Exp { $$=createASTnode(3, DEC, "Dec", yylineno, $1, $2, $3); }
        ;

Exp
        : Exp ASSIGNOP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | Exp AND_OP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | Exp OR_OP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | NOT_OP Exp { $$=createASTnode(2, EXP, "Exp", yylineno, $1, $2); }
        | Exp RELOP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | Exp EQ_OP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | Exp ADD_OP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | Exp SUB_OP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | Exp MUL_OP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | Exp DIV_OP Exp { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | INC_OP Exp { $$=createASTnode(2, EXP, "Exp", yylineno, $1, $2); }
        | Exp INC_OP { $$=createASTnode(2, EXP, "Exp", yylineno, $1, $2); }
        | DEC_OP Exp { $$=createASTnode(2, EXP, "Exp", yylineno, $1, $2); }
        | Exp DEC_OP { $$=createASTnode(2, EXP, "Exp", yylineno, $1, $2); }
        | LP Exp RP { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | ID LP RP { $$=createASTnode(3, EXP, "Exp", yylineno, $1, $2, $3); }
        | ID LP Args RP { $$=createASTnode(4, EXP, "Exp", yylineno, $1, $2, $3, $4); }
        | ID { $$=createASTnode(1,EXP,"Exp", yylineno, $1); }
        | INT { $$=createASTnode(1,EXP,"Exp", yylineno, $1); }
        | FLOAT { $$=createASTnode(1,EXP,"Exp", yylineno, $1); }
        | DOUBLE { $$=createASTnode(1,EXP,"Exp", yylineno, $1); }
        ;

Args
        : Exp COMMA Args { $$=createASTnode(3, ARGS, "Args", yylineno, $1, $2, $3); }
        | Exp { $$=createASTnode(1, ARGS, "Args", yylineno, $1); }
        ;


%%

int main(int argc, char** argv){
    if(!(yyin=fopen(argv[1], "r")))
        return 1;
    printf("Input FileName: %s\n", argv[1]);
    yylineno = 1;//初始化yylineno
    char* outputPath = "./syntaxTree.txt";
    if((yyout = fopen(outputPath, "w")) == NULL)
        return 1;
    yyparse();//开始语法分析
    return 0;
}

//变长参数列表
void yyerror(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "Error type B at Line %d: [Grammar Error Start@Column: %d]", yylloc.first_line, yylloc.first_column);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, ".\n");
}
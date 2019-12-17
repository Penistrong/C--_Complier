%{
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "singleParser.tab.h"
#include "ast.h"

char* _strupr_d(char* src);

/*维护语法单元位置，供BISON使用*/
int yycolumn = 1;

/*yylloc是bison内置变量，表示当前语法单元对应词法单元的位置信息*/
#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

typedef union {
    int type_int;
    int type_float;
    char type_id[32];
    struct ASTnode *node;
} YYLVAL;
#define YYSTYPE YYLVAL

int reserved2Type(char* s);

%}

%option yylineno
%x      BLOCK_COMMENT

O       [0-7]
DIGIT   [0-9]
NZD     [1-9]
H       [a-fA-F0-9]
HP      0[xX]
LETTER  [A-Za-z_]
A       [A-Za-z_0-9]

NUMBER  [+-]?({DIGIT}*\.?{DIGIT}+|{DIGIT}+\.)
ID      {LETTER}{A}*
SPACE   [ \t\r\f]

RESERVED    if|else|do|while|break|continue|return|switch|case|void|int|float|double|char|bool|struct
OPERATOR    "+"|"-"|"*"|"/"|"!"|"<"|"<="|">"|">="|"=="|"!="|"="|"&&"|"||"|"++"|"--"
DELIMITER   ";"|","|"("|")"|"["|"]"|"{"|"}"|"/*"|"*/"|"//"|":"


%%

"/*"    { printf("LBC|开启块注释\t\t\t%s\n", yytext); BEGIN BLOCK_COMMENT;}

<BLOCK_COMMENT>"*/" { printf("RBC|结束块注释\t\t\t%s\n", yytext); BEGIN INITIAL;}
<BLOCK_COMMENT>.    { /*注释状态下的所有字符均不参与词法分析，直接过滤*/ }

"//".*    { printf("LINEC|开启单行注释\t\t\t%s\n", yytext); }

\n      { yycolumn = 1; /*发现换行符后复位列,维护位置信息*/}

{SPACE} { /*Just do nothing as a '\s' filter*/}

{RESERVED}  {   /*strupr(yytext)*/
                printf("Reserved|%s\t\t\t%s\n", _strupr_d(yytext), yytext);
                return reserved2Type(yytext);
            }

{NUMBER}    {
                printf("Constant|");
                REJECT;/*转向下方实际数字的匹配*/
            }

{HP}{H}+                { printf("INT|HEXICAL\t\t\t%s\n", yytext); yylval.node=createASTnode(0, INT,"INT", yylineno); yylval.node->type_int=atoi(yytext); return INT;}
"-"?{NZD}{DIGIT}*       { printf("INT|DECIMAL\t\t\t%s\n", yytext); yylval.node=createASTnode(0, INT,"INT", yylineno); yylval.node->type_int=atoi(yytext); return INT;}
"0"                     { printf("INT|ZERO\t\t\t%s\n", yytext); yylval.node=createASTnode(0, INT,"INT", yylineno); yylval.node->type_int=atoi(yytext); return INT;}
"-"?{DIGIT}+"."{DIGIT}+ { printf("FLOAT|DECIMAL\t\t\t%s\n", yytext); yylval.node=createASTnode(0, FLOAT,"FLOAT", yylineno); yylval.node->type_float=atof(yytext); return FLOAT;}

{ID}    {
            printf("Identifier|VAR\t\t\t%s\n",yytext);
            yylval.node = createASTnode(0, ID, "ID", yylineno);
            strcpy(yylval.node->type_id,  yytext);
            return ID;
        }

{DELIMITER} {
                printf("Delimiter|");
                REJECT;/*转向下方实际界符的匹配*/
            }

[;] { printf("SEMI|分号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, SEMI, "SEMI", yylineno); return SEMI;}
[,] { printf("COMMA|逗号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, COMMA, "COMMA", yylineno); return COMMA;}
[(] { printf("LP|左括号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, LP, "LP", yylineno); return LP;}
[)] { printf("RP|右括号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, RP, "RP", yylineno); return RP;}
[{] { printf("LC|左花括号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, LC, "LC", yylineno); return LC;}
[}] { printf("RC|右花括号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, RC, "RC", yylineno); return RC;}
\[  { printf("LB|左中括号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, LB, "LB", yylineno); return LB;}
\]  { printf("RB|右中括号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, RB, "RB", yylineno); return RB;}
[:] { printf("COLON|冒号\t\t\t%s\n", yytext); yylval.node=createASTnode(0, COLON, "COLON", yylineno); return COLON;}

{OPERATOR}  {
                printf("OPERATOR|");
                REJECT;/*转向下方实际操作符的匹配*/
            }

"==" {printf("RELOP|EQ_OP\t\t\t%s\n", yytext);yylval.node=createASTnode(0, EQ_OP, "EQ_OP", yylineno);return EQ_OP;/*从RELOP中拆分出了 == (EQ_OP)*/}

">"|"<"|">="|"<="|"!=" {
                                printf("RELOP|条件判断\t\t\t%s\n", yytext);
                                yylval.node=createASTnode(0, RELOP, "RELOP", yylineno);
                                strcpy(yylval.node->type_id, yytext);
                                printf("已建立RELOP叶结点,实际逻辑表达式为: %s\n", yylval.node->type_id);
                                return RELOP;
                            }
"++"    { printf("INC|自增\t\t\t%s\n", yytext); yylval.node=createASTnode(0, INC_OP, "INC_OP", yylineno);return INC_OP;}
"--"    { printf("DEC|自减\t\t\t%s\n", yytext); yylval.node=createASTnode(0, DEC_OP, "DEC_OP", yylineno);return DEC_OP;}
"=" { printf("ASSIGNOP|赋值\t\t\t%s\n", yytext); yylval.node=createASTnode(0, ASSIGNOP, "ASSIGNOP", yylineno); return ASSIGNOP;}
"+" { printf("ADD|加\t\t\t%s\n", yytext); yylval.node=createASTnode(0, ADD_OP, "ADD_OP", yylineno);return ADD_OP;}
"-" { printf("SUB|减\t\t\t%s\n", yytext); yylval.node=createASTnode(0, SUB_OP, "SUB_OP", yylineno);return SUB_OP;}
"*" { printf("MUL|乘\t\t\t%s\n", yytext); yylval.node=createASTnode(0, MUL_OP, "MUL_OP", yylineno);return MUL_OP;}
"/" { printf("DIV|除\t\t\t%s\n", yytext); yylval.node=createASTnode(0, DIV_OP, "DIV_OP", yylineno);return DIV_OP;}
"&&"    { printf("AND|与\t\t\t%s\n", yytext); yylval.node=createASTnode(0, AND_OP, "AND_OP", yylineno);return AND_OP;}
"||"    { printf("OR |或\t\t\t%s\n", yytext); yylval.node=createASTnode(0, OR_OP, "OR_OP", yylineno);return OR_OP;}
"!"     { printf("NOT|非\t\t\t%s\n", yytext); yylval.node=createASTnode(0, NOT_OP, "NOT_OP", yylineno);return NOT_OP;}



.   { printf("Error type A at Line %d :[Lexical error]Mysterious character \'%s\'\n", yylineno, yytext); }

%%

int reserved2Type(char* s)
{
    if(strcmp(s,"if")==0){
        yylval.node = createASTnode(0, IF, "IF", yylineno);
        return IF;
    }
    else if(strcmp(s,"else")==0){
        yylval.node = createASTnode(0, ELSE, "ELSE", yylineno);
        return ELSE;
    }
    else if(strcmp(s,"do")==0){
        yylval.node = createASTnode(0, DO, "DO", yylineno);
        return DO;
    }
    else if(strcmp(s,"while")==0){
        yylval.node = createASTnode(0, WHILE, "WHILE", yylineno);
        return WHILE;
    }
    else if(strcmp(s,"break")==0){
        yylval.node = createASTnode(0, BREAK, "BREAK", yylineno);
        return BREAK;
    }
    else if(strcmp(s,"continue")==0){
        yylval.node = createASTnode(0, CONTINUE, "CONTINUE", yylineno);
        return CONTINUE;
    }
    else if(strcmp(s,"return")==0){
        yylval.node = createASTnode(0, RETURN, "RETURN", yylineno);
        return RETURN;
    }
    else if(strcmp(s, "struct")==0){
        yylval.node = createASTnode(0, STRUCT, "Struct", yylineno);
        return STRUCT;
    }
    //for switch case continue 待完成

    //变量类型保留字及其值传递
    yylval.node = createASTnode(0, TYPE, "TYPE", yylineno);
    if(strcmp(s,"int")==0){
        strcpy(yylval.node->type_id,  yytext);
        return TYPE;
    }
    else if(strcmp(s,"float")==0){
        strcpy(yylval.node->type_id,  yytext);
        return TYPE;
    }
    else if(strcmp(s,"double")==0){
        strcpy(yylval.node->type_id,  yytext);
        return TYPE;
    }
    else if(strcmp(s,"char")==0){
        strcpy(yylval.node->type_id,  yytext);
        return TYPE;
    }
    else if(strcmp(s,"bool")==0){
        strcpy(yylval.node->type_id,  yytext);
        return TYPE;
    }
    else if(strcmp(s,"void")==0){
        strcpy(yylval.node->type_id,  yytext);
        return TYPE;
    }
}


/*字符串转大写且不改变原串值*/
char* _strupr_d(char* src)
{
    char* temp = (char*)malloc(sizeof(char)*20);
    strcpy(temp, src);
    strupr(temp);
    return temp;
}

/*以下部分与bison联用时不需要,paser.y里有主程序入口*/
/*int main(int argc, char** argv)
{
    if(argc>1){
        if(!(yyin = fopen(argv[1], "r"))){
            perror(argv[1]);
            return 1;
        }
    }
    char* outputPath="../product/parser_out.txt";
    printf("文件名:%s\n",argv[1]);
    //freopen(outputPath,"w",stdout);//输出重定向
    return yylex();
}*/

//此函数必须由用户提供
int yywrap()
{
    return 1;
}
/*
* @Author: Penistrong
* @Date:   2019-11-27 12:29:25
* @Last Modified by:   Penistrong
* @Last Modified time: 2019-12-06 13:42:42
*/
#include "ast.h"
#include "singleParser.tab.h"

extern FILE* yyout; //lex输出文件流

struct ASTnode * createASTnode(int num, int kind, char* name, int pos, ...){
    //使用calloc而不是malloc,前者分配内存空间后会将每一位都初始化为0
    struct ASTnode *root = (struct ASTnode *)calloc(sizeof(struct ASTnode), 1);
    if(!root){
        printf("Memory is out of space...yyparse() is terminated.");
        exit(0);
    }
    root->kind = kind;
    root->name = (char*)malloc(sizeof(char)*20);
    strcpy(root->name, name);

    va_list pArgs = NULL;
    va_start(pArgs, pos);
    int i = 0;
    for (i = 0; i<num; i++)
        root->childNode[i] = va_arg(pArgs, struct ASTnode *);
    //将未用到的子树指针置空
    while (i<8) root->childNode[i++] = NULL;
    va_end(pArgs);

    //对于位置而言应该与从左数第一个孩子的位置为准
    if(root->childNode[0] == NULL) //为终结符节点，即叶节点，所在行号就是其实际位置
        root->pos = pos;
    else
        root->pos = root->childNode[0]->pos;

    return root;
}

//BKDRHash算法,计算字符串散列值,用以switch/case语句switch字符串
unsigned int hashCode(char *str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    while (*str)
    {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

//先序遍历抽象语法树AST
void printAST(struct ASTnode *root, int lvl, int prelvl, char* prefix, int hasBro){
    if(root == NULL)
        return;
    if(prefix == NULL){
        prefix = (char*)calloc(sizeof(char)*MAX_BUFFER_SIZE, 1);
        strcpy(prefix,"");
    }
    char* tempPreFix = (char*)calloc(sizeof(char)*MAX_BUFFER_SIZE, 1);
    strcpy(tempPreFix, prefix);
    fprintf(yyout, "%s", prefix);
    //打印枝干同时给前缀字符串拼接枝干
    if(prelvl != -1){
        if(hasBro)
            strcat(tempPreFix, "│  ");
        else
            strcat(tempPreFix,"    ");
        //由于不是双向链表存储，无法向前查看父节点有无其他子节点，即有无兄弟节点
        //用hasBro作控制, 在上一步里由父节点确定
        if(hasBro == 0)
            fprintf(yyout, "└─");
        else
            fprintf(yyout, "├─");
    }

    //对标识符和内置类型
    switch(root->kind){
    case ID:
        fprintf(yyout, "ID:  %s\n", root->type_id);
        break;
    case TYPE:
        fprintf(yyout, "TYPE:  %s\n", root->type_id);
        break;
    case RELOP:
        fprintf(yyout, "RELOP:[%s]\n", root->type_id);
        break;
    case INT:
        fprintf(yyout, "%s:  %d\n", root->name, root->type_int);
        break;
    case FLOAT:
        fprintf(yyout, "%s:  %f\n", root->name, root->type_float);
        break;
    case DOUBLE:
        fprintf(yyout, "%s:  %f\n", root->name, root->type_double);
        break;
    default:
        if(root->name == NULL)
            fprintf(yyout, "[ERROR]:此处竟然为空，其kind为: %d\n", root->kind);
        fprintf(yyout, "%s  (%d)\n", root->name, root->pos);
        break;
    }

    //开始遍历
    for(int i = 0;i < 8;i++)
        if(root->childNode[i]!=NULL)
            if(i != 7)
                printAST(root->childNode[i], lvl+1, lvl, tempPreFix, root->childNode[i+1]!=NULL?1:0);
            else
                printAST(root->childNode[i], lvl+1, lvl, tempPreFix, 0);
}
/*
* @Author:	Penistrong
* @Date:	2019-12-17 21:45:21
* @Last Modified by:	Penistrong
* @Last Modified time:	2019-12-17 21:45:21
* @Description: Exp3 TAC code generation.Assume there is no error in semantic analysis then call TAC generator.
*/
#include "TAC.h"

int initTACgenerator(){
    if((err_out = fopen("./error.output","w")) == NULL)
        return 0;
    if((TAC_out = fopen("./TACresult.txt","w")) == NULL)
        return 0;
    fprintf(err_out, RED"TAC generator has initialized..."NONE"\n");
    return 1;
}

struct opn* newOpn(){
    struct opn* newOpn = (struct opn *)malloc(sizeof(struct opn));
}

pTACnode newTACnode(){
    pTACnode newTACnode = (pTACnode)malloc(sizeof(struct TACnode));
    newTACnode->opn1 = newTACnode->opn2 = newTACnode->result = NULL;
    newTACnode->next = newTACnode->prior = NULL;
    return newTACnode;
}

//生成一个TAC的元组,返回一个未插入双向列表的一个TAC结点
pTACnode generateTAC(int op, int pNum, ...){
    pTACnode TACnode = newTACnode();
    TACnode->op = op;
    va_list pArgs = NULL;
    va_start(pArgs, pNum);
    if(pNum == 1){
        char* label = va_arg(pArgs, char*);
        switch(op){
        case LABEL:
            TACnode->op = LABEL;
            TACnode->result = newOpn();
            strcpy(TACnode->result->id, label);
            break;
        case GOTO:
            TACnode->op = GOTO;
            TACnode->result = newOpn();
            strcpy(TACnode->result->id, label);
            break;
        default:
            fprintf(err_out, RED"Fatal Error: TAC genertor has encountered\n");
        }
    }else{
        TACnode->op = op;
        TACnode->opn1 = va_arg(pArgs, struct TACnode*);
        TACnode->opn2 = va_arg(pArgs, struct TACnode*);
        TACnode->result = va_arg(pArgs, struct TACnode*);
    }
    va_end(pArgs);
}

//TAC生成器周游AST时,已经假定没有任何语义错误存在,故而直接生成TAC代码而不加检错
int TAC_Traversal(struct ASTnode* root){
    if(root == NULL)
        return 0;
    switch(root->kind){
    case EXTDEF:
        break;
    default:
        for(int i = 0;i<8;i++)
            TAC_Traversal(root->childNode[0]);
    }
}

int GA_ExtDef(struct ASTnode* extDef){
    struct ASTnode* spec = extDef->childNode[0];
    struct ASTnode* ID;
    char* type;
    switch(extDef->childNode[1]->kind){
    case EXTDECLIST:
        break;
    case FUNCDEC:
        break;
    case SEMI:
        break;
    }
}
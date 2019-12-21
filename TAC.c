/*
* @Author:	Penistrong
* @Date:	2019-12-17 21:45:21
* @Last Modified by:	Penistrong
* @Last Modified time:	2019-12-17 21:45:21
* @Description: Exp3 TAC code generation.Assume there is no error in semantic analysis then call TAC generator.
*/
#include "TAC.h"

extern FILE* yyout;
struct XASTnode* root;  //整颗拓展语法树的根节点

//层号
int lvl = 0;

//字符串拼接,返回的是新串,不改变原串
char* _strcat_(char* des, char* src){
    char* res = (char*)malloc(sizeof(char)*(strlen(des)+strlen(src)));
    strcpy(res, des);
    strcat(res, src);
    return res;
}

int initTACgenerator(){
    if((err_out = fopen("./error.output","w")) == NULL)
        return 0;
    if((TAC_out = fopen("./TACresult.txt","w")) == NULL)
        return 0;
    fprintf(err_out, RED"TAC generator has initialized..."NONE"\n");
    return 1;
}

//拓展原始节点
struct  XASTnode* createXASTnode(struct ASTnode* root){
    struct XASTnode* xnode = (struct XASTnode *)calloc(sizeof(struct XASTnode), 1);
    xnode->kind = root->kind;
    xnode->pos = root->pos;
    xnode->name = (char*)malloc(sizeof(char)*20);
    printf("%s\n", root->name);
    strcpy(xnode->name, root->name);

    xnode->tac_head = NULL;
    xnode->Jwbt = (char*)malloc(sizeof(char)*20);
    xnode->Jwbf = (char*)malloc(sizeof(char)*20);
    xnode->Snext = (char*)malloc(sizeof(char)*20);
    switch(root->kind){
    case TYPE:
        strcpy(xnode->type_id, root->type_id);
        break;
    case ID:
        strcpy(xnode->type_id, root->type_id);
        break;
    case RELOP:
        strcpy(xnode->type_id, root->type_id);
        break;
    case INT:
        xnode->type_int = root->type_int;
        break;
    case FLOAT:
        xnode->type_float = root->type_float;
        break;
    case DOUBLE:
        xnode->type_double = root->type_double;
        break;
    }
    return xnode;
}

//将原始抽象语法树拓展,为它的数据结构添加新的属性,而不破坏语义分析中ASTnode的使用
struct XASTnode* expandAST(struct ASTnode* root){
    if(root == NULL)
        return NULL;
    struct XASTnode* xnode = createXASTnode(root);
    for(int i = 0;i < 8;i++)
        xnode->childNode[i] = expandAST(root->childNode[i]);
    return xnode;
}

void printXAST(struct XASTnode *root, int lvl, int prelvl, char* prefix, int hasBro){
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
                printXAST(root->childNode[i], lvl+1, lvl, tempPreFix, root->childNode[i+1]!=NULL?1:0);
            else
                printXAST(root->childNode[i], lvl+1, lvl, tempPreFix, 0);
}

struct opn* newOpn(){
    struct opn* newOpn = (struct opn *)calloc(sizeof(struct opn), 1);
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
            fprintf(err_out, RED"Fatal Error: TAC genertor has encountered...\n");
        }
    }else{
        TACnode->op = op;
        TACnode->opn1 = va_arg(pArgs, struct opn*);
        TACnode->opn2 = va_arg(pArgs, struct opn*);
        TACnode->result = va_arg(pArgs, struct opn*);
    }
    va_end(pArgs);
    return TACnode;
}

//合并多个TAC的双向循环链表，并使其首尾相连
pTACnode mergeTAC(int num, ...){
    pTACnode h1,h2,p,t1,t2;
    va_list pArgs;
    va_start(pArgs, num);
    h1 = va_arg(pArgs, pTACnode);
    while(--num > 0){
        h2 = va_arg(pArgs, pTACnode);
        if(h1 == NULL)
            h1 = h2;
        else if(h2 != NULL){
            t1 = h1->prior;
            t2 = h2->prior;
            t1->next=h2;
            t2->next=h1;
            h1->prior=t2;
            h2->prior=t1;
        }
    }
    va_end(pArgs);
    return h1;
}

//根据类型返回变量宽度(即字节数)
//按照32位编译器习惯,贴合C语言存储
int calWidth(int kind){
    //TODO:You may add some more TYPEs in your complier
    switch(kind){
    case INT:
        return 4;
    case FLOAT:
        return 4;
    case DOUBLE:
        return 8;
    case BOOL:
        return 1;
    case CHAR:
        return 1;
    }
}

char* auto_Alias(){
    static int counter = 1;
    char s[10];
    sprintf(s, "%d", counter++);
    return _strcat_("var",s);
}

char* auto_Label(){
    static int counter = 1;
    char s[10];
    sprintf(s, "%d", counter++);
    return _strcat_("label",s);
}

char* auto_Temp(){
    static int counter = 1;
    char s[10];
    sprintf(s, "%d", counter++);
    return _strcat_("temp",s);
}

int fill_ST(int kind, int num, char* type, int offset, ...){
    va_list pArgs = NULL;
    va_start(pArgs, offset);
}

//TAC生成器周游AST时,已经假定没有任何语义错误存在,故而直接生成TAC代码而不加检错
int TAC_Traversal(struct XASTnode* root){
    if(root == NULL)
        return 0;
    int num,width;                              //标识节点数和偏移量
    switch(root->kind){
    case EXTDEFLIST:
        root->childNode[0]->offset = root->offset;
        TAC_Traversal(root->childNode[0]);
        root->tac_head = root->childNode[0]->tac_head;
        //判断是否有ExtDefList->ExtDef ExtDefList子树存在
        if(root->childNode[1] != NULL){
            root->childNode[1]->offset = root->childNode[0]->offset + root->childNode[0]->width;    //第二个子树的起始偏移量为第一个子树的偏移量+第一个子树的宽度
            TAC_Traversal(root->childNode[1]);
            root->tac_head = mergeTAC(2, root->tac_head, root->childNode[1]->tac_head);             //合并左子树与右子树的TAC链表
        }
        break;
    case EXTDEF:
        GA_ExtDef(root);
        break;
    default:
        for(int i = 0;i<8;i++)
            TAC_Traversal(root->childNode[0]);
    }
}

//Generator Analyze NT[ExtDef]
int GA_ExtDef(struct XASTnode* extDef){
    struct XASTnode* spec = extDef->childNode[0];
    struct XASTnode* ID;
    char* type;
    switch(extDef->childNode[1]->kind){
    case EXTDECLIST:
        //ExtDef -> Specifier ExtDecList SEMI
        type = spec->childNode[0]->type_id;
        //ExtDecList下可能有多个外部变量声明
        struct XASTnode* extDecList = extDef->childNode[1];
        struct XASTnode* ID;
        while(extDecList != NULL){
            ID = extDecList->childNode[0]->childNode[0];
            ID->place
            extDecList = extDecList->childNode[2];
        }
        break;
    case FUNCDEC:
        break;
    case SEMI:
        break;
    }
}
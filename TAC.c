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
int level = 0;

//TAC生成器使用的，区别于语义分析中的符号表
TAC_ST *symbolTable;

//字符串拼接,返回的是新串,不改变原串
char* _strcat_(char* des, char* src){
    char* res = (char*)malloc(sizeof(char)*(strlen(des)+strlen(src)));
    strcpy(res, des);
    strcat(res, src);
    return res;
}

int initSymbolTable(){
    if((symbolTable = (TAC_ST*)malloc(sizeof(TAC_ST))) == NULL)
        return 0;
    symbolTable->vindex = symbolTable->findex = 0;
    symbolTable->var_symbol = NULL;
    symbolTable->func_symbol = NULL;
    return 1;
}

int initTACgenerator(){
    if((err_out = fopen("./error.output","w")) == NULL)
        return 0;
    if((TAC_out = fopen("./TACresult.txt","w")) == NULL)
        return 0;
    if(initSymbolTable() == 0)
        return 0;
    fprintf(err_out, "TAC generator has initialized...\n");
    return 1;
}

//拓展原始节点
struct  XASTnode* createXASTnode(struct ASTnode* root){
    struct XASTnode* xnode = (struct XASTnode *)calloc(sizeof(struct XASTnode), 1);
    xnode->kind = root->kind;
    xnode->pos = root->pos;
    xnode->name = (char*)malloc(sizeof(char)*20);
    //printf("%s\n", root->name);
    strcpy(xnode->name, root->name);

    xnode->num = 0;
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
int calWidth(int kind, ...){
    va_list pArgs;
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
    case TYPE:
        va_start(pArgs, kind);
        char* type_id = va_arg(pArgs, char*);
        if(!strcmp(type_id, "int"))
            return 4;
        if(!strcmp(type_id, "float"))
            return 4;
        if(!strcmp(type_id, "double"))
            return 8;
        if(!strcmp(type_id, "char"))
            return 1;
        if(!strcmp(type_id, "bool"))
            return 1;
    default:
        return 4;
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

int fill_ST(int symbol_kind, int num, char* type, int offset, ...){
    struct XASTnode* xID;
    va_list pArgs = NULL;
    va_start(pArgs, offset);
    switch(symbol_kind){
    case VAR_SYMBOL:  
        while(num-- > 0){
            xID = va_arg(pArgs, struct XASTnode*);
            pVar newV = createNewVar();
            newV->alias = auto_Alias();
            strcpy(newV->name, xID->type_id);
            strcpy(newV->type, type);
            newV->line = xID->pos;
            newV->lvl = level;
            newV->alias = auto_Alias(); 
            newV->offset = offset;   
            //插入符号表中的变量符号子表的链表尾部
            pVar tempV = symbolTable->var_symbol;
            //newV->next = symbolTable->var_symbol;
            //symbolTable->var_symbol = newV;
            if(symbolTable->var_symbol==NULL){
                symbolTable->var_symbol = newV;
            }else{
                while(tempV->next != NULL)
                    tempV = tempV->next;
                tempV->next = newV;
            }
            symbolTable->vindex++;  //填入后vindex自增(注意索引下限从1开始)          
        }
        return symbolTable->vindex;
    case FUNC_SYMBOL:
        num == 1;
        pFunc newF = createNewFunc();
        newF->alias = auto_Alias();
        xID = va_arg(pArgs, struct XASTnode*);
        strcpy(newF->fType, type);
        strcpy(newF->name, xID->type_id);
        newF->line = xID->pos;
        newF->isDef = 1;
        int paramNum = va_arg(pArgs, int);      //形参个数
        newF->paramNum = paramNum;
        if(paramNum == 0){
            strcpy(newF->rType, "void");
            newF->paramList = NULL;
        }else{
            struct XASTnode** pDecList = va_arg(pArgs, struct XASTnode**);
        }
        //插入符号表中的函数符号子表的链表尾部
        pFunc tempF = symbolTable->func_symbol;
        if(symbolTable->func_symbol == NULL){
            symbolTable->func_symbol = newF;
        }else{
            while(tempF->next != NULL)
                tempF = tempF->next;
            tempF->next = newF;
        }
        symbolTable->findex++;
        return symbolTable->findex;
    }
    
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
    case PROGRAM:
        root->childNode[0]->offset = root->offset;
        TAC_Traversal(root->childNode[0]);
        root->tac_head = root->childNode[0]->tac_head;
        break;
    default:
        for(int i = 0;i<8;i++)
            TAC_Traversal(root->childNode[i]);
    }
}

//Generator Analyze NT[ExtDef]
int GA_ExtDef(struct XASTnode* extDef){
    struct XASTnode* spec = extDef->childNode[0];
    struct XASTnode* ID;
    struct XASTnode* tempNode;
    char* type;     //类型的字符串形式名称
    int kind;       //标识类型
    int tempOffset = extDef->offset; //偏移量临时变量
    int width;      //宽度
    switch(extDef->childNode[1]->kind){
    case EXTDECLIST:
        //ExtDef -> Specifier ExtDecList SEMI
        type = spec->childNode[0]->type_id;
        kind = spec->childNode[0]->kind;
        width = calWidth(kind, type);
        //ExtDecList下可能有多个外部变量声明
        struct XASTnode* extDecList = extDef->childNode[1];
        struct XASTnode* ID;
        while(extDecList != NULL){
            extDef->num++;  //本ExtDef父节点下的一次类型声明下定义的多个变量的计数器
            //将该类型对应的一个变量的宽度,数量(1),偏移量向下传递直到ID结点
            tempNode = extDecList;
            while(tempNode != NULL){
                tempNode->offset = tempOffset;
                tempNode->width = width;
                tempNode->num = 1;     
                tempNode = tempNode->childNode[0];
            }
            ID = extDecList->childNode[0]->childNode[0];
            ID->place = fill_ST(VAR_SYMBOL, 1, type, ID->offset, ID);
            tempOffset += width;
            extDecList = extDecList->childNode[2];
        }
        extDef->width = width * extDef->num;
        extDef->tac_head = NULL;//外部变量不生成TAC
        break;
    case FUNCDEC:
        type = spec->childNode[0]->type_id; //此时为函数的返回类型
        kind = spec->childNode[0]->kind;    //上述对应的枚举值
        struct XASTnode* funcDec = extDef->childNode[1];
        extDef->offset = DX;                //函数活动记录中局部变量的初始偏移量
        if(funcDec->childNode[2]->kind != VARLIST){
            //无参函数
            extDef->place = fill_ST(FUNC_SYMBOL, 1, type, 0, funcDec->childNode[0], 0);
        }else{
            struct ASTnode* varList = funcDec->childNode[2];
            struct ASTnode* paramList[8];
        }
        break;
    case SEMI:
        break;
    }
}

void printTAC_ST(){
    printf("max_vindex: %d\n", symbolTable->vindex);
    printf("变量名\t类型\t别名\t偏移量\t层号\n");
    pVar tempV = symbolTable->var_symbol;
    for(int i = 0;i<symbolTable->vindex;i++){
        printf("%s\t%s\t%s\t%d\t%d\n",tempV->name,tempV->type,tempV->alias,tempV->offset,tempV->lvl);
        fprintf(TAC_out, "%s\t%s\t\t%s\t%d\t%d\n",tempV->name,tempV->type,tempV->alias,tempV->offset,tempV->lvl);
        tempV=tempV->next;
    }
}
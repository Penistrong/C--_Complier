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
    //添加初始化函数,int read(),int write(int x),用于MIPS32从控制台接收数字并进行相应测试
    struct XASTnode* init_ID = (struct XASTnode *)calloc(sizeof(struct XASTnode), 1);
    init_ID->pos = 0;
    strcpy(init_ID->type_id, "read");
    fill_ST(FUNC_SYMBOL, 1, "int", 0, init_ID, 0);
    strcpy(init_ID->type_id, "write");
    fill_ST(FUNC_SYMBOL, 1, "int", 0, init_ID, 1, 1);
    strcpy(init_ID->type_id, "x");
    fill_ST(VAR_SYMBOL, 1, "int", 0, init_ID);
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
    xnode->width = 0;
    xnode->tac_head = NULL;
    xnode->content = (char*)malloc(sizeof(char)*20);
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
    newTACnode->next = newTACnode->prior = newTACnode;
    return newTACnode;
}

//生成一个TAC的元组,返回一个未插入双向列表的一个TAC结点
pTACnode generateTAC(int op, int pNum, ...){
    pTACnode TACnode = newTACnode();
    TACnode->op = op;
    va_list pArgs = NULL;
    va_start(pArgs, pNum);
    struct XASTnode *xID;
    char* label;
    int place;
    if(pNum == 1){
        switch(op){
        case LABEL:
            TACnode->result = newOpn();
            label = va_arg(pArgs, char*);
            strcpy(TACnode->result->id, label);
            break;
        case GOTO:
            TACnode->result = newOpn();
            label = va_arg(pArgs, char*);
            strcpy(TACnode->result->id, label);
            break;
        case PARAM:
            TACnode->result = newOpn();
            xID = va_arg(pArgs, struct XASTnode*);
            strcpy(TACnode->result->id, searchAlias(xID->place));
            TACnode->result->offset = xID->offset;
            TACnode->result->kind = ID;
            break;
        case FUNCTION:
            TACnode->result = newOpn();
            xID = va_arg(pArgs, struct XASTnode*);
            strcpy(TACnode->result->id, xID->type_id);
            TACnode->result->offset = xID->place;
            TACnode->result->kind = ID;
            break;
        case RETURN:
            TACnode->result = newOpn();
            place = va_arg(pArgs, int);
            strcpy(TACnode->result->id, searchAlias(place));
            TACnode->result->offset = place;
            TACnode->result->kind = ID;
        case ARG:
            TACnode->result = newOpn();
            strcpy(TACnode->result->id, va_arg(pArgs, char*));
            TACnode->result->kind = ID;
            TACnode->result->offset = 0;
            break;
        default:
            fprintf(err_out, RED"Fatal Error: TAC generator has encountered...\n");
        }
    }else if(pNum == 2){
        switch(op){
        case ASSIGN:
            TACnode->opn1 = va_arg(pArgs, struct opn*);
            TACnode->result = va_arg(pArgs, struct opn*);
            break;
        case CALL:
            TACnode->opn1 = va_arg(pArgs, struct opn*);
            TACnode->result = va_arg(pArgs, struct opn*);
            break;
        case NOT:
            TACnode->opn1 = va_arg(pArgs, struct opn*);
            TACnode->result = va_arg(pArgs, struct opn*);
            break;
        }
    }
    else{
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
    case VOID:
        return 0;
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

char* searchAlias(int index){
    pVar tempV = symbolTable->var_symbol;
    while(--index > 0)
        tempV = tempV->next;
    return tempV->alias;
}

int searchVar(char* id_name){
    int rtn_index;
    pVar tempV = symbolTable->var_symbol;
    for(int i = 0;i<symbolTable->vindex;i++){
        if(!strcmp(tempV->name, id_name))
            rtn_index = i + 1;
        tempV = tempV->next;
    }
    return rtn_index;
}

char* searchType(int index){
    pVar tempV = symbolTable->var_symbol;
    while(--index > 0)
        tempV = tempV->next;
    return tempV->type;
}

int searchKind(char* type){
    if(type==NULL)
        return -1;
    if(!strcmp(type,"int"))
        return INT;
    if(!strcmp(type,"float"))
        return FLOAT;
    if(!strcmp(type,"double"))
        return DOUBLE;
    if(!strcmp(type,"void"))
        return VOID;
}

int searchFunc(char* func_name){
    int rtn_index = 0;
    pFunc tempF = symbolTable->func_symbol;
    for(int i=0;i<symbolTable->findex;i++){
        if(!strcmp(tempF->name, func_name))
            rtn_index = i + 1;
        tempF=tempF->next;
    }
    return rtn_index;
}

char* searchFuncType(int func_index){
    pFunc tempF = symbolTable->func_symbol;
    while(--func_index > 0)
        tempF = tempF->next;
    return tempF->fType;
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
        strcpy(newF->rType, "void");//暂时性先填写实际返回类型为void
        strcpy(newF->fType, type);
        strcpy(newF->name, xID->type_id);
        newF->line = xID->pos;
        newF->isDef = 1;
        int paramNum = va_arg(pArgs, int);      //形参个数
        newF->paramNum = paramNum;
        newF->paramList = NULL;
        if(paramNum != 0)
            newF->pIndex_begin = va_arg(pArgs, int);  //形参在变量符号表中的起始位置
        //插入符号表中的函数符号子表的链表尾部
        pFunc tempF = symbolTable->func_symbol;
        if(symbolTable->func_symbol == NULL){
            symbolTable->func_symbol = newF;
        }else{
            while(tempF->next != NULL)
                tempF = tempF->next;
            tempF->next = newF;
        }
        return ++symbolTable->findex;
    case TEMP_SYMBOL:
        xID = va_arg(pArgs, struct XASTnode *); //这里的xID实际对应各类型的常量节点
        pVar newV = createNewVar();
        switch(xID->kind){
        case INT:
            sprintf(newV->name,"%d",xID->type_int);
            break;
        case FLOAT:
            sprintf(newV->name,"%f",xID->type_float);
            break;
        case DOUBLE:
            sprintf(newV->name,"%lf",xID->type_double);
            break;
        case REAL_VAL:
            sprintf(newV->name,"%s","VAL");
            break;
        } 
        strcpy(newV->type, type);
        newV->line = xID->pos;
        newV->lvl = level;
        newV->alias = auto_Temp(); 
        newV->offset = offset; 
        pVar tempV = symbolTable->var_symbol;
        if(symbolTable->var_symbol==NULL){
                symbolTable->var_symbol = newV;
        }else{
            while(tempV->next != NULL)
                tempV = tempV->next;
            tempV->next = newV;
        }
        return ++symbolTable->vindex; 
    }
    
}

//TAC生成器周游AST时,已经假定没有任何语义错误存在,故而直接生成TAC代码而不加检错
int TAC_Traversal(struct XASTnode* root){
    if(root == NULL)
        return 0;
    int num,width;                              //标识节点数和偏移量
    switch(root->kind){
    case PROGRAM:
        //文法起始符,设置总初始偏移量为0
        root->childNode[0]->offset = root->offset = 0;
        TAC_Traversal(root->childNode[0]);
        root->tac_head = root->childNode[0]->tac_head;
        break;
    case EXTDEFLIST:
        //ExtDefList->ExtDef ExtDefList
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
        printf("分析完毕所有ExtDef\n");
        break;
    default:
        for(int i = 0;i<8;i++)
            if(root->childNode[i]!=NULL)
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
        extDef->width = 0;                  //设置函数宽度为0，对外部变量地址分配无影响
        extDef->offset = DX;                //函数活动记录大小,offset在此不是偏移量含义
        funcDec->offset = extDef->offset;   //函数活动记录中局部变量的初始偏移量
        if(funcDec->childNode[2]->kind != VARLIST){
            //无参函数
            funcDec->place = fill_ST(FUNC_SYMBOL, 1, type, 0, funcDec->childNode[0], 0);
        }else{
            struct XASTnode* varList = funcDec->childNode[2];
            varList->offset = funcDec->offset;
            int pIndex_begin = GA_VarList(varList);
            extDef->offset += varList->width;
            funcDec->width += varList->width;
            //VarList->ParamDec->Specifier VarDec
            //连接函数定义TAC及其形参定义TAC,形参个数储存在第一个varList节点中
            int paramNum = varList->num;
            funcDec->place = fill_ST(FUNC_SYMBOL, 1, type, 0, funcDec->childNode[0], paramNum, pIndex_begin);
        }
        extDef->place = funcDec->childNode[0]->place = funcDec->place;
        funcDec->tac_head = generateTAC(FUNCTION,1,funcDec->childNode[0]);
        if(funcDec->childNode[2]->kind == VARLIST)
            funcDec->tac_head = mergeTAC(2, funcDec->tac_head, funcDec->childNode[2]->tac_head);
        struct XASTnode* compSt = extDef->childNode[2];
        compSt->offset = funcDec->offset + funcDec->width;
        GA_CompSt(compSt);
        extDef->offset += compSt->width;
        extDef->tac_head = mergeTAC(2, funcDec->tac_head, compSt->tac_head);
        printf("合并完成\n");
        printTAC_code(extDef->tac_head);
        break;
    case SEMI:
        break;
    }
}

//处理函数的形参列表,返回的为第一个形参在变量符号表中的索引
int GA_VarList(struct XASTnode* varList){
    struct XASTnode *pDec, *ID, *tempNode, *first_vl = varList;
    char *pType;
    int pKind, width, tempOffset = varList->offset, pIndex_begin = -1;
    while(varList != NULL){
        pDec = varList->childNode[0];
        pType = pDec->childNode[0]->childNode[0]->type_id;
        pKind = pDec->childNode[0]->childNode[0]->kind;
        width = calWidth(pKind, pType);
        //ParamDec->Specifier VarDec;VarDec->ID
        ID = pDec->childNode[1]->childNode[0];
        pDec->offset = pDec->childNode[1]->offset = ID->offset = tempOffset;
        pDec->width = pDec->childNode[1]->width = ID->width = width;
        pDec->num = pDec->childNode[1]->num = ID->num = 1;
        pDec->place = pDec->childNode[1]->place = ID->place = fill_ST(VAR_SYMBOL, 1, pType, ID->offset, ID);
        tempOffset += width;
        if(pIndex_begin == -1)
            pIndex_begin = pDec->place;
        varList->tac_head = pDec->tac_head = generateTAC(PARAM, 1, ID);
        first_vl->width += width;   //第一个VarList节点的宽度要被其父节点拿去计算总宽度，故而存储所有VarList的宽度
        first_vl->num++;
        varList = varList->childNode[2];
    }
    //连接各形参定义TAC
    pTACnode tempTac = first_vl->tac_head;
    varList = first_vl;
    for(int i = 0;i<first_vl->num-1;i++){
        varList = varList->childNode[2];
        tempTac = mergeTAC(2, tempTac, varList->tac_head);
    }
    return pIndex_begin;
}

//处理复合语句体
int GA_CompSt(struct XASTnode* compSt){
    level++;    //层号加1
    int topIndex = symbolTable->vindex;     //保存当前变量符号表中最后一个变量的索引,在其后的所有变量都是在本复合语句内部的变量,退出时全部删除
    char* type;
    int kind, width, place, tempOffset = compSt->offset;
    struct XASTnode* defList;
    struct XASTnode* stmList;
    struct XASTnode* tempNode;
    for(int i=0;i<4;i++){
        if(compSt->childNode[i] != NULL){
            switch(compSt->childNode[i]->kind){
            case LC:
                printf("进入一个复合节点\n");
                break;
            case RC:
                level--;    //退出复合语句节点
                //打印当前符号表
                printf("即将退出复合语句作用域,当前符号表为\n");
                printTAC_ST();
                //删除变量
                exitCompSt(topIndex);
                printf("退出复合作用域, 删除变量完成\n");
                break;
            case DEFLIST:
                printf("分析DefList\n");
                //处理局部变量定义
                struct XASTnode* first_dl = defList = compSt->childNode[i];
                defList->offset = tempOffset;
                while(defList != NULL){
                    struct XASTnode* def = defList->childNode[0];
                    type = def->childNode[0]->childNode[0]->type_id;
                    kind = def->childNode[0]->childNode[0]->kind;
                    width = calWidth(kind, type);
                    struct XASTnode* decList = def->childNode[1];
                    while(decList != NULL){
                        def->num++; //当前局部变量定义根节点参数个数+1
                        place = fill_ST(VAR_SYMBOL, 1, type, tempOffset, decList->childNode[0]->childNode[0]->childNode[0]);
                        tempNode = decList;
                        //把该decList处的各属性向下传递至ID节点
                        while(tempNode!=NULL){
                            tempNode->num = 1;
                            tempNode->width = width;
                            tempNode->offset = tempOffset;
                            tempNode->place = place;
                            tempNode = tempNode->childNode[0];
                        }
                        tempOffset += width;
                        if(decList->childNode[0]->childNode[1] != NULL){
                            //此时有定义时便赋值的初始化语句
                            struct XASTnode* exp = decList->childNode[0]->childNode[2];
                            exp->offset = tempOffset;
                            GA_Exp(exp);
                            struct opn* opn1 = newOpn();
                            strcpy(opn1->id, searchAlias(place));
                            opn1->kind = ID;
                            struct opn* result = newOpn();
                            strcpy(result->id, searchAlias(exp->place));//TODO:分析右值(常数)并给常数生成一个临时变量
                            result->kind = ID;
                            decList->tac_head = decList->childNode[0]->tac_head = mergeTAC(2,exp->tac_head, generateTAC(ASSIGN, 2, result, opn1));
                            tempOffset += calWidth(exp->childNode[0]->kind);
                        }
                        decList = decList->childNode[2];
                    }
                    printf("分析完DecList\n");
                    //将当前def下的所有decList对应的TAC合并
                    pTACnode tac = def->tac_head;
                    struct XASTnode* t_decl= def->childNode[1];
                    for(int i = 0;i<def->num;i++){
                        tac = mergeTAC(2, tac, t_decl->tac_head);
                        t_decl = t_decl->childNode[2];
                    }
                    //printTAC_code(tac);
                    defList->tac_head = def->tac_head = tac;
                    first_dl->width += def->width;
                    first_dl->num++;
                    defList=defList->childNode[1];
                }
                //将当前所有defList下的def全部合并到首个defList的TAC节点下
                pTACnode tac = first_dl->tac_head;
                struct XASTnode* t_defl = first_dl;
                for(int j = 0;j<first_dl->num;j++){
                    tac = mergeTAC(2, tac, t_defl->tac_head);
                    t_defl = t_defl->childNode[1];
                }
                compSt->tac_head = first_dl->tac_head = tac;
                compSt->width += first_dl->width;
                break;
            case STMLIST:
                //printTAC_ST();
                stmList = compSt->childNode[i];
                struct XASTnode* first_sl = stmList;
                tac = first_sl->tac_head;
                //TODO:处理StmtList
                while(stmList != NULL){
                    struct XASTnode* stmt = stmList->childNode[0];
                    //为有其他StmtList的序列生成标号,指明本条stmt结束后下一条的跳转位置
                    if(stmList->childNode[1] != NULL)
                        strcpy(stmt->Snext, auto_Label());
                    stmList->offset = stmt->offset = tempOffset;
                    GA_Stmt(stmt);
                    stmList->tac_head = stmt->tac_head;
                    tac = mergeTAC(2, tac, stmList->tac_head);
                    compSt->width += stmList->width;
                    stmList = stmList->childNode[1];    //一定要放在最后
                }
                first_sl->tac_head = tac;
                compSt->tac_head = mergeTAC(2, compSt->tac_head, first_sl->tac_head);
                break;
            }
        }
    }
}

void GA_Stmt(struct XASTnode* stmt){
    struct XASTnode* Else;
    switch(stmt->childNode[0]->kind){
    case COMPST:
        GA_CompSt(stmt->childNode[0]);
        stmt->tac_head = stmt->childNode[0]->tac_head;
        break;
    case EXP:
        GA_Exp(stmt->childNode[0]);
        stmt->tac_head = stmt->childNode[0]->tac_head;
        break;
    case RETURN:
        //Stmt->RETURN Exp SEMI
        GA_Exp(stmt->childNode[1]);
        printf("RETURN的exp的索引:%d,符号表最大索引:%d, 别名:%s\n", stmt->childNode[1]->place,symbolTable->vindex,searchAlias(stmt->childNode[1]->place));
        stmt->tac_head = mergeTAC(2, stmt->childNode[1]->tac_head, generateTAC(RETURN, 1, stmt->childNode[1]->place));
        break;
    case IF:
        Else = stmt->childNode[5];
        if(Else == NULL){
            //IF_THEN语句，没有else作跳转
            strcpy(stmt->Jwbt, auto_Label());
            strcpy(stmt->Jwbf, stmt->Snext);
            struct XASTnode* exp = stmt->childNode[2];
            strcpy(exp->Jwbt, stmt->Jwbt);
            strcpy(exp->Jwbf, stmt->Snext);
            GA_boolExp(exp);
            stmt->width += exp->width;
            GA_Stmt(stmt->childNode[4]);
            stmt->width += stmt->childNode[4]->width;
            stmt->tac_head = mergeTAC(5,exp->tac_head,generateTAC(GOTO,1,exp->Jwbf),generateTAC(LABEL,1,exp->Jwbt),\
                                        stmt->childNode[4]->tac_head,generateTAC(LABEL,1,stmt->Jwbf));
        }else{
            //IF_THEN_ELSE语句
            strcpy(stmt->Jwbt, auto_Label());
            strcpy(stmt->Jwbf, auto_Label());
            struct XASTnode* exp = stmt->childNode[2];
            strcpy(exp->Jwbt, stmt->Jwbt);
            strcpy(exp->Jwbf, stmt->Jwbf);
            GA_boolExp(exp);
            stmt->width += exp->width;
            GA_Stmt(stmt->childNode[4]);
            stmt->width += stmt->childNode[4]->width;
            GA_Stmt(stmt->childNode[6]);
            stmt->width += stmt->childNode[6]->width;
            stmt->tac_head = mergeTAC(7,exp->tac_head,generateTAC(GOTO,1,exp->Jwbf),\
                                        generateTAC(LABEL,1,exp->Jwbt),stmt->childNode[4]->tac_head,\
                                        generateTAC(LABEL,1,exp->Jwbf),stmt->childNode[6]->tac_head,\
                                        generateTAC(LABEL,1,stmt->Snext));
        }
        break;
    case WHILE:
        strcpy(stmt->Jwbt, auto_Label());
        strcpy(stmt->Jwbf, stmt->Snext);
        struct XASTnode* exp = stmt->childNode[2];
        strcpy(exp->Jwbt, stmt->Jwbt);
        strcpy(exp->Jwbf, stmt->Snext);
        GA_boolExp(exp);
        stmt->width += exp->width;
        strcpy(stmt->childNode[4]->Snext, auto_Label());
        printf("开始分析while子句中的compSt节点\n");
        GA_Stmt(stmt->childNode[4]);
        printf("分析完成\n");
        stmt->width += stmt->childNode[4]->width;
        stmt->tac_head = mergeTAC(7,generateTAC(LABEL,1,stmt->childNode[4]->Snext),\
                                         exp->tac_head,generateTAC(GOTO,1,stmt->Jwbf),\
                                         generateTAC(LABEL,1,exp->Jwbt),stmt->childNode[4]->tac_head,\
                                         generateTAC(GOTO,1,stmt->childNode[4]->Snext),generateTAC(LABEL,1,stmt->Jwbf));
        break;
    }
}

void GA_Exp(struct XASTnode* exp){
    if(exp == NULL)
        return;
    int place;
    struct opn *opn1, *opn2, *result;
    //匹配Exp下只有一个子节点的情况,通常归结为标识符或者常量
    if(exp->childNode[1]==NULL){
        struct XASTnode* Const;
        switch(exp->childNode[0]->kind){
        case ID:
            place = searchVar(exp->childNode[0]->type_id);
            strcpy(exp->content,searchType(place));
            exp->width = calWidth(searchKind(exp->content));
            printf("分析已定义的变量%s,其索引为%d,类型为%s,宽度为%d\n", exp->childNode[0]->type_id, place, exp->content,exp->width);
            exp->place = exp->childNode[0]->place = place;
            break;
        default://对应各类型常量
            Const = exp->childNode[0];
            Const->offset = exp->offset;
            exp->place = Const->place = fill_ST(TEMP_SYMBOL, 1, strlwr(Const->name), Const->offset, Const);
            exp->width = calWidth(searchKind(strlwr(Const->name)));
            //生成为临时变量赋予常数值的TAC
            result = newOpn();
            strcpy(result->id, searchAlias(Const->place));
            result->kind = ID;
            opn1 = newOpn();
            switch(Const->kind){
            case INT:
                opn1->kind = INT;
                opn1->const_int = Const->type_int;
                break;
            case FLOAT:
                opn1->kind = FLOAT;
                opn1->const_float = Const->type_float;
                break;
            case DOUBLE:
                opn1->kind = DOUBLE;
                opn1->const_double = Const->type_double;
                break;
            }
            strcpy(exp->content,searchType(exp->place));
            exp->tac_head = Const->tac_head = generateTAC(ASSIGN, 2, opn1, result);
            break;
        }
    }else if((exp->childNode[0]->kind != EXP) && (exp->childNode[1]->kind == EXP) && (exp->childNode[2] == NULL)){
        //左单目操作符
        //TODO
    }
    else if((exp->childNode[1]->kind != EXP) && (exp->childNode[0]->kind == EXP) && (exp->childNode[2] == NULL)){
        //右单目操作符
        //TODO
    }else if((exp->childNode[0]->kind==EXP) && (exp->childNode[2]->kind==EXP)){
        //双目表达式,两端为Exp,中间为双目操作符
        struct XASTnode* operator = exp->childNode[1];
        struct XASTnode* tempID;    //用于存放临时的计算结果
        switch(operator->kind){
        case ASSIGNOP:
            GA_Exp(exp->childNode[0]);
            GA_Exp(exp->childNode[2]);
            opn1 = newOpn();
            strcpy(opn1->id, searchAlias(exp->childNode[2]->place));
            opn1->kind = ID;
            result = newOpn();
            strcpy(result->id, searchAlias(exp->childNode[0]->place));
            result->kind = ID;
            strcpy(exp->content, exp->childNode[0]->content);
            exp->tac_head = mergeTAC(2,exp->childNode[2]->tac_head, generateTAC(ASSIGN, 2, opn1, result));
            break;
        //以下加减乘除时,都要给运算结果生成一个临时变量以储存
        case ADD_OP:
            GA_Exp(exp->childNode[0]);
            GA_Exp(exp->childNode[2]);
            strcpy(exp->content, exp->childNode[0]->content);//默认临时变量类型为左值的类型,用content进行了传递
            opn1 = newOpn();
            strcpy(opn1->id, searchAlias(exp->childNode[0]->place));
            opn1->kind = ID;
            opn2 = newOpn();
            strcpy(opn2->id, searchAlias(exp->childNode[2]->place));
            opn2->kind = ID;
            //为两者运算结果生成一个临时变量,默认类型为左值类型
            tempID = (struct XASTnode*)calloc(sizeof(struct XASTnode), 1);
            tempID->kind = REAL_VAL;//searchKind(exp->content);
            tempID->pos = exp->pos;
            exp->place = fill_ST(TEMP_SYMBOL, 1, exp->content, exp->offset, tempID);
            result = newOpn();
            strcpy(result->id, searchAlias(exp->place));
            result->kind = ID;
            exp->tac_head = mergeTAC(2,exp->childNode[2]->tac_head, generateTAC(ADD, 3, opn1, opn2, result));
            break;
        case SUB_OP:
            GA_Exp(exp->childNode[0]);
            GA_Exp(exp->childNode[2]);
            strcpy(exp->content, exp->childNode[0]->content);//默认临时变量类型为左值的类型,用content进行了传递
            opn1 = newOpn();
            strcpy(opn1->id, searchAlias(exp->childNode[0]->place));
            opn1->kind = ID;
            opn2 = newOpn();
            strcpy(opn2->id, searchAlias(exp->childNode[2]->place));
            opn2->kind = ID;
            //为两者运算结果生成一个临时变量,默认类型为左值类型
            tempID = (struct XASTnode*)calloc(sizeof(struct XASTnode), 1);
            tempID->kind = REAL_VAL;//                    searchKind(exp->content);
            tempID->pos = exp->pos;
            exp->place = fill_ST(TEMP_SYMBOL, 1, exp->content, exp->offset, tempID);
            result = newOpn();
            strcpy(result->id, searchAlias(exp->place));
            result->kind = ID;
            exp->tac_head = mergeTAC(2,exp->childNode[2]->tac_head, generateTAC(SUB, 3, opn1, opn2, result));
            break;
        case MUL_OP:
            GA_Exp(exp->childNode[0]);
            GA_Exp(exp->childNode[2]);
            strcpy(exp->content, exp->childNode[0]->content);//默认临时变量类型为左值的类型,用content进行了传递
            opn1 = newOpn();
            strcpy(opn1->id, searchAlias(exp->childNode[0]->place));
            opn1->kind = ID;
            opn2 = newOpn();
            strcpy(opn2->id, searchAlias(exp->childNode[2]->place));
            opn2->kind = ID;
            //为两者运算结果生成一个临时变量,默认类型为左值类型
            tempID = (struct XASTnode*)calloc(sizeof(struct XASTnode), 1);
            tempID->kind = REAL_VAL;//searchKind(exp->content);
            tempID->pos = exp->pos;
            exp->place = fill_ST(TEMP_SYMBOL, 1, exp->content, exp->offset, tempID);
            result = newOpn();
            strcpy(result->id, searchAlias(exp->place));
            result->kind = ID;
            exp->tac_head = mergeTAC(2,exp->childNode[2]->tac_head, generateTAC(MUL, 3, opn1, opn2, result));
            break;
        case DIV_OP:
            GA_Exp(exp->childNode[0]);
            GA_Exp(exp->childNode[2]);
            strcpy(exp->content, exp->childNode[0]->content);//默认临时变量类型为左值的类型,用content进行了传递
            opn1 = newOpn();
            strcpy(opn1->id, searchAlias(exp->childNode[0]->place));
            opn1->kind = ID;
            opn2 = newOpn();
            strcpy(opn2->id, searchAlias(exp->childNode[2]->place));
            opn2->kind = ID;
            //为两者运算结果生成一个临时变量,默认类型为左值类型
            tempID = (struct XASTnode*)calloc(sizeof(struct XASTnode), 1);
            tempID->kind = REAL_VAL;//searchKind(exp->content);
            tempID->pos = exp->pos;
            exp->place = fill_ST(TEMP_SYMBOL, 1, exp->content, exp->offset, tempID);
            result = newOpn();
            strcpy(result->id, searchAlias(exp->place));
            result->kind = ID;
            exp->tac_head = mergeTAC(2,exp->childNode[2]->tac_head, generateTAC(DIV, 3, opn1, opn2, result));
            break;
        case AND_OP:
            break;
        case OR_OP:
            break;
        case NOT_OP:
            break;
        case EQ_OP:
            break;
        case RELOP:
            break;
        }
    }else if((exp->childNode[0]->kind == ID) && (exp->childNode[1]->kind == LP)){
        printf("分析函数调用\n");
        //函数调用
        struct XASTnode* id = exp->childNode[0];
        place = searchFunc(id->type_id);
        strcpy(exp->content, searchFuncType(place));
        int width = calWidth(searchKind(exp->content)); //函数返回值宽度
        if((exp->childNode[2] != NULL) && (exp->childNode[2]->kind==ARGS)){
            //TODO有参函数调用
            struct XASTnode* args = exp->childNode[2];
            pTACnode tac = args->tac_head;//此时为第一个ARGS的tac序列,最后要将所有ARGS的TAC合并到首个ARGS下
            while(args != NULL){
                GA_Exp(args->childNode[0]);
                exp->width += args->childNode[0]->width;
                tac = mergeTAC(3, tac, args->childNode[0]->tac_head, generateTAC(ARG,1,searchAlias(args->childNode[0]->place)));
                args = args->childNode[2];
            }
            exp->childNode[2]->tac_head = mergeTAC(2, exp->tac_head, tac);
        }else{
            //无参函数调用
            exp->width += width;
        }
        //为函数调用新建一个临时变量
        struct XASTnode* tempID = (struct XASTnode*)calloc(sizeof(struct XASTnode), 1);
        tempID->kind = searchKind(exp->content);
        tempID->pos = exp->pos;
        exp->place = fill_ST(TEMP_SYMBOL,1,exp->content,exp->offset+exp->width, tempID);
        opn1 = newOpn();
        opn1->offset = place;   //这里的函数调用opn1的offset用来保存函数在符号表中的索引
        opn1->kind = ID;
        strcpy(opn1->id, id->type_id);
        result = newOpn();
        strcpy(result->id, searchAlias(exp->place));
        result->kind = ID;
        result->offset = exp->offset+exp->width;
        //实参传递TAC放在函数调用前
        exp->tac_head = mergeTAC(3, exp->tac_head, exp->childNode[2]->tac_head, generateTAC(CALL, 2, opn1, result));
    }else if((exp->childNode[0]->kind==LP) && (exp->childNode[2]->kind==RP)){
        //括号括起来的Exp
        GA_Exp(exp->childNode[1]);
        exp->tac_head = exp->childNode[1]->tac_head;
    }
}

void GA_boolExp(struct XASTnode* exp){
    if(exp==NULL)
        return;
    int op;
    struct opn *opn1,*opn2,*result;
    if(exp->childNode[1]==NULL){
        //已经分析到叶节点了,此处exp下为标识符或者常量
        GA_Exp(exp);
    }
    else if(exp->childNode[2]==NULL){
        //单目布尔表达式如 !Exp
        //strcpy(exp->)
        strcpy(exp->content, "NOT");
        //子Exp跳转与父Exp恰好颠倒
        strcpy(exp->childNode[1]->Jwbt,exp->Jwbf);
        strcpy(exp->childNode[1]->Jwbf,exp->Jwbt);
        GA_boolExp(exp->childNode[1]);
        opn1 = newOpn();
        strcpy(opn1->id, searchAlias(exp->place));
        opn1->kind = ID;
        result = newOpn();
        strcpy(result->id, exp->Jwbt);
        result->kind = ID;
        exp->tac_head = mergeTAC(2, exp->childNode[1]->tac_head, generateTAC(NOT,2,opn1,result));
    }
    else{
        //双目布尔表达式
        struct XASTnode* operator = exp->childNode[1];
        struct XASTnode* leftExp = exp->childNode[0];
        struct XASTnode* rightExp = exp->childNode[2];
        switch(operator->kind){
        case RELOP:
            //注意"=="被单独提了出去做了EQ_OP的判断,实际是一样的逻辑
            if(!strcmp(operator->type_id, "<"))
                op = JL;
            else if(!strcmp(operator->type_id, "<="))
                op = JLE;
            else if(!strcmp(operator->type_id, ">"))
                op = JG;
            else if(!strcmp(operator->type_id, ">="))
                op = JGE;
            else if(!strcmp(operator->type_id, "!="))
                op = NEQ;
            strcpy(exp->content, operator->type_id);
            strcpy(leftExp->Jwbt, exp->Jwbt);
            strcpy(leftExp->Jwbf, exp->Jwbf);
            strcpy(rightExp->Jwbt, exp->Jwbt);
            strcpy(rightExp->Jwbf, exp->Jwbf);
            GA_boolExp(leftExp);
            GA_boolExp(rightExp);
            opn1 = newOpn();
            opn2 = newOpn();
            result = newOpn();
            opn1->kind = opn2->kind = ID;
            result->kind = LABEL;
            strcpy(opn1->id, searchAlias(leftExp->place));
            strcpy(opn2->id, searchAlias(rightExp->place));
            strcpy(result->id, exp->Jwbt);
            exp->tac_head = mergeTAC(3,leftExp->tac_head,rightExp->tac_head,generateTAC(op,3,opn1,opn2,result));
            break;
        case EQ_OP:
            op=EQ;
            strcpy(exp->content, operator->type_id);
            strcpy(leftExp->Jwbt, exp->Jwbt);
            strcpy(leftExp->Jwbf, exp->Jwbf);
            strcpy(rightExp->Jwbt, exp->Jwbt);
            strcpy(rightExp->Jwbf, exp->Jwbf);
            GA_boolExp(leftExp);
            GA_boolExp(rightExp);
            opn1 = newOpn();
            opn2 = newOpn();
            result = newOpn();
            opn1->kind = opn2->kind = ID;
            result->kind = LABEL;
            strcpy(opn1->id, searchAlias(leftExp->place));
            strcpy(opn2->id, searchAlias(rightExp->place));
            strcpy(result->id, exp->Jwbt);
            exp->tac_head = mergeTAC(3,leftExp->tac_head,rightExp->tac_head,generateTAC(op,3,opn1,opn2,result));
            break;
        case AND_OP:
            op=AND;
            strcpy(exp->content, operator->type_id);
            strcpy(leftExp->Jwbt, exp->Jwbt);
            strcpy(leftExp->Jwbf, exp->Jwbf);
            strcpy(rightExp->Jwbt, exp->Jwbt);
            strcpy(rightExp->Jwbf, exp->Jwbf);
            GA_boolExp(leftExp);
            GA_boolExp(rightExp);
            opn1 = newOpn();
            opn2 = newOpn();
            result = newOpn();
            opn1->kind = opn2->kind = ID;
            result->kind = LABEL;
            strcpy(opn1->id, searchAlias(leftExp->place));
            strcpy(opn2->id, searchAlias(rightExp->place));
            strcpy(result->id, exp->Jwbt);
            exp->tac_head = mergeTAC(3,leftExp->tac_head,rightExp->tac_head,generateTAC(op,3,opn1,opn2,result));
            break;
        case OR_OP:
            op=OR;
            strcpy(exp->content, operator->type_id);
            strcpy(leftExp->Jwbt, exp->Jwbt);
            strcpy(leftExp->Jwbf, exp->Jwbf);
            strcpy(rightExp->Jwbt, exp->Jwbt);
            strcpy(rightExp->Jwbf, exp->Jwbf);
            GA_boolExp(leftExp);
            GA_boolExp(rightExp);
            opn1 = newOpn();
            opn2 = newOpn();
            result = newOpn();
            opn1->kind = opn2->kind = ID;
            result->kind = LABEL;
            strcpy(opn1->id, searchAlias(leftExp->place));
            strcpy(opn2->id, searchAlias(rightExp->place));
            strcpy(result->id, exp->Jwbt);
            exp->tac_head = mergeTAC(3,leftExp->tac_head,rightExp->tac_head,generateTAC(op,3,opn1,opn2,result));
            break;
        }
    }
}

//删除给定索引后的所有复合语句节点内部新建的变量
void exitCompSt(int topIndex){
    pVar p = symbolTable->var_symbol;
    for(int i=0;i<topIndex - 1;i++)
        p=p->next;
    p->next = NULL;
    symbolTable->vindex = topIndex;//复位符号表最大索引
}

void printTAC_ST(){
    printf(LIGHT_PURPLE"max_vindex: %d"NONE"\n", symbolTable->vindex);
    printf("索引\t变量名\t类型\t别名\t偏移量\t层号\n");
    pVar tempV = symbolTable->var_symbol;
    for(int i = 0;i<symbolTable->vindex;i++){
        printf("%d\t%-*.5s\t%s\t%s\t%d\t%d\n", i+1, strlen(tempV->name), tempV->name,tempV->type,tempV->alias,tempV->offset,tempV->lvl);
        fprintf(TAC_out, "%d\t%s\t%s\t\t%s\t%d\t%d\n", i, tempV->name,tempV->type,tempV->alias,tempV->offset,tempV->lvl);
        tempV=tempV->next;
    }
    printf(LIGHT_GREEN"max_findex: %d"NONE"\n", symbolTable->findex);
    printf("函数名\t返回类型\t实际类型\t形参个数\t形参起始索引\t别名\t偏移量\n");
    pFunc tempF = symbolTable->func_symbol;
    for(int i=0;i<symbolTable->findex;i++){
        printf("%s\t%s\t\t%s\t\t%d\t\t%d\t\t%s\t%d\n",tempF->name, tempF->fType, tempF->rType, tempF->paramNum, tempF->pIndex_begin, tempF->alias, tempF->offset);
        tempF = tempF->next;
    }
}

//打印以首尾相连的双向循环链表TAC
void printTAC_code(pTACnode tac_head){
    if(tac_head == NULL){
        printf("空TAC，无法打印\n");
        return;
    }
    pTACnode h = tac_head;
    do{
        switch(h->op){
        case LABEL:
            printf(LIGHT_CYAN"LABEL"GREEN" %s"NONE":\n",h->result->id);
            break;
        case GOTO:
            printf("\t"LIGHT_RED"GOTO "GREEN"%s"NONE"\n",h->result->id);
            break;
        case FUNCTION:
            printf(PURPLE"FUNCTION"BROWN" %s"NONE":\n",h->result->id);
            break;
        case PARAM:
            printf("\tPARAM"BLUE" %s "NONE"[offset:%d]\n", h->result->id, h->result->offset);
            break;
        case RETURN:
            printf("\t"LIGHT_PURPLE"RETURN"NONE" %s\n", h->result->id);
            break;
        case ASSIGN:
            switch(h->opn1->kind){
            case ID:
                printf("\t%s := %s \n", h->result->id, h->opn1->id);
                break;
            case INT:
                printf("\t%s := #%d \n", h->result->id, h->opn1->const_int);
                break;
            case FLOAT:
                printf("\t%s := #%f \n", h->result->id, h->opn1->const_float);
                break;
            case DOUBLE:
                printf("\t%s := #%lf \n", h->result->id, h->opn1->const_double);
                break;
            }
            break;
        case CALL:
            printf("\t%s := CALL %s()\n", h->result->id, h->opn1->id);
            break;
        case ARG:
            printf("\t\tARG %s\n",h->result->id);
            break;
        case NOT:
            printf("\t"RED"IF"NONE" !%s "RED"GOTO "NONE"%s\n",h->opn1->id,h->result->id);
            break;
        case ADD:
            printf("\t%s := %s + %s\n", h->result->id, h->opn1->id, h->opn2->id);
            break;
        case SUB:
            printf("\t%s := %s - %s\n", h->result->id, h->opn1->id, h->opn2->id);
            break;
        case MUL:
            printf("\t%s := %s * %s\n", h->result->id, h->opn1->id, h->opn2->id);
            break;
        case DIV:
            printf("\t%s := %s / %s\n", h->result->id, h->opn1->id, h->opn2->id);
            break;
        case JL:
            printf(RED"\tIF"NONE" %s < %s"LIGHT_RED" GOTO "GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        case JLE:
            printf(RED"\tIF"NONE" %s <= %s"LIGHT_RED" GOTO "GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        case JG:
            printf(RED"\tIF"NONE" %s > %s"LIGHT_RED" GOTO"GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        case JGE:
            printf(RED"\tIF"NONE" %s >= %s"LIGHT_RED" GOTO "GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        case NEQ:
            printf(RED"\tIF"NONE" %s != %s"LIGHT_RED" GOTO "GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        case EQ:
            printf(RED"\tIF"NONE" %s == %s"LIGHT_RED" GOTO "GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        case AND:
            printf(RED"\tIF"NONE" %s && %s"LIGHT_RED" GOTO "GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        case OR:
            printf(RED"\tIF"NONE" %s || %s"LIGHT_RED" GOTO "GREEN"%s"NONE"\n",h->opn1->id,h->opn2->id,h->result->id);
            break;
        }
        h = h->next;
    }while(h != tac_head);
}
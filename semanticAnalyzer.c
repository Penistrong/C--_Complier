/*
* @Author: Penistrong
* @Date:   2019-12-03 11:06:21
* @Last Modified by:   Penistrong
* @Last Modified time: 2019-12-12 10:00:48
*/
#include "ast.h"
#include "semanticAnalyzer.h"
#include "TAC.h"
#include "OC.h"

//采用作用域与多符号表组织

//作用域符号表栈，每当进入新作用域要将当前作用域符号表压栈
extern pStack stack;
//Assume Symbol Table as ST
pScope_ST current_ST;
//当前作用域顶层函数，用以检测返回值类型是否匹配
struct ASTnode* cur_func_ID = NULL;
//当前作用域顶层函数对应层号，用以判断嵌套复合作用域时遇到RC有无退出函数
int cur_func_lvl = 0;
int cur_func_has_return = 0;
//层号
int lvl = 0;

//初始化各符号表
void initializeSymbolTable(){
    printf("开始创建符号表\n");
    current_ST = (pScope_ST)malloc(sizeof(Scope_ST));
    current_ST->scope_name = (char*)malloc(sizeof(char)*20);
    strcpy(current_ST->scope_name, "OverAll_Scope");

    //创建变量符号表
    current_ST->var_st = (pVar)malloc(sizeof(struct Var_Symbol));
    current_ST->var_st->name = (char*)malloc(sizeof(char)*20);
    current_ST->var_st->type = (char*)malloc(sizeof(char)*20);
    strcpy(current_ST->var_st->name, "VarName");
    strcpy(current_ST->var_st->type, "VarType");
    current_ST->var_st->line = -1;
    current_ST->var_st->next = NULL;

    //创建新的函数符号表, 默认函数为__read__函数
    pFunc_ST_Head func_st = createNewFunc();
    strcpy(func_st->name, "_read_");
    func_st->isDef = 1;
    strcpy(func_st->fType, "void");
    strcpy(func_st->rType, "void");
    func_st->paramNum = 0;
    func_st->next = NULL;
    current_ST->func_st = func_st;

    //创建结构体符号表
    pStruct_ST_Head struct_st = createNewStruct();
    current_ST->struct_st = struct_st;

    current_ST->array_st = (pArray)malloc(sizeof(struct Array_Symbol));

    printf("全局作用域符号表创建成功\n");
}

pScope_ST newScope_ST(){
    pScope_ST n = (pScope_ST)malloc(sizeof(Scope_ST));
    n->scope_name = (char*)malloc(sizeof(char)*20);
}

/*分析抽象语法树AST，且为其生成对应的符号表*/
void semantic_Analyze(struct ASTnode *root){
    if(root == NULL){
        return;
    }
    //printXAST(expandAST(root), 0, -1, NULL, 0);此句用来测试拓展语法树结构是否正常
    stack = initStack();
    initializeSymbolTable();
    push(current_ST);           //当前符号表入栈
    AST_Traversal(root);
    printScope_ST(current_ST);
    if(initTACgenerator() != 0){
        struct XASTnode *xRoot = expandAST(root);
        TAC_Traversal(xRoot);
        printTAC_ST();
        printTAC_code(xRoot->tac_head);
        generateOC(xRoot->tac_head);
    }
}

//递归，抽象语法树的递归遍历
//观察语法树可知只需找到所有的ExtDef节点再对其进行分析
void AST_Traversal(struct ASTnode *root){
    if(root==NULL)
        return;
    switch(root->kind){
    case EXTDEF:
        analyzeExtDef(root);
        return;
    default:
        for(int i = 0;i<8;i++)
            if(root->childNode[i] != NULL)
                AST_Traversal(root->childNode[i]);
    }
}

//判断变量在当前作用域是否已定义, 
int isVarDef(struct ASTnode *ID){
    if(ID == NULL)
        return 0;
    pVar p = current_ST->var_st->next;  //函数表第一元组是占位符，故从第二元组开始
    while(p!=NULL){
        //当作用域相同且同名时DEBUG Error
        if((strcmp(ID->type_id, p->name)==0) && (p->lvl==lvl))
            return 1;
        p = p->next;
    }
    return 0;
}

//判断变量在所有作用域是否有定义
int isVarDec(struct ASTnode *ID){
    if(ID == NULL)
        return 0;
    pVar p = current_ST->var_st->next;  //函数表第一元组是占位符，故从第二元组开始
    while(p!=NULL){
        if((strcmp(ID->type_id, p->name)==0))
            return 1;
        p = p->next;
    }
    return 0;
}

//查找变量类型
char* typeVar(struct ASTnode* ID){
    if(ID == NULL)
        return NULL;
    pVar p =current_ST->var_st->next;
    while(p != NULL){
        if(strcmp(ID->type_id, p->name)==0)
            return p->type;
        p = p->next;
    }
    printf("Type of Var Undefined at Line : %d\n", ID->pos);
    return NULL;
}

int newVarSymbol(int num, char* type, ...){
    va_list pArgs = NULL;
    va_start(pArgs, type);
    for(int i = 0;i < num;i++){
        struct ASTnode *ID = va_arg(pArgs, struct ASTnode*);
        if(!isVarDef(ID)){
            pVar newVar = createNewVar();
            strcpy(newVar->name, ID->type_id);
            strcpy(newVar->type, type);
            newVar->line = ID->pos;
            newVar->lvl = lvl;                  //作为作用域标识的层号
            pVar tempVar = current_ST->var_st;
            while(tempVar->next != NULL)
                tempVar = tempVar->next;
            tempVar->next = newVar;
        }else
            printf(RED"Error type 30 at Line %d : "LIGHT_RED"Redefined Variable '%s'"NONE"\n", ID->pos, ID->type_id);
    }
    va_end(pArgs);
}

pVar createNewVar(){
    pVar newVar = (pVar)malloc(sizeof(struct Var_Symbol));
    newVar->name = (char*)malloc(sizeof(char)*20);
    newVar->type = (char*)malloc(sizeof(char)*20);
    newVar->next = NULL;
    return newVar;
}

int isFuncDef(struct ASTnode *ID){
    if(ID == NULL)
        return 1;
    pFunc p = current_ST->func_st;
    while(p!=NULL){
        if(!strcmp(ID->type_id, p->name))
            return 1;
        p = p->next;
    }
    return 0;
}

char* typeFunc(struct ASTnode* ID){
    if(ID == NULL)
        return NULL;
    pFunc p = current_ST->func_st;
    while(p!=NULL){
        if(!strcmp(ID->type_id, p->name))
            return p->fType;
        p = p->next;
    }
    printf("Type of Func Undefined at Line : %d\n", ID->pos);
    return NULL;
}

//查找函数形参
pVar plistFunc(struct ASTnode* ID){
    if(ID == NULL)
        return NULL;
    pFunc p = current_ST->func_st;
    while(p != NULL){
        if(!strcmp(ID->type_id, p->name))
            return p->paramList;
        p = p->next;
    }
    return NULL;
}

int newFuncSymbol(char* type, char* name, int line, int paramNum, ...){
    pFunc newFunc = createNewFunc();
    strcpy(newFunc->fType, type);
    strcpy(newFunc->name, name);
    newFunc->isDef = 1;
    newFunc->paramNum = paramNum;
    newFunc->line = line;

    if(paramNum == 0){
        //建立无参函数
        strcpy(newFunc->rType, "void");
        newFunc->paramList = NULL;
    }else{
        //建立有参函数
        va_list pArgs = NULL;
        va_start(pArgs, paramNum);
        struct ASTnode** pDecList = va_arg(pArgs, struct ASTnode **);//保存paramDec节点的数组
        pVar tempParam = newFunc->paramList;
        for(int i = 0;i<paramNum;i++){
            struct ASTnode* pDec = pDecList[i];
            strcpy(tempParam->type, pDec->childNode[0]->childNode[0]->type_id); //Specifier->Type.type_id
            strcpy(tempParam->name, pDec->childNode[1]->childNode[0]->type_id); //VarDec->ID.type_id
            //将形参加入变量符号表
            newVarSymbol(1, tempParam->type, pDec->childNode[1]->childNode[0]);
            if(i != paramNum - 1){
                tempParam->next = createNewVar();
                tempParam = tempParam->next;
            }
            //将形参列表最后一个有值节点的next置空,上述循环的最后多建立了一个未初始化赋值的Var链表节点
        }
        va_end(pArgs);

        strcpy(newFunc->rType, "void");
    }
    //将新建的funcSymbol链表节点接在函数符号表尾部
    pFunc tempFunc = current_ST->func_st;
    while(tempFunc->next != NULL)
        tempFunc = tempFunc->next;
    tempFunc->next = newFunc;
}

pFunc createNewFunc(){
    pFunc newFunc = (pFunc)malloc(sizeof(struct Func_Symbol));
    newFunc->name = (char*)malloc(sizeof(char)*50);
    newFunc->fType = (char*)malloc(sizeof(char)*20);
    newFunc->rType = (char*)malloc(sizeof(char)*20);
    newFunc->paramList = createNewVar();
    newFunc->next = NULL;
    return newFunc;
}

int isStrucDef(struct ASTnode* ID){
    if(ID == NULL)
        return 1;
    pStruct p = current_ST->struct_st;
    while(p != NULL){
        if(!strcmp(ID->type_id, p->name))
            return 1;
        p = p->next;
    }
    return 0;
}

int newStrucSymbol(char* name, int line, int memberNum, ...){
    pStruct newStruct = createNewStruct();
    strcpy(newStruct->name, name);
    newStruct->line = line;
    if(memberNum != 0){
        va_list pArgs = NULL;
        va_start(pArgs, memberNum);
        struct ASTnode** memberList = va_arg(pArgs, struct ASTnode **);
        va_end(pArgs);
    }
    pStruct tempS = current_ST->struct_st;
    while(tempS->next != NULL)
        tempS = tempS->next;
    tempS->next = newStruct;
}

pStruct createNewStruct(){
    pStruct newStruct = (pStruct)malloc(sizeof(struct Struct_Symbol));
    newStruct->name = (char*)malloc(sizeof(char)*20);
    newStruct->memberList = createNewVar();
    newStruct->next = NULL;
    return newStruct;
}

int analyzeExtDef(struct ASTnode *extDef){
    if(extDef->childNode[0]->kind != SPECIFIER)
        return 0;
    struct ASTnode* ID;
    char* type;
    switch(extDef->childNode[1]->kind){
    case EXTDECLIST:
        //ExtDef -> Specifier ExtDecList SEMI
        type = extDef->childNode[0]->childNode[0]->type_id;
        struct ASTnode* extDecList = extDef->childNode[1];
        if(extDecList->childNode[1] == NULL){
            //ExtDecList->VarDec->ID
            newVarSymbol(1, type, extDecList->childNode[0]->childNode[0]);
        }else{
            newVarSymbol(1, type, extDecList->childNode[0]->childNode[0]);      //第一个标识符
            while(extDecList->childNode[2] != NULL){
                extDecList = extDecList->childNode[2];
                newVarSymbol(1, type, extDecList->childNode[0]->childNode[0]);  //以COMMA分割的剩余标识符
            }
        }
        break;
    case FUNCDEC:
        //ExtDef -> Specifier FuncDec CompSt
        ID = extDef->childNode[1]->childNode[0];
        if(!isFuncDef(ID)){
            type = extDef->childNode[0]->childNode[0]->type_id;
            struct ASTnode* funcDec = extDef->childNode[1];
            if(funcDec->childNode[3]==NULL){
                //无参函数
                newFuncSymbol(type, funcDec->childNode[0]->type_id, funcDec->childNode[0]->pos, 0);
            }else{
                //有参函数,即有VarList节点
                struct ASTnode* varList = funcDec->childNode[2];
                struct ASTnode* paramList[8];
                for(int i=0;i<8;i++)
                    paramList[i] = NULL;
                //VarList->ParamDec->Specifier VarDec
                paramList[0] = varList->childNode[0];
                int paramNum = 1;
                while(varList->childNode[2] != NULL){
                    varList = varList->childNode[2];
                    paramList[paramNum++] = varList->childNode[0];
                }
                newFuncSymbol(type, funcDec->childNode[0]->type_id, funcDec->childNode[0]->pos, paramNum, paramList);
            }
            cur_func_ID = funcDec->childNode[0];    //将当前检测到的函数保存为顶级作用域函数
            cur_func_lvl = lvl;                     //保存一下函数层级记录
            analyzeCompSt(extDef->childNode[2], 0);
        }else
            printf(RED"Error type 32 at Line %d : "LIGHT_RED"Duplicated Function Declaration '%s'"NONE"\n", ID->pos, ID->type_id);
        break;
    case SEMI:
        //对应后续结构体定义
        analyzeStrucSpec(extDef->childNode[0]->childNode[0]);
        break;
    }
}

int analyzeStrucSpec(struct ASTnode *strucSpec){
    struct ASTnode *strucID = NULL;
    switch(strucSpec->childNode[1]->kind){
        case OPTSTRUCTNAME:
            //结构体定义
            strucID = strucSpec->childNode[1]->childNode[0];
            struct ASTnode **memberList = NULL;
            char* name = (char*)malloc(sizeof(char)*20);
            int memberNum = 0;
            struct ASTnode *defList = strucSpec->childNode[3];
            while(defList != NULL){
                memberNum++;
                defList = defList->childNode[1];
            }
            if(strucID == NULL){
                strcpy(name, "Anony");
                newStrucSymbol(name, strucSpec->pos, memberNum, memberList);
            }else if(!isStrucDef(strucID)){
                strcpy(name, strucID->type_id);
                newStrucSymbol(name, strucID->pos, memberNum, memberList);
            }else
                printf(RED"Error type 31 at Line %d : "BLUE"Redefined Struct '%s'"NONE"\n", strucSpec->pos, strucID->type_id);
            break;
        case STRUCTDECL:
            //结构体声明
            strucID = strucSpec->childNode[1]->childNode[0];
            if(!isStrucDef(strucID)){
                printf(RED"Error type 11 at Line %d : "BLUE"Using undefined Struct '%s'"NONE"\n", strucID->pos, strucID->type_id);
                return 0;
            }else
                return 1;
            break;
    }
}

int analyzeCompSt(struct ASTnode *compSt, int isInBlock, ...){
    lvl++;  //遇到复合语句节点,层级+1，表示进入了一个新作用域
    //current_ST = newScope_ST();
    int i = 0;
    char* type;
    struct ASTnode* stmList;
    struct ASTnode* defList;
    int flag = 0;
    if(isInBlock == 1){
        //语句块标识,标记CompSt是否处于while或do或for语句块内部
        va_list block_flag = NULL;
        va_start(block_flag, isInBlock);
        flag = va_arg(block_flag, int);
        va_end(block_flag);
    }
    //使用for是防止CompSt->LC DefList StmList RC 中间两节点任意为空导致后续兄弟节点无法访问
    for(i;i<4;i++){
        if(compSt->childNode[i] != NULL){
            switch(compSt->childNode[i]->kind){
            case LC:
                break;
            case RC:
                lvl--;      //作用域关闭，层级退出
                //pop();
                //current_ST = top();
                if(lvl == cur_func_lvl)
                    if(cur_func_has_return == 0 && (strcmp(typeFunc(cur_func_ID), "void") != 0))
                        printf(RED"Error type 170 at Line %d : "LIGHT_RED"Function '%s' has no RETURN statement"NONE"\n", cur_func_ID->pos, cur_func_ID->type_id);
                cur_func_has_return = 0;    //重置计数器
                break;
            case DEFLIST:
                defList = compSt->childNode[i];
                while(defList != NULL){
                    struct ASTnode* def = defList->childNode[0];
                    //Def->Specifier DecList SEMI
                    //Specifier->TYPE|StructSpecifier
                    if(def->childNode[0]->childNode[0]->kind == TYPE)
                        type = def->childNode[0]->childNode[0]->type_id;
                    else{
                        struct ASTnode* strucSpec = def->childNode[0]->childNode[0];
                        type = (char*)malloc(sizeof(char)*20);
                        if(!analyzeStrucSpec(strucSpec))
                            strcpy(type, "Unknown");
                        else
                            strcpy(type, "Struct");
                    }
                    
                    struct ASTnode* decList = def->childNode[1];
                    struct ASTnode* ID = decList->childNode[0]->childNode[0]->childNode[0];
                    newVarSymbol(1, type, ID);
                    //Dec->VarDec ASSIGNOP Exp
                    struct ASTnode *exp = decList->childNode[0]->childNode[2];
                    if(exp != NULL)
                        if(strcmp(type, analyzeExp(exp)) != 0)
                            printf(RED"Error type 150 at Line %d : "BROWN"Type mismatch at both ends of Binocular Expression[Left:%s Right:%s]"NONE"\n", exp->pos, type, analyzeExp(exp));
                    defList = defList->childNode[1];
                }
                break;
            case STMLIST:
                stmList = compSt->childNode[i];
                while(stmList != NULL){
                    struct ASTnode* stmt = stmList->childNode[0];
                    analyzeStmt(stmt, isInBlock, flag);
                    stmList = stmList->childNode[1];
                }
                break;
            }
        }
    }
}

int analyzeStmt(struct ASTnode* stmt, int isInBlock, ...){
    int flag = 0;
    if(isInBlock == 1){
        //语句块标识,标记stmt是否处于while或do或for语句块内部
        va_list block_flag = NULL;
        va_start(block_flag, isInBlock);
        flag = va_arg(block_flag, int);
        va_end(block_flag);
    }
    struct ASTnode* logic_exp = NULL;
    struct ASTnode* Else = NULL;
    char* logic_val = NULL;

    switch(stmt->childNode[0]->kind){
    case COMPST:
        analyzeCompSt(stmt->childNode[0], isInBlock, flag);
        break;
    case EXP:
        analyzeExp(stmt->childNode[0]);
        break;
    case RETURN:
        //return语句,用以检测是否匹配当前顶层函数作用域的实际类型
        analyzeReturn(stmt);
        break;
    case IF:
        Else = stmt->childNode[5];
        if(Else==NULL)
            analyzeStmt(stmt->childNode[4], isInBlock, flag);
        else{
            analyzeStmt(stmt->childNode[4], isInBlock, flag);
            analyzeStmt(stmt->childNode[6], isInBlock, flag);
        }
        break;
    case DO:
        logic_exp = stmt->childNode[4];
        logic_val = analyzeExp(logic_exp);
        analyzeStmt(stmt->childNode[1], 1, DO);
        break;
    case WHILE:
        logic_exp = stmt->childNode[2];
        logic_val = analyzeExp(logic_exp);
        analyzeStmt(stmt->childNode[4], 1, WHILE);
        break;
    case BREAK:
        switch(flag){
        case 0:
            printf(RED"Error type 180 at Line %d : "RED"BREAK is not in any loop block or switch block"NONE"\n", stmt->pos);
            break;
        case WHILE:
        case DO:
        case FOR:
            break;
        }
        break;
    case CONTINUE:
        switch(flag){
        case 0:
            printf(RED"Error type 190 at Line %d : "RED"CONTINUE is not in any loop block"NONE"\n", stmt->pos);
            break;
        case WHILE:
        case DO:
        case FOR:
            break;
        }
        break;
    }
}

//递归分析Exp,使用继承属性进行自底向上传值
char* analyzeExp(struct ASTnode* exp){
    char* content = (char*)malloc(sizeof(char)*20);
    //优先匹配只有一个子节点，省去检空操作
    if(exp->childNode[1]==NULL){
        //Exp->Terminal(such as ID、INT、FLOAT...)
        if(exp->childNode[0]->kind == ID){
            struct ASTnode* id = exp->childNode[0];
            if(isVarDec(id)){
                strcpy(content, typeVar(id));
            }
            else if(isFuncDef(id)){
                //对函数名采用变量调用形式
                printf(RED"Error type 40 at Line %d : "WHITE"Implicit variable call using non variable name '%s'"NONE"\n", id->pos, id->type_id);
                strcpy(content, typeFunc(id));
            }
            else{
                printf(RED"Error type 10 at Line %d : "PURPLE"Using Undefined Variable '%s'"NONE"\n", id->pos, id->type_id);
                strcpy(content, "Unknown Type");
            }
        }else{
            strcpy(content, exp->childNode[0]->name);
            strlwr(content);    //各常量对应的name为其实际类型的大写，故使用VC下的strlwr()函数小写化，若在linux下运行则需手动实现strlwr
        }
    }
    else if((exp->childNode[0]->kind != EXP) && (exp->childNode[1]->kind == EXP) && (exp->childNode[2] == NULL)){
        //左单目操作符
        return analyzeExp(exp->childNode[1]);
    }
    else if((exp->childNode[1]->kind != EXP) && (exp->childNode[0]->kind == EXP) && (exp->childNode[2] == NULL)){
        //右单目操作符
        return analyzeExp(exp->childNode[0]);
    }
    else if((exp->childNode[0]->kind == ID) && (exp->childNode[1]->kind == LP)){
        //函数调用
        //防止与编码了的枚举值ID混淆，这里使用小写
        struct ASTnode* id = exp->childNode[0];
        if(isFuncDef(id)){
            strcpy(content, typeFunc(id));
            //检查是否为有参函数调用
            if(exp->childNode[2]->kind == ARGS){
                //检查形参列表个数与类型是否匹配
                pVar plist = plistFunc(id);                             //获得对应函数的形参列表
                struct ASTnode* args = exp->childNode[2];               //Exp->ID LP ARGS RP
                struct ASTnode* e;                                      //ARGS->EXP|EXP COMMA ARGS
                //检测到第一个错误便结束形参实参比较，防止输出多余信息
                while(args != NULL){
                    //逐一比较函数调用中实参与形参类型是否匹配
                    e = args->childNode[0];
                    char* rArgType = analyzeExp(e);
                    if(strcmp(plist->type, rArgType)){
                        printf(RED"Error type 70 at Line %d : "LIGHT_GREEN"In function '%s' call: formal param's type is '%s', but real argument's type is '%s'"NONE"\n", e->pos, id->type_id, plist->type, rArgType);
                        break;
                    }

                    plist = plist->next;
                    args = args->childNode[2];

                    //判断实参与形参个数是否匹配
                    if((plist != NULL) && (args == NULL)){
                        //实参少于形参个数
                        printf(RED"Error type 60 at Line %d : "GREEN"Function '%s' call doesn't have enough params"NONE"\n", e->pos, id->type_id);
                        break;
                    }else if((plist == NULL) && (args != NULL)){
                        //实参多余形参个数
                        printf(RED"Error type 61 at Line %d : "GREEN"Function '%s' call has nuisance params"NONE"\n", e->pos, id->type_id);
                        break;
                    }

                }
            }
        }
        else if(isVarDec(id)){
            printf(RED"Error type 41 at Line %d : "WHITE"Implicit function call using non function name '%s' of variable"NONE"\n", id->pos, id->type_id);
            strcpy(content, typeVar(id));
        }
        else{
            printf(RED"Error type 20 at Line %d : "PURPLE"Undefined function call of '%s'"NONE"\n", id->pos, id->type_id);
            strcpy(content, "Unknown Type");
        }
    }
    else if((exp->childNode[0]->kind == EXP) && (exp->childNode[2]->kind == EXP)){
        //首先是双目操作符,中间是operator,两端是Exp节点,进行类型匹配判断
        //Exp->Exp Operator Exp
        char* left_content = analyzeExp(exp->childNode[0]);
        char* right_content = analyzeExp(exp->childNode[2]);
        if(strcmp(left_content, right_content) != 0)
            printf(RED"Error type 150 at Line %d : "BROWN"Type mismatch at both ends of Binocular Expression[Left:%s Right:%s]"NONE"\n", exp->pos, left_content, right_content);
        //优先返回左部表达式的类型
        return left_content;
    }
    else if((exp->childNode[0]->kind == LP) && (exp->childNode[2]->kind == RP)){
        //括号里的EXP表达式
        return analyzeExp(exp->childNode[1]);
    }

    return content;
}

int analyzeReturn(struct ASTnode* rStmt){
    if(cur_func_ID == NULL)
        return 0;
    cur_func_has_return = 1;        //用以标记当前函数是否有返回值
    struct ASTnode* rExp = rStmt->childNode[1];
    char* fType = typeFunc(cur_func_ID);
    char* rType = analyzeExp(rExp);
    if(strcmp(fType, rType) != 0)
        printf(RED"Error type 160 at Line %d : "LIGHT_PURPLE"Actual return value's type '%s' mismatch defined function type '%s'"NONE"\n", rExp->pos, rType, fType);
    pFunc p = current_ST->func_st;
    while(p != NULL){
        if(!strcmp(p->name, cur_func_ID->type_id)){
            strcpy(p->rType, rType);
            break;
        }
        p = p->next;
    }
}

void printVar_ST(pVar_ST_Head var_ST_head){
    if(var_ST_head == NULL){
        printf("Error, 变量符号表为空！");
        return;
    }
    pVar tempVar = var_ST_head->next;
    printf(CYAN"变量符号表\n");
    printf(LIGHT_CYAN"┌───────────────────────┬───────────────────────┬───────────────────────┐\n");
    printf("变量名\t\t\t类型\t\t\t行号\t\t\t层号"NONE"\n");
    while(tempVar != NULL){
        printf(" %s\t\t\t%s\t\t\t %d\t\t\t %d\n", tempVar->name, tempVar->type, tempVar->line, tempVar->lvl);
        tempVar = tempVar->next;
    }
    printf(LIGHT_CYAN"└───────────────────────┴───────────────────────┴───────────────────────┘"NONE"\n");
}

void printFunc_ST(pFunc_ST_Head func_ST_head){
    if(func_ST_head == NULL){
        printf("Error, 函数符号表为空！");
        return;
    }
    pFunc tempF = func_ST_head;
    printf(GREEN"函数符号表\n");
    printf(LIGHT_GREEN"┌────────────────────┬────────────────┬────────────┬────────────┐\n");
    printf("函数名\t\t函数返回类型\t实际返回类型\t形参个数\t行号"NONE"\n");
    while(tempF != NULL){
        printf(" %s\t\t  %s\t\t    %s\t   %d\t\t %d\n",tempF->name, tempF->fType, tempF->rType, tempF->paramNum, tempF->line);
        /*if(tempF->paramNum != 0){
            pVar tempParam = tempF->paramList;
            int i = 1;
            while(tempParam != NULL){
                printf("形参 %d : %s %s\n", i++, tempParam->type, tempParam->name);
                tempParam = tempParam->next;
            }
        }*/
        tempF = tempF->next;
    }
    printf(LIGHT_GREEN"└────────────────────┴────────────────┴────────────┴────────────┘"NONE"\n");
}

void printStruct_ST(pStruct_ST_Head struc_ST_head){
    if(struc_ST_head == NULL){
        printf("Error, 结构体符号表为空！");
        return;
    }else if(struc_ST_head->next == NULL)
        return;
    pStruct tempStruc = struc_ST_head->next;
    printf(BROWN"变量符号表\n");
    printf(YELLOW"┌───────────────────────┬───────────────────────┐\n");
    printf("结构体名\t\t成员个数\t\t行号"NONE"\n");
    while(tempStruc != NULL){
        printf(" %s\t\t\t%d\t\t\t %d\n", tempStruc->name, tempStruc->memberNum, tempStruc->line);
        tempStruc = tempStruc->next;
    }
    printf(YELLOW"└───────────────────────┴───────────────────────┘"NONE"\n");
}

void printScope_ST(pScope_ST scope_st){
    if(scope_st != NULL){
        printf("符号表作用域:  %s\n", scope_st->scope_name);
        printVar_ST(scope_st->var_st);
        printFunc_ST(scope_st->func_st);
        printStruct_ST(scope_st->struct_st);
    }
}
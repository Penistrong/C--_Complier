/*
* @Author:	Penistrong
* @Date:	2019-12-17 21:45:21
* @Last Modified by:	Penistrong
* @Last Modified time:	2019-12-17 21:45:21
* @Description: Exp3 TAC code generation
*/
#include "TAC.h"

struct opn* newOpn(){
    struct opn* newOpn = (struct opn *)malloc(sizeof(struct opn));
}

pTACnode newTACnode(){
    pTACnode newTACnode = (pTACnode)malloc(sizeof(struct TACnode));
    newTACnode->opn1 = newTACnode->opn2 = newTACnode->result = NULL;
    newTACnode->next = newTACnode->prior = NULL;
    return newTACnode;
}

pTACnode generateTAC(int op, int pNum, ...){
    va_list pArgs = NULL;
    va_start(pArgs, pNum);

    va_end(pArgs);
}
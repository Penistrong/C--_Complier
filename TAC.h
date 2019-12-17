/*
* @Author:	Penistrong
* @Date:	2019-12-17 21:36:27
* @Last Modified by:	Penistrong
* @Last Modified time:	2019-12-17 21:36:27
* @Description: Exp3 TAC code generation
*/
#ifndef _TAC_H
#define _TAC_H
#include "ast.h"

struct opn{

};

struct TACnode{
    int op;                         //操作类型说明
    struct opn opn1, opn2, result;  //操作数1,操作数2,运算结果
    struct TACnode *next, *prior;
};





#endif
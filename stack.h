/*
* @Author: Penistrong
* @Date:   2019-12-04 12:26:20
* @Last Modified by:   Penistrong
* @Last Modified time: 2019-12-04 12:26:36
*/

#include <stdio.h>
#include <stdlib.h>

#ifndef _STACK_H
#define _STACK_H
#include "semanticAnalyzer.h"

typedef struct Scope_Symbol_Table *pScope_ST; //前向声明。以防头文件展开时
typedef pScope_ST ElementType;

//链表栈
typedef struct LinkedStack
{
    ElementType e;
    struct LinkedStack *next;
}Stack, *pStack, *pStackBlock;

pStack initStack();
int isEmpty(pStack stack);//判空
int push(ElementType e);//压栈
ElementType pop();//弹出栈顶
ElementType top();//返回栈顶,不弹栈

#endif


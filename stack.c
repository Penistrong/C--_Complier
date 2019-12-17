/*
* @Author: Penistrong
* @Date:   2019-12-04 12:26:20
* @Last Modified by:   Penistrong
* @Last Modified time: 2019-12-08 21:25:40
*/

#include "stack.h"

pStack stack;

pStack initStack(){
    pStack newStack = (pStack)malloc(sizeof(Stack));
    newStack->e = NULL;
    newStack->next = NULL;
    return newStack;
}

int isEmpty(pStack stack){
    if(stack == NULL || stack->e == NULL)
        return 1;
    else return 0;
}

//如果要对stack进行操作，stack作为形参时，要在函数内部使用 *stack 改变实参所指向的真正地址
//不然无法使stack指向下一个节点
int push(ElementType e){
    if(stack == NULL)
        return 0;       //Assume 0 equ Failure
    pStackBlock newNode = (pStackBlock)malloc(sizeof(Stack));
    newNode->e = e;
    //头部插入
    newNode->next = stack;
    stack = newNode;
    return 1;
}

ElementType pop(){
    if(isEmpty(stack))
        return NULL;
    pStackBlock tempNode;
    tempNode = stack;
    stack = stack->next;
    return tempNode->e;
}

ElementType top(){
    if(isEmpty(stack))
        return NULL;
    return stack->e;
}

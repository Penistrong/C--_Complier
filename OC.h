/*
* @Author:	Penistrong
* @Date:	2019-12-24 21:38:55
* @Last Modified by:	Penistrong
* @Last Modified time:	2019-12-24 21:38:55
* @Description: 生成目标代码按照MIPS32输出
*/
#ifndef _OC_H
#define _OC_H
#include <string.h>
#include <stdlib.h>
#include "TAC.h"
#define REGISTERNUM 18

FILE* OC_out;
//寄存器结构体定义
struct Register{
    int isFull;
    char name[10];
    char content[10];
};

struct Register reg[18];    //MIPS使用的32个寄存器中暂时只取用$t0~$t9,$s0~$s7

//初始化寄存器
void initRegister();
int initOCgenerator();
void initOCasm(FILE* file);
void generateOC(pTACnode tac_head);

//根据要放入寄存器内的变量别名去寻找已放置该变量的寄存器,若无则使用新的寄存器存放
char* searchReg(char* alias);
char* newReg(char* alias);








#endif
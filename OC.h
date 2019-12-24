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

FILE* OC_out;
//寄存器结构体定义
struct Register{
    int isFull;
    char* name;
    char* content;
};

int initOCgenerator();
void initOCasm(FILE* file);
void generateOC(pTACnode tac_root);









#endif
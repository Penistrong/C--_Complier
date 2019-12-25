/*
* @Author:	Penistrong
* @Date:	2019-12-24 21:35:16
* @Last Modified by:	Penistrong
* @Last Modified time:	2019-12-24 21:35:16
* @Description: 生成目标代码
*/
#include "OC.h"

//srcR=sourceRegister, rR=resultRegister,存储各种即将要使用的寄存器的名字
char *srcR1, *srcR2, *rR;

int initOCgenerator(){
    printf("初始化OC生成器\n");
    if((OC_out = fopen("./ObjectCode.asm", "w")) == NULL)
        return 0;
    srcR1 = (char*)calloc(sizeof(char)*10, 1);
    srcR2 = (char*)calloc(sizeof(char)*10, 1);
    rR = (char*)calloc(sizeof(char)*10, 1);
    initRegister();
    initOCasm(OC_out);
}

void initRegister(){
    //初始化MIPS32的32个寄存器?
    //从寄存器t0用起,t0~t7,t8~t9
    int i = 0;
    for(i;i<10;i++){
        sprintf(reg[i].name, "$t%d", i);
        reg[i].isFull = 0;
    }
    //初始化$s0~$s7
    for(i;i<18;i++){
        sprintf(reg[i].name, "$s%d", i - 10);
        reg[i].isFull = 0;
    }
}

//给ObjectCode.asm添加初始化的头部,诸如read与write函数
void initOCasm(FILE* file){
    fprintf(file,".data\n");
    fprintf(file,"_prompt: .asciiz \"Enter an integer:\"\n");
    fprintf(file,"_ret: .asciiz \"\\n\" \n");
    fprintf(file,".globl main \n");
    fprintf(file,".text \n");
    fprintf(file,"read: \n");
    fprintf(file,"\tli $v0, 4 \n");
    fprintf(file,"\tla $a0, _prompt \n");
    fprintf(file,"\tsyscall \n");
    fprintf(file,"\tli $v0, 5 \n");
    fprintf(file,"\tsyscall \n");
    fprintf(file,"\tjr $ra \n");
    fprintf(file,"write: \n");
    fprintf(file,"\tli $v0, 1 \n");
    fprintf(file,"\tsyscall\n");
    fprintf(file,"\tli $v0, 4 \n");
    fprintf(file,"\tla $a0, _ret \n");
    fprintf(file,"\tsyscall \n");
    fprintf(file,"\tmove $v0, $0 \n");
    fprintf(file,"\tjr $ra \n");
    //fflush(file);
}

void generateOC(pTACnode tac_head){
    if(!initOCgenerator())
        return;
    pTACnode h = tac_head;
    char* val_str = (char*)malloc(sizeof(char)*10);     //转化立即数为其对应的字符串值
    do{
        switch(h->op){
        case LABEL:
            fprintf(OC_out, "%s:\n", h->result->id);
            break;
        case FUNCTION:
            fprintf(OC_out, "%s:\n",h->result->id);
            break;
        case GOTO:
            fprintf(OC_out, "j %s\n",h->result->id);
            break;
        case RETURN:
            strcpy(rR, searchReg(h->result->id));
            fprintf(OC_out, "move $v0, %s\n", rR);
            fprintf(OC_out, "jr $ra\n");
            break;
        case ASSIGN:
            switch(h->opn1->kind){
            case ID://寄存器取值赋给另一个寄存器
                strcpy(srcR1, searchReg(h->opn1->id));
                strcpy(rR, searchReg(h->result->id));
                fprintf(OC_out, "move %s, %s\n", rR, srcR1);
                break;
            case INT:
                strcpy(rR, searchReg(h->result->id));
                sprintf(val_str, "%d", h->opn1->const_int);
                fprintf(OC_out, "li %s, %s\n", rR, val_str);
                break;
            case FLOAT:
                strcpy(rR, searchReg(h->result->id));
                sprintf(val_str, "%f", h->opn1->const_float);
                fprintf(OC_out, "li %s, %s\n", rR, val_str);
                break;
            case DOUBLE:
                strcpy(rR, searchReg(h->result->id));
                sprintf(val_str, "%lf", h->opn1->const_double);
                fprintf(OC_out, "li %s, %s\n", rR, val_str);
                break;
            }
            break;
        case JL:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            fprintf(OC_out, "blt %s, %s, %s\n", srcR1, srcR2, h->result->id);
            break;
        case JLE:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            fprintf(OC_out, "ble %s, %s, %s\n", srcR1, srcR2, h->result->id);
            break;
        case JG:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            fprintf(OC_out, "bgt %s, %s, %s\n", srcR1, srcR2, h->result->id);
            break;
        case JGE:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            fprintf(OC_out, "bge %s, %s, %s\n", srcR1, srcR2, h->result->id);
            break;
        case EQ:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            fprintf(OC_out, "beq %s, %s, %s\n", srcR1, srcR2, h->result->id);
            break;
        case NEQ:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            fprintf(OC_out, "bne %s, %s, %s\n", srcR1, srcR2, h->result->id);
            break;
        case ADD:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            strcpy(rR, searchReg(h->result->id));
            fprintf(OC_out, "add %s, %s, %s\n", rR, srcR1, srcR2);
            break;
        case SUB:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            strcpy(rR, searchReg(h->result->id));
            fprintf(OC_out, "sub %s, %s, %s\n", rR, srcR1, srcR2);
            break;
        case MUL:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            strcpy(rR, searchReg(h->result->id));
            fprintf(OC_out, "mul %s, %s, %s\n", rR, srcR1, srcR2);
            break;
        case DIV:
            strcpy(srcR1, searchReg(h->opn1->id));
            strcpy(srcR2, searchReg(h->opn2->id));
            strcpy(rR, searchReg(h->result->id));
            fprintf(OC_out, "div %s, %s\n", srcR1, srcR2);
            fprintf(OC_out, "mflo %s\n", rR);
            break;
        case CALL:
            if (!strcmp(h->opn1->id,"write")){
                fprintf(OC_out,"addi $sp,$sp,-4\n");
                fprintf(OC_out,"sw $ra,0($sp)\n");
                fprintf(OC_out,"jal write\n");
                fprintf(OC_out,"lw $ra,0($sp)\n");
                fprintf(OC_out,"addi $sp ,$sp ,4\n");
            }
            else if (!strcmp(h->opn1->id,"read")){
                strcpy(rR, searchReg(h->result->id));
                fprintf(OC_out,"addi $sp,$sp,-4\n");
                fprintf(OC_out,"sw $ra,0($sp)\n");
                fprintf(OC_out,"jal read\n");
                fprintf(OC_out,"lw $ra,0($sp)\n");
                fprintf(OC_out,"addi $sp ,$sp ,4\n");
                fprintf(OC_out,"move %s,$v0\n", rR);
            }
            break;
        case ARG:
            strcpy(rR, searchReg(h->result->id));
            fprintf(OC_out, "move $a0, %s\n", rR);
            break;
        }
        h = h->next;
    }while(h != tac_head);
    printf("OC生成完毕,请查看当前目录下的ObjectCode.asm\n");
}

char* searchReg(char* alias){
    for(int i = 0;i<REGISTERNUM;i++)
        if(reg[i].isFull != 0)
            if(!strcmp(reg[i].content, alias)){
                if(reg[i].content[0]=='t')
                    reg[i].isFull = 0;//如果是寄存器储存的变量名是临时变量(tempX),则访问过后将其空置
                return reg[i].name;
            }
    //寻找后都没有,那么使用新的寄存器
    return newReg(alias);
}

char* newReg(char* alias){
    //找到第一个内含为空的寄存器,将变量名送入
    for(int i =0;i<REGISTERNUM;i++)
        if(reg[i].isFull == 0){
            reg[i].isFull = 1;
            strcpy(reg[i].content, alias);
            return reg[i].name;
        }
}
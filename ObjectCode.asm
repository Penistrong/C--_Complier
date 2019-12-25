.data
_prompt: .asciiz "Enter an integer:"
_ret: .asciiz "\n" 
.globl main 
.text 
read: 
	li $v0, 4 
	la $a0, _prompt 
	syscall 
	li $v0, 5 
	syscall 
	jr $ra 
write: 
	li $v0, 1 
	syscall
	li $v0, 4 
	la $a0, _ret 
	syscall 
	move $v0, $0 
	jr $ra 
main:
li $t0, 0
move $t0, $t0
li $t1, 1
move $t1, $t1
li $t2, 0
move $t2, $t2
addi $sp,$sp,-4
sw $ra,0($sp)
jal read
lw $ra,0($sp)
addi $sp ,$sp ,4
move $t3,$v0
move $t3, $t3
label4:
blt $t2, $t3, label3
j label2
label3:
add $t4, $t0, $t1
move $t4, $t4
move $a0, $t1
addi $sp,$sp,-4
sw $ra,0($sp)
jal write
lw $ra,0($sp)
addi $sp ,$sp ,4
move $t0, $t1
move $t1, $t4
li $t5, 1
add $t5, $t2, $t5
move $t2, $t5
j label4
label2:
li $t5, 0
move $v0, $t5
jr $ra

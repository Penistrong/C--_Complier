# C+-_Complier
This repository is created for the HUST SSE class “Complier Construction Principles and  Practice".
Just type
bison -d -v singleParser.y
flex singleParser.lex
gcc -o parser singleParser.tab.c ast.c stack.c semanticAnalyzer.c lex.yy.c -lfl
Then the syntax tree will appear in the syntaxTree.txt in your workpath
Others will be print on the shell.


/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton interface for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INT = 258,
     FLOAT = 259,
     DOUBLE = 260,
     VOID = 261,
     CHAR = 262,
     BOOL = 263,
     STRUCT = 264,
     ID = 265,
     TYPE = 266,
     RELOP = 267,
     ASSIGNOP = 268,
     ADD_OP = 269,
     SUB_OP = 270,
     MUL_OP = 271,
     DIV_OP = 272,
     AND_OP = 273,
     OR_OP = 274,
     NOT_OP = 275,
     INC_OP = 276,
     DEC_OP = 277,
     EQ_OP = 278,
     IF = 279,
     ELSE = 280,
     DO = 281,
     WHILE = 282,
     BREAK = 283,
     RETURN = 284,
     FOR = 285,
     SWITCH = 286,
     CASE = 287,
     CONTINUE = 288,
     SEMI = 289,
     COMMA = 290,
     LP = 291,
     RP = 292,
     LC = 293,
     RC = 294,
     LB = 295,
     RB = 296,
     COLON = 297,
     LOWER_THAN_ELSE = 298
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 22 ".\\singleParser.y"

    int type_int;
    float type_float;
    double type_double;

    char type_id[32];
    struct ASTnode *node;



/* Line 1676 of yacc.c  */
#line 106 "singleParser.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif

extern YYLTYPE yylloc;


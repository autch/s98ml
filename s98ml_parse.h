
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
     EOL = 258,
     COLON = 259,
     COMMA = 260,
     SLASH = 261,
     VERSION = 262,
     TIMER = 263,
     TAG = 264,
     DEVICE = 265,
     ENCODING = 266,
     LOOP_START = 267,
     LOOP_END = 268,
     SYNC = 269,
     HEX = 270,
     DEC = 271,
     OCT = 272,
     BIN = 273,
     HEX_PAIR = 274,
     PART = 275,
     SYMBOL = 276,
     STRING = 277
   };
#endif
/* Tokens.  */
#define EOL 258
#define COLON 259
#define COMMA 260
#define SLASH 261
#define VERSION 262
#define TIMER 263
#define TAG 264
#define DEVICE 265
#define ENCODING 266
#define LOOP_START 267
#define LOOP_END 268
#define SYNC 269
#define HEX 270
#define DEC 271
#define OCT 272
#define BIN 273
#define HEX_PAIR 274
#define PART 275
#define SYMBOL 276
#define STRING 277




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 1676 of yacc.c  */
#line 12 "s98ml_parse.y"

    struct hex_pair {
        uint8_t addr, val;
    } hp;
    struct timerspec {
        int numerator, denominator;
    } ts;
    char* s;
    int n;



/* Line 1676 of yacc.c  */
#line 109 "s98ml_parse.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif



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




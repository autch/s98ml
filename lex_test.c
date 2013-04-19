
#include <stdio.h>
#include <stdint.h>
#include "s98ml_parse.h"
#include "s98ml_lex.h"

void yyerror(char* s)
{
    printf(stderr, "%s\n", s);
}

int main()
{
    int t;
    YYSTYPE value;

    while((t = yylex(&value)) != 0) {
    }
    return 0;
}


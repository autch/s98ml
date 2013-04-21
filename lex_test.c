
#include <stdio.h>
#include <stdint.h>
#include "s98ml_parse.h"
#include "s98ml_lex.h"

void yyerror(char* s)
{
    fprintf(stderr, "%s\n", s);
}

int main()
{
    int t;
    YYSTYPE value;
    YYLTYPE location;
    yyscan_t scanner;

    yylex_init(&scanner);

    while((t = yylex(&value, &location, scanner)) != 0) {
    }

    yylex_destroy(scanner);

    return 0;
}


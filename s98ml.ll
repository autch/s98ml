%{
%}

%x STRING
%x COMMENT

LOOP_START \[
LOOP_END   \]
HEX        \$[0-9a-fA-F]+
OCT        \&[0-7]+
DEC        \#?[0-9]+
BIN        \'[01]+
HEX_PAIR   [0-9a-fA-F]{2}:[0-9a-fA-F]{2}

EOL        \n

COLON      \:
COMMA      \,
SLASH      \/


VERSION   #version
TIMER     #timer
TAG       #tag
DEVICE    #device

DIRECTIVES ({VERSION}|{TIMER}|{TAG}|{DEVICE})

PART      [A-Z]

SYMBOL    [_a-zA-Z0-9]+

%%

<<EOF>>		printf("EOF\n"); return 0;

";"		BEGIN(COMMENT); yymore();
<COMMENT>{EOL}  printf("COMMENT: %s\n", yytext); BEGIN(INITIAL); 
<COMMENT>[^\n]+ yymore();

\"		BEGIN(STRING); yymore();
<STRING>{EOL}	fprintf(stderr, "unterminated quote: %s_n", yytext); BEGIN(INITIAL);
<STRING>([^\"\n]|\\\"|\\\n)+ yymore();
<STRING>\"	printf("STRING: %s\n", yytext); BEGIN(INITIAL);

^{DIRECTIVES}	printf("DIR: %s\n", yytext);

{HEX}		printf("HEX: %s\n", yytext);
{OCT}		printf("OCT: %s\n", yytext);
{DEC}		printf("DEC: %s\n", yytext);
{BIN}		printf("BIN: %s\n", yytext);
{HEX_PAIR}	printf("HP: %s\n", yytext);

{COLON}		printf("COLON: %s\n", yytext);
{COMMA}		printf("COMMA: %s\n", yytext);
^{SLASH}	printf("SYNC: %s\n", yytext);
{SLASH}		printf("SLASH: %s\n", yytext);

^{LOOP_START}	printf("LOOP: %s\n", yytext);
^{LOOP_END}	printf("END: %s\n", yytext);

^{PART}		printf("PART: %s\n", yytext);

{SYMBOL}	printf("SYMBOL: %s\n", yytext);

{EOL}		printf("//\n");

[[:blank:]]+

.		printf("unrecognized token: \"%s\"\n", yytext);

%%

int main()
{
  yyin = stdin;
  return yylex();

}

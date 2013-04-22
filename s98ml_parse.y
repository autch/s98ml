/* -*- mode: c; tab-width: 8; -*- */

%{
    #include <stdio.h>
    #include <stdint.h>

    #define YYLEX_PARAM scanner
%}
%error-verbose
%pure_parser
%locations
%union{
    struct hex_pair {
        uint8_t addr, val;
    } hp;
    struct timerspec {
        int numerator, denominator;
    } ts;
    char* s;
    int n;
}
%{
    #include "s98c_types.h"
    #include "s98ml_lex.h"
    #include "s98c.h"
    extern void yyerror(YYLTYPE* loc, struct s98c* ctx, yyscan_t scanner, char* s);
%}
%parse-param { struct s98c* ctx }
%parse-param { yyscan_t scanner }

%type <ts> timerspec
%type <n> num
%type <n> optional_num

%token EOL
%token COLON
%token COMMA
%token SLASH
%token VERSION
%token TIMER
%token TAG
%token DEVICE
%token ENCODING
%token LOOP_START
%token LOOP_END SYNC

%token <n> HEX
%token <n> DEC
%token <n> OCT
%token <n> BIN
%token <hp> HEX_PAIR
%token <n> PART
%token <s> SYMBOL
%token <s> STRING

%destructor { free($$); } SYMBOL STRING

%%

lines:
| lines line
;

line: EOL
| directives EOL
| register_writes EOL
| commands EOL
;

directives: VERSION DEC		{ s98c_register_version(ctx, $2); }
| TIMER timerspec		{ s98c_register_timer(ctx, $2.numerator, $2.denominator); }
| TAG SYMBOL STRING		{ if(s98c_register_tag(ctx, $2, $3)) YYERROR; /*free($2); free($3);*/ }
| DEVICE SYMBOL num optional_num { if(s98c_register_device(ctx, $2, $3, $4)) YYERROR; free($2); }
| ENCODING SYMBOL		{ printf("ENC: %s\n", $2); free($2); }
;

timerspec: DEC SLASH DEC	{ $$.numerator = $1; $$.denominator = $3; }
| DEC				{ $$.numerator = $1; $$.denominator = DEFAULT_TIMER_DENOMINATOR; } 
;

optional_num:			{ $$ = -1; }
| num
;

register_writes: PART { if(s98c_set_part(ctx, $1)) YYERROR; } pairs
;

pairs: pairs pair
| pair
;

pair: HEX_PAIR			{ s98c_write_reg(ctx, $1.addr, $1.val); }
| num COMMA num			{ s98c_write_reg(ctx, $1, $3); }
;

commands: sync
| LOOP_START			{ s98c_set_loopstart(ctx); }
| LOOP_END			{ s98c_write(ctx, 0xfd); }
;

sync: SYNC			{ s98c_write_sync_n(ctx, 1); }
| SYNC num			{ s98c_write_sync_n(ctx, $2); }
;

num: HEX
| OCT
| DEC
| BIN
;

%%

%{
    #include <stdio.h>
    #include <stdint.h>
%}
%union{
    struct hex_pair {
        uint8_t addr, val;
    } hp;
    char* s;
    int n;
}

%token COLON COMMA SLASH
%token <n> HEX DEC OCT BIN
%token <hp> HEX_PAIR
%token EOL
%token VERSION TIMER TAG DEVICE ENCODING
%token <n> PART
%token LOOP_START LOOP_END SYNC
%token <s> SYMBOL STRING

%%

line: EOL
| directives EOL
| register_writes EOL
| commands EOL
;

directives: VERSION DEC {  }
| TIMER timerspec   { }
| TAG SYMBOL STRING   {  }
| DEVICE SYMBOL num optional_num {  }
| ENCODING SYMBOL { }
;

timerspec: DEC SLASH DEC
| DEC
;

optional_num:
| num
;

register_writes: PART pairs
;

pairs: pairs pair
| pair
;

pair: HEX_PAIR
| num COMMA num
;

commands: sync
| LOOP_START
| LOOP_END
;

sync: SYNC
| SYNC num
;

num: HEX
| OCT
| DEC
| BIN
;

%%

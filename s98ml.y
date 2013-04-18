%{

%}

%token COLON COMMA SLASH
%token HEX DEC OCT BIN HEX_PAIR
%token EOL
%token VERSION TIMER TAG DEVICE
%token PART
%token LOOP_START LOOP_END
%token SYMBOL STRING

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

sync: SLASH
| SLASH num
;

num: HEX
| OCT
| DEC
| BIN
;

%%

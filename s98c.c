
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "s98c_types.h"
#include "s98ml_parse.h"
#include "s98ml_lex.h"

#include "s98c.h"

void init_context(struct s98c* ctx);
void free_context(struct s98c* ctx);

int yyparse(struct s98c* ctx, yyscan_t scanner);

void yyerror(YYLTYPE* loc, struct s98c* ctx, yyscan_t scanner, char* s)
{
    fprintf(stderr, "%s\n", s);
}

int main(int ac, char** av)
{
    struct s98c context;
    struct s98c* ctx = &context;
    yyscan_t scanner;
    char* inputfile;
    FILE* infp;
    int ret;

    memset(ctx, 0, sizeof(struct s98c));

    if(ac < 2 || ac > 3) {
        fprintf(stderr, "Usage: %s inputfile.s98ml [outputfile.s98]\n", *av);
        return 1;
    }

    inputfile = *++av;
    infp = fopen(inputfile, "rb");
    if(infp == NULL) {
        perror(inputfile);
        return 1;
    }
    ctx->input_filename = inputfile;

    if(ac == 3) {
        ctx->output_filename = *++av;
    }

    init_context(ctx);

    yylex_init(&scanner);

    yyset_in(infp, scanner);

    ret = yyparse(ctx, scanner);

    yylex_destroy(scanner);

    fclose(infp);

    if(ret == 0) {
        write_s98(ctx);
    }

    free_context(ctx);

    return 0;
}

void init_context(struct s98c* ctx)
{
    ctx->header.version = -1;
    ctx->header.timer_numerator = DEFAULT_TIMER_NUMERATOR;
    ctx->header.timer_denominator = DEFAULT_TIMER_DENOMINATOR;

    ctx->dump_buffer = malloc(INITIAL_DUMP_SIZE);
    ctx->dump_size = INITIAL_DUMP_SIZE;
    ctx->p = ctx->dump_buffer;

    ctx->tags = calloc(INITIAL_TAGS, sizeof(struct s98taginfo));
    ctx->tags_allocated = INITIAL_TAGS;
    ctx->tags_count = 0;

    ctx->devices = calloc(INITIAL_DEVICES, sizeof(struct s98deviceinfo));
    ctx->dev_count = INITIAL_DEVICES;
    ctx->header.device_count = 0;
}

void free_context(struct s98c* ctx)
{
    if(ctx->devices) {
        free(ctx->devices);
    }
    if(ctx->tags) {
        int i;
        for(i = 0; i < ctx->tags_count; i++) {
            free(ctx->tags[i].key);
            free(ctx->tags[i].value);
        }
        free(ctx->tags);
    }
    if(ctx->dump_buffer) {
        free(ctx->dump_buffer);
    }
}

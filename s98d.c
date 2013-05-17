// s98 decomposer

#if HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE 500
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "s98d_types.h"
#include "s98d.h"

extern const
struct s98deviceinfo default_opna;

static const
char* device_names[] = {
    "none",
    "ym2149",
    "ym2203",
    "ym2612",
    "ym2608",
    "ym2151",
    "ym2413",
    "ym3526",
    "ym3812",
    "ymf262",
    "ay_3_8910",
    "sn76489",
    NULL
};

uint32_t read_dword(struct s98context* ctx)
{
    uint32_t r = 0;

    r = ctx->p[0] | (ctx->p[1] << 8) | (ctx->p[2] << 16) | (ctx->p[3] << 24);
    ctx->p += 4;
 
    return r;
}

uint8_t read_byte(struct s98context* ctx)
{
    return *(ctx->p++);
}

void set_offset(struct s98context* ctx, off_t offset)
{
    ctx->p = ctx->s98_buffer + offset;
}

const char*
device_name(enum s98devicetype type)
{
    if(type < s98NONE || type >= s98END_DEVICES)
        return NULL;

    return device_names[type];
}

int main(int ac, char** av)
{
    struct s98context context;
    struct s98context* ctx = &context;
    struct s98header* h = &context.header;
    FILE* fp;

    if(ac != 2) {
        fprintf(stderr, "Usage: %s filename.s98\n", *av);
        return 1;
    }

    fp = fopen(*++av, "rb");
    if(fp == NULL) {
        perror(*av);
        return -1;
    }

    memset(ctx, 0, sizeof context);

    fseek(fp, 0, SEEK_END);
    ctx->s98_size = ftell(fp);
    ctx->s98_buffer = malloc(ctx->s98_size);

    rewind(fp);
    fread(ctx->s98_buffer, 1, ctx->s98_size, fp);
    set_offset(ctx, 0);
    fclose(fp);

    if(read_header(ctx) != 0)
        goto cleanup;
    read_devices(ctx);

    printf("; S98 File: %s\n", *av);
    printf("; Offset to tag: 0x%08x (0 if none)\n", h->offset_to_tag);
    printf("; Dump start: 0x%08x\n", h->offset_to_dump);
    printf("; Loop start: 0x%08x (0 if non-looped)\n", h->offset_to_loop);

    printf("#version %d\n", h->version);
    printf("#timer %d/%d\n", h->timer_numerator, h->timer_denominator);

    read_tag(ctx);

    putchar('\n');
    {
        int i;
        for(i = 0; i < h->device_count; i++) {
            struct s98deviceinfo* info;
            info = ctx->devices + i;

            printf("#device %s %d $%02x\n", device_name(info->device), info->clock, info->panpot);
        }
    }

    putchar('\n');

    s98d_dump(ctx);

    putchar('\n');


cleanup:
    free_context(&context);

    return 0;
}

int read_tag(struct s98context* ctx)
{
    if(ctx->header.offset_to_tag == 0) {
        ctx->tag_or_title = NULL;
        return 0;
    }

    ctx->tag_or_title = (char*)(ctx->s98_buffer + ctx->header.offset_to_tag);
    if(ctx->header.version != 3) {
        printf("#tag title \"%s\"\n", ctx->tag_or_title);
        return 0;
    }
    
    char* tag_area = strdup(ctx->tag_or_title);
    int this_is_utf8 = 0;
    char* tags_start = tag_area + 5;
    if(memcmp(tag_area, "[S98]", 5) != 0) {
        free(tag_area);
        return -1;
    }
    if(memcmp(tag_area + 5, "\xef\xbb\xbf", 3) == 0) {
        this_is_utf8 = 1;
        tags_start += 3;
        printf("#encoding UTF-8\n");
    }

    char* line;
#ifdef HAVE_STRTOK_R
    char* saveptr = NULL;
    while((line = strtok_r(tags_start, "\x0a", &saveptr)) != NULL) {
#else
    while((line = strtok(tags_start, "\x0a")) != NULL) {
#endif
        tags_start = NULL;
        char* value = strchr(line, '=');
        if(value == NULL) break;
        *value = '\0';
        printf("#tag %s \"%s\"\n", line, value + 1);
    }
    
    free(tag_area);

    return 0;
}

void free_context(struct s98context* ctx)
{
    if(ctx->devices == &default_opna) {
        // neet not to free(ctx->devices)
    } else {
        free(ctx->devices);
    }
    free(ctx->s98_buffer);
}

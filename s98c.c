
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#include "s98c_types.h"
#include "s98ml_parse.h"
#include "s98ml_lex.h"

struct s98deviceref {
    char* name;
    enum s98devicetype dev_num;
} s98devicenames[] = {
    { "NONE",   s98NONE },
    { "YM2149", s98YM2149 },      { "SSG", s98YM2149 },
    { "YM2203", s98YM2203 },      { "OPN", s98YM2203 },
    { "YM2612", s98YM2612 },      { "OPN2", s98YM2612 },
    { "YM2608", s98YM2608 },      { "OPNA", s98YM2608 },
    { "YM2151", s98YM2151 },      { "OPM", s98YM2151 },
    { "YM2413", s98YM2413 },      { "OPLL", s98YM2413 },
    { "YM3526", s98YM3526 },      { "OPL", s98YM3526 },
    { "YM3812", s98YM3812 },      { "OPL2", s98YM3812 },
    { "YMF262", s98YMF262 },      { "OPL3", s98YMF262 },
    { "AY_3_8910", s98AY_3_8910 },{ "PSG", s98AY_3_8910 },
    { "SN76489", s98SN76489 },    { "DCSG", s98SN76489 },

    { NULL, s98NONE }
};


int s98c_register_version(struct s98c* ctx, int version)
{
    if(ctx->header.version == -1) {
        ctx->header.version = version;
        return 0;
    } else {
        fprintf(stderr, "Cannot place multiple #version's (already defined as %d)\n", ctx->header.version);
        return 1;
    }
}

void s98c_register_timer(struct s98c* ctx, int numerator, int denominator)
{
    ctx->header.timer_numerator = numerator;
    ctx->header.timer_denominator = denominator;
}

uint32_t s98c_find_device(char* symbol)
{
    struct s98deviceref* ref = s98devicenames;

    for(; ref->name != NULL; ref++) {
        if(strcasecmp(ref->name, symbol) == 0) {
            return ref->dev_num;
        }
    }

    return -1;
}

int s98c_register_device(struct s98c* ctx, char* dev_name, uint32_t clock, uint8_t panpot)
{
    if(ctx->header.version == -1) {
        fprintf(stderr, "Cannot define #device without #version\n");
        return 1;
    }
/*    if(ctx->header.version == 1) {
      fprintf(stderr, "Cannot define #device in S98 version 1\n");
      return 1;
      }*/

    {
        struct s98deviceinfo info;

        info.device = s98c_find_device(dev_name);
        if(info.device == -1) {
            fprintf(stderr, "Undefined device name: %s\n", dev_name);
            return 2;
        }
        info.clock = clock;
        info.panpot = panpot;
        info.reserved = 0;

        if(ctx->header.device_count >= ctx->dev_count) {
            ctx->dev_count += INITIAL_DEVICES;
            ctx->devices = realloc(ctx->devices, ctx->dev_count * sizeof(struct s98deviceinfo));
        }

        ctx->devices[ctx->header.device_count++] = info;
    }
    return 0;
}

int s98c_register_tag(struct s98c* ctx, char* tagname, char* value)
{
    int i;
    struct s98taginfo info;

    for(i = 0; i < ctx->tags_count; i++) {
        if(strcasecmp(ctx->tags[i].key, tagname) == 0) {
            fprintf(stderr, "Tag \"%s\" already exists: %s\n", tagname, ctx->tags[i].value);
            return 1;
        }
    }
    if(ctx->tags_count >= ctx->tags_allocated) {
        ctx->tags_allocated += INITIAL_TAGS;
        ctx->tags = realloc(ctx->tags, ctx->tags_allocated * sizeof(struct s98taginfo));
    }

    info.key = tagname;
    info.value = value;
    ctx->tags[ctx->tags_count++] = info;

    return 0;
}

void s98c_write(struct s98c* ctx, uint8_t n)
{
    if(ctx->p >= ctx->dump_buffer + ctx->dump_size) {
        ptrdiff_t p_offset = ctx->p - ctx->dump_buffer;
        ptrdiff_t loop_offset = 0;

        if(ctx->loop_start != NULL) {
            loop_offset = ctx->loop_start - ctx->dump_buffer;
        }

        ctx->dump_size += DUMP_SIZE_INCREMENT;
        ctx->dump_buffer = realloc(ctx->dump_buffer, ctx->dump_size);

        ctx->p = ctx->dump_buffer + p_offset;
        if(ctx->loop_start != NULL) {
            ctx->loop_start = ctx->dump_buffer + loop_offset;
        }
    }
    
    *ctx->p++ = n;
}

int s98c_set_part(struct s98c* ctx, int part)
{
    if(ctx->header.version == -1) {
        fprintf(stderr, "Missing #version\n");
        return 1;
    }
    if(part >= ctx->header.device_count * 2) {
        fprintf(stderr, "Use of part %c exceeds number of defined #device's\n",
                part + 'A');
        return 2;
    }

    ctx->current_device = part;
    return 0;
}

void s98c_write_reg(struct s98c* ctx, uint8_t addr, uint8_t value)
{
    s98c_write(ctx, ctx->current_device);
    s98c_write(ctx, addr);
    s98c_write(ctx, value);
}

void s98c_set_loopstart(struct s98c* ctx)
{
    ctx->loop_start = ctx->p;
    // s98 doesn't have LOOP START command, it's just an address
}

void s98c_write_sync_n(struct s98c* ctx, uint32_t num)
{
    if(num == 1) {
        s98c_write(ctx, 0xff);
    } else if(ctx->header.version != 1) {
        uint8_t v;

        s98c_write(ctx, 0xfe);
        num -= 2;
        do {
            v = (num & 0x7f);
            num >>= 7;
            if(num != 0) {
                v |= 0x80;
            }
            s98c_write(ctx, v);
        } while(num);
    } else {
        num -= 2;
        s98c_write(ctx, 0xfe);
        s98c_write(ctx, num & 0xff);
    }
}

void free_context(struct s98c* ctx);

int yyparse(struct s98c* ctx, yyscan_t scanner);

void yyerror(YYLTYPE* loc, struct s98c* ctx, yyscan_t scanner, char* s)
{
    fprintf(stderr, "%s\n", s);
}

int write_s98(struct s98c* ctx);

int main(int ac, char** av)
{
    struct s98c context;
    struct s98c* ctx = &context;
    struct s98deviceinfo default_device = DEFAULT_OPNA_DEVICE;
    yyscan_t scanner;

    memset(ctx, 0, sizeof(struct s98c));

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

    yylex_init(&scanner);

    yyparse(ctx, scanner);

    yylex_destroy(scanner);

    write_s98(ctx);

    free_context(ctx);

    return 0;
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

int write_dword(FILE* fp, uint32_t v)
{
    uint8_t b[4];

    b[0] = v & 0xff;
    b[1] = (v >> 8) & 0xff;
    b[2] = (v >> 16) & 0xff;
    b[3] = (v >> 24) & 0xff;

    return fwrite(b, 1, 4, fp);
}

char* find_tag_value(struct s98c* ctx, char* tagname)
{
    int i;
    for(i = 0; i < ctx->tags_count; i++) {
        if(strcasecmp(ctx->tags[i].key, tagname) == 0) {
            return ctx->tags[i].value;
        }
    }
    return NULL;
}

int write_s98v1(struct s98c* ctx, FILE* fp);
int write_s98v2(struct s98c* ctx, FILE* fp);
int write_s98v3(struct s98c* ctx, FILE* fp);

int write_s98(struct s98c* ctx)
{
    FILE* fp;

    fp = fopen("s98out.s98", "wb");

    if(ctx->header.version == 1) {
        write_s98v1(ctx, fp);
    }
    if(ctx->header.version == 2) {
        write_s98v2(ctx, fp);
    }
    if(ctx->header.version == 3) {
        write_s98v3(ctx, fp);
    }

    fclose(fp);

    return 0;
}

int write_s98v1(struct s98c* ctx, FILE* fp)
{
    int header_size = 0x40; 
    char* title = find_tag_value(ctx, "title");
    int tag_length = (title == NULL) ? 0 : strlen(title) + 1;
    int dump_offset = header_size + tag_length;
    int loop_offset = 0;
    int dump_length = ctx->p - ctx->dump_buffer;

    if(dump_offset < 0x80) dump_offset = 0x80;
    if(ctx->loop_start != NULL) {
        loop_offset = dump_offset + (ctx->loop_start - ctx->dump_buffer);
    }
    ctx->header.offset_to_tag = (title == NULL) ? 0 : 0x40;
    ctx->header.offset_to_dump = dump_offset;
    ctx->header.offset_to_loop = loop_offset;

    fwrite("S98", 1, 3, fp); // 0
    fputc('0' + ctx->header.version, fp); // 3
    write_dword(fp, ctx->header.timer_numerator); // 4
    write_dword(fp, 0); // 8
    write_dword(fp, 0); // c
    write_dword(fp, 0x40); // 10 
    write_dword(fp, ctx->header.offset_to_dump); //14
    write_dword(fp, ctx->header.offset_to_loop); //18

    char zero[40];
    memset(zero, 0, sizeof zero);
    fwrite(zero, 1, 36, fp);

    if(tag_length > 0) {
        fwrite(title, 1, tag_length, fp);
    }
        
    fwrite(zero, 1, 0x40 - tag_length, fp);

    fwrite(ctx->dump_buffer, 1, dump_length, fp);

    return 0;
}

int write_s98v2(struct s98c* ctx, FILE* fp)
{
    int header_size = 0x20; 
    char* title = find_tag_value(ctx, "title");
    int tag_length = (title == NULL) ? 0 : strlen(title) + 1;
    int devices_size = sizeof(struct s98deviceinfo) * (ctx->header.device_count + 1);
    int dump_offset = header_size + devices_size + tag_length;
    int loop_offset = 0;
    int dump_length = ctx->p - ctx->dump_buffer;

    if(ctx->loop_start != NULL) {
        loop_offset = dump_offset + (ctx->loop_start - ctx->dump_buffer);
    }
    ctx->header.offset_to_tag = (title == NULL) ? 0 : header_size + devices_size;
    ctx->header.offset_to_dump = dump_offset;
    ctx->header.offset_to_loop = loop_offset;

    fwrite("S98", 1, 3, fp); // 0
    fputc('0' + ctx->header.version, fp); // 3
    write_dword(fp, ctx->header.timer_numerator); // 4
    write_dword(fp, ctx->header.timer_denominator); // 8
    write_dword(fp, 0); // c
    write_dword(fp, ctx->header.offset_to_tag); // 10 
    write_dword(fp, ctx->header.offset_to_dump); //14
    write_dword(fp, ctx->header.offset_to_loop); //18
    write_dword(fp, 0);

    int i;
    for(i = 0; i < ctx->header.device_count; i++) {
        struct s98deviceinfo* info = ctx->devices + i;

        write_dword(fp, info->device);
        write_dword(fp, info->clock);
        write_dword(fp, info->panpot);
        write_dword(fp, info->reserved);
    }
    write_dword(fp, s98NONE);
    write_dword(fp, 0);
    write_dword(fp, 0);
    write_dword(fp, 0);
    
    if(tag_length > 0) {
        fwrite(title, 1, tag_length, fp);
    }
        
    fwrite(ctx->dump_buffer, 1, dump_length, fp);

    return 0;
}

char* serialize_tags(struct s98c* ctx)
{
    int i;
    size_t total_size = 5 + 1; //"[S98]" and the last nil

    if(ctx->tags_count == 0) return NULL;

    if(0) { total_size += 3; } // BOM (unsupported)
   
    for(i = 0; i < ctx->tags_count; i++) {
        struct s98taginfo* info = ctx->tags + i;
        
        total_size += strlen(info->key) + 1 + strlen(info->value) + 1; // key '=' value '\n'
    }

    char* buffer = malloc(total_size);
    char* p = buffer;
    
    p += sprintf(p, "[S98]");
    for(i = 0; i < ctx->tags_count; i++) {
        struct s98taginfo* info = ctx->tags + i;
        p += sprintf(p, "%s=%s\x0a", info->key, info->value);
    }
    *p++ = '\0';

    return buffer;
}

int write_s98v3(struct s98c* ctx, FILE* fp)
{
    int header_size = 0x20; 
    int devices_size = sizeof(struct s98deviceinfo) * ctx->header.device_count;
    int dump_offset = header_size + devices_size;
    int loop_offset = 0;
    int dump_length = ctx->p - ctx->dump_buffer;

    if(dump_offset < 0x80) dump_offset = 0x80;

    if(ctx->loop_start != NULL) {
        loop_offset = dump_offset + (ctx->loop_start - ctx->dump_buffer);
    }
    ctx->header.offset_to_tag = (ctx->tags_count == 0) ? 0 : dump_offset + dump_length;
    ctx->header.offset_to_dump = dump_offset;
    ctx->header.offset_to_loop = loop_offset;

    fwrite("S98", 1, 3, fp); // 0
    fputc('0' + ctx->header.version, fp); // 3
    write_dword(fp, ctx->header.timer_numerator); // 4
    write_dword(fp, ctx->header.timer_denominator); // 8
    write_dword(fp, 0); // c
    write_dword(fp, ctx->header.offset_to_tag); // 10 
    write_dword(fp, ctx->header.offset_to_dump); //14
    write_dword(fp, ctx->header.offset_to_loop); //18
    write_dword(fp, ctx->header.device_count);

    int i;
    for(i = 0; i < ctx->header.device_count; i++) {
        struct s98deviceinfo* info = ctx->devices + i;

        write_dword(fp, info->device);
        write_dword(fp, info->clock);
        write_dword(fp, info->panpot);
        write_dword(fp, info->reserved);
    }
    
    fwrite(ctx->dump_buffer, 1, dump_length, fp);

    char* tags = serialize_tags(ctx);
    if(tags != NULL) {
        fwrite(tags, 1, strlen(tags) + 1, fp);

        free(tags);
    }

    return 0;
}


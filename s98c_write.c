
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_ICONV
#include <iconv.h>
#endif
#include "s98c_types.h"

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
    char* filename = NULL;

    if(ctx->output_filename == NULL) {
        size_t fname_size;
        char* p;

        fname_size = strlen(ctx->input_filename);

        filename = malloc(fname_size + 8 + 1);
        strcpy(filename, ctx->input_filename);
        p = strrchr(filename, '.');
        if(p == NULL) p = filename + fname_size;
        strcpy(p, ".s98");

        fp = fopen(filename, "wb");
        if(fp == NULL) {
            perror(filename);
            free(filename);
            return 1;
        }
    } else {
        fp = fopen(ctx->output_filename, "wb");
        if(fp == NULL) {
            perror(ctx->output_filename);
            return 1;
        }
    }

    switch(ctx->header.version) {
    case 0:
    case 1:
        write_s98v1(ctx, fp);
        break;
    case 2:
        write_s98v2(ctx, fp);
        break;
    case 3:
        write_s98v3(ctx, fp);
        break;        
    }

    fclose(fp);
    free(filename);

    return 0;
}

int write_s98v1(struct s98c* ctx, FILE* fp)
{
    int header_size = 0x40; 
    char* title = find_tag_value(ctx, "title");
    int tag_length = (title == NULL) ? 0 : strlen(title) + 1;
    int dump_offset = header_size + tag_length; // where dump bytes *REALLY* starts
    int loop_offset = 0;
    int dump_length = ctx->p - ctx->dump_buffer;

    if(dump_offset < 0x80) dump_offset = 0x80;
    if(ctx->loop_start != NULL) {
        loop_offset = dump_offset + (ctx->loop_start - ctx->dump_buffer);
    }
    if(ctx->dump_start != NULL) {
        dump_offset += (ctx->dump_start - ctx->dump_buffer);
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
    int tag_offset = (title == NULL) ? 0 : header_size + devices_size;
    int loop_offset = 0;
    int dump_length = ctx->p - ctx->dump_buffer;

    if(ctx->loop_start != NULL) {
        loop_offset = dump_offset + (ctx->loop_start - ctx->dump_buffer);
    }
    if(ctx->dump_start != NULL) {
        dump_offset += (ctx->dump_start - ctx->dump_buffer);
    }
    ctx->header.offset_to_tag = tag_offset;
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

#ifdef HAVE_ICONV
int write_iconv(iconv_t cd, char* input, FILE* fp)
{
    char buffer[1024] = { 0 };
    char* in_p;
    char* out_p;
    size_t in_size, out_size, ret = -1;

    in_p = input;
    in_size = strlen(input);

    out_p = buffer;
    out_size = sizeof buffer;

    iconv(cd, NULL, NULL, NULL, NULL);
    iconv(cd, NULL, NULL, &out_p, &out_size);
    
    while(in_size > 0 && out_size > 0) {
        ret = iconv(cd, &in_p, &in_size, &out_p, &out_size);
        if(ret == -1) {
            switch(errno) {
            case EINVAL:
            case E2BIG:
            case EILSEQ:
            default:
                perror("iconv failed: ");
                return -1;
            }
        } else { 
            fwrite(buffer, 1, out_p - buffer, fp);
            out_p = buffer;
            out_size = sizeof buffer;
        }
    }
    return ret;
}

int write_s98v3_tags_utf8(struct s98c* ctx, FILE* fp)
{
    int i;
    iconv_t cd;

    if(ctx->tags_count == 0) return 0;

    cd = iconv_open("UTF-8", ctx->source_encoding);
    if(cd == (iconv_t)-1) {
        perror("Cannot setup charset conversion: ");
        return 1;
    }

    fprintf(fp, "[S98]\xef\xbb\xbf");

    for(i = 0; i < ctx->tags_count; i++) {
        struct s98taginfo* info = ctx->tags + i;

        write_iconv(cd, info->key, fp);
        fputc('=', fp);
        write_iconv(cd, info->value, fp);
        fputc('\x0a', fp);
    }

    fputc(0, fp);
    iconv_close(cd);
    
    return 0;
}
#endif

int write_s98v3_tags(struct s98c* ctx, FILE* fp)
{
    int i;

#ifdef HAVE_ICONV
    if(ctx->source_encoding != NULL) return write_s98v3_tags_utf8(ctx, fp);
#endif

    if(ctx->tags_count == 0) return 0;

    fprintf(fp, "[S98]");
    for(i = 0; i < ctx->tags_count; i++) {
        struct s98taginfo* info = ctx->tags + i;
        fprintf(fp, "%s=%s\x0a", info->key, info->value);
    }
    fputc(0, fp);

    return 0;
}


int write_s98v3(struct s98c* ctx, FILE* fp)
{
    int header_size = 0x20; 
    int devices_size = sizeof(struct s98deviceinfo) * ctx->header.device_count;
    int dump_offset = header_size + devices_size;
    int loop_offset = 0;
    int dump_length = ctx->p - ctx->dump_buffer;
    int tag_offset = (ctx->tags_count == 0) ? 0 : dump_offset + dump_length;

    if(ctx->loop_start != NULL) {
        loop_offset = dump_offset + (ctx->loop_start - ctx->dump_buffer);
    }
    if(ctx->dump_start != NULL) {
        dump_offset += (ctx->dump_start - ctx->dump_buffer);
    }
    ctx->header.offset_to_tag = tag_offset;
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
    write_s98v3_tags(ctx, fp);

    return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "s98c_types.h"

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


// s98 decomposer

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
    struct stat stat;
    int fd;

    fd = open(*++av, O_RDONLY);
    if(fd < 0) {
        perror(*av);
        return -1;
    }

    memset(ctx, 0, sizeof context);

    fstat(fd, &stat);
    ctx->s98_size = stat.st_size;
    ctx->s98_buffer = malloc(ctx->s98_size);
    read(fd, ctx->s98_buffer, ctx->s98_size);
    set_offset(ctx, 0);
    close(fd);

    read_header(ctx);
    read_devices(ctx);
//    read_tag(fp, &context);

    fprintf(stderr, "S98 Version %d\n", h->version);
    fprintf(stderr, "Sync period %d/%d seconds\n", h->timer_numerator, h->timer_denominator);
    fprintf(stderr, "Compression %s\n", h->compression ? "enabled" : "disabled");
    fprintf(stderr, "Offset to tag: 0x%08x (0 if none)\n", h->offset_to_tag);
    fprintf(stderr, "Dump start: 0x%08x\n", h->offset_to_dump);
    fprintf(stderr, "Loop start: 0x%08x (0 if non-looped)\n", h->offset_to_loop);
    fprintf(stderr, "Defined devices: %d\n", h->device_count);

    printf("#version %d\n", h->version);
    printf("#timer %d/%d\n", h->timer_numerator, h->timer_denominator);

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

    free_context(&context);

    return 0;
}

// this function rewind()s!!
int read_tag(struct s98context* ctx)
{
    if(ctx->header.offset_to_tag == 0) {
        ctx->tag_or_title = NULL;
        return 0;
    }

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

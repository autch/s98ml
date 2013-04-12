// s98 decomposer

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>

#include "s98_types.h"

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

int read_header(FILE* fp, struct s98header* header);

int read_devices(FILE* fp, struct s98context* ctx);
int read_tag(FILE* fp, struct s98context* ctx);

void free_context(struct s98context* ctx);

int read_dword(FILE* fp, uint32_t* result)
{
    uint8_t bytes[4] = { 0 };
    uint32_t r = 0;
    size_t bytes_read;

    bytes_read = fread(bytes, 1, 4, fp);
    if(bytes_read < 4) {
        return -1;
    }
    r = bytes[0];
    r |= bytes[1] << 8;
    r |= bytes[2] << 16;
    r |= bytes[3] << 24;
 
    *result = r;
    return 0;
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
    FILE* fp = fopen(*++av, "rb");
    struct s98context context;

    if(fp == NULL) {
        perror(*av);
        return -1;
    }

    memset(&context, 0, sizeof context);

    read_header(fp, &context.header);
    read_devices(fp, &context);
//    read_tag(fp, &context);

    fprintf(stderr, "S98 Version %d\n", context.header.version);
    fprintf(stderr, "Sync period %d/%d seconds\n", context.header.timer_numerator, context.header.timer_denominator);
    fprintf(stderr, "Compression %s\n", context.header.compression ? "enabled" : "disabled");
    fprintf(stderr, "Offset to tag: 0x%08x (0 if none)\n", context.header.offset_to_tag);
    fprintf(stderr, "Dump start: 0x%08x\n", context.header.offset_to_dump);
    fprintf(stderr, "Loop start: 0x%08x (0 if non-looped)\n", context.header.offset_to_loop);
    fprintf(stderr, "Defined devices: %d\n", context.header.device_count);

    printf("#version %d\n", context.header.version);
    printf("#timer %d/%d\n", context.header.timer_numerator, context.header.timer_denominator);

    putchar('\n');
    {
        int i;
        for(i = 0; i < context.header.device_count; i++) {
            struct s98deviceinfo* info;
            info = context.devices + i;

            printf("#device %s %d $%02x\n",
                   device_name(info->device),
                   info->clock, info->panpot);
        }
    }

    putchar('\n');

    {
        int ch;
        int sync_done = 0;
        int current_ch = -1;
        int line_chars = 0;
        uint8_t addr, value;
        off_t current_offset = context.header.offset_to_dump;

        fseek(fp, context.header.offset_to_dump, SEEK_SET);

        while((ch = fgetc(fp)) != EOF) {
            if(current_offset >= context.header.offset_to_tag) {
                break;
            }
            if(current_offset == context.header.offset_to_loop) {
                printf("\n\n[\n");
                current_ch = -1;
            }

            int ofs;
            if(ch < 0x80) {
                // register write
                addr = fgetc(fp);
                value = fgetc(fp);

                if(current_ch != ch) {
                    char part_name = ch + 'A';
                    printf("\n%c ", part_name);
                    current_ch = ch;
                    line_chars = 2;
                }
                if(line_chars + 6 >= 80) {
                    printf("\n%c ", current_ch + 'A');
                    line_chars = 2;
                }
                printf("%02x:%02x ", addr, value);
                line_chars += 6;
                ofs = 3;
            } else {
                switch(ch) {
                case 0xfd:
                    printf("\n\n]\n");
                    current_ch = -1;
                    ofs = 1;
                    break;
                case 0xfe:
                {
                    int times = 0;
                    if(context.header.version == 1) {
                        times = fgetc(fp) + 2;
                        ofs = 2;
                    } else {
                        int s = 0, n = 0;
                        uint8_t v = fgetc(fp);
                        ofs = 2;
                        do {
                            n |= (v & 0x7f) << s;
                            s += 7;
                            ofs++;
                        } while((v = fgetc(fp)) & 0x80);
                        ungetc(v, fp);
                        ofs--;
                        times = n + 2;
                    }
                    printf("\n/ %d", times);
                    current_ch = -1;
                    break;
                }
                case 0xff:
                    printf("\n/");
                    current_ch = -1;
                    ofs = 1;
                    break;
                default:
                    // 80-fc
                    ofs = -1;
                    break;
                }
            }
            if(ofs < 1) break;
            current_offset += ofs;
        }
    }

    fclose(fp);

    free_context(&context);

    return 0;
}

// this function rewind()s!!
int read_tag(FILE* fp, struct s98context* ctx)
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
}


#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "s98d_types.h"
#include "s98d.h"

int getvv(struct s98context* ctx)
{
    int s = 0, n = 0;
    uint8_t* p = ctx->p;
    do {
        n |= (*p & 0x7f) << s;
        s += 7;
    } while(*p++ & 0x80);
    ctx->p = p;
    return n + 2;
}

int s98d_dump(struct s98context* ctx)
{
    int ch;
    int current_ch = -1;
    int line_chars = 0;
    struct s98header* h = &ctx->header;
    uint8_t* tag_ptr;
    uint8_t* loop_ptr;

    set_offset(ctx, h->offset_to_dump);
    tag_ptr = ctx->s98_buffer + h->offset_to_tag;
    loop_ptr = ctx->s98_buffer + h->offset_to_loop;

    for(;;) {
        ch = read_byte(ctx);

        if(ctx->p >= tag_ptr) {
            break;
        }
        if(ctx->p == loop_ptr) {
            printf("\n\n[\n");
            current_ch = -1;
        }

        if(ch < 0x80) {
            uint8_t addr, value;
            // register write
            addr = read_byte(ctx);
            value = read_byte(ctx);

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
            continue;
        }
        switch(ch) {
        case 0xfd:
            printf("\n\n]\n");
            current_ch = -1;
            break;
        case 0xfe:
        {
            int times = 0;
            if(h->version == 1) {
                times = read_byte(ctx) + 2;
            } else {
                times = getvv(ctx);
            }
            printf("\n/ %d", times);
            current_ch = -1;
            break;
        }
        case 0xff:
            printf("\n/");
            current_ch = -1;
            break;
        default:
            // 80-fc
            return 1;
        }
    }

    return 0;
}


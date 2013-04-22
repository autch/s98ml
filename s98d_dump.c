
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include "s98d_types.h"
#include "s98d.h"

#define BILLION		(1000*1000*1000)

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

void step_timer(struct s98context* ctx, int count)
{
    long clock = count * ctx->header.timer_numerator;
    long clocks_per_nsec = BILLION / ctx->header.timer_denominator;

    ctx->sync_count += count;

    clock *= clocks_per_nsec;
    ctx->ts.tv_nsec += clock;
    if(ctx->ts.tv_nsec >= BILLION) {
	ctx->ts.tv_sec += ctx->ts.tv_nsec / BILLION;
	ctx->ts.tv_nsec %= BILLION;
    }
}

void print_timestamp(struct s98context* ctx, FILE* fp)
{
    struct timespec ts = ctx->ts;
    int hh, mm, ss, nn;

    nn = ts.tv_nsec;
    ss = ts.tv_sec % 60;
    mm = (ts.tv_sec / 60) % 60;
    hh = (ts.tv_sec / 60 / 60);

    fprintf(fp, "\t; %d:%02d:%02d.%09d (sync %" PRId64 ")",
	    hh, mm, ss, nn, ctx->sync_count); // hh:mm:ss.nnnnnnnnn
}

int s98d_dump(struct s98context* ctx)
{
    int ch;
    int current_ch = -1;
    int line_chars = 0;
    struct s98header* h = &ctx->header;
    uint8_t* tag_ptr;
    uint8_t* loop_ptr;
    uint8_t* end_ptr;

    set_offset(ctx, h->offset_to_dump);
    tag_ptr = ctx->s98_buffer + h->offset_to_tag;
    loop_ptr = ctx->s98_buffer + h->offset_to_loop;
    end_ptr = ctx->s98_buffer + ctx->s98_size;
    if(ctx->p < tag_ptr)
        end_ptr = tag_ptr;

    for(;;) {
        if(ctx->p == loop_ptr) {
            printf("\n\n[\n");
            current_ch = -1;
        }
        ch = read_byte(ctx);

        if(ctx->p > end_ptr) {
            // fprintf(stderr, "end of dump reached\n");
            break;
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
            printf("\n\n/ %d", times);
	    step_timer(ctx, times);
	    print_timestamp(ctx, stdout);
	    putchar('\n');
            current_ch = -1;
            break;
        }
        case 0xff:
            printf("\n\n/");
	    step_timer(ctx, 1);
	    print_timestamp(ctx, stdout);
	    putchar('\n');
            current_ch = -1;
            break;
        default:
            // 80-fc
            return 1;
        }
    }

    return 0;
}


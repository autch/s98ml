
#include <memory.h>
#include <stdio.h>

#include "s98d_types.h"
#include "s98d.h"

int read_header_v1(struct s98context* ctx);
int read_header_v2(struct s98context* ctx);
int read_header_v3(struct s98context* ctx);

int read_header(struct s98context* ctx)
{
    char signature[4] = { 0 };
    struct s98header* header = &ctx->header;

    memset(&ctx->header, 0, sizeof(struct s98header));

    memcpy(signature, ctx->p, 4);
    ctx->p += 4;

    if(memcmp(signature, "S98", 3) != 0) {
        fprintf(stderr, "Magic mismatch: %.*s\n", 3, signature);
        return -2;
    }

    switch(signature[3]) {
    case '0': // v1
    case '1': // v1
    case '2': // v2
    case '3': // v3
        header->version = signature[3] - '0';
        break;
    default:
        fprintf(stderr, "Unknown S98 version: %c\n", signature[3]);
        return -3;
    }

    switch(header->version) {
    case 1: return read_header_v1(ctx);
    case 2: return read_header_v2(ctx);
    case 3: return read_header_v3(ctx);
    default: return -1;
    }
}

int read_header_v1(struct s98context* ctx)
{
    struct s98header* header = &ctx->header;

    header->timer_numerator = read_dword(ctx);
    if(header->timer_numerator == 0)
        header->timer_numerator = DEFAULT_TIMER_NUMERATOR;
    header->timer_denominator = read_dword(ctx);
    header->timer_denominator = DEFAULT_TIMER_DENOMINATOR;
    header->compression = read_dword(ctx);
    
    if(header->compression != 0) {
        fprintf(stderr, "Compression is not supported\n");
        return -1;
    }

    header->offset_to_tag = read_dword(ctx);
    header->offset_to_dump = read_dword(ctx);
    header->offset_to_loop = read_dword(ctx);

    header->device_count = 1;

    // skip reserved headers
    set_offset(ctx, 0x40);

    return 0;
}

int read_header_v2(struct s98context* ctx)
{
    struct s98header* header = &ctx->header;

    header->timer_numerator = read_dword(ctx);
    if(header->timer_numerator == 0)
        header->timer_numerator = DEFAULT_TIMER_NUMERATOR;
    header->timer_denominator = read_dword(ctx);
    if(header->timer_denominator == 0)
        header->timer_denominator = DEFAULT_TIMER_DENOMINATOR;
    header->compression = read_dword(ctx);
    
    if(header->compression != 0) {
        fprintf(stderr, "Compression is not supported\n");
        return -1;
    }

    header->offset_to_tag = read_dword(ctx);
    header->offset_to_dump = read_dword(ctx);
    header->offset_to_loop = read_dword(ctx);
    header->offset_to_compressed_data = read_dword(ctx);

    return 0;
}

int read_header_v3(struct s98context* ctx)
{
    struct s98header* header = &ctx->header;

    header->timer_numerator = read_dword(ctx);
    if(header->timer_numerator == 0)
        header->timer_numerator = DEFAULT_TIMER_NUMERATOR;
    header->timer_denominator = read_dword(ctx);
    if(header->timer_denominator == 0)
        header->timer_denominator = DEFAULT_TIMER_DENOMINATOR;
    header->compression = read_dword(ctx);

    header->offset_to_tag = read_dword(ctx);
    header->offset_to_dump = read_dword(ctx);
    header->offset_to_loop = read_dword(ctx);
    header->device_count = read_dword(ctx);

    return 0;
}



#include <memory.h>
#include <stdio.h>

#include "s98_types.h"

int read_dword(FILE* fp, uint32_t* result);

int read_header_v1(FILE* fp, struct s98header* header);
int read_header_v2(FILE* fp, struct s98header* header);
int read_header_v3(FILE* fp, struct s98header* header);

int read_header(FILE* fp, struct s98header* header)
{
    size_t bytes_read;
    char signature[4] = { 0 };

    memset(header, 0, sizeof *header);

    bytes_read = fread(signature, 1, 4, fp);
    if(bytes_read < 4) return -1;

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
    case 1: return read_header_v1(fp, header);
    case 2: return read_header_v2(fp, header);
    case 3: return read_header_v3(fp, header);
    default: return -1;
    }
}

int read_header_v1(FILE* fp, struct s98header* header)
{
    read_dword(fp, &header->timer_numerator);
    if(header->timer_numerator == 0)
        header->timer_numerator = DEFAULT_TIMER_NUMERATOR;
    read_dword(fp, &header->timer_denominator);
    header->timer_denominator = DEFAULT_TIMER_DENOMINATOR;
    read_dword(fp, &header->compression);
    
    if(header->compression != 0) {
        fprintf(stderr, "Compression is not supported\n");
        return -1;
    }

    read_dword(fp, &header->offset_to_tag);
    read_dword(fp, &header->offset_to_dump);
    read_dword(fp, &header->offset_to_loop);

    header->device_count = 1;

    // skip reserved headers
    fseek(fp, 0x40, SEEK_SET);

    return 0;
}

int read_header_v2(FILE* fp, struct s98header* header)
{
    read_dword(fp, &header->timer_numerator);
    if(header->timer_numerator == 0)
        header->timer_numerator = DEFAULT_TIMER_NUMERATOR;
    read_dword(fp, &header->timer_denominator);
    if(header->timer_denominator == 0)
        header->timer_denominator = DEFAULT_TIMER_DENOMINATOR;
    read_dword(fp, &header->compression);
    
    if(header->compression != 0) {
        fprintf(stderr, "Compression is not supported\n");
        return -1;
    }

    read_dword(fp, &header->offset_to_tag);
    read_dword(fp, &header->offset_to_dump);
    read_dword(fp, &header->offset_to_loop);
    read_dword(fp, &header->offset_to_compressed_data);

    return 0;
}

int read_header_v3(FILE* fp, struct s98header* header)
{
    read_dword(fp, &header->timer_numerator);
    if(header->timer_numerator == 0)
        header->timer_numerator = DEFAULT_TIMER_NUMERATOR;
    read_dword(fp, &header->timer_denominator);
    if(header->timer_denominator == 0)
        header->timer_denominator = DEFAULT_TIMER_DENOMINATOR;
    read_dword(fp, &header->compression);
    read_dword(fp, &header->offset_to_tag);
    read_dword(fp, &header->offset_to_dump);
    read_dword(fp, &header->offset_to_loop);
    read_dword(fp, &header->device_count);

    return 0;
}


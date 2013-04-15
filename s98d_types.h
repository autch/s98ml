#if !defined(s98_types_h)
#define s98_types_h

#include <stdint.h>

#define DEFAULT_TIMER_NUMERATOR 10
#define DEFAULT_TIMER_DENOMINATOR 1000
#define DEFAULT_OPNA_CLOCK 7987200
#define DEFAULT_OPNA_DEVICE { s98YM2608, DEFAULT_OPNA_CLOCK, 0 }

enum s98devicetype {
    s98NONE = 0, // or end mark
    s98YM2149,
    s98YM2203,
    s98YM2612,
    s98YM2608,
    s98YM2151,
    s98YM2413,
    s98YM3526,
    s98YM3812,
    s98YMF262,
    s98AY_3_8910,
    s98SN76489,

    s98END_DEVICES
};

struct s98deviceinfo {
    uint32_t device; // use s98devicetype
    uint32_t clock;
    uint32_t panpot;
    uint32_t reserved;
};

struct s98header {
    int version;
    uint32_t timer_numerator;
    uint32_t timer_denominator;
    uint32_t compression;
    uint32_t offset_to_tag;
    uint32_t offset_to_dump;
    uint32_t offset_to_loop;
    uint32_t offset_to_compressed_data;
    uint32_t device_count;
};

struct s98context {
    struct s98header header;
    struct s98deviceinfo* devices;
    char* tag_or_title; // points to s98_buffer

    uint8_t* s98_buffer;
    size_t s98_size;
    uint8_t* p; // points s98_buffer
};

#endif // !s98_types_h

#if !defined(s98c_types_h)
#define s98c_types_h

#include <stdint.h>

#define DEFAULT_S98_VERSION			3
#define DEFAULT_TIMER_NUMERATOR		10
#define DEFAULT_TIMER_DENOMINATOR	1000
#define DEFAULT_OPNA_CLOCK			7987200
#define DEFAULT_OPNA_DEVICE			{ s98YM2608, DEFAULT_OPNA_CLOCK, 0 }

#define INITIAL_DEVICES				4
#define INITIAL_TAGS				4

#define INITIAL_DUMP_SIZE		    (256 * 1024)
#define DUMP_SIZE_INCREMENT			(16 * 1024)

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

struct s98taginfo {
    char* key;
    char* value;
};

struct s98c {
    struct s98header header;
    struct s98deviceinfo* devices;
    size_t dev_count; // # of devices allocated

    struct s98taginfo* tags;
    size_t tags_allocated;
    size_t tags_count;

    uint8_t* dump_buffer;
    size_t dump_size; // # of bytes allocated at dump_buffer
    uint8_t* p; // points dump_buffer

    uint8_t* loop_start; // points dump_buffer, address of LOOP_START

    int current_device; // current device, part - 'A'

    char* input_filename;
    char* output_filename; // points argv
};

#endif // !s98c_types_h

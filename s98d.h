#if !defined(s98d_h)
#define s98d_h

#include <sys/types.h>
#include <stdint.h>
#include "s98d_types.h"

int read_header(struct s98context* ctx);
int read_devices(struct s98context* ctx);
int read_tag(struct s98context* ctx);

void free_context(struct s98context* ctx);

uint32_t read_dword(struct s98context* ctx);
uint8_t read_byte(struct s98context* ctx);
void set_offset(struct s98context* ctx, off_t offset);

const char*
device_name(enum s98devicetype type);

int s98d_dump(struct s98context* ctx);

#endif //! s98d_h

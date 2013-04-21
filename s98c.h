#if !defined(s98c_h)
#define s98c_h

#include "s98c_types.h"

void s98c_register_version(struct s98c* ctx, int version);
void s98c_register_timer(struct s98c* ctx, int numerator, int denominator);
int s98c_register_device(struct s98c* ctx, char* dev_name, uint32_t clock, uint8_t panpot);
int s98c_register_tag(struct s98c* ctx, char* tagname, char* value);

int s98c_set_part(struct s98c* ctx, int part);
void s98c_write_reg(struct s98c* ctx, uint8_t addr, uint8_t value);
void s98c_set_loopstart(struct s98c* ctx);
void s98c_write_sync_n(struct s98c* ctx, uint32_t num);
void s98c_write(struct s98c* ctx, uint8_t n);

int write_s98(struct s98c* ctx);

#endif //! s98c_h

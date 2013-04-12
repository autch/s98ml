
#include <stdio.h>
#include <stdlib.h>
#include "s98_types.h"

const struct s98deviceinfo default_opna = DEFAULT_OPNA_DEVICE;

int read_dword(FILE* fp, uint32_t* result);

int read_devices(FILE* fp, struct s98context* ctx)
{
    if(ctx->header.version == 1) {
        ctx->devices = &default_opna;
        return 0;
    }

    if(ctx->header.version == 2) {
        int devices_allocated = 0;
        int devices = 0;

        ctx->devices = NULL;

        for(;;) {
            struct s98deviceinfo info;
            
            read_dword(fp, &info.device);
            read_dword(fp, &info.clock);
            read_dword(fp, &info.panpot);
            read_dword(fp, &info.reserved);

            if(info.device == s98NONE) break;

            if(devices + 1 >= devices_allocated)  {
                devices_allocated += 4;
                ctx->devices = realloc(ctx->devices, devices_allocated * sizeof(struct s98deviceinfo));
            }
            ctx->devices[devices] = info;
            devices++;
        }
        if(devices == 0) {
            ctx->devices = &default_opna;
            ctx->header.device_count = 1;
            return 0;
        } else {
            ctx->devices = realloc(ctx->devices, devices);
            ctx->header.device_count = devices;
        }
    }
    if(ctx->header.version == 3) {
        if(ctx->header.device_count == 0) {
            ctx->devices = &default_opna;
            ctx->header.device_count = 1;
            return 0;
        }

        int devices;

        ctx->devices = calloc(ctx->header.device_count, sizeof(struct s98deviceinfo));

        for(devices = 0; devices < ctx->header.device_count; devices++) {
            struct s98deviceinfo info;
            
            read_dword(fp, &info.device);
            read_dword(fp, &info.clock);
            read_dword(fp, &info.panpot);
            read_dword(fp, &info.reserved);

            ctx->devices[devices] = info;
        }
    }
    return 0;
}

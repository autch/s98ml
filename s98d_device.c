
#include <stdio.h>
#include <stdlib.h>
#include "s98_types.h"
#include "s98d.h"

struct s98deviceinfo default_opna = DEFAULT_OPNA_DEVICE;

int read_devices(struct s98context* ctx)
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
            
            info.device = read_dword(ctx);
            info.clock = read_dword(ctx);
            info.panpot = read_dword(ctx);
            info.reserved = read_dword(ctx);

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

            info.device = read_dword(ctx);
            info.clock = read_dword(ctx);
            info.panpot = read_dword(ctx);
            info.reserved = read_dword(ctx);

            ctx->devices[devices] = info;
        }
    }
    return 0;
}

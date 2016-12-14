#include "config.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>


#define BUF_SIZE 100

static const uint32_t NO_FREQUENCY = UINT32_C(0xFFFFFFFF);
static const uint32_t NO_BANDWIDTH = UINT32_C(0xFFFFFFFF);

static const uint32_t DEFAULT_FREQUENCY = UINT32_C(754000000);
static const uint32_t DEFAULT_BANDWIDTH = UINT32_C(8);


#define MAKE_GETTER(TYPE, NAME, NOT_FOUND, CONVERSION) \
static TYPE get_##NAME(FILE *f) \
{ \
    rewind(f); \
 \
    while (!feof(f)) \
    { \
        char buf[BUF_SIZE]; \
        fgets(buf, BUF_SIZE, f); \
 \
        TYPE ret; \
        if (sscanf(buf, #NAME" = "CONVERSION, &ret) == 1) \
            return ret; \
    } \
 \
    return NOT_FOUND; \
}

MAKE_GETTER(uint32_t, frequency, NO_FREQUENCY, "%"SCNu32);
MAKE_GETTER(uint32_t, bandwidth, NO_BANDWIDTH, "%"SCNu32);


struct config_init_ch_info config_get_init_ch_info(FILE *f)
{
    struct config_init_ch_info init_info = { 0 };

    uint32_t frequency = get_frequency(f);
    init_info.freq = (frequency == NO_FREQUENCY)? DEFAULT_FREQUENCY : frequency;

    uint32_t bandwidth = get_bandwidth(f);
    init_info.bandwidth = (bandwidth == NO_BANDWIDTH)? DEFAULT_BANDWIDTH : bandwidth;

    return info;
}

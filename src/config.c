/*! \file config.c
    \brief Implementation for the configuration file interface.
*/
// Matching include
#include "config.h"
// C includes
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
// Local includes
#include "tdp_api.h"


/// \param Buffer size for reading the file.
#define BUF_SIZE 100


static const uint32_t NO_FREQUENCY = UINT32_C(0xFFFFFFFF);
static const uint32_t NO_BANDWIDTH = UINT32_C(0xFFFFFFFF);
static const uint16_t NO_VIDEO_PID = UINT16_C(0xFFFF);
static const uint16_t NO_AUDIO_PID = UINT16_C(0xFFFF);
static const uint16_t NO_CH_NUM    = UINT16_C(0xFFFF);
static const int      NO_MODULE    = -1;
static const int      NO_VIDEO_TYPE = -1;
static const int      NO_AUDIO_TYPE = -1;
static const int      NO_TELETEXT = -1;

static const uint32_t DEFAULT_FREQUENCY = UINT32_C(754000000);
static const uint32_t DEFAULT_BANDWIDTH = UINT32_C(8);
static const uint16_t DEFAULT_VIDEO_PID = UINT16_C(101);
static const uint16_t DEFAULT_AUDIO_PID = UINT16_C(103);
static const uint16_t DEFAULT_CH_NUM    = UINT16_C(490);
static const int      DEFAULT_MODULE    = DVB_T;
static const int      DEFAULT_VIDEO_TYPE = VIDEO_TYPE_MPEG2;
static const int      DEFAULT_AUDIO_TYPE = AUDIO_TYPE_MPEG_AUDIO;
static const int      DEFAULT_TELETEXT = 0;


/// \brief Creates a getter function for a field.
/// The generated function is named get_NAME, and searches through the file,
/// for the field NAME, with type TYPE, and input conversion CONVERSION.
/// \param TYPE type of the field, and also return type of the function.
/// \param NAME name of the field, which is also part of the function name.
/// \param NOT_FOUND value of type \ref TYPE, to be returned if field was not
/// found.
/// \param CONVERSION A compile-time string that specifies the input
/// conversion à la scanf.
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

/// \brief Attempts to load frequency from file.
MAKE_GETTER(uint32_t, frequency, NO_FREQUENCY, "%"SCNu32);
/// \brief Attempts to load bandwidth from file.
MAKE_GETTER(uint32_t, bandwidth, NO_BANDWIDTH, "%"SCNu32);
/// \brief Attempts to load video PID from file.
MAKE_GETTER(uint16_t, video_pid, NO_VIDEO_PID, "%"SCNu16);
/// \brief Attempts to load audio PID from file.
MAKE_GETTER(uint16_t, audio_pid, NO_AUDIO_PID, "%"SCNu16);
/// \brief Attempts to load channel number from file.
MAKE_GETTER(uint16_t, ch_num, NO_CH_NUM, "%"SCNu16);
/// \brief Attempts to load module type from file.
MAKE_GETTER(int, module, NO_MODULE, "%d");
/// \brief Attempts to load video type from file.
MAKE_GETTER(int, video_type, NO_VIDEO_TYPE, "%d");
/// \brief Attempts to load audio type from file.
MAKE_GETTER(int, audio_type, NO_AUDIO_TYPE, "%d");
/// \brief Attempts t oload teletext from file
MAKE_GETTER(int, teletext, NO_TELETEXT, "%d");


/// \brief Attempts to load all fields from the file.
struct config_init_ch_info config_get_init_ch_info(FILE *f)
{
    struct config_init_ch_info init_info = { 0 };

    uint32_t frequency = get_frequency(f);
    init_info.freq = (frequency == NO_FREQUENCY)? DEFAULT_FREQUENCY : frequency;

    uint32_t bandwidth = get_bandwidth(f);
    init_info.bandwidth = (bandwidth == NO_BANDWIDTH)? DEFAULT_BANDWIDTH : bandwidth;

    uint16_t video_pid = get_video_pid(f);
    init_info.vpid = (video_pid == NO_VIDEO_PID)? DEFAULT_VIDEO_PID : video_pid;

    uint16_t audio_pid = get_audio_pid(f);
    init_info.apid = (audio_pid == NO_AUDIO_PID)? DEFAULT_AUDIO_PID : audio_pid;

    uint16_t ch_num = get_ch_num(f);
    init_info.ch_num = (ch_num == NO_CH_NUM)? DEFAULT_CH_NUM : ch_num;

    int module = get_module(f);
    init_info.module = (module == NO_MODULE)? DEFAULT_MODULE : module;

    int video_type = get_video_type(f);
    init_info.vtype = (video_type == NO_VIDEO_TYPE)? DEFAULT_VIDEO_TYPE : video_type;

    int audio_type = get_audio_type(f);
    init_info.atype = (audio_type == NO_AUDIO_TYPE)? DEFAULT_AUDIO_TYPE : audio_type;

    int teletext = get_teletext(f);
    init_info.teletext = (teletext == NO_TELETEXT)? DEFAULT_TELETEXT : teletext;

    return init_info;
}


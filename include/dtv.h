/*! \file dtv.h
    \brief Contains DTV API
*/
#ifndef DTV_H
#define DTV_H


#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <inttypes.h>
#include "tdp_api.h"
#include "config.h"
#include "parsing.h"



#define END_OF_CHANNELS UINT16_C(0xFFFF)

struct dtv_channel_info
{
    uint16_t ch_num;
    uint16_t vpid;
    uint16_t apid;
    bool teletext;
};

/// \brief Function that initializes internal DTV state.
void dtv_init(struct config_init_ch_info init_info);
/// \brief Tries to switch to the desired channel.
struct dtv_channel_info dtv_switch_channel(uint16_t ch_num);
/// \brief Tries to set the volume to the desired value.
/// \param vol Desired volume, should be [0-10].
t_Error dtv_set_volume(uint8_t vol);
/// \brief Gets the time information
struct tm dtv_get_time();
/// \brief Gets the channel descriptions
struct sdt dtv_get_info(uint16_t ch_num);
/// \brief Deinitializes the internal DTV state.
void dtv_deinit();


#endif


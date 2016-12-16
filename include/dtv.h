/*! \file dtv.h
    \brief Contains DTV API
*/
#ifndef DTV_H
#define DTV_H


#include <stdint.h>
#include <inttypes.h>
#include "tdp_api.h"
#include "config.h"


#define END_OF_CHANNELS UINT16_C(0xFFFF)


/// \brief Function that initializes internal DTV state.
void dtv_init(struct config_init_ch_info init_info);
/// \brief Returns a pointer to the array of program (channel) numbers.
const uint16_t* dtv_get_channels();
/// \brief Tries to switch to the desired channel.
t_Error dtv_switch_channel(uint16_t ch_num);
/// \brief Tries to set the volume to the desired value.
/// \param vol Desired volume, should be [0-10].
t_Error dtv_set_volume(uint8_t vol);
/// \brief Deinitializes the internal DTV state.
void dtv_deinit();


#endif


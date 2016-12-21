/*! \file dtv.h
    \brief Contains DTV API
*/
#ifndef DTV_H
#define DTV_H


// C includes
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
// Local includes
#include "config.h"
#include "parsing.h"
#include "tdp_api.h"


/// \defgroup dtv DTV interface
/// \addtogroup dtv
/// @{
/// \brief Functions and structures for controling DTV functionality.

/// \brief Contains basic channel info.
struct dtv_channel_info
{
    uint16_t ch_num; ///< Channel number.
    uint16_t vpid; ///< PID of the video stream.
    uint16_t apid; ///< PID of the audio stream.
    bool teletext; ///< Specifies whether the channel has teletext.
};


/// \brief Function that initializes internal DTV state.
void dtv_init(struct config_init_ch_info init_info);
/// \brief Tries to switch to the desired channel.
struct dtv_channel_info dtv_switch_channel(uint16_t ch_num);
/// \brief Tries to set the volume to the desired value.
/// \param vol Desired volume, should be [0-10].
t_Error dtv_set_volume(uint8_t vol);
/// \brief Gets the time information.
struct tm dtv_get_time();
/// \brief Gets the SDT information for the specified channel.
struct sdt dtv_get_info(uint16_t ch_num);
/// \brief Deinitializes the internal DTV state.
void dtv_deinit();
/// @}


#endif


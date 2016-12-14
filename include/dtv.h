/*! \file dtv.h
    \brief Contains DTV API
*/
#ifndef DTV_H
#define DTV_H


#include "tdp_api.h"


/// \brief A structure that holds the initial dtv settings.
struct init_pr_info
{
	uint32_t freq;
	uint32_t bw;
	t_Module module;
	uint16_t vpid; ///< pid of the initial video stream
	uint16_t apid; ///< pid of the initial audio stream
	t_StreamType vtype; ///< type of the inital video stream
	t_StreamType atype; ///< type of the initial audio stream
	uint32_t pr_num;
};


/// \brief Function that initializes internal DTV state.
void dtv_init(struct init_pr_info init_info);
/// \brief Returns a pointer to the array of program (channel) numbers.
const uint16_t* dtv_get_channels();
/// \brief Tries to switch to the desired channel.
t_Error dtv_switch_channel(uint16_t pr_num);
/// \brief Tries to set the volume to the desired value.
/// \param vol Desired volume, should be [0-10].
t_Error dtv_set_volume(uint8_t vol);
/// \brief Deinitializes the internal DTV state.
void dtv_deinit();


#endif


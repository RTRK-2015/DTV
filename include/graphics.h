/*! \file graphics.h
    \brief Contains the graphics API
*/
#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <stdbool.h>
#include "tdp_api.h"


/// \brief A struct that contains some basic channel info.
struct graphics_channel_info
{
	uint16_t ch_num; ///< The number of the channel.
	bool teletex; ///< Whether the channel has teletext.
	uint16_t vpid; ///< The video PID of the channel.
	uint16_t apid; ///< The audio PID of the channel.
};


/// \brief Initializes the internal graphics state.
t_Error graphics_init();
/// \brief Displays some basic information about a channel on the screen.
t_Error graphics_show_channel_info(struct graphics_channel_info info);
/// \brief Displays volume information on the screen.
t_Error graphics_show_volume(uint8_t vol);
/// \brief Deinitializes the internal graphics state.
void graphics_deinit();


#endif


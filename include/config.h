/*! \file config.h
    \brief Contains configuration file API
*/
#ifndef CONFIG_H
#define CONFIG_H


#include "tdp_api.h"


/// \brief A structure that holds the initial dtv settings.
struct config_init_ch_info
{
    uint32_t freq;
    uint32_t bandwidth;
    t_Module module; ///< Whether the channel uses DTB-T or DTB-T2.
    uint16_t vpid; ///< pid of the initial video stream.
    uint16_t apid; ///< pid of the initial audio stream.
    t_StreamType vtype; ///< type of the inital video stream.
    t_StreamType atype; ///< type of the initial audio stream.
    uint32_t ch_num; ///< Channel number.
};


/// \brief Reads the initial settings from the specified file
struct config_init_ch_info config_get_init_ch_info(FILE *f);


#endif

/*! \file config.h
    \brief Contains configuration file API
*/
#ifndef CONFIG_H
#define CONFIG_H


// Local includes
#include <stdio.h>
#include "tdp_api.h"


/// \defgroup config Configuration file interface
/// \addtogroup config
/// @{
/// \brief Functions and structures for retrieveing configuration options.

/// \brief A structure that holds the initial dtv settings.
struct config_init_ch_info
{
    uint32_t freq; ///< Tuner frequency.
    uint32_t bandwidth; ///< Tuner bandwidth.
    enum t_Module module; ///< Whether the channel uses DTB-T or DTB-T2.
    uint16_t vpid; ///< pid of the initial video stream.
    uint16_t apid; ///< pid of the initial audio stream.
    enum t_StreamType vtype; ///< type of the inital video stream.
    enum t_StreamType atype; ///< type of the initial audio stream.
    uint32_t ch_num; ///< Channel number.
};


/// \brief Reads the initial settings from the specified file
struct config_init_ch_info config_get_init_ch_info(FILE *f);
/// @}


#endif


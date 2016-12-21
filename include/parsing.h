/*! \file parsing.h
    \brief Contains streamlined internal representations of tables.
*/
#ifndef PARSING_H
#define PARSING_H


// C includes
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>


struct pmt;


/// \defgroup parsing Table parsing interface
/// \addtogroup parsing
/// @{
/// \brief Functions and structures for parsing DVB tables into internal form.

/// \brief Contains important info from the PAT table.
struct pat
{
    uint16_t tsi; ///< Transport stream id.

    size_t pmt_len; ///< How many PMTs exist in the stream.

    /// \brief Contains enough info to identify a PMT table.
    struct pmt_basic
    { 
    	uint16_t pid; ///< The PID of the PMT table.
    	uint16_t ch_num; ///< The channel number of the PMT table.
    } *pmts; ///< Array of PMTs that exist in the stream.
};


/// \brief Contains important info from the PMT table.
struct pmt
{
    uint16_t pid; ///< The pid of the PMT table.
    uint16_t ch_num; ///< The channel number of the PMT table.

    uint16_t video_pid;
    ///< PID of the video stream. It is -1 if it doesn't exist.
    uint16_t audio_pid;
    ///< PID of the audio stream. It is -1 if it doesn't exist.

    bool teletext; ///< Specifies whether the channel has teletext.
};


/// \brief Contains important info from the SDT table.
struct sdt
{
    uint8_t st; ///< Service type.
    char name[100]; ///< Channel name.
};


/// \brief Parses the given buffer as a PAT table.
struct pat parse_pat(const uint8_t *buffer);
/// \brief Parses the given buffer as a PMT table.
struct pmt parse_pmt(const uint8_t *buffer);
/// \brief Parses the given buffer as a TOT table and converts its info to C
/// representation of time.
struct tm  parse_tot(const uint8_t *buffer);
/// \brief Parses the given buffer as a SDT table and extracts information
/// about the specified channel.
struct sdt parse_sdt(const uint8_t *buffer, uint16_t ch_num);
/// @}


#endif


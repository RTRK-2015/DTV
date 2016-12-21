/*! \file rc.h
    \brief Contains Remote Control API
*/
#ifndef RC_H
#define RC_H


// Local includes
#include "tdp_api.h"


/// \defgroup rc Remote-control interface
/// \addtogroup rc
/// @{
/// \brief Functions and structures for remote control interaction.

/// \brief A callback that should take action on key press.
typedef void (*rc_key_callback)(int key_no);


/// \brief A function that starts the loop that waits for input events
/// from the remote control.
/// \param dev Name of the device to capture events from.
void rc_start_loop(const char *dev, rc_key_callback callback);
/// \brief Stops the event loop.
void rc_stop_loop();
/// @}


#endif


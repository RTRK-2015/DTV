/*! \file rc.h
    \brief Contains Remote Control API
*/
#ifndef RC_H
#define RC_H


// Local includes
#include "tdp_api.h"


/// Specifies the code of the back key.
#define KEY_BACK 1
/// Specifies the code of the 1 key.
#define KEY_1 2
/// Specifies the code of the 0 key.
#define KEY_0 11
/// Specifies the code of the mute key.
#define KEY_MUTE 60
/// Specifies the code of the channel down key.
#define KEY_CHANNEL_DOWN 61
/// Specifies the code of the channelup key.
#define KEY_CHANNEL_UP 62
/// Specifies the code of the volume up key.
#define KEY_VOLUME_UP 63
/// Specifies the code of the volume down key.
#define KEY_VOLUME_DOWN 64
/// Specifies the code of the info key.
#define KEY_INFO 358


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


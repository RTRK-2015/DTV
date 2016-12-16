/*! \file rc.h
    \brief Contains Remote Control API
*/
#ifndef RC_H
#define RC_H


#include "tdp_api.h"


/// \brief A callback that should take action on key press.
typedef void (*rc_key_callback)(int key_no);

/// \brief A function that starts the loop that waits for input events
/// from the remote control.
void rc_start_loop(const char *dev, rc_key_callback callback);
void rc_stop_loop();


#endif


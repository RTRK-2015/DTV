#ifndef MAPPING_H
#define MAPPING_H


// Unix includes
#include <pthread.h>
// Local includes
#include "common.h"


typedef void (*key_callback)(int key_code);

// Structure that contains the addresses of callback functions associated
// with different types of key events from the remote.
struct key_mapping
{
    key_callback
        key_1,
        key_2,
        key_3,
        key_4,
        key_5,
        key_6,
        key_7,
        key_8,
        key_9,
        key_0,
        key_star,
        key_hash,
        key_prog_up,
        key_prog_down,
        key_vol_up,
        key_vol_down,
        key_exit;
};


// This function starts a new thread which loops endlessly, waiting for the
// events from the remote and then dispatching to the associated callback.
// The thread is detached after starting.
void start_event_loop(const char *dev, struct key_mapping km);


#endif

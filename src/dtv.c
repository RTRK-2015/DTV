#define _POSIX_C_SOURCE 199309L

// Matching include
#include "dtv.h"
// C includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
// STD includes
#include <pthread.h>
#include <unistd.h>
// TDP includes
#include "tdp_api.h"
// Local includes
#include "common.h"
#include "parsing.h"
#include "structures.h"


static pthread_cond_t status_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t pat_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pat_mutex = PTHREAD_MUTEX_INITIALIZER;


static uint32_t
	player_handle,
	source_handle,
	filter_handle,
	video_handle,
	audio_handle;


struct
{
    uint32_t tuner : 1;
    uint32_t tuner_callback : 1;

    uint32_t player : 1;
    uint32_t source : 1;

    uint32_t demux_callback : 1;

    uint32_t video : 1;
	uint32_t audio : 1;
} static exit_flags;



static int32_t status_callback(t_LockStatus status)
{
	static size_t tries = 0;

    if (status == STATUS_LOCKED)
        pthread_cond_signal(&status_cond);
    else if (tries++ == 2)
        FAIL("%s\n", nameof(status_callback));
}


static struct pat my_pat;
static int32_t pat_callback(uint8_t *buffer)
{
    if (buffer[0] == 0x00)
    {
        my_pat = parse_pat(buffer);
        Demux_Free_Filter(player_handle, filter_handle);
    }

    pthread_cond_signal(&pat_cond);
}


void dtv_init(struct dtv_init_ch_info init_info)
{
    // Tuner
    if (Tuner_Init() == ERROR)
        FAIL("%s\n", nameof(Tuner_Init));
    exit_flags.tuner = 1;

    if (Tuner_Register_Status_Callback(status_callback) == ERROR)
        FAIL("%s\n", nameof(Tuner_Register_Status_Callback));
    exit_flags.tuner_callback = 1;

    if (Tuner_Lock_To_Frequency
    	( init_info.freq
    	, init_info.bandwidth
    	, init_info.module
    	) == ERROR)
        FAIL("%s\n", nameof(Tuner_Lock_To_Frequency));

    struct timespec ts_status;
    clock_gettime(CLOCK_REALTIME, &ts_status);
    ts_status.tv_sec += 5;

    if (pthread_cond_timedwait(&status_cond, &status_mutex, &ts_status) < 0)
        FAIL_STD("%s\n", nameof(pthread_cond_timedwait));

    // Player
    if (Player_Init(&player_handle) == ERROR)
        FAIL("%s\n", nameof(Player_Init));
    exit_flags.player = 1;

    if (Player_Source_Open(player_handle, &source_handle) == ERROR)
        FAIL("%s\n", nameof(Player_Source_Open));
    exit_flags.source = 1;

    // Demux
    if (Demux_Set_Filter(player_handle, 0x0000, 0x00, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));

    if (Demux_Register_Section_Filter_Callback(pat_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Register_Section_Filter_Callback));
    exit_flags.demux_callback = 1;

    struct timespec ts_pat;
    clock_gettime(CLOCK_REALTIME, &ts_pat);
    ts_pat.tv_sec += 5;
    
    if (pthread_cond_timedwait(&pat_cond, &pat_mutex, &ts_pat) < 0)
    	FAIL("%s\n", nameof(pthread_cond_timedwait));
}


void dtv_deinit()
{
    if (exit_flags.video)
        Player_Stream_Remove(player_handle, source_handle, video_handle);
    if (exit_flags.audio)
        Player_Stream_Remove(player_handle, source_handle, audio_handle);

    if (exit_flags.demux_callback)
        Demux_Unregister_Section_Filter_Callback(pat_callback);

    if (exit_flags.source)
        Player_Source_Close(player_handle, source_handle);

    if (exit_flags.player)
        Player_Deinit(player_handle);

    if (exit_flags.tuner_callback)
        Tuner_Unregister_Status_Callback(status_callback);
    if (exit_flags.tuner)
        Tuner_Deinit();
}



#define _POSIX_C_SOURCE 199309L

// Matching include
#include "dtv.h"
// C includes
#include <math.h>
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


#define LOG_DTV(fmt, ...) LOG("DTV", fmt, ##__VA_ARGS__)

static pthread_cond_t status_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t filter_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t filter_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint16_t sdt_ch_num;

static uint32_t
    player_handle,
    source_handle,
    filter_handle,
    video_handle,
    audio_handle;


/// \brief Structure that keeps information about what should be deinitialized
/// at the end of the program.
static struct
{
    uint32_t tuner : 1;
    uint32_t tuner_callback : 1;

    uint32_t player : 1;
    uint32_t source : 1;

    uint32_t demux_callback : 1;

    uint32_t video : 1;
    uint32_t audio : 1;
} exit_flags;



/// \brief A callback function that responds to frequency locking attempts.
static int32_t status_callback(t_LockStatus status)
{
    static size_t tries = 0;
    static const size_t max_tries = 2;

    LOG_DTV("Locking: %zu. try (of %zu)\n", tries + 1, max_tries + 1);

    if (status == STATUS_LOCKED)
        pthread_cond_signal(&status_cond);
    else if (tries++ == max_tries)
        FAIL("%s\n", nameof(status_callback));

    return 0;
}


static struct pat my_pat;
static struct pmt my_pmt;
static struct tm my_tm;
static struct sdt my_sdt;
/// \brief A callback function that calls parsing routines.
static int32_t filter_callback(uint8_t *buffer)
{
    LOG_DTV("In filter callback!\n");

    if (buffer[0] == 0x00)
    {
        LOG_DTV("Parsing pat...\n");
        my_pat = parse_pat(buffer);
        Demux_Free_Filter(player_handle, filter_handle);

        LOG_DTV("Found pat\n");
        for (size_t i = 0; i < my_pat.pmt_len; ++i)
        LOG_DTV("with pmt %d on pid %d\n", my_pat.pmts[i].ch_num, my_pat.pmts[i].pid);
    }
    else if (buffer[0] == 0x02)
    {
        LOG_DTV("Parsing pmt...\n");
        my_pmt = parse_pmt(buffer);
        Demux_Free_Filter(player_handle, filter_handle);

        LOG_DTV("Found pmt\n");
        LOG_DTV("audio_pid: %d\n", my_pmt.audio_pid);
        LOG_DTV("video_pid: %d\n", my_pmt.video_pid);
    }
    else if (buffer[0] == 0x73)
    {
        LOG_DTV("Parsing tot...\n");
        my_tm = parse_tot(buffer);
        Demux_Free_Filter(player_handle, filter_handle);
    }
    else if (buffer[0] == 0x42)
    {
        LOG_DTV("Parsing sdt...\n");
        my_sdt = parse_sdt(buffer, sdt_ch_num);
        Demux_Free_Filter(player_handle, filter_handle);
    }

    pthread_cond_signal(&filter_cond);

    return 0;
}


static uint16_t *channels = NULL;
/// \brief 
void dtv_get_channels()
{
    if (channels == NULL)
    {
        channels = (uint16_t *)malloc((my_pat.pmt_len) * sizeof(uint16_t));
        if (channels == NULL)
            FAIL_STD("%s\n", nameof(malloc));

        for (size_t i = 0; i < my_pat.pmt_len; ++i)
            channels[i] = my_pat.pmts[i].ch_num;
    }
}


void dtv_init(struct config_init_ch_info init_info)
{
    // Tuner
    LOG_DTV("Initializing tuner...\n");
    if (Tuner_Init() == ERROR)
        FAIL("%s\n", nameof(Tuner_Init));
    exit_flags.tuner = 1;

    LOG_DTV("Registering locking callback...\n");
    if (Tuner_Register_Status_Callback(status_callback) == ERROR)
        FAIL("%s\n", nameof(Tuner_Register_Status_Callback));
    exit_flags.tuner_callback = 1;

    LOG_DTV("Locking to frequency...\n");
    if (Tuner_Lock_To_Frequency
        ( init_info.freq
        , init_info.bandwidth
        , init_info.module
        ) == ERROR)
        FAIL("%s\n", nameof(Tuner_Lock_To_Frequency));

    struct timespec ts_status;
    clock_gettime(CLOCK_REALTIME, &ts_status);
    ts_status.tv_sec += 5;

    if (pthread_cond_timedwait(&status_cond, &status_mutex, &ts_status) > 0)
        FAIL_STD("%s\n", nameof(pthread_cond_timedwait));

    LOG_DTV("Initializing player...\n");
    // Player
    if (Player_Init(&player_handle) == ERROR)
        FAIL("%s\n", nameof(Player_Init));
    exit_flags.player = 1;

    LOG_DTV("Open source...\n");
    if (Player_Source_Open(player_handle, &source_handle) == ERROR)
        FAIL("%s\n", nameof(Player_Source_Open));
    exit_flags.source = 1;

    LOG_DTV("Setting PAT filter...\n");
    // Demux
    if (Demux_Set_Filter(player_handle, 0x0000, 0x00, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));

    LOG_DTV("Registering PAT callback...\n");
    if (Demux_Register_Section_Filter_Callback(filter_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Register_Section_Filter_Callback));
    exit_flags.demux_callback = 1;

    struct timespec ts_pat;
    clock_gettime(CLOCK_REALTIME, &ts_pat);
    ts_pat.tv_sec += 5;

    if (pthread_cond_timedwait(&filter_cond, &filter_mutex, &ts_pat) > 0)
        FAIL("%s\n", nameof(pthread_cond_timedwait));

    if (Player_Stream_Create
        ( player_handle
           , source_handle
        , init_info.vpid
        , init_info.vtype
        , &video_handle
        ) == ERROR)
        FAIL("%s\n", nameof(Player_Stream_Create));
    exit_flags.video = 1;
    
    if (Player_Stream_Create
        ( player_handle
        , source_handle
        , init_info.apid
        , init_info.atype
        , &audio_handle
        ) == ERROR)
        FAIL("%s\n", nameof(Player_Stream_Create));

    dtv_get_channels();
}


static void dtv_remove_stream()
{
    LOG_DTV("Video handle: %d\n", video_handle);
    if (video_handle != (uint32_t)-1)
    {
        if (Player_Stream_Remove
        ( player_handle
        , source_handle
        , video_handle
        ) == ERROR)
            FAIL("%s\n", nameof(Player_Stream_Remove));
    }
    exit_flags.video = 0;
    video_handle = -1;
    LOG_DTV("Removed video\n");

    LOG_DTV("Audio handle: %d\n", audio_handle);
    if (audio_handle != (uint32_t)-1)
    {
        if (Player_Stream_Remove
        ( player_handle
        , source_handle
        , audio_handle
        ) == ERROR)
            FAIL("%s\n", nameof(Player_Stream_Remove));
    }
    exit_flags.audio = 0;
    audio_handle = -1;
    LOG_DTV("Removed audio\n");
}


struct dtv_channel_info dtv_switch_channel(uint16_t ch_num)
{
    uint16_t pid = UINT16_C(0xFFFF);
    
    for (size_t i = 0; i < my_pat.pmt_len; ++i)
    {
        if (my_pat.pmts[i].ch_num == ch_num)
        {
            pid = my_pat.pmts[i].pid;
            break;
        }
    }

    LOG_DTV("pmt pid: %d\n", pid);
    if (pid == UINT16_C(0xFFFF))
    {
        struct dtv_channel_info channel_info =
        { .ch_num = ch_num
        , .vpid = -1
        , .apid = -1
        , .teletext = false
        };

        dtv_remove_stream();

        return channel_info;
    }

    if (Demux_Set_Filter(player_handle, pid, 0x02, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;

    if (pthread_cond_timedwait(&filter_cond, &filter_mutex, &ts) > 0)
        FAIL_STD("%s\n", nameof(pthread_cond_timedwait));
    LOG_DTV("Got pmt\n");

    dtv_remove_stream();

    if (my_pmt.video_pid != UINT16_C(0xFFFF))
    {
        if (Player_Stream_Create
            ( player_handle
            , source_handle
            , my_pmt.video_pid
            , VIDEO_TYPE_MPEG2
            , &video_handle
            ) == ERROR)
            FAIL("%s\n", nameof(Player_Stream_Create));
        LOG_DTV("Started video with pid: %d\n", my_pmt.video_pid);
        exit_flags.video = 1;
    }

    if (my_pmt.audio_pid != UINT16_C(0xFFFF))
    {
        if (Player_Stream_Create
            ( player_handle
            , source_handle
            , my_pmt.audio_pid
            , AUDIO_TYPE_MPEG_AUDIO
            , &audio_handle
            ) == ERROR)
            FAIL("%s\n", nameof(Player_Stream_Create));
        LOG_DTV("Started audio with pid: %d\n", my_pmt.audio_pid);
        exit_flags.audio = 1;
    }

    struct dtv_channel_info channel_info =
    { .ch_num = ch_num
    , .vpid = my_pmt.video_pid
    , .apid = my_pmt.audio_pid
    , .teletext = my_pmt.teletext
    };

    return channel_info;
}


t_Error dtv_set_volume(uint8_t vol)
{
    LOG_DTV("Setting volume to %d\n", vol);
    
    if (vol > 10)
        return ERROR;

    const uint32_t adjusted_volume = (uint32_t)vol * 200000000L;

    return Player_Volume_Set(player_handle, adjusted_volume);
}


struct tm dtv_get_time()
{
    if (Demux_Set_Filter(player_handle, 0x0014, 0x73, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 30;

    if (pthread_cond_timedwait(&filter_cond, &filter_mutex, &ts) > 0)
        FAIL_STD("%s\n", nameof(pthread_cond_wait));

    return my_tm;
}


struct sdt dtv_get_info(uint16_t ch_num)
{
    sdt_ch_num = ch_num;

    if (Demux_Set_Filter(player_handle, 0x0011, 0x42, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;

    LOG_DTV("waiting info\n");
    pthread_cond_timedwait(&filter_cond, &filter_mutex, &ts);


    return my_sdt;
}


void dtv_deinit()
{
    LOG_DTV("DTV Deinit\n");

    if (channels != NULL)
    {
        LOG_DTV("Freeing channels\n");
        free(channels);
    }

    if (exit_flags.video)
    {
        LOG_DTV("Removing video\n");
        Player_Stream_Remove(player_handle, source_handle, video_handle);
    }
    if (exit_flags.audio)
    {
        LOG_DTV("Removing audio\n");
        Player_Stream_Remove(player_handle, source_handle, audio_handle);
    }

    if (exit_flags.demux_callback)
    {
        LOG_DTV("Unregistering filter callback\n");
        Demux_Unregister_Section_Filter_Callback(filter_callback);
    }

    if (exit_flags.source)
    {
        LOG_DTV("Closing source\n");
        Player_Source_Close(player_handle, source_handle);
    }

    if (exit_flags.player)
    {
        LOG_DTV("Deinitializing player\n");
        Player_Deinit(player_handle);
    }

    if (exit_flags.tuner_callback)
    {
        LOG_DTV("Unregistering tuner callback\n");
        Tuner_Unregister_Status_Callback(status_callback);
    }
    if (exit_flags.tuner)
    {
        LOG_DTV("Deinitializing tuner\n");
        Tuner_Deinit();
    }
}



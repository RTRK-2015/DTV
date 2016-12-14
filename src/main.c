#define _POSIX_C_SOURCE 199309L

// C includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
// STD includes
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
// TDP includes
#include "tdp_api.h"
// Local includes
#include "common.h"
#include "mapping.h"
#include "parsing.h"
#include "structures.h"


static pthread_cond_t status_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_mutex_t filter_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t pmts_processing_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pmts_processing_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint32_t
    p_h[1],
    s_h0[2],
    f_h0[8],
    str_h0_0[3],
    str_h0_1[3];


static void handle_signal(int no)
{
    exit(EXIT_SUCCESS);
}

struct
{
    uint32_t tuner : 1;
    uint32_t tuner_callback : 1;

    uint32_t player0 : 1;
    uint32_t source0_0 : 1;
    uint32_t source0_1 : 1;

    uint32_t demux_callback : 1;
    uint32_t filter0_0 : 1;
    uint32_t filter0_1 : 1;
    uint32_t filter0_2 : 1;
    uint32_t filter0_3 : 1;
    uint32_t filter0_4 : 1;
    uint32_t filter0_5 : 1;
    uint32_t filter0_6 : 1;
    uint32_t filter0_7 : 1;

    uint32_t stream0_0_0 : 1;
    uint32_t stream0_0_1 : 1;
    uint32_t stream0_0_2 : 1;

    uint32_t stream0_1_0 : 1;
    uint32_t stream0_1_1 : 1;
    uint32_t stream0_1_2 : 1;
} static exit_flags;


static size_t tries = 0;
static int32_t status_callback(t_LockStatus status)
{
    if (status == STATUS_LOCKED)
        pthread_cond_signal(&status_cond);
    else if (tries++ == 2)
        FAIL("%s\n", nameof(status_callback));
}


static struct pat my_pat;
static size_t selected_pmt = 0;

static int32_t filter_callback(uint8_t *buffer)
{
    static int current_pmt = 0;

    pthread_mutex_lock(&filter_mutex);

    if (buffer[0] == 0x00)
    {
        my_pat = parse_pat(buffer);
        Demux_Free_Filter(p_h[0], f_h0[0]);

        Demux_Set_Filter(p_h[0], my_pat.pmts[current_pmt].pid, 0x02, &f_h0[0]);
    }
    else if (buffer[0] == 0x02)
    {
        parse_pmt(buffer, &my_pat);
        ++current_pmt;

        Demux_Free_Filter(p_h[0], f_h0[0]);
        if (current_pmt < my_pat.pmt_len)
        {
            Demux_Set_Filter(p_h[0], my_pat.pmts[current_pmt].pid, 0x02, &f_h0[0]);
        }
        else
        {
            exit_flags.filter0_0 = 0;
            pthread_cond_signal(&pmts_processing_cond);
        }
    }

    pthread_mutex_unlock(&filter_mutex);
}



static void destructor()
{
    if (exit_flags.stream0_1_2)
        Player_Stream_Remove(p_h[0], s_h0[1], str_h0_1[2]);
    if (exit_flags.stream0_1_1)
        Player_Stream_Remove(p_h[0], s_h0[1], str_h0_1[1]);
    if (exit_flags.stream0_1_0)
        Player_Stream_Remove(p_h[0], s_h0[1], str_h0_1[0]);

    if (exit_flags.stream0_0_2)
        Player_Stream_Remove(p_h[0], s_h0[0], str_h0_0[2]);
    if (exit_flags.stream0_0_1)
        Player_Stream_Remove(p_h[0], s_h0[0], str_h0_0[1]);
    if (exit_flags.stream0_0_0)
        Player_Stream_Remove(p_h[0], s_h0[0], str_h0_0[0]);

    if (exit_flags.filter0_7)
        Demux_Free_Filter(p_h[0], f_h0[7]);
    if (exit_flags.filter0_6)
        Demux_Free_Filter(p_h[0], f_h0[6]);
    if (exit_flags.filter0_5)
        Demux_Free_Filter(p_h[0], f_h0[5]);
    if (exit_flags.filter0_4)
        Demux_Free_Filter(p_h[0], f_h0[4]);
    if (exit_flags.filter0_3)
        Demux_Free_Filter(p_h[0], f_h0[3]);
    if (exit_flags.filter0_2)
        Demux_Free_Filter(p_h[0], f_h0[2]);
    if (exit_flags.filter0_1)
        Demux_Free_Filter(p_h[0], f_h0[1]);
    if (exit_flags.filter0_0)
        Demux_Free_Filter(p_h[0], f_h0[0]);
    if (exit_flags.demux_callback)
        Demux_Unregister_Section_Filter_Callback(filter_callback);

    if (exit_flags.source0_1)
        Player_Source_Close(p_h[0], s_h0[1]);
    if (exit_flags.source0_0)
        Player_Source_Close(p_h[0], s_h0[0]);

    if (exit_flags.player0)
        Player_Deinit(p_h[0]);

    if (exit_flags.tuner_callback)
        Tuner_Unregister_Status_Callback(status_callback);
    if (exit_flags.tuner)
        Tuner_Deinit();
}

void react_channel_up(int _)
{
    if (selected_pmt == my_pat.pmt_len - 1)
        selected_pmt = 0;
    else
        ++selected_pmt;

    printf("%d\n", selected_pmt);
    Player_Stream_Remove(p_h[0], s_h0[0], str_h0_0[0]);
    Player_Stream_Create(p_h[0], s_h0[0], my_pat.pmts[selected_pmt].video_pids[0],
            VIDEO_TYPE_MPEG2, &str_h0_0[0]);
}

void react_channel_down(int _)
{
    if (selected_pmt == 0)
        selected_pmt = my_pat.pmt_len - 1;
    else
        --selected_pmt;

    printf("%d\n", selected_pmt);
    Player_Stream_Remove(p_h[0], s_h0[0], str_h0_0[0]);
    Player_Stream_Create(p_h[0], s_h0[0], my_pat.pmts[selected_pmt].video_pids[0],
            VIDEO_TYPE_MPEG2, &str_h0_0[0]);
}

struct key_mapping km =
{ .key_prog_up = react_channel_up
, .key_prog_down = react_channel_down
};


int main()
{
    const uint32_t FREQUENCY = 754000000UL;
    const uint32_t BANDWIDTH = 8;

    signal(SIGINT, handle_signal);
    atexit(destructor);

    // Tuner
    if (Tuner_Init() == ERROR)
        FAIL("%s\n", nameof(Tuner_Init));
    exit_flags.tuner = 1;

    if (Tuner_Register_Status_Callback(status_callback) == ERROR)
        FAIL("%s\n", nameof(Tuner_Register_Status_Callback));
    exit_flags.tuner_callback = 1;

    if (Tuner_Lock_To_Frequency(FREQUENCY, BANDWIDTH, DVB_T) == ERROR)
        FAIL("%s\n", nameof(Tuner_Lock_To_Frequency));

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;

    if (pthread_cond_timedwait(&status_cond, &status_mutex, &ts) < 0)
        FAIL_STD(nameof(pthread_cond_timedwait));

    // Player
    if (Player_Init(&p_h[0]) == ERROR)
        FAIL("%s\n", nameof(Player_Init));
    exit_flags.player0 = 1;

    if (Player_Source_Open(p_h[0], &s_h0[0]) == ERROR)
        FAIL("%s\n", nameof(Player_Source_Open));
    exit_flags.source0_0 = 1;

    // Demux
    if (Demux_Set_Filter(p_h[0], 0x0000, 0x00, &f_h0[0]) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));
    exit_flags.filter0_0 = 1;

    if (Demux_Register_Section_Filter_Callback(filter_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Register_Section_Filter_Callback));
    exit_flags.demux_callback = 1;

    pthread_cond_wait(&pmts_processing_cond, &pmts_processing_mutex);

    start_event_loop("/dev/input/event0", km);

    printf("tsi: %d, pmt_len: %d\n", my_pat.tsi, my_pat.pmt_len);
    for (size_t i = 0; i < my_pat.pmt_len; ++i)
    {
        struct pmt *my_pmt = &my_pat.pmts[i];

        printf("pid: %d, pr_num: %d\n", my_pmt->pid, my_pmt->pr_num);
        printf("video_len: %d, audio_len: %d, data_pids: %d\n",
            my_pmt->video_len, my_pmt->audio_len, my_pmt->data_len);

        for (size_t j = 0; j < my_pmt->video_len; ++j)
        {
            printf("video pid: %d\n", my_pmt->video_pids[j]);
        }
        for (size_t j = 0; j < my_pmt->audio_len; ++j)
        {
            printf("audio pid: %d\n", my_pmt->audio_pids[j]);
        }
        for (size_t j = 0; j < my_pmt->data_len; ++j)
        {
            printf("data pid: %d\n", my_pmt->data_pids[j]);
        }
    }

    printf("pid: %d\n", my_pat.pmts[selected_pmt].video_pids[0]);
    if (Player_Stream_Create(p_h[0], s_h0[0], my_pat.pmts[selected_pmt].video_pids[0],
            VIDEO_TYPE_MPEG2, &str_h0_0[0]) == ERROR)
        FAIL("%s\n", nameof(Player_Stream_Create));

    while (1);
}


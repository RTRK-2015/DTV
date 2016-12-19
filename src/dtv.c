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


static pthread_cond_t status_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t status_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t pat_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pat_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t pmt_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t pmt_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t tot_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t tot_mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t sdt_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t sdt_mutex = PTHREAD_MUTEX_INITIALIZER;

static Demux_Section_Filter_Callback current_callback = NULL;


static uint16_t sdt_ch_num;

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
    const static size_t max_tries = 2;

    printf("Locking: %zu. try (of %zu)\n", tries + 1, max_tries) + 1;

    if (status == STATUS_LOCKED)
        pthread_cond_signal(&status_cond);
    else if (tries++ == max_tries)
        FAIL("%s\n", nameof(status_callback));
}


static struct pat my_pat;
static int32_t pat_callback(uint8_t *buffer)
{
    printf("Parsing pat...\n");
    my_pat = parse_pat(buffer);
    Demux_Free_Filter(player_handle, filter_handle);

    printf("Found pat\n");
    for (size_t i = 0; i < my_pat.pmt_len; ++i)
	printf("with pmt %d on pid %d\n", my_pat.pmts[i].ch_num, my_pat.pmts[i].pid);

    pthread_cond_signal(&pat_cond);
}

static struct pmt my_pmt;
static int32_t pmt_callback(uint8_t *buffer)
{
    printf("Parsing pmt...\n");
    my_pmt = parse_pmt(buffer);
    Demux_Free_Filter(player_handle, filter_handle);
	
    printf("Found pmt\n");
    printf("audio_pid: %d\n", my_pmt.audio_pid);
    printf("video_pid: %d\n", my_pmt.video_pid);

    pthread_cond_signal(&pmt_cond);
}


static struct tm my_tm;
static int32_t tot_callback(uint8_t *buffer)
{
    printf("Parsing tot...\n");
    my_tm = parse_tot(buffer);
    Demux_Free_Filter(player_handle, filter_handle);

    pthread_cond_signal(&tot_cond);
}


static struct sdt my_sdt;
static int32_t sdt_callback(uint8_t *buffer)
{
    printf("Parsing sdt...\n");
    my_sdt = parse_sdt(buffer, sdt_ch_num);
    Demux_Free_Filter(player_handle, filter_handle);

    pthread_cond_signal(&sdt_cond);
}


static uint16_t *channels = NULL;
const void dtv_get_channels()
{
    if (channels == NULL)
    {
        channels = (uint16_t *)malloc((my_pat.pmt_len + 1) * sizeof(uint16_t));
        if (channels == NULL)
            FAIL_STD("%s\n", nameof(malloc));

        for (size_t i = 0; i < my_pat.pmt_len; ++i)
            channels[i] = my_pat.pmts[i].ch_num;
        channels[my_pat.pmt_len] = END_OF_CHANNELS;
    }
}


void dtv_init(struct config_init_ch_info init_info)
{
    // Tuner
    printf("Initializing tuner...\n");
    if (Tuner_Init() == ERROR)
        FAIL("%s\n", nameof(Tuner_Init));
    exit_flags.tuner = 1;

    printf("Registering locking callback...\n");
    if (Tuner_Register_Status_Callback(status_callback) == ERROR)
        FAIL("%s\n", nameof(Tuner_Register_Status_Callback));
    exit_flags.tuner_callback = 1;

    printf("Locking to frequency...\n");
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

    printf("Initializing player...\n");
    // Player
    if (Player_Init(&player_handle) == ERROR)
        FAIL("%s\n", nameof(Player_Init));
    exit_flags.player = 1;

    printf("Open source...\n");
    if (Player_Source_Open(player_handle, &source_handle) == ERROR)
        FAIL("%s\n", nameof(Player_Source_Open));
    exit_flags.source = 1;

    printf("Setting PAT filter...\n");
    // Demux
    if (Demux_Set_Filter(player_handle, 0x0000, 0x00, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));

    printf("Registering PAT callback...\n");
    if (Demux_Register_Section_Filter_Callback(pat_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Register_Section_Filter_Callback));
    current_callback = pat_callback;
    exit_flags.demux_callback = 1;

    struct timespec ts_pat;
    clock_gettime(CLOCK_REALTIME, &ts_pat);
    ts_pat.tv_sec += 5;

    if (pthread_cond_timedwait(&pat_cond, &pat_mutex, &ts_pat) < 0)
        FAIL("%s\n", nameof(pthread_cond_timedwait));

    if (Demux_Unregister_Section_Filter_Callback(pat_callback) == ERROR)
    	FAIL("%s\n", nameof(Demux_Unregister_Section_Filter_Callback));
    current_callback = NULL;
    exit_flags.demux_callback = 0;
    
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
	
	printf("pmt pid: %d\n", pid);
	if (pid == UINT16_C(0xFFFF))
        {
            struct dtv_channel_info channel_info =
            { .ch_num = ch_num
            , .vpid = -1
            , .apid = -1
            };

            return channel_info;
        }
		
	if (Demux_Set_Filter(player_handle, pid, 0x02, &filter_handle) == ERROR)
		FAIL("%s\n", nameof(Demux_Set_Filter));
		
	if (Demux_Register_Section_Filter_Callback(pmt_callback) == ERROR)
		FAIL("%s\n", nameof(Demux_Register_Section_Filter_Callback));
	current_callback = pmt_callback;
	exit_flags.demux_callback = 1;
	
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += 5;
	
	if (pthread_cond_timedwait(&pmt_cond, &pmt_mutex, &ts) < 0)
		FAIL_STD("%s\n", nameof(pthread_cond_timedwait));
	printf("Got pmt\n");
	
	if (Demux_Unregister_Section_Filter_Callback(pmt_callback) == ERROR)
		FAIL("%s\n", nameof(Demux_Unregister_Section_Filter_Callback));
	exit_flags.demux_callback = 0;

	if (video_handle != -1 &&
		Player_Stream_Remove(player_handle, source_handle, video_handle) == ERROR)
		FAIL("%s\n", nameof(Player_Stream_Remove));
	exit_flags.video = 0;
	video_handle = -1;
	printf("Removed video\n");
	
	if (audio_handle != - 1 &&
		Player_Stream_Remove(player_handle, source_handle, audio_handle) == ERROR)
		FAIL("%s\n", nameof(Player_Stream_Remove));
	exit_flags.audio = 0;
	audio_handle = -1;
	printf("Removed audio\n");
	
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
		printf("Started video with pid: %d\n", my_pmt.video_pid);
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
		printf("Started audio with pid: %d\n", my_pmt.audio_pid);
		exit_flags.audio = 1;
	}

        struct dtv_channel_info channel_info =
        { .ch_num = ch_num
        , .vpid = my_pmt.video_pid
        , .apid = my_pmt.audio_pid
        };

        return channel_info;
}


t_Error dtv_set_volume(uint8_t vol)
{
	printf("Setting volume to %d\n", vol);
	
	if (vol > 10)
		return ERROR;

	return Player_Volume_Set(player_handle, vol);
}


struct tm dtv_get_time()
{
    if (Demux_Set_Filter(player_handle, 0x0014, 0x73, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_Filter));

    if (Demux_Register_Section_Filter_Callback(tot_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Register_Section_Filter_Callback));
    current_callback = tot_callback;
    exit_flags.demux_callback = 1;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;

    if (pthread_cond_timedwait(&tot_cond, &tot_mutex, &ts) < 0)
        FAIL_STD("%s\n", nameof(pthread_cond_timedwait));

    if (Demux_Unregister_Section_Filter_Callback(tot_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Unregister_Section_Filter_Callback));
    current_callback = NULL;
    exit_flags.demux_callback = 0;

    return my_tm;
}


struct sdt dtv_get_info(uint16_t ch_num)
{
    sdt_ch_num = ch_num;

    if (Demux_Set_Filter(player_handle, 0x0011, 0x42, &filter_handle) == ERROR)
        FAIL("%s\n", nameof(Demux_Set_FilteR));

    if (Demux_Register_Section_Filter_Callback(sdt_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Register_Section_Filter_Callback));
    current_callback = sdt_callback;
    exit_flags.demux_callback = 1;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;

    if (pthread_cond_timedwait(&sdt_cond, &sdt_mutex, &ts) < 0)
        FAIL_STD("%s\n", nameof(pthread_cond_timedwait));

    if (Demux_Unregister_Section_Filter_Callback(sdt_callback) == ERROR)
        FAIL("%s\n", nameof(Demux_Unregister_Section_Filter_Callback));
    current_callback = NULL;
    exit_flags.demux_callback = 0;
    
    return my_sdt;
}


void dtv_deinit()
{
	if (channels != NULL)
		free(channels);

    if (exit_flags.video)
        Player_Stream_Remove(player_handle, source_handle, video_handle);
    if (exit_flags.audio)
        Player_Stream_Remove(player_handle, source_handle, audio_handle);

    if (exit_flags.demux_callback && current_callback != NULL)
        Demux_Unregister_Section_Filter_Callback(current_callback);

    if (exit_flags.source)
        Player_Source_Close(player_handle, source_handle);

    if (exit_flags.player)
        Player_Deinit(player_handle);

    if (exit_flags.tuner_callback)
        Tuner_Unregister_Status_Callback(status_callback);
    if (exit_flags.tuner)
        Tuner_Deinit();
}



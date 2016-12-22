#define _POSIX_C_SOURCE 200809L

#include <directfb.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "drawing.h"
#include "graphics.h"

/// \brief Error codes to be used for internal graphics functions
typedef enum g_error
{
    G_ERROR = -1,
    G_NO_ERROR
}g_error;

#define LOG_GRAPHICS(fmt, ...) LOG("Graphics", fmt, ##__VA_ARGS__)

#define DRAWCHECK(err) \
{ \
    if (err != EXIT_SUCCESS) \
    { \
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__, __func__); \
        release(); \
        return G_ERROR; \
     } \
}

static struct graphics_flags
{
    bool info;
    bool volume;
    bool blackscreen;
    bool no_channel;
    bool audio_only;
    bool ch_num;
    bool time;
    bool mute;
} gf = { 0 };

static bool end = false;

static struct draw_interface draw_interface =
{
    .surface = NULL,
    .dfb_interface = NULL,
    .screen_width = 0,
    .screen_height = 0,
    .vol_surfaces = { NULL },
    .font_interface = NULL
};

static timer_t timer_info, timer_time, timer_num, timer_vol;
static const struct itimerspec reset = { 0 };
static const struct itimerspec sec3 =
{
    .it_value.tv_sec = 3
};
static const struct itimerspec sec1 =
{
    .it_value.tv_sec = 1
};

static void reset_info(union sigval s)
{
    LOG_GRAPHICS("Reset info\n");
    timer_settime(timer_info, 0, &reset, NULL);
    gf.info = false;
}
static struct graphics_channel_info to_draw_info;
void graphics_show_channel_info(struct graphics_channel_info info)
{
    to_draw_info = info;
    LOG_GRAPHICS("ch_num: %d, vpid: %d, apid: %d, st: %d, name: %s\n",
            info.ch_num, info.vpid, info.apid, info.sdt.st, info.sdt.name);
    if (to_draw_info.vpid == (uint16_t)-1 && to_draw_info.apid == (uint16_t)-1)
    {
        LOG_GRAPHICS("No channel\n");
        gf.audio_only = false;
        gf.no_channel = true;
    }
    else if (to_draw_info.vpid == (uint16_t)-1)
    {
        LOG_GRAPHICS("Audio only\n");
        gf.no_channel = false;
        gf.audio_only = true;
    }
    else
    {
        LOG_GRAPHICS("Normal\n");
        gf.no_channel = false;
        gf.audio_only = false;
    }
    LOG_GRAPHICS("Set info timer\n");
    timer_settime(timer_info, 0, &sec3, NULL);
    gf.info = true;
}

static void reset_time(union sigval s)
{
    LOG_GRAPHICS("Reset time\n");
    timer_settime(timer_time, 0, &reset, NULL);
    gf.time = false;
}
static struct tm to_draw_tm;
void graphics_show_time(struct tm tm)
{
    timer_settime(timer_time, 0, &sec3, NULL);
    to_draw_tm = tm;
    gf.time = true;
}

static void reset_vol(union sigval s)
{
    LOG_GRAPHICS("Reset vol\n");
    timer_settime(timer_vol, 0, &reset, NULL);
    gf.volume = false;
}
static uint8_t to_draw_vol;
void graphics_show_volume(uint8_t vol)
{
    timer_settime(timer_vol, 0, &sec3, NULL);
    to_draw_vol = vol;
    gf.volume = true;
}


void graphics_show_mute()
{
    gf.mute = true;
}

void graphics_hide_mute()
{
    gf.mute = false;
}


static void reset_ch_num(union sigval s)
{
    LOG_GRAPHICS("Reset ch_num\n");
    timer_settime(timer_num, 0, &reset, NULL);
    gf.ch_num = false;
}
static uint16_t to_draw_ch_num;
void graphics_show_channel_number(uint16_t ch_num)
{
    timer_settime(timer_num, 0, &sec1, NULL);
    to_draw_ch_num = ch_num;
    gf.ch_num = true;
}

void graphics_blackscreen()
{
    gf.blackscreen = true;
}

void graphics_clear()
{
    memset(&gf, 0, sizeof(gf));
}

/// \brief Function called on end of program to deinitialize drawing interface
static void release()
{
    draw_deinit(&draw_interface);
}

/// \brief Function that continuously refreshes graphics display
/// according to the current state of graphics_flags
g_error graphics_render(int *argc, char ***argv)
{
    LOG_GRAPHICS("Try init draw_interface with arguements: %d, %s\n",
            *argc, *argv[0]);

    if (draw_init(&draw_interface, argc, argv) != EXIT_SUCCESS)
        FAIL("%s\n", nameof(draw_init));

    LOG_GRAPHICS("Successfully init draw_i, screen width: %d, screen height: %d\n", 
            draw_interface.screen_width, draw_interface.screen_height);

    do
    {
        struct timespec tp;
        clock_gettime(CLOCK_REALTIME, &tp);

        DRAWCHECK(draw_clear(&draw_interface));

        if (gf.no_channel)
        {
            DRAWCHECK(draw_blackscreen(&draw_interface));
            DRAWCHECK(draw_no_channel(&draw_interface));
        }

        if (gf.audio_only)
        {
            DRAWCHECK(draw_blackscreen(&draw_interface));
            DRAWCHECK(draw_audio_only(&draw_interface));
        }

        if (gf.info)
        {
            DRAWCHECK(draw_channel_info(&draw_interface, to_draw_info));
        }

        if (gf.mute)
        {
            DRAWCHECK(draw_volume(&draw_interface, -1));
        }
        else if (gf.volume)
        {
            DRAWCHECK(draw_volume(&draw_interface, to_draw_vol));
        }

        if (gf.time)
        {
            DRAWCHECK(draw_time(&draw_interface, to_draw_tm));
        }

        if (gf.ch_num)
        {
            DRAWCHECK(draw_channel_number(&draw_interface, to_draw_ch_num));
        }

        DRAWCHECK(draw_refresh(&draw_interface));

        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);

        if (ts.tv_sec - tp.tv_sec == 0 && ts.tv_nsec - tp.tv_nsec < 16000000)
        {
            struct timespec tr;
            tr.tv_sec = ts.tv_sec;
            tr.tv_nsec = 160000000 - (ts.tv_nsec - tp.tv_nsec);
            nanosleep(&tr, NULL);
        }

    } while (!end);

    LOG_GRAPHICS("Finish render loop\n");
    release();

    return G_NO_ERROR;
}


struct args
{
    int *argcx;
    char ***argvx;
};

static pthread_mutex_t args_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t args_cond = PTHREAD_COND_INITIALIZER;

static void* graphics_render_loop(void *args)
{
    struct args a = *(struct args *)args;
    pthread_cond_signal(&args_cond);

    if (graphics_render(a.argcx, a.argvx) == G_ERROR)
        FAIL("%s", nameof(graphics_render));

    return NULL;
}

static pthread_t th;

void graphics_start_render(int *argc, char ***argv)
{
    struct sigevent se_info =
    {
        .sigev_notify_function = reset_info,
        .sigev_notify = SIGEV_THREAD
    };
    timer_create(CLOCK_REALTIME, &se_info, &timer_info);

    struct sigevent se_time =
    {
        .sigev_notify_function = reset_time,
        .sigev_notify = SIGEV_THREAD
    };
    timer_create(CLOCK_REALTIME, &se_time, &timer_time);

    struct sigevent se_vol =
    {
        .sigev_notify_function = reset_vol,
        .sigev_notify = SIGEV_THREAD
    };
    timer_create(CLOCK_REALTIME, &se_vol, &timer_vol);

    struct sigevent se_num =
    {
        .sigev_notify_function = reset_ch_num,
        .sigev_notify = SIGEV_THREAD
    };
    timer_create(CLOCK_REALTIME, &se_num, &timer_num);
    
        
    struct args a =
    {
        .argcx = argc,
        .argvx = argv
    };

    LOG_GRAPHICS("Creating render thread\n");
    if (pthread_create(&th, NULL, graphics_render_loop, (void *)&a) != 0)
        FAIL_STD("%s\n", nameof(pthread_create));

    pthread_cond_wait(&args_cond, &args_mutex);

}

void graphics_stop()
{
    LOG_GRAPHICS("Stopping graphics\n");
    end = true;
    pthread_join(th, NULL);
}


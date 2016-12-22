/*! \file graphics.c
    \brief Contains implementation for graphics interface.
*/
#define _POSIX_C_SOURCE 200809L
// C includes
#include <stdint.h>
#include <stdio.h>
#include <time.h>
// Unix includes
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
// Other includes
#include <directfb.h>
// Local includes
#include "common.h"
#include "drawing.h"
#include "graphics.h"


/// \brief Error codes to be used for internal graphics functions
typedef enum g_error
{
    G_ERROR = -1, ///< Specifies that a graphics error has occurred.
    G_NO_ERROR ///< Specifies that a graphics_error has *not* occurred.
};
static enum g_error g_error;

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


/// \brief A struct that keeps the state of what should be displayed.
struct graphics_flags
{
    bool info; ///< Specifies whether the info panel should be displayed.
    bool volume; ///< Specifies whether the volume panel should be displayed.
    bool blackscreen;
    ///< Specifies whether the screen should be filled with black.
    bool no_channel;
    ///< Specifies whether the "NO CHANNEL" message should be displayed.
    bool audio_only;
    ///< Specifies whether the "AUDIO ONLY" message should be displayed.
    bool ch_num;
    ///< Specifies whether the top left channel number should be displayed.
    bool time;
    ///< Specifies whether the time should be displayed.
    bool init;
    ///< Specifies whether the "INITIALIZING" message should be displayed.
    bool mute;
    ///< \brief Specifies whether the mute symbol should be displayed.
    /// Takes precedence over \ref volume.
};
static struct graphics_flags gf = { 0 };

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

static timer_t timer_info, timer_time, timer_num, timer_vol, timer_black;
static const struct itimerspec reset = { 0 };
static const struct itimerspec sec4 = { .it_value.tv_sec = 4 };
static const struct itimerspec sec3 = { .it_value.tv_sec = 3 };
static const struct itimerspec sec1 = { .it_value.tv_sec = 1 };

/// \brief Resets the info panel timer.
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

/// \brief Resets the time panel timer.
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

/// \brief Resets the volume panel timer.
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


void graphics_show_init()
{
    gf.init = true;
}

void graphics_hide_init()
{
    gf.init = false;
}


void graphics_show_mute()
{
    gf.mute = true;
}

void graphics_hide_mute()
{
    gf.mute = false;
}


/// \brief Resets the channel number panel timer.
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

/// \brief Resets the black screen timer.
static void reset_black(union sigval s)
{
    timer_settime(timer_black, 0, &reset, NULL);
    gf.blackscreen = false;
}

void graphics_blackscreen()
{
    gf.blackscreen = true;
    timer_settime(timer_black, 0, &sec4, NULL);
}

void graphics_clear()
{
    bool mute = gf.mute;

    memset(&gf, 0, sizeof(gf));

    gf.mute = mute;
}

/// \brief Function called on end of program to deinitialize drawing interface.
static void release()
{
    draw_deinit(&draw_interface);
}

/// \brief Function that continuously refreshes graphics display
/// according to the current state of graphics_flags.
enum g_error graphics_render(int *argc, char ***argv)
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

        if (gf.blackscreen)
        {
            DRAWCHECK(draw_blackscreen(&draw_interface));
        }

        if (gf.init)
        {
            DRAWCHECK(draw_blackscreen(&draw_interface));
            DRAWCHECK(draw_init_message(&draw_interface));
        }

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


/// \brief A shim structure to pass main arguments to DirectFB.
struct graphics_args
{
    int *argcx;
    char ***argvx;
};

static pthread_mutex_t args_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t args_cond = PTHREAD_COND_INITIALIZER;

/// \brief Function that starts the rendering process.
static void* graphics_render_loop(void *args)
{
    struct graphics_args a = *(struct graphics_args *)args;
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

    struct sigevent se_black =
    {
        .sigev_notify_function = reset_black,
        .sigev_notify = SIGEV_THREAD
    };
    timer_create(CLOCK_REALTIME, &se_black, &timer_black);
    
        
    struct graphics_args a =
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


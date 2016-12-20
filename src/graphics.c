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

struct graphics_flags
{
    bool info;
    bool volume;
    bool blackscreen;
    bool no_channel;
    bool audio_only;
    bool ch_num;
    bool time;
};

bool end = false;
static struct graphics_flags gf = { .info = false, .volume = false };

struct draw_interface draw_interface =
{
    .surface = NULL,
    .dfb_interface = NULL,
    .screen_width = 0,
    .screen_height = 0,
    .vol_surfaces = { NULL },
    .font_interface = NULL
};

struct graphics_channel_info to_draw_info;
void graphics_show_channel_info(struct graphics_channel_info info)
{
    to_draw_info = info;
    if (to_draw_info.vpid < 0 && to_draw_info.apid < 0)
        gf.no_channel = true;
    else if (to_draw_info.vpid < 0 && to_draw_info.apid > 0)
        gf.audio_only = true;
    else
        gf.info = true;
}

struct tm to_draw_tm;
void graphics_show_time(struct tm tm)
{
    to_draw_tm = tm;
    gf.time = true;
}

uint8_t to_draw_vol;
void graphics_show_volume(uint8_t vol)
{
    to_draw_vol = vol;
    gf.volume = true;
}

uint16_t to_draw_ch_num;
void graphics_show_channel_number(uint16_t ch_num)
{
    to_draw_ch_num = ch_num;
    gf.ch_num = true;
}

void graphics_blackscreen()
{
    gf.blackscreen = true;
}

void graphics_clear()
{
    gf.info = false;
    gf.volume = false;
    gf.blackscreen = false;
    gf.no_channel = false;
    gf.audio_only = false;
    gf.ch_num = false;
    gf.time = false;
}

void graphics_stop()
{
    end = true;
}


void release()
{
    draw_deinit(&draw_interface);
}

t_Error graphics_render(int *argc, char ***argv)
{
    printf("\nTry init draw_interface with arguements: %d, %s\n", *argc, *argv[0]);
    if (draw_init(&draw_interface, argc, argv) < 0)
        FAIL("%s\n", nameof(draw_init));

    printf("Successfully init draw_i, screen width: %d, screen height: %d\n", 
            draw_interface.screen_width, draw_interface.screen_height);

    do
    {
        struct timespec tp;
        clock_gettime(CLOCK_REALTIME, &tp);

        if (draw_clear(&draw_interface) < 0)
        {
            release();
            return ERROR;
        }

        if (gf.info)
        {
            if (draw_channel_info(&draw_interface, to_draw_info) < 0)
            {
                release();
                return ERROR;
            }
        }

        if (gf.volume)
        {
            if (draw_volume(&draw_interface, to_draw_vol) < 0)
            {
                release();
                return ERROR;
            }
        }

        if (gf.time)
        {
            if (draw_time(&draw_interface, to_draw_tm) < 0)
            {
                release();
                return ERROR;
            }
        }

        if (draw_refresh(&draw_interface) < 0)
        {
            release();
            return ERROR;
        }

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

    release();

    return NO_ERROR;
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

    graphics_render(a.argcx, a.argvx);
}

void graphics_start_render(int *argc, char ***argv)
{
    pthread_t th;
    struct args a =
    {
        .argcx = argc,
        .argvx = argv
    };

    if (pthread_create(&th, NULL, graphics_render_loop, (void *)&a) < 0)
        FAIL_STD("%s\n", nameof(pthread_create));

    pthread_cond_wait(&args_cond, &args_mutex);

}


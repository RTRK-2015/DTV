#include <directfb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "drawing.h"
#include "graphics.h"

struct graphics_flags
{
    bool info;
    bool volume;
};

bool end = false;
static struct graphics_flags gf = { .info = false, .volume = false };

struct draw_interface draw_interface =
{
    .surface = NULL,
    .dfb_interface = NULL,
    .screen_width = 0,
    .screen_height = 0,
};

struct graphics_channel_info to_draw_info;
void graphics_show_channel_info(struct graphics_channel_info info)
{
    to_draw_info = info;
    gf.info = true;
}

uint8_t to_draw_vol;
void graphics_show_volume(uint8_t vol)
{
    to_draw_vol = vol;
    gf.volume = true;
}

void graphics_clear()
{
    gf.info = false;
    gf.volume = false;
}

void graphics_stop()
{
    end = true;
}

void handle_signal(int no)
{
    exit(0);
}

void release()
{
    draw_deinit(&draw_interface);
}

t_Error graphics_render(int *argc, char ***argv)
{
    if (draw_init(&draw_interface, argc, argv) < 0);
        FAIL("%s\n", nameof(draw_init));    

    atexit(release);
    signal(SIGINT, handle_signal);


    do
    {
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

        if (draw_refresh(&draw_interface) < 0)
        {
            release();
            return ERROR;
        }

    } while (!end);

    return NO_ERROR;
}

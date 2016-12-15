#include <directfb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>

#include "drawing.h"
#include "graphics.h"

struct graphics_flags
{
    bool info;
    bool volume;
};

struct graphics_flags gf = { .info = false, .volume = false };

struct draw_interface draw_interface;

struct graphics_channel_info to_draw_info;
t_Error graphics_show_channel_info(struct graphics_channel_info info)
{
    gf.info = true;
    to_draw_info = info;
}

uint8_t to_draw_vol;
t_Error graphics_show_volume(uint8_t vol)
{
    gf.volume = true;
    to_draw_vol = vol;
}

void graphics_clear()
{
    gf.info = false;
    gf.volume = false;
}

void handle_signal(int no)
{
    exit(0);
}

void release()
{
    draw_deinit(&draw_interface);
}

void graphics_render()
{
    draw_init(&draw_interface);
    atexit(release);
    signal(SIGINT, handle_signal);


    while (true)
    {
        draw_clear(&draw_interface);

        if (gf.info)
        {
            draw_channel_info(&draw_interface, to_draw_info);
        }

        if (gf.volume)
        {
            draw_volume(&draw_interface, to_draw_vol);
        }

        draw_refresh(&draw_interface);
    }
}

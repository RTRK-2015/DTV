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
static IDirectFBSurface *primary = NULL;
IDirectFB *dfb_interface = NULL;
static int32_t screen_width = 0;
static int32_t screen_geight = 0;


t_Error graphics_show_channel_info(struct graphics_channel_info info)
{
    gf.info = true;
}

t_Error graphics_show_volume(uint8_t vol)
{
    gf.volume = true;
}

void render()
{

    while (true)
    {
        if (gf.info)
        {
        }

        if (gf.volume)
        {
        }
    }
}

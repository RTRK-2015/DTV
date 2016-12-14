#include <directfb.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>

#include "graphics.h"


static IDirectFBSurface *primary = NULL;
IDirectFB *dfb_interface = NULL;
static int32_t screen_width = 0;
static int32_t screen_geight = 0;


t_Error graphics_init()
{
}

t_Error graphics_show_channel_info(struct graphics_channel_info info)
{
}

t_Error graphics_show_volume(uint8_t vol)
{
}

void graphics_deinit()
{
}

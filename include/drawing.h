/*! \file drawing.h
    \brief contains API for drawing graphics elements
*/
#ifndef DRAWING_H
#define DRAWING_H

#include <directfb.h>
#include <stdint.h>

#include "graphics.h"
#include "tdp_api.h"

/// \brief Basic interface necessary to display graphics elements
struct draw_interface
{
    IDirectFBSurface *surface; ///< Surface on which to draw
    IDirectFB *dfb_interface; ///< Main DFB interface
    int32_t screen_width;
    int32_t screen_height;
    DFBSurfaceDescription surface_desc;    
    IDirectFBSurface *vol_surfaces[11];
    IDirectFBFont *font_interface;
    int16_t font_height;
};

/// \brief Initialize drawing interface
int32_t draw_init(struct draw_interface *draw_i, int *argc, char ***argv);

/// \brief Draw channel_info graphics element
int32_t draw_channel_info(struct draw_interface *draw_i, struct graphics_channel_info info);

/// \brief Draw volume graphics element
int32_t draw_volume(struct draw_interface *draw_i, uint8_t vol);

/// \brief Draw a black rectangle
int32_t draw_blackscreen(struct draw_interface *draw_i);

/// \brief Clear screen of graphics elements
int32_t draw_clear(struct draw_interface *draw_i);

/// \brief Refresh display of graphics
int32_t draw_refresh(struct draw_interface *draw_i);

/// \brief Deinitialize drawing interface
int32_t draw_deinit(struct draw_interface *draw_i);

#endif

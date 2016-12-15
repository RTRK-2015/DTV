/*! \file drawing.h
    \brief contains API for drawing graphics elements
*/
#ifndef DRAWING_H
#define DRAWING_H

#include <directfb.h>

#include "graphics.h"
#include "tdp_api.h"

/// \brief Basic interface necessary to display graphics elements
struct draw_interface
{
    IDirectFBSurface *surface; ///< Surface on which to draw
    IDirectFB *dfb_interface; ///< Main DFB interface
    int screen_width;
    int screen_height;
    DFBSurfaceDescription surface_desc;
};

/// \brief Initialize drawing interface
void draw_init(struct draw_interface *draw_i, int *argc, char ***argv);

/// \brief Draw channel_info graphics element
void draw_channel_info(struct draw_interface *draw_i, struct graphics_channel_info info);

/// \brief Draw volume graphics element
void draw_volume(struct draw_interface *draw_i, uint8_t vol);

/// \brief Draw a black rectangle
void draw_blackscreen(struct draw_interface *draw_i);

/// \brief Clear screen of graphics elements
void draw_clear(struct draw_interface *draw_i);

/// \brief Refresh display of graphics
void draw_refresh(struct draw_interface *draw_i);

/// \brief Deinitialize drawing interface
void draw_deinit(struct draw_interface *draw_i);

#endif

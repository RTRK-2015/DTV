/*! \file drawing.h
    \brief Contains API for drawing graphics elements
*/
#ifndef DRAWING_H
#define DRAWING_H

// C includes
#include <stdint.h>
// Other includes
#include <directfb.h>
// Local includes
#include "graphics.h"
#include "tdp_api.h"


/// \defgroup drawing Drawing interface
/// \addtogroup drawing
/// @{
/// \brief Functions and structures for drawing.

/// \brief Basic interface necessary to display graphics elements.
struct draw_interface
{
    IDirectFBSurface *surface; ///< Surface on which to draw.
    IDirectFB *dfb_interface; ///< Main DFB interface.
    int32_t screen_width; ///< The width of the screen.
    int32_t screen_height; ///< The height of the screen.
    IDirectFBSurface *vol_surfaces[12]; ///< Preloaded volume images.
    IDirectFBFont *font_interface; ///< Preloaded font.
};


/// \brief Initialize drawing interface.
int32_t draw_init(struct draw_interface *draw_i, int *argc, char ***argv);

/// \brief Draw channel_info graphics element.
int32_t draw_channel_info(struct draw_interface *draw_i, struct graphics_channel_info info);

/// \brief Draw time graphics element.
int32_t draw_time(struct draw_interface *draw_i, struct tm tm);

/// \brief Draw volume graphics element.
int32_t draw_volume(struct draw_interface *draw_i, uint8_t vol);

/// \brief Draw no channel display.
int32_t draw_no_channel(struct draw_interface *draw_i);

/// \brief Draw radio graphics.
int32_t draw_audio_only(struct draw_interface *draw_i);

/// \brief Draw channel number.
int32_t draw_channel_number(struct draw_interface *draw_i, uint16_t ch_num);

/// \brief Draw a black rectangle.
int32_t draw_blackscreen(struct draw_interface *draw_i);

/// \brief Clear the screen.
int32_t draw_clear(struct draw_interface *draw_i);

/// \brief Refresh display.
int32_t draw_refresh(struct draw_interface *draw_i);

/// \brief Deinitialize drawing interface.
int32_t draw_deinit(struct draw_interface *draw_i);
/// @}


#endif

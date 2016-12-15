
#include "drawing.h"

#define DFBCHECK(x...)                                       \
{                                                            \
DFBResult err = x;                                           \
if (err != DFB_OK)                                           \
    {                                                        \
        fprintf(stderr, "%s <%d>:\n\t", __FILE__, __LINE__); \
        DirectFBErrorFatal( #x, err);                        \
    }                                                        \
}                                                           

int32_t draw_init(struct draw_interface *draw_i, int *argc, char ***argv)
{
    DFBCHECK(DirectFBInit(argc, argv));

    DFBCHECK(DirectFBCreate(&(draw_i->dfb_interface)));

    DFBCHECK(draw_i->dfb_interface->SetCooperativeLevel(draw_i->dfb_interface, DFSCL_FULLSCREEN));

    draw_i->surface_desc.flags = DSDESC_CAPS;
    draw_i->surface_desc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    DFBCHECK(draw_i->dfb_interface->CreateSurface(draw_i->dfb_interface,
                                                &(draw_i->surface_desc),
                                                &(draw_i->surface)));

    DFBCHECK(draw_i->surface->GetSize(draw_i->surface,
                                    &(draw_i->screen_width),
                                    &(draw_i->screen_height)));

}

int32_t draw_channel_info(struct draw_interface *draw_i, struct graphics_channel_info info)
{
}

int32_t draw_volume(struct draw_interface *draw_i, uint8_t vol)
{
}

int32_t draw_blackscreen(struct draw_interface *draw_i)
{
}

int32_t draw_clear(struct draw_interface *draw_i)
{
    DFBCHECK(draw_i->surface->Clear(draw_i->surface, 0, 0, 0, 0));
}

int32_t draw_refresh(struct draw_interface *draw_i)
{
    DFBCHECK(draw_i->surface->Flip(draw_i->surface, NULL, 0));
}

int32_t draw_deinit(struct draw_interface *draw_i)
{
    draw_i->surface->Release(draw_i->surface);
    draw_i->dfb_interface->Release(draw_i->dfb_interface);
}


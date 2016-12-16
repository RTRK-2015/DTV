
#include <stdlib.h>
#include <string.h>

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

    return EXIT_SUCCESS;
}

int32_t draw_channel_info(struct draw_interface *draw_i, struct graphics_channel_info info)
{
    const int window_width = 400;
    const int window_height = 200;

    const int window_x = draw_i->screen_width - window_width - 20;
    const int window_y = draw_i->screen_height - window_height - 20;

}

int32_t draw_volume(struct draw_interface *draw_i, uint8_t vol)
{
    IDirectFBImageProvider *provider;
    IDirectFBSurface *image_surface = NULL;
    int32_t image_height, image_width;

    char image_name[sizeof("assets/volume_xx")];
    sprintf(image_name, "assets/volume_%d", vol);

    DFBCHECK(draw_i->dfb_interface->CreateImageProvider(draw_i->dfb_interface,
                                                        image_name,
                                                        &provider));

    DFBCHECK(provider->GetSurfaceDescription(provider, &(draw_i->surface_desc)));

    DFBCHECK(draw_i->dfb_interface->CreateSurface(draw_i->dfb_interface, 
                                                &(draw_i->surface_desc),
                                                &image_surface));

    DFBCHECK(provider->RenderTo(provider, image_surface, NULL));

    provider->Release(provider);

    DFBCHECK(image_surface->GetSize(image_surface, &image_width, &image_height));

    const int x_pos = draw_i->screen_width - image_width - 50;
    const int y_pos = 50;

    DFBCHECK(draw_i->surface->Blit(draw_i->surface,
                                   image_surface,
                                   NULL,
                                   x_pos,
                                   y_pos));

    return EXIT_SUCCESS;
}

int32_t draw_blackscreen(struct draw_interface *draw_i)
{
}

int32_t draw_clear(struct draw_interface *draw_i)
{
    DFBCHECK(draw_i->surface->Clear(draw_i->surface, 0, 0, 0, 0));

    return EXIT_SUCCESS;
}

int32_t draw_refresh(struct draw_interface *draw_i)
{
    DFBCHECK(draw_i->surface->Flip(draw_i->surface, NULL, 0));

    return EXIT_SUCCESS;
}

int32_t draw_deinit(struct draw_interface *draw_i)
{
    draw_i->surface->Release(draw_i->surface);
    draw_i->dfb_interface->Release(draw_i->dfb_interface);

    return EXIT_SUCCESS;
}


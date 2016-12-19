
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
    printf("Try DFBInit with args: %d, %s\n", *argc, *argv[0]);
    DFBCHECK(DirectFBInit(argc, argv));
    printf("Successfully init\n");

    DFBCHECK(DirectFBCreate(&draw_i->dfb_interface));
    printf("Created interface\n");
    
    DFBCHECK(draw_i->dfb_interface->SetCooperativeLevel(draw_i->dfb_interface, DFSCL_FULLSCREEN));
    printf("SetCoopLvl\n");

    draw_i->surface_desc.flags = DSDESC_CAPS;
    draw_i->surface_desc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
    DFBCHECK(draw_i->dfb_interface->CreateSurface(draw_i->dfb_interface,
                                                &(draw_i->surface_desc),
                                                &(draw_i->surface)));
    printf("Created surface\n");

    DFBCHECK(draw_i->surface->GetSize(draw_i->surface,
                                    &(draw_i->screen_width),
                                    &(draw_i->screen_height)));

    printf("Screen_h: %d, Screen_w: %d\n", draw_i->screen_width, draw_i->screen_height);

    printf("Preload volume assets\n");

    for (uint8_t i = 0; i < 11; ++i)
    {
        IDirectFBImageProvider *provider;

        char image_name[sizeof("assets/volume_xx.png")];
        sprintf(image_name, "assets/volume_%d.png", i);
        
        DFBCHECK(draw_i->dfb_interface->CreateImageProvider(draw_i->dfb_interface,
                                                            image_name,
                                                            &provider));

        DFBCHECK(provider->GetSurfaceDescription(provider, &draw_i->surface_desc));

        DFBCHECK(draw_i->dfb_interface->CreateSurface(draw_i->dfb_interface,
                                                     &draw_i->surface_desc,
                                                     &draw_i->vol_surfaces[i]));

        DFBCHECK(provider->RenderTo(provider, draw_i->vol_surfaces[i], NULL));

        provider->Release(provider);
        
        printf("Loaded vol_%d\n", i);

    }

    printf("Try set font\n");
    DFBFontDescription font_desc;
    draw_i->font_height = 52;
    printf("Set font_height: %d\n", draw_i->font_height);
    font_desc.flags = DFDESC_HEIGHT;
    font_desc.height = draw_i->font_height;

    DFBCHECK(draw_i->dfb_interface->CreateFont(draw_i->dfb_interface,
                                              "/home/galois/fonts/DejaVuSans.ttf",
                                              &font_desc,
                                              &draw_i->font_interface));

    DFBCHECK(draw_i->surface->SetFont(draw_i->surface, draw_i->font_interface));
    
    printf("Set font\n");

    return EXIT_SUCCESS;
}

int32_t draw_channel_info(struct draw_interface *draw_i, struct graphics_channel_info info)
{
    const int16_t frame_width = 814;
    const int16_t frame_height = 314;
    const int16_t window_width = 800;
    const int16_t window_height = 300;
    const int16_t font_height = draw_i->font_height;
    
    const int16_t window_x = draw_i->screen_width - window_width - 20;
    const int16_t window_y = draw_i->screen_height - window_height - 20;

    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 45, 45, 45, 0x88));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x - 7,
                                            window_y - 7,
                                            frame_width,
                                            frame_height));
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 88, 88, 88, 0x88));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x,
                                            window_y,
                                            window_width,
                                            window_height));


    const char fmt1[] = "Ch_num: %d, Teletext: %s";
    char ch_num_tel[sizeof(fmt1) + 5];
    sprintf(ch_num_tel, fmt1, info.ch_num, info.teletext ? "Yes" : "No");

    const int16_t offset = 20;
    const int16_t str1_x = window_x + offset;
    const int16_t str1_y = window_y + font_height + offset;
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 0x13, 0x96, 0x14, 0xAA));
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                        ch_num_tel,
                                        -1, 
                                        str1_x,
                                        str1_y,
                                        DSTF_LEFT));

    const char fmt2[] = "V_pid: %d, A_pid: %d";
    char pids[sizeof(fmt2) + 6];
    sprintf(pids, fmt2, info.vpid, info.apid);
    const int16_t str2_y = str1_y + font_height + offset;
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         pids,
                                         -1,
                                         str1_x,
                                         str2_y,
                                         DSTF_LEFT));

    const char fmt3[] = "%FT%T";
#define TIME_SIZE 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1
    char time_str[TIME_SIZE];
    strftime(time_str, TIME_SIZE, fmt3, &info.tm);
    const int16_t str3_y = str2_y + font_height + offset;
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         time_str,
                                         -1,
                                         str1_x,
                                         str3_y,
                                         DSTF_LEFT));

    return EXIT_SUCCESS;
}

int32_t draw_volume(struct draw_interface *draw_i, uint8_t vol)
{
    int32_t image_height, image_width;

    DFBCHECK(draw_i->vol_surfaces[0]->GetSize(draw_i->vol_surfaces[0],
                                             &image_width, &image_height));

    const int x_pos = draw_i->screen_width - image_width - 50;
    const int y_pos = 50;

    DFBCHECK(draw_i->surface->Blit(draw_i->surface,
                                   draw_i->vol_surfaces[vol],
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

    for (uint8_t i = 0; i < 11; ++i)
    {
        draw_i->vol_surfaces[i]->Release(draw_i->vol_surfaces[i]);
    }

    return EXIT_SUCCESS;
}


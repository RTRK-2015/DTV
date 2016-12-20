
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


struct color
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

static struct color frame_color =
{
    .r = 45,
    .g = 45,
    .b = 45,
    .a = 0xff
};
static struct color back_color =
{
    .r = 88,
    .g = 88,
    .b = 88,
    .a = 0xff
};
static struct color text_color =
{
    .r = 0x13,
    .g = 0x96,
    .b = 0x14,
    .a = 0xff
};
static const int16_t offset = 20;
static const int16_t border = 7;
static const int16_t font_height = 52;

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
    draw_i->font_height = font_height;
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
    const int16_t window_width = 800;
    const int16_t window_height = 225;
    const int16_t frame_width = window_width + 2 * border;
    const int16_t frame_height = window_height + 2 * border;
    const int16_t window_x = draw_i->screen_width - window_width - offset;
    const int16_t window_y = draw_i->screen_height - window_height - offset;

    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       frame_color.r,
                                       frame_color.g,
                                       frame_color.b,
                                       frame_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x - border,
                                            window_y - border,
                                            frame_width,
                                            frame_height));
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 
                                       back_color.r,
                                       back_color.g,
                                       back_color.b,
                                       back_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x,
                                            window_y,
                                            window_width,
                                            window_height));


    const char fmt1[] = "Ch_num: %d, Teletext: %s";
    char ch_num_tel[sizeof(fmt1) + 5];
    sprintf(ch_num_tel, fmt1, info.ch_num, info.teletext ? "Yes" : "No");

    const int16_t str_x = window_x + offset;
    const int16_t str1_y = window_y + font_height;
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 
                                       text_color.r,
                                       text_color.g,
                                       text_color.b,
                                       text_color.a));
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                        ch_num_tel,
                                        -1, 
                                        str_x,
                                        str1_y,
                                        DSTF_LEFT));

    const char fmt2[] = "V_pid: %d, A_pid: %d";
    char pids[sizeof(fmt2) + 6];
    sprintf(pids, fmt2, info.vpid, info.apid);
    const int16_t str2_y = str1_y + font_height + offset;
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         pids,
                                         -1,
                                         str_x,
                                         str2_y,
                                         DSTF_LEFT));

    const char fmt3[] = "S_type: %d, Ch_name: %s";
    char sdt_str[sizeof(fmt3) + 20];
    sprintf(sdt_str, fmt3, info.sdt.st, info.sdt.name);
    const int16_t str3_y = str2_y + font_height + offset;
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         sdt_str,
                                         -1,
                                         str_x,
                                         str3_y,
                                         DSTF_LEFT));
    return EXIT_SUCCESS;
}

int32_t draw_time(struct draw_interface *draw_i, struct tm tm)
{
    const int16_t window_width = 600;
    const int16_t window_height = 70;
    const int16_t window_x = offset;
    const int16_t window_y = draw_i->screen_height - window_height - offset;
    const int16_t frame_width = window_width + 2 * border;
    const int16_t frame_height = window_height + 2 * border;

    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       frame_color.r,
                                       frame_color.g,
                                       frame_color.b,
                                       frame_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x - border,
                                            window_y - border,
                                            frame_width,
                                            frame_height));
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 
                                       back_color.r,
                                       back_color.g,
                                       back_color.b,
                                       back_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x,
                                            window_y,
                                            window_width,
                                            window_height));

    const char fmt[]= "%FT%T";
#define TIME_SIZE 4 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 + 1 + 2 +1
    char time_str[TIME_SIZE];
    strftime(time_str, TIME_SIZE, fmt, &tm);
    const int16_t str_x = window_x + offset;
    const int16_t str_y = window_y + font_height;
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       text_color.r,
                                       text_color.g,
                                       text_color.b,
                                       text_color.a));
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         time_str,
                                         -1,
                                         str_x,
                                         str_y,
                                         DSTF_LEFT));
#undef TIME_SIZE

    return EXIT_SUCCESS;
}

int32_t draw_volume(struct draw_interface *draw_i, uint8_t vol)
{
    int32_t image_height, image_width;

    DFBCHECK(draw_i->vol_surfaces[0]->GetSize(draw_i->vol_surfaces[0],
                                             &image_width, &image_height));

    const int16_t x_pos = draw_i->screen_width - image_width - offset;
    const int16_t y_pos = offset;

    DFBCHECK(draw_i->surface->Blit(draw_i->surface,
                                   draw_i->vol_surfaces[vol],
                                   NULL,
                                   x_pos,
                                   y_pos));

    return EXIT_SUCCESS;
}

int32_t draw_no_channel(struct draw_interface *draw_i)
{
    const int16_t window_width = 400;
    const int16_t window_height = 70;
    const int16_t window_x = draw_i->screen_width/2 - window_width/2;
    const int16_t window_y = draw_i->screen_height/2 - window_height/2;
    const int16_t frame_width = window_width + 2 * border;
    const int16_t frame_height = window_height + 2 * border;

    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       frame_color.r,
                                       frame_color.g,
                                       frame_color.b,
                                       frame_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x - border,
                                            window_y - border,
                                            frame_width,
                                            frame_height));
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 
                                       back_color.r,
                                       back_color.g,
                                       back_color.b,
                                       back_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x,
                                            window_y,
                                            window_width,
                                            window_height));

    const char no_channel_str[] = "NO CHANNEL";
    const int16_t str_x = window_x + offset;
    const int16_t str_y = window_y + font_height;
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       text_color.r,
                                       text_color.g,
                                       text_color.b,
                                       text_color.a));
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         no_channel_str,
                                         -1,
                                         str_x,
                                         str_y,
                                         DSTF_LEFT));


    return EXIT_SUCCESS;
}

int32_t draw_audio_only(struct draw_interface *draw_i)
{
    const int16_t window_width = 400;
    const int16_t window_height = 70;
    const int16_t window_x = draw_i->screen_width/2 - window_width/2;
    const int16_t window_y = draw_i->screen_height/2 - window_height/2;
    const int16_t frame_width = window_width + 2 * border;
    const int16_t frame_height = window_height + 2 * border;

    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       frame_color.r,
                                       frame_color.g,
                                       frame_color.b,
                                       frame_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x - border,
                                            window_y - border,
                                            frame_width,
                                            frame_height));
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 
                                       back_color.r,
                                       back_color.g,
                                       back_color.b,
                                       back_color.a));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            window_x,
                                            window_y,
                                            window_width,
                                            window_height));

    const char audio_only_str[] = "AUDIO ONLY";
    const int16_t str_x = window_x + offset;
    const int16_t str_y = window_y + font_height;
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       text_color.r,
                                       text_color.g,
                                       text_color.b,
                                       text_color.a));
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         audio_only_str,
                                         -1,
                                         str_x,
                                         str_y,
                                         DSTF_LEFT));


    return EXIT_SUCCESS;
}

int32_t draw_channel_number(struct draw_interface *draw_i, uint16_t ch_num)
{
    const int16_t str_x = 150 + offset;
    const int16_t str_y = font_height + offset;
    char ch_num_str[5];
    sprintf(ch_num_str, "%d", ch_num);
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface,
                                       text_color.r,
                                       text_color.g,
                                       text_color.b,
                                       0xFF));
    DFBCHECK(draw_i->surface->DrawString(draw_i->surface,
                                         ch_num_str,
                                         -1,
                                         str_x,
                                         str_y,
                                         DSTF_RIGHT));
    return EXIT_SUCCESS;
}

int32_t draw_blackscreen(struct draw_interface *draw_i)
{
    DFBCHECK(draw_i->surface->SetColor(draw_i->surface, 0, 0, 0, 0xff));
    DFBCHECK(draw_i->surface->FillRectangle(draw_i->surface,
                                            0,
                                            0,
                                            draw_i->screen_width,
                                            draw_i->screen_height));

    return EXIT_SUCCESS;
                                            
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
    printf("Releasing font interface\n");
    draw_i->font_interface->Release(draw_i->font_interface);

    for (uint8_t i = 0; i < 11; ++i)
    {
        printf("Releasing volume surface %d\n", i);
        draw_i->vol_surfaces[i]->Release(draw_i->vol_surfaces[i]);
    }
    
    printf("Releasing surface\n");
    draw_i->surface->Release(draw_i->surface);

    printf("Releasing DFB interface\n");
    draw_i->dfb_interface->Release(draw_i->dfb_interface);
    
    return EXIT_SUCCESS;
}


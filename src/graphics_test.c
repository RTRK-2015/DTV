#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "common.h"
#include "graphics.h"

struct args
{
    int *argcx;
    char ***argvx;
};

void handle_signal(int no)
{
    exit(0);
}

int32_t f(int argc, char **argv)
{
    printf("main args: %d, %s\n", argc, argv[0]);

    graphics_start_render(&argc, &argv);
    atexit(graphics_stop);

    
    for(int i = 0; i < 11; ++i)
    {
        //printf("Show volume %d\n", i);
        //sleep(1);
        //graphics_show_volume(i);
    }


    printf("Show info\n");
    struct graphics_channel_info info =
    {
        .ch_num = 490,
        .teletext = true,
        .vpid = 101,
        .apid = 103
    };
    info.tm.tm_sec = 33;
    info.tm.tm_min = 27;
    info.tm.tm_hour = 16;
    info.tm.tm_mday = 19;
    info.tm.tm_mon = 9;
    info.tm.tm_year = 116;
    info.sdt.st = 1;
    strcpy(info.sdt.name, "La 1");
    sleep(2);
    graphics_show_channel_info(info);

    while(true);
    printf("Clear\n");
    sleep(2);
    graphics_clear();

    printf("End\n");
    graphics_stop();

    return EXIT_SUCCESS;
}

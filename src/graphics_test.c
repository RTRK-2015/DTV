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

void* render(void *args)
{
    struct args *a = (struct args *)args;
    int argc = *(a->argcx);
    char **argv = *(a->argvx);

    printf("Enter render thread with args: %d, %s\n", argc, argv[0]);
    
    if (graphics_render(&argc, &argv) == ERROR)
        FAIL("%s", nameof(graphics_render));

}

int32_t f(int argc, char **argv)
{
    pthread_t th;
    char c;

    printf("main args: %d, %s\n", argc, argv[0]);

    struct args a =
    {
        .argcx = &argc,
        .argvx = &argv
    };

    pthread_create(&th, NULL, render, &a);


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
        /*.tm = 
        {
            .tm_sec = 33,
            .tm_min = 27,
            .tm_hour = 16,
            .tm_day = 19,
            .tm_mon = 9,
            .tm_year = 116,
        }*/
    };

    info.tm.tm_sec = 33;
    info.tm.tm_min = 27;
    info.tm.tm_hour = 16;
    info.tm.tm_mday = 19;
    info.tm.tm_mon = 9;
    info.tm.tm_year = 116;
    sleep(2);
    graphics_show_channel_info(info);

    while(true);
    printf("Clear\n");
    sleep(2);
    graphics_clear();

    printf("End\n");
    graphics_stop();


    pthread_join(th, NULL);


    return EXIT_SUCCESS;
}

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

int32_t main(int argc, char **argv)
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
        printf("Show volume %d\n", i);
        sleep(1);
        graphics_show_volume(i);
    }


    printf("Show info\n");
    struct graphics_channel_info info = { 0 };
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

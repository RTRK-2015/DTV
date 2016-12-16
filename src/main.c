#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

    if (graphics_render(a->argcx, a->argvx) == ERROR)
        FAIL("%s", nameof(graphics_render));

}

int32_t main(int argc, char **argv)
{
    pthread_t th;
    

    struct args a =
    {
        .argcx = &argc,
        .argvx = &argv
    };

    pthread_create(&th, NULL, render, &a);

    for(int i = 0; i < 11; ++i)
    {
        printf("Show volume %d\n", i);
        getchar();
        graphics_show_volume(i);
    }

    printf("Clear\n");
    getchar();
    graphics_clear();
    
    printf("End\n");
    getchar();
    graphics_stop();


    pthread_join(th, NULL);


    return EXIT_SUCCESS;
}

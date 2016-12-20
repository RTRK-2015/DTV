#define _POSIX_C_SOURCE 200809L
// Matching include
#include "rc.h"
// C includes
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
// Unix includes
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
// Linux includes
#include <linux/input.h>
// Local includes
#include "common.h"


#define EVENT_KEY_PRESS 1
#define EVENT_KEY_AUTOREPEAT 2


static pthread_t event_th;


static ssize_t get_keys(int fd, size_t count, char *buf)
{
    ssize_t ret = read(fd, buf, count * sizeof(struct input_event));

    if (ret <= 0)
        FAIL_STD("%s\n", nameof(open));

    return ret / (ssize_t)sizeof(struct input_event);
}


struct args
{
    int fd;
    rc_key_callback kc;
};


static pthread_mutex_t args_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t args_cond = PTHREAD_COND_INITIALIZER;
bool stop = false;
static void* event_loop(void *args)
{
    struct args a = *(struct args *)args;
    pthread_cond_signal(&args_cond);

    struct input_event *event_buffer = malloc(sizeof(struct input_event));
    if (event_buffer == NULL)
        FAIL_STD("%s\n", nameof(malloc));

    while (true)
    {
        ssize_t event_count = get_keys(a.fd, 1, (char *)event_buffer);

        if (event_count > 0)
        {
            if (event_buffer->value == EVENT_KEY_PRESS
                || event_buffer->value == EVENT_KEY_AUTOREPEAT)
            {
            	    a.kc(event_buffer->code);
            }
        }
    }

    return NULL;
}


void rc_start_loop(const char *dev, rc_key_callback kc)
{
    int fd = open(dev, O_RDWR);
    if (fd < 0)
        FAIL_STD("%s\n", nameof(open));

    char device_name[20];
    if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) < 0)
        FAIL_STD("%s\n", nameof(ioctl));

    struct args a = { .fd = fd, .kc = kc };

    if (pthread_create(&event_th, NULL, event_loop, (void *)&a) > 0)
        FAIL_STD("%s\n", nameof(pthread_create));

    pthread_cond_wait(&args_cond, &args_mutex);
}


void rc_stop_loop()
{
    printf("Stopping event loop\n");
    pthread_cancel(event_th);
}


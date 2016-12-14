/*// Matching include
#include "mapping.h"
// C includes
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// Unix includes
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
// Linux includes
#include <linux/input.h>
// Local includes
#include "common.h"


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
    struct key_mapping km;
};


static pthread_mutex_t args_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t args_cond = PTHREAD_COND_INITIALIZER;
static void* event_loop(void *args)
{
    static const size_t NUM_EVENTS = 5;
    struct args a = *(struct args *)args;
    pthread_cond_signal(&args_cond);

    struct input_event *event_buffer =
        malloc(NUM_EVENTS * sizeof(struct input_event));
    if (event_buffer == NULL)
        FAIL_STD("%s\n", nameof(malloc));

    while (1)
    {
        ssize_t event_count = get_keys(a.fd, NUM_EVENTS, (char *)event_buffer);

        for (size_t i = 0; i < event_count; ++i)
        {
        	if (event_buffer[i].value == 1)
        	{
            	switch (event_buffer[i].code)
	            {
	            case 62:
	            	a.km.key_prog_up(62);
	            	break;
	            	
	            case 61:
	            	a.km.key_prog_down(61);
	            	break;
	            }
	        }
        }
    }
}


void start_event_loop(const char *dev, struct key_mapping km)
{
    int fd = open(dev, O_RDWR);
    if (fd < 0)
        FAIL_STD("%s\n", nameof(open));

    char device_name[20];
    if (ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name) < 0)
        FAIL_STD("%s\n", nameof(ioctl));

    struct args a = { .fd = fd, .km = km };

    pthread_t loop_thread;
    if (pthread_create(&loop_thread, NULL, event_loop, (void *)&a) < 0)
        FAIL_STD("%s\n", nameof(pthread_create));

    pthread_cond_wait(&args_cond, &args_mutex);
}
*/

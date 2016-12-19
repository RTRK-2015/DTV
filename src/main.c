#define _POSIX_C_SOURCE 199309L
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include "common.h"
#include "graphics.h"
#include "config.h"
#include "dtv.h"
#include "rc.h"


static uint8_t volume = 5;

static bool started_selecting = false;
static uint16_t selected_channel = -1;
static timer_t ch_timer;
static const struct itimerspec reset = { 0 };


void handle_signal(int signum)
{
	exit(EXIT_SUCCESS);
}

void confirm_channel(union sigval s)
{
	printf("Confirmed channel: %d\n", selected_channel);
	timer_settime(ch_timer, 0, &reset, NULL);
	
	dtv_switch_channel(selected_channel);
	
	started_selecting = false;
}


void react_to_keypress(int key_code)
{	
	printf("received code: %d\n", key_code);
		
	struct itimerspec ts = { .it_value.tv_sec = 1, .it_value.tv_nsec = 500000000 };
        struct dtv_channel_info dci;
        struct graphics_channel_info gci;

	switch (key_code)
	{
	case 2 ... 11:
		if (!started_selecting)
		{
			started_selecting = true;
			selected_channel = key_code - 1;
		}
		else
		{
			selected_channel = 10 * selected_channel + (key_code - 1);
		}
		printf("selected_channel: %d\n", selected_channel);
		timer_settime(ch_timer, 0, &reset, NULL);
		timer_settime(ch_timer, 0, &ts, NULL);	
		break;

	case 62:
                ++selected_channel;
		printf("Switching to channel %d\n", selected_channel);
		
                dci = dtv_switch_channel(selected_channel);
                dtv_get_info(selected_channel);
                break;
		
	case 61:
                --selected_channel;
		printf("Switching to channel %d\n", selected_channel);
		
                dci = dtv_switch_channel(selected_channel);	
                dtv_get_info(selected_channel);
                break;

	case 63:
		if (volume < 10)
			++volume;
		dtv_set_volume(volume);
		break;
		
	case 64:
		if (volume > 0)
			--volume;
		dtv_set_volume(volume);
		break;
	}
}


int main(int argc, char **argv)
{
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	
	struct sigevent se =
	{ .sigev_notify_function = confirm_channel
	, .sigev_notify = SIGEV_THREAD
	};
	timer_create(CLOCK_REALTIME, &se, &ch_timer);
	
	FILE *f = fopen(argv[1], "r");
	if (f == NULL)
		FAIL_STD("%s\n", nameof(fopen));
		
	struct config_init_ch_info init_info = config_get_init_ch_info(f);
        selected_channel = init_info.ch_num;
        printf("Channel: %d\n", selected_channel);
	
	dtv_init(init_info);
	atexit(dtv_deinit);
	
	rc_start_loop("/dev/input/event0", react_to_keypress);
	atexit(rc_stop_loop);

	while (true);
}

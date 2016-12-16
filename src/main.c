#define _POSIX_C_SOURCE 199309L
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include "common.h"
#include "config.h"
#include "dtv.h"
#include "rc.h"


static const uint16_t *channels = NULL;
static size_t channels_len = 0;
static size_t channels_idx = 0;

static uint8_t volume = 5;

static bool started_selecting = false;
static uint16_t selected_channel = 0;
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
	
	size_t i = 0;
	for (; i < channels_len; ++i)
	{
		if (channels[i] == selected_channel)
		{
			channels_idx = i;
			dtv_switch_channel(selected_channel);
		}
	}
	
	started_selecting = false;
	
	if (i == channels_len)
		printf("Bullshit channel!\n");
}


void react_to_keypress(int key_code)
{	
	printf("received code: %d\n", key_code);
		
	struct itimerspec ts = { .it_value.tv_sec = 1, .it_value.tv_nsec = 500000000 };
	
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
		if (channels_idx == channels_len - 1)
			channels_idx = 0;
		else
			++channels_idx;
			
		printf("Index: %d\n", channels_idx);
		printf("Switching to channel %d\n", channels[channels_idx]);
		
		dtv_switch_channel(channels[channels_idx]);
		break;
		
	case 61:
		if (channels_idx == 0)
			channels_idx = channels_len - 1;
		else
			--channels_idx;
			
		printf("Index: %d\n", channels_idx);
		printf("Switching to channel %d\n", channels[channels_idx]);
		
		dtv_switch_channel(channels[channels_idx]);
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
	
	dtv_init(init_info);
	atexit(dtv_deinit);
	
	channels = dtv_get_channels();
	size_t len = 0;
	const uint16_t *i = channels;
	while (*i != END_OF_CHANNELS)
	{
		++len;
		++i;
	}
	channels_len = len;
	
	printf("Channels:\n");
	for (size_t i = 0; i < channels_len; ++i)
		printf("%d\n", channels[i]);
	
	rc_start_loop("/dev/input/event0", react_to_keypress);
	atexit(rc_stop_loop);
	
	while (true);
}

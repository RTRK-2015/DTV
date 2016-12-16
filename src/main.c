#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "common.h"
#include "config.h"
#include "dtv.h"
#include "rc.h"


static const uint16_t *channels = NULL;
static size_t channels_len = 0;
static size_t channels_idx = 0;

static uint8_t volume = 5;


void handle_signal(int signum)
{
	exit(EXIT_SUCCESS);
}

void react_to_keypress(int key_code)
{
	printf("received code: %d\n", key_code);
	
	switch (key_code)
	{
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

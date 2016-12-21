#define _POSIX_C_SOURCE 200809L
// C includes
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
// Unix includes
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
// Local includes
#include "common.h"
#include "graphics.h"
#include "config.h"
#include "dtv.h"
#include "rc.h"


#define LOG_MAIN(fmt, ...) LOG("Main", fmt, ##__VA_ARGS__)

static pthread_mutex_t channel_mutex = PTHREAD_MUTEX_INITIALIZER;

static uint8_t volume = 5;

static bool started_selecting = false;
static uint16_t selected_channel = -1;
static timer_t ch_timer;
static timer_t updown_timer;
static const struct itimerspec reset = { 0 };
static bool muted = false;
static struct graphics_channel_info gci;
static time_t t_tot = 0, t_start;


#define KEY_BACK 1
#define KEY_1 2
#define KEY_0 11
#define KEY_MUTE 60
#define KEY_CHANNEL_DOWN 61
#define KEY_CHANNEL_UP 62
#define KEY_VOLUME_UP 63
#define KEY_VOLUME_DOWN 64
#define KEY_POWEROFF 116
#define KEY_INFO 358


void handle_signal(int signum)
{
    if (signum == SIGINT)
        LOG_MAIN("Caught SIGINT\n");
    else if (signum == SIGTERM)
        LOG_MAIN("Caught SIGTERM\n");
    else
        LOG_MAIN("Caught signal %d\n", signum);

    timer_delete(ch_timer);
    timer_delete(updown_timer);
    exit(EXIT_SUCCESS);
}

void calculate_time()
{
    time_t t_current = time(NULL);
    LOG_MAIN("t_tot was: %d\n", t_tot);
    t_tot += (t_current - t_start);
    LOG_MAIN("is now: %d\n", t_tot);
    t_start = t_current;
    gci.tm = *gmtime(&t_tot);
}

void confirm_channel(union sigval s)
{
    pthread_mutex_lock(&channel_mutex);

    timer_settime(ch_timer, 0, &reset, NULL);
    timer_settime(updown_timer, 0, &reset, NULL);
    started_selecting = false; 

    LOG_MAIN("Confirmed channel: %d\n", selected_channel);
    struct dtv_channel_info dci = dtv_switch_channel(selected_channel);
    struct sdt sdt = dtv_get_info(selected_channel);

    gci.ch_num = dci.ch_num;
    gci.vpid = dci.vpid;
    gci.apid = dci.apid;
    gci.teletext = dci.teletext;
    gci.sdt = sdt;
    calculate_time();
    graphics_show_channel_info(gci);

    pthread_mutex_unlock(&channel_mutex);
}


void react_to_keypress(int key_code)
{	
	LOG_MAIN("received code: %d\n", key_code);
		
	static const struct itimerspec confirm_ts =
        { .it_value.tv_sec = 1
        , .it_value.tv_nsec = 250000000L
        };
        static const struct itimerspec updown_ts =
        { .it_value.tv_sec = 0
        , .it_value.tv_nsec = 400000000L
        };
        int adjusted_key_code = (key_code == KEY_0)? 0 : key_code - 1;

	switch (key_code)
	{
	case KEY_1 ... KEY_0:
	    if (!started_selecting)
	    {
		started_selecting = true;
		selected_channel = adjusted_key_code;
	    }
	    else
	    {
		selected_channel = 10 * selected_channel + adjusted_key_code;
	    }
	    LOG_MAIN("selected_channel: %d\n", selected_channel);
            graphics_show_channel_number(selected_channel);
	    timer_settime(ch_timer, 0, &confirm_ts, NULL);	
	    break;

	case KEY_CHANNEL_UP:
            timer_settime(updown_timer, 0, &reset, NULL);
            ++selected_channel;
            graphics_show_channel_number(selected_channel);
            timer_settime(updown_timer, 0, &updown_ts, NULL);
            break;
		
	case KEY_CHANNEL_DOWN:
            timer_settime(updown_timer, 0, &reset, NULL);
            --selected_channel;
            graphics_show_channel_number(selected_channel);
            timer_settime(updown_timer, 0, &updown_ts, NULL);
            break;

	case KEY_VOLUME_UP:
	    if (volume < 10)
		++volume;
	    if (!muted)
            {
                dtv_set_volume(volume);
                graphics_show_volume(volume);
            }
	    break;
		
	case KEY_VOLUME_DOWN:
	    if (volume > 0)
		--volume;
	    if (!muted)
            {   
                dtv_set_volume(volume);
                graphics_show_volume(volume);
            }
	    break;

        case KEY_MUTE:
            if (!muted)
            {
                muted = true;
                dtv_set_volume(0);
                graphics_show_volume(0);
            }
            else
            {
                muted = false;
                dtv_set_volume(volume);
                graphics_show_volume(volume);
            }
            break;

        case KEY_POWEROFF:
            handle_signal(SIGTERM);
            break;

        case KEY_BACK:
            graphics_clear();
            break;

        case KEY_INFO:
            calculate_time();
            graphics_show_channel_info(gci);
            break;
	}
}


void* update_time(void *args)
{
    LOG_MAIN("Got tm\n");
    struct tm tm = dtv_get_time();
    t_tot = mktime(&tm);
    LOG_MAIN("t_tot set to: %d\n", t_tot);

    return NULL;
}


int main(int argc, char **argv)
{	
    t_start = time(NULL);

    struct sigevent se =
    { .sigev_notify_function = confirm_channel
    , .sigev_notify = SIGEV_THREAD
    };
    timer_create(CLOCK_REALTIME, &se, &ch_timer);
    timer_create(CLOCK_REALTIME, &se, &updown_timer);
	
    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
	FAIL_STD("%s\n", nameof(fopen));

    struct config_init_ch_info init_info = config_get_init_ch_info(f);
    selected_channel = init_info.ch_num;

    graphics_start_render(&argc, &argv);
 
    dtv_init(init_info);

    gci.ch_num = init_info.ch_num;
    gci.vpid = init_info.vpid;
    gci.apid = init_info.apid;
    gci.teletext = true;
    gci.sdt = dtv_get_info(init_info.ch_num);
    graphics_show_channel_info(gci);
    graphics_show_channel_number(init_info.ch_num);

    pthread_t time_th;
    if (pthread_create(&time_th, 0, update_time, NULL) < 0)
        FAIL_STD("%s\n", nameof(pthread_create));
    if (pthread_detach(time_th) < 0)
        FAIL_STD("%s\n", nameof(pthread_detach));

    rc_start_loop("/dev/input/event0", react_to_keypress);

    sleep(2);
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    atexit(graphics_stop);
    atexit(dtv_deinit);
    atexit(rc_stop_loop);

    while (true);
}

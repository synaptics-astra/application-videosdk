#include "keypad.h"

#ifdef CURR_MACH_DVF120
/* DVF120: SW23 */
#define EV_KEY_CODE 0x17
#define KEY_DESC "DVF120 EVK SW23 for SynaAVeda demo launch"
#define DEMO_LAUNCHER "/home/root/synaaveda_dvf120/demo_dvf120.sh"
#endif

#ifdef CURR_MACH_VS640
/* VS640:  */
#define EV_KEY_CODE 0x72
#define KEY_DESC "VS640 EVK SW3  (VOL-) for SynaAVeda demo launch"
#define DEMO_LAUNCHER "/home/root/synaaveda_vs640/demo_vs640.sh"
#endif

#ifdef CURR_MACH_VS680
/* VS680 */
#define EV_KEY_CODE 0x73
#define KEY_DESC "VS680 EVK SW4 (VOL+) for SynaAVeda demo launch"
#define DEMO_LAUNCHER "/home/root/synaaveda_vs680/demo_vs680.sh"
#endif

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)

int fd;
bool demo_state = 0;
int m_pid = -1;

int check_supported_key(int fd, int key)
{
	unsigned int type, code;
	unsigned long bit[EV_MAX][NBITS(KEY_MAX)];

	memset(bit, 0, sizeof(bit));
	ioctl(fd, EVIOCGBIT(0, EV_MAX), bit[0]);

	for (type = 0; type < EV_MAX; type++) {
		if (test_bit(type, bit[0]) && type != EV_REP) {
			if (type == EV_SYN) continue;
			ioctl(fd, EVIOCGBIT(type, KEY_MAX), bit[type]);
			for (code = 0; code < KEY_MAX; code++)
			{
				if (test_bit(code, bit[type])) {
					if(code == key)
					{
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

int read_event()
{
	struct input_event ev;

	while (read(fd, &ev, sizeof(ev)) >= 0)
	{
		if (ev.type == EV_KEY && ev.code == EV_KEY_CODE)
		{
			return ev.value;
		}
	}
	return -1;
}

void start_demo(void)
{
	printf("Start demo: %s\n", DEMO_LAUNCHER);
	system(DEMO_LAUNCHER);
}

void daemon_func(void)
{
	printf("keypad-daemon: %s\n", KEY_DESC);
	char path[256] = {0};
	int evt = -1, ret = 0, cnt = 0;

	snprintf (path, sizeof (path), "/dev/input/event%d", cnt);

retry:
	{
		fd = open (path, O_RDONLY);

		if(fd == -1)
		{
			printf("ERROR Opening %s\n", path);
			return;
		}

		ret = check_supported_key(fd, EV_KEY_CODE);
		if(ret)
		{
			printf("Device (%s) found for key(%d) \n", path, EV_KEY_CODE);
		}
		else
		{
			close(fd);
			snprintf (path, sizeof (path), "/dev/input/event%d", ++cnt);
			printf("Retrying with %s\n", path);
			goto retry;
		}
	}

	while (1)
	{
		if ((evt = read_event()) == 1)
		{
			do
			{
				evt = read_event();
			} while (evt != 0);

			printf("Button (%s) Pressed and Released\n", KEY_DESC);

			if (demo_state == 0)
			{
				demo_state = 1;
				start_demo();

				while(1)
					sleep(1);
				return;
			}
		}
	}

	close(fd);

	return;
}

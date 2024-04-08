#include "keypad.h"

int main(int argc, char** argv)
{
	printf("Keypad-daemon is started\n");
	daemon_func();
	printf("Keypad-daemon is exited\n");
    return 0;
}

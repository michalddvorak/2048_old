#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include "io.h"

void nonblock(int state)
{
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate);
	
	if(state == NB_ENABLE)
	{
		ttystate.c_lflag &= ~ICANON;
		ttystate.c_cc[VMIN] = 1;
	}
	else if(state == NB_DISABLE)
	{
		ttystate.c_lflag |= ICANON;
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int kbhit()
{
	struct timeval tv;
	fd_set fds;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
	return FD_ISSET(STDIN_FILENO,&fds);
}


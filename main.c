#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "game.h"

#define DEFAULT_ROWS 4
#define DEFAULT_COLS 4
#define DEFAULT_TARGET 2048
#define DEFAULT_RANGE 2
#define DEFAULT_NUMGEN 1


void init_terminal(struct termios* old_ttystate)
{
	tcgetattr(STDIN_FILENO, old_ttystate); //store the old state
	struct termios ttystate;
	tcgetattr(STDIN_FILENO, &ttystate);
	ttystate.c_lflag &= ~ICANON; //disable canonical mode
	ttystate.c_cc[VMIN] = 1; //react on 1 character
	ttystate.c_lflag &= ~ECHO;
	tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

void restore_terminal(struct termios* ttystate)
{
	tcsetattr(STDIN_FILENO, TCSANOW, ttystate);
}


void usage()
{
	printf("2048 game\n"
		   "+-------+\n"
		   "|Options|\n"
		   "+-------+\n"  "-r <num> ... set the number of rows (default = 4)\n"
		   "-c <num> ... set the number of columns (default = 4)\n"
		   "-t <num> ... set the target number (default = 2048, max = 8192)\n"
		   "-a <num> ... set the range of numbers to be generated (default = 2)\n"
		   "         ... if range=r, then numbers 2^1, 2^2, ..., 2^r will be generated\n"
		   "-g <num> ... set number of new tiles to be generated with each move (default = 1)"
		   "-h ... show this help message and exit\n"
		   "+-----------+\n"
		   "|How to play|\n"
		   "+-----------+\n"
		   "w,a,s,d ... movement (up, left, down, right, respectively)\n"
		   "x ... exit\n");
}


int parse_args(int argc, char* argv[], int* rows, int* cols, int* target, int* range, int* numgen)
{
	int c;
	while((c = getopt(argc, argv, "hr:c:t:a:g:")) != -1)
		switch(c)
		{
			case 'r':
				if(!sscanf(optarg, "%d", rows) || *rows <= 0)
				{
					fprintf(stderr, "Invalid argument to -r option, expected positive number\n");
					return 0;
				}
				break;
			case 'c':
				if(!sscanf(optarg, "%d", cols) || *cols <= 0)
				{
					fprintf(stderr, "Invalid argument to -c option, expected positive number\n");
					return 0;
				}
				break;
			case 't':
				if(!sscanf(optarg, "%d", target) || __builtin_popcount(*target) != 1 || *target > 9999)
				{
					fprintf(stderr, "Invalid argument to -t option, expected positive number which is a power of 2 and less than 10000\n");
					return 0;
				}
				break;
			case 'a':
				if(!sscanf(optarg, "%d", range) || (*range) < 1 || (*range) > 13)
				{
					fprintf(stderr, "Invalid argument to -a option, expected positive number between 1 and 13 (inclusive)\n");
					return 0;
				}
				break;
			case 'g':
				if(!sscanf(optarg, "%d", numgen) || (*numgen) < 1)
				{
					fprintf(stderr, "Invalid argument to -g option, expected positive number greater than 0\n");
					return 0;
				}
				break;
			case 'h':
				usage();
				exit(0);
			case '?':
				return 0;
			default:
				abort();
		}
	return 1;
}

int main(int argc, char* argv[])
{
	int rows = DEFAULT_ROWS;
	int cols = DEFAULT_COLS;
	int target = DEFAULT_TARGET;
	int range = DEFAULT_RANGE;
	int numgen = DEFAULT_NUMGEN;
	if(!parse_args(argc, argv, &rows, &cols, &target, &range, &numgen))
		return 1;
	struct termios ttystate;
	init_terminal(&ttystate);
	game(rows, cols, target, range, numgen);
	restore_terminal(&ttystate);
	return 0;
}

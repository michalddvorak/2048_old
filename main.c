#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include "board.h"
#include "printer.h"
#include "io.h"

#define DEFAULT_ROWS 4
#define DEFAULT_COLS 4
#define DEFAULT_TARGET 2048
#define DEFAULT_RANGE 2
#define DEFAULT_NUMGEN 1
int generate_rand_idx(const struct board* board, int* row_idx, int* col_idx)
{
	int cnt = 0;
	for(int i = 0; i < board->m_rows; ++i)
		for(int j = 0; j < board->m_cols; ++j)
			if(board->m_arr[i][j] == 0)
				cnt++;
	
	if(cnt == 0)
		return 0;
	
	int r = rand() % cnt;
	
	for(int i = 0; i < board->m_rows; ++i)
		for(int j = 0; j < board->m_cols; ++j)
			if(board->m_arr[i][j] == 0 && r-- == 0)
			{
				*row_idx = i;
				*col_idx = j;
				return 1;
			}
	return 1;
}

#define ROW 1
#define COL 2
int bcheck(const struct board* board, int idx, int dir, int rc)
{
	if(dir == 1)
	{
		//Note that when iterating over rows, the boundary is the number of columns and vice versa
		if(rc == ROW)
			return idx < board->m_cols;
		else //rc == COL
			return idx < board->m_rows;
	}
	else //dir == -1
		return idx >= 0;
}
int calc_line(struct board* board, int i, int dir)
{
	int nline[board->m_cols];
	memset(nline, 0, sizeof(nline));
	int nlineidx, j;
	if(dir == 1)
		nlineidx = j = 0;
	else
		nlineidx = j = board->m_cols - 1;
	
	while(1)
	{
		while(bcheck(board, j, dir, ROW) && board->m_arr[i][j] == 0)
			j += dir;
		if(!bcheck(board, j, dir, ROW))
			break;
		int cur = board->m_arr[i][j];
		j += dir;
		while(bcheck(board, j, dir, ROW) && board->m_arr[i][j] == 0)
			j += dir;
		if(!bcheck(board, j, dir, ROW))
		{
			nline[nlineidx] = cur;
			break;
		}
		int next = board->m_arr[i][j];
		if(cur == next)
		{
			nline[nlineidx] = cur + next;
			nlineidx += dir;
			j += dir;
		}
		else
		{
			nline[nlineidx] = cur;
			nlineidx += dir;
		}
	}
	int res = 0;
	for(j = 0; j < board->m_cols; ++j)
	{
		res = res || board->m_arr[i][j] != nline[j];
		board->m_arr[i][j] = nline[j];
	}
	return res;
}

int calc_column(struct board* board, int j, int dir)
{
	int ncolumn[board->m_rows];
	memset(ncolumn, 0, sizeof(ncolumn));
	int ncolumnidx, i;
	if(dir == 1)
		ncolumnidx = i = 0;
	else
		ncolumnidx = i = board->m_rows - 1;
	
	while(1)
	{
		while(bcheck(board, i, dir, COL) && board->m_arr[i][j] == 0)
			i += dir;
		if(!bcheck(board, i, dir, COL))
			break;
		int cur = board->m_arr[i][j];
		i += dir;
		while(bcheck(board, i, dir, COL) && board->m_arr[i][j] == 0)
			i += dir;
		if(!bcheck(board, i, dir, COL))
		{
			ncolumn[ncolumnidx] = cur;
			break;
		}
		int next = board->m_arr[i][j];
		if(cur == next)
		{
			ncolumn[ncolumnidx] = cur + next;
			ncolumnidx += dir;
			i += dir;
		}
		else
		{
			ncolumn[ncolumnidx] = cur;
			ncolumnidx += dir;
		}
	}
	int res = 0;
	for(i = 0; i < board->m_rows; ++i)
	{
		res = res || board->m_arr[i][j] != ncolumn[i];
		board->m_arr[i][j] = ncolumn[i];
	}
	return res;
}

int move_left(struct board* board)
{
	int res = 0;
	for(int i = 0; i < board->m_rows; ++i)
		res = calc_line(board, i, 1) || res;
	return res;
}

int move_right(struct board* board)
{
	int res = 0;
	for(int i = 0; i < board->m_rows; ++i)
		res = calc_line(board, i, -1) || res;
	return res;
}

int move_down(struct board* board)
{
	int res = 0;
	for(int j = 0; j < board->m_cols; ++j)
		res = calc_column(board, j, -1) || res;
	return res;
}

int move_up(struct board* board)
{
	int res = 0;
	for(int j = 0; j < board->m_cols; ++j)
		res = calc_column(board, j, 1) || res;
	return res;
}

#define LEFT 0
#define RIGHT 1
#define UP 2
#define DOWN 3

//returns true if board changed
int move(int dir, struct board* main_board, struct board* tmp_board)
{
	static int (* moves[4])(struct board*) = {move_left, move_right, move_up, move_down};
	return moves[dir](main_board);
}

int islost(const struct board* board)
{
	//The game is lost if and only if there is no space (all nonzero)
	//and across all columns and rows there are no two adjacent same numbers
	
	for(int i = 0; i < board->m_rows; ++i)
		for(int j = 0; j < board->m_cols; ++j)
			if(board->m_arr[i][j] == 0)
				return 0;
	
	for(int i = 0; i < board->m_rows; ++i)
		for(int j = 0; j < board->m_cols - 1; ++j)
			if(board->m_arr[i][j] == board->m_arr[i][j + 1])
				return 0;
	
	for(int j = 0; j < board->m_cols; ++j)
		for(int i = 0; i < board->m_rows - 1; ++i)
			if(board->m_arr[i][j] == board->m_arr[i + 1][j])
				return 0;
	return 1;
}

int iswon(const struct board* board, int target)
{
	for(int i = 0; i < board->m_rows; ++i)
		for(int j = 0; j < board->m_cols; ++j)
			if(board->m_arr[i][j] == target)
				return 1;
	return 0;
}

void put_random(struct board* board, int range, int numgen)
{
	for(int n = 0; n < numgen; ++n)
	{
		int i, j;
		if(!generate_rand_idx(board, &i, &j))
			return;
		int r = 1 + rand() % range;
		board->m_arr[i][j] = 1 << r;
	}
}

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


int get_score(const struct board* board)
{
	int score = 0;
	for(int i = 0; i < board->m_rows; ++i)
		for(int j = 0; j < board->m_cols; ++j)
			score += board->m_arr[i][j];
	return score;
}


#define EXIT 0
#define LOST 1
#define WON 2

void handle_end(int state, const struct board* board)
{
	static const char* strs[] = {"Exited", "You lost :(", "You won!"};
	puts(strs[state]);
	printf("Your score: %d\n", get_score(board));
}

/**
 * @brief
 * @param rows - number of rows
 * @param cols - number of columns
 * @param target - targenumber which, if present, the game is won
 * @param range - number range to be generated, the following will be generated: 2^1, 2^2, ... 2^range
 * @param numgen - number of new tiles to be generated
 */
void game(int rows, int cols, int target, int range, int numgen)
{
	struct board main_board;
	struct board tmp_board;
	main_board.m_rows = tmp_board.m_rows = rows;
	main_board.m_cols = tmp_board.m_cols = cols;
	alloc_board(&main_board);
	alloc_board(&tmp_board);
	zero_board(&main_board);
	put_random(&main_board, range, numgen);
	print_board(&main_board);
	while(1)
		if(kbhit())
		{
			int m = -1; //move
			switch(getc(stdin))
			{
				case 'a':
					m = move(LEFT, &main_board, &tmp_board);
					break;
				case 'd':
					m = move(RIGHT, &main_board, &tmp_board);
					break;
				case 's':
					m = move(DOWN, &main_board, &tmp_board);
					break;
				case 'w':
					m = move(UP, &main_board, &tmp_board);
					break;
				case 'x':
					handle_end(EXIT, &main_board);
					return;
				default:
					continue;
			}
			if(m == 1)
			{
				put_random(&main_board, range, numgen);
				print_board(&main_board);
			}
			if(islost(&main_board))
			{
				handle_end(LOST, &main_board);
				return;
			}
			if(iswon(&main_board, target))
			{
				handle_end(WON, &main_board);
				return;
			}
		}
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
	srand(time(NULL));
	clear_screen();
	game(rows, cols, target, range, numgen);
	restore_terminal(&ttystate);
	return 0;
}

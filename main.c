#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include "board.h"
#include "printer.h"
#include "io.h"

#define M 4
#define N 4


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
		if(rc == ROW)
			return idx < board->m_rows;
		else //rc == COL
			return idx < board->m_cols;
	}
	else //dir == -1
		return idx >= 0;
}
void calc_line(struct board* board, int i, int dir)
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
	
	for(j = 0; j < board->m_cols; ++j)
		board->m_arr[i][j] = nline[j];
}

void calc_column(struct board* board, int j, int dir)
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
	for(i = 0; i < M; ++i)
		board->m_arr[i][j] = ncolumn[i];
}

void move_left(struct board* board)
{
	for(int i = 0; i < board->m_rows; ++i)
		calc_line(board, i, 1);
}

void move_right(struct board* board)
{
	for(int i = 0; i < board->m_rows; ++i)
		calc_line(board, i, -1);
}

void move_down(struct board* board)
{
	for(int j = 0; j < board->m_cols; ++j)
		calc_column(board, j, -1);
}

void move_up(struct board* board)
{
	for(int j = 0; j < board->m_cols; ++j)
		calc_column(board, j, 1);
}


int islost(const struct board* board)
{
	struct board tmp_board;
	tmp_board.m_rows = board->m_rows;
	tmp_board.m_cols = board->m_cols;
	alloc_board(&tmp_board);
	clone_board(board, &tmp_board);
	int ret = 1;
	move_left(&tmp_board);
	if(!is_equal_board(&tmp_board, board))
	{
		ret = 0;
		goto ret;
	}
	move_right(&tmp_board);
	if(!is_equal_board(&tmp_board, board))
	{
		ret = 0;
		goto ret;
	}
	move_down(&tmp_board);
	if(!is_equal_board(&tmp_board, board))
	{
		ret = 0;
		goto ret;
	}
	move_up(&tmp_board);
	if(!is_equal_board(&tmp_board, board))
	{
		ret = 0;
		goto ret;
	}
ret:
	free_board(&tmp_board);
	return ret;
}

int iswon(const struct board* board)
{
	for(int i = 0; i < board->m_rows; ++i)
		for(int j = 0; j < board->m_cols; ++j)
			if(board->m_arr[i][j] == 2048)
				return 1;
	return 0;
}

void put_random(struct board* board)
{
	int i, j;
	if(!generate_rand_idx(board, &i, &j))
		return;
	
	int poss_vals[] = {2, 4};
	
	int r = rand() % (sizeof(poss_vals) / sizeof(*poss_vals));
	board->m_arr[i][j] = poss_vals[r];
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
	static const char* strs [] = { "Exited", "You lost :(", "You won!" };
	puts(strs[state]);
	printf("Your score: %d\n", get_score(board));
}


int main()
{
	struct termios ttystate;
	init_terminal(&ttystate);
	clear_screen();
	struct board board[2];
	for(int i = 0; i < 2; ++i)
	{
		board[i].m_rows = M;
		board[i].m_cols = N;
		alloc_board(&board[i]);	
	}
	zero_board(&board[0]);
	put_random(&board[0]);
	print_board(&board[0]);
	srand(time(NULL));
	while(1)
	{
		if(kbhit())
		{
			clone_board(&board[0], &board[1]);
			int c = getc(stdin);
			if(c == 'a')
				move_left(&board[0]);
			else if(c == 'd')
				move_right(&board[0]);
			else if(c == 's')
				move_down(&board[0]);
			else if(c == 'w')
				move_up(&board[0]);
			else if(c == 'x')
			{
				handle_end(EXIT,&board[0]);
				break;
			}
			else
				continue;
			if(!is_equal_board(&board[0], &board[1]))
				put_random(&board[0]);
			print_board(&board[0]);
			if(islost(&board[0]))
			{
				handle_end(LOST, &board[0]);
				break;
			}
			if(iswon(&board[0]))
			{
				handle_end(WON, &board[0]);
				break;
			}
		}
	}
	for(int i = 0; i < 2; ++i)
		free_board(&board[i]);
	restore_terminal(&ttystate);
	return 0;
}

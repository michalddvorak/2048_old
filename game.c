#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "game.h"
#include "board.h"
#include "printer.h"
#include "io.h"


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

int bound_check(const struct board* board, int idx, int dir, int rc)
{
	if(dir == 1)
	{
		//when iterating over rows, the boundary is the number of columns and vice versa
		if(rc == ROW)
			return idx < board->m_cols;
		else //rc == COL
			return idx < board->m_rows;
	}
	else //dir == -1
		return idx >= 0;
}

//dir = 1   ->
//dir = -1  <-
int update_line(struct board* board, int i, int dir)
{
	int new_line[board->m_cols];
	memset(new_line, 0, sizeof(new_line));
	int new_line_idx;
	int j;
#define APPEND(x) new_line[new_line_idx] = x, new_line_idx += dir
	
	//This initializes the index to 0 if direction is ->
	//and to board->m_cols - 1 otherwise (direction is <-)
	new_line_idx = j = (dir == -1) * (board->m_cols - 1);
	
	while(1)
	{
		while(bound_check(board, j, dir, ROW) && board->m_arr[i][j] == 0)
			j += dir;
		if(!bound_check(board, j, dir, ROW))
			break;
		int cur = board->m_arr[i][j];
		j += dir;
		while(bound_check(board, j, dir, ROW) && board->m_arr[i][j] == 0)
			j += dir;
		if(!bound_check(board, j, dir, ROW))
		{
			new_line[new_line_idx] = cur;
			break;
		}
		int next = board->m_arr[i][j];
		if(cur == next)
		{
			APPEND(cur + next);
			j += dir;
		}
		else
			APPEND(cur);
	}
#undef APPEND
	int res = 0;
	for(j = 0; j < board->m_cols; ++j)
	{
		res = res || board->m_arr[i][j] != new_line[j];
		board->m_arr[i][j] = new_line[j];
	}
	return res;
}

//TODO: maybe this entire calculation can be done inplace?
int update_column(struct board* board, int j, int dir)
{
	int new_column[board->m_rows];
	memset(new_column, 0, sizeof(new_column));
	int new_column_idx;
	int i;
#define APPEND(x) new_column[new_column_idx] = x, new_column_idx += dir
	
	new_column_idx = i = (dir == -1) * (board->m_rows - 1);
	
	while(1)
	{
		while(bound_check(board, i, dir, COL) && board->m_arr[i][j] == 0)
			i += dir;
		if(!bound_check(board, i, dir, COL))
			break;
		int cur = board->m_arr[i][j];
		i += dir;
		while(bound_check(board, i, dir, COL) && board->m_arr[i][j] == 0)
			i += dir;
		if(!bound_check(board, i, dir, COL))
		{
			new_column[new_column_idx] = cur;
			break;
		}
		int next = board->m_arr[i][j];
		if(cur == next)
		{
			APPEND(cur + next);
			i += dir;
		}
		else
			APPEND(cur);
	}
#undef APPEND
	int res = 0;
	for(i = 0; i < board->m_rows; ++i)
	{
		res = res || board->m_arr[i][j] != new_column[i];
		board->m_arr[i][j] = new_column[i];
	}
	return res;
}

int move_left(struct board* board)
{
	int res = 0;
	for(int i = 0; i < board->m_rows; ++i)
		res = update_line(board, i, 1) || res;
	return res;
}

int move_right(struct board* board)
{
	int res = 0;
	for(int i = 0; i < board->m_rows; ++i)
		res = update_line(board, i, -1) || res;
	return res;
}

int move_down(struct board* board)
{
	int res = 0;
	for(int j = 0; j < board->m_cols; ++j)
		res = update_column(board, j, -1) || res;
	return res;
}

int move_up(struct board* board)
{
	int res = 0;
	for(int j = 0; j < board->m_cols; ++j)
		res = update_column(board, j, 1) || res;
	return res;
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


void game(int rows, int cols, int target, int range, int numgen)
{
	srand(time(NULL));
	clear_screen();
	struct board main_board;
	main_board.m_rows = rows;
	main_board.m_cols = cols;
	alloc_board(&main_board);
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
					m = move_left(&main_board);
					break;
				case 'd':
					m = move_right(&main_board);
					break;
				case 's':
					m = move_down(&main_board);
					break;
				case 'w':
					m = move_up(&main_board);
					break;
				case 'x':
					handle_end(EXIT, &main_board);
					goto ret;
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
				goto ret;
			}
			if(iswon(&main_board, target))
			{
				handle_end(WON, &main_board);
				goto ret;
			}
		}
ret:
	free_board(&main_board);
}




#include <string.h>
#include <stdlib.h>
#include "board.h"

void alloc_board(struct board* board)
{
	board->m_arr = malloc(board->m_rows * sizeof(int*));
	int* arr = malloc(board->m_rows * board->m_cols * sizeof(int));
	for(int i = 0; i < board->m_rows; ++i)
		board->m_arr[i] = arr + i * board->m_cols;
}

void zero_board(struct board* board)
{
	memset(*board->m_arr, 0, board->m_rows * board->m_cols * sizeof(int));
}

void free_board(struct board* board)
{
	if(board->m_arr)
		free(*board->m_arr);
	free(board->m_arr);
}

int is_equal_board(const struct board* b1, const struct board* b2)
{
	return !memcmp(*b1->m_arr, *b2->m_arr, b1->m_rows * b1->m_cols * sizeof(int));
}

void clone_board(const struct board* old_board, struct board* new_board)
{
	for(int i = 0; i < new_board->m_rows; ++i)
		for(int j = 0; j < new_board->m_cols; ++j)
			new_board->m_arr[i][j] = old_board->m_arr[i][j];
}
